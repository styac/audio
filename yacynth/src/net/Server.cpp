/*
 * Copyright (C) 2016 Istvan Simon -- stevens37 at gmail dot com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/*
 * File:   Server.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on February 20, 2016, 7:25 AM
 */

#include    "Server.h"
#include    <netinet/in.h>
#include    <arpa/inet.h>
#include    <unistd.h>
#include    <errno.h>
#include    <linux/random.h>
#include    <sys/syscall.h>
#include    "sha3.h"

namespace yacynth {

// --------------------------------------------------------------------

Server::Server( Sysman&  sysmanP, const uint16_t portP  )
:   sysman(sysmanP)
,   logger(spdlog::stdout_color_mt("console"))
,   socketListen(-1)
,   socketAccept(-1)
,   errnoNet(0)
,   connected(false)
,   authenticated(false)
,   stopServer(false)
{
    struct sockaddr_in serv_addr;
    socketListen = socket( AF_INET, SOCK_STREAM, 0 );
    memset( &serv_addr, '0', sizeof(serv_addr) );
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(portP);
    bind( socketListen, (struct sockaddr*)&serv_addr, sizeof(serv_addr) );
    if( socketListen < 1 ) {
        errnoNet = errno;
    }
    memset( seedAuth, 0, sizeof(seedAuth));
} // end Server::Server

// --------------------------------------------------------------------

Server::~Server()
{
    if(socketListen>0) {
        shutdown( socketListen, SHUT_RDWR );
        close( socketListen );
        socketListen = -1;
    }
    shut();
}

// --------------------------------------------------------------------

bool Server::run()
{
    if( errnoNet ) {
        logger->warn("ctor failed {:d}", errnoNet);
        return false;
    }
    logger->warn(" ***** Server::doListen" );
    if( ! doListen() ) {
        logger->warn(" ***** Server::doListen failed" );
        return false;
    }
    while( ! stopServer ) {
        logger->warn(" ***** Server::doAccept" );
        if( ! doAccept() ) {
            logger->warn(" ***** Server::doAccept failed" );
//            return false;
        }
        if( ! authenticate() ) {
            logger->warn(" ***** Server::authenticate failed" );
//            return false;
        }
        execute();
        logger->warn(" ***** Server::connection broken" );
    }
    shut();
    return true;
} // end Server::run

void Server::shut()
{
    if( socketAccept > 0 ) {
        shutdown( socketAccept, SHUT_RDWR );
        close( socketAccept );
        socketAccept = -1;
    }
}

void Server::execute()
{
    std::string str;
    lastSequenceNumber = 0;
    while( doRecv() ) {
        if( lastSequenceNumber++ != message.sequenceNr ) {
            logger->warn( "** seq nr diff {0} {1}", lastSequenceNumber, message.sequenceNr );
        }
        // execute received command
        logger->warn("** execute command {0}", message.print(str).data() );
        switch( message.messageType ) {
        case yaxp::MessageT::requestC2E:
            sysman.evalMessage( message );
            break;

        case yaxp::MessageT::stopServer:
            stopServer = true;
            logger->warn("** stop server" );
            // response ??
            return;

        default:
            message.messageType = yaxp::MessageT::illegalContext;
            message.length = 0;
        }
        // send response
        if( ! doSend() ) {
            return;
        }
    }
}

bool Server::doAccept()
{
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    socketAccept = accept( socketListen, (struct sockaddr*)NULL, NULL );
    if( socketAccept < 1 ) {
        errnoNet = errno;
        logger->warn("doAccept failed {:d}", errnoNet);
        return false;
    }

//    if( setsockopt (socketAccept, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0 )
//        logger->warn("setsockopt failed {:d}", errnoNet);
//    if( setsockopt (socketAccept, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0 )
//        logger->warn("setsockopt failed {:d}", errnoNet);
    return true;
}

bool Server::doListen()
{
    int res = listen(socketListen, 1);
    if( res < 0 ) {
        errnoNet = errno;
        logger->warn("doListen failed {:d}", errnoNet);
        return false;
    }
    return true;
}

bool Server::authenticate()
{
    uint8_t randBlock[ randLength ];
    std::string str;
    constexpr int msglen = 13;

    fillRandom( randBlock );
    logger->warn(" ***** Server::authenticate" );
    char auth[msglen] = {"****auth****"};
    message.getTargetData( randBlock );
    message.messageType = yaxp::MessageT::authRequest;
    message.sequenceNr = 0;
    if( ! doSend() ) {
        logger->warn(" ***** Server::authenticate doSend false" );
        return false;
    }
    sleep(1);
    // PEEK
    if( ! doPeek() ) {
        logger->warn(" ***** Server::authenticate no response" );
        return false;
    }

    if( ! doRecv() ) {
        logger->warn(" ***** Server::authenticate doRecv false" );
        return false;
    }
    if( message.messageType != yaxp::MessageT::authResponse ) {
        logger->warn(" ***** Server::authenticate mst type error" );
        return false;
    }

    message.print(str);
    logger->warn( "authenticate received {0}", str.data() );

    if( checkAuth( randBlock, message.data, message.length ) ) {
        logger->warn( "authenticate ok" );
        authenticated = true;
        return true;
    }

    logger->warn( "authenticate error" );
    return false;
}

bool Server::doRecv()
{
    std::string str;
    logger->warn(" ***** Server::doRecv" );
    if( ! recvAll( (char *)&message, sizeof(yaxp::Header) ) ) {
        logger->warn( "header received error" );
        return false;
    }
    if( message.messageType < yaxp::MessageT::validLength ) {
        message.print(str);
        logger->warn( "received {0}", str.data() );
        return true;
    }
    if( ! recvAll( (char *)message.data, message.length ) ) {
        logger->warn( "body received error" );
        return false;
    }
    message.print(str);
    logger->warn( "message {0} ", str.data() );
    return true;
}

bool Server::recvAll( char *p, uint32_t size )
{
    if(size==0)
        return true;
    ssize_t s = (ssize_t) size;
    while(true) {
        int res = recv( socketAccept, p, s, 0 );
        if( res == s )
            return true;
        if( res == 0 ) {
            logger->warn("recvAll disconnect");
            errnoNet = ENOTCONN;
            connected = false;
            shut();
            return false;
        }
        if( res < 0 ) {
            if( EAGAIN == errno ) {
                logger->warn("recvAll EAGAIN");
                continue;
            }
            errnoNet = errno;
            logger->warn("recvAll failed {:d}", errnoNet);
            return false;
        }
        p += res;
        s -= res;
    }
}

bool Server::doSend()
{
    logger->warn(" ***** Server::doSend" );
    std::size_t length = message.length + sizeof(yaxp::Header);
    if( message.messageType < yaxp::MessageT::validLength ) {
        length = sizeof(yaxp::Header);
    }

    int res = send(socketAccept, (char *)&message, length, 0);
    if( res == length)
        return true;
    if( res < 0 ) {
        errnoNet = errno;
        logger->warn("doSend failed {:d}", errnoNet);
        return false;
    }
}

bool Server::doPeek()
{
    return true;
} // end Server::doAwaitStop


bool Server::fillRandom( uint8_t * dst )
{
    int rf = open( "/dev/urandom", O_RDONLY );
    if( rf > 0 ) {
        int res = read( rf, dst, randLength );
        if( res == randLength ) {
            close( rf );
            return true;
        }
        logger->warn("urandom read error {:d}", errno );
        close( rf );
        return false;
    }
    logger->warn("urandom open error {:d}", errno );
    return false;
}

void Server::setAuthSeed( const uint8_t * src, size_t lng )
{
    memcpy( seedAuth, src, std::min( lng, sizeof(seedAuth)));
    // xor ( seedAuth, "VERSION 1" ); // can auth only the correct version
}

bool Server::checkAuth( const uint8_t * randBuff, const uint8_t * respBuff, size_t respLng )
{
    sha3_ctx_t  context;
    uint8_t resBlock[ authLength ];
    if( authLength != respLng ) {
        return false;
    }
    sha3_init( &context, authLength);
    sha3_update( &context, seedAuth, sizeof(seedAuth));
    sha3_update( &context, randBuff, randLength);
    sha3_final( resBlock, &context );
    return memcmp( resBlock, respBuff, authLength) == 0;
}

// --------------------------------------------------------------------

} // end namespace yacynth



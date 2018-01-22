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
#include    <unistd.h>
#include    <errno.h>
#include    <fstream>
#include    <linux/random.h>
#include    <sys/syscall.h>
#include    "sha3.h"

namespace yacynth {
namespace net {

void printHex( const uint8_t *buf, uint16_t lng, const char *name )
{
    if( buf==0 || lng==0 || name==0 )
        return;

    std::cout << "\n " << name << std::hex << std::setw(2) << std::endl;
    for( auto i=0u; i<lng; ++i ) {
        std::cout << uint16_t(buf[i]) << " ";
    }
    std::cout << std::endl;
}

Server::Type Server::create( Sysman& sysman, const Setting& setting )
{
    switch( setting.getConnMode() ) {
    case yaxp::CONN_MODE::CONNECTION_REMOTE: {
            Server::Type me( new RemoteServer( sysman, setting.getControlPortRemote(), setting.getAuthKeyFile() ));
            return me;
        }
    case yaxp::CONN_MODE::CONNECTION_LOCAL: {
            Server::Type me( new LocalServer( sysman, setting.getControlPortLocal(), setting.getAuthKeyFile() ));
            return me;
        }
    default:
        throw std::runtime_error("illegal connection type");
    }
}

Server::Server( Sysman& sysmanP, const std::string& authKeyFile )
:   sysman(sysmanP)
,   logger(spdlog::stdout_color_mt("console"))
,   socketListen(-1)
,   socketAccept(-1)
,   socketSendStatus(-1)
,   errnoNet(0)
,   connected(false)
,   authenticated(false)
,   stopServer(false)
{
    if( ! setAuthSeed( authKeyFile ) ) {
        throw std::runtime_error("can't read the authentication data");
    }
}

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
        }
        if( ! authenticate() ) {
            logger->warn(" ***** Server::authenticate failed" );
            shut();
        }
        execute();
        logger->warn(" ***** Server::connection broken" );
    }
    shut();
    return true;
} // end Server::run

void Server::shut()
{
    logger->warn(" ***** Server::shut" );
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
        if( ++lastSequenceNumber != message.sequenceNr ) {
            logger->warn( "** seq nr diff {0} {1}", lastSequenceNumber, message.sequenceNr );
        }
        lastMessageType = message.messageType;
        switch( message.messageType ) {
        case yaxp::MessageT::requestC2E:
            sysman.evalMessage( message );
            if( lastMessageType == message.messageType ) {
                // the response was not set by programming error
                message.messageType = yaxp::MessageT::internalError;
                message.length = 0;
            }
            break;

        case yaxp::MessageT::stopServer:
            stopServer = true;
            logger->warn("** stop server" );
            // response : connection shutdown
            return;

        default:
            message.messageType = yaxp::MessageT::illegalContext;
            message.length = 0;
            logger->warn("** illegalContext" );
        }
        // send response
        if( ! doSend() ) {
            return;
        }
    }
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
    fillRandom( randBlock );
    logger->warn(" ***** Server::authenticate" );
    message.getTargetData( randBlock );
    message.messageType = yaxp::MessageT::authRequest;
    message.sequenceNr = 0;
    if( ! doSend() ) {
        logger->warn(" ***** Server::authenticate doSend false" );
        return false;
    }

    if( ! doPeek() ) {
        timespec req;
        req.tv_nsec = 1000*1000*10;
        req.tv_sec = 0;
        logger->warn(" ***** Server::authenticate doPeek false" );
        nanosleep(&req,nullptr); // 10 millisec
        if( ! doPeek() ) { // no response
            logger->warn(" ***** Server::authenticate no response" );
            return false;
        }
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
    //logger->warn(" ***** Server::doRecv" );
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
    //logger->warn( "message {0} ", str.data() );
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
    std::size_t length = message.length + sizeof(yaxp::Header);
    if( message.messageType < yaxp::MessageT::validLength ) {
        length = sizeof(yaxp::Header);
    }
    int res = send(socketAccept, (char *)&message, length, 0);
    if( res == int(length) )
        return true;
    if( res < 0 ) {
        errnoNet = errno;
        logger->warn("doSend failed {:d}", errnoNet);
        return false;
    }
     // this should never happen
    logger->warn("doSend sent more then requested {:d}",res);
    return false;
}

bool Server::doPeek()
{
    constexpr int s = 32;
    char p[ s ];
    int res = recv( socketAccept, p, s, MSG_PEEK );
    return res > 0;
} // end Server::doPeek


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

bool Server::setAuthSeed( const std::string& seedFileName )
{
    std::ifstream seedFile( seedFileName );
    if( ! seedFile.is_open() ) {
        logger->warn("setAuthSeed open err {:d}", errno );
        return false;
    }
    seedFile.read( seedAuth, sizeof(seedAuth));
    if( seedFile.fail() ) {
        logger->warn("setAuthSeed read err {:d}", errno );
        return false;
    }
    seedFile.close();
    return true;
}

bool Server::checkAuth( const uint8_t * randBuff, const uint8_t * respBuff, size_t respLng )
{
    sha3_ctx_t  context;
    uint8_t resBlock[ authLength ];
    if( authLength > respLng ) {
        return false;
    }
    sha3_init( &context, authLength);
    sha3_update( &context, seedAuth, sizeof(seedAuth));
    sha3_update( &context, randBuff, randLength);
    sha3_final( resBlock, &context );
    return 0 == memcmp( resBlock, respBuff, authLength);
}

// --------------------------------------------------------------------
RemoteServer::RemoteServer( Sysman& sysman, const uint16_t port, const std::string& authKeyFile )
:   Server(sysman, authKeyFile)
,   portControl(port)   // on the server side
,   portStatus(port+1)    // on the client side

{
    memset( &statusSockAddr6, '0', sizeof(statusSockAddr6) );
    sockaddr_in serv_addr;
    socketListen = socket( AF_INET, SOCK_STREAM, 0 );
    memset( &serv_addr, '0', sizeof(serv_addr) );
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(portControl);
    bind( socketListen, (sockaddr*)&serv_addr, sizeof(serv_addr) );
    if( socketListen < 1 ) {
        errnoNet = errno;
    }
    //create a UDP socket
    socketSendStatus = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if( socketSendStatus < 1 ) {
        errnoNet = errno;
    }
} // end Server::Server

// --------------------------------------------------------------------

RemoteServer::~RemoteServer()
{
    if(socketListen>0) {
        shutdown( socketListen, SHUT_RDWR );
        close( socketListen );
        socketListen = -1;
    }
    if(socketAccept>0) {
        shutdown( socketAccept, SHUT_RDWR );
        close( socketAccept );
        socketAccept = -1;
    }
    if(socketSendStatus>0) {
        shutdown( socketSendStatus, SHUT_RDWR );
        close( socketSendStatus );
        socketSendStatus = -1;
    }
    shut();
}

// send UDP -- max size 512
bool RemoteServer::sendStatus( const yacynth::yaxp::Message& message )
{
    const uint32_t size = yacynth::yaxp::Message::headerSize + message.length;
    if( size > maxStatusSize ) {
        return false;
    }
    if( 0 != sendto( socketSendStatus, (void *)&message, size, 0, (const sockaddr *)&statusSockAddr4, sizeof(statusSockAddr4)) ){
        errnoNet = errno;
        logger->warn("doAccept failed {:d}", errnoNet);
        return false;
    }
    return true;
}

bool RemoteServer::doAccept()
{
    sockaddr_in     remoteSock;
    socklen_t       addrlen = sizeof(remoteSock);

    socketAccept = accept( socketListen, (struct sockaddr*)&remoteSock, &addrlen );
    if( socketAccept < 1 ) {
        errnoNet = errno;
        logger->warn("doAccept failed {:d}", errnoNet);
        return false;
    }

    switch( remoteSock.sin_family ) {
    case AF_INET:
        statusSockAddr4.sin_family = AF_INET;
        statusSockAddr4.sin_addr = remoteSock.sin_addr;
        statusSockAddr4.sin_port = htons(portStatus);
        break;
//    case AF_INET6:
//        statusSockAddr6.sin_family = AF_INET6;
//        statusSockAddr6.sin_addr = remoteSock.sin_addr;
//        statusSockAddr6.sin_port = htons(portStatus);
//        break;
    }
    return true;
}

// --------------------------------------------------------------------

LocalServer::LocalServer( Sysman&  sysman, const char *port, const std::string& authKeyFile )
:   Server(sysman, authKeyFile)
,   portControl(port)
,   portStatus(port)
{
    portControl += yaxp::localPortControlSuffix;
    portStatus += yaxp::localPortStatusSuffix;
    sockaddr_un serv_addr;
    socketListen = socket( PF_LOCAL, SOCK_STREAM, 0 );
    unlink(portControl.data());
    memset( &serv_addr, '0', sizeof(serv_addr) );
    serv_addr.sun_family = AF_LOCAL;
    strncpy( serv_addr.sun_path, portControl.data(), sizeof(serv_addr.sun_path) );
    bind( socketListen, (sockaddr*)&serv_addr, sizeof(serv_addr) );
    if( socketListen < 1 ) {
        errnoNet = errno;
    }
    socketSendStatus = socket( PF_LOCAL, SOCK_DGRAM, 0 );
    if( socketSendStatus < 1 ) {
        errnoNet = errno;
    }
} // end Server::Server

// --------------------------------------------------------------------

LocalServer::~LocalServer()
{
    if(socketListen>0) {
        shutdown( socketListen, SHUT_RDWR );
        close( socketListen );
        socketListen = -1;
    }
    if(socketAccept>0) {
        shutdown( socketAccept, SHUT_RDWR );
        close( socketAccept );
        socketAccept = -1;
    }
    if(socketSendStatus>0) {
        shutdown( socketSendStatus, SHUT_RDWR );
        close( socketSendStatus );
        socketSendStatus = -1;
    }

    shut();
}

bool LocalServer::sendStatus( const yacynth::yaxp::Message& message )
{
    const uint32_t size = yacynth::yaxp::Message::headerSize + message.length;
    if( size > maxStatusSize ) {
        return false;
    }

    if( 0 != sendto( socketSendStatus, (void *)&message, size, 0, (const sockaddr *)&statusSockAddr, sizeof(statusSockAddr)) ){
        errnoNet = errno;
        logger->warn("doAccept failed {:d}", errnoNet);
        return false;
    }
    return true;
}

bool LocalServer::doAccept()
{
    socketAccept = accept( socketListen, (sockaddr*)NULL, NULL );
    if( socketAccept < 1 ) {
        errnoNet = errno;
        logger->warn("doAccept failed {:d}", errnoNet);
        return false;
    }
    return true;
}

// --------------------------------------------------------------------

} // end namespace yacnet
} // end namespace yacynth



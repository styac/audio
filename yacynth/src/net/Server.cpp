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

#include "yacynth_config.h"
#include "Server.h"

#include "control/Nsleep.h"
#include "blake2.h"
#include "logDefs.h"

#include <unistd.h>
#include <errno.h>
#include <fstream>
#include <linux/random.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace yacynth {
namespace net {

namespace {
constexpr auto LogCategoryMask              = LOGCAT_net;
constexpr auto LogCategoryMaskAlways        = LogCategoryMask | nanolog::LogControl::log_always;
constexpr const char * const LogCategory    = "NETS";
}

// singleton
Server * Server::instance = nullptr;

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

bool Server::create( Sysman& sysman, const Setting& setting )
{
    switch( setting.getConnMode() ) {
    case yaxp::CONN_MODE::CONNECTION_REMOTE:
        instance = new RemoteServer( sysman, setting.getControlPortRemote(), setting.getAuthKeyFile() );
        return ! instance->createFailed;
//  case yaxp::CONN_MODE::CONNECTION_LOCAL: {
    default:
        instance = new LocalServer( sysman, setting.getControlPortLocal(), setting.getAuthKeyFile() );
        return ! instance->createFailed;
    }
}

Server::Server( Sysman& sysmanP, const std::string& authKeyFile )
: sysman(sysmanP)
, socketListen(-1)
, socketAccept(-1)
, socketSendStatus(-1)
, errnoNet(0)
, connected(false)
, authenticated(false)
, stopServer(false)
, createFailed(false)
{
    if( ! setAuthSeed( authKeyFile ) ) {
        createFailed = true;
        LOG_CRIT_CAT(LogCategoryMask,LogCategory) << "Server setAuthSeed failed";
    }
}

// TODO: make singleton and call  stop from signal handler to avoid TIME_WAIT
void Server::stop()
{
    std::lock_guard<std::mutex> lk(stopMutex);
    if(stopServer) {
        return;
    }
    stopServer = true;
    shut();
    close(socketListen);
    socketListen = -1;
    nsleep(1000*1000*100);
}

bool Server::run()
{
    if( errnoNet ) {
        LOG_INFO_CAT(LogCategoryMask,LogCategory) << "Server ctor failed:" << errnoNet;
        return false;
    }
    LOG_INFO_CAT(LogCategoryMask,LogCategory) << "doListen";

    if( ! doListen() ) {
        LOG_INFO_CAT(LogCategoryMask,LogCategory) << "doListen failed";
        return false;
    }
    while( ! stopServer ) {
        LOG_INFO_CAT(LogCategoryMask,LogCategory) << "doAccept";
        if( ! doAccept() ) {
            LOG_INFO_CAT(LogCategoryMask,LogCategory) << "doAccept failed";
        }
        if( stopServer ) {
            shut();
            return true;
        }
        if( ! authenticate() ) {
            LOG_INFO_CAT(LogCategoryMask,LogCategory) << "authenticate failed";
            shut();
        }
        execute();
        LOG_INFO_CAT(LogCategoryMask,LogCategory) << "connection broken";
    }
    shut();
    return true;
} // end Server::run

void Server::shut()
{
    if( socketAccept > 0 ) {
        LOG_INFO_CAT(LogCategoryMask,LogCategory) << "shut";

        shutdown( socketAccept, SHUT_RDWR );
        close( socketAccept );
        socketAccept = -1;
        nsleep(1000);
    }
}

void Server::execute()
{
    std::string str;
    lastSequenceNumber = 0;
    while( doRecv() ) {
        if( ++lastSequenceNumber != message.sequenceNr ) {
            LOG_INFO_CAT(LogCategoryMask,LogCategory) << "seq nr diff:" << lastSequenceNumber << " " << message.sequenceNr ;
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
            LOG_INFO_CAT(LogCategoryMask,LogCategory) << "Server stop";
            return;

        case yaxp::MessageT::heartbeatRequest:
            LOG_INFO_CAT(LogCategoryMask,LogCategory) << "heartbeatRequest";
            message.messageType = yaxp::MessageT::heartbeatResponse;
            message.length = 0;
            break;

        default:
            message.messageType = yaxp::MessageT::illegalContext;
            message.length = 0;
            LOG_INFO_CAT(LogCategoryMask,LogCategory) << "illegalContext";
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
        LOG_INFO_CAT(LogCategoryMask,LogCategory) << "doListen failed:" << errnoNet;
        return false;
    }
    return true;
}

bool Server::authenticate()
{
    uint8_t randBlock[ randLength ];
    std::string str;
    fillRandom( randBlock );
    LOG_INFO_CAT(LogCategoryMask,LogCategory) << "authenticate";
    message.getTargetData( randBlock );
    message.messageType = yaxp::MessageT::authRequest;
    message.sequenceNr = 0;
    if( ! doSend() ) {
        LOG_CRIT_CAT(LogCategoryMask,LogCategory) << "authenticate doSend false";
        return false;
    }

    if( ! doPeek() ) {
        nsleep(1000*1000*10);
        LOG_INFO_CAT(LogCategoryMask,LogCategory) << "authenticate doPeek false";
        if( ! doPeek() ) { // no response
            LOG_INFO_CAT(LogCategoryMask,LogCategory) << "authenticate no response";
            return false;
        }
    }

    if( ! doRecv() ) {
        LOG_INFO_CAT(LogCategoryMask,LogCategory) << "authenticate doRecv false";
        return false;
    }
    if( message.messageType != yaxp::MessageT::authResponse ) {
        LOG_INFO_CAT(LogCategoryMask,LogCategory) << "authenticate mst type error";
        return false;
    }

    message.print(str);
    LOG_INFO_CAT(LogCategoryMask,LogCategory) << "authenticate received: " << str;

    if( checkAuth( randBlock, message.data, message.length ) ) {
        LOG_INFO_CAT(LogCategoryMask,LogCategory) << "authenticate ok";
        authenticated = true;
        return true;
    }

    LOG_INFO_CAT(LogCategoryMask,LogCategory) << "authenticate error";
    return false;
}

bool Server::doRecv()
{
    std::string str;
    //logger->warn("doRecv" );
    if( ! recvAll( (char *)&message, sizeof(yaxp::Header) ) ) {
        LOG_INFO_CAT(LogCategoryMask,LogCategory) << "doRecv header received error";
        return false;
    }
    // TODO
    // if( ( message.length == 0 ) || ( message.messageType < yacynth::yaxp::MessageT::validLength ) ) {
    if( message.messageType < yaxp::MessageT::validLength ) {
        message.print(str);
        LOG_INFO_CAT(LogCategoryMask,LogCategory) << "doRecv " << str;
        return true;
    }
    if( ! recvAll( (char *)message.data, message.length ) ) {
        LOG_INFO_CAT(LogCategoryMask,LogCategory) << "doRecv body received error";
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
            LOG_INFO_CAT(LogCategoryMask,LogCategory) << "recvAll disconnect";
            errnoNet = ENOTCONN;
            connected = false;
            shut();
            return false;
        }
        if( res < 0 ) {
            if( EAGAIN == errno ) {
                LOG_INFO_CAT(LogCategoryMask,LogCategory) << "recvAll EAGAIN";
                continue;
            }
            errnoNet = errno;
            LOG_INFO_CAT(LogCategoryMask,LogCategory) << "recvAll failed " << errnoNet;
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
        LOG_INFO_CAT(LogCategoryMask,LogCategory) << "doSend failed " << errnoNet;
        return false;
    }
     // this should never happen
    LOG_INFO_CAT(LogCategoryMask,LogCategory) << "doSend sent more then requested " << res;
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
        LOG_CRIT_CAT(LogCategoryMask,LogCategory) << "urandom read failed " << errno;
        close( rf );
        return false;
    }
    LOG_CRIT_CAT(LogCategoryMask,LogCategory) << "urandom open failed " << errno;
    return false;
}

// if error then init error of filesystem error - practically never happens
bool Server::setAuthSeed( const std::string& seedFileName )
{
    // TODO for local - there is a default key
    // const char * defaultAuth = localNet ? "LOCAL-AUTH-DEFAULT-KEY";
    std::ifstream seedFile( seedFileName );
    if( ! seedFile.is_open() ) {
        LOG_CRIT_CAT(LogCategoryMask,LogCategory) << "setAuthSeed open failed " << errnoNet;
        return false;
    }
    seedFile.read( seedAuth, sizeof(seedAuth));
    if( seedFile.fail() ) {
        LOG_INFO_CAT(LogCategoryMask,LogCategory) << "doAccept read failed " << errnoNet;
        return false;
    }
    seedFile.close();
    return true;
}

bool Server::checkAuth( const uint8_t * randBuff, const uint8_t * respBuff, size_t respLng )
{
    uint8_t resBlock[ authLength ];
    if( authLength > respLng ) {
        return false;
    }
    blake2b( resBlock, sizeof(resBlock),
             randBuff, sizeof(resBlock),
             seedAuth, sizeof(seedAuth));
    return 0 == memcmp( resBlock, respBuff, authLength);
}

// --------------------------------------------------------------------
RemoteServer::RemoteServer( Sysman& sysman, const uint16_t port, const std::string& authKeyFile )
:   Server(sysman, authKeyFile )
{
    portControlRemote = port;   // on the server side
    portStatusRemote  = port+1;   // on the client side
    // put this in to function
    int reuse = 1;
    memset( &statusSockAddr6, '0', sizeof(statusSockAddr6) );
    sockaddr_in serv_addr;
    socketListen = socket( AF_INET, SOCK_STREAM, 0 );
    memset( &serv_addr, 0, sizeof(serv_addr) );
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(portControlRemote);

    if( setsockopt(socketListen, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0) {
        errnoNet = errno;
        createFailed = true;
        LOG_CRIT_CAT(LogCategoryMask,LogCategory) << "setsockopt failed " << errnoNet;
    }

    // TODO check return status
    auto bres = bind( socketListen, (sockaddr*)&serv_addr, sizeof(serv_addr) );
    if( bres < 0 ) {
        errnoNet = errno;
        createFailed = true;
        LOG_CRIT_CAT(LogCategoryMask,LogCategory) << "bind failed " << errnoNet;
    }
    //create a UDP socket
    socketSendStatus = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if( socketSendStatus < 1 ) {
        errnoNet = errno;
        createFailed = true;
        LOG_CRIT_CAT(LogCategoryMask,LogCategory) << "socket failed " << errnoNet;
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
        LOG_CRIT_CAT(LogCategoryMask,LogCategory) << "sendStatus failed " << errnoNet;
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
        LOG_CRIT_CAT(LogCategoryMask,LogCategory) << "doAccept failed " << errnoNet;
        return false;
    }

    switch( remoteSock.sin_family ) {
    case AF_INET:
        statusSockAddr4.sin_family = AF_INET;
        statusSockAddr4.sin_addr = remoteSock.sin_addr;
        statusSockAddr4.sin_port = htons(portStatusRemote);
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
:   Server(sysman, authKeyFile )
{
    portControlLocal    = port;
    portStatusLocal     = port;
    portControlLocal    += yaxp::localPortControlSuffix;
    portStatusLocal     += yaxp::localPortStatusSuffix;

    // put this to a function
    sockaddr_un serv_addr;
    socketListen = socket( PF_LOCAL, SOCK_STREAM, 0 );
    unlink(portControlLocal.data());
    // TODO '0' or 0
    memset( &serv_addr, '0', sizeof(serv_addr) );
    serv_addr.sun_family = AF_LOCAL;
    strncpy( serv_addr.sun_path, portControlLocal.data(), sizeof(serv_addr.sun_path) );

//    if( setsockopt( socketListen, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0) {
//        errnoNet = errno;
//    }

    // TODO check return addr
    auto bres = bind( socketListen, (sockaddr*)&serv_addr, sizeof(serv_addr) );
    if( bres < 0 ) {
        errnoNet = errno;
        createFailed = true;
        LOG_CRIT_CAT(LogCategoryMask,LogCategory) << "bind failed " << errnoNet;
    }
    socketSendStatus = socket( PF_LOCAL, SOCK_DGRAM, 0 );
    if( socketSendStatus < 1 ) {
        errnoNet = errno;
        createFailed = true;
        LOG_CRIT_CAT(LogCategoryMask,LogCategory) << "socket failed " << errnoNet;
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
        LOG_INFO_CAT(LogCategoryMask,LogCategory) << "sendStatus failed " << errnoNet;
        return false;
    }
    return true;
}

bool LocalServer::doAccept()
{
    socketAccept = accept( socketListen, (sockaddr*)NULL, NULL );
    if( socketAccept < 1 ) {
        errnoNet = errno;
        LOG_INFO_CAT(LogCategoryMask,LogCategory) << "doAccept failed " << errnoNet;
        return false;
    }
    return true;
}
// --------------------------------------------------------------------

} // end namespace yacnet
} // end namespace yacynth



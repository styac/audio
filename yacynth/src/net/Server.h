#pragma once

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
 * File:   Server.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on February 19, 2016, 11:55 PM
 */

#include    "protocol.h"
#include    "control/Sysman.h"
#include    "control/Setting.h"
#include    "spdlog/spdlog.h"
#include    <iostream>
#include    <memory>
#include    <netinet/in.h>
#include    <arpa/inet.h>
#include    <sys/un.h>

namespace yacynth {
namespace net {

class Server {
public:
    NON_COPYABLE_NOR_MOVABLE(Server);
    Server() = delete;
    static constexpr size_t randLength = yaxp::seedLength;
    static constexpr size_t authLength = yaxp::seedLength;
    static constexpr size_t maxStatusSize = 512;

    typedef std::shared_ptr<Server> Type;

    virtual ~Server() = default;

    static Server::Type create( Sysman& sysmanP, const Setting& setting );

    bool run( void );

    virtual bool sendStatus( const yacynth::yaxp::Message& message ) = 0;

protected:
    virtual bool doAccept() = 0;
    bool doRecv();
    bool doSend();
    bool doPeek();
    bool doListen();
    bool authenticate();
    void execute();
    void shut();
    bool recvAll( char *p, uint32_t size );
    bool fillRandom( uint8_t * randBuff );
    bool checkAuth( const uint8_t * randBuff, const uint8_t * respBuff, size_t respLng );

    Server( Sysman& sysmanP, const std::string& authKeyFile );

    bool setAuthSeed( const std::string& seedFileName );

    Sysman&                 sysman;
    std::shared_ptr<spdlog::logger> logger;
    int                     socketListen;
    int                     socketAccept;
    int                     socketSendStatus;
    int                     errnoNet;
    char                    seedAuth[ yaxp::seedLength ];
    yaxp::Message           message;
    uint16_t                cliendId;
    uint8_t                 lastSequenceNumber;
    yaxp::MessageT          lastMessageType;
    bool                    connected;
    bool                    authenticated;
    bool                    stopServer;
};

class RemoteServer : public Server {
public:
    NON_COPYABLE_NOR_MOVABLE(RemoteServer);
    RemoteServer() = delete;
    RemoteServer( Sysman& sysmanP, const uint16_t port, const std::string& authKeyFile );
    ~RemoteServer();

    bool sendStatus( const yacynth::yaxp::Message& message ) override;

private:
    bool doAccept()  override;
    union {
        sockaddr_in     statusSockAddr4;    // UDP for sending status updates
        sockaddr_in6    statusSockAddr6;    // not implemented
    };

    uint16_t        portControl;    // can be removed
    uint16_t        portStatus;     // UDP for sending status updates
};

class LocalServer : public Server {
public:
    NON_COPYABLE_NOR_MOVABLE(LocalServer);
    LocalServer() = delete;
    LocalServer( Sysman& sysmanP, const char *port, const std::string& authKeyFile );
    ~LocalServer();

    bool sendStatus( const yacynth::yaxp::Message& message ) override;

private:
    bool doAccept()  override;
    sockaddr_un     statusSockAddr;
    std::string     portControl;
    std::string     portStatus;
};

} // end namespace yacnet
} // end namespace yacynth


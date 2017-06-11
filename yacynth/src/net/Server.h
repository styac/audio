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
#include    "../../include/protocol.h"
#include    "../control/Sysman.h"
#include    <iostream>
#include    "spdlog/spdlog.h"

namespace yacynth {

class   Server {
public:
    static constexpr size_t seedLength = 32;
    static constexpr size_t randLength = 32;
    static constexpr size_t authLength = 32;

    explicit Server(
        Sysman&             sysmanP,
        const uint16_t      port
    );

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;
    ~Server();

    // set the seed block
    void setAuthSeed( const uint8_t * src, size_t lng );
    bool run( void );

private:
    bool doRecv();
    bool doSend();
    bool doPeek();
    bool doListen();
    bool doAccept();
    bool authenticate();
    void execute();
    void shut();
    bool recvAll( char *p, uint32_t size );
    bool fillRandom( uint8_t * randBuff );
    bool checkAuth( const uint8_t * randBuff, const uint8_t * respBuff, size_t respLng );

    Sysman&                 sysman;
    std::shared_ptr<spdlog::logger> logger;
    int                     socketListen;
    int                     socketAccept;
    int                     errnoNet;
    uint8_t                 seedAuth[ seedLength ];
    yaxp::Message           message;
    uint16_t                cliendId;
    uint8_t                 lastSequenceNumber;
    bool                    connected;
    bool                    authenticated;
    bool                    stopServer;
};

} // end namespace yacynth


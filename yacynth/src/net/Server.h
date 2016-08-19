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
#include    <boost/array.hpp>
#include    <boost/asio.hpp>
#include    <boost/bind.hpp>

using namespace boost::asio;

namespace yacynth {

class   Server {
public:
    explicit Server(
        const std::string&  address, 
        const uint16_t      port,
        Sysman&             sysmanP );
    
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    bool run( void );
    
private:
    void doAwaitStop(void);
    io_service              ioservice;
    ip::tcp::endpoint       endpoint;
    ip::tcp::acceptor       acceptor;
    signal_set              signals;
    ip::tcp::socket         socketAccept;
    Sysman&                 sysman;
};

} // end namespace yacynth


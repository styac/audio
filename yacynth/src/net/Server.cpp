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

namespace yacynth {

// --------------------------------------------------------------------

Server::Server( const std::string& addressP, const uint16_t portP, Sysman&  sysmanP )
:   sysman(sysmanP)
,   ioservice()
,   endpoint( ip::address_v4::from_string( addressP ), portP )
//,   endpoint( ip::tcp::v4(), portP )
,   acceptor( ioservice, endpoint, false ) // reuse address
,   socketAccept( ioservice )
,   signals(ioservice)
{
    signals.add(SIGINT);
    signals.add(SIGTERM);
    signals.add(SIGQUIT);
    try {
        signals.async_wait( std::bind( &Server::doAwaitStop, this) );
    } catch ( ... ) {
        acceptor.close();
    }
            
} // end Server::Server

// --------------------------------------------------------------------

bool Server::run(void)
{    
    acceptor.accept(socketAccept); 
    std::cout << " connected" << std::endl;
    return sysman.eval( socketAccept );
} // end Server::run

// --------------------------------------------------------------------

void Server::doAwaitStop(void)
{
    acceptor.close();
    std::cout << " signal " << std::endl;
    exit(0);
} // end Server::doAwaitStop


// --------------------------------------------------------------------

} // end namespace yacynth



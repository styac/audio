/*
 * Copyright (C) 2017 ist
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
 * File:   Router.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on October 4, 2017, 7:06 PM
 */
#include    "Router.h"

// next generation 

namespace yacynth {

using namespace TagRouterLevel_01;

// --------------------------------------------------------------------

Router::Router( ControlQueueVector& inQueue )
:   queueIn(inQueue)
{
} // end Router::Router

// --------------------------------------------------------------------

void Router::midiInCB( void *data, RouteIn in )
{
    static_cast<Router *>(data)->translate( in );
}
// --------------------------------------------------------------------

bool Router::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    const uint8_t tag = message.getTag(tagIndex);
    switch( TagRouter( tag ) ) {
    case TagRouter::Clear :
        return true;
    case TagRouter::SetToneBank :
        return true;        
    }

    message.setStatus( yaxp::MessageT::illegalTag );
    return false;
}


} // end namespace yacynth 

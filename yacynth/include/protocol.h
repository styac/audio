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
 * File:   protocol.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on February 20, 2016, 10:15 AM
 */
#include    <cstdint>
#include    <string>

namespace yacynth {

namespace   Yax {

    enum    CMD : char {
        CMD_TARGET  =   '@',
    //  client to server set default prefix --> shorthand
    //  = /yax/something
        CMD_PREFIX  =   '=',
    //  client to server set a value
    //  > /component value
        CMD_SET     =   '>',
    //  client to server get a value
    //  < /component
        CMD_GET     =   '<',
    //  server to client response positive
    //  + /component value
        CMD_POSITIV =   '+',
    //  server to client response negative
    //  - /component "error string" or "error code"
        CMD_NEGATIV =   '-',
    // server to client that something has changed
        CMD_ADVISE  =   '^',
    // value at position N -- for vectors 
    // @ N data
        CMD_AT  =   '@',
    };
    
    const uint32_t      maxLen = 1<<16;    
    const std::string   msgGet          ("get ");
    const std::string   msgSet          ("set ");
    const std::string   msgAsciiBegin   ("{A00 ");
    const std::string   msgBinBegin     ("{B00 ");
    const std::string   msgEnd          ("\n}\n\0");
    const std::string   helloClient     ("{A00 /hello-c \n}\n\0");
    const std::string   helloServer     ("{A00 /hello-s \n}\n\0");
    const std::string   byeClient       ("{A00 /bye-c \n}\n\0");
    const std::string   byeServer       ("{A00 /bye-s \n}\n\0");
};


} // end namespace yacynth

#if 0

{00A put 
@ /spectrum/matrix
/vector 0
/count 256
= /tone/pitch/vector
v1
v2
...
v256
= /tone/amplitude
a1
a2
...
a256
}

#endif
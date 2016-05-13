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
 * File:   YaIoJackPort.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on January 31, 2016, 1:14 PM
 */
#ifndef     YAIO_JACK_PORT
#define     YAIO_JACK_PORT

#include    "YaIo.h"

#include    <jack/jack.h>
#include    <jack/midiport.h>
#include    <jack/ringbuffer.h>

#include    <cstddef>
#include    <cstdint>
#include    <cstdio>
#include    <iostream>
#include    <cassert>
#include    <iterator>
#include    <string>

namespace yacynth {

struct YaIoJackPort {
    YaIoJackPort( std::string name, std::string type, uint64_t flg )
    :    jackPort(0)
    ,    portName(name)
    ,    portType(type)
    ,    flags(flg)
    {};

    bool reg(       jack_client_t *client );
    bool unreg(     jack_client_t *client );
    void *getBuffer( jack_nframes_t nframes ) {
        return jack_port_get_buffer( jackPort, nframes );
    };

    jack_port_t         *jackPort;
    std::string         portName;
    std::string         portType;
    std::uint64_t       flags;
};

} // end namespace yacynth

#endif /* YAIO_JACK_PORT */


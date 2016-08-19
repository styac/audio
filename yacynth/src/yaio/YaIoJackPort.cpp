/*
 * Copyright (C) 2016 Istvan Simon
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
 * File:   YaIoJackPort.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on January 31, 2016, 1:14 PM
 */

#include    "YaIoJackPort.h"

namespace yacynth {

bool YaIoJackPort::reg( jack_client_t *client )
{
    jackPort = jack_port_register( client, portName.c_str(), portType.c_str(), flags, 0 );
    if( nullptr == jackPort ) {

#ifdef     YAC_JACK_DEBUG
        std::cout
            << "port register failed: " << portName
            << std::endl;
#endif
        return false;
    }
    return true;
} // end YaIoJackPort::reg(void)

bool YaIoJackPort::unreg( jack_client_t *client )
{
    return 0 == jack_port_unregister( client, jackPort );
} // end YaIoJackPort::unreg(void)

} // end namespace yacynth
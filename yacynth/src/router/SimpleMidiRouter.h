#pragma once

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
 * File:   SimpleMidiRouter.h
 * Author: Istvan Simon
 *
 * Created on February 27, 2016, 10:15 PM
 */
#include    "AbstractRouter.h"

namespace yacynth {

class SimpleMidiRouter : public AbstractRouter {
public:
    SimpleMidiRouter()
    {};
    virtual ~SimpleMidiRouter() = default;

    void clear(void) {};

    virtual Yamsgrt     translate( const RouteIn& in ) override;
    virtual uint32_t    getPitch( int32_t noteNr, uint16_t tableNr = 0 );
    virtual void        setTransposition( int8_t val ) override;
};

} // end namespace yacynth


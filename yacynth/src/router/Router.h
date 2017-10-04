#pragma once
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
 * File:   Router.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on October 4, 2017, 7:07 PM
 */

#include    "yacynth_globals.h"
#include    "../message/yamsg.h"
#include    "../message/Midi.h"
#include    "../control/Controllers.h"
#include    "../router/ControlQueue.h"

namespace yacynth {

class Router {
public:
        
    Router() = delete;
    Router( ControlQueueVector& inQueue );

    ~Router() = default;

    void clear(void);
    
    static void midiInCB( void *, RouteIn in );
   
    inline MidiController&  getMidiController(void) { return midiController; }

    bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex ); 
    
protected:
    void inline translate( RouteIn in )
    {
        Yamsgrt out;
    
    }

    ControlQueueVector& queueIn;
    MidiController      midiController;
};


} // end namespace yacynth 



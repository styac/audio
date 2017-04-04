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
 * File:   YaIoJack.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on January 31, 2016, 12:26 PM
 */

#ifndef     YAIO_JACK
#define     YAIO_JACK

#include    "YaIo.h"
#include    "YaIoJackPort.h"

#include    <jack/jack.h>
#include    <jack/midiport.h>
#include    <jack/ringbuffer.h>

namespace yacynth {

class   YaIoJack : public YaIo {
public:
    virtual ~YaIoJack();
    bool initialize(    void );
    void shutdown(      void );
    bool run(           void );
    void mute(   void ) { muted = true; };
    void unmute( void ) { muted = false; };
    
    // process
    // static JackProcessCallback             processCB;
    // void *arg == this
    static int processCB( jack_nframes_t nframes, void *arg ); 

// ----------- ?????
    
    // not really needed yet
    static JackThreadInitCallback          threadInitCB;
    // jackd was shut down
    static JackShutdownCallback            shutdownCB;
    // change buffer size - not allowed
    static JackBufferSizeCallback          bufsizeCB;
    // change sample rate  - not allowed
    static JackSampleRateCallback          samplerateCB;

    static inline YaIoJack&   getInstance(void) 
    {
        static YaIoJack    jack;
        return jack;
    }
    
protected:
    bool            muted;        
    jack_client_t   *client;
    jack_options_t  jackOptions;
    jack_status_t   jackStatus;
    YaIoJackPort    midiInPort;
    YaIoJackPort    audioOutPort1;
    YaIoJackPort    audioOutPort2;

private:
    YaIoJack();
    NON_COPYABLE_NOR_MOVABLE(YaIoJack)
};

} // end namespace yacynth
#endif /* YAIO_JACK */


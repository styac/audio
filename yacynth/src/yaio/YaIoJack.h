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
    
    // process
    // static JackProcessCallback             processCB;
    // void *arg == this
    static int processCB( jack_nframes_t nframes, void *arg ); 

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
        static YaIoJack inst;
        return inst;
    }
    
protected:
    // put this to audioIOProcessing
    inline void processJackMidiIn()
    {
        void * midiIn = midiInPort.getBuffer( nframes );
        jack_midi_event_t in_event;
        jack_nframes_t event_count = jack_midi_get_event_count( midiIn );
        for( auto i=0; i < event_count; ++i ) {
            jack_midi_event_get( &in_event, midiIn , i );
            printEvent( in_event.buffer, in_event.size, ( event_count-1 ) == i );
            midiOutProcessing( userData, in_event.buffer, in_event.size );
        }    
    }
    
    void printEvent( uint8_t *eventp, uint32_t eventSize, bool lastEvent)
    {
        std::cout << "ev: " << std::hex;
        for( auto i=0; i< eventSize; ++i, ++eventp ){
            const uint16_t val = *eventp;
            std::cout << " " <<  val ;
        }
        if( lastEvent ) {
            std::cout << " ****";
        }
        std::cout << std::endl;
    }
    
    jack_client_t   *client;
    jack_options_t  jackOptions;
    jack_status_t   jackStatus;
    YaIoJackPort    midiInPort;
    YaIoJackPort    audioOutPort1;
    YaIoJackPort    audioOutPort2;
    YaIoJackPort    audioInPort1;
    YaIoJackPort    audioInPort2;
    jack_nframes_t  nframes;

private:
    YaIoJack();
    NON_COPYABLE_NOR_MOVABLE(YaIoJack)
};

} // end namespace yacynth
#endif /* YAIO_JACK */


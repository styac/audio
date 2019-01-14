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

#include "YaIo.h"
#include "YaIoJackPort.h"
#include "message/Midi.h"

#include <jack/jack.h>
#include <jack/midiport.h>
#include <jack/ringbuffer.h>

namespace yacynth {

class   YaIoJack : public YaIo {
public:
    virtual ~YaIoJack();
    bool initialize(    void );
    void shutdown(      void );
    bool run(           void );

    static int  processAudioMidiCB( jack_nframes_t nframes, void * arg );
    static int  processAudioCB( jack_nframes_t nframes, void * arg );
    static void shutdownCB( void * arg );

    static inline YaIoJack&   getInstance()
    {
        static YaIoJack inst;
        return inst;
    }

    // TODO : global singleton to store cycleCount
    int64_t getCycleCount() const
    {
        return cycleCount;
    }

    int getRealTimePriority() const;

protected:
    inline void processJackMidiIn()
    {
        RouteIn  midi;
        void * midiIn = midiInPort.getBuffer( nframes );
        jack_midi_event_t in_event;
        jack_nframes_t event_count = jack_midi_get_event_count( midiIn );
        for( auto i=0u; i < event_count; ++i ) {
            jack_midi_event_get( &in_event, midiIn , i );
            printEvent( in_event.buffer, in_event.size, ( event_count-1 ) == i );
            midi.op                 = ( in_event.buffer[ 0 ] ) >> 4;
            midi.channel            = ( in_event.buffer[ 0 ] ) & 0x0F;
            if( 2 < in_event.size ) {
                midi.velocity_val   = (in_event.buffer[ 2 ]);
                midi.note_cc_val    = (in_event.buffer[ 1 ]);
            } else if ( 1 < in_event.size ) {
                midi.note_cc_val    = (in_event.buffer[ 1 ]);
            }
            midiOutProcessing( midiProcessorData, midi );
        }
    }

    void printEvent( uint8_t *eventp, uint32_t eventSize, bool lastEvent)
    {
        std::cout << "ev: " << std::hex;
        for( auto i=0u; i< eventSize; ++i, ++eventp ){
            const uint16_t val = *eventp;
            std::cout << " " <<  val ;
        }
        if( lastEvent ) {
            std::cout << " ****";
        }
        std::cout << std::endl;
    }

    jack_client_t   *client;
    YaIoJackPort    midiInPort;
    YaIoJackPort    audioOutPort1;
    YaIoJackPort    audioOutPort2;
    YaIoJackPort    audioInPort1;
    YaIoJackPort    audioInPort2;
    jack_options_t  jackOptions;
    jack_status_t   jackStatus;
    jack_nframes_t  nframes;

private:
    int64_t         cycleCount;
    YaIoJack();
    NON_COPYABLE_NOR_MOVABLE(YaIoJack)
};

} // end namespace yacynth

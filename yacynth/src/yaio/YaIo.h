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
 * File:   YaIo.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on January 31, 2016, 9:17 AM
 */

#ifndef     YAIO
#define     YAIO

#include    "yacynth_globals.h"
#include    "../message/yamsg.h"
#include    "../message/Midi.h"

#include    <cstddef>
#include    <cstdint>
#include    <vector>
#include    <cstdio>
#include    <iostream>
#include    <cassert>
#include    <iterator>
#include    <string>

#define YAC_JACK_DEBUG  1

namespace yacynth {

class   YaIo {
public:
    YaIo();
    
    NON_COPYABLE_NOR_MOVABLE(YaIo)

    // processor function types
    typedef void ( * MidiProcessorType )( void *data, RouteIn msg );
    typedef void ( * AudioOutProcessorType )( void *data, uint32_t nframes, float *outp1, float *outp2 );
    typedef void ( * AudioInOutProcessorType )( void *data, uint32_t nframes, float *outp1, float *outp2, float *inp1, float *inp2 );
    
    virtual ~YaIo() = default;
    virtual bool initialize( void ) = 0;
    virtual bool run(        void ) = 0;
    virtual void shutdown(   void ) = 0;

    void registerAudioProcessor( void* userData, 
        AudioOutProcessorType audioOutCB, AudioInOutProcessorType audioInOutCB );

    void registerMidiProcessor( void* userData, 
        MidiProcessorType midiInCB );

    void clearProcessCB();

    void                setMyName( const std::string& name ) 
        { nameClient = name; }
    const std::string   getMyName( void )       
        { return nameClient; }
    const std::string   getMyNameActual( void ) 
        { return nameClientReal; }
    const std::string   getErrorString( void )  
        { return errorString; }
    void mute( void ) 
        { mutedOutput = true; mutedInput = true; }
    void unmuteOutput( void ) 
        { mutedOutput = false; }
    void unmute( void )  
        { mutedInput = false; mutedOutput = false; }
    inline int16_t getBufferSizeRate()
        { return bufferSizeMult; }

protected:    
    static void noMidiProcessing( void *data, RouteIn msg );
    static void noAudioOutProcesing( void *data, uint32_t nframes, float *outp1, float *outp2 );
    static void noAudioInOutProcesing( void *data, uint32_t nframes, float *outp1, float *outp2, float *inp1, float *inp2 );
    
    uint32_t        sampleRateMin;
    uint32_t        sampleRateMax;
    uint16_t        bufferSizeMin;
    uint16_t        bufferSizeMax;
    uint16_t        bufferSizeJack;
    uint16_t        bufferSizeMult;     // how many internal buffer(64samples) fill a jack buffer(256)
    std::string     nameClient;
    std::string     nameClientReal;
    std::string     errorString;
    
    void                  * audioProcessorData;
    void                  * midiProcessorData;    
    MidiProcessorType       midiOutProcessing;
    AudioOutProcessorType   audioOutProcesing;
    AudioInOutProcessorType audioInOutProcesing;
    
    bool            mutedOutput;        
    bool            mutedInput;        
};

} // end namespace yacynth
#endif /* YAIO */

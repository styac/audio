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
 * File:   YaIo.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on January 31, 2016, 9:17 AM
 */

#include    "YaIo.h"

namespace yacynth {

YaIo::YaIo()
:   sampleRateMin(48000)
,   sampleRateMax(48000) // TODO 88200, 96000
,   bufferSizeMin(64)
,   bufferSizeMax(256)
,   midiOutProcessing(noMidiProcessing)
,   audioOutProcesing(noAudioOutProcesing)
,   audioInOutProcesing(noAudioInOutProcesing)
,   audioProcessorData(nullptr)
,   midiProcessorData(nullptr)
,   nameClient("yacsynth")
,   nameClientReal("yacsynth")
,   errorString("err: ")
,   mutedOutput(true)
,   mutedInput(true)
{}
    
void YaIo::registerAudioProcessor( void* userData, 
    AudioOutProcessorType audioOutCB, AudioInOutProcessorType audioInOutCB )
{
    audioProcessorData  = userData;
    audioOutProcesing   = audioOutCB;
    audioInOutProcesing = audioInOutCB;
}

void YaIo::registerMidiProcessor( void* userData, 
    MidiProcessorType midiInCB )
{
    midiProcessorData = userData;
    midiOutProcessing = midiInCB;
}

void YaIo::clearProcessCB()
{
    audioProcessorData  = nullptr;
    midiProcessorData   = nullptr;
    midiOutProcessing   = noMidiProcessing;
    audioOutProcesing   = noAudioOutProcesing;
    audioInOutProcesing = noAudioInOutProcesing;
}

// empty functions to fill unused callbacks
void YaIo::noMidiProcessing( void *data, RouteIn msg )
{}
void YaIo::noAudioOutProcesing( void *data, uint32_t nframes, float *outp1, float *outp2 )
{}
void YaIo::noAudioInOutProcesing( void *data, uint32_t nframes, float *outp1, float *outp2, float *inp1, float *inp2 )
{}
    

} // end namespace yacynth

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
 * File:   IOthread.h
 * Author: Istvan Simon
 *
 * Created on February 15, 2016, 4:59 PM
 */

#include    "YaIoInQueue.h"
#include    "../oscillator/OscillatorOutput.h"
#include    "../message/Midi.h"
#include    "../control/Controllers.h"
#include    "../router/SimpleMidiRouter.h"
#include    "../effects/Panmix.h"
#include    "../effects/Mixer.h"

#include    <cstddef>
#include    <cstdint>
#include    <vector>
#include    <cstdio>
#include    <iostream>
#include    <cassert>
#include    <iterator>
#include    <string>

namespace yacynth {

// actually this is runnnig in the jack callback

class IOThread {
public:
    static constexpr  uint16_t  mixerChannelCount    = 2;
    IOThread()  = delete;
    IOThread(   YaIoInQueueVector&      in,
                OscillatorOutVector&    out,
                AbstractRouter&         router );

    static void midiInCB(   void *data, uint8_t *eventp, uint32_t eventSize, bool lastEvent );
    static void audioOutCB( void *data, uint32_t nframes, float *outp1, float *outp2, int16_t bufferSizeMult );

private:
    void    printEvent( uint8_t *eventp, uint32_t eventSize, bool lastEvent);
    YaIoInQueueVector&              queueIn;
    OscillatorOutVector&            queueOut; 
    AbstractRouter&                 midiRouter;
    Panmix                          panMixer;    
    EndMixer<mixerChannelCount>     endMixer;
    
    // profiling
    uint64_t    timer;
    int64_t     count;    
};

} // end namespace yacynth




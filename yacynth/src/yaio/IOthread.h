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

#include "router/ControlQueue.h"
#include "oscillator/OscillatorOutput.h"
#include "message/Midi.h"
#include "control/Controllers.h"
#include "utils/GaloisNoiser.h"
#include "effects/FxMixer.h"
#include "effects/FxOscillatorMixer.h"
#include "effects/FxInput.h"
#include "control/Timer.h"

#include <cstddef>
#include <cstdint>
#include <vector>
#include <cstdio>
#include <iostream>
#include <cassert>
#include <iterator>
#include <string>

#define IO_DEBUG 

using namespace noiser;

namespace yacynth {

class IOThread {
public:
    IOThread()  = delete;
    IOThread( OscillatorOutVector & out );
    static void audioOutCB(     void *data, uint32_t nframes, float *outp1, float *outp2 );
    static void audioInOutCB(   void *data, uint32_t nframes, float *outp1, float *outp2, float *inp1, float *inp2 );
    inline FxRunner&        getFxRunner()
        { return fxRunner; }
    inline void setBufferSizeRate( int16_t val )
        { bufferSizeRate = val; }
    inline void clearFxInput()
        { toClearFxInput = true; }

    int8_t getCoreNr() const
    {
        return coreNr;
    }
    
private:    
    inline void setCore() 
    {
        if( coreNr < 0 ) {
            coreNr = sched_getcpu();
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(coreNr, &cpuset);
            pthread_setaffinity_np( pthread_self(), sizeof(cpu_set_t), &cpuset );            
        }
    }
    
    OscillatorOutVector   & queueOut;
    FxMixer                 fxEndMixer;
    FxOscillatorMixer       fxOscillatorMixer;
    FxInput                 fxInput;
    FxRunner                fxRunner;
    uint16_t                bufferSizeRate; // get it from IO interface external / internal frame size
    struct {
        uint16_t            toClearFxInput : 1;
    };
    
#ifdef IO_DEBUG
    NanosecCollector        nanosecCollector;
#endif
    // to reset the noise generators
    uint32_t                cycleNoise;
    int8_t                  coreNr;
};

} // end namespace yacynth




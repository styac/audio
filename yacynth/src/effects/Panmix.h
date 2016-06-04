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
 * File:   Panmix.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on February 18, 2016, 6:08 PM
 */

#include    "yacynth_globals.h"
#include    "v4.h"
#include    "Filter.h"
#include    "Ebuffer.h"
//#include    "Echo.h"
//#include    "Comb.h"
//#include    "CombInterpolated.h"
//#include    "FilterBank.h"
#include    "EffectBase.h"

#include    "../oscillator/Tables.h"
#include    "../oscillator/NoiseFrame.h"
#include    "../oscillator/NoiseSample.h"

#include    "../oscillator/OscillatorOutput.h"
#include    "../control/Controllers.h"
#include    "../oscillator/Lfo.h"
#include    "../utils/GaloisNoiser.h"
#include    "../utils/FilterComb.h"
#include    "../utils/FilterAllpass.h"
//#include    "../utils/BurningWood.h"

#include    <cstdint>
#include    <iostream>
#include    <tgmath.h>

using namespace limiter;
using namespace filter;

namespace yacynth {

class Panmix {
public:
    struct alignas(16) AddVector {
        union {
            v2di    i[oscillatorOutSampleCount/2];
            int64_t v[oscillatorOutSampleCount];
        };
    };
    Panmix();

    inline const EIObuffer& get(void) const { return out; };
    void summOscillatorOut( OscillatorOut& inp );
    void process( OscillatorOut& inp );

private:
    inline void amplitudeFilter( const uint64_t v )
    {
        constexpr   uint8_t scale = 5;
        amplitude = ((amplitude * 27)>>scale) + (v>>scale);
    }
    EIObuffer           inFilter;
    EIObuffer           out;           // final out
    NoiseFrame< FrameInt< EbufferPar::EffectFrameSizeExp > >       
                        noiseFrame;
    NoiseFrame< FrameFloat< EbufferPar::EffectFrameSizeExp > >       
                        noiseFrameF;
    NoiseFrame< EIObuffer >       
                        noiseFrameE;
    NoiseSample         noiseSample;
    ControlledFilter    controlledFilter;
    uint64_t            amplitude;
    FilterCombInt64<10>       filterComb1;
    FilterCombInt64<10>       filterComb2;
    FilterCombInt64<10>       filterComb3;
    FilterCombInt64<10>       filterComb4;
//    FilterComb<5>       filterComb1;
//    FilterComb<5>       filterComb2;
//    FilterComb<5>       filterComb3;
//    FilterComb<5>       filterComb4;
    FilterAllpass1<1>     allpass1;
    FilterAllpass1<1>     allpass2;

    float               allpassk[2];
    int                 allpasskind;
    int                 allpasskindcng;

    int                 noiselength;
    int                 noiseinterleave;

    //NoiseFilterInt<EbufferPar::EffectFrameSizeExp>      noiseFilter;
    OscillatorNoise      oscillatorNoise;
//    OscillatorNoise      oscillatorNoise1;
    OscillatorNoiseFloat oscillatorNoiseFloat;
    bool                enableEffectFilter;
    // profiling
    uint64_t    timer;
    uint64_t    timer1;
    int64_t     count;
    
};

} // end namespace yacynth


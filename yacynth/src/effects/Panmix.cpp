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
 * File:   Panmix.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on February 18, 2016, 6:24 PM
 */

#include    "Panmix.h"
#include    "../utils/Limiters.h"


//
// TODO: collect the osc output channels to diff multitimbre channels
// need a map to describe which channels to which part
//
//
namespace yacynth {
// --------------------------------------------------------------------
Panmix::Panmix()
:   enableEffectFilter(false)
,   controlledFilter( )
//,   echo(13)        // length = 1 << (13+6)  !!!!
//,   comb(10,300,-0.9,0.9)
//,   combInterpolated(12, 137, 0.5, 0.8, 7 )
//,   lfo( freq2deltaPhase( 300.0 ) )
//,   testampl(1<<24)
{};


// --------------------------------------------------------------------
void Panmix::process( OscillatorOut& inp )
{
    // put the amplitude value to the global controller
    amplitudeFilter( inp.amplitudeSumm );
    ControllerMatrix::getInstance().setFloat( ControllerMatrix::C_FLOAT_AMPLITUDE, static_cast<float>( amplitude>>40 ));

//    enableEffectFilter = true;
    summOscillatorOut( inp );
    if( enableEffectFilter ) {
        controlledFilter.process( inFilter, out );
    }
} // end  Panmix::process
// --------------------------------------------------------------------
void Panmix::summOscillatorOut( OscillatorOut& inp ) {
    constexpr   float   gainref = 1.0f/(1L<<32);
    AddVector  addbuff;
    float * outApf;
    float * outBpf;
    if( enableEffectFilter ) {
        outApf  = inFilter.channelA;
        outBpf  = inFilter.channelB;
        inFilter.channelGain = gainref;
    } else {
        outApf  = out.channelA;
        outBpf  = out.channelB;
        out.channelGain = gainref;
    }


    for( uint16_t si = 0u; si < oscillatorOutSampleCount; ++si  ) {
        addbuff.v[si] = inp.layer[0][si] + inp.layer[1][si];
    }

    for( uint16_t ovi = 2u; ovi < inp.overtoneCount; ++ovi ) {
        for( uint16_t si = 0u; si < oscillatorOutSampleCount; ++si ) {
            addbuff.v[si] += inp.layer[ovi][si];
        }
    }
    for( uint16_t si = 0u; si < oscillatorOutSampleCount; ++si ) {
        *outBpf++ = *outApf++ = static_cast<float>( addbuff.v[si] );
    }
};

// --------------------------------------------------------------------

} // end namespace yacynth



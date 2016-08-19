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
 * File:   FxOscillatorMixer.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on June 23, 2016, 9:41 PM
 */

#include    "yacynth_globals.h"
#include    "v4.h"
#include    "FxBase.h"
#include    "../oscillator/OscillatorOutput.h"


namespace yacynth {

class FxOscillatorMixerParam {
public:
    FxOscillatorMixerParam()
    : gain(gainref)
    {};
    // mandatory fields
    static constexpr char const * const name = "OscillatorMixer";
    static constexpr std::size_t maxMode     = 1; // 0 is always exist> 0,1,2
    static constexpr std::size_t inputCount  = 0; // input is different

    // temp
    static constexpr   float   gainref = 1.0f/(1L<<23);

    // optional fields
    float gain;
    
    // index to put the amplitudeSumm 
    // interface to controller
    float amplitudeSumm;
};

class FxOscillatorMixer : public Fx<FxOscillatorMixerParam>  {
public:
    using MyType = FxOscillatorMixer;
    FxOscillatorMixer()
    :   Fx<FxOscillatorMixerParam>()
    {
//        fillSprocessv<0>(sprocess_00);
//        fillSprocessv<1>(sprocess_01);
//        fillSprocessv<2>(sprocess_02);
//        fillSprocessv<3>(sprocess_03);
//        fillSprocessv<4>(sprocess_04);
//        fillSprocessv<5>(sprocess_05);        
    
    }
    
    // real process 
    void process( const OscillatorOut& inp );
    virtual bool connect( const FxBase * v, uint16_t ind ) override
    {
        doConnect(v,ind);
    }; 
    
private:
    struct alignas(16) AddVector {
        union {
            v2di    i[oscillatorOutSampleCount/2];
            int64_t v[oscillatorOutSampleCount];
        };
    };

};

} // end namespace yacynth

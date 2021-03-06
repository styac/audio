#pragma once

/*
 * Copyright (C) 2017 Istvan Simon
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
 * File:   FxOscillatorMixerParam.h
 * Author: Istvan Simon
 *
 * Created on May 20, 2017, 8:14 PM
 */



#include "protocol.h"
#include "Tags.h"
#include "control/Controllers.h"


namespace yacynth {

using namespace TagEffectTypeLevel_02;
using namespace TagEffectFxOscillatorMixerModeLevel_03;

class FxOscillatorMixerParam {
public:
    FxOscillatorMixerParam()
    {};
    // mandatory fields
    static constexpr char const * const name      = "OscillatorMixer";
    static constexpr TagEffectType  type          = TagEffectType::FxOscillatorMixer;
    static constexpr std::size_t maxMode          = 1; //
    static constexpr std::size_t inputCount       = 0; // input is oscillator thread out
    static constexpr std::size_t slaveCount       = layerCount - 1; // 0-base signal 1-modulation
    static constexpr char const * const slavename = "^";
    static constexpr float gainref    = 1.0f/(1L<<24);
    static constexpr uint8_t subtype    = uint8_t(TagEffectFxOscillatorMixerMode::SetParametersMode01);

    bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    void clear()
    {
        for( auto &p : gainZero ) {
            p = 0;
        }        
        gain[ 0 ] = gain[ 1 ] = gainref;
    }
    
    bool check()
    {
        return true;
    }
    
    union {
        float       gain[ oscOutputChannelCount ] = { gainref, gainref }; // param change must clear slave.out()
        uint64_t    gainZero[ oscOutputChannelCount / 2 ];
    };
};


}; // end namespace yacynth
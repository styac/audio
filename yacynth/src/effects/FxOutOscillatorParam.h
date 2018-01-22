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
 * File:   FxOutOscillatorParam.h
 * Author: Istvan Simon
 *
 * Created on May 20, 2017, 8:14 PM
 */

#include    "protocol.h"
#include    "Tags.h"
#include    "control/Controllers.h"


namespace yacynth {

using namespace TagEffectTypeLevel_02;
using namespace TagEffectFxOutOscillatorModeLevel_03;

class FxOutOscillatorParam {
public:
    // mandatory fields
    static constexpr char const * const name    = "SimpleOscillator"; // 1 main + 3 slave
    static constexpr TagEffectType  type        = TagEffectType::FxOutOscillator;
    static constexpr std::size_t maxMode        = 13; // 0 is always exist> 0,1,2
    static constexpr std::size_t inputCount     = 0;
    static constexpr std::size_t slaveCount     = 3; // 0-base signal 1-modulation
    static constexpr char const * const slavename = "^";

    bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    struct Mode01 {
        static constexpr uint8_t subtype         = uint8_t(TagEffectFxOutOscillatorMode::SetParametersMode01);

        // master
        ControllerMapLinear<1>  freqMapper;         // controller value to freq (delta phase)
        ControllerIndex         indexPhaseDelta;    // controller index of freq of channel 0
        // master + n slaves
        ControllerIndex         indexPhaseFreqDiff[slaveCount+1]; // controller index of phase or freq diff of channel 1

    } mode01;

    // need a direct steady control for multiphase source if there will be
};

}; // end namespace yacynth
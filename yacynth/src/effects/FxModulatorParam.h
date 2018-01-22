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
 * File:   FxModulatorParam.h
 * Author: Istvan Simon
 *
 * Created on May 20, 2017, 8:13 PM
 */



#include    "protocol.h"
#include    "Tags.h"
#include    "control/Controllers.h"


namespace yacynth {

using namespace TagEffectTypeLevel_02;
using namespace TagEffectFxModulatorModeLevel_03;

class FxModulatorParam {
public:
    // mandatory fields
    static constexpr char const * const name    = "Modulator";
    static constexpr TagEffectType  type        = TagEffectType::FxModulator;
    static constexpr std::size_t maxMode        = 6; // 0 is always bypass
    static constexpr std::size_t inputCount     = 2; // 0 : base signal; 1 : modulation
    static constexpr char const * const modeName[maxMode+1] =
    {   "copy"
    ,   "processModulation"
    ,   "processRing"
    ,   "processModulationMix"
    ,   "processRingVolColtrol"
    };
    static constexpr uint8_t subtype         = uint8_t(TagEffectFxModulatorMode::SetParametersMode01);
    bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );
    ControllerIndex inMultIndex;
    ControllerIndex mixMultIndex;
};


}; // end namespace yacynth
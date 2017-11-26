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
 * File:   FxOutNoiseParam.h
 * Author: Istvan Simon
 *
 * Created on May 20, 2017, 8:14 PM
 */

#include    "protocol.h"
#include    "Tags.h"
#include    "../control/Controllers.h"

namespace yacynth {
using namespace TagEffectTypeLevel_02;
using namespace TagEffectFxOutNoiseModeLevel_03;

class FxOutNoiseParam {
public:
    // mandatory fields
    static constexpr char const * const name    = "NoiseSource";
    static constexpr TagEffectType  type        = TagEffectType::FxOutNoise;
    static constexpr std::size_t maxMode        = 11; // 0 is always exist> 0,1,2
    static constexpr std::size_t inputCount     = 0;

    bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    static constexpr uint8_t subtype = uint8_t(TagEffectFxOutNoiseMode::SetParametersMode01);
    static constexpr float inigains[ maxMode ] = {
        1.0 / (1<<28),  // 1 - mode 1
        1.0 / (1<<26),  // 2
        1.0 / (1<<26),  // 3
        1.0 / (1<<26),  // 4
        1.0 / (1<<24),  // 5
        1.0 / (1<<21),  // 6
        1.0 / (1<<27),  // 7
        1.0 / (1<<26),  // 8
        1.0 / (1<<21),  // 9
        1.0 / (1<<24),  // 10
        1.0 / (1<<24),  // 10
    };

    void clear()
    {

        for( auto gi = 0u; gi < maxMode; ++gi )
            gains[ gi ] = inigains[ gi ];
    }
    
    float gains[ maxMode ] = {
        1.0 / (1<<28),  // 1 - mode 1
        1.0 / (1<<26),  // 2
        1.0 / (1<<26),  // 3
        1.0 / (1<<26),  // 4
        1.0 / (1<<24),  // 5
        1.0 / (1<<21),  // 6
        1.0 / (1<<27),  // 7
        1.0 / (1<<26),  // 8
        1.0 / (1<<21),  // 9
        1.0 / (1<<24),  // 10
        1.0 / (1<<24),  // 11
    };    // gain for each mode
    
    // optional fields
    uint8_t redPole;
    uint8_t purplePole;
    uint8_t blueZero;
    uint8_t rfu;
};

}; // end namespace yacynth
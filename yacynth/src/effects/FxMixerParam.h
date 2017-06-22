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
 * File:   FxMixerParam.h
 * Author: Istvan Simon
 *
 * Created on May 20, 2017, 8:13 PM
 */



#include    "protocol.h"
#include    "Tags.h"
#include    "../control/Controllers.h"


namespace yacynth {

using namespace TagEffectTypeLevel_02;
using namespace TagEffectFxMixerModeLevel_03;
// --------------------------------------------------------------------

class FxMixerParam {
public:
    static constexpr char const * const name    = "Mixer4";
    static constexpr TagEffectType  type        = TagEffectType::FxMixer;
    static constexpr std::size_t maxMode        = 4;
    static constexpr std::size_t inputCount     = 4;

    static constexpr uint8_t subtype         = uint8_t(TagEffectFxMixerMode::SetParametersMode01);

    inline void clear()
    {
        for( auto &p : gainIndex ) {
            p.setIndex( InnerController::CC_NULL );
        }
    }

    bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    ControllerIndex     gainIndex[inputCount];
     // range : n * -6 dB step
    float   gainRange[ inputCount ] = {0.5f, 0.5f, 0.5f, 0.5f };
};


}; // end namespace yacynth
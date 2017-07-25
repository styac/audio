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
 * File:   FxChorusParam.h
 * Author: Istvan Simon
 *
 * Created on May 20, 2017, 8:11 PM
 */

#include    "protocol.h"
#include    "Tags.h"
#include    "../control/Controllers.h"

namespace yacynth {

using namespace TagEffectTypeLevel_02;
using namespace TagEffectFxChorusModeLevel_03;

class FxChorusParam {
public:
    // mandatory fields
    static constexpr const char * const typeName    = "FxChorusParam";        
    static constexpr char const * const name        = "FxChorus";
    static constexpr TagEffectType  type            = TagEffectType::FxChorus;
    static constexpr std::size_t maxMode            = 2;
    static constexpr std::size_t inputCount         = 1;

    static constexpr std::size_t tapSize            = 4;
    static constexpr std::size_t delayLngExp        = 7;
    static constexpr std::size_t delayLng           = 1<<(delayLngExp+effectFrameSizeExp);
    static constexpr std::size_t delayOffsMaxLng    = delayLng - 1;
    static constexpr std::size_t delayOffsMinLng    = effectFrameSize * 2;

    static constexpr uint64_t minBaseDelay          = 256LL << 32;  // ca 6 msec
    static constexpr uint64_t maxBaseDelay          = 4000LL << 32; // ca 80 msec

    bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    //
    // delay center value
    // mod freq controller
    // mod depth controller
    // osc mode > sin, rand
    // oscillator: base sine (16 bit) + red noise (24 bit)
    // osc << 16+n + noise << 7
    //
    struct Mode01 {
        static constexpr uint8_t subtype         = uint8_t(TagEffectFxChorusMode::SetParametersMode01);

        bool check()
        {
            static_assert( delayLng >= (maxBaseDelay>>32)+64, "illegal delay length" );

            if( baseDelay < minBaseDelay ) {
                baseDelay = minBaseDelay;
            }
            if( baseDelay > maxBaseDelay ) {
                baseDelay = maxBaseDelay;
            }
            if( baseDelay < (256LL<<32)) {
                return false;
            }

            if( wetgain < -1.0 ) {
                wetgain = -1.0;
            }

            if( wetgain > 1.0 ) {
                wetgain = 1.0;
            }
            wetgain /= tapCount;
            return true;
        }
        uint64_t        baseDelay;          // chorus tap delay
        float           wetgain;
        int32_t         sineRange;       // iterator multiplier -- rename depth - controller
        int32_t         noiseRange;       // iterator multiplier -- rename depth - controller
        // manual
        ControllerIndex depthIndex;         // to set the mod depth
        ControllerIndex deltaPhaseIndex;    // to set the freq
        ControllerIndex phaseDiffIndex;     // to set the phase diff A-B

        // get the sine component of the modulation signal
        ControllerIndex oscMasterIndex;     // to get the osc phase chA
        ControllerIndex oscSlaveIndex;      // to get the osc phase chB

        // noise freq limit - pole for low cut
        uint8_t         basaeDepthNoiseExp; // noise amplitude - controller
        uint8_t         tapCount;
    } mode01;

};


}; // end namespace
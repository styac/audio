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
 * File:   FxEarlyReflectionParam.h
 * Author: Istvan Simon
 *
 * Created on May 20, 2017, 8:11 PM
 */

#include "protocol.h"
#include "Tags.h"
#include "control/Controllers.h"
#include "effects/DelayTap.h"

namespace yacynth {
using namespace TagEffectTypeLevel_02;
using namespace TagEffectFxEarlyReflectionModeLevel_03;

class FxEarlyReflectionParam {
public:
    // mandatory fields
    static constexpr char const * const name        = "EarlyReflection";
    static constexpr TagEffectType  type            = TagEffectType::FxEarlyReflection;
    static constexpr std::size_t maxMode            = 4; // 0 is always exist> 0,1,2
    static constexpr std::size_t inputCount         = 1;
    static constexpr std::size_t slaveCount         = 1; // 0-base signal 1-modulation
    static constexpr char const * const slavename   = "^ EarlyReflectionDelayedOutput";
    // optional
    static constexpr std::size_t channelCount       = 2;
    static constexpr std::size_t tapCount           = 1<<4; // 5 ?

    static constexpr int delayLngExp        = 13; // ca: 150 msec - 128*1.3
    static constexpr int delayLng           = 1<<delayLngExp;
    static constexpr int delayOffsMaxLng    = delayLng - 1;
    static constexpr int delayOffsMinLng    = effectFrameSize+1;

    bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    struct Mode01 {
        static constexpr uint8_t subtype = uint8_t(TagEffectFxEarlyReflectionMode::SetParametersMode01);
        static constexpr char const * const delayLateReverbName = "lateReverbDelay";
        static constexpr char const * const earlyreflectionName = "earlyReflection";
        static constexpr int delayOffsMaxLng    = FxEarlyReflectionParam::delayOffsMaxLng;
        static constexpr int delayOffsMinLng    = FxEarlyReflectionParam::delayOffsMinLng;

        bool check()
        {
            for( auto &v0 : lateReverb.delayIndex.v ) {
                if(( v0 > delayOffsMaxLng ) || ( v0 < delayOffsMinLng )) {
                    std::cout << " error delayOffsMaxLng " << v0 << std::endl;
                    return false;
                }
            }

            for( auto &v0 : tap.coeff.v ) {
                if(( v0 > 1.0f ) || ( v0 < -1.0f )) {
                    std::cout << " error coeff " << std::endl;
                    return false;
                }
            }

            for( auto &v0 : tap.modDepth.v ) {
                if(( v0 > 1.0f ) || ( v0 < -1.0f )) {
                    std::cout << " error modDepth " << std::endl;
                    return false;
                }
            }

            constexpr int32_t minModDp = freq2deltaPhaseControlLfo(0.1);
            constexpr int32_t maxModDp = freq2deltaPhaseControlLfo(20.0);
            for( auto &v0 : tap.modDeltaPhase.v ) {
                if(( v0 > maxModDp ) || ( v0 < minModDp )) {
                    std::cout << " error maxModDp " << std::endl;
                    return false;
                }
            }

            for( auto &v0 : tap.delayIndex.v ) {
                if(( v0 > delayOffsMaxLng ) || ( v0 < delayOffsMinLng )) {
                    std::cout << " error delayOffsMaxLng " << v0 << std::endl;
                    return false;
                }
            }
            return true;
        }

        DelayModulatedTapArrayNCH< tapCount, channelCount > tap;
        // left-right delay for late reverb
        // need a controller later !
        DelayTapArrayNCH< 1, channelCount >
                lateReverb;
        // uint8_t tapCount;
    } mode01;
};

}; // end namespace yacynth
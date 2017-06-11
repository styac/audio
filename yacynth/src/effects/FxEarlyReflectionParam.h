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


#include    "protocol.h"
#include    "Tags.h"
#include    "../control/Controllers.h"


namespace yacynth {
using namespace TagEffectTypeLevel_02;
using namespace TagEffectFxEarlyReflectionModeLevel_03;

class FxEarlyReflectionParam {
public:
    // mandatory fields
    static constexpr char const * const name        = "FxEarlyReflection";
    static constexpr TagEffectType  type            = TagEffectType::FxEarlyReflection;
    static constexpr std::size_t maxMode            = 4; // 0 is always exist> 0,1,2
    static constexpr std::size_t inputCount         = 1;
    static constexpr std::size_t slaveCount         = 1; // 0-base signal 1-modulation
    static constexpr char const * const slavename   = " ^EarlyReflectionDelayedOutput";
    // optional
    static constexpr std::size_t channelCount       = 2;
    static constexpr std::size_t tapCount           = 1<<4;
    static constexpr std::size_t tapCountMask       = tapCount-1;
    static constexpr std::size_t coeffSetCount      = 1<<4;
    static constexpr std::size_t coeffSetCountMask  = coeffSetCount-1;

    static constexpr std::size_t delayLngExp        = 9; // ca: 600 msec - 512*1.3
    static constexpr std::size_t delayLng           = 1<<(delayLngExp+effectFrameSizeExp);
    static constexpr std::size_t delayOffsMaxLng    = delayLng - 1;
    static constexpr std::size_t delayOffsMinLng    = effectFrameSize * 2;

    // types for each sub struct

    bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    // no Tap... type is used because of the modulation experiment

    struct Mode01 {
        static constexpr uint8_t subtype         = uint8_t(TagEffectFxEarlyReflectionMode::SetParametersMode01);
        // should return error code not bool
        bool check()
        {
            for( auto &v0 : delayLateReverb ) {
                if(( v0 > delayOffsMaxLng ) || ( v0 < delayOffsMinLng )) {
                    return false;
                }
            }
            for( auto &v0 : delaysEarlyreflection ) {
                for( auto &v1 : v0 ) {
                    if( ( v1 > delayOffsMaxLng ) || ( v1 < delayOffsMinLng )) {
                        return false;
                    }
                }
            }
            for( auto &v0 : coeffsEarlyreflection ) {
                for( auto &v1 : v0 ) {
                    for( auto &v2 : v1 ) {
                        if(( v2 > 1.0f ) || ( v2 < -1.0f )) {
                            return false;
                        }
                    }
                }
            }
            for( auto &v0 : modulatorPeriod ) {
                constexpr std::size_t maxPeriod = 2048 / coeffSetCount; // 2.5 sec if 1 period / set
                constexpr std::size_t minPeriod = 3;    // 3 period ca 4 msec * 16 - 64 msec - 10-15 Hz
                if(( v0 > maxPeriod ) || ( v0 < minPeriod )) { // irreal
                    return false;
                }
            }
            return true;
        }
        // left-right delay for late reverb
        // need a controller later !
        uint32_t    delayLateReverb[channelCount];
        // left-right delay for early reflections
        uint32_t    delaysEarlyreflection[tapCount][channelCount];
        // left-right for early reflections
        // coeffSetCount - rotating with low frequency modulation -simple case uses 0
        // there should be 1 period - deviance about 10%
        float       coeffsEarlyreflection[coeffSetCount][tapCount][channelCount];
        uint16_t    modulatorPeriod[tapCount];
    } mode01;
};

}; // end namespace yacynth
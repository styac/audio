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
 * File:   FxFlangerParam.h
 * Author: Istvan Simon
 *
 * Created on May 20, 2017, 8:12 PM
 */



#include    "protocol.h"
#include    "Tags.h"
#include    "../control/Controllers.h"


namespace yacynth {

using namespace TagEffectTypeLevel_02;
using namespace TagEffectFxFlangerModeLevel_03;

class FxFlangerParam {
public:
    // mandatory fields
    static constexpr char const * const name        = "FxFlanger";
    static constexpr TagEffectType  type            = TagEffectType::FxFlanger;
    static constexpr std::size_t maxMode            = 4;
    static constexpr std::size_t inputCount         = 1;

    // static constexpr std::size_t tapSize            = 4;
    static constexpr std::size_t delayLngExp        = 5; // 2048
    static constexpr std::size_t delayLng           = 1<<(delayLngExp+effectFrameSizeExp);
    static constexpr std::size_t delayOffsMaxLng    = delayLng - 1;
    static constexpr std::size_t delayOffsMinLng    = 0; // 0 ?

    static constexpr uint64_t minBaseDelay          = 128LL << 32;  // ca 2.5 msec
    static constexpr uint64_t maxBaseDelay          = 1000LL << 32; // ca 20 msec

    static constexpr int32_t  depthHighLimit        = 1<<29;
    static constexpr int32_t  depthLowLimit         = 1<<4;

    bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    struct Mode01 {
        static constexpr uint8_t subtype         = uint8_t(TagEffectFxFlangerMode::SetParametersMode01);

        bool check()
        {
            if( baseDelay < minBaseDelay ) {
                baseDelay = minBaseDelay;
            }
            if( baseDelay > maxBaseDelay ) {
                baseDelay = maxBaseDelay;
            }
            if( gain < -1.0 ) {
                gain = -1.0;
            }
            if( gain > 1.0 ) {
                gain = 1.0;
            }
            if( feedbackGain < -0.99 ) {
                feedbackGain = -0.99;
            }
            if( feedbackGain > 0.99 ) {
                feedbackGain = 0.99;
            }
            if( depth > depthHighLimit ) {
                depth = depthHighLimit;
            }
            if( depth < depthLowLimit ) {
                depth = depthLowLimit;
            }

            return true;
        }

        // use minDelay + unipolar function > sin, triangle

        uint64_t        baseDelay;          // controller ?
        float           gain;            // wetIndex
        float           feedbackGain;       // feedbackIndex
        int32_t         depth;              // depthIndex

        // TODO : check max range with baseDelay !
        // manual
        ControllerIndex feedbackIndex;      // tune feedbackGain
        ControllerIndex depthIndex;         // tune sineRange
        ControllerIndex deltaPhaseIndex;    // to set the freq
        ControllerIndex phaseDiffIndex;     // to set the phase diff A-B

        // get the sine component of the modulation signal
        ControllerIndex oscMasterIndex;     // to get the osc phase chA
        ControllerIndex oscSlaveIndex;      // to get the osc phase chB

    } mode01;

};

}; // end namespace yacynth
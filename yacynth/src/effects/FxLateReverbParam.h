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
 * File:   FxLateReverbParam.h
 * Author: Istvan Simon
 *
 * Created on May 20, 2017, 8:12 PM
 */

#include    "protocol.h"
#include    "Tags.h"
#include    "../control/Controllers.h"
#include    "../effects/DelayTap.h"

namespace yacynth {
using namespace TagEffectTypeLevel_02;
using namespace TagEffectFxLateReverbModeLevel_03;

class FxLateReverbParam {
public:
    // mandatory fields
    static constexpr char const * const name    = "LateReverb";
    static constexpr TagEffectType  type        = TagEffectType::FxLateReverb;
    static constexpr std::size_t maxMode        = 3;    // 0 is always exist> 0,1,2
    static constexpr std::size_t inputCount     = 1;
    static constexpr std::size_t allpassLngExp  = 11;   // much more then needed - will be not used
    static constexpr std::size_t combLngExp     = 11;   // much more then needed
    static constexpr std::size_t allpassCount   = 4;
    static constexpr std::size_t combCount      = 8;
    static constexpr std::size_t delayLng       = 1<<(combLngExp+effectFrameSizeExp);

    static constexpr float householderFeedback  = -2.0f;     // combCount;
    static constexpr float lowpassLowLimit      = f2FilterOnePole_F( 18000.0 );
    static constexpr float lowpassHighLimit     = f2FilterOnePole_F( 30.0 );
    static constexpr float feedbackLimit        = 0.6f;
    static constexpr float outputLimit          = 0.8f;
    static constexpr float crossFeedbackLimit   = 0.8f;

    bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    // control feedback - decay
    // control low pass - tone
    // control early reflection - other effect
    // control mixing dry - wet - mixer

    struct Mode01 {
        static constexpr uint8_t subtype         = uint8_t(TagEffectFxLateReverbMode::SetParametersMode01);
        static constexpr char const * const tapFeedbackName = "tapFeedback";
        static constexpr char const * const tapFeedbackInternalName = "tapFeedbackInternal";
        static constexpr char const * const tapOutputName = "tapOutput";

        bool check()
        {
            for( uint16_t ci = 0; ci <combCount; ++ci ) {
                // check tapFeedback
                if(( tapFeedback.coeffLowPass.v[ci] > lowpassHighLimit ) || ( tapFeedback.coeffLowPass.v[ci] < lowpassLowLimit )) {
                    std::cout << "\n---- check 2"  << std::endl;
                    return false;
                }
                // feedback -- householder actually 0.5?
                if( std::abs( tapFeedback.coeff.v[ci] ) >= feedbackLimit * ( 1.0f - tapFeedback.coeffLowPass.v[ci] )) {
                    std::cout << "\n---- check 1" << std::endl;
                    return false;
                }
                if(( tapFeedback.coeffHighPass.v[ci] > lowpassHighLimit ) || ( tapFeedback.coeffHighPass.v[ci] < lowpassLowLimit )) {
                    std::cout << "\n---- check 3" << std::endl;
                    return false;
                }

                // check output
                if(( tapOutput.coeffLowPass.v[ci] > lowpassHighLimit ) || ( tapOutput.coeffLowPass.v[ci] < lowpassLowLimit ))  {
                    std::cout << "\n---- check 4" << std::endl;
                    return false;
                }
                // 0.8f -- summ of 4 combs must be less then 1.0
                if( std::abs( tapOutput.coeff.v[ci] ) >= outputLimit * ( 1.0f - tapOutput.coeffLowPass.v[ci] ))  {
                    std::cout << "\n---- check 5" << std::endl;
                    return false;
                }
                if( tapOutput.delayIndex[ci] >= delayLng ) {
                    std::cout << "\n---- check 6" << std::endl;
                    return false;
                }

                // check internal
                if( std::abs( tapFeedbackInternal.coeff.v[ci] ) >= crossFeedbackLimit )  {
                    std::cout << "\n---- check 7" << std::endl;
                    return false;
                }
                if( tapFeedbackInternal.delayIndex[ci]  >= delayLng ) { // 3 is practical const
                    std::cout << "\n---- check 8" << std::endl;
                    return false;
                }
            }
            return true;
        }
        DelayMultBandpassTapArrayNCH<combCount,1> tapFeedback;
        DelayMultLowpassTapArrayNCH<combCount,1>  tapFeedbackInternal;
        DelayMultLowpassTapArrayNCH<combCount,1>  tapOutput;
    } mode01;
};

}; // end namespace yacynth
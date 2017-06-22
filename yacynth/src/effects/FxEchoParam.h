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
 * File:   FxEchoParam.h
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
using namespace TagEffectFxEchoModeLevel_03;

class FxEchoParam {
public:
    // mandatory fields
    static constexpr char const * const name        = "Echo";
    static constexpr TagEffectType  type            = TagEffectType::FxEcho;
    static constexpr std::size_t maxMode            = 2;
    static constexpr std::size_t inputCount         = 1;

    static constexpr std::size_t tapOutputSize      = 8;
    static constexpr std::size_t tapFeedbackSize    = 8;
    static constexpr std::size_t tapOutputLPSize    = 8;
    static constexpr std::size_t tapFeedbackLPSize  = 8;

    static constexpr std::size_t delayLngExp        = 13;
    static constexpr std::size_t delayLng           = 1<<(delayLngExp+effectFrameSizeExp);
    static constexpr std::size_t delayOffsMaxLng    = delayLng - 1;
    static constexpr std::size_t delayOffsMinLng    = effectFrameSize * 2;

    static constexpr float lowpassLowLimit          = f2FilterOnePole_F( 18000.0 );
    static constexpr float lowpassHighLimit         = f2FilterOnePole_F( 30.0 );
    static constexpr float feedbackLimit            = 0.6f;
    static constexpr float outputLimit              = 1.0f;

    static constexpr uint8_t subtype         = uint8_t(TagEffectFxEchoMode::SetParametersMode01);

    bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    bool check()
    {
        for( auto &v0 : tapOutput.delayIndex ) {
            for( auto &v1 : v0 ) {
                if( ( v1 > delayOffsMaxLng ) || ( v1 < delayOffsMinLng )) {
                    std::cout << "\n---- check 1"  << std::endl;
                    return false;
                }
            }
        }

        for( auto &v0 : tapFeedback.delayIndex ) {
            for( auto &v1 : v0 ) {
                if( ( v1 > delayOffsMaxLng ) || ( v1 < delayOffsMinLng )) {
                    std::cout << "\n---- check 2"  << std::endl;
                    return false;
                }
            }
        }

        for( auto &v0 : tapOutputLP.delayIndex ) {
            for( auto &v1 : v0 ) {
                if( ( v1 > delayOffsMaxLng ) || ( v1 < delayOffsMinLng )) {
                    std::cout << "\n---- check 3"  << std::endl;
                    return false;
                }
            }
        }

        for( auto &v0 : tapFeedbackLP.delayIndex ) {
            for( auto &v1 : v0 ) {
                if( ( v1 > delayOffsMaxLng ) || ( v1 < delayOffsMinLng )) {
                    std::cout << "\n---- check 4"  << std::endl;
                    return false;
                }
            }
        }

        // check coeffs
        // output abs : 0 ...< 1
        // feedback  abs : 0 ...< 1
        // pole feedback k : 0.1 ...+0.99 (check 30 Hz...14kHz ?)
        // pole A*(1-k) : A < 1.0 ->


        for( auto &v0 : tapOutput.coeff ) {
            for( auto &v1 : v0.v ) {
                if( std::abs(v1) > outputLimit ) {
                    std::cout << "\n---- check 5"  << std::endl;
                    return false;
                }
            }
        }
        for( auto &v0 : tapFeedback.coeff ) {
            for( auto &v1 : v0.v ) {
                if( std::abs(v1) > outputLimit ) {
                    std::cout << "\n---- check 6"  << std::endl;
                    return false;
                }
            }
        }

        // this should be A * (1-k) : |A| < 1.0
        for( auto &v0 : tapOutputLP.coeff ) {
            for( auto &v1 : v0.v ) {
                if( std::abs(v1) > outputLimit ) {
                    std::cout << "\n---- check 7"  << std::endl;
                    return false;
                }
            }
        }


        for( auto &v0 : tapOutputLP.coeffLowPass ) {
            for( auto &v1 : v0.v ) {
                if(( v1 < lowpassLowLimit ) || ( v1 > lowpassHighLimit )) {
                    std::cout << "\n---- check 8"  << std::endl;
                    return false;
                }
            }
        }
        // this should be A * (1-k) : |A| < 1.0
        for( auto &v0 : tapFeedbackLP.coeff ) {
            for( auto &v1 : v0.v ) {
                if( std::abs(v1) > outputLimit ) {
                    std::cout << "\n---- check 9"  << std::endl;
                    return false;
                }
            }
        }

        for( auto &v0 : tapFeedbackLP.coeffLowPass ) {
            for( auto &v1 : v0.v ) {
                if(( v1 < lowpassLowLimit ) || ( v1 > lowpassHighLimit )) {
                    std::cout << "\n---- check 10"  << std::endl;
                    return false;
                }
            }
        }

        // limit the counts
        if( tapOutputCount > tapOutputSize ) {
            tapOutputCount = tapOutputSize;
        }

        if( tapFeedbackCount > tapFeedbackSize ) {
            tapFeedbackCount = tapFeedbackSize;
        }

        if( tapOutputLPCount > tapOutputLPSize ) {
            tapOutputLPCount = tapOutputLPSize;
        }

        if( tapFeedbackLPCount > tapFeedbackLPSize ) {
            tapFeedbackLPCount = tapFeedbackLPSize;
        }

        // limit the mixer values
        if( dry[0] > 1.0f ) {
            dry[0] = 1.0f;
        } else if( dry[0] < -1.0f ) {
            dry[0] = -1.0f;
        }

        if( dry[1] > 1.0f ) {
            dry[1] = 1.0f;
        } else if( dry[1] < -1.0f ) {
            dry[1] = -1.0f;
        }
        return true;
    }
    StereoDelayTapArray<tapOutputSize>              tapOutput;
    StereoDelayTapArray<tapFeedbackSize>            tapFeedback;
    StereoDelayLowPassTapArray<tapOutputLPSize>     tapOutputLP;
    StereoDelayLowPassTapArray<tapFeedbackLPSize>   tapFeedbackLP;
    // actual value : tap...Count <= ... tap...Size
    uint8_t                                         tapOutputCount;
    uint8_t                                         tapFeedbackCount;
    uint8_t                                         tapOutputLPCount;
    uint8_t                                         tapFeedbackLPCount;
    // dry mix value for chA,chB
    float                                           dry[2];
};



}; // end namespace yacynth
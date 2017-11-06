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
    static constexpr char const * const name        = "Chorus";
    static constexpr TagEffectType  type            = TagEffectType::FxChorus;
    static constexpr std::size_t maxMode            = 2;
    static constexpr std::size_t inputCount         = 1;

    static constexpr int64_t delayLngExp            = 13;   // 8192 sample * 20 nsec - 160msec
    static constexpr int64_t delayLng               = 1<<delayLngExp;

    // fractional delay values: (1<<32)
    static constexpr int64_t minBaseDelay           = 1LL << 32;
    static constexpr int64_t maxBaseDelay           = 5 * (delayLng << 28) ;

    static constexpr std::size_t tapSize            = 8;

    bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    struct Mode01 {
        static constexpr uint8_t subtype         = uint8_t(TagEffectFxChorusMode::SetParametersMode01);

        bool check()
        {
            for( auto i=0u; i<tapSize; ++i ) {
                for( auto j=0; j<2; ++j ) {
                    if( baseDelay[ i ][ j ] < minBaseDelay ) {
                        baseDelay[ i ][ j ] = minBaseDelay;
                    }
                    if( baseDelay[ i ][ j ] > maxBaseDelay ) {
                        baseDelay[ i ][ j ] = maxBaseDelay;               
                    }
                }                
            }
            
            if( wetGain < -1.0 ) {
                wetGain = -1.0;
            }

            if( wetGain > 1.0 ) {
                wetGain = 1.0;
            }
            
            if( tapCount < 1 ) {
                tapCount = 1;
            }
            if( tapCount > tapSize ) {
                tapCount = tapSize;
            }
            // wetgain /= tapCount;
            return true;
        }
        int64_t         baseDelay[ tapSize ][ 2 ]; // chorus tap base delay ( fractional value -- high 32 bit delay in samples !)
        int32_t         depth[ tapSize ][ 2 ];          
        float           wetGain;

        // manual
        // how to handle all parameters ?
        ControllerIndex depthIndex;         // to set the mod depth
        ControllerIndex deltaPhaseIndex;    // to set the freq
        ControllerIndex phaseDiffIndex;     // to set the phase diff A-B

        // get the sine component of the modulation signal
        
        // TODO yet
//        ControllerIndex oscMasterIndex[ tapSize ]; // to get the osc phase chA
//        ControllerIndex oscSlaveIndex[ tapSize ];  // to get the osc phase chB
        
        // TODO > random phase controller
        
        ControllerIndex oscMasterIndex;     // to get the osc phase chA
        ControllerIndex oscSlaveIndex;      // to get the osc phase chB

        uint8_t         tapCount;
        
    } mode01;
};


}; // end namespace
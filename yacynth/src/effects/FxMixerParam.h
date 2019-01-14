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

#include "protocol.h"
#include "Tags.h"
#include "control/Controllers.h"


namespace yacynth {

using namespace TagEffectTypeLevel_02;
using namespace TagEffectFxMixerModeLevel_03;
// --------------------------------------------------------------------
// template< uint8_t ccount > -- mixer rework
class FxMixerParam {
public:
    static constexpr char const * const name    = "Mixer16x";
    static constexpr TagEffectType  type        = TagEffectType::FxMixer;
    static constexpr std::size_t maxMode        = 1;
    static constexpr std::size_t inputCount     = 16;
    static constexpr std::size_t slaveCount     = 0; 

    static constexpr uint8_t subtype            = uint8_t(TagEffectFxMixerMode::SetParametersMode01);

    bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    inline bool check()
    {
        if( effectiveInputCount > inputCount) {
            return false;
        }
        
        // check the gainRange <= 1.0
        // check the gainIndex valid
        
        return true;
    }
    
    inline void clear()
    {
        effectiveInputCount = 1;
        for( auto &p : gainIndex ) {
            p.setIndex( InnerController::CC_NULL );
        }
        for( auto &p : gainZero ) {
            p = 0;
        }
    }

    inline void preset0()
    {
        effectiveInputCount = 1;
        for( auto &p : gainIndex ) {
            p.setIndex( InnerController::CC_127 );
        }
        for( auto &p : gainZero ) {
            p = 0;
        }
        gainRange[ 0 ][ 0 ] = gainRange[ 0 ][ 1 ] = 0.9f;
        gainRange[ 1 ][ 0 ] = gainRange[ 1 ][ 1 ] = 0.9f;
    }

    bool setRange( uint8_t ch, float ch0, float ch1 ) 
    {
        if( ch >= inputCount || std::abs(ch0) > 1.0f || std::abs(ch1) > 1.0f ) {
            return false;
        }
        gainRange[ ch ][ 0 ] = ch0;
        gainRange[ ch ][ 1 ] = ch1;
        return true;
    }

    union {
        float       gainRange[ inputCount ][ 2 ];  // for each stereo channel
        uint64_t    gainZero[ inputCount ]; // to set,test if zero        
    };

    ControllerIndex gainIndex[ inputCount ];    // default: CC_127
    uint8_t         effectiveInputCount;        // 1..inputCount
};


}; // end namespace yacynth
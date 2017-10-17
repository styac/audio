/*
 * Copyright (C) 2016 Istvan Simon -- stevens37 at gmail dot com
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
 * File:   FxOscillatorMixer.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on June 23, 2016, 9:41 PM
 */

#include "FxOscillatorMixer.h"

namespace yacynth {

using namespace TagEffectFxOscillatorMixerModeLevel_03;

bool FxOscillatorMixerParam::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    const uint8_t tag = message.getTag(tagIndex);
    switch( TagEffectFxOscillatorMixerMode( tag ) ) {
    case TagEffectFxOscillatorMixerMode::Clear :
        clear();
        message.setStatusSetOk();        
        return true;

    case TagEffectFxOscillatorMixerMode::Preset :
        for( auto& g : gain ) {
            g = gainref;
        }
        message.setStatusSetOk();        
        return true;

    case TagEffectFxOscillatorMixerMode::SetParametersMode01 :  // set all volumes
        // TODO
        message.setStatusSetOk();        
        return true;

    case TagEffectFxOscillatorMixerMode::SetChannelVolume :     // set 1 volume
        // TODO
        message.setStatusSetOk();        
        return true;
    }

    message.setStatus( yaxp::MessageT::illegalTag );
    return false;
}


void FxOscillatorMixer::clearState()
{
    //out().clear();    
    // + internal state    
    //for( uint16_t si = 0u; si < FxOscillatorMixerParam::slaveCount; ++si ) {
    //    slaves[ si ].clear();
    //}                
}


bool FxOscillatorMixer::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    // 1st tag is tag effect type
    const uint8_t tagType = message.getTag(tagIndex);
    if( uint8_t(param.type) != tagType ) {
        message.setStatus( yaxp::MessageT::illegalTagEffectType );
        TAG_DEBUG(yaxp::MessageT::illegalTagEffectType, uint8_t(param.type), tagType, "FxOscillatorMixer" );
        return false;
    }
    // 2nd tag is tag operation
    const uint8_t tag = message.getTag(++tagIndex);
    switch( TagEffectFxOscillatorMixerMode( tag ) ) {
    case TagEffectFxOscillatorMixerMode::ClearState:
        clearState(); // this must be called to cleanup
        message.setStatusSetOk();
        return true;
    case TagEffectFxOscillatorMixerMode::Clear:
        clearState(); // this must be called to cleanup
        break;
    }
    // forward to param
    return param.parameter( message, tagIndex, paramIndex );
}

bool FxOscillatorMixer::connect( const FxBase * v, uint16_t ind )
{
    doConnect( v, ind );
};

bool FxOscillatorMixer::setSprocessNext( uint16_t mode )
{
    return true; // no modes at the moment
}

} // end namespace yacynth



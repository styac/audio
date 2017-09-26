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
//    switch(  ( message.getTag(tagIndex) ) ) {
//    case  :
//        TAG_DEBUG(TagMidiController::ClearChannelVector, tagIndex, paramIndex, " " );
//        return true;
//    }
//
    switch( TagEffectFxOscillatorMixerMode( tag ) ) {
    case TagEffectFxOscillatorMixerMode::Clear :
        for( auto& g : gain ) {
            g = 0.0f;
        }
        return true;

    case TagEffectFxOscillatorMixerMode::Preset0 :
        for( auto& g : gain ) {
            g = gainref;
        }
        return true;

    case TagEffectFxOscillatorMixerMode::SetParametersMode01 :  // set all volumes
        return true;

    case TagEffectFxOscillatorMixerMode::SetChannelVolume :     // set 1 volume
        return true;
    }

    message.setStatus( yaxp::MessageT::illegalTag );
    return false;
}


void FxOscillatorMixer::clearTransient()
{
    EIObuffer::clear();
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
    case TagEffectFxOscillatorMixerMode::Clear:
        clearTransient(); // this must be called to cleanup
        break;
    }
    // forward to param
    return param.parameter( message, tagIndex, paramIndex );
}

template<>
bool FxSlave<FxOscillatorMixerParam>::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    // 1st tag is tag effect type
    const uint8_t tagType = message.getTag(tagIndex);
    if( uint8_t(TagEffectType::FxSlave) != tagType ) {
        message.setStatus( yaxp::MessageT::illegalTagEffectType );
        return false;
    }
    // 2nd tag is tag operation
    const uint8_t tag = message.getTag(++tagIndex);
//    if( uint8_t(TagEffectFxFilterMode::Clear) == tag ) {
//        clearTransient(); // this must be called to cleanup
//    }
    // forward to param
    return true;
};

template<>
void FxSlave<FxOscillatorMixerParam>::clearTransient()
{
};


} // end namespace yacynth



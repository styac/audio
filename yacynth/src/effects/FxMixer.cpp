/*
 * Copyright (C) 2016 Istvan Simon
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
 * File:   Mixer.cpp
 * Author: Istvan Simon
 *
 * Created on March 15, 2016, 6:23 PM
 */

#include "FxMixer.h"

namespace yacynth {
using namespace TagEffectFxMixerModeLevel_03;


bool FxMixerParam::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    uint16_t index = 0;
    const uint8_t tag       = message.getTag(tagIndex);
    if( !message.checkParamIndex(paramIndex) ) {
        message.setStatus( yaxp::MessageT::illegalParamIndex );
        return false;
    }
    const uint16_t channel  = message.params[paramIndex];
    if(channel >= inputCount) {
        message.setStatus( yaxp::MessageT::illegalTargetIndex);
        return false;
    }
    switch( TagEffectFxMixerMode( tag ) ) {
    case TagEffectFxMixerMode::Clear :
        TAG_DEBUG(TagEffectFxMixerMode::Clear, tagIndex, paramIndex, "FxMixerParam" );
        clear();
        message.setStatusSetOk();
        return true;

    case TagEffectFxMixerMode::SetVolumeControllerIndex :
        TAG_DEBUG(TagEffectFxMixerMode::SetVolumeControllerIndex, tagIndex, paramIndex, "FxMixerParam" );
        if( message.setTargetData( index ) ) {
            gainIndex[channel].setIndex(index);
            message.setStatusSetOk();
            return true;
        }
        message.setStatus( yaxp::MessageT::illegalDataLength );
        return false;

    case TagEffectFxMixerMode::SetVolumeRange : {
        TAG_DEBUG(TagEffectFxMixerMode::SetVolumeRange, tagIndex, paramIndex, "FxMixerParam" );
        if( message.length != 2 * sizeof(float) ) {
            message.setStatus( yaxp::MessageT::illegalDataLength );
            return false;            
        }
        const float * chx = (float *) message.data;
        if( setRange( channel, chx[0], chx[1] ) ) {
            std::cout <<  "--- channel " << channel <<  " volume " << gainRange[channel] << std::endl;
            message.setStatusSetOk();
            return true;
        }
        }
        message.setStatus( yaxp::MessageT::illegalDataLength );
        return false;

    case TagEffectFxMixerMode::SetChannelCount :
        TAG_DEBUG(TagEffectFxMixerMode::SetChannelCount, tagIndex, paramIndex, "FxMixerParam" );
        effectiveInputCount = channel;
        message.setStatusSetOk();
                std::cout << "---- effectiveInputCount " << uint16_t(effectiveInputCount)  << std::endl;

        return true;
                                       
    case TagEffectFxMixerMode::Preset : {
        TAG_DEBUG(TagEffectFxMixerMode::Preset, tagIndex, paramIndex, "FxMixerParam" );
        preset0();
        message.setStatusSetOk();
        return true;
        }
    
    default:
        break;
    } // end switch
    TAG_DEBUG(TagEffectFxMixerMode::Nop, tagIndex, paramIndex, "FxMixerParam" );
    message.setStatus( yaxp::MessageT::illegalTag );
    return false;

} // end FxMixerParam::parameter

bool FxMixer::connect( const FxBase * v, uint16_t ind )
{
    return doConnect(v,ind);
};

void FxMixer::clearState()
{
    // out().clear();    
}


bool FxMixer::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    // 1st tag is tag effect type
    const uint8_t tagType = message.getTag(tagIndex);
    if( uint8_t(param.type) != tagType ) {
        message.setStatus( yaxp::MessageT::illegalTagEffectType );
        TAG_DEBUG(yaxp::MessageT::illegalTagEffectType, uint8_t(param.type), tagType, "FxMixer" );
        return false;
    }
    // 2nd tag is tag operation
    const uint8_t tag = message.getTag(++tagIndex);
    switch( TagEffectFxMixerMode( tag ) ) {
    case TagEffectFxMixerMode::ClearState:
        clearState(); // this must be called to cleanup
        message.setStatusSetOk();
        return true;
    case TagEffectFxMixerMode::Clear:
        clearState(); // this must be called to cleanup
        break;
    case TagEffectFxMixerMode::Preset :
        setProcessingMode(1);    
        break;    
    default:
        break;
    }
    // forward to param
    return param.parameter( message, tagIndex, paramIndex );
}

void FxMixer::sprocess_01( void * thp )
{
    static_cast< MyType * >(thp)->process();
}


bool FxMixer::setSprocessNext( uint16_t mode )
{
    switch( mode ) {
    case 0:
        procMode = 0;
        sprocesspNext = FxBase::sprocessClear2Nop;
        sprocessp = FxBase::sprocessFadeOut;        
        return true;
        
    case 1:
        sprocesspNext = sprocess_01;
        break;
        
    default:
        return false;
    }
    
    bool fadeIn = 0 == procMode;
    procMode = mode;
    if( fadeIn ) {
        sprocesspCurr = sprocesspNext;
        sprocessp = FxBase::sprocessFadeIn;
        return true;
    }
    sprocessp = FxBase::sprocessCrossFade;
    return true;
}

} // end namespace yacynth

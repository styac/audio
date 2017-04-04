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
using namespace TagEffectFxOscillatorMixerModeLevel_03;

FxMixerParam::FxMixerParam()
{
    gainTarget.index = InnerController::CC_MAINVOLUME;
    gainTarget.setShift(0);

}

bool FxMixerParam::parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    const uint8_t tag = message.getTag(tagIndex);
//    switch(  ( message.getTag(tagIndex) ) ) {
//    case  :
//        TAG_DEBUG(TagMidiController::ClearChannelVector, tagIndex, paramIndex, " " );
//        return true;
//    }
    switch( TagEffectFxOscillatorMixerMode( tag ) ) {
    case TagEffectFxOscillatorMixerMode::Clear :
        return true;
    }
            
    message.setStatus( Yaxp::MessageT::illegalTag, tag );    
    return false;
    
}

bool FxMixer::connect( const FxBase * v, uint16_t ind )
{
    doConnect(v,ind);
};

void FxMixer::clearTransient()
{
    EIObuffer::clear();
}
#if 1
bool FxMixer::parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    // 1st tag is tag effect type
    const uint8_t tagType = message.getTag(tagIndex);    
    if( uint8_t(param.type) != tagType ) {
        message.setStatus( Yaxp::MessageT::illegalTagEffectType, uint8_t(param.type) );
        TAG_DEBUG(Yaxp::MessageT::illegalTagEffectType, uint8_t(param.type), tagType, "FxMixer" );
        return false;        
    }
    // 2nd tag is tag operation
    const uint8_t tag = message.getTag(++tagIndex);
    if( uint8_t(TagEffectFxOscillatorMixerMode::Clear) == tag ) {
        clearTransient(); // this must be called to cleanup
    }
    // forward to param
    return param.parameter( message, tagIndex, paramIndex );    
}
#endif

} // end namespace yacynth

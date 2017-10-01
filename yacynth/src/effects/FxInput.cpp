/*
 * Copyright (C) 2017 ist
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
 * File:   FxInput.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on September 16, 2017, 10:37 AM
 */
#include    "FxInput.h"

namespace yacynth {
using namespace TagEffectFxInputModeLevel_03;

// probably no settable parameter
bool FxInputParam::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    const uint8_t tag = message.getTag(tagIndex);
//    switch(  ( message.getTag(tagIndex) ) ) {
//    case  :
//        TAG_DEBUG(TagMidiController::ClearChannelVector, tagIndex, paramIndex, " " );
//        return true;
//    }
//    
    switch( TagEffectFxInputMode( tag ) ) {
    case TagEffectFxInputMode::Clear :
        return true;
    }
            
    message.setStatus( yaxp::MessageT::illegalTag );
    return false;
    
}


void FxInput::clearTransient()
{
    out().clear();    
}

bool FxInput::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    // 1st tag is tag effect type
    const uint8_t tagType = message.getTag(tagIndex);    
    if( uint8_t(param.type) != tagType ) {
        message.setStatus( yaxp::MessageT::illegalTagEffectType );
        TAG_DEBUG(yaxp::MessageT::illegalTagEffectType, uint8_t(param.type), tagType, "FxInput" );
        return false;        
    }
    // 2nd tag is tag operation
    const uint8_t tag = message.getTag(++tagIndex);
    switch( TagEffectFxInputMode( tag ) ) {
    case TagEffectFxInputMode::ClearState:
        clearTransient(); // this must be called to cleanup
        message.setStatusSetOk();
        return true;
        
    case TagEffectFxInputMode::Clear:
        clearTransient(); // this must be called to cleanup
        break;
    }
    // forward to param
    return param.parameter( message, tagIndex, paramIndex ); 
}


} // end namespace yacynth



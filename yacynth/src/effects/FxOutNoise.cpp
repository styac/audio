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

#include    "FxOutNoise.h"

namespace yacynth {
using namespace TagEffectFxOutNoiseModeLevel_03;

bool FxOutNoiseParam::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    const uint8_t tag = message.getTag(tagIndex);
    switch( TagEffectFxOutNoiseMode( tag ) ) {
    case TagEffectFxOutNoiseMode::Clear :
        TAG_DEBUG(TagEffectFxOutNoiseMode::Clear, tagIndex, paramIndex, "FxOutNoiseParam" );
        message.setStatusSetOk();
        return true;

    case TagEffectFxOutNoiseMode::SetParametersMode01 :
        TAG_DEBUG(TagEffectFxOutNoiseMode::SetParametersMode01, tagIndex, paramIndex, "FxOutNoiseParam" );
        if( !message.setTargetData(*this) ) {
            message.setStatus( yaxp::MessageT::illegalDataLength );
            return false;
        }
        message.setStatusSetOk();
        return true;
        
    default:
        break;        
    }
    message.setStatus( yaxp::MessageT::illegalTag );
    return false;
}

void FxOutNoise::clearState()
{
    // out().clear();        
}

bool FxOutNoise::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    // 1st tag is tag effect type
    const uint8_t tagType = message.getTag(tagIndex);
    if( uint8_t(param.type) != tagType ) {
        message.setStatus( yaxp::MessageT::illegalTagEffectType );
        TAG_DEBUG(yaxp::MessageT::illegalTagEffectType, uint8_t(param.type), tagType, "FxOutNoise" );
        return false;
    }
    // 2nd tag is tag operation
    const uint8_t tag = message.getTag(++tagIndex);
    switch( TagEffectFxOutNoiseMode( tag ) ) {
    case TagEffectFxOutNoiseMode::ClearState:
        clearState(); // this must be called to cleanup
        message.setStatusSetOk();
        return true;
        
    case TagEffectFxOutNoiseMode::Clear:
        clearState(); // this must be called to cleanup
        break;
    default:
        break;        
    }
    // forward to param
    return param.parameter( message, tagIndex, paramIndex );
}

bool FxOutNoise::connect( const FxBase * v, uint16_t ind )
{
    return doConnect(v,ind);
};


bool FxOutNoise::setSprocessNext( uint16_t mode ) 
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
    case 2:
        sprocesspNext = sprocess_02;
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

// --------------------------------------------------------------------
} // end namespace yacynth
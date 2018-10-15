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
 * File:   FxModulator.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on June 23, 2016, 6:14 PM
 */
#include "yacynth_config.h"
#include "FxModulator.h"

namespace yacynth {
using namespace TagEffectFxModulatorModeLevel_03;

bool FxModulatorParam::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    const uint8_t tag = message.getTag(tagIndex);
    switch( TagEffectFxModulatorMode( tag ) ) {
    case TagEffectFxModulatorMode::Clear :
        TAG_DEBUG(TagEffectFxModulatorMode::Clear, tagIndex, paramIndex, "FxModulatorParam" );
        message.setStatusSetOk();                
        return true;
        
    case TagEffectFxModulatorMode::SetParametersMode01 :
        TAG_DEBUG(TagEffectFxModulatorMode::SetParametersMode01, tagIndex, paramIndex, "FxModulatorParam" );
        if( message.setTargetData(*this) ) {
            message.setStatusSetOk();                            
            return true;
        }        
        message.setStatus( yaxp::MessageT::illegalDataLength );
        return false;

    default:
        break;
    }            
    message.setStatus( yaxp::MessageT::illegalTag );
    return false;    
}

void FxModulator::clearState()
{
    // out().clear();    
    // + internal state
}

bool FxModulator::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    // 1st tag is tag effect type
    const uint8_t tagType = message.getTag(tagIndex);    
    if( uint8_t(param.type) != tagType ) {
        message.setStatus( yaxp::MessageT::illegalTagEffectType );
        TAG_DEBUG(yaxp::MessageT::illegalTagEffectType, uint8_t(param.type), tagType, "FxModulator" );
        return false;        
    }
    // 2nd tag is tag operation
    const uint8_t tag = message.getTag(++tagIndex);
    switch( TagEffectFxModulatorMode( tag ) ) {
    case TagEffectFxModulatorMode::ClearState:
        clearState(); // this must be called to cleanup
        message.setStatusSetOk();
        return true;
        
    case TagEffectFxModulatorMode::Clear:
        clearState(); // this must be called to cleanup
        break;

    default:
        break;        
    }
    // forward to param
    return param.parameter( message, tagIndex, paramIndex );    
}



bool FxModulator::connect( const FxBase * v, uint16_t ind )
{
    return doConnect(v,ind);
};


void FxModulator::sprocess_01( void * thp )
{
    static_cast< MyType * >(thp)->processModulation();
}
void FxModulator::sprocess_02( void * thp )
{
    static_cast< MyType * >(thp)->processRing();
}
void FxModulator::sprocess_03( void * thp )
{
    static_cast< MyType * >(thp)->processModulationMix(); 
}
void FxModulator::sprocess_04( void * thp )
{
    static_cast< MyType * >(thp)->processRingVolColtrol();
}
void FxModulator::sprocess_05( void * thp )
{
    static_cast< MyType * >(thp)->processModulationMono(); 
}
void FxModulator::sprocess_06( void * thp )
{
    static_cast< MyType * >(thp)->processRingMono();
}

bool FxModulator::setSprocessNext( uint16_t mode ) 
{
    switch( mode ) {
    case 0:
        procMode = 0;
        sprocesspNext = FxBase::sprocessClear2Nop;
        sprocessp = FxBase::sprocessFadeOut;        
        return true;
    case 1:         // TODO : Bypass
        sprocesspNext = sprocess_01;
        break;
    case 2:
        sprocesspNext = sprocess_02;
        break;
    case 3:
        sprocesspNext = sprocess_03;
        break;
    case 4:
        sprocesspNext = sprocess_04;
        break;
    case 5:
        sprocesspNext = sprocess_05;
        break;
    case 6:
        sprocesspNext = sprocess_06;
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



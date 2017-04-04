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

#include "FxModulator.h"

namespace yacynth {
using namespace TagEffectFxModulatorModeLevel_03;

FxModulatorParam::FxModulatorParam()
{
    inMult = 1.0f;
//    inMult = 0.0f;
    mixMult = 0.5f;
}


bool FxModulatorParam::parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    const uint8_t tag = message.getTag(tagIndex);
    switch( TagEffectFxModulatorMode( tag ) ) {
    case TagEffectFxModulatorMode::Clear :
        TAG_DEBUG(TagEffectFxModulatorMode::Clear, tagIndex, paramIndex, "FxModulatorParam" );
        return true;
        
    case TagEffectFxModulatorMode::SetMode_01_amplitudeModulation :
        TAG_DEBUG(TagEffectFxModulatorMode::SetMode_01_amplitudeModulation, tagIndex, paramIndex, "FxModulatorParam" );
        if( message.setTargetData(mode_01_amplitudeModulation) ) {
            mode_01_amplitudeModulation.inMultIndex.setInnerValue();
            mode_01_amplitudeModulation.mixMultIndex.setInnerValue();
            return true;
        }        
        message.setStatus( Yaxp::MessageT::illegalDataLength );
        return false;

    case TagEffectFxModulatorMode::GetMode_01_amplitudeModulation :
        TAG_DEBUG(TagEffectFxModulatorMode::GetMode_01_amplitudeModulation, tagIndex, paramIndex, "FxModulatorParam" );
        message.getTargetData(mode_01_amplitudeModulation );
        return true;

    case TagEffectFxModulatorMode::SetMode_02_ringModulation :
        TAG_DEBUG(TagEffectFxModulatorMode::SetMode_02_ringModulation, tagIndex, paramIndex, "FxModulatorParam" );
        if( message.setTargetData(mode_02_ringModulation) ) {
            mode_02_ringModulation.inMultIndex.setInnerValue();
            return true;
        }        
        message.setStatus( Yaxp::MessageT::illegalDataLength );
        return false;

    case TagEffectFxModulatorMode::GetMode_02_ringModulation :
        TAG_DEBUG(TagEffectFxModulatorMode::GetMode_02_ringModulation, tagIndex, paramIndex, "FxModulatorParam" );
        message.getTargetData(mode_02_ringModulation );
        return true;
    }            
    message.setStatus( Yaxp::MessageT::illegalTag, tag );
    return false;
    
}

void FxModulator::clearTransient()
{
    EIObuffer::clear();    
}

bool FxModulator::parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    // 1st tag is tag effect type
    const uint8_t tagType = message.getTag(tagIndex);    
    if( uint8_t(param.type) != tagType ) {
        message.setStatus( Yaxp::MessageT::illegalTagEffectType, uint8_t(param.type) );
        TAG_DEBUG(Yaxp::MessageT::illegalTagEffectType, uint8_t(param.type), tagType, "FxModulator" );
        return false;        
    }
    // 2nd tag is tag operation
    const uint8_t tag = message.getTag(++tagIndex);
    if( uint8_t(TagEffectFxModulatorMode::Clear) == tag ) {
        clearTransient(); // this must be called to cleanup
    }
    // forward to param
    return param.parameter( message, tagIndex, paramIndex );    
}



bool FxModulator::connect( const FxBase * v, uint16_t ind )
{
    doConnect(v,ind);
};

void FxModulator::sprocessTransient( void * thp )
{
    auto& th = *static_cast< MyType * >(thp);
    switch( th.fadePhase ) {
    // 1 phase
    case FadePhase::FPH_fadeNo:
        th.sprocessp = th.sprocesspSave =  th.sprocessv[ th.procMode ];
        th.sprocesspSave(thp);
        return;

    // clear then switch to nop
    case FadePhase::FPH_fadeOutClear:
        th.clear();
        th.procMode = 0;
        th.sprocessp = th.sprocesspSave = sprocessNop;
        return;

    case FadePhase::FPH_fadeOutSimple:
        th.sprocesspSave(thp);
        th.fadeOut();   // then clear -- then nop
        th.sprocessp = th.sprocesspSave =  th.sprocessv[ th.procMode ];
        return;

    // 1 phase
    case FadePhase::FPH_fadeInSimple:
        th.sprocessp = th.sprocesspSave =  th.sprocessv[ th.procMode ];
        th.sprocesspSave(thp);
        th.fadeIn();
        return;

    // 1 of 2 phase
    case FadePhase::FPH_fadeOutCross:
        th.sprocesspSave(thp);
        th.fadeOut();
        th.sprocesspSave =  th.sprocessv[ th.procMode ];
        th.fadePhase = FadePhase::FPH_fadeInCross;
        return;

    // 2 of 2 phase
    case FadePhase::FPH_fadeInCross: // the same as FPH_fadeInSimple ???
        th.sprocessp = th.sprocesspSave =  th.sprocessv[ th.procMode ];
        th.sprocesspSave(thp);
        th.fadeIn();
        return;
    }

}

// 00 is bypass
void FxModulator::sprocess_00( void * thp )
{
    static_cast< MyType * >(thp)->processCopy0();
}
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


} // end namespace yacynth



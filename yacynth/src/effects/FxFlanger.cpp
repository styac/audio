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
 * File:   FxFlanger.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on June 23, 2016, 7:46 PM
 */

#include    "FxFlanger.h"

namespace yacynth {
using namespace TagEffectFxFlangerModeLevel_03;

bool FxFlangerParam::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    const uint8_t tag = message.getTag(tagIndex);
    switch( TagEffectFxFlangerMode( tag ) ) {
    case TagEffectFxFlangerMode::Clear :
        TAG_DEBUG(TagEffectFxFlangerMode::Clear, tagIndex, paramIndex, "TagEffectFxFlangerMode" );
        message.setStatusSetOk();
        return true;

    case TagEffectFxFlangerMode::SetParametersMode01 :
        TAG_DEBUG(TagEffectFxFlangerMode::SetParametersMode01, tagIndex, paramIndex, "TagEffectFxFlangerMode" );
        if( !message.checkTargetData(mode01) ) {
            message.setStatus( yaxp::MessageT::illegalData );
            return false;
        }

        if( !message.setTargetData(mode01) ) {
            message.setStatus( yaxp::MessageT::illegalDataLength );
            return false;
        }
        message.setStatusSetOk();
        return true;
    }
    message.setStatus( yaxp::MessageT::illegalTag );
    return false;
}

void FxFlanger::clearTransient()
{
    out().clear();    
}

bool FxFlanger::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    // 1st tag is tag effect type
    const uint8_t tagType = message.getTag(tagIndex);
    if( uint8_t(param.type) != tagType ) {
        message.setStatus( yaxp::MessageT::illegalTagEffectType );
        TAG_DEBUG(yaxp::MessageT::illegalTagEffectType, uint8_t(param.type), tagType, "FxFlanger" );
        return false;
    }

    // 2nd tag is tag operation
    const uint8_t tag = message.getTag(++tagIndex);
    switch( TagEffectFxFlangerMode( tag ) ) {
    case TagEffectFxFlangerMode::ClearState:
        clearTransient(); // this must be called to cleanup
        message.setStatusSetOk();
        return true;
        
    case TagEffectFxFlangerMode::Clear:
        clearTransient(); // this must be called to cleanup
        break;
    }
    
    // forward to param
    return param.parameter( message, tagIndex, paramIndex );
 }

bool FxFlanger::connect( const FxBase * v, uint16_t ind )
{
    doConnect(v,ind);
};


void FxFlanger::sprocessTransient( void * thp )
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

// 00 is always clear for output or bypass for in-out
void FxFlanger::sprocess_00( void * thp )
{
//        static_cast< MyType * >(thp)->clear();
}

void FxFlanger::sprocess_01( void * thp )
{
    static_cast< MyType * >(thp)->useSine();
    static_cast< MyType * >(thp)->processForward();
}

void FxFlanger::sprocess_02( void * thp )
{
    static_cast< MyType * >(thp)->useTriangle();
    static_cast< MyType * >(thp)->processForward();
}

void FxFlanger::sprocess_03( void * thp )
{
    static_cast< MyType * >(thp)->useSine();
    static_cast< MyType * >(thp)->processFeedback();
}

void FxFlanger::sprocess_04( void * thp )
{
    static_cast< MyType * >(thp)->useTriangle();
    static_cast< MyType * >(thp)->processFeedback();
}

        
} // end namespace yacynth



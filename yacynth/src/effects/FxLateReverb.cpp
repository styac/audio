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
 * File:   FxLateReverb.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on June 23, 2016, 7:46 PM
 */

#include    "FxLateReverb.h"
#include    "yacynth_globals.h"

namespace yacynth {

bool FxLateReverbParam::parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    const uint8_t tag = message.getTag(tagIndex);
    switch( TagEffectFxLateReverbMode( tag ) ) {
    case TagEffectFxLateReverbMode::Clear :
        TAG_DEBUG(TagEffectFxLateReverbMode::Clear, tagIndex, paramIndex, "TagEffectFxLateReverbMode" );
        return true;

    case TagEffectFxLateReverbMode::SetParametersMode01 :
        TAG_DEBUG(TagEffectFxLateReverbMode::SetParametersMode01, tagIndex, paramIndex, "TagEffectFxLateReverbMode" );
        if( !message.setTargetData(mode01_Householder) ) {
            message.setStatus( Yaxp::MessageT::illegalDataLength,uint8_t(TagEffectFxLateReverbMode::SetParametersMode01));
            return false;
        }
        if( mode01_Householder.check() ) {
            return true;
        }
        message.setStatus( Yaxp::MessageT::illegalData, tag );
        return false;
    }
    message.setStatus( Yaxp::MessageT::illegalTag, tag );
    return false;
}

void FxLateReverb::clearTransient()
{
    EIObuffer::clear();
}

bool FxLateReverb::parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    // 1st tag is tag effect type
    const uint8_t tagType = message.getTag(tagIndex);
    if( uint8_t(param.type) != tagType ) {
        message.setStatus( Yaxp::MessageT::illegalTagEffectType, uint8_t(param.type) );
        TAG_DEBUG(Yaxp::MessageT::illegalTagEffectType, uint8_t(param.type), tagType, "FxLateReverb" );
        return false;
    }
    // 2nd tag is tag operation
    const uint8_t tag = message.getTag(++tagIndex);
    if( uint8_t(TagEffectFxLateReverbMode::Clear) == tag ) {
        clearTransient(); // this must be called to cleanup
    }
    // forward to param
    return param.parameter( message, tagIndex, paramIndex );
 }

bool FxLateReverb::connect( const FxBase * v, uint16_t ind )
{
    doConnect(v,ind);
};


void FxLateReverb::sprocessTransient( void * thp )
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
void FxLateReverb::sprocess_00( void * thp )
{
    static_cast< MyType * >(thp)->clear();
}
void FxLateReverb::sprocess_01( void * thp )
{
    static_cast< MyType * >(thp)->process_01();
}

void FxLateReverb::sprocess_02( void * thp )
{
//    static_cast< MyType * >(thp)->process();
}

void FxLateReverb::sprocess_03( void * thp )
{
//    static_cast< MyType * >(thp)->process();
}

} // end namespace yacynth



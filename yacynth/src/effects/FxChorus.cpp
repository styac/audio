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
 * File:   FxChorus.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on June 23, 2016, 7:46 PM
 */

#include    "FxChorus.h"

namespace yacynth {
using namespace TagEffectFxChorusModeLevel_03;

bool FxChorusParam::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    const uint8_t tag = message.getTag(tagIndex);
    switch( TagEffectFxChorusMode( tag ) ) {
    case TagEffectFxChorusMode::Clear :
        TAG_DEBUG(TagEffectFxChorusMode::Clear, tagIndex, paramIndex, "TagEffectFxChorusMode" );
        message.setStatusSetOk();
        return true;

    case TagEffectFxChorusMode::SetParametersMode01 :
        TAG_DEBUG(TagEffectFxChorusMode::SetParametersMode01, tagIndex, paramIndex, "TagEffectFxChorusMode" );
        if( !message.checkTargetData(mode01) ) {
            message.setStatus( yaxp::MessageT::illegalData );
            return false;
        }

        if( !message.setTargetData(mode01) ) {
            message.setStatus( yaxp::MessageT::illegalDataLength );
            return false;
        }
        // test data -- to controller
        mode01.deltaPhaseIndex.setInnerValue( 0x8000000 );
        mode01.phaseDiffIndex.setInnerValue( 0x40000000 ); // cos
        // --------------
        message.setStatusSetOk();
        return true;
    }
    message.setStatus( yaxp::MessageT::illegalTag );
    return false;
}

void FxChorus::clearState()
{
    // out().clear();    
}

bool FxChorus::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    // 1st tag is tag effect type
    const uint8_t tagType = message.getTag(tagIndex);
    if( uint8_t(param.type) != tagType ) {
        message.setStatus( yaxp::MessageT::illegalTagEffectType );
        TAG_DEBUG(yaxp::MessageT::illegalTagEffectType, uint8_t(param.type), tagType, "FxChorus" );
        return false;
    }

    // 2nd tag is tag operation
    const uint8_t tag = message.getTag(++tagIndex);
    if( uint8_t(TagEffectFxChorusMode::Clear) == tag ) {
        clearState(); // this must be called to cleanup
    }
    // forward to param
    return param.parameter( message, tagIndex, paramIndex );
 }

bool FxChorus::connect( const FxBase * v, uint16_t ind )
{
    doConnect(v,ind);
};

void FxChorus::sprocess_01( void * thp )
{
    static_cast< MyType * >(thp)->process_01();
}

void FxChorus::sprocess_02( void * thp )
{
    static_cast< MyType * >(thp)->process_02_testTriangle();
}

bool FxChorus::setSprocessNext( uint16_t mode ) 
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

} // end namespace yacynth



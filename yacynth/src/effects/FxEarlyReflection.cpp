/*
 * Copyright (C) 2017 Istvan Simon
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
 * File:   FxEarlyReflection.cpp
 * Author: Istvan Simon
 *
 * Created on April 17, 2017, 11:22 AM
 */
#include "yacynth_config.h"
#include "effects/FxEarlyReflection.h"
#include <iomanip>
#include <iostream>

namespace yacynth {

bool FxEarlyReflectionParam::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    const uint8_t tag = message.getTag(tagIndex);
    switch( TagEffectFxEarlyReflectionMode( tag ) ) {
    case TagEffectFxEarlyReflectionMode::Clear :
        message.setStatusSetOk();
        TAG_DEBUG(TagEffectFxEarlyReflectionMode::Clear, tagIndex, paramIndex, "TagEffectFxEarlyReflectionMode" );
        return true;

    case TagEffectFxEarlyReflectionMode::SetParametersMode01 :
        TAG_DEBUG(TagEffectFxEarlyReflectionMode::SetParametersMode01, tagIndex, paramIndex, "TagEffectFxEarlyReflectionMode" );
        if( !message.setTargetData(mode01) ) {
            message.setStatus( yaxp::MessageT::illegalDataLength );
            return false;
        }
        if( mode01.check() ) {
            message.setStatusSetOk();
            return true;
        }
        message.setStatus( yaxp::MessageT::illegalData );
        return false;

    default:    // suppress warning
        break;
    }
    message.setStatus( yaxp::MessageT::illegalTag );
    return false;
}

void FxEarlyReflection::clearState()
{
    // out().clear();
}

bool FxEarlyReflection::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    // 1st tag is tag effect type
    const uint8_t tagType = message.getTag(tagIndex);
    if( uint8_t(param.type) != tagType ) {
        message.setStatus( yaxp::MessageT::illegalTagEffectType );
        TAG_DEBUG(yaxp::MessageT::illegalTagEffectType, uint8_t(param.type), tagType, "FxEarlyReflection" );
        return false;
    }
    // 2nd tag is tag operation
    const uint8_t tag = message.getTag(++tagIndex);
    switch( TagEffectFxEarlyReflectionMode( tag ) ) {
    case TagEffectFxEarlyReflectionMode::ClearState:
        clearState(); // this must be called to cleanup
        message.setStatusSetOk();
        return true;

    case TagEffectFxEarlyReflectionMode::Clear:
        clearState(); // this must be called to cleanup
        break;
    default:    // supress warning
        break;
    }
    // forward to param
    return param.parameter( message, tagIndex, paramIndex );
 }

bool FxEarlyReflection::connect( const FxBase * v, uint16_t ind )
{
    return doConnect(v,ind);
};


void FxEarlyReflection::sprocess_01( void * thp )
{
    static_cast< MyType * >(thp)->processNonModulated();
}

void FxEarlyReflection::sprocess_02( void * thp )
{
    static_cast< MyType * >(thp)->processModulated();
}

void FxEarlyReflection::sprocess_03( void * thp )
{
    static_cast< MyType * >(thp)->processNonModulated();
    static_cast< MyType * >(thp)->processSlave();
}

void FxEarlyReflection::sprocess_04( void * thp )
{
    static_cast< MyType * >(thp)->processModulated();
    static_cast< MyType * >(thp)->processSlave();
}

bool FxEarlyReflection::setSprocessNext( uint16_t mode )
{
    switch( mode ) {
    case 0:
        procMode = 0;
        sprocesspNext = FxEarlyReflection::sprocessClear2Nop;
        sprocessp = FxBase::sprocessFadeOut;
        return true;
    case 1:
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
    default:
        return false;
    }
    bool fadeIn = 0 == procMode;
    procMode = mode;
    if( fadeIn ) {
        sprocesspCurr = sprocesspNext;
        sprocessp = FxEarlyReflection::sprocessFadeIn;
        return true;
    }
    sprocessp = FxEarlyReflection::sprocessCrossFade;
    return true;
}

// --------------------------------------------------------------------
// TODO : clear fadein fadeout the slaves

void FxEarlyReflection::sprocessClear2Nop( void * data )
{
    FxEarlyReflection& thp = * static_cast<FxEarlyReflection *> ( data );
    thp.sprocessp = FxBase::sprocessNop;
    thp.EIObuffer::clear();
// TODO : clear fadein fadeout the slaves

}
// --------------------------------------------------------------------
void FxEarlyReflection::sprocessFadeOut( void * data )
{
    FxEarlyReflection& thp = * static_cast<FxEarlyReflection *> ( data );
    thp.sprocessp = FxEarlyReflection::sprocessClear2Nop;
    thp.sprocesspCurr( data );
    thp.EIObuffer::fadeOut();
// TODO : clear fadein fadeout the slaves
}
// --------------------------------------------------------------------
void FxEarlyReflection::sprocessFadeIn( void * data )
{
    FxEarlyReflection& thp = * static_cast<FxEarlyReflection *> ( data ) ;
    thp.clearState(); // clears the internal state but not the output - new mode starts
// TODO : clear fadein fadeout the slaves
    SpfT sprocessX( thp.sprocesspNext );
    thp.sprocessp = sprocessX;
    thp.sprocesspCurr = sprocessX;
    sprocessX( data );
    thp.EIObuffer::fadeIn();
// TODO : clear fadein fadeout the slaves
}
// --------------------------------------------------------------------
// not real cross fade at the moment but fade out - fade in
void FxEarlyReflection::sprocessCrossFade( void * data )
{
    FxEarlyReflection& thp = * static_cast<FxEarlyReflection *> ( data );
    thp.sprocessp = FxEarlyReflection::sprocessFadeIn;
    thp.sprocesspCurr( data );
    thp.EIObuffer::fadeOut();
// TODO : clear fadein fadeout the slaves
}

} // end yacynth
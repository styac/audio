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
 * File:   FxOutOscillator.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on June 23, 2016, 7:46 PM
 */
#include "yacynth_config.h"

#include "FxOutOscillator.h"
#include "yacynth_globals.h"

namespace yacynth {
using namespace TagEffectFxOutOscillatorModeLevel_03;

const char slavename[]  = " slave";

bool FxOutOscillatorParam::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    const uint8_t tag = message.getTag(tagIndex);
    switch( TagEffectFxOutOscillatorMode( tag ) ) {
    case TagEffectFxOutOscillatorMode::Clear:
        TAG_DEBUG(TagEffectFxOutOscillatorMode::Clear, tagIndex, paramIndex, "TagEffectFxOutOscillatorMode" );
        message.setStatusSetOk();
        return true;

    case TagEffectFxOutOscillatorMode::SetParametersMode01 :
        TAG_DEBUG(TagEffectFxOutOscillatorMode::SetParametersMode01, tagIndex, paramIndex, "TagEffectFxOutOscillatorMode" );
        if( !message.setTargetData(*this) ) {
            message.setStatus( yaxp::MessageT::illegalDataLength );
            return false;
        }
        if( ! mode01.freqMapper.check() ) {
            message.setStatus( yaxp::MessageT::dataCheckError );
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

void FxOutOscillator::clearState()
{
    // out().clear();        
}

bool FxOutOscillator::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    // 1st tag is tag effect type
    const uint8_t tagType = message.getTag(tagIndex);
    if( uint8_t(param.type) != tagType ) {
        message.setStatus( yaxp::MessageT::illegalTagEffectType );
        TAG_DEBUG(yaxp::MessageT::illegalTagEffectType, uint8_t(param.type), tagType, "FxOutOscillator" );
        return false;
    }
    // 2nd tag is tag operation
    const uint8_t tag = message.getTag(++tagIndex);
    switch( TagEffectFxOutOscillatorMode( tag ) ) {
    case TagEffectFxOutOscillatorMode::ClearState:
        clearState(); // this must be called to cleanup
        message.setStatusSetOk();
        return true;
        
    case TagEffectFxOutOscillatorMode::Clear:
        clearState(); // this must be called to cleanup
        break;
    default:
        break;        
        
    }
    // forward to param
    return param.parameter( message, tagIndex, paramIndex );
 }

bool FxOutOscillator::connect( const FxBase * v, uint16_t ind )
{
    return doConnect(v,ind);
};

void FxOutOscillator::sprocess_01( void * thp )
{
    static_cast< MyType * >(thp)->updateParamPhaseDiff();
    static_cast< MyType * >(thp)->processSine();
}

void FxOutOscillator::sprocess_02( void * thp )
{
    static_cast< MyType * >(thp)->updateParamFreqDiff();
    static_cast< MyType * >(thp)->processSine();
}


#if 0
void FxOutOscillator::sprocess_02( void * thp )
{
    static_cast< MyType * >(thp)->updateParamPhaseDiff();
    static_cast< MyType * >(thp)->processSinePd0();
}
#endif
void FxOutOscillator::sprocess_03( void * thp )
{
    static_cast< MyType * >(thp)->updateParamPhaseDiff();
    static_cast< MyType * >(thp)->processSinePd1();
}

void FxOutOscillator::sprocess_04( void * thp )
{
    static_cast< MyType * >(thp)->updateParamPhaseDiff();
    static_cast< MyType * >(thp)->processSinePd2();
}

void FxOutOscillator::sprocess_05( void * thp )
{
    static_cast< MyType * >(thp)->updateParamPhaseDiff();
    static_cast< MyType * >(thp)->processSinePd3();
}

void FxOutOscillator::sprocess_06( void * thp )
{
    static_cast< MyType * >(thp)->processTestDirac();
}

void FxOutOscillator::sprocess_07( void * thp )
{
    static_cast< MyType * >(thp)->processTestSinImpulse();
}

void FxOutOscillator::sprocess_08( void * thp )
{
    static_cast< MyType * >(thp)->processTestSine440d16();
}

void FxOutOscillator::sprocess_09( void * thp )
{
    static_cast< MyType * >(thp)->processTestSine440d4();
}

void FxOutOscillator::sprocess_10( void * thp )
{
    static_cast< MyType * >(thp)->processTestSine440();
}

void FxOutOscillator::sprocess_11( void * thp )
{
    static_cast< MyType * >(thp)->processTestSine440m4();
}

void FxOutOscillator::sprocess_12( void * thp )
{
    static_cast< MyType * >(thp)->processTestSine440m16();
}

void FxOutOscillator::sprocess_13( void * thp )
{
    static_cast< MyType * >(thp)->processTestSine20000();
}


bool FxOutOscillator::setSprocessNext( uint16_t mode ) 
{
    switch( mode ) {
    case 0:
        procMode = 0;
        sprocesspNext = FxOutOscillator::sprocessClear2Nop;  // TODO : FxOutOscillator
        sprocessp = FxOutOscillator::sprocessFadeOut;        // TODO : FxOutOscillator   
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
    case 5:
        sprocesspNext = sprocess_05;
        break;
    case 6:
        sprocesspNext = sprocess_06;
        break;
    case 7:
        sprocesspNext = sprocess_07;
        break;
    case 8:
        sprocesspNext = sprocess_08;
        break;
    case 9:
        sprocesspNext = sprocess_09;
        break;
    case 10:
        sprocesspNext = sprocess_10;
        break;
    case 11:
        sprocesspNext = sprocess_11;
        break;
    case 12:
        sprocesspNext = sprocess_12;
        break;
    case 13:
        sprocesspNext = sprocess_13;
        break;
    default:
        return false;
    }
    
    bool fadeIn = 0 == procMode;
    procMode = mode;
    if( fadeIn ) {
        sprocesspCurr = sprocesspNext;
        sprocessp = FxOutOscillator::sprocessFadeIn;
        return true;
    }
    sprocessp = FxOutOscillator::sprocessCrossFade; 
    return true;
}
// --------------------------------------------------------------------

// TODO : clear fadein fadeout the slaves
void FxOutOscillator::sprocessClear2Nop( void * data )
{
    FxOutOscillator& thp = * static_cast<FxOutOscillator *> ( data );
    thp.sprocessp = FxBase::sprocessNop;
    thp.EIObuffer::clear();
// TODO : clear fadein fadeout the slaves
}
// --------------------------------------------------------------------
void FxOutOscillator::sprocessFadeOut( void * data )
{
    FxOutOscillator& thp = * static_cast<FxOutOscillator *> ( data );
    thp.sprocessp = FxOutOscillator::sprocessClear2Nop;
    thp.sprocesspCurr( data );
    thp.EIObuffer::fadeOut();
// TODO : clear fadein fadeout the slaves
}
// --------------------------------------------------------------------
void FxOutOscillator::sprocessFadeIn( void * data )
{
    FxOutOscillator& thp = * static_cast<FxOutOscillator *> ( data ) ;
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
void FxOutOscillator::sprocessCrossFade( void * data )
{
    FxOutOscillator& thp = * static_cast<FxOutOscillator *> ( data );
    thp.sprocessp = FxOutOscillator::sprocessFadeIn;
    thp.sprocesspCurr( data );
    thp.EIObuffer::fadeOut();
// TODO : clear fadein fadeout the slaves
}

} // end namespace yacynth



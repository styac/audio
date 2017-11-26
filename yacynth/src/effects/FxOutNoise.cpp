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
    case 14:
        sprocesspNext = sprocess_14;
        break;
    case 15:
        sprocesspNext = sprocess_15;
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

void FxOutNoise::sprocess_01( void * thp )
{
    constexpr float gain = 1.0f/(1<<23);
//    const float gain = static_cast< MyType * >(thp)->param.gains[ 1 - 1 ];
    static_cast< MyType * >(thp)->fillWhite();
    static_cast< MyType * >(thp)->mult(gain);
}
void FxOutNoise::sprocess_02( void * thp )
{
    constexpr float gain = 1.0f/(1<<23);
//    const float gain = static_cast< MyType * >(thp)->param.gains[ 2 - 1 ];
    static_cast< MyType * >(thp)->fillWhiteStereo();
    static_cast< MyType * >(thp)->mult(gain);
}

void FxOutNoise::sprocess_03( void * thp )
{
//    constexpr float gain = 1.0f/(1<<26);
    const float gain = static_cast< MyType * >(thp)->param.gains[ 3 - 1 ];
    static_cast< MyType * >(thp)->fillWhiteLowCut();
    static_cast< MyType * >(thp)->mult(gain);
}
void FxOutNoise::sprocess_04( void * thp )
{
//    constexpr float gain = 1.0f/(1<<26);
    const float gain = static_cast< MyType * >(thp)->param.gains[ 4 - 1 ];
    static_cast< MyType * >(thp)->fillBlue();
    static_cast< MyType * >(thp)->mult(gain);
}
void FxOutNoise::sprocess_05( void * thp )
{
//    constexpr float gain = 1.0f/(1<<24);
    const float gain = static_cast< MyType * >(thp)->param.gains[ 5 - 1 ];
    static_cast< MyType * >(thp)->fillRed();
    static_cast< MyType * >(thp)->mult(gain);
}
void FxOutNoise::sprocess_06( void * thp )
{
//    constexpr float gain = 1.0f/(1<<21);
    const float gain = static_cast< MyType * >(thp)->param.gains[ 6 - 1 ];
    static_cast< MyType * >(thp)->fillPurple();
    static_cast< MyType * >(thp)->mult(gain);
}
void FxOutNoise::sprocess_07( void * thp )
{
//    constexpr float gain = 1.0f/(1<<27);
    const float gain = static_cast< MyType * >(thp)->param.gains[ 7 - 1 ];
    static_cast< MyType * >(thp)->fillPink();
    static_cast< MyType * >(thp)->mult(gain);
}
void FxOutNoise::sprocess_08( void * thp )
{
//    constexpr float gain = 1.0f/(1<<26);
    const float gain = static_cast< MyType * >(thp)->param.gains[ 8 - 1 ];
    static_cast< MyType * >(thp)->fillPinkLow();
    static_cast< MyType * >(thp)->mult(gain);
}
void FxOutNoise::sprocess_09( void * thp )
{
//    constexpr float gain = 1.0f/(1<<21);
    const float gain = static_cast< MyType * >(thp)->param.gains[ 9 - 1 ];
    static_cast< MyType * >(thp)->fillPurpleVar( static_cast< MyType * >(thp)->param.purplePole );
    static_cast< MyType * >(thp)->mult(gain);
}
void FxOutNoise::sprocess_10( void * thp )
{
//        constexpr float gain = 1.0f/(1<<24);
    const float gain = static_cast< MyType * >(thp)->param.gains[ 10 - 1 ];
    static_cast< MyType * >(thp)->fillRedVar( static_cast< MyType * >(thp)->param.redPole );
    static_cast< MyType * >(thp)->mult(gain);
}

void FxOutNoise::sprocess_11( void * thp )
{
    constexpr float gain = 1.0f/(1<<2);
//    const float gain = static_cast< MyType * >(thp)->param.gains[ 10 - 1 ];
    static_cast< MyType * >(thp)->fillVelvet<2>();
    static_cast< MyType * >(thp)->mult(gain);
}

void FxOutNoise::sprocess_12( void * thp )
{
    constexpr float gain = 1.0f/(1<<2);
//    const float gain = static_cast< MyType * >(thp)->param.gains[ 10 - 1 ];
    static_cast< MyType * >(thp)->fillVelvet<3>();
    static_cast< MyType * >(thp)->mult(gain);
}

void FxOutNoise::sprocess_13( void * thp )
{
    constexpr float gain = 1.0f/(1<<2);
//    const float gain = static_cast< MyType * >(thp)->param.gains[ 10 - 1 ];
    static_cast< MyType * >(thp)->fillVelvet2CH<3>();
    static_cast< MyType * >(thp)->mult(gain);
}

void FxOutNoise::sprocess_14( void * thp )
{
    constexpr float gain = 1.0f/(1<<2);
//    const float gain = static_cast< MyType * >(thp)->param.gains[ 10 - 1 ];
    static_cast< MyType * >(thp)->fillVelvetTriangle();
    static_cast< MyType * >(thp)->mult(gain);
}

void FxOutNoise::sprocess_15( void * thp )
{
//    constexpr float gain = 1.0f/(1<<2);
//    const float gain = static_cast< MyType * >(thp)->param.gains[ 10 - 1 ];
    static_cast< MyType * >(thp)->fillVelvetTriangle2CH();
//    static_cast< MyType * >(thp)->mult(gain);
}

// --------------------------------------------------------------------
} // end namespace yacynth
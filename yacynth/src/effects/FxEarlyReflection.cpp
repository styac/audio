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

#include    "../effects/FxEarlyReflection.h"
#include    <iomanip>
#include    <iostream>

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

    }
    message.setStatus( yaxp::MessageT::illegalTag );
    return false;
}

void FxEarlyReflection::clearTransient()
{
    EIObuffer::clear();
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
    if( uint8_t(TagEffectFxEarlyReflectionMode::Clear) == tag ) {
        clearTransient(); // this must be called to cleanup
    }
    // forward to param
    return param.parameter( message, tagIndex, paramIndex );
 }

bool FxEarlyReflection::connect( const FxBase * v, uint16_t ind )
{
    doConnect(v,ind);
};

void FxEarlyReflection::sprocessTransient( void * thp )
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
void FxEarlyReflection::sprocess_00( void * thp )
{
    static_cast< MyType * >(thp)->clear();
}

void FxEarlyReflection::sprocess_01( void * thp )
{
    static_cast< MyType * >(thp)->process_01_simple();
}

void FxEarlyReflection::sprocess_02( void * thp )
{
    static_cast< MyType * >(thp)->process_02_modulated();
}

void FxEarlyReflection::sprocess_03( void * thp )
{
    static_cast< MyType * >(thp)->process_03_simple_noslave();
}

void FxEarlyReflection::sprocess_04( void * thp )
{
    static_cast< MyType * >(thp)->process_04_modulated_noslave();
}


// slave has no params
template<>
bool FxSlave<FxEarlyReflectionParam>::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    return false;
};
template<>
void FxSlave<FxEarlyReflectionParam>::clearTransient()
{
};

// simple genrator for coeffs

void generator_FxEarlyReflectionParam( float gain, float modampl, float decay )
{
    // make sine modulation
    float deltafi = PI2 / FxEarlyReflectionParam::coeffSetCount;
    std::cout << std::fixed  << std::setprecision( 7 ) << std::endl;
    std::cout << "// generated by generator_FxEarlyReflectionParam - gain: "
            << gain
            << " mod ampl " << modampl
            << " decay " << decay
            << std::endl;

    for( int ci=0; ci<FxEarlyReflectionParam::coeffSetCount; ++ci ) {
        float fi = deltafi * ci;
        float am0 = ( 1.0f - modampl * std::sin(fi) ) * gain;
        float am1 = ( 1.0f - modampl * std::cos(fi) ) * gain;

        std::cout << "// ----  " <<  ci << std::endl;

        for( int ti=0; ti<FxEarlyReflectionParam::tapCount; ++ti ) {
            // exponential decay
            am0 -= am0 * decay;
            am1 -= am1 * decay;
            std::cout
                << "    "   << am0
                << ", "     << am1
                << ","
                << std::endl;
        }
        std::cout << std::endl;
    }
}


} // end yacynth
#pragma once

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
 * File:   FxFlanger.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on June 23, 2016, 7:46 PM
 */

#include "effects/FxFlangerParam.h"
#include "utils/Fastsincos.h"
#include "oscillator/Tables.h"
#include "utils/Limiters.h"
#include "effects/DelayTap.h"
#include "oscillator/NoiseSample.h"
#include "effects/FxBase.h"

using namespace tables;

namespace yacynth {

class FxFlanger : public Fx<FxFlangerParam>  {
public:
    static constexpr std::size_t modulatorNorm  = 19;

    using MyType = FxFlanger;
    FxFlanger()
    :   Fx<FxFlangerParam>()
    ,   delay(FxFlangerParam::delayLngExp - effectFrameSizeExp)
    {
    }

    virtual bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    virtual bool setSprocessNext( uint16_t mode ) override;

    virtual bool connect( const FxBase * v, uint16_t ind ) override;

private:
    virtual void clearState() override;

    static void sprocess_01( void * thp );
    static void sprocess_02( void * thp );
    static void sprocess_03( void * thp );
    static void sprocess_04( void * thp );
    static void sprocess_05( void * thp );
    static void sprocess_06( void * thp );

    inline void processForward()
    {
        out().mult( inp(), param.mode01.gain );
        delay.pushSection( inp() );
        for( auto si = 0u; si < EbufferPar::sectionSize; ++si ) {
            const int64_t si64 = int64_t( EbufferPar::sectionSize - si ) << 32;
            // always positive - negative makes no sense
            const int64_t delayA = modulatorValue[ chA ].getInc() + si64;
            const int64_t delayB = modulatorValue[ chB ].getInc() + si64;
            out().channel[ chA ][ si ] += delay.getInterpolated2Order<chA>( uint64_t(delayA) ) * param.mode01.wetGain;
            out().channel[ chB ][ si ] += delay.getInterpolated2Order<chB>( uint64_t(delayB) ) * param.mode01.wetGain;
        }
    }

    // TODO check middle point -- 
    inline void processFeedback()
    {
        out().mult( inp(), param.mode01.gain );
        delay.pushSection( inp() );
        for( auto si = 0u; si < EbufferPar::sectionSize; ++si ) {
            const int64_t si64 = int64_t( EbufferPar::sectionSize - si ) << 32;
            // always positive - negative makes no sense
            const int64_t delayA = modulatorValue[ chA ].getInc() + si64;
            const int64_t delayB = modulatorValue[ chB ].getInc() + si64;
            out().channel[ chA ][ si ] += delay.getInterpolated2Order<chA>( uint64_t(delayA) ) * param.mode01.wetGain;
            out().channel[ chB ][ si ] += delay.getInterpolated2Order<chB>( uint64_t(delayB) ) * param.mode01.wetGain;
        }
        delay.feedbackSection( middleDelay, param.mode01.feedbackGain );  // NON FRACTIONAL !!
    }
    
#if 0
    inline void processFeedbackX()
    {
        const uint32_t startIndex = delay.getSectionIndex();
        out().mult( inp(), param.mode01.gain );
        delay.pushSection( inp() );
        for( int64_t si = 0; si < EbufferPar::sectionSize; ++si ) {
            const int64_t si64 = int64_t( EbufferPar::sectionSize - si ) << 32;
            const int64_t delayA = modulatorValue[ chA ].getInc() + si64;
            const int64_t delayB = modulatorValue[ chB ].getInc() + si64;
            const float vA = delay.getInterpolated2Order<chA>( delayA );
            const float vB = delay.getInterpolated2Order<chB>( delayB );
            out().channel[ chA ][ si ] += vA * param.mode01.wetGain;
            out().channel[ chB ][ si ] += vB * param.mode01.wetGain;
            delay.channel[ chA ][ startIndex + si ] -= vA * param.mode01.feedbackGain;
            delay.channel[ chB ][ startIndex + si ] -= vB * param.mode01.feedbackGain;
        }
    }
#endif
    
    inline void processVibrato()
    {
        delay.pushSection( inp() );        
        for( auto si = 0u; si < EbufferPar::sectionSize; ++si ) {
            const int64_t si64 = int64_t( EbufferPar::sectionSize - si ) << 32;
            const int64_t delayA = modulatorValue[ chA ].getInc() + si64;
            const int64_t delayB = modulatorValue[ chB ].getInc() + si64;
            out().channel[ chA ][ si ] = delay.getInterpolated2Order<chA>( delayA );
            out().channel[ chB ][ si ] = delay.getInterpolated2Order<chB>( delayB );
        }
    }
        
    inline void modulateSine()
    {
        constexpr uint8_t maxAmplExp = 16+31-32; // sine 16 bit, multiplier 31 bit signed fractional part 32
        constexpr uint8_t corrAmplExp = maxAmplExp - param.delayLngExp;
        const int64_t depth = param.mode01.depth;
        middleDelay = ( depth >> ( corrAmplExp + 16 + 1));
        modulatorValue[ chA ].set((( param.mode01.oscMasterIndex.getLfoSinI16() + 0x7FFFU ) * depth ) >> corrAmplExp );
        modulatorValue[ chB ].set((( param.mode01.oscSlaveIndex.getLfoSinI16()  + 0x7FFFU ) * depth ) >> corrAmplExp );
    }
    
    inline void modulateTriangle()
    {
        constexpr uint8_t maxAmplExp = 30+31-32; // triangle 30 bit, multiplier 31 bit signed fractional part 32
        constexpr uint8_t corrAmplExp = maxAmplExp - param.delayLngExp;
        const int64_t depth = param.mode01.depth; 
        middleDelay = ( depth >> ( corrAmplExp + 30 + 1));
        modulatorValue[ chA ].set(( param.mode01.oscMasterIndex.getLfoTriangleU32() * depth ) >> corrAmplExp );
        modulatorValue[ chB ].set(( param.mode01.oscSlaveIndex.getLfoTriangleU32()  * depth ) >> corrAmplExp );
    }

    EDelayLine  delay;
    ControllerLinearIterator<int64_t,EbufferPar::sectionSizeExp> modulatorValue[ 2 ];
    uint32_t middleDelay; // NON FRACTIONAL !!
};


} // end namespace yacynth


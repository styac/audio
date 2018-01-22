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
 * File:   FxChorus.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on June 23, 2016, 7:46 PM
 */

#include    "utils/Fastsincos.h"
#include    "oscillator/Tables.h"
#include    "utils/Limiters.h"
#include    "effects/DelayTap.h"
#include    "oscillator/NoiseSample.h"
#include    "effects/FxChorusParam.h"
#include    "effects/FxBase.h"

using namespace tables;

namespace yacynth {

class FxChorus : public Fx<FxChorusParam>  {
public:
    using MyType = FxChorus;
    static constexpr uint8_t poleExp = 9;
    FxChorus()
    :   Fx<FxChorusParam>()
    ,   delay(FxChorusParam::delayLngExp - effectFrameSizeExp)   // 8192 * 64 sample - 10 sec
    {
    }

    virtual bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    virtual bool setSprocessNext( uint16_t mode ) override;

    virtual bool connect( const FxBase * v, uint16_t ind ) override;

private:
    virtual void clearState(void) override;
    static void sprocess_01( void * thp );
    static void sprocess_02( void * thp );
    static void sprocess_05( void * thp );
//
// http://web.arch.usyd.edu.au/~wmar0109/DESC9115/DAFx_chapters/03.PDF
//
    // modulation signal = sine + noise

    inline void processChorus()
    {
        delay.pushSection( inp() );
        out().clear();
        for( auto ti = 0u; ti < param.mode01.tapCount; ++ti ) {
            for( auto si = 0u; si < EbufferPar::sectionSize; ++si ) {
                const int64_t si64 = int64_t( EbufferPar::sectionSize - si ) << 32;
                // always positive - negative makes no sense
                const int64_t delayA = modulatorValue[ ti ][ chA ].getInc() + si64;
                const int64_t delayB = modulatorValue[ ti ][ chB ].getInc() + si64;
                out().channel[ chA ][ si ] += delay.getInterpolated2Order<chA>( uint64_t(delayA) );
                out().channel[ chB ][ si ] += delay.getInterpolated2Order<chB>( uint64_t(delayB) );
            }
        }
        out().multAdd( param.mode01.wetGain, inp() ); // mult
    }

    inline void modulateSine(void)
    {
        constexpr uint8_t maxAmplExp = 16+31-32; // sine 16 bit, multiplier 31 bit signed fractional part 32
        constexpr uint8_t corrAmplExp = maxAmplExp - param.delayLngExp + 1;
        for( auto ti = 0u; ti < param.mode01.tapCount; ++ti ) {
            const int64_t depthA = param.mode01.depth[ ti ][ chA ];
            const int64_t depthB = param.mode01.depth[ ti ][ chB ];
            modulatorValue[ ti ][ chA ].set( param.mode01.baseDelay[ ti ][ chA ] +
                ((( param.mode01.oscMasterIndex.getLfoSinI16() + 0x7FFFU ) * depthA ) >> corrAmplExp ));
            modulatorValue[ ti ][ chB ].set( param.mode01.baseDelay[ ti ][ chB ] + 
                ((( param.mode01.oscSlaveIndex.getLfoSinI16()  + 0x7FFFU ) * depthB ) >> corrAmplExp ));           
        }
    }

    inline void modulateTriangle(void)
    {
        constexpr uint8_t maxAmplExp = 30+31-32; // triangle 30 bit, multiplier 31 bit signed fractional part 32
        constexpr uint8_t corrAmplExp = maxAmplExp - param.delayLngExp + 1;
        for( auto ti = 0u; ti < param.mode01.tapCount; ++ti ) {
            const int64_t depthA = param.mode01.depth[ ti ][ chA ];
            const int64_t depthB = param.mode01.depth[ ti ][ chB ];
            modulatorValue[ ti ][ chA ].set( param.mode01.baseDelay[ ti ][ chA ] +
                (( param.mode01.oscMasterIndex.getLfoTriangleU32() * depthA ) >> corrAmplExp ));
            modulatorValue[ ti ][ chB ].set( param.mode01.baseDelay[ ti ][ chB ] +
                (( param.mode01.oscSlaveIndex.getLfoTriangleU32()  * depthB ) >> corrAmplExp ));            
        }
    }

    inline void process_test()
    {
        for( auto si = 0u; si < EbufferPar::sectionSize; ++si ) {
            const int64_t delayA = modulatorValue[ 0 ][ chA ].getInc();
            const int64_t delayB = modulatorValue[ 0 ][ chB ].getInc();
            out().channel[ chA ][ si ] = delayA * (1.0f/(1LL<<(32+param.delayLngExp)));
            out().channel[ chB ][ si ] = delayB * (1.0f/(1LL<<(32+param.delayLngExp)));            
        }
    }

    EDelayLine  delay;
    // to iterate in the
    ControllerLinearIterator<int64_t,EbufferPar::sectionSizeExp>  modulatorValue[ FxChorusParam::tapSize ][ 2 ];
    
    // TODO consts 
    //int64_t delayOffsetSine = 0x7FFFU * param.mode01.sineDepth + param.mode01.baseDelay;
    //int64_t delayOffsetTriangle = param.mode01.baseDelay;

};


} // end namespace yacynth


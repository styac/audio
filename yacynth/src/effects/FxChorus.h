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

#include    "../utils/Fastsincos.h"
#include    "../oscillator/Tables.h"
#include    "../utils/Limiters.h"
#include    "../effects/DelayTap.h"
#include    "../oscillator/NoiseSample.h"
#include    "../effects/FxChorusParam.h"
#include    "../effects/FxBase.h"

using namespace tables;

namespace yacynth {

class FxChorus : public Fx<FxChorusParam>  {
public:
    using MyType = FxChorus;
    static constexpr uint8_t poleExp = 6;
    FxChorus()
    :   Fx<FxChorusParam>()
    ,   delay(FxChorusParam::delayLngExp - effectFrameSizeExp)   // 8192 * 64 sample - 10 sec
//    ,   noiseSample( GaloisShifterSingle<seedThreadEffect_noise>::getInstance() )
    ,   galoisShifter( GaloisShifterSingle<seedThreadEffect_noise>::getInstance() )
    {
    }

    virtual bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    virtual bool setSprocessNext( uint16_t mode ) override;

    virtual bool connect( const FxBase * v, uint16_t ind ) override;

private:
    virtual void clearState(void) override;
    static void sprocess_01( void * thp );
    static void sprocess_02( void * thp );
    static void sprocess_03( void * thp );
//
// http://web.arch.usyd.edu.au/~wmar0109/DESC9115/DAFx_chapters/03.PDF
//
    inline void process_test(void)
    {
        delay.pushSection( inp<0>() );
        
        const int64_t sineDepth = param.mode01.sineDepth; // 0..1<<32
        const int64_t noiseDepth = param.mode01.noiseDepth;
        const int64_t sineChA = param.mode01.oscMasterIndex.getLfoSinI16() * sineDepth;
        const int64_t sineChB = param.mode01.oscSlaveIndex.getLfoSinI16()  * sineDepth;

        const int64_t noiseChA = galoisShifter.getWhite24();
        const int64_t noiseChB = galoisShifter.getWhite24();

        modulatorValue[ 0 ][ chA ].set( sineChA + noiseChA * noiseDepth );
        modulatorValue[ 0 ][ chB ].set( sineChB + noiseChB * noiseDepth );
        for( int64_t si = 0; si < EbufferPar::sectionSize; ++si ) {
            const int64_t si64 = si<<32;
            const int64_t delayA = modulatorValue[ 0 ][ chA ].getInc();
            const int64_t delayB = modulatorValue[ 0 ][ chB ].getInc();
            pole[ 0 ][ chA ] += (delayA>>4) - ( pole[ 0 ][ chA ] >> poleExp );
            pole[ 0 ][ chB ] += (delayB>>4) - ( pole[ 0 ][ chB ] >> poleExp );
//            out().channel[ chA ][ si ] = delay.getInterpolated2Order<chA>( param.mode01.baseDelay + pole[ 0 ][ chA ] - si64 );
//            out().channel[ chB ][ si ] = delay.getInterpolated2Order<chB>( param.mode01.baseDelay + delayB - si64 );
//            out().channel[ chA ][ si ] = sineChA * (1.0f/(1LL<<32));
//            out().channel[ chB ][ si ] = inp().channel[ chA ][ si ];
            out().channel[ chA ][ si ] = ( pole[ 0 ][ chA ] )  * (1.0f/(1LL<<38));
            out().channel[ chB ][ si ] = ( pole[ 0 ][ chB ] )  * (1.0f/(1LL<<38));
        }
    }

    // modulation signal = sine + noise

    inline void processChorusSine(void)
    {
        delay.pushSection( inp() );
        out().clear();
        //out().copy( inp() );
        const int64_t sineRange = param.mode01.sineDepth;
        const int64_t noiseRange = param.mode01.noiseDepth;
        const int64_t sineChA = param.mode01.oscMasterIndex.getLfoSinI16() * sineRange;
        const int64_t sineChB = param.mode01.oscSlaveIndex.getLfoSinI16()  * sineRange;
        for( auto ti = 0u; ti < param.mode01.tapCount; ++ti ) {
            const int64_t noiseChA = galoisShifter.getWhite24();
            const int64_t noiseChB = galoisShifter.getWhite24();
            modulatorValue[ ti ][ chA ].set( sineChA + noiseChA * noiseRange );
            modulatorValue[ ti ][ chB ].set( sineChB + noiseChB * noiseRange );
            for( int64_t si = 0; si < EbufferPar::sectionSize; ++si ) {
                const int64_t si64 = si<<32;
                const int64_t delayA = modulatorValue[ ti ][ chA ].getInc();
                const int64_t delayB = modulatorValue[ ti ][ chB ].getInc();
                pole[ ti ][ chA ] += (delayA>>4) - ( pole[ ti ][ chA ] >> poleExp );
                pole[ ti ][ chB ] += (delayB>>4) - ( pole[ ti ][ chB ] >> poleExp );
                out().channel[ chA ][ si ] += delay.getInterpolated2Order<chA>( param.mode01.baseDelay + pole[ ti ][ chA ] - si64 );
                out().channel[ chB ][ si ] += delay.getInterpolated2Order<chB>( param.mode01.baseDelay + pole[ ti ][ chB ] - si64 );
            }
        }
        out().multAdd( param.mode01.wetgain, inp() ); // mult
    }

    // feedback from middle delay 
    // forward and back gain : 0.7071
    inline void processWhiteChorusSine(void)
    {
        const float fgain(0.7071)
        ;
        delay.pushSection( inp() );
        //out().clear();
        out().copy( inp() );
        const int64_t sineRange = param.mode01.sineDepth;
        const int64_t noiseRange = param.mode01.noiseDepth;
        const int64_t sineChA = param.mode01.oscMasterIndex.getLfoSinI16() * sineRange;
        const int64_t sineChB = param.mode01.oscSlaveIndex.getLfoSinI16()  * sineRange;
        for( auto ti = 0u; ti < param.mode01.tapCount; ++ti ) {
            const int64_t noiseChA = galoisShifter.getWhite24();
            const int64_t noiseChB = galoisShifter.getWhite24();
            modulatorValue[ ti ][ chA ].set( sineChA + noiseChA * noiseRange );
            modulatorValue[ ti ][ chB ].set( sineChB + noiseChB * noiseRange );
            for( int64_t si = 0; si < EbufferPar::sectionSize; ++si ) {
                const int64_t si64 = si<<32;
                const int64_t delayA = modulatorValue[ ti ][ chA ].getInc();
                const int64_t delayB = modulatorValue[ ti ][ chB ].getInc();
                pole[ ti ][ chA ] += (delayA>>4) - ( pole[ ti ][ chA ] >> poleExp );
                pole[ ti ][ chB ] += (delayB>>4) - ( pole[ ti ][ chB ] >> poleExp );
                out().channel[ chA ][ si ] += delay.getInterpolated2Order<chA>( param.mode01.baseDelay + pole[ ti ][ chA ] - si64 );
                out().channel[ chB ][ si ] += delay.getInterpolated2Order<chB>( param.mode01.baseDelay + pole[ ti ][ chB ] - si64 );
            }
        }
        out().multAdd( param.mode01.wetgain, inp<0>() );
    }
    
    // test with triangle clean modulation
    //
    inline int64_t getTriangle(void)
    {
        constexpr int level     = 20;
        const int64_t tmp = (int64_t(testPhase)>>level);
        return ((tmp>>(63-level)) ^ tmp ) - ((1ULL<<(62-level)));
    }

    inline void process_02_testTriangle(void)
    {
        constexpr uint64_t dphase =  (double(1ULL<<63) / 48000.0) * 2.0;
        constexpr uint64_t delayBase = 2048LL<<32;
        delay.pushSection( inp<0>() );

        // testing mode
        for( uint64_t si = 0u; si < EbufferPar::sectionSize; ++si ) {
            testPhase += dphase;
            //                        ffff fd64 02bb 0cf8
            const uint64_t delayDiff = (getTriangle()>>3)  - ( si<<32 );
            //out().channel[ chB ][ si ] = float(getTriangl()) * float(0.9 / (1ULL<<42));
            out().channel[ chA ][ si ] = delay.getInterpolated2Order<chA>( delayBase + delayDiff );
            out().channel[ chB ][ si ] = delay.getInterpolated1Order<chA>( delayBase + delayDiff );
            //out().channel[ chB ][ si ] = inp<0>().channel[ chA ][ si ];
        }
    }

    GaloisShifter&  galoisShifter;
    EDelayLine  delay;
    // to iterate in the
    ControllerLinearIterator<int64_t,EbufferPar::sectionSizeExp> modulatorValue[ FxChorusParam::tapSize ][ 2 ];
    int64_t pole[ FxChorusParam::tapSize ][ 2 ];
  //  NoiseSample2CH     noiseSample;

    // test mode helper variables
    uint64_t    testPhase;
    uint64_t    triangle;

};


} // end namespace yacynth


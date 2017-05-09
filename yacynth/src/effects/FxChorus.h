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


#include    "FxBase.h"
#include    "../utils/Fastsincos.h"
#include    "../oscillator/Tables.h"
#include    "../utils/Limiters.h"
#include    "../effects/DelayTap.h"
#include    "../oscillator/NoiseSample.h"


using namespace tables;

namespace yacynth {
using namespace TagEffectTypeLevel_02;

class FxChorusParam {
public:
    // mandatory fields
    static constexpr char const * const name        = "FxChorus";
    static constexpr TagEffectType  type            = TagEffectType::FxChorus;
    static constexpr std::size_t maxMode            = 2;
    static constexpr std::size_t inputCount         = 1;

    static constexpr std::size_t tapSize            = 4;
    static constexpr std::size_t delayLngExp        = 7;
    static constexpr std::size_t delayLng           = 1<<(delayLngExp+EbufferPar::sectionSizeExp);
    static constexpr std::size_t delayOffsMaxLng    = delayLng - 1;
    static constexpr std::size_t delayOffsMinLng    = EbufferPar::sectionSize * 2;

    static constexpr uint64_t minBaseDelay          = 256LL << 32;  // ca 6 msec
    static constexpr uint64_t maxBaseDelay          = 4000LL << 32; // ca 80 msec

    bool parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    //
    // delay center value
    // mod freq controller
    // mod depth controller
    // osc mode > sin, rand
    // oscillator: base sine (16 bit) + red noise (24 bit)
    // osc << 16+n + noise << 7
    //
    struct Mode01 {
        bool check()
        {
            static_assert( delayLng >= (maxBaseDelay>>32)+64, "illegal delay length" );

            if( baseDelay < minBaseDelay ) {
                baseDelay = minBaseDelay;
            }
            if( baseDelay > maxBaseDelay ) {
                baseDelay = maxBaseDelay;
            }
            if( baseDelay < (256LL<<32)) {
                return false;
            }

            if( wetgain < -1.0 ) {
                wetgain = -1.0;
            }

            if( wetgain > 1.0 ) {
                wetgain = 1.0;
            }
            wetgain /= tapCount;
            return true;
        }
        uint64_t        baseDelay;          // chorus tap delay
        float           wetgain;
        int32_t         sineRange;       // iterator multiplier -- rename depth - controller
        int32_t         noiseRange;       // iterator multiplier -- rename depth - controller
        // manual
        ControllerIndex depthIndex;         // to set the mod depth
        ControllerIndex deltaPhaseIndex;    // to set the freq
        ControllerIndex phaseDiffIndex;     // to set the phase diff A-B

        // get the sine component of the modulation signal
        ControllerIndex oscMasterIndex;     // to get the osc phase chA
        ControllerIndex oscSlaveIndex;      // to get the osc phase chB

        // noise freq limit - pole for low cut
        uint8_t         basaeDepthNoiseExp; // noise amplitude - controller
        uint8_t         tapCount;
    } mode01;

};

class FxChorus : public Fx<FxChorusParam>  {
public:
    using MyType = FxChorus;
    FxChorus()
    :   Fx<FxChorusParam>()
    ,   delay(FxChorusParam::delayLngExp)   // 8192 * 64 sample - 10 sec
    ,   noiseSample( GaloisShifterSingle<seedThreadEffect_noise>::getInstance() )
    {
        fillSprocessv<0>(sprocess_00);
        fillSprocessv<1>(sprocess_01);
        fillSprocessv<2>(sprocess_02);
    }

    virtual bool parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );


    // go up to Fx ??
    // might change -> set sprocessTransient
    // FIRST TEST WITHOUT TRANSIENT
    // THEN  WITH TRANSIENT -> all types > out,
    // 00 is always clear for output or bypass for in-out == effect OFF
    bool setProcMode( uint16_t ind )  override
    {
        std::cout << "---- FxChorus setProcMode " << ind << std::endl;
        if( procMode == ind ) {
            return true; // no change
        }
        if( getMaxMode() < ind ) {
            return false; // illegal
        }
        if( 0 == procMode ) {
            fadePhase = FadePhase::FPH_fadeInSimple;
        } else if( 0 == ind ) {
            fadePhase = FadePhase::FPH_fadeOutSimple;
        } else {
            fadePhase = FadePhase::FPH_fadeOutCross;
        }

        procMode = ind;
        sprocessp = sprocesspSave = sprocessv[ind];
        // sprocesspSave = sprocessv[ind];
        // sprocessp = sprocessTransient;
        return true;
    }
#if 0
    // go up to Fx ?? virtual ?
    SpfT getProcMode( uint16_t ind ) const override
    {
        switch( ind ) {
        case 0:
            return sprocess_00;
        case 1:
            return sprocess_01;
        case 2:
            return sprocess_02;
        case 3:
            return sprocess_03;
        case 4:
            return sprocess_04;
        case 5:
            return sprocess_05;
        default:
            return sprocessp; // illegal index no change
        }
    }
#endif
    virtual bool connect( const FxBase * v, uint16_t ind ) override;

private:
    virtual void clearTransient(void) override;

    static void sprocessTransient( void * thp );

    static void sprocess_00( void * thp );  // bypass > inp<0> -> out
    static void sprocess_01( void * thp );
    static void sprocess_02( void * thp );
//
// http://web.arch.usyd.edu.au/~wmar0109/DESC9115/DAFx_chapters/03.PDF
//
    // modulation signal = sine + noise

    inline void process_01(void)
    {
        delay.pushSection( inp<0>() );
        out().clear();
        // sine : normalize to 28 bit
        const int64_t sineRange = param.mode01.sineRange;
        const int64_t noiseRange = param.mode01.noiseRange;
        const int64_t sineChA = ( param.mode01.oscMasterIndex.getLfoSinI16() << 12 ) * sineRange;
        const int64_t sineChB = ( param.mode01.oscSlaveIndex.getLfoSinI16() << 12 ) * sineRange;
        for( auto ti = 0u; ti < param.mode01.tapCount; ++ti ) {
            // noise : normalize to 28 bit
            // getAvg -- 25 bit noise
            const int64_t noiseChA = ( noiseSample.getRed() << 3 ) * noiseRange;
            const int64_t noiseChB = ( noiseSample.getRed() << 3 ) * noiseRange;
            // fill noiseFrame or noiseSample ????
            // add noiseFrame +

            modulatorValue[ ti ][ chA ].set( sineChA + noiseChA );
            modulatorValue[ ti ][ chB ].set( sineChB + noiseChB );
            for( int64_t si = 0; si < EbufferPar::sectionSize; ++si ) {
                const int64_t si64 = si<<32;
                // modulatorValue = 28 + 31 = 59 bit
                // 59-32 = 27
                // max 10 bit
                const int64_t delayA = modulatorValue[ ti ][ chA ].getInc() >> 17;
                const int64_t delayB = modulatorValue[ ti ][ chB ].getInc() >> 17;
                // out().channel[ chA ][ si ] = delayA * (1.0f/(1LL<<42));
                // out().channel[ chB ][ si ] = delayB * (1.0f/(1LL<<42));

                // float xx = ( ( modulatorValue[ ti ][ chA ].getInc() )) * ( 1.0f / (1LL<<44));
                //out().channel[ chA ][ si ] = xx;
                //out().channel[ chB ][ si ] = sineChA * (1.0f / (1LL<<44));
                out().channel[ chA ][ si ] += delay.getInterpolated2Order<chA>( param.mode01.baseDelay + delayA - si64 );
                out().channel[ chB ][ si ] += delay.getInterpolated2Order<chB>( param.mode01.baseDelay + delayB - si64 );
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


    EDelayLine  delay;
    // to iterate in the
    ControllerLinearIterator<int64_t,EbufferPar::sectionSizeExp> modulatorValue[FxChorusParam::tapSize][2];
    NoiseSample     noiseSample;

    // test mode helper variables
    uint64_t    testPhase;
    uint64_t    triangle;

};


} // end namespace yacynth


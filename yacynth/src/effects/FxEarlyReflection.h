#pragma once

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
 * File:   FxEarlyReflection.h
 * Author: Istvan Simon
 *
 * Created on April 17, 2017, 11:22 AM
 */

#include    "../effects/FxBase.h"

namespace yacynth {
using namespace TagEffectFxEarlyReflectionModeLevel_03;

class FxEarlyReflectionParam {
public:
    // mandatory fields
    static constexpr char const * const name        = "FxEarlyReflection";
    static constexpr TagEffectType  type            = TagEffectType::FxEarlyReflection;
    static constexpr std::size_t maxMode            = 4; // 0 is always exist> 0,1,2
    static constexpr std::size_t inputCount         = 1;
    static constexpr std::size_t slaveCount         = 1; // 0-base signal 1-modulation
    static constexpr char const * const slavename   = " ^EarlyReflectionDelayedOutput";
    // optional
    static constexpr std::size_t channelCount       = 2;
    static constexpr std::size_t tapCount           = 1<<4;
    static constexpr std::size_t tapCountMask       = tapCount-1;
    static constexpr std::size_t coeffSetCount      = 1<<4;
    static constexpr std::size_t coeffSetCountMask  = coeffSetCount-1;

    static constexpr std::size_t delayLngExp        = 9; // ca: 600 msec - 512*1.3
    static constexpr std::size_t delayLng           = 1<<(delayLngExp+EbufferPar::sectionSizeExp);
    static constexpr std::size_t delayOffsMaxLng    = delayLng - 1;
    static constexpr std::size_t delayOffsMinLng    = EbufferPar::sectionSize * 2;

    bool parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    // no Tap... type is used because of the modulation experiment
    
    struct Mode01 {
        // should return error code not bool
        bool check()
        {
            for( auto &v0 : delayLateReverb ) {
                if(( v0 > delayOffsMaxLng ) || ( v0 < delayOffsMinLng )) {
                    return false;
                }
            }
            for( auto &v0 : delaysEarlyreflection ) {
                for( auto &v1 : v0 ) {
                    if( ( v1 > delayOffsMaxLng ) || ( v1 < delayOffsMinLng )) {
                        return false;
                    }
                }
            }
            for( auto &v0 : coeffsEarlyreflection ) {
                for( auto &v1 : v0 ) {
                    for( auto &v2 : v1 ) {
                        if(( v2 > 1.0f ) || ( v2 < -1.0f )) {
                            return false;
                        }
                    }
                }
            }
            for( auto &v0 : modulatorPeriod ) {
                constexpr std::size_t maxPeriod = 2048 / coeffSetCount; // 2.5 sec if 1 period / set
                constexpr std::size_t minPeriod = 3;    // 3 period ca 4 msec * 16 - 64 msec - 10-15 Hz
                if(( v0 > maxPeriod ) || ( v0 < minPeriod )) { // irreal
                    return false;
                }
            }
            return true;
        }
        // left-right delay for late reverb
        // need a controller later !
        uint32_t    delayLateReverb[channelCount];
        // left-right delay for early reflections
        uint32_t    delaysEarlyreflection[tapCount][channelCount];
        // left-right for early reflections
        // coeffSetCount - rotating with low frequency modulation -simple case uses 0
        // there should be 1 period - deviance about 10%
        float       coeffsEarlyreflection[coeffSetCount][tapCount][channelCount];
        uint16_t    modulatorPeriod[tapCount];
    } mode01;
};

class FxEarlyReflection : public Fx<FxEarlyReflectionParam>  {
public:

    using MyType = FxEarlyReflection;
    FxEarlyReflection()
    :   Fx<FxEarlyReflectionParam>()
    ,   delayLine(FxEarlyReflectionParam::delayLngExp)
    ,   modulatorIndex{0}
    {
        for( auto& si : slaves ) si.setMasterId(id());

        fillSprocessv<0>(sprocess_00);
        fillSprocessv<1>(sprocess_01);
        fillSprocessv<2>(sprocess_02);
        fillSprocessv<3>(sprocess_03);
        fillSprocessv<4>(sprocess_04);
        //std::cout << "\n---- delayLine bufferSize " << std::dec << delayLine.bufferSize << std::endl;
    }

    virtual bool parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    // go up to Fx ??
    // might change -> set sprocessTransient
    // FIRST TEST WITHOUT TRANSIENT
    // THEN  WITH TRANSIENT -> all types > out,
    // 00 is always clear for output or bypass for in-out == effect OFF
    bool setProcMode( uint16_t ind )  override
    {
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

    virtual bool connect( const FxBase * v, uint16_t ind ) override;

private:
    virtual void clearTransient(void) override;

    static void sprocessTransient( void * thp );

    static void sprocess_00( void * thp );  // bypass > inp<0> -> out
    static void sprocess_01( void * thp );
    static void sprocess_02( void * thp );
    static void sprocess_03( void * thp );
    static void sprocess_04( void * thp );

    // simple: no coeff modulation
    inline void process_01_simple(void)
    {
        static_assert(FxEarlyReflectionParam::tapCount>1,"tap count must be greater then 1");
        delayLine.pushSection( inp<0>().channel[ chA ], inp<0>().channel[ chB ] );
        delayLine.fillTDLSection(
                param.mode01.delayLateReverb[ chA ],
                param.mode01.delayLateReverb[ chB ],
                slaves[ 0 ].out().channel[ chA ],
                slaves[ 0 ].out().channel[ chB ] );

        // first tap deletes
        delayLine.fillTDLSection(
                param.mode01.delaysEarlyreflection[ 0 ][ chA ],
                param.mode01.delaysEarlyreflection[ 0 ][ chB ],
                param.mode01.coeffsEarlyreflection[ 0 ][ 0 ][ chA ],
                param.mode01.coeffsEarlyreflection[ 0 ][ 0 ][ chB ],
                out().channel[chA],
                out().channel[chB] );

        // 1..n added
        for( uint32_t si=1; si<FxEarlyReflectionParam::tapCount; ++si ) {
            delayLine.addTDLSection(
                param.mode01.delaysEarlyreflection[ si ][ chA ],
                param.mode01.delaysEarlyreflection[ si ][ chB ],
                param.mode01.coeffsEarlyreflection[ 0 ][ si ][ chA ],
                param.mode01.coeffsEarlyreflection[ 0 ][ si ][ chB ],
                out().channel[ chA ],
                out().channel[ chB ] );
        }
    }

    // coeff modulation
    inline void process_02_modulated(void)
    {
        static_assert(param.tapCount>1,"tap count must be greater then 1");
        delayLine.pushSection( inp<0>().channel[ chA ], inp<0>().channel[ chB ] );
        delayLine.fillTDLSection(
                param.mode01.delayLateReverb[ chA ],
                param.mode01.delayLateReverb[ chB ],
                slaves[ 0 ].out().channel[ chA ],
                slaves[ 0 ].out().channel[ chB ] );

        incModulatorIndex();

        // first tap deletes
        delayLine.fillTDLSection(
                param.mode01.delaysEarlyreflection[ 0 ][ chA ],
                param.mode01.delaysEarlyreflection[ 0 ][ chB ],
                param.mode01.coeffsEarlyreflection[ modulatorIndex[ 0 ][ 0 ] ][ 0 ][ chA ],
                param.mode01.coeffsEarlyreflection[ modulatorIndex[ 0 ][ 1 ] ][ 0 ][ chB ],
                out().channel[chA],
                out().channel[chB] );

        // 1..n added
        for( uint32_t si=1; si<FxEarlyReflectionParam::tapCount; ++si ) {
            delayLine.addTDLSection(
                param.mode01.delaysEarlyreflection[ si ][ chA ],
                param.mode01.delaysEarlyreflection[ si ][ chB ],
                param.mode01.coeffsEarlyreflection[ modulatorIndex[ si ][ 0 ] ][ si ][ chA ],
                param.mode01.coeffsEarlyreflection[ modulatorIndex[ si ][ 1 ] ][ si ][ chB ],
                out().channel[ chA ],
                out().channel[ chB ] );
        }
    }

    // same as 01 but slave is not supplíed
    inline void process_03_simple_noslave(void)
    {
        static_assert(FxEarlyReflectionParam::tapCount>1,"tap count must be greater then 1");
        delayLine.pushSection( inp<0>().channel[ chA ], inp<0>().channel[ chB ] );

        // first tap deletes
        delayLine.fillTDLSection(
                param.mode01.delaysEarlyreflection[ 0 ][ chA ],
                param.mode01.delaysEarlyreflection[ 0 ][ chB ],
                param.mode01.coeffsEarlyreflection[ 0 ][ 0 ][ chA ],
                param.mode01.coeffsEarlyreflection[ 0 ][ 0 ][ chB ],
                out().channel[chA],
                out().channel[chB] );

        // 1..n added
        for( uint32_t si=1; si<FxEarlyReflectionParam::tapCount; ++si ) {
            delayLine.addTDLSection(
                param.mode01.delaysEarlyreflection[ si ][ chA ],
                param.mode01.delaysEarlyreflection[ si ][ chB ],
                param.mode01.coeffsEarlyreflection[ 0 ][ si ][ chA ],
                param.mode01.coeffsEarlyreflection[ 0 ][ si ][ chB ],
                out().channel[ chA ],
                out().channel[ chB ] );
        }
    }

    // // same as 02 but slave is not supplíed
    inline void process_04_modulated_noslave(void)
    {
        static_assert(param.tapCount>1,"tap count must be greater then 1");

        delayLine.pushSection( inp<0>().channel[ chA ], inp<0>().channel[ chB ] );

        incModulatorIndex();

        // first tap deletes
        delayLine.fillTDLSection(
                param.mode01.delaysEarlyreflection[ 0 ][ chA ],
                param.mode01.delaysEarlyreflection[ 0 ][ chB ],
                param.mode01.coeffsEarlyreflection[ modulatorIndex[ 0 ][ 0 ]  ][ 0 ][ chA ],
                param.mode01.coeffsEarlyreflection[ modulatorIndex[ 0 ][ 1 ]  ][ 0 ][ chB ],
                out().channel[chA],
                out().channel[chB] );

        // 1..n added
        for( uint32_t si=1; si<FxEarlyReflectionParam::tapCount; ++si ) {
            delayLine.addTDLSection(
                param.mode01.delaysEarlyreflection[ si ][ chA ],
                param.mode01.delaysEarlyreflection[ si ][ chB ],
                param.mode01.coeffsEarlyreflection[ modulatorIndex[ si ][ 0 ] ][ si ][ chA ],
                param.mode01.coeffsEarlyreflection[ modulatorIndex[ si ][ 1 ] ][ si ][ chB ],
                out().channel[ chA ],
                out().channel[ chB ] );
        }
    }

    void incModulatorIndex()
    {
        for( uint32_t si=1; si<FxEarlyReflectionParam::tapCount; ++si ) {
            switch( --modulatorIndexPhase[ si ] ) {
            case 0:
                modulatorIndexPhase[ si ] = param.mode01.modulatorPeriod[ si ];
                modulatorIndex[ si ][ 0 ] = ( ++modulatorIndex[ si ][ 0 ] ) & FxEarlyReflectionParam::coeffSetCountMask;
                return;
            case 2:
                // shift 2 section
                modulatorIndex[ si ][ 1 ] = modulatorIndex[ si ][ 0 ];
                return;
            }
        }
    }
    // slave instance
    FxSlave<FxEarlyReflectionParam>   slaves[ FxEarlyReflectionParam::slaveCount ];

    EDelayLine  delayLine;

    int16_t     modulatorIndexPhase[    FxEarlyReflectionParam::tapCount ];
    uint8_t     modulatorIndex[         FxEarlyReflectionParam::tapCount ][ 2 ];
};

// generator for test parameters

void generator_FxEarlyReflectionParam( float gain = 1.0f, float modampl = 0.1f, float decay = 0.2f );



} // end yacynth
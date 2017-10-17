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

#include    "../effects/FxEarlyReflectionParam.h"
#include    "../effects/FxBase.h"

namespace yacynth {

class FxEarlyReflection : public Fx<FxEarlyReflectionParam>  {
public:

    using MyType = FxEarlyReflection;
    FxEarlyReflection()
    :   Fx<FxEarlyReflectionParam>()
    ,   delayLine(FxEarlyReflectionParam::delayLngExp)
    ,   phase {0}
    ,   coeff {0}
    {
        for( auto& si : slaves ) si.setMasterId(id());
    }

    virtual bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    virtual bool setSprocessNext( uint16_t mode ) override;

    virtual bool connect( const FxBase * v, uint16_t ind ) override;

private:
    virtual void clearState(void) override;
    // do slaves also
    static void sprocessClear2Nop( void * );
    static void sprocessFadeOut( void * );
    static void sprocessFadeIn( void * );
    static void sprocessCrossFade( void * );

    static void sprocess_01( void * thp );
    static void sprocess_02( void * thp );
    static void sprocess_03( void * thp );
    static void sprocess_04( void * thp );

    // simple: no coeff modulation
    inline void process_01_simple(void)
    {
        static_assert(FxEarlyReflectionParam::tapCount>1,"tap count must be greater then 1");
        static_assert(FxEarlyReflectionParam::channelCount>1,"channel count must be greater then 1");
        delayLine.pushSection( inp<0>().channel[ chA ], inp<0>().channel[ chB ] );

        delayLine.fillTDLSection(
                param.mode01.lateReverb.delayIndex.v[ chA ],
                param.mode01.lateReverb.delayIndex.v[ chB ],
                slaves[ 0 ].out().channel[ chA ],
                slaves[ 0 ].out().channel[ chB ] );

        // first tap deletes
        delayLine.fillTDLSection(
                param.mode01.tap.delayIndex.v2[ 0 ][ chA ],
                param.mode01.tap.delayIndex.v2[ 0 ][ chB ],
                param.mode01.tap.coeff.v2[ 0 ][ chA ],
                param.mode01.tap.coeff.v2[ 0 ][ chB ],
                out().channel[ chA ],
                out().channel[ chB ] );

        // 1..n added
        for( uint32_t si=1; si<FxEarlyReflectionParam::tapCount; ++si ) {
            delayLine.addTDLSection(
                param.mode01.tap.delayIndex.v2[ si ][ chA ],
                param.mode01.tap.delayIndex.v2[ si ][ chB ],
                param.mode01.tap.coeff.v2[ si ][ chA ],
                param.mode01.tap.coeff.v2[ si ][ chB ],
                out().channel[ chA ],
                out().channel[ chB ] );
        }
    }

    // coeff modulation
    inline void process_02_modulated(void)
    {
        static_assert(FxEarlyReflectionParam::tapCount>1,"tap count must be greater then 1");
        static_assert(FxEarlyReflectionParam::channelCount>1,"channel count must be greater then 1");
        delayLine.pushSection( inp<0>().channel[ chA ], inp<0>().channel[ chB ] );

        stepAMcoeff();
        delayLine.fillTDLSection(
                param.mode01.lateReverb.delayIndex.v[ chA ],
                param.mode01.lateReverb.delayIndex.v[ chB ],
                slaves[ 0 ].out().channel[ chA ],
                slaves[ 0 ].out().channel[ chB ] );

        // first tap deletes
        delayLine.fillTDLSection(
                param.mode01.tap.delayIndex.v2[ 0 ][ chA ],
                param.mode01.tap.delayIndex.v2[ 0 ][ chB ],
                coeff.v2[ 0 ][ chA ],
                coeff.v2[ 0 ][ chB ],
                out().channel[ chA ],
                out().channel[ chB ] );

        // 1..n added
        for( uint32_t si=1; si<FxEarlyReflectionParam::tapCount; ++si ) {
            delayLine.addTDLSection(
                param.mode01.tap.delayIndex.v2[ si ][ chA ],
                param.mode01.tap.delayIndex.v2[ si ][ chB ],
                coeff.v2[ si ][ chA ],
                coeff.v2[ si ][ chB ],
                out().channel[ chA ],
                out().channel[ chB ] );
        }
    }

    // same as 01 but slave is not supplíed
    inline void process_03_simple_noslave(void)
    {
        static_assert(FxEarlyReflectionParam::tapCount>1,"tap count must be greater then 1");
        static_assert(FxEarlyReflectionParam::channelCount>1,"channel count must be greater then 1");
        delayLine.pushSection( inp<0>().channel[ chA ], inp<0>().channel[ chB ] );

        // first tap deletes
        delayLine.fillTDLSection(
                param.mode01.tap.delayIndex.v2[ 0 ][ chA ],
                param.mode01.tap.delayIndex.v2[ 0 ][ chB ],
                param.mode01.tap.coeff.v2[ 0 ][ chA ],
                param.mode01.tap.coeff.v2[ 0 ][ chB ],
                out().channel[ chA ],
                out().channel[ chB ] );

        // 1..n added
        for( uint32_t si=1; si < FxEarlyReflectionParam::tapCount; ++si ) {
            delayLine.addTDLSection(
                param.mode01.tap.delayIndex.v2[ si ][ chA ],
                param.mode01.tap.delayIndex.v2[ si ][ chB ],
                param.mode01.tap.coeff.v2[ si ][ chA ],
                param.mode01.tap.coeff.v2[ si ][ chB ],
                out().channel[ chA ],
                out().channel[ chB ] );
        }
//        out().copyCH<chB>(inp<0>());
    }

    // // same as 02 but slave is not supplíed
    inline void process_04_modulated_noslave(void)
    {
        static_assert(FxEarlyReflectionParam::tapCount>1,"tap count must be greater then 1");
        static_assert(FxEarlyReflectionParam::channelCount>1,"channel count must be greater then 1");
        delayLine.pushSection( inp<0>().channel[ chA ], inp<0>().channel[ chB ] );

        stepAMcoeff();
        // first tap deletes
        delayLine.fillTDLSection(
                param.mode01.tap.delayIndex.v2[ 0 ][ chA ],
                param.mode01.tap.delayIndex.v2[ 0 ][ chB ],
                coeff.v2[ 0 ][ chA ],
                coeff.v2[ 0 ][ chB ],
                out().channel[ chA ],
                out().channel[ chB ] );

        // 1..n added
        for( uint32_t si=1; si<FxEarlyReflectionParam::tapCount; ++si ) {
            delayLine.addTDLSection(
                param.mode01.tap.delayIndex.v2[ si ][ chA ],
                param.mode01.tap.delayIndex.v2[ si ][ chB ],
                coeff.v2[ si ][ chA ],
                coeff.v2[ si ][ chB ],
                out().channel[ chA ],
                out().channel[ chB ] );
        }
//        out().copyCH<chB>(inp<0>());
    }

    // to test
    void stepAMcoeff()
    {
#if 0
        constexpr std::size_t vsize = V4fMvec<FxEarlyReflectionParam::tapCount,   FxEarlyReflectionParam::channelCount>::size4;
        // TODO use V4
        for( auto vi = 0u; vi < size4; ++vi ) {
            // make triangle
            const v4si tmp = phase.v4[ vi ]>>1;
            const v4sf triangle = (v4sf) ((tmp>>30) ^ tmp );
            // mult int*float with v4 ?
            coeff.v4[ vi ] = param.mode01.tap.coeff.v4[ vi ] - triangle * param.mode01.tap.modDepth.v4[ vi ];
            phase.v4[ vi ] += param.mode01.tap.modDeltaPhase.v4[ vi ];
        }
#else
        constexpr std::size_t vsize = V4fMvec<FxEarlyReflectionParam::tapCount,   FxEarlyReflectionParam::channelCount>::size;
        for( auto vi = 0u; vi < vsize; ++vi ) {
            // make triangle
            const int32_t tmp = phase.v[ vi ]>>1;
            const float triangle = (float) ((tmp>>30) ^ tmp );
            // subtract: triangle -- decrease amplitude
            coeff.v[ vi ] = param.mode01.tap.coeff.v[ vi ] - triangle * param.mode01.tap.modDepth.v[ vi ];
            phase.v[ vi ] += param.mode01.tap.modDeltaPhase.v[ vi ];
        }
#endif
    }

    // new
    V4i32Mvec<FxEarlyReflectionParam::tapCount, FxEarlyReflectionParam::channelCount>   phase;
    V4fMvec<FxEarlyReflectionParam::tapCount,   FxEarlyReflectionParam::channelCount>   coeff;

    EDelayLine  delayLine;

    // slave instance
    FxSlave<FxEarlyReflectionParam>   slaves[ FxEarlyReflectionParam::slaveCount ];
};


} // end yacynth
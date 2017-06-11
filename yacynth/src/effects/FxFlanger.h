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


#include    "../effects/FxFlangerParam.h"
#include    "../utils/Fastsincos.h"
#include    "../oscillator/Tables.h"
#include    "../utils/Limiters.h"
#include    "../effects/DelayTap.h"
#include    "../oscillator/NoiseSample.h"
#include    "../effects/FxBase.h"

//     0.3 mS to 14.4 ms -- 0 .. 1024
// + feedback
// sine triangle-lin triangle-exp

using namespace tables;

// #define CHECK_DEBUG_FLANGER

namespace yacynth {

class FxFlanger : public Fx<FxFlangerParam>  {
public:
    static constexpr std::size_t modulatorNorm  = 19;

    using MyType = FxFlanger;
    FxFlanger()
    :   Fx<FxFlangerParam>()
    ,   delay(FxFlangerParam::delayLngExp)
    {
        fillSprocessv<0>(sprocess_00);
        fillSprocessv<1>(sprocess_01);
        fillSprocessv<2>(sprocess_02);
        fillSprocessv<3>(sprocess_03);
        fillSprocessv<4>(sprocess_04);
    }

    virtual bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );


    // go up to Fx ??
    // might change -> set sprocessTransient
    // FIRST TEST WITHOUT TRANSIENT
    // THEN  WITH TRANSIENT -> all types > out,
    // 00 is always clear for output or bypass for in-out == effect OFF
    bool setProcMode( uint16_t ind )  override
    {
        std::cout << "---- FxFlanger setProcMode " << ind << std::endl;
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
    static void sprocess_03( void * thp );
    static void sprocess_04( void * thp );

    inline void processForward(void)
    {
        out().mult( inp<0>(), param.mode01.gain );
        delay.pushSection( inp<0>() );
        for( int64_t si = 0; si < EbufferPar::sectionSize; ++si ) {
            const int64_t si64 = si<<32;
            const int64_t delayA = modulatorValue[ chA ].getInc() >> modulatorNorm;
            const int64_t delayB = modulatorValue[ chB ].getInc() >> modulatorNorm;
            out().channel[ chA ][ si ] += delay.getInterpolated2Order<chA>( param.mode01.baseDelay + delayA - si64 ) * param.mode01.gain;
            out().channel[ chB ][ si ] += delay.getInterpolated2Order<chB>( param.mode01.baseDelay + delayB - si64 ) * param.mode01.gain;
//            out().channel[ chB ][ si ] = delayA * (1.0f/(1LL<<42) ) ;
        }
    }

    inline void processFeedback(void)
    {
        const uint32_t startIndex = delay.getSectionIndex();
        out().mult( inp<0>(), param.mode01.gain );
        delay.pushSection( inp<0>() );
        for( int64_t si = 0; si < EbufferPar::sectionSize; ++si ) {
            const int64_t si64 = si<<32;
            const int64_t delayA = modulatorValue[ chA ].getInc() >> modulatorNorm;
            const int64_t delayB = modulatorValue[ chB ].getInc() >> modulatorNorm;
            const float vA = delay.getInterpolated2Order<chA>( param.mode01.baseDelay + delayA - si64 );
            const float vB = delay.getInterpolated2Order<chB>( param.mode01.baseDelay + delayB - si64 );
            out().channel[ chA ][ si ] += vA * param.mode01.gain;
            out().channel[ chB ][ si ] += vB * param.mode01.gain;
            delay.channel[ chA ][ startIndex + si ] -= vA * param.mode01.feedbackGain;
            delay.channel[ chB ][ startIndex + si ] -= vB * param.mode01.feedbackGain;
        }
    }

    inline void useSine(void)
    {
        modulatorValue[ chA ].set(( param.mode01.oscMasterIndex.getLfoSinU32()) * int64_t( param.mode01.depth ));
        modulatorValue[ chB ].set(( param.mode01.oscSlaveIndex.getLfoSinU32())  * int64_t( param.mode01.depth ));
    }

    inline void useTriangle(void)
    {
        modulatorValue[ chA ].set(( param.mode01.oscMasterIndex.getLfoTriangleU32()) * int64_t( param.mode01.depth ));
        modulatorValue[ chB ].set(( param.mode01.oscSlaveIndex.getLfoTriangleU32())  * int64_t( param.mode01.depth ));
    }

    EDelayLine  delay;
    ControllerLinearIterator<int64_t,EbufferPar::sectionSizeExp> modulatorValue[2];
};


} // end namespace yacynth


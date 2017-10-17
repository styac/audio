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
 * File:   FxFilter.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on June 21, 2016, 9:57 PM
 */

#include    "FxFilterParam.h"

#include    "../effects/FxBase.h"
#include    "../utils/Filter4Pole.h"
#include    "../utils/FilterAllpass.h"
#include    "../utils/FilterStateVariable.h"

// TODO : BW : stage 0 (tan) - FREQ : STAGE 1 (cos)
using namespace filter;

namespace yacynth {
using namespace TagEffectCollectorLevel_01;
using namespace TagEffectTypeLevel_02;

// --------------------------------------------------------------------
// FxFilterParam ------------------------------------------------------
// --------------------------------------------------------------------
// controller index
// cache
// mapper
// function
//

// modes:
//
//  1,2,3,4 input mono -> output stereo
//  2,4,6,8 input stereo ( 1,2,3,4 * 2)
//
//  1 LP | BP
//  2 LP+BP | BP+BP
//  3 LP+BP+BP | BP+BP+BP
//  4 LP+BP+BP+BP | BP+BP+BP+BP
// control : controller(1..8)( y0 + slope ), amplitude (1) ( y0 + slope ), oscillator (1..8) ( y0 + slope )
//
// key>     keyprefix+instance
// key-refix> /effect/filter/...
// data>    comment+const+var
// data-const> inputcount,maxmode,filtercount,channelcount,


// --------------------------------------------------------------------
// FxFilter -----------------------------------------------------------
// --------------------------------------------------------------------

class FxFilter : public Fx<FxFilterParam>  {
public:
    using MyType = FxFilter;
    FxFilter()
    :   Fx<FxFilterParam>()
    ,   wetDryGain(1.0f)
    {
    }

    virtual bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex ) override;

    virtual void clearState() override;

    virtual bool setSprocessNext( uint16_t mode ) override;

    virtual bool connect( const FxBase * v, uint16_t ind ) override;

private:
    // 00 is always clear for output or bypass for in-out
    static void sprocess_01( void * thp );
    static void sprocess_02( void * thp );
    static void sprocess_03( void * thp );
    static void sprocess_04( void * thp );
    static void sprocess_05( void * thp );
    static void sprocess_06( void * thp );
    static void sprocess_07( void * thp );
    static void sprocess_08( void * thp );
    static void sprocess_09( void * thp );
    // ------------------------------------------------------------
    // processors

    // 2nd order allpass 2x channel 4x filter -- phaser
    // TODO retest with stage 0 - BW stage 1 freq
    void process_01_ap4x(void)
    {
        for( auto si=0u; si < sectionSize; ++si ) {
            filterAllpass.allpass2x8(
                inp<0>().channel[0][si], inp<0>().channel[1][si],
                   out().channel[0][si],    out().channel[1][si] );
                    // out().channel[1][si] = cval_masterLfo.getValue() * (1.0f/(1<<16));
                    // check the param change
                    // out().channel[1][si] = filterAllpass.km3[0][0][0];
        }
        out().wetDryBalance( inp<0>(), wetDryGain );
//        const auto sinv         = param.mode_2ch_x4ap_phaser_mode01.oscMasterIndex.getLfoTriangleI16();// .getLfoSinI16();

        // use 8 offset instead of setYcentAll : settable the distance : Barks

        const auto sinv         = param.modeAP01.oscMasterIndex.getLfoSinI16();
        const auto sinvScaled   = param.modeAP01.notchMapper.getScaled(sinv);
        const auto sinvYcent    = param.modeAP01.notchMapper.getOffseted(sinvScaled);
        filterAllpass.setYcentAll<1>(sinvYcent, 1<<23); // freq is stage 2
//        filterAllpass.setYcentAll<0>(sinvYcent, 1<<23);

        // std::cout << "old " << ycentKA << " new " << sinvYcent << std::endl;

        if( feedbackGainCache.update( param.modeAP01.feedbackGainIndex ) ) {
        // must be init
//            filterAllpass.setFeedback(feedbackGainCache.getExpValueFloat());
            filterAllpass.setFeedback(0.3f);
        }

        if( wetDryGainCache.update( param.modeAP01.wetDryGainIndex ) ) {
        // must be init
        //    wetDryGain = wetDryGainCache.getExpValueFloat();
        }

        if( bandWidhthCache.update( param.modeAP01.bandWidhthIndex ) ) {
            const auto bwCVal   = bandWidhthCache.getValueI32();
            const auto bwScaled = param.modeAP01.bandwidthMapper.getScaled( bwCVal );
            const auto bw       = param.modeAP01.bandwidthMapper.getOffseted( bwScaled );
//            filterAllpass.setYcentAll<1>(bw, 0);
            // TODO direct set by K  -- should be -0.5 .. -0.9999xx check sign !!
            // setKAll<0>( bw ) > see OscillatorNoiseInt> 0.75 ... 0.99999 --- CC 0..127
            filterAllpass.setYcentAll<0>(bw, 0);
        }

        if( deltaPhaseControlCache.update( param.modeAP01.deltaPhaseControlIndex ) ) {
        // must be init
        //    wetDryGain = wetDryGainCache.getExpValueFloat();
        }

        if( phaseDiff00ControlCache.update( param.modeAP01.phaseDiff00ControlIndex ) ) {
        // must be init
        //    wetDryGain = wetDryGainCache.getExpValueFloat();
        }
    }

    // ------------------------------------------------------------
    void process_02_svf_1x(void)
    {
        for( auto si=0u; si < sectionSize; ++si ) {
            filterStateVariable.set1x1<0>( inp<0>().channel[0][si] );
//            filterStateVariable.get1x1LP<0>( out().channel[0][si] );
//            filterStateVariable.get1x1LP<0>( out().channel[1][si] );
            filterStateVariable.get1x1BP1<0>( out().channel[0][si] );
            filterStateVariable.get1x1BP1<0>( out().channel[1][si] );
        }

        if( fControlCache.updateDelta( param.modeSV01.fControlIndex ) ) {
            const int32_t ycent = param.modeSV01.fMapper.getOffseted( param.modeSV01.fMapper.getScaled( fControlCache.value ) );
            filterStateVariable.set_F( ycent, 0 );
        }

        if( qControlCache.updateDelta( param.modeSV01.qControlIndex ) ) {
            filterStateVariable.set_Q( qControlCache.getExpValueFloat_127() );
        }
    }
    // ------------------------------------------------------------

    void process_03_pole4_1x(void)
    {
        for( auto si=0u; si < sectionSize; ++si ) {
            filter4Pole.set1x1<0>( inp<0>().channel[0][si] );
            filter4Pole.get1x1BP32<0,2>( out().channel[0][si] );
            filter4Pole.get1x1BP32<0,2>( out().channel[1][si] );
        }

        if( fControlCache.updateDelta( param.mode4P01.fControlIndex ) ) {
            const int32_t ycent = param.mode4P01.fMapper.getOffseted( param.mode4P01.fMapper.getScaled( fControlCache.value ) );
            filter4Pole.set_F( ycent, 0 );
        }

        if( qControlCache.updateDelta( param.mode4P01.qControlIndex ) ) {
            filter4Pole.set_Q( qControlCache.getExpValueFloat() );
        }
    }
    // ------------------------------------------------------------

private:
    // possible filters
    FilterAllpass<2,1,1>            filterAllpass;
    Filter4Pole<2,1>                filter4Pole;
    FilterStateVariable2xOVS<2,1>   filterStateVariable;

    // cache and other variables
    ControllerCache             wetDryGainCache;
    ControllerCache             feedbackGainCache;
    ControllerCache             bandWidhthCache;
    ControllerCache             deltaPhaseControlCache;
    ControllerCache             phaseDiff00ControlCache;
    float                       wetDryGain;

    // manual control
    ControllerCacheRate<5>      fControlCache;
    ControllerCacheRate<5>      qControlCache;
    //ControllerCache
};

} // end namespace yacynth


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

#include    "FxBase.h"
#include    "../utils/Filter4Pole.h"
#include    "../utils/FilterAllpass.h"
#include    "protocol.h"

using namespace filter;

namespace yacynth {
using namespace TagEffectCollectorLevel_01;
using namespace TagEffectTypeLevel_02;

// --------------------------------------------------------------------
// FxFilterParam ------------------------------------------------------
// --------------------------------------------------------------------

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

// interface to save load DB !!
class FxFilterParam {
public:
    FxFilterParam();

    // mandatory fields
    static constexpr char const * const name    = "Filter";
    static constexpr TagEffectType  type        = TagEffectType::FxFilter;
    static constexpr std::size_t maxMode        = 9; // 0 is always exist> 0,1,2
    static constexpr std::size_t inputCount     = 1;

    static constexpr uint8_t  filterCount       = 8;
    static constexpr uint8_t  vfilterCount      = 2;
    static constexpr uint8_t  channelCountExp   = 3;
    static constexpr uint8_t  channelCount      = 1<<channelCountExp;
    static constexpr uint8_t  channelCountMask  = channelCount-1;
    static constexpr uint8_t  filterSampleCount = EIObuffer::sectionSize;

    bool parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    // OBSOLETE
    // to control the effect with manual ()
    ControlledValue<filterCount>    freq[filterCount];   // 8 controller - 8 filter ???
    ControlledValue<1>              q[filterCount];

    // OBSOLETE
    // to control the effect with lfo
    ControlledValue<1> testMasterLfo1;
    ControlledValue<1> testSlaveLfo1;
    ControlledValue<1> testMasterLfoDeltaPhase1;
    ControlledValue<1> testSlaveLfoDeltaPhase1;

    // NEW
    // each mode has own param set
    struct Mode_01_ap22x4x {
        // used by setter
        bool check(void) const
        {
            return true;
        }
        ControllerIndex         cindex_masterLfoDeltaPhase; // to control the frequency
        ControllerIndex         cindex_masterLfo;           // actual phase
        ControllerMapLinear<8>  cmaplin_masterLfo;          // mapping to amplitude to control
    } mode_01_ap4x;

    struct Mode_02_ap4x {
        // used by setter
        bool check(void) const
        {
            return true;
        }
        ControllerIndex         cindex_masterLfoDeltaPhase; // to control the frequency
        ControllerIndex         cindex_slaveLfoDeltaPhase;  // to control the phase diff
        ControllerIndex         cindex_masterLfo;           // actual phase
        ControllerIndex         cindex_slaveLfo;            // actual phase
        ControllerMapLinear<4>  cmaplin_masterLfo;          // mapping to amplitude to control
        ControllerMapLinear<4>  cmaplin_slaveLfo;           // mapping to amplitude to control
    } mode_02_ap4x ;
};

// --------------------------------------------------------------------
// FxFilter -----------------------------------------------------------
// --------------------------------------------------------------------

class FxFilter : public Fx<FxFilterParam>  {
public:
    using MyType = FxFilter;
    FxFilter()
    :   Fx<FxFilterParam>()
    {
        // fill the call table
        fillSprocessv<0>(sprocess_00);
        fillSprocessv<1>(sprocess_01);
        fillSprocessv<2>(sprocess_02);
        fillSprocessv<3>(sprocess_03);
        fillSprocessv<4>(sprocess_04);
        fillSprocessv<5>(sprocess_05);
        fillSprocessv<6>(sprocess_06);
        fillSprocessv<7>(sprocess_07);
        fillSprocessv<8>(sprocess_08);
        fillSprocessv<9>(sprocess_09);
    }

    virtual bool parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex ) override;

    virtual void clearTransient() override;

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
        default:
            return sprocessp; // illegal index no change
        }
    }
#endif
    virtual bool connect( const FxBase * v, uint16_t ind ) override
    {
        doConnect(v,ind);
    };

private:
    // 00 is always clear for output or bypass for in-out
    static void sprocess_00( void * thp );
    static void sprocess_01( void * thp );
    static void sprocess_02( void * thp );
    static void sprocess_03( void * thp );
    static void sprocess_04( void * thp );
    static void sprocess_05( void * thp );
    static void sprocess_06( void * thp );
    static void sprocess_07( void * thp );
    static void sprocess_08( void * thp );
    static void sprocess_09( void * thp );
    // go up to Fx ???
    static void sprocessTransient( void * thp )
    {
        MyType& th = *static_cast< MyType * >(thp);
        switch( th.fadePhase ) {
        // 1 phase
        case FadePhase::FPH_fadeNo:
            th.sprocessp = th.sprocesspSave = th.sprocessv[ th.procMode ];
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


    // 2nd order allpass 2x channel 4x filter -- phaser
    void process_01_ap4x(void)
    {
        // new
//        ControllerValue    cval_masterLfo;
        cval_masterLfo.updateLfoSin( param.mode_01_ap4x.cindex_masterLfo );

        const auto mult = param.mode_01_ap4x.cmaplin_masterLfo.getScaled( cval_masterLfo.value );
        const auto k1f0 = param.mode_01_ap4x.cmaplin_masterLfo.getOffseted( mult, 0 );
        const auto k1f1 = param.mode_01_ap4x.cmaplin_masterLfo.getOffseted( mult, 1 );
        const auto k1f2 = param.mode_01_ap4x.cmaplin_masterLfo.getOffseted( mult, 2 );
        const auto k1f3 = param.mode_01_ap4x.cmaplin_masterLfo.getOffseted( mult, 3 );

//        param.testMasterLfo1.updateLfoTriangle();
//        param.testSlaveLfo1.updateLfoTriangle();
        param.testMasterLfo1.updateLfoTriangle();
        //param.testSlaveLfo1.updateLfoSin();
        int32_t ycentKA = param.testMasterLfo1.getYcent8Value();
//        int32_t ycentKB = param.testSlaveLfo1.getYcent8Value();
//        int32_t ycentKA = 0x16000000;
//        int32_t ycentKB = 0x17000000;
        int32_t ycentK0 = freq2ycent(800.0f);

        // stage 1 tan
        /*
        filterAllpass.setYcent<0,0>(ycentK0,0);
        filterAllpass.setYcent<1,0>(ycentK0,0);
        filterAllpass.setYcent<0,0>(ycentK0,1);
        filterAllpass.setYcent<1,0>(ycentK0,1);
        filterAllpass.setYcent<0,0>(ycentK0,2);
        filterAllpass.setYcent<1,0>(ycentK0,2);
        filterAllpass.setYcent<0,0>(ycentK0,3);
        filterAllpass.setYcent<1,0>(ycentK0,3);
        */
        filterAllpass.setYcentAll<0>(ycentK0, 0);
#if 0
        // stage 2 cos
        filterAllpass.setYcent<0,1>(ycentKA,0);
        filterAllpass.setYcent<1,1>(ycentKA+0x1000000,0);
        filterAllpass.setYcent<0,1>(ycentKA+0x2000000,1);
        filterAllpass.setYcent<1,1>(ycentKA+0x3000000,1);
        filterAllpass.setYcent<0,1>(ycentKA+0x4000000,2);
        filterAllpass.setYcent<1,1>(ycentKA+0x5000000,2);
        filterAllpass.setYcent<0,1>(ycentKA+0x6000000,3);
        filterAllpass.setYcent<1,1>(ycentKA+0x7000000,3);

        filterAllpass.setYcent<1>(ycentKA,0);
        filterAllpass.setYcent<1>(ycentKA+0x1000000,1);
        filterAllpass.setYcent<1>(ycentKA+0x2000000,2);
        filterAllpass.setYcent<1>(ycentKA+0x3000000,3);
        filterAllpass.setYcent<1>(ycentKA+0x4000000,4);
        filterAllpass.setYcent<1>(ycentKA+0x5000000,5);
        filterAllpass.setYcent<1>(ycentKA+0x6000000,6);
        filterAllpass.setYcent<1>(ycentKA+0x7000000,7);
#else

        filterAllpass.setYcentAll<1>(ycentKA, 0x1000000);
#endif

        filterAllpass.setFeedback(0.8f);

        for( auto si=0u; si < sectionSize; ++si ) {
          filterAllpass.allpass2x8(
            inp<0>().channel[0][si],inp<0>().channel[1][si],
            out().channel[0][si], out().channel[1][si] );

            out().channel[0][si] += inp<0>().channel[0][si];
            out().channel[1][si] += inp<0>().channel[1][si];
        }
    }

    void process_02(void)
    {

        uint32_t si=0;
        for( auto sik=0u; sik < 8; ++sik ) {
            // update 1x k0, 1x k1 -- set masks modulo 2^k
            // k0 = 4,5,6,7,0,1,2,3
            // k1 = 0,1,2,3,4,5,6,7
//            filterAllpass.setYcent<0,0>(ycentK0,++k0index);  // tan
//            filterAllpass.setYcent<0,1>(ycentK1,++k1index);  // cos
//            filterAllpass.setYcent<1,0>(ycentK0,++k0index);  // tan
//            filterAllpass.setYcent<1,1>(ycentK1,++k1index);  // cos

            for( auto sil=0u; sil < sectionSize/8; ++sil ) {
                // run filter

                ++si;
            }
        }
    }
#if 0
    // ok
    inline void test_lfoController(void)
    {
//        param.testMasterLfo1.updateLfoTriangle();
//        param.testSlaveLfo1.updateLfoTriangle();
        param.testMasterLfo1.updateLfoTriangle();
        param.testSlaveLfo1.updateLfoTriangle();
//        float A = param.testMasterLfo1.getValue() * (1.0/(1L<<16));
//        float B = param.testSlaveLfo1.getValue() * (1.0/(1L<<16));
        int32_t ycentKA = param.testMasterLfo1.getYcent8Value();
        int32_t ycentKB = param.testSlaveLfo1.getYcent8Value();
        int32_t ycentK0 = freq2ycent(200.0f);

        filterAllpass.setYcent<0,0>(ycentK0);  // tan
        filterAllpass.setYcent<0,1>(ycentKA);  // cos
        filterAllpass.setYcent<1,0>(ycentK0);  // tan
        filterAllpass.setYcent<1,1>(ycentKB);  // cos

        for( auto si=0u; si < sectionSize; ++si ) {
//            out().channel[1][si] = A;
//            out().channel[0][si] = B;
          filterAllpass.allpass2(
            inp<0>().channel[0][si],inp<0>().channel[1][si],
            out().channel[0][si], out().channel[1][si] );
//          filterAllpass.allpass2_1mult<0>( inp<0>().channel[0][si], out().channel[0][si] );
//          filterAllpass.allpass2_transposed<1>( inp<0>().channel[1][si], out().channel[1][si] );
//          filterAllpass.allpass2_direct<1>( inp<0>().channel[1][si], out().channel[1][si] );
//          filterAllpass.allpass2_1mult_transposed<1>( inp<0>().channel[1][si], out().channel[1][si] );

           // filterAllpass.allpass2ch0( inp<0>().channel[0][si], out().channel[0][si] );
            // notch filter for testing
//            out().channel[1][si] = out().channel[0][si] - out().channel[1][si];
            out().channel[0][si] += inp<0>().channel[0][si];
            out().channel[1][si] += inp<0>().channel[1][si];

        }
    }


    inline void test_allpass1(void)
    {
        float freqK0 = 200.0;
        if( ++count & 0x03FF ) {
            if( count & 0x400 ) {
                freqK0 = 200.0;
            } else {
                freqK0 = 3200.0;
            }

            int32_t ycentK0 = freq2ycent(freqK0);
            filterAllpass.setYcent<0,0>(ycentK0);  // tan
            filterAllpass.setYcent<1,0>(ycentK0);  // tan
        }



        for( auto si=0u; si < sectionSize; ++si ) {
//          filterAllpass.allpass1_direct<0>( inp<0>().channel[0][si], out().channel[0][si] );
          filterAllpass.allpass1_1mult<0>( inp<0>().channel[0][si], out().channel[0][si] );
          filterAllpass.allpass1_transposed<1>( inp<0>().channel[1][si], out().channel[1][si] );
//          filterAllpass.allpass1_direct<1>( inp<0>().channel[1][si], out().channel[1][si] );

            out().channel[1][si] += inp<0>().channel[1][si];
            out().channel[0][si] += inp<0>().channel[0][si];

//            out().channel[1][si] = out().channel[0][si] + inp<0>().channel[0][si];
//            out().channel[0][si] = out().channel[1][si];
        }
    }

    inline void test_allpass2(void)
    {
        float freqK0 = 800.0;
        float freqK1 = 200.0;
        if( ++count & 0x03FF ) {

//            k0 = -0.9;
//            k1 = +0.90;
            if( count & 0x400 ) {
            //    k0 = -0.99;
//                k1 = +0.99;
//                freqK0 = 20.0;
                freqK1 = 200.0;
            } else {
            //    k0 = -0.1;
//                k1 = +0.90;
//                k1 = -0.90;
                freqK1 = 3200.0;
            }

            int32_t ycentK0 = freq2ycent(freqK0);
            int32_t ycentK1 = freq2ycent(freqK1);

            // 2 order
//            filterAllpass.setFreq<0,0>(fcK0);  // tan
//            filterAllpass.setFreq<0,1>(fcK1);  // cos
//            filterAllpass.setFreq<1,0>(fcK0);  // tan
//            filterAllpass.setFreq<1,1>(fcK1);  // cos
            //filterAllpass.setK<0,0>(-1.1);
#if 1
            filterAllpass.setYcent<0,0>(ycentK0);  // tan
            filterAllpass.setYcent<0,1>(ycentK1);  // cos
            filterAllpass.setYcent<1,0>(ycentK0);  // tan
            filterAllpass.setYcent<1,1>(ycentK1);  // cos
#endif

//            filterAllpass.setK<0,0>(k0);  // tan
//            filterAllpass.setK<0,1>(k1);  // cos
//            filterAllpass.setK<1,0>(k0);  // tan
//            filterAllpass.setK<1,1>(k1);  // cos
        }



        for( auto si=0u; si < sectionSize; ++si ) {
          filterAllpass.allpass2_1mult<0>( inp<0>().channel[0][si], out().channel[0][si] );
//          filterAllpass.allpass2_transposed<1>( inp<0>().channel[1][si], out().channel[1][si] );
//          filterAllpass.allpass2_direct<1>( inp<0>().channel[1][si], out().channel[1][si] );
          filterAllpass.allpass2_1mult_transposed<1>( inp<0>().channel[1][si], out().channel[1][si] );

           // filterAllpass.allpass2ch0( inp<0>().channel[0][si], out().channel[0][si] );
            // notch filter for testing
//            out().channel[1][si] = out().channel[0][si] - out().channel[1][si];
            out().channel[1][si] += inp<0>().channel[1][si];
            out().channel[0][si] += inp<0>().channel[0][si];

//            out().channel[1][si] = out().channel[0][si] + inp<0>().channel[0][si];
//            out().channel[0][si] = out().channel[1][si];
        }
    }

    // bandwidth test k0
    inline void test_allpass2_bw(void)
    {
        float freqK1 = 300.0;
        int32_t ycentK1 = freq2ycent(freqK1);
        filterAllpass.setYcent<0,1>(ycentK1);  // cos
        filterAllpass.setYcent<1,1>(ycentK1);  // cos
        float freqK0 = 40.0;


        if( ++count & 0x03FF ) {
            k0 = -0.9;
            if( count & 0x400 ) {
                freqK0 = 40.0;
            } else {
                freqK0 = 3200.0;
            }
            int32_t ycentK0 = freq2ycent(freqK0);
            filterAllpass.setYcent<0,0>(ycentK0);  // tan
            filterAllpass.setYcent<1,0>(ycentK0);  // tan
        }

        for( auto si=0u; si < sectionSize; ++si ) {
          filterAllpass.allpass2_1mult<0>( inp<0>().channel[0][si], out().channel[0][si] );
//          filterAllpass.allpass2_transposed<1>( inp<0>().channel[1][si], out().channel[1][si] );
//          filterAllpass.allpass2_direct<1>( inp<0>().channel[1][si], out().channel[1][si] );
          filterAllpass.allpass2_1mult_transposed<1>( inp<0>().channel[1][si], out().channel[1][si] );

           // filterAllpass.allpass2ch0( inp<0>().channel[0][si], out().channel[0][si] );
            // notch filter for testing
//            out().channel[1][si] = out().channel[0][si] - out().channel[1][si];
            out().channel[1][si] += inp<0>().channel[1][si];
            out().channel[0][si] += inp<0>().channel[0][si];

//            out().channel[1][si] = out().channel[0][si] + inp<0>().channel[0][si];
//            out().channel[0][si] = out().channel[1][si];
        }
    }
#endif
// obsolate
//    union {
//        v4sf v4[FxFilterParam::channelCount];
//        Filter4PoleOld<FxFilterParam::channelCountExp>    filter4pole;
//    };

    FilterAllpass<2,1,1> filterAllpass;
    // new controller
    // for each ControllerIndex
    ControllerCache    cval_Ext[8]; // to control the modulation and or filter directly
    ControllerCache    cval_masterLfo;
    ControllerCache    cval_slaveLfo;

    // temp
    float k0;
    float k1;
    float kbw;
    int count;
};


} // end namespace yacynth


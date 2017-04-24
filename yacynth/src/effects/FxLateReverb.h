#pragma once

/*
 * Copyright (C) 2016-2017 Istvan Simon -- stevens37 at gmail dot com
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
 * File:   FxLateReverb.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 */


// TODO early reflection FxEarlyReflection -- simple fast short echo without feedback
// TODO try more internal reflections
// TODO extend to 16 combs

#if 0
doc:

https://ccrma.stanford.edu/~jos/Reverb/
https://ccrma.stanford.edu/realsimple/Reverb/


https://ccrma.stanford.edu/~jos/Reverb/Zita_Rev1_Reverberator.html
https://ccrma.stanford.edu/~jos/Reverb/Zita_Rev1_Damping_Filters.html
https://ccrma.stanford.edu/~jos/Reverb/High_Frequency_Damping_Lowpass.html

https://ccrma.stanford.edu/~jos/Reverb/Reverb.pdf
https://ccrma.stanford.edu/~jos/Reverb/Reverb_2up.pdf
https://ccrma.stanford.edu/~jos/Reverb/Reverb_4up.pdf


https://ccrma.stanford.edu/~dattorro/music.html
https://ses.library.usyd.edu.au/bitstream/2123/8436/2/FinalTechnologyReview_DarioRizo.pdf

http://www.convexoptimization.com/wikimization/index.php/Dattorro_Convex_Optimization_of_a_Reverberator

https://christianfloisand.wordpress.com/tag/dattorro/
https://christianfloisand.wordpress.com/downloads/
http://soundprogramming.blogspot.co.at/2011/05/datorros-reverb-part-2.html


http://kokkinizita.linuxaudio.org/linuxaudio/zita-rev1-doc/quickguide.html
http://kokkinizita.linuxaudio.org/linuxaudio/downloads/index.html
http://kokkinizita.linuxaudio.org/papers/index.html


https://linuxmusicians.com/viewtopic.php?t=10953
http://linux-audio.4202.n7.nabble.com/Favourite-Linux-reverbs-td97375.html


    // https://www.kvraudio.com/forum/viewtopic.php?p=4603337
    // FDN - householder
    // https://www.dsprelated.com/freebooks/pasp/Choice_Lossless_Feedback_Matrix.html
    // https://www.dsprelated.com/freebooks/pasp/FDN_Reverberation.html
    // https://ccrma.stanford.edu/realsimple/Reverb/Householder_FDN_Coupled.html
    // http://dspace.library.uvic.ca:8080/bitstream/handle/1828/2961/yli_thesis.pdf;jsessionid=1E35C27885AD3B1C44233C0525D25007?sequence=1
    //

#endif

#include    "../effects/FxBase.h"
#include    "../utils/Fastsincos.h"
#include    "../utils/FilterComb.h"
#include    "../effects/DelayTap.h"

using namespace tables;
using namespace filter;

namespace yacynth {
using namespace TagEffectFxLateReverbModeLevel_03;

class FxLateReverbParam {
public:
    // mandatory fields
    static constexpr char const * const name    = "FxLateReverb";
    static constexpr TagEffectType  type        = TagEffectType::FxLateReverb;
    static constexpr std::size_t maxMode        = 3;    // 0 is always exist> 0,1,2
    static constexpr std::size_t inputCount     = 1;
    static constexpr std::size_t allpassLngExp  = 11;   // much more then needed - will be not used
    static constexpr std::size_t combLngExp     = 11;   // much more then needed
    static constexpr std::size_t allpassCount   = 4;
    static constexpr std::size_t combCount      = 8;

    static constexpr float householderFeedback  = -2.0f / combCount;

    bool parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    // control feedback - decay
    // control low pass - tone
    // control early reflection - other effect
    // control mixing dry - wet - mixer

    struct Mode01_Housholder {
        bool check()
        {
            for( uint16_t ci = 0; ci <combCount; ++ci ) {
                if( std::abs( combTaps.coeffFB_v[ci] ) >= 0.5f ) // householder case
                    return false;
                if( (combTaps.coeffLP_v[ci] > 0.999f) || (combTaps.coeffLP_v[ci] < 0 ) )
                    return false;
                if( std::abs( combTapsInternal.coeffFB_v[ci] ) >= 0.5f ) // householder case
                    return false;
                if( combTaps.delaySrc[ci] >= (1<<(combLngExp+EbufferPar::sectionSizeExp)) )
                    return false;
                if( combTapsInternal.delaySrc[ci] > (combTaps.delaySrc[ci]-3) ) // 3 is practical const
                    return false;
            }
            return true;
        }
        // TODO considering 1coeff version - check if 4 is needed
        MonoDelayLowpassTapArray<combCount>  combTaps;
        MonoDelayTapArray<combCount>         combTapsInternal;
        MonoDelayTapArray<allpassCount>      allpassTaps;
        // controller index...
    } mode01_Householder;
};

class FxLateReverb : public Fx<FxLateReverbParam>  {
public:
    typedef EDelayLineArray<FxLateReverbParam::combCount>       CombDelay;
    typedef EDelayLineArray<FxLateReverbParam::allpassCount>    AllpassDelay;

    using MyType = FxLateReverb;
    FxLateReverb()
    :   Fx<FxLateReverbParam>()
//    ,   preDelay(param.delayLngExp)
    ,   allPassVector(param.allpassLngExp)
    ,   combVector(param.combLngExp)
    {
        fillSprocessv<0>(sprocess_00);
        fillSprocessv<1>(sprocess_01);
        fillSprocessv<2>(sprocess_02);
        fillSprocessv<3>(sprocess_03);
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

    static void sprocess_01( void * thp );  // reverb 1 mode

    static void sprocess_02( void * thp );  // reverb 2 mode

    static void sprocess_03( void * thp );  // reverb 3 mode
#if 0
    // save for later
    inline void process_01_2(void)
    {
        static_assert( param.combCount==8, "this uses 8 comb filters" );
        static_assert( param.allpassCount==4, "this uses 4 allpass filters" );

        // TODO: controller for change feedback, lowpwass

        coeffFeedback.v4[0]   = param.mode01_Householder.combTaps.coeffFB_v4[0];
        coeffFeedback.v4[1]   = param.mode01_Householder.combTaps.coeffFB_v4[1];
        coeffLowpass.v4[0]    = param.mode01_Householder.combTaps.coeffLP_v4[0];
        coeffLowpass.v4[1]    = param.mode01_Householder.combTaps.coeffLP_v4[1];
        coeffAllpass.v4[0]    = param.mode01_Householder.allpassTaps.coeffFB_v4[0];

        union alignas(16) {
            v4sf    vchannel[ EbufferPar::vsectionSize ];
            float   channel[ EbufferPar::sectionSize ];
        };

        AllpassDelay::PackedChannel apOutput;
        AllpassDelay::PackedChannel apTmp;
        CombDelay::PackedChannel    combOutput;
        CombDelay::PackedChannel    combTmp;
#if 0
        // add input to local buff
        for( auto i=0u; i < vsectionSize; ++i ) {
            vchannel[i] = inp<0>().vchannel[chA][i] + inp<0>().vchannel[chB][i];
        }
#else
        // test
        for( auto si=0u; si < vsectionSize; ++si ) {
            vchannel[si] = inp<0>().vchannel[chA][si];
//            out().vchannel[chA][si] = inp<0>().vchannel[chA][si];
        }

#endif
        // TODO
        // calculate allpasses
        // 1st experiment: parallel +1 framedelay

/*

    template< std::size_t CH >
    inline void allpass1( const float x0, float& y0 )
    {

        const float t0 = ( input - storage ) * k;
        output = t0 + storage;
        storage = t0 + input;
    }

 */
#if 0
        // 4x allpass - serial - 1 mult version
        for( auto si=0u; si<sectionSize; ++si ) {
            allPassVector.get( param.mode01_Householder.allpassTaps.delaySrc, apTmp );
            const V4vf input ( channel[si], apTmp.v[0], apTmp.v[1], apTmp.v[2] );
            const V4vf t0   = ( input.v4[0] - apTmp.v4[0] ) * coeffAllpass.v4[0];
            channel[si] = t0.v[3] + apTmp.v[3]; // write back
            apTmp.v4[0]   = t0.v4 + input.v4;
            allPassVector.push( apTmp );

            // test
            //out().channel[0][si] = channel[si];
        }
        //return;
#endif
        // calc combs 2x4 channel
        for( auto si=0u; si<sectionSize; ++si ) {
            combOutput.v4[0] = stateLowPass.v4[0];
            combOutput.v4[1] = stateLowPass.v4[1];
            // generate output
            const V4vf res1 ( combOutput.v4[0] + combOutput.v4[1] ); // summ 8 output to 4 (pairs)
            const float r00 = res1.v[0] + res1.v[2];
            const float r11 = res1.v[1] - res1.v[3];
            out().channel[0][si] = r00 + r11;
            out().channel[1][si] = r00 - r11;

            // output : delay + filter (1 mult)
            // feedback output * fbmult
            combVector.get( param.mode01_Householder.combTaps.delaySrc, combTmp );
            // -1.0 < vcoeffFeedback < 1.0  -- positiv or negative
            stateLowPass.v4[0] = combTmp.v4[0] + ( stateLowPass.v4[0] - combTmp.v4[0] ) * coeffLowpass.v4[0];
            stateLowPass.v4[1] = combTmp.v4[1] + ( stateLowPass.v4[1] - combTmp.v4[1] ) * coeffLowpass.v4[1];

            // 0 < coeffLowpass < 1.0
            combTmp.v4[0] = coeffFeedback.v4[0] * combOutput.v4[0];
            combTmp.v4[1] = coeffFeedback.v4[1] * combOutput.v4[1];

            // householder feedback  4x4 2 channels

            const float hp23 = combTmp.v[2] + combTmp.v[3];
            const float hp01 = combTmp.v[0] + combTmp.v[1];
            const float hm23 = combTmp.v[2] - combTmp.v[3];
            const float hm01 = combTmp.v[0] - combTmp.v[1];

            combOutput.v[0] =  hm01 - hp23;
            combOutput.v[1] = -hm01 - hp23;
            combOutput.v[2] = -hp01 + hm23;
            combOutput.v[3] = -hp01 - hm23;

            const float hp67 = combTmp.v[6] + combTmp.v[7];
            const float hp45 = combTmp.v[4] + combTmp.v[5];
            const float hm67 = combTmp.v[6] - combTmp.v[7];
            const float hm45 = combTmp.v[4] - combTmp.v[5];

            combOutput.v[4] =  hm45 - hp67;
            combOutput.v[5] = -hm45 - hp67;
            combOutput.v[6] = -hp45 + hm67;
            combOutput.v[7] = -hp45 - hm67;

            combOutput.v4[0] += channel[si];
            combOutput.v4[1] += channel[si];
            combVector.push( combOutput );
        }
    }


    inline void process_01_3(void)
    {
        static_assert( param.combCount==8, "this uses 8 comb filters" );
//        static_assert( param.allpassCount==4, "this uses 4 allpass filters" );

        // TODO: controller for change feedback,internal feedback, lowpass
        //

        coeffFeedback.v4[0]         = param.mode01_Householder.combTaps.coeffFB_v4[0];
        coeffFeedback.v4[1]         = param.mode01_Householder.combTaps.coeffFB_v4[1];
        coeffLowpass.v4[0]          = param.mode01_Householder.combTaps.coeffLP_v4[0];
        coeffLowpass.v4[1]          = param.mode01_Householder.combTaps.coeffLP_v4[1];
        coeffFeedbackInternal.v4[0] = param.mode01_Householder.combTapsInternal.coeffFB_v4[0];
        coeffFeedbackInternal.v4[1] = param.mode01_Householder.combTapsInternal.coeffFB_v4[1];

        coeffAllpass.v4[0]    = param.mode01_Householder.allpassTaps.coeffFB_v4[0];


        // temp buffer -- might not be needed
        union alignas(16) {
            v4sf    vchannel[ EbufferPar::vsectionSize ];
            float   channel[ EbufferPar::sectionSize ];
        };

        AllpassDelay::PackedChannel apOutput;
        AllpassDelay::PackedChannel apTmp;
        CombDelay::PackedChannel    combOutput;
        CombDelay::PackedChannel    combTmp;
        CombDelay::PackedChannel    combInternal;
#if 1
        // add input to local buff
        for( auto si=0u; si < vsectionSize; ++si ) {
            vchannel[si] = inp<0>().vchannel[chA][si] + inp<0>().vchannel[chB][si];
        }
#else
        // test
        for( auto si=0u; si < vsectionSize; ++si ) {
            vchannel[si] = inp<0>().vchannel[chA][si];
            out().vchannel[chA][si] = inp<0>().vchannel[chA][si];
        }

#endif


        // calc combs 2x4 channel
        for( auto si=0u; si<sectionSize; ++si ) {
            combOutput.v4[0] = stateLowPass.v4[0];
            combOutput.v4[1] = stateLowPass.v4[1];
            // generate output
            const V4vf res1 ( combOutput.v4[0] + combOutput.v4[1] ); // summ 8 output to 4 (pairs)
            const float r00 = res1.v[0] + res1.v[2];
            const float r11 = res1.v[1] - res1.v[3];
            out().channel[chA][si] = r00 + r11;
            out().channel[chB][si] = r00 - r11;

            // output : delay + filter (1 mult)
            // feedback output * fbmult
            combVector.get( param.mode01_Householder.combTaps.delaySrc, combTmp );
            combVector.get( param.mode01_Householder.combTapsInternal.delaySrc, combInternal );

            // -1.0 < vcoeffFeedback < 1.0  -- positiv or negative
            stateLowPass.v4[0] = combTmp.v4[0] + ( stateLowPass.v4[0] - combTmp.v4[0] ) * coeffLowpass.v4[0];
            stateLowPass.v4[1] = combTmp.v4[1] + ( stateLowPass.v4[1] - combTmp.v4[1] ) * coeffLowpass.v4[1];

            // 0 < coeffLowpass < 1.0
            combTmp.v4[0] = coeffFeedback.v4[0] * combOutput.v4[0];
            combTmp.v4[1] = coeffFeedback.v4[1] * combOutput.v4[1];

            // householder feedback  4x4 2 channels

            const float hp23 = combTmp.v[2] + combTmp.v[3];
            const float hp01 = combTmp.v[0] + combTmp.v[1];
            const float hm23 = combTmp.v[2] - combTmp.v[3];
            const float hm01 = combTmp.v[0] - combTmp.v[1];

            combOutput.v[0] =  hm01 - hp23;
            combOutput.v[1] = -hm01 - hp23;
            combOutput.v[2] = -hp01 + hm23;
            combOutput.v[3] = -hp01 - hm23;

            const float hp67 = combTmp.v[6] + combTmp.v[7];
            const float hp45 = combTmp.v[4] + combTmp.v[5];
            const float hm67 = combTmp.v[6] - combTmp.v[7];
            const float hm45 = combTmp.v[4] - combTmp.v[5];

            combOutput.v[4] =  hm45 - hp67;
            combOutput.v[5] = -hm45 - hp67;
            combOutput.v[6] = -hp45 + hm67;
            combOutput.v[7] = -hp45 - hm67;

            // internal low cycle feedback
            combOutput.v4[0] -= combInternal.v4[0] * coeffFeedbackInternal.v4[0];
            combOutput.v4[1] -= combInternal.v4[1] * coeffFeedbackInternal.v4[1];

            // if there will not be allpass section
            // const float inp = inp<0>().channel[chA][si] + inp<0>().channel[chB][si];
            // add input and push back delay
            combOutput.v4[0] += channel[si];
            combOutput.v4[1] += channel[si];
            combVector.push( combOutput );
        }
    }

#endif

    // no allpass
    inline void process_01(void)
    {
        static_assert( param.combCount >= 8, "this uses 8 comb filters" );

        // std::cout << "CombDelay::bufferSize " <<  combVector.bufferSize  << std::endl;

        // TODO:
        // controller for change feedback,internal feedback, lowpass
        // feedback controller value 0 < x <= 1 - input is always the max (longest reverb)
        // low pass controller value 0 < x <= 1 - input is always the lowest freq (1 pole ! low pass coeff)
        // delay length is only reload

        coeffFeedback.v4[0]         = param.mode01_Householder.combTaps.coeffFB_v4[0];
        coeffFeedback.v4[1]         = param.mode01_Householder.combTaps.coeffFB_v4[1];
        coeffLowpass.v4[0]          = param.mode01_Householder.combTaps.coeffLP_v4[0];
        coeffLowpass.v4[1]          = param.mode01_Householder.combTaps.coeffLP_v4[1];
        coeffFeedbackInternal.v4[0] = param.mode01_Householder.combTapsInternal.coeffFB_v4[0];
        coeffFeedbackInternal.v4[1] = param.mode01_Householder.combTapsInternal.coeffFB_v4[1];

        CombDelay::PackedChannel    combOutput;
        CombDelay::PackedChannel    combTmp;
        CombDelay::PackedChannel    combInternal;

        // calc combs 2x4 channel
        for( auto si=0u; si<sectionSize; ++si ) {
            
            // this makes an extra 1 unit delay 
            // TODO to clean it
            combOutput.v4[0] = stateLowPass.v4[0];
            combOutput.v4[1] = stateLowPass.v4[1];
            // generate output
            const V4vf res1 ( combOutput.v4[0] + combOutput.v4[1] ); // summ 8 output to 4 (pairs)
            const float r00 = res1.v[0] + res1.v[2];
            const float r11 = res1.v[1] - res1.v[3];
            out().channel[chA][si] = r00 + r11;
            out().channel[chB][si] = r00 - r11;

            // output : delay + filter (1 mult)
            // feedback output * fbmult
            combVector.get( param.mode01_Householder.combTaps.delaySrc, combTmp );
            combVector.get( param.mode01_Householder.combTapsInternal.delaySrc, combInternal );

            // -1.0 < vcoeffFeedback < 1.0  -- positiv or negative
            stateLowPass.v4[0] = combTmp.v4[0] + ( stateLowPass.v4[0] - combTmp.v4[0] ) * coeffLowpass.v4[0];
            stateLowPass.v4[1] = combTmp.v4[1] + ( stateLowPass.v4[1] - combTmp.v4[1] ) * coeffLowpass.v4[1];

            // 0 < coeffLowpass < 1.0
            combTmp.v4[0] = coeffFeedback.v4[0] * combOutput.v4[0];
            combTmp.v4[1] = coeffFeedback.v4[1] * combOutput.v4[1];

            // householder feedback  4x4 2 channels

            const float hp23 = combTmp.v[2] + combTmp.v[3];
            const float hp01 = combTmp.v[0] + combTmp.v[1];
            const float hm23 = combTmp.v[2] - combTmp.v[3];
            const float hm01 = combTmp.v[0] - combTmp.v[1];

            combOutput.v[0] =  hm01 - hp23;
            combOutput.v[1] = -hm01 - hp23;
            combOutput.v[2] = -hp01 + hm23;
            combOutput.v[3] = -hp01 - hm23;

            const float hp67 = combTmp.v[6] + combTmp.v[7];
            const float hp45 = combTmp.v[4] + combTmp.v[5];
            const float hm67 = combTmp.v[6] - combTmp.v[7];
            const float hm45 = combTmp.v[4] - combTmp.v[5];

            combOutput.v[4] =  hm45 - hp67;
            combOutput.v[5] = -hm45 - hp67;
            combOutput.v[6] = -hp45 + hm67;
            combOutput.v[7] = -hp45 - hm67;

            // internal low cycle feedback
            combOutput.v4[0] -= combInternal.v4[0] * coeffFeedbackInternal.v4[0];
            combOutput.v4[1] -= combInternal.v4[1] * coeffFeedbackInternal.v4[1];

            // add input and push back delay
            combOutput.v4[0] += inp<0>().channel[0][si];
            combOutput.v4[1] += inp<0>().channel[1][si];
            combVector.push( combOutput );
        }
    }

    inline void process_02(void)
    {
    }

    inline void process_03(void)
    {
    }

    // + controllerCache?

    // parameter cache
    CombDelay::PackedChannel    coeffFeedback;
    CombDelay::PackedChannel    coeffFeedbackInternal;
    CombDelay::PackedChannel    coeffLowpass;

    // low pass state
    CombDelay::PackedChannel    stateLowPass;


    AllpassDelay::PackedChannel coeffAllpass; // obsolate

    // delay lines
    CombDelay                   combVector;
    AllpassDelay                allPassVector; // obsolate
};


} // end namespace yacynth


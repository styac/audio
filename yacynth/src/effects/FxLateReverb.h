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

// TODO output taps with own lowpass
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

    static constexpr float householderFeedback  = -2.0f;     // combCount;
    static constexpr float lowpassLowLimit      = f2FilterOnePole_F( 18000.0 );
    static constexpr float lowpassHighLimit     = f2FilterOnePole_F( 30.0 );
    static constexpr float feedbackLimit        = 0.6f;
    static constexpr float outputLimit          = 0.8f;
    static constexpr float crossFeedbackLimit   = 0.8f;

    bool parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    // control feedback - decay
    // control low pass - tone
    // control early reflection - other effect
    // control mixing dry - wet - mixer

    struct Mode01 {
        bool check()
        {
            for( uint16_t ci = 0; ci <combCount; ++ci ) {
                // check tapFeedback
                if(( tapFeedback.coeffLowPass.v[ci] > lowpassHighLimit ) || ( tapFeedback.coeffLowPass.v[ci] < lowpassLowLimit )) {
                    std::cout << "\n---- check 2"  << std::endl;
                    return false;
                }
                // feedback -- householder actually 0.5?
                if( std::abs( tapFeedback.coeff.v[ci] ) >= feedbackLimit * ( 1.0f - tapFeedback.coeffLowPass.v[ci] )) {
                    std::cout << "\n---- check 1" << std::endl;
                    return false;
                }
                if(( tapFeedback.coeffHighPass.v[ci] > lowpassHighLimit ) || ( tapFeedback.coeffHighPass.v[ci] < lowpassLowLimit )) {
                    std::cout << "\n---- check 3" << std::endl;
                    return false;
                }

                // check output
                if(( tapOutput.coeffLowPass.v[ci] > lowpassHighLimit ) || ( tapOutput.coeffLowPass.v[ci] < lowpassLowLimit ))  {
                    std::cout << "\n---- check 4" << std::endl;
                    return false;
                }
                // 0.8f -- summ of 4 combs must be less then 1.0
                if( std::abs( tapOutput.coeff.v[ci] ) >= outputLimit * ( 1.0f - tapOutput.coeffLowPass.v[ci] ))  {
                    std::cout << "\n---- check 5" << std::endl;
                    return false;
                }
                if( tapOutput.delayIndex[ci] >= (1<<( combLngExp+EbufferPar::sectionSizeExp ))) {
                    std::cout << "\n---- check 6" << std::endl;
                    return false;
                }

                // check internal
                if( std::abs( tapFeedbackInternal.coeff.v[ci] ) >= crossFeedbackLimit )  {
                    std::cout << "\n---- check 7" << std::endl;
                    return false;
                }
                if( tapFeedbackInternal.delayIndex[ci]  >= (1<<( combLngExp+EbufferPar::sectionSizeExp ))) { // 3 is practical const
                    std::cout << "\n---- check 8" << std::endl;
                    return false;
                }
            }
            return true;
        }
        MonoDelayBandpassTapArray<combCount> tapFeedback;
        MonoDelayLowpassTapArray<combCount>  tapFeedbackInternal;
        MonoDelayLowpassTapArray<combCount>  tapOutput;
    } mode01;
};

class FxLateReverb : public Fx<FxLateReverbParam>  {
public:
    typedef EDelayLineArray<FxLateReverbParam::combCount>  CombDelay;

    using MyType = FxLateReverb;
    FxLateReverb()
    :   Fx<FxLateReverbParam>()
    ,   combVector(FxLateReverbParam::combLngExp)
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

    // use internal output tap
    // low pass uses the simple multiplier form : forward multiplier = A * (1-k) !!!
    inline void process_01(void)
    {
        static_assert( param.combCount >= 8, "this uses 8 comb filters" );
        CombDelay::PackedChannel    stateOutput;
        CombDelay::PackedChannel    stateInternal;

        // calc combs 2x4 channel
        for( auto si=0u; si<sectionSize; ++si ) {
            //
            // get the output and filter - low pass
            //
            combVector.get( param.mode01.tapOutput.delayIndex, stateOutput );
            stateOutput.v4[ 0 ] = stateLowPassOutput.v4[ 0 ] * param.mode01.tapOutput.coeffLowPass.v4[ 0 ] +
                stateOutput.v4[ 0 ] * param.mode01.tapOutput.coeff.v4[ 0 ];
            stateOutput.v4[ 1 ] = stateLowPassOutput.v4[ 1 ] * param.mode01.tapOutput.coeffLowPass.v4[ 1 ] +
                stateOutput.v4[ 1 ] * param.mode01.tapOutput.coeff.v4[ 1 ];
            stateLowPassOutput.v4[ 0 ] = stateOutput.v4[ 0 ];
            stateLowPassOutput.v4[ 1 ] = stateOutput.v4[ 1 ];
            //
            // generate output
            //
            const V4vf res1 ( stateOutput.v4[ 0 ] - stateOutput.v4[ 1 ] ); // diff: lower peaks in the 1st round
            //
            // spatialize
            //
            const float r00 = res1.v[ 0 ] + res1.v[ 2 ];
            const float r11 = res1.v[ 1 ] - res1.v[ 3 ];
            out().channel[chA][si] = r00 + r11;
            out().channel[chB][si] = r00 - r11;
            //
            // do the feedback
            //
            combVector.get( param.mode01.tapFeedback.delayIndex, stateOutput );
            //
            // high pass
            //
            const CombDelay::PackedChannel tmp = stateHighPassFeedback;
            stateOutput.v4[ 0 ] += stateHighPassFeedback.v4[ 0 ] * param.mode01.tapFeedback.coeffHighPass.v4[ 0 ];
            stateOutput.v4[ 1 ] += stateHighPassFeedback.v4[ 1 ] * param.mode01.tapFeedback.coeffHighPass.v4[ 1 ];
            stateHighPassFeedback.v4[ 0 ] = stateOutput.v4[ 0 ];
            stateHighPassFeedback.v4[ 1 ] = stateOutput.v4[ 1 ];
            stateOutput.v4[ 0 ] -= tmp.v4[ 0 ];
            stateOutput.v4[ 1 ] -= tmp.v4[ 1 ];
            //
            // low pass pole
            // amplification
            //
            stateOutput.v4[ 0 ] = stateLowPassFeedback.v4[ 0 ] * param.mode01.tapFeedback.coeffLowPass.v4[ 0 ] +
                stateOutput.v4[ 0 ] * param.mode01.tapFeedback.coeff.v4[ 0 ];
            stateOutput.v4[ 1 ] = stateLowPassFeedback.v4[ 1 ] * param.mode01.tapFeedback.coeffLowPass.v4[ 1 ] +
                stateOutput.v4[ 1 ] * param.mode01.tapFeedback.coeff.v4[ 1 ];
            stateLowPassFeedback.v4[ 0 ] = stateOutput.v4[ 0 ];
            stateLowPassFeedback.v4[ 1 ] = stateOutput.v4[ 1 ];
            //
            // householder feedback  4x4 2 channels
            //
            const float hp23 = stateOutput.v[ 2 ] + stateOutput.v[ 3 ];
            const float hp01 = stateOutput.v[ 0 ] + stateOutput.v[ 1 ];
            const float hm23 = stateOutput.v[ 2 ] - stateOutput.v[ 3 ];
            const float hm01 = stateOutput.v[ 0 ] - stateOutput.v[ 1 ];
            stateOutput.v[ 0 ] =  hm01 - hp23;
            stateOutput.v[ 1 ] = -hm01 - hp23;
            stateOutput.v[ 2 ] = -hp01 + hm23;
            stateOutput.v[ 3 ] = -hp01 - hm23;

            const float hp67 = stateOutput.v[ 6 ] + stateOutput.v[ 7 ];
            const float hp45 = stateOutput.v[ 4 ] + stateOutput.v[ 5 ];
            const float hm67 = stateOutput.v[ 6 ] - stateOutput.v[ 7 ];
            const float hm45 = stateOutput.v[ 4 ] - stateOutput.v[ 5 ];
            stateOutput.v[ 4 ] =  hm45 - hp67;
            stateOutput.v[ 5 ] = -hm45 - hp67;
            stateOutput.v[ 6 ] = -hp45 + hm67;
            stateOutput.v[ 7 ] = -hp45 - hm67;
            //
            // inner cross feedback : rotate by 1 channel
            //
            combVector.getRotate( param.mode01.tapFeedbackInternal.delayIndex, stateInternal );

            // low pass
            stateInternal.v4[ 0 ] = stateLowPassInternal.v4[ 0 ] * param.mode01.tapFeedbackInternal.coeffLowPass.v4[ 0 ] +
                stateOutput.v4[ 0 ] * param.mode01.tapFeedbackInternal.coeff.v4[ 0 ];
            stateInternal.v4[ 1 ] = stateLowPassInternal.v4[ 1 ] * param.mode01.tapFeedbackInternal.coeffLowPass.v4[ 1 ] +
                stateOutput.v4[ 1 ] * param.mode01.tapFeedbackInternal.coeff.v4[ 1 ];
            stateLowPassInternal.v4[ 0 ] = stateInternal.v4[ 0 ];
            stateLowPassInternal.v4[ 1 ] = stateInternal.v4[ 1 ];
            //
            // add input and push back delay
            // inner cross feedback
            //
            stateOutput.v4[ 0 ] += inp<0>().channel[ 0 ][ si ] +
                stateInternal.v4[ 1 ] * param.mode01.tapFeedbackInternal.coeff.v4[ 1 ];
            stateOutput.v4[ 1 ] += inp<0>().channel[ 1 ][ si ] +
                stateInternal.v4[ 0 ] * param.mode01.tapFeedbackInternal.coeff.v4[ 0 ];
            combVector.push( stateOutput );
        }
    }

    // filter  states
    CombDelay::PackedChannel    stateLowPassFeedback;
    CombDelay::PackedChannel    stateHighPassFeedback;
    CombDelay::PackedChannel    stateLowPassInternal;
    CombDelay::PackedChannel    stateLowPassOutput;
    // delay line
    CombDelay                   combVector;
};


} // end namespace yacynth


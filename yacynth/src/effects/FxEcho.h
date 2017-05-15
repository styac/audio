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
 * File:   Echo.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on March 7, 2016, 10:54 AM
 */

#include    "../oscillator/Tables.h"
#include    "../utils/Limiters.h"
#include    "../effects/DelayTap.h"
#include    "FxBase.h"

// taps for testing
// #include    "../settings/EchoTaps.h"

// #include    "yacynth_globals.h"
#include    "Ebuffer.h"
#include    <array>
#include    <iostream>


using namespace limiter;

//      TODO
// templatizing
// interenal feeedback
// fill
// filtering etc
// interpolating
// internal feedback TLDD
// interpolating internal feedback
// comb feedforward, feedback,allpass
//
namespace yacynth {

class FxEchoParam {
public:
    // mandatory fields
    static constexpr char const * const name        = "Echo";
    static constexpr TagEffectType  type            = TagEffectType::FxEcho;
    static constexpr std::size_t maxMode            = 2;
    static constexpr std::size_t inputCount         = 1;

    static constexpr std::size_t tapOutputSize      = 8;
    static constexpr std::size_t tapFeedbackSize    = 8;
    static constexpr std::size_t tapOutputLPSize    = 8;
    static constexpr std::size_t tapFeedbackLPSize  = 8;

    static constexpr std::size_t delayLngExp        = 13;
    static constexpr std::size_t delayLng           = 1<<(delayLngExp+EbufferPar::sectionSizeExp);
    static constexpr std::size_t delayOffsMaxLng    = delayLng - 1;
    static constexpr std::size_t delayOffsMinLng    = EbufferPar::sectionSize * 2;

    static constexpr float lowpassLowLimit          = f2FilterOnePole_F( 18000.0 );
    static constexpr float lowpassHighLimit         = f2FilterOnePole_F( 30.0 );
    static constexpr float feedbackLimit            = 0.6f;
    static constexpr float outputLimit              = 1.0f;

    bool parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    bool check()
    {
        for( auto &v0 : tapOutput.delayIndex ) {
            for( auto &v1 : v0 ) {
                if( ( v1 > delayOffsMaxLng ) || ( v1 < delayOffsMinLng )) {
                    std::cout << "\n---- check 1"  << std::endl;
                    return false;
                }
            }
        }

        for( auto &v0 : tapFeedback.delayIndex ) {
            for( auto &v1 : v0 ) {
                if( ( v1 > delayOffsMaxLng ) || ( v1 < delayOffsMinLng )) {
                    std::cout << "\n---- check 2"  << std::endl;
                    return false;
                }
            }
        }

        for( auto &v0 : tapOutputLP.delayIndex ) {
            for( auto &v1 : v0 ) {
                if( ( v1 > delayOffsMaxLng ) || ( v1 < delayOffsMinLng )) {
                    std::cout << "\n---- check 3"  << std::endl;
                    return false;
                }
            }
        }

        for( auto &v0 : tapFeedbackLP.delayIndex ) {
            for( auto &v1 : v0 ) {
                if( ( v1 > delayOffsMaxLng ) || ( v1 < delayOffsMinLng )) {
                    std::cout << "\n---- check 4"  << std::endl;
                    return false;
                }
            }
        }

        // check coeffs
        // output abs : 0 ...< 1
        // feedback  abs : 0 ...< 1
        // pole feedback k : 0.1 ...+0.99 (check 30 Hz...14kHz ?)
        // pole A*(1-k) : A < 1.0 ->


        for( auto &v0 : tapOutput.coeff ) {
            for( auto &v1 : v0.v ) {
                if( std::abs(v1) > outputLimit ) {
                    std::cout << "\n---- check 5"  << std::endl;
                    return false;
                }
            }
        }
        for( auto &v0 : tapFeedback.coeff ) {
            for( auto &v1 : v0.v ) {
                if( std::abs(v1) > outputLimit ) {
                    std::cout << "\n---- check 6"  << std::endl;
                    return false;
                }
            }
        }

        // this should be A * (1-k) : |A| < 1.0
        for( auto &v0 : tapOutputLP.coeff ) {
            for( auto &v1 : v0.v ) {
                if( std::abs(v1) > outputLimit ) {
                    std::cout << "\n---- check 7"  << std::endl;
                    return false;
                }
            }
        }


        for( auto &v0 : tapOutputLP.coeffLowPass ) {
            for( auto &v1 : v0.v ) {
                if(( v1 < lowpassLowLimit ) || ( v1 > lowpassHighLimit )) {
                    std::cout << "\n---- check 8"  << std::endl;
                    return false;
                }
            }
        }
        // this should be A * (1-k) : |A| < 1.0
        for( auto &v0 : tapFeedbackLP.coeff ) {
            for( auto &v1 : v0.v ) {
                if( std::abs(v1) > outputLimit ) {
                    std::cout << "\n---- check 9"  << std::endl;
                    return false;
                }
            }
        }

        for( auto &v0 : tapFeedbackLP.coeffLowPass ) {
            for( auto &v1 : v0.v ) {
                if(( v1 < lowpassLowLimit ) || ( v1 > lowpassHighLimit )) {
                    std::cout << "\n---- check 10"  << std::endl;
                    return false;
                }
            }
        }

        // limit the counts
        if( tapOutputCount > tapOutputSize ) {
            tapOutputCount = tapOutputSize;
        }

        if( tapFeedbackCount > tapFeedbackSize ) {
            tapFeedbackCount = tapFeedbackSize;
        }

        if( tapOutputLPCount > tapOutputLPSize ) {
            tapOutputLPCount = tapOutputLPSize;
        }

        if( tapFeedbackLPCount > tapFeedbackLPSize ) {
            tapFeedbackLPCount = tapFeedbackLPSize;
        }

        // limit the mixer values
        if( dry[0] > 1.0f ) {
            dry[0] = 1.0f;
        } else if( dry[0] < -1.0f ) {
            dry[0] = -1.0f;
        }

        if( dry[1] > 1.0f ) {
            dry[1] = 1.0f;
        } else if( dry[1] < -1.0f ) {
            dry[1] = -1.0f;
        }
        return true;
    }
    StereoDelayTapArray<tapOutputSize>              tapOutput;
    StereoDelayTapArray<tapFeedbackSize>            tapFeedback;
    StereoDelayLowPassTapArray<tapOutputLPSize>     tapOutputLP;
    StereoDelayLowPassTapArray<tapFeedbackLPSize>   tapFeedbackLP;
    // actual value : tap...Count <= ... tap...Size
    uint8_t                                         tapOutputCount;
    uint8_t                                         tapFeedbackCount;
    uint8_t                                         tapOutputLPCount;
    uint8_t                                         tapFeedbackLPCount;
    // dry mix value for chA,chB
    float                                           dry[2];
};

// --------------------------------------------------------------------

class FxEcho : public Fx<FxEchoParam> {

public:
    using MyType = FxEcho;
    FxEcho()
    :   Fx<FxEchoParam>()
    ,   delay(FxEchoParam::delayLngExp)   // 8192 * 64 sample - 10 sec
    {
        fillSprocessv<0>(sprocess_00);
        fillSprocessv<1>(sprocess_01);
        fillSprocessv<2>(sprocess_02);
        delay.clear();
    };

    virtual bool parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex ) override;

    virtual bool connect( const FxBase * v, uint16_t ind ) override;

    static void sprocess_00( void * thp );  // bypass > inp<0> -> out
    static void sprocess_01( void * thp );
    static void sprocess_02( void * thp );

    // void clear(void) {  delay.clear(); };
    virtual void clearTransient() override ;

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
            clearTransient(); // ????
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

    inline void process_add_dry(void)
    {
        out().mult( inp<0>(), param.dry[ chA ], param.dry[ chB ] );
    }

    inline void process_clear_dry(void)
    {
        out().clear();
    }

    inline void process_echo_wet(void)
    {
        const uint32_t ind = delay.getSectionIndex(); // current
        delay.pushSection( inp<0>() );
        // delay.pushSection( inp<0>().channel[ chA ], inp<0>().channel[ chB ] );

        //
        // unfiltered taps for output
        // processing by v4sf chunks -- delay is section granulated
        //  120 bpm => beat : 0.5 sec
        //      64 * 375 / 48000 = 0.5
        // fast horizontal v4sf processing
        //
        for( auto ti = 0u; ti < param.tapOutputCount; ++ti ) {
            const V4vf coeff = param.tapOutput.coeff[ti];
            const uint32_t indA = delay.getSectionIndex( param.tapOutput.delayIndex[ ti ][ chA ] );
            const uint32_t indB = delay.getSectionIndex( param.tapOutput.delayIndex[ ti ][ chB ] );
            v4sf * chv4ApI = (v4sf *) &delay.channel[ chA ][ indA ];
            v4sf * chv4BpI = (v4sf *) &delay.channel[ chB ][ indB ];
            v4sf * chv4ApO = out().vchannel[ chA ];
            v4sf * chv4BpO = out().vchannel[ chB ];

            for( auto si = 0u; si < EbufferPar::vsectionSize; ++si ) {
                const v4sf dvA = *chv4ApI++;
                const v4sf dvB = *chv4BpI++;
                *chv4ApO++ += dvA * coeff.v2[ chA ][ chA ] + dvB * coeff.v2[ chA ][ chB ];
                *chv4BpO++ += dvA * coeff.v2[ chB ][ chA ] + dvB * coeff.v2[ chB ][ chB ];
            }
        }

        //
        // low pass filtered taps for output
        // sample level processing
        //
        for( auto ti = 0u; ti < param.tapOutputLPCount; ++ti ) {
            const V4vf coeff    = param.tapOutputLP.coeff[ ti ];
            const V4vf coeffLP  = param.tapOutputLP.coeffLowPass[ ti ];
            const uint32_t indA = delay.getSectionIndex( param.tapOutputLP.delayIndex[ ti ][ chA ] );
            const uint32_t indB = delay.getSectionIndex( param.tapOutputLP.delayIndex[ ti ][ chB ] );

            float * chvApI = &delay.channel[ chA ][ indA ];
            float * chvBpI = &delay.channel[ chB ][ indB ];
            float * chvApO = out().channel[ chA ];
            float * chvBpO = out().channel[ chB ];
            auto &scurr =  sFilterOutput[ ti ]; // current filter - 4 separate for mixing

            for( auto si = 0u; si < EbufferPar::sectionSize; ++si ) {
                const auto dvA = *chvApI++;
                const auto dvB = *chvBpI++;
                V4vf snext ( scurr.v4 * coeffLP.v4 ); // get the next state
                snext.v2[ chA ][ chA ] += dvA * coeff.v2[ chA ][ chA ];
                snext.v2[ chA ][ chB ] += dvB * coeff.v2[ chA ][ chB ];
                snext.v2[ chB ][ chA ] += dvA * coeff.v2[ chB ][ chA ];
                snext.v2[ chB ][ chB ] += dvB * coeff.v2[ chB ][ chB ];
                scurr = snext;
                *chvApO++ += snext.v2[ chA ][ chA ] + snext.v2[ chA ][ chB ];
                *chvBpO++ += snext.v2[ chB ][ chA ] + snext.v2[ chB ][ chB ];
            }
        }

        //
        // unfiltered taps for feedback
        // fast horizontal v4sf processing
        //
        for( auto ti = 0u; ti < param.tapFeedbackCount; ++ti ) {
            const V4vf coeff = param.tapFeedback.coeff[ti];
            const uint32_t indA = delay.getSectionIndex( param.tapFeedback.delayIndex[ ti ][ chA ] );
            const uint32_t indB = delay.getSectionIndex( param.tapFeedback.delayIndex[ ti ][ chB ] );
            v4sf * chv4ApI  = (v4sf *) &delay.channel[ chA ][ indA ];
            v4sf * chv4BpI  = (v4sf *) &delay.channel[ chB ][ indB ];
            v4sf * chv4ApO  = (v4sf *) &delay.channel[ chA ][ ind ];
            v4sf * chv4BpO  = (v4sf *) &delay.channel[ chB ][ ind ];

            for( auto si = 0u; si < EbufferPar::vsectionSize; ++si ) {
                const v4sf dvA = *chv4ApI++;
                const v4sf dvB = *chv4BpI++;
                *chv4ApO++ += dvA * coeff.v2[ chA ][ chA ] + dvB * coeff.v2[ chA ][ chB ];
                *chv4BpO++ += dvA * coeff.v2[ chB ][ chA ] + dvB * coeff.v2[ chB ][ chB ];
            }
        }

        //
        // low pass filtered taps for feedback
        // sample level processing
        //
        for( auto ti = 0u; ti < param.tapFeedbackLPCount; ++ti ) {
            const V4vf coeff    = param.tapFeedbackLP.coeff[ ti ];
            const V4vf coeffLP  = param.tapFeedbackLP.coeffLowPass[ ti ];
            const uint32_t indA = delay.getSectionIndex( param.tapFeedbackLP.delayIndex[ ti ][ chA ] );
            const uint32_t indB = delay.getSectionIndex( param.tapFeedbackLP.delayIndex[ ti ][ chB ] );

            float * chvApI = &delay.channel[ chA ][ indA ];
            float * chvBpI = &delay.channel[ chB ][ indB ];
            float * chvApO = &delay.channel[ chA ][ ind ];
            float * chvBpO = &delay.channel[ chB ][ ind ];
            auto &scurr =  sFilterFeedback[ ti ]; // current filter - 4 separate for mixing

            for( auto si = 0u; si < EbufferPar::sectionSize; ++si ) {
                const auto dvA = *chvApI++;
                const auto dvB = *chvBpI++;
                V4vf snext ( scurr.v4 * coeffLP.v4 ); // get the next state
                snext.v2[ chA ][ chA ] += dvA * coeff.v2[ chA ][ chA ];
                snext.v2[ chA ][ chB ] += dvB * coeff.v2[ chA ][ chB ];
                snext.v2[ chB ][ chA ] += dvA * coeff.v2[ chB ][ chA ];
                snext.v2[ chB ][ chB ] += dvB * coeff.v2[ chB ][ chB ];
                scurr = snext;
                *chvApO++ += snext.v2[ chA ][ chA ] + snext.v2[ chA ][ chB ];
                *chvBpO++ += snext.v2[ chB ][ chA ] + snext.v2[ chB ][ chB ];
            }
        }
    }

protected:
    V4vf            sFilterOutput[   FxEchoParam::tapOutputLPSize ];
    V4vf            sFilterFeedback[ FxEchoParam::tapFeedbackLPSize ];
    EDelayLine      delay;
};
// --------------------------------------------------------------------

// --------------------------------------------------------------------
} // end namespace yacynth


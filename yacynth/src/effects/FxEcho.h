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

#include "oscillator/Tables.h"
#include "utils/Limiters.h"
#include "effects/DelayTap.h"
#include "effects/FxEchoParam.h"
#include "effects/FxBase.h"

#include "Ebuffer.h"
#include <array>
#include <iostream>


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
// --------------------------------------------------------------------

class FxEcho final : public Fx<FxEchoParam> {

public:
    using MyType = FxEcho;
    FxEcho()
    :   Fx<FxEchoParam>()
    ,   delay(FxEchoParam::delayLngExp - effectFrameSizeExp)   // 8192 * 64 sample - 10 sec
    {
        delay.clear();
    };

    virtual bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex ) override;

    virtual bool connect( const FxBase * v, uint16_t ind ) override;

    static void sprocess_01( void * thp );
    static void sprocess_02( void * thp );

    // void clear() {  delay.clear(); };
    virtual void clearState() override ;

    virtual bool setSprocessNext( uint16_t mode ) override;

    inline void process_add_dry()
    {
        out().mult( inp<0>(), param.dry[ chA ], param.dry[ chB ] );
    }

    inline void process_clear_dry()
    {
        out().clear();
    }

    inline void process_echo_wet()
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
        for( auto ti = 0u; ti < param.tapOutputUsed; ++ti ) {
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
        for( auto ti = 0u; ti < param.tapOutputLPUsed; ++ti ) {
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
        for( auto ti = 0u; ti < param.tapFeedbackUsed; ++ti ) {
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
        for( auto ti = 0u; ti < param.tapFeedbackLPUsed; ++ti ) {
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
    V4vf            sFilterOutput[   FxEchoParam::tapOutputLPCount ];
    V4vf            sFilterFeedback[ FxEchoParam::tapFeedbackLPCount ];
    EDelayLine      delay;
};
// --------------------------------------------------------------------

// --------------------------------------------------------------------
} // end namespace yacynth


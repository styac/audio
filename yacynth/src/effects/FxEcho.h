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
#include    "../settings/EchoTaps.h"

#include    "yacynth_globals.h"
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
    static constexpr char const * const name = "Echo";
    static constexpr std::size_t maxMode     = 1; // 0 is always exist> 0,1,2
    static constexpr std::size_t inputCount  = 1; //  0-base signal 1-modulation

    FxEchoParam();
    
//    float   gain;
//    float   modulationIndex;
    
    // optional fields
    
};


// --------------------------------------------------------------------
// template< std::size_t outputTapCount, std::size_t delayTapCount >

class FxEcho : public Fx<FxEchoParam> {
    
public:
    static constexpr uint16_t   maxTapsExp      = 5;
    static constexpr uint16_t   maxTaps         = 1<<maxTapsExp;
    static constexpr auto       sectionSize     = EIObuffer::sectionSize;
    static constexpr auto       sectionSizeExp  = EIObuffer::sectionSizeExp;
    using TapOutputVector   =   StereoDelayTapVector< sectionSizeExp, maxTapsExp >;
    using TapFeedbackVector =   StereoDelayTapVector< sectionSizeExp, maxTapsExp >;
    using MyType = FxEcho;
    void testvect(void);

    FxEcho() = delete;

    // to be revised
    FxEcho( const uint16_t lengthExp )
    :   Fx<FxEchoParam>()
    ,   delay(lengthExp)   // 8192 * 64 sample
    {
//        fillSprocessv<0>(sprocess_00);
        
        delay.clear();
        tapOutputVector.setDelayLength( delay.bufferSize );
        tapFeedbackVector.setDelayLength( delay.bufferSize );
        testvect();
    };



//    virtual bool fill( std::stringstream& ser )         override;

    void clear(void) {  delay.clear(); };
    void mixInput( const bool v = true ) { inMix=v; };
    void setGains( const float wet, const float decay )
    {
        setGainWet(wet);
        setGainDecay(decay);
    };
    void setGainWet(  const float wet  )
    {
        gains.wet = wet;
        tapOutputVector.mult( wet );
//        if( gain < 1e-20 ) {
//            inMix = false;
//        }
    };
    void setGainDecay( const float decay )
    {
        gains.decay = decay;
        tapFeedbackVector.mult( decay );
    };

     void process(void) ;

protected:
    // ----------------------------------------
    // new version
    TapOutputVector             tapOutputVector;
    TapFeedbackVector           tapFeedbackVector;
    TapFeedbackVector           tapFilteredFeedbackVector;
    TapFeedbackVector           tapInternalFeedbackVector;
    EDelayLine                  delay;
    StereoEchoFilter            lpfilter;
    Gains                       gains;
    bool                        inMix;

}; 
// --------------------------------------------------------------------

// --------------------------------------------------------------------
} // end namespace yacynth


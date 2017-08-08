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
 * File:   ToneShaper.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on January 30, 2016, 11:45 PM
 */

// -O3 -mfpmath=sse -msse2 -msse3 -msse4.2 -ffast-math -funsafe-math-optimizations -mfma -mfma4 -mavx -funsafe-loop-optimizations -funswitch-loops -funroll-loops -fvariable-expansion-in-unroller -ftree-vectorizer-verbose=2
// -O3 -mfpmath=sse -msse2 -msse3 -msse4.2 -ffast-math -funsafe-math-optimizations -mfma -mfma4 -mavx -funsafe-loop-optimizations -funswitch-loops -fvariable-expansion-in-unroller -ftree-vectorizer-verbose=2

#include    "yacynth_globals.h"

#include    <array>
#include    <iostream>
#include    <cstring>

namespace yacynth {

//
// interpolated values for frequency dependent values
// range is 8 octaves
// dx => logaritmic
// higher freq alwazs lower
//
struct InterpolatedDecreaseU16 {
    inline int32_t get(void) const
    {
        return lowBase;
    }
    // dx = 0..FFFF
    inline int32_t get( const int16_t dx ) const
    {
        const int32_t tick = int32_t(lowBase) + ((int32_t(rate) * dx)>>15) ;
//        std::cout << "-- tick " << tick << " dx " << dx << std::endl;
        return tick;
    }

    // for tick rate is always negative: time must decrease with freq
    inline void setTickPar( int16_t lb, int16_t rt ) 
    {        
        constexpr int16_t maxLowBase    = 1<<13; // 8192*1.3 sec        
        constexpr int16_t lowLimit      = 5; // 8192*1.3 sec        
        lowBase = lb;
        rate = rt;
        if( lowBase <= 0 )
            lowBase = 0;
        if( lowBase > maxLowBase )
            lowBase = maxLowBase;
        if( lowBase < lowLimit || rt >= 0 ) { // too small or illegal
            rate = 0;
            return;
        }
        // result never will be negative -- we will see
        if( lowBase < -rate ) { // this must be revised
            rate = -lowBase;
        } 
    }
    
    // for decay rate is always positive: must increase with freq
    inline void setDecayPar( int16_t lb, int16_t rt ) 
    {
        constexpr int16_t maxLowBase = 1<<14;  // check it
        lowBase = lb;
        rate = rt;
        if( lowBase <= 0 )
            lowBase = 0;
        if( lowBase > maxLowBase )
            lowBase = maxLowBase;
        if( lowBase <= 0 || rt <= 0 ) {
            rate = 0;
            return;
        }          
    }
    
    int16_t     lowBase;    // value at low frequencies - always positive (tick, decay)
    // rate negative for tick positive for decay -- increase with freq
    int16_t     rate;       // value_high_frequencies - value_low_frequencies
};

//
// normally this increases with freq
// result could be uint64_t -- velocity can be incorporated here ?
//   ( velocity * static_cast<uint64_t>(currentEnvelopeKnot.targetValue.get() )) >> 8; 
//
struct InterpolatedAmplitudeU32 {
    static constexpr int8_t   scaleUp = 14; // this must be optimized, probably 12..14
#if 0
    inline uint64_t get( uint16_t velocity ) const
    {
        // 16 * 16 -> 25 is needed ! down by 7
        return uint64_t( uint32_t(lowBase) * velocity )<<6;
    }    
#endif    
    inline uint64_t get( uint16_t velocity, int16_t dx ) const
    {
        //                                    15 * 15 = 30 bit   *  15 = 45 - 8  = 37
        return (((uint64_t(lowBase)<<15) + ((int32_t(rate) * dx))) * velocity ) >> 8;
    }
    
    inline void setPar( uint16_t lb, int16_t rt ) 
    {
//        constexpr int16_t maxLowBase = 1<<14;  // check it
        lowBase = lb;
        rate = rt;
#if 0
        if( lowBase <= 0 )
            lowBase = 0;
        if( lowBase > maxLowBase )
            lowBase = maxLowBase;
        if( lowBase <= 0 || rt <= 0 ) {
            rate = 0;
            return;
        }          
#endif
    }
        
    uint16_t    lowBase;    // value at low frequencies
    int16_t     rate;       // value_high_frequencies - value_low_frequencies
};

//
// in legato mode :
//  release: -- test
//  transient -- test
//

struct AmplitudeTransient  {
    static constexpr const char * const typeName = "AmplitudeTransient";        
    static constexpr int8_t   curveSpeedLimit       = 3;
    static constexpr uint32_t tickLimit             = 10000;    // 13 sec
    static constexpr int8_t   amplEnvFreqDepRange   = 2;    // min max

    void clear(void)
    {
        targetValue.lowBase = 0;
        targetValue.rate    = 0x7FFF;
        tickFrame.lowBase   = 0;
        tickFrame.rate      = 0x7FFF;
        curveSpeed          = 0;        
    };

    bool check(void) const
    {
        return true;
    };

    void update( const AmplitudeTransient& val )
    {
        *this = val;
    };
    
    void setCurveSpeed( int8_t val )
    {
        if( val <= -curveSpeedLimit )
            curveSpeed = -curveSpeedLimit;
        else if( val >= curveSpeedLimit )
            curveSpeed = curveSpeedLimit;
        else 
            curveSpeed = val;            
    };

    //------- data ----------
    // target amplitude value at the end of period
    InterpolatedAmplitudeU32    targetValue;
    // frame count for the given transient part
    InterpolatedDecreaseU16     tickFrame;
    // curve : convex concave : -3 ... +3
    int8_t                      curveSpeed;
    // padding
    int8_t                      rfu1;
};

// --------------------------------------------------------------------
struct AmplitudeSustain {
    static constexpr const char * const typeName = "AmplitudeSustain";    
    enum {
        MODTYPE_INPHASE = 0,
        MODTYPE_RAND1,
        MODTYPE_RAND2,
        MODTYPE_RAND3,
    };
    void clear(void)
    {
        *this = {0};
    };
    bool check(void) const
    {
        return true;
    };
    void update( const AmplitudeSustain& val )
    {
        *this = val;
    };
    //----------------------------------
    // decayed sustain speed - interpolated by freq
    InterpolatedDecreaseU16 decayCoeff;
    // sustain integrating modulator parameters
    uint16_t        sustainModPeriod;
    uint8_t         sustainModDepth;    // modulation depth 0==disable -> sustainModDepth/256
    uint8_t         sustainModType;
    int8_t          sustainModDeltaFreq;    // speed up (+), slow down (-)
    uint8_t         sustainModDeltaCount;   // how many cycles
};

// --------------------------------------------------------------------
//template< uint16_t transientKnotCountT >
struct ToneShaper {
    static constexpr const char * const typeName = "ToneShaper";
    static constexpr uint32_t transientVectorSize = transientKnotCount;
    void clear(void)
    {
        memset(this,0,sizeof(ToneShaper));
    };
    void update( const ToneShaper& val )
    {
        *this = val;
    };
    bool dump( ToneShaper& val, size_t size )
    {
        if( sizeof(*this ) > size )
            return false;
        val = *this;
        return true;
    };
    bool check(void) const
    {
        return true;
    };

    // no implicit padding 184 = 4 + 10*16 + 10 + 4 + 2 + 4
    // pitch in ycent 
    int32_t                     pitch;
    // transient vector
    AmplitudeTransient          transient[ transientVectorSize ];
    // sustain parameters
    AmplitudeSustain            sustain;      
    // release frame - interpolated by frequency
    InterpolatedDecreaseU16     tickFrameRelease; 
    // special detune parameter -- amplitude dependent -
    int16_t                     amplitudeDetune;   
    // release curve convex concave : -3 .. +3
    int8_t                      curveSpeedRelease;
    // oscillator type : 0 = sine
    uint8_t                     oscillatorType;
    // output channel for the overtone - not implemented yet
    uint8_t                     outChannel;         // TODO : which channel to write
    // padding
    uint8_t                     rfu1;   
};
// --------------------------------------------------------------------
struct ToneShaperVector {
    static constexpr const char * const typeName = "ToneShaperVector";
    static constexpr uint16_t  idlength = 127;
    static constexpr uint32_t  toneShaperVectorSize = overtoneCountOscDef;
    inline uint16_t maxOscillatorCount(void) const { return overtoneCountOscDef; };
    ToneShaperVector()
    :   oscillatorCountUsed( overtoneCountOscDef )
    {};

    void clear(void)
    {
        for( auto& vi : toneShaper ) vi.clear();
    };
    bool check(void)
    {
        return true;
    };
    void update( const ToneShaper& val, uint16_t index )
    {
        toneShaper[ index & overtoneCountOscDefMask ].update( val );
    };

    
    ToneShaper  toneShaper[ toneShaperVectorSize ];
    uint16_t    oscillatorCountUsed; 
};
// --------------------------------------------------------------------
} // end namespace yacynth


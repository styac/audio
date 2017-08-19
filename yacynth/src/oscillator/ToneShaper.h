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

// interpolated values for frequency dependent values
// range is 8 octaves
// dx => logaritmic
//
// TICK always decreases by frequency
struct InterpolatedTick {
    static constexpr uint16_t lowBaseLimit      = 5;       
    static constexpr int8_t   curveSpeedLimit   = 3;
    void clear(void)
    {
        lowBase       = 0u;
        rate          = 0u;
        curveSpeed    = 0;        
    }

    inline uint16_t get( const uint16_t dx ) const
    {
        return lowBase - uint16_t(( uint64_t(rate) * dx * lowBase ) >> 24 );
    }

    inline void setPar( uint16_t lb, uint8_t rt, int8_t speed )
    {                
        lowBase = lb;
        rate    = rt;
        if( speed <= -curveSpeedLimit )
            curveSpeed = -curveSpeedLimit;
        else if( speed >= curveSpeedLimit )
            curveSpeed = curveSpeedLimit;
        else 
            curveSpeed = speed;            
    }
    uint16_t    lowBase;    // value at low frequencies
    uint8_t     rate;       // value_high_frequencies
    int8_t      curveSpeed;
};

// Decay always increases by frequency
struct InterpolatedDecay {
    void clear(void)
    {
        lowBase       = 0u;
        rate          = 0u;
    };

    inline uint32_t get(void) const
    {
        return lowBase;
    }

    inline uint32_t get( const uint16_t dx ) const
    {
        return lowBase + ((uint32_t(rate) * dx)>>16);
    }

    // for decay rate is always positive: must increase with freq
    inline void setPar( uint16_t lb, uint16_t rt ) 
    {
        lowBase = lb;
        rate = std::min( rt, uint16_t(0x0FFFFU - lowBase) );
    }    
    uint16_t    lowBase;    // value at low frequencies - always positive (tick, decay)
    uint16_t    rate;       // value_high_frequencies - value_low_frequencies
};

//
// in legato mode :
//  release: -- test
//  transient -- test
//

struct AmplitudeTransient  {
    static constexpr const char * const typeName = "AmplitudeTransient";        
    static constexpr uint32_t tickLimit             = 10000;    // 13 sec
    static constexpr int8_t   amplEnvFreqDepRange   = 2;    // min max

    void clear(void)
    {
        targetValue             = 0u;
        tickFrame.clear();
    };

    bool check(void) const
    {
        return true;
    };

    void update( const AmplitudeTransient& val )
    {
        *this = val;
    };
    
    // target amplitude value at the end of period
    uint32_t            targetValue;
    // frame count for the given transient part
    InterpolatedTick    tickFrame;
};

// --------------------------------------------------------------------
struct AmplitudeSustain {
    static constexpr const char * const typeName = "AmplitudeSustain";    
    void clear(void)
    {
        *this = {0};
    }
    bool check(void) const
    {
        return true;
    }
    void update( const AmplitudeSustain& val )
    {
        *this = val;
    }
    void setModParam( uint16_t depth, uint16_t deltaPhase )
    {
        constexpr uint16_t maxExp = 11;
        constexpr uint16_t maxDeltaPhase = 1<<maxExp;
        constexpr uint16_t minDeltaPhase = 4;
        constexpr uint16_t maxDepth = 1<<maxExp;
        if(( 0==depth ) || ( 0==deltaPhase )) {
            modDeltaPhase = 0;
            modDepth = 0;
            return;
        }
        if( depth > maxDepth ) {
            depth = maxDepth;
        }
        modDeltaPhase   = std::max( minDeltaPhase, std::min( deltaPhase, maxDeltaPhase ));
        modDepth        = ( depth * uint32_t(modDeltaPhase) ) >> (16-maxExp);
    }

    InterpolatedDecay   decayCoeff;
    uint16_t            modDeltaPhase;
    uint16_t            modDepth;
};

// --------------------------------------------------------------------
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
    int32_t             pitch;              // pitch in ycent +- ( 24+5 bit )
    AmplitudeTransient  transient[ transientVectorSize ];
    AmplitudeSustain    sustain;      
    InterpolatedTick    tickFrameRelease;     
    int16_t             amplitudeDetune;    // special detune parameter -- amplitude dependent -    
    uint8_t             oscillatorType;     // oscillator type : 0 = sine    
    uint8_t             outChannel;         // // output channel for the overtone - not implemented yet
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


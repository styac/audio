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
#include    "Serialize.h"

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

inline void serialize( YsifOutStream& ser, const InterpolatedDecreaseU16& val, const char * const name )
{
    serializeObjBeg(ser,name);
    serialize(ser, val.rate, "rate");
    serialize(ser, val.lowBase, "lowBase");
    serializeObjEnd(ser);
};
// --------------------------------------------------------------------

inline bool deserialize( YsifInpStream& ser, InterpolatedDecreaseU16& val, const char * const name )
{
    bool ret = true;
    ret = ret && deserializeObjBeg(ser,name);
    ret = ret && deserialize(ser, val.rate, "rate");
    ret = ret && deserialize(ser, val.lowBase, "lowBase");
    ret = ret && deserializeObjEnd(ser);
    return ret;
};

//
// normally this increases with freq
// result could be uint64_t -- velocity can be incorporated here ?
//   ( velocity * static_cast<uint64_t>(currentEnvelopeKnot.targetValue.get() )) >> 8; 
//
struct InterpolatedAmplitudeU32 {
    static constexpr int8_t   scaleUp = 14; // this must be optimized, probably 12..14
    inline uint64_t get( uint16_t velocity ) const
    {
        // 16 * 16 -> 25 is needed ! down by 7
        return uint64_t( uint32_t(lowBase) * velocity )<<6;
    }    
    
    inline uint64_t get( uint16_t velocity, int16_t dx ) const
    {
        return (((uint64_t(lowBase)<<15) + ((int32_t(rate) * dx))) * velocity ) >> 9;
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

inline void serialize( YsifOutStream& ser, const InterpolatedAmplitudeU32& val, const char * const name )
{
    serializeObjBeg(ser,name);
    serialize(ser, val.rate, "rate");
    serialize(ser, val.lowBase, "lowBase");
    serializeObjEnd(ser);
};
// --------------------------------------------------------------------

inline bool deserialize( YsifInpStream& ser, InterpolatedAmplitudeU32& val, const char * const name )
{
    bool ret = true;
    ret = ret && deserializeObjBeg(ser,name);
    ret = ret && deserialize(ser, val.rate, "rate");
    ret = ret && deserialize(ser, val.lowBase, "lowBase");
    ret = ret && deserializeObjEnd(ser);
    return ret;
};

// --------------------------------------------------------------------
//
//  tickFrame >= 0
//  if tickFrame == 0 then ignored
//
//
//  targetValue >= 0
//  -3 <= multiplierExp <= 3    : 7 different curves
//
//
//
//  N knot ( practically 16) are in a set
//  release                             : knot[15]
//  transient starts with the highest-1 : knot[14]
//  sustain should be the last          : knot[0]
//
// in legato mode :
//  release: -- test
//  transient -- test
//

// --------------------------------------------------------------------
// sustain level !!!!
// check -> max 1/4 of peek : 1<<28
// TODO freq dependent amplitude !!! direct equalisation - higher less overtones !!!
// --------------------------------------------------------------------

//
// value + value * ( ( pitch - t0 ) * k )
// pitch    - int32
// k        - int16
// fac = ( ( pitch * k   + 0x08000 ) >> 16 )
// value + ( value * fac + 0x08000 ) >> 16 )
//


struct AmplitudeTransient  {
    static constexpr int8_t   curveSpeedLimit       = 3;
    static constexpr uint32_t tickLimit             = 10000;    // 13 sec
    static constexpr int8_t   amplEnvFreqDepRange   = 2;    // min max

    bool clear(void)
    {
        targetValue.lowBase = 0;
        targetValue.rate    = 0x7FFF;
        tickFrame.lowBase   = 0;
        tickFrame.rate      = 0x7FFF;
        curveSpeed          = 0;
    };

    bool check(void) const
    {
        // sustain level !!!! - last node node[0]
        // check -> max 1/4 of peek : 1<<28

        // valid only -3..+3
        //curveSpeed = saturate<int8_t,curveSpeedLimit>(curveSpeed); // no set onlz check

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
    InterpolatedAmplitudeU32    targetValue;
    InterpolatedDecreaseU16     tickFrame;
    int8_t                      curveSpeed;         // -3 .. 0 .. +3 : curve concave-lin-convex
    int8_t                      rfu1;
};
// --------------------------------------------------------------------
inline void serialize( YsifOutStream& ser, const AmplitudeTransient& val )
{
    serializeObjBeg(ser, "AmplitudeTransient");
    serialize(ser, val.targetValue, "targetValue");
    serialize(ser, val.tickFrame, "tickFrame");
    serialize(ser, val.curveSpeed, "curveSpeed");
    serializeObjEnd(ser);
};
// --------------------------------------------------------------------
inline bool deserialize( YsifInpStream& ser, AmplitudeTransient& val )
{
    bool ret = true;
    ret = ret && deserializeObjBeg(ser, "AmplitudeTransient");
    ret = ret && deserialize(ser, val.targetValue, "targetValue");
    ret = ret && deserialize(ser, val.tickFrame, "tickFrame");
    ret = ret && deserialize(ser, val.curveSpeed, "curveSpeed");
    ret = ret && deserializeObjEnd(ser);
    return ret;
};
// --------------------------------------------------------------------
struct AmplitudeSustain {
    // static constexpr uint8_t decayCoeffCount = 8;
    enum {
        MODTYPE_INPHASE = 0,
        MODTYPE_RAND1,
        MODTYPE_RAND2,
        MODTYPE_RAND3,
    };
    bool clear(void)
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
    InterpolatedDecreaseU16 decayCoeff;
    uint16_t        sustainModPeriod;
    uint8_t         sustainModDepth;    // modulation depth 0==disable -> sustainModDepth/256
    uint8_t         sustainModType;
    int8_t          sustainModDeltaFreq;    // speed up (+), slow down (-)
    uint8_t         sustainModDeltaCount;   // how many cycles
};
// --------------------------------------------------------------------
inline void serialize( YsifOutStream& ser, const AmplitudeSustain& val )
{
    serializeObjBeg(ser,"AmplitudeSustain");
    serialize( ser, val.decayCoeff, "decayCoeff" );
    serialize( ser, val.sustainModPeriod, "sustainModPeriod" );
    serialize( ser, val.sustainModDepth, "sustainModDepth" );
    serialize( ser, val.sustainModType, "sustainModType" );
    serialize( ser, val.sustainModDeltaFreq, "sustainModDeltaFreq" );
    serialize( ser, val.sustainModDeltaCount, "sustainModDeltaCount" );
    serializeObjEnd( ser );
};
// --------------------------------------------------------------------
inline bool deserialize( YsifInpStream& ser, AmplitudeSustain& val )
{
    bool ret = true;
    ret = ret && deserializeObjBeg(ser,"AmplitudeSustain");
    ret = ret && deserialize( ser, val.decayCoeff, "decayCoeff" );
    ret = ret && deserialize( ser, val.sustainModPeriod, "sustainModPeriod" );
    ret = ret && deserialize( ser, val.sustainModDepth, "sustainModDepth" );
    ret = ret && deserialize( ser, val.sustainModType, "sustainModType" );
    ret = ret && deserialize( ser, val.sustainModDeltaFreq, "sustainModDeltaFreq" );
    ret = ret && deserialize( ser, val.sustainModDeltaCount, "sustainModDeltaCount" );
    ret = ret && deserializeObjEnd( ser );
    return ret;
};
// --------------------------------------------------------------------
//template< uint16_t transientKnotCountT >
struct ToneShaper {
    static constexpr uint32_t transientVectorSize = transientKnotCount;
#ifndef TONESHAPER_CTOR    
    // this must be elliminated 
    ToneShaper(); // obsolate - must be eliminated if init is ready or make operator=
#endif
    bool clear(void)
    {
        memset(this,0,sizeof(ToneShaper));
    };

    // update all the fields
    // this struct MUST NOT HAVE ANY DYNAMIC SH*T

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
    int32_t                     pitch;              // may be negative !! - undertones
    AmplitudeTransient          transient[ transientVectorSize ]; // transient knots
    AmplitudeSustain            sustain;            // suntain parameters
    InterpolatedDecreaseU16     tickFrameRelease;   // freq dependent but only index 0 used yet
    int16_t                     amplitudeDetune;   
    int8_t                      curveSpeedRelease;
    uint8_t                     oscillatorType;
    uint8_t                     outChannel;         // TODO : which channel to write
    uint8_t                     rfu1;   
};
// --------------------------------------------------------------------

inline void serialize( YsifOutStream& ser, const ToneShaper& val )
{
    serializeComment(ser) << "-----------" << std::endl;
    serializeObjBeg(ser,"ToneShaper");
    serialize( ser, val.pitch, "pitch" );
    serialize( ser, val.tickFrameRelease, "tickFrameRelease"  );
    serialize( ser, val.amplitudeDetune, "amplitudeDetune" );
    serialize( ser, val.curveSpeedRelease, "curveSpeedRelease"  );
    serialize( ser, val.oscillatorType, "oscillatorType" );
    serialize( ser, val.outChannel, "outChannel" );
    uint16_t index = 0;
    for( auto& v :  val.transient ) {
        serializeVecBeg( ser, index, "indexTransientVector", ToneShaper::transientVectorSize-1 );
        serialize( ser, v );
        serializeVecEnd( ser, index==ToneShaper::transientVectorSize-1 );
        ++index;
    };
    serialize( ser, val.sustain );
    serializeObjEnd( ser );
};
// --------------------------------------------------------------------
inline bool deserialize( YsifInpStream& ser, ToneShaper& val )
{
    bool ret = true;
    ret = ret && deserializeObjBeg(ser,"ToneShaper");
    ret = ret && deserialize( ser, val.pitch, "pitch" );
    ret = ret && deserialize( ser, val.tickFrameRelease, "tickFrameRelease"  );
    ret = ret && deserialize( ser, val.amplitudeDetune, "amplitudeDetune" );
    ret = ret && deserialize( ser, val.curveSpeedRelease, "curveSpeedRelease"  );
    ret = ret && deserialize( ser, val.oscillatorType, "oscillatorType" );
    ret = ret && deserialize( ser, val.outChannel, "outChannel" );
    uint32_t index = 0;
    // TODO replace direct indexing
    for( auto& v :  val.transient ) {
        bool last;
        ret = ret && deserializeVecBeg( ser, index, "indexTransientVector", ToneShaper::transientVectorSize-1 );
        // TODO set index
        ret = ret && deserialize( ser, v );
        ret = ret && deserializeVecEnd(ser, last);
        if( !ret ) {
            // end of vector - try continue
            break;
        }
    }
    ret = ret && deserialize( ser, val.sustain );
    ret = ret && deserializeObjEnd( ser );
    return ret;
};
// --------------------------------------------------------------------


// this will be saved to the DB
//template< uint16_t overtoneCountOfOscillator >
struct ToneShaperVector {
    static constexpr uint16_t  idlength = 127;
    static constexpr uint32_t  toneShaperVectorSize = overtoneCountOscDef;


    inline uint16_t maxOscillatorCount(void) const { return overtoneCountOscDef; };
    ToneShaperVector()
    :   oscillatorCountUsed( overtoneCountOscDef )
    {
        // test for the new system
        // SAWTOOTH LIKE THE OLD
        genFreqNtimes(); // step 1 if the same result
        genAmpl1pN();

    };
    
    // OBSOLATE
    static int32_t relFreq2pitch( double relf ) {
        return std::round( std::log2( relf ) * ycentNorm );
    };
    bool clear(void)
    {
        for( auto& vi : toneShaperVec ) vi.clear();
    };
    bool check(void)
    {
        return true;
    };
    void update( const ToneShaper& val, uint16_t index )
    {
        toneShaperVec[ index & overtoneCountOscDefMask ].update( val );
    };
    // ----------------------------------
    // OBSOLATE
    // generator helper funcs for testing
    // copy 0 to 1..N
    void genO0toAll(void)
    {
        for( auto vi = 1u; vi < overtoneCountOscDef; ++vi ) {
            toneShaperVec[vi].update( toneShaperVec[0] );
        }
    }
    // ----------------------------------
    // generator helper funcs for testing
    // OBSOLATE
    // pitch[ k ] = k* pitch[0]
    void genFreqNtimes(void)
    {
        for( auto vi = 0u; vi < overtoneCountOscDef; ++vi ) {
            toneShaperVec[vi].pitch = relFreq2pitch( vi+1 );
        }
    }
    // ----------------------------------
    // generator helper funcs for testing
    // OBSOLATE
    // amplitudes = 1/n
    void genAmpl1pN(void)
    {
        for( auto vi = 0u; vi < overtoneCountOscDef; ++vi ) {
            const float onevi = 1.0f/float(vi+1);
            for( auto ni = 0u; ni < transientKnotCount; ++ni ) {
                toneShaperVec[vi].transient[ni].targetValue.lowBase *= onevi;
            }
        }
    }

    ToneShaper  toneShaperVec[ toneShaperVectorSize ];
    uint16_t    oscillatorCountUsed; 
};
// --------------------------------------------------------------------
inline void serialize( YsifOutStream& ser, const ToneShaperVector& val )
{
    serializeObjBeg( ser, "ToneShaperVector" );
    //serialize( ser, val.oscillatorCountUsed, "oscillatorCountUsed" );
    uint32_t index = 0;
    for( auto& v : val.toneShaperVec ) {
        serializeVecBeg( ser, index, "indexToneShaperVector", ToneShaperVector::toneShaperVectorSize-1 );
        serialize(ser, v);
        serializeVecEnd( ser, index==ToneShaperVector::toneShaperVectorSize-1 );
        ++index;
    }
    serializeObjEnd(ser);
};
// --------------------------------------------------------------------
inline bool deserialize( YsifInpStream& ser, ToneShaperVector& val )
{
    bool ret = true;
    //ret = ret && deserialize( ser, val.oscillatorCountUsed, "oscillatorCountUsed" );
    ret = ret && deserializeObjBeg( ser, "ToneShaperVector" );
    uint32_t index = 0;
    for( auto& v : val.toneShaperVec ) {
        bool last;
        ret = ret && deserializeVecBeg( ser, index, "indexToneShaperVector", ToneShaperVector::toneShaperVectorSize-1 );
        ret = ret && deserialize( ser, v );
        ret = ret && deserializeVecEnd(ser,last);
    }
    ret = ret && deserializeObjEnd(ser);
    return ret;
};
// --------------------------------------------------------------------

// --------------------------------------------------------------------
} // end namespace yacynth


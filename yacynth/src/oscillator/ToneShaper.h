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
 * File:   Envelope.h
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

namespace yacynth {

//
// interpolated values for frequency dependent values
// range is 8 octaves
// dx => logaritmic
//
template< typename T, std::size_t rlim  >
struct Interpolated {
    static constexpr float rateLimitHigh = rlim;
    static constexpr float rateLimitLow  = 1.0f/rlim;

    Interpolated()
    :   rate(1.0f)
    {};

    // rate for 8 octaves
    inline void setRate( const float r )
    {
        rate = r;
        if( rateLimitHigh < r ) {
            rate = rateLimitHigh;
        } else if( rateLimitLow > r ) {
            rate = rateLimitLow;
        }
        set();
    }

    inline void set( const T low )
    {
        lf  = low;
        set();
    }

    inline T get(void) const
    {
        return lf;
    }

    // temp for testing
    inline void mult( const float m )
    {
        lf *= m;
        set();
    }

    inline void set(void)
    {
        const int64_t hf = std::max((static_cast<float>(lf) * rate ), static_cast<float>( std::numeric_limits<T>::max()));
        diff = hf - lf;
    }

    inline T get( const uint16_t dx ) const
    {
        return lf + (( diff * dx ) >> 16 );
    }

    int64_t diff;   // value_high_frequencies - value_low_frequencies
    float   rate;   // hf = rate*lf   -- practically for 8 octaves
    T       lf;     // value at low frequencies
};


template< typename T,std::size_t S>
inline void serialize( YsifOutStream& ser, const Interpolated<T,S>& val, const char * const name )
{
    serializeObjBeg(ser,name);
    serialize(ser, val.rate, "rate");
    serialize(ser, val.lf, "lowFreq");
    serializeObjEnd(ser);
};
// --------------------------------------------------------------------
template< typename T,std::size_t S>
inline bool deserialize( YsifInpStream& ser, Interpolated<T,S>& val, const char * const name )
{
    bool ret = true;
    ret = ret && deserializeObjBeg(ser,name);
    ret = ret && deserialize(ser, val.rate, "rate");
    ret = ret && deserialize(ser, val.lf, "lowFreq");
    ret = ret && deserializeObjEnd(ser);
    val.set();
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
        targetValue.set(0);
        targetValue.setRate(1.0f);
        tickFrame.set(0);
        tickFrame.setRate(1.0f);
        amplitudeDetune = 0;
        curveSpeed = 0;
    };

    bool check(void) const
    {
        // sustain level !!!! - last node node[0]
        // check -> max 1/4 of peek : 1<<28

        // valid only -3..+3
        //curveSpeed = saturate<int8_t,curveSpeedLimit>(curveSpeed); // no set onlz check

        return true;
    };
    void set( const uint32_t target, uint16_t tick, int8_t curve, uint32_t freqDep = 0 )
    {
        clear();
        targetValue.set( target );
        tickFrame.set( std::min( uint32_t(tick), tickLimit ) ); // std::min wants the same types
        curveSpeed  = saturate<int8_t,curveSpeedLimit>(curve);
//        curveSpeed  = curve <= -curveSpeedLimit ? -curveSpeedLimit : curve >= curveSpeedLimit ? curveSpeedLimit : curve;
    };
    void update( const AmplitudeTransient& val )
    {
        *this = val;
    };

    // input is amplitude 39 bit
    inline int32_t getDetune( int64_t amplitude ) const
    {
        constexpr int  rangeExp = 41; // 23 bit -- TODO must be tested -> max +- 1/2 octave ???
        return ( amplitude * amplitudeDetune ) >> rangeExp;
    }

    //------- data ----------
    Interpolated<uint32_t,256> targetValue;
    Interpolated<uint16_t,8> tickFrame;
    int16_t     amplitudeDetune;    // TODO: pitch + amplitudeDetune * amplitude >> ??
    int8_t      curveSpeed;         // -3 .. 0 .. +3 : curve concave-lin-convex
    int8_t      rfu1;               // padding
};
// --------------------------------------------------------------------
inline void serialize( YsifOutStream& ser, const AmplitudeTransient& val )
{
    serializeObjBeg(ser, "AmplitudeTransient");
    serialize(ser, val.targetValue, "targetValue");
    serialize(ser, val.tickFrame, "tickFrame");
    serialize(ser, val.amplitudeDetune, "amplitudeDetune");
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
    ret = ret && deserialize(ser, val.amplitudeDetune, "amplitudeDetune");
    ret = ret && deserialize(ser, val.curveSpeed, "curveSpeed");
    ret = ret && deserializeObjEnd(ser);
    return ret;
};
// --------------------------------------------------------------------
struct AmplitudeSustain {
    static constexpr uint8_t decayCoeffCount = 8;
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

    // TODO -- considering this -> test it
    // after transient first
    uint16_t    antiDecayCoeff; // used as (1 + antiDecayCoeff/64k)
    uint16_t    antiDecayCyles; // antiDecayCyles * 1.3 msec
    //----------------------------------
    Interpolated<uint16_t,decayCoeffCount> decayCoeff;

    uint16_t    sustainModPeriod;
    uint8_t     sustainModDepth;    // modulation depth 0==disable -> sustainModDepth/256
    // 0 = no randomization
    // 1.....255 = diff randomization
    uint8_t     sustainModType;
    int8_t      sustainModDeltaFreq;    // speed up (+), slow down (-)
    uint8_t     sustainModDeltaCount;   // how many cycles
};
// --------------------------------------------------------------------
inline void serialize( YsifOutStream& ser, const AmplitudeSustain& val )
{
    //serialize( ser, val.sermagic );
    serializeObjBeg(ser,"AmplitudeSustain");
    serialize( ser, val.antiDecayCoeff, "antiDecayCoeff" );
    serialize( ser, val.antiDecayCyles, "antiDecayCyles" );
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
    ret = ret && deserialize( ser, val.antiDecayCoeff, "antiDecayCoeff" );
    ret = ret && deserialize( ser, val.antiDecayCyles, "antiDecayCyles" );
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
    ToneShaper(); // obsolate - must be eliminated if init is ready or make operator=
    bool clear(void)
    {
        pitch = 0;
        oscillatorType = 0;
        sustain.clear();
        for( auto& iv : transient ) iv.clear();
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

    int32_t                 pitch;              // may be negative !! - undertones
    AmplitudeTransient      transient[transientVectorSize]; // transient knots
    AmplitudeSustain        sustain;            // suntain parameters
    Interpolated<uint16_t,8>
                            tickFrameRelease;   // freq dependent but only index 0 used yet
    int8_t                  curveSpeedRelease;
    uint8_t                 oscillatorType;

    uint8_t                 outChannel;         // TODO : which channel to write
    uint8_t                 rfu1;
    uint8_t                 rfu2;
    uint8_t                 rfu3;
};
// --------------------------------------------------------------------

inline void serialize( YsifOutStream& ser, const ToneShaper& val )
{
    serializeComment(ser) << "-----------" << std::endl;
    serializeObjBeg(ser,"ToneShaper");
    serialize( ser, val.pitch, "pitch" );
    serialize( ser, val.tickFrameRelease, "tickFrameRelease"  );
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
    // pitch[ k ] = k* pitch[0]
    void genFreqNtimes(void)
    {
        for( auto vi = 0u; vi < overtoneCountOscDef; ++vi ) {
            toneShaperVec[vi].pitch = relFreq2pitch( vi+1 );
        }
    }
    // ----------------------------------
    // generator helper funcs for testing
    // amplitudes = 1/n
    void genAmpl1pN(void)
    {
        for( auto vi = 0u; vi < overtoneCountOscDef; ++vi ) {
            const float onevi = 1.0f/float(vi+1);
            for( auto ni = 0u; ni < transientKnotCount; ++ni ) {
                toneShaperVec[vi].transient[ni].targetValue.mult( onevi );
            }
        }
    }

    ToneShaper  toneShaperVec[ toneShaperVectorSize ];
    uint16_t    oscillatorCountUsed;    // how many are used with this shaper
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

} // end namespace yacynth


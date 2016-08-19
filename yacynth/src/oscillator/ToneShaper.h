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
inline void serialize( std::stringstream& ser, const Interpolated<T,S>& val )
{
    serialize(ser, val.rate);
    serialize(ser, val.lf);
};
// --------------------------------------------------------------------
template< typename T,std::size_t S>
inline bool deserialize( std::stringstream& ser, Interpolated<T,S>& val )
{
    bool ret = true;
    ret = ret && deserialize(ser, val.rate);
    ret = ret && deserialize(ser, val.lf);
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
    static constexpr Sermagic sermagic = "AMTR:01";
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
    
    bool check(void)
    {
        // sustain level !!!! - last node node[0]
        // check -> max 1/4 of peek : 1<<28

        // valid only -3..+3
        curveSpeed = saturate<int8_t,curveSpeedLimit>(curveSpeed);

        return true;
    };
    void set( const uint32_t target, uint16_t tick, int8_t curve, uint32_t freqDep = 0 )
    {
        clear();
<<<<<<< HEAD
        targetValueLF = target;
        tickFrameLF= std::min( uint32_t(tick), tickLimit ); // std::min wants the same types
=======
        targetValue.set( target );
        tickFrame.set( std::min( uint32_t(tick), tickLimit ) ); // std::min wants the same types
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
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
        return ( amplitude * amplitudeDetune ) >> 41; // 23 bit -- TODO must be tested -> max +- 1/2 octave ???
    }

    //------- data ----------
<<<<<<< HEAD
    uint32_t    targetValueLF;   //freq dependent amplitude
    uint32_t    targetValueHF;   //freq dependent amplitude    
    uint16_t    tickFrameLF;     // number of frames (64 samples at 48kHz: 1.3msec) - freq dependent
    uint16_t    tickFrameHF;     // number of frames (64 samples at 48kHz: 1.3msec) - freq dependent
    int16_t     amplitudeDetune;    // pitch + amplitudeDetune * amplitude >> ??
=======
    Interpolated<uint32_t,256> targetValue;
    Interpolated<uint16_t,8> tickFrame;
    int16_t     amplitudeDetune;    // TODO: pitch + amplitudeDetune * amplitude >> ??
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
    int8_t      curveSpeed;         // -3 .. 0 .. +3 : curve concave-lin-convex
    int8_t      rfu1;               // padding
};
// --------------------------------------------------------------------
inline void serialize( std::stringstream& ser, const AmplitudeTransient& val )
{
    serialize(ser, val.sermagic);
<<<<<<< HEAD
    serialize(ser, val.targetValueLF);
    serialize(ser, val.targetValueHF);
    serialize(ser, val.tickFrameLF);
    serialize(ser, val.tickFrameHF);
=======
    serialize(ser, val.targetValue);
    serialize(ser, val.tickFrame);
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
    serialize(ser, val.amplitudeDetune);
    serialize(ser, val.curveSpeed);
};
// --------------------------------------------------------------------
inline bool deserialize( std::stringstream& ser, AmplitudeTransient& val )
{
    bool ret = true;
    ret = ret && deserialize(ser, val.sermagic);
<<<<<<< HEAD
    ret = ret && deserialize(ser, val.targetValueLF);
    ret = ret && deserialize(ser, val.targetValueHF);
    ret = ret && deserialize(ser, val.tickFrameLF);
    ret = ret && deserialize(ser, val.tickFrameHF);
=======
    ret = ret && deserialize(ser, val.targetValue);
    ret = ret && deserialize(ser, val.tickFrame);
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
    ret = ret && deserialize(ser, val.amplitudeDetune);
    ret = ret && deserialize(ser, val.curveSpeed);
    return ret;
};
// --------------------------------------------------------------------
struct AmplitudeSustain {
    static constexpr Sermagic sermagic = "AMSU:01";
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
    bool check(void)
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
<<<<<<< HEAD
    uint16_t    decayCoeffLF;
    uint16_t    decayCoeffHF;
=======
    Interpolated<uint16_t,8> decayCoeff;
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
    
    uint16_t    sustainModPeriod;
    uint8_t     sustainModDepth;    // modulation depth 0==disable -> sustainModDepth/256
    // 0 = no randomization
    // 1.....255 = diff randomization
    uint8_t     sustainModType;
    int8_t      sustainModDeltaFreq;    // speed up (+), slow down (-)
    uint8_t     sustainModDeltaCount;   // how many cycles
};
// --------------------------------------------------------------------
inline void serialize( std::stringstream& ser, const AmplitudeSustain& val )
{
    serialize( ser, val.sermagic );
    serialize( ser, val.antiDecayCoeff );
    serialize( ser, val.antiDecayCyles );
    serialize( ser, val.decayCoeffLF );
    serialize( ser, val.decayCoeffHF );
    serialize( ser, val.sustainModPeriod );
    serialize( ser, val.sustainModDepth );
    serialize( ser, val.sustainModType );
    serialize( ser, val.sustainModDeltaFreq );
    serialize( ser, val.sustainModDeltaCount );
};
// --------------------------------------------------------------------
inline bool deserialize( std::stringstream& ser, AmplitudeSustain& val )
{
    bool ret = true;
    ret = ret && deserialize( ser, val.sermagic );
    ret = ret && deserialize( ser, val.antiDecayCoeff );
    ret = ret && deserialize( ser, val.antiDecayCyles );
    ret = ret && deserialize( ser, val.decayCoeffLF );
    ret = ret && deserialize( ser, val.decayCoeffHF );
    ret = ret && deserialize( ser, val.sustainModPeriod );
    ret = ret && deserialize( ser, val.sustainModDepth );
    ret = ret && deserialize( ser, val.sustainModType );
    ret = ret && deserialize( ser, val.sustainModDeltaFreq );
    ret = ret && deserialize( ser, val.sustainModDeltaCount );
    return ret;
};
// --------------------------------------------------------------------
//template< uint16_t transientKnotCountT >
struct ToneShaper {
    static constexpr Sermagic sermagic = "TSOT:01";
    ToneShaper(); // obsolate
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

    bool check(void)
    {
        return true;
    };

<<<<<<< HEAD
    int32_t             pitch;              // may be negative !! - undertones
    AmplitudeTransient  transient[transientKnotCount];
    AmplitudeSustain    sustain;
    uint16_t            tickFrameReleaseLF;
    uint16_t            tickFrameReleaseHF;    // TODO : change
    int8_t              curveSpeedRelease;      // TODO : change
    uint8_t             oscillatorType;

    uint8_t             outChannel;         // TODO : which channel to write
    uint8_t             rfu1;
    uint8_t             rfu2;
    uint8_t             rfu3;
=======
    int32_t                 pitch;              // may be negative !! - undertones
    AmplitudeTransient      transient[transientKnotCount];
    AmplitudeSustain        sustain;
    Interpolated<uint16_t,8>  
                            tickFrameRelease;
    int8_t                  curveSpeedRelease;
    uint8_t                 oscillatorType;

    uint8_t                 outChannel;         // TODO : which channel to write
    uint8_t                 rfu1;
    uint8_t                 rfu2;
    uint8_t                 rfu3;
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262

};
// --------------------------------------------------------------------
inline void serialize( std::stringstream& ser, const ToneShaper& val )
{
    serialize( ser, val.sermagic );
    serialize( ser, val.pitch );
    serialize( ser, val.tickFrameReleaseLF );
    serialize( ser, val.tickFrameReleaseHF );
    serialize( ser, val.curveSpeedRelease );
    serialize( ser, val.oscillatorType );
    serialize( ser, val.outChannel );
    for( auto& v :  val.transient) serialize( ser, v );
    serialize( ser, val.sustain );
};
// --------------------------------------------------------------------
inline bool deserialize( std::stringstream& ser, ToneShaper& val )
{
    bool ret = true;
    ret = ret && deserialize( ser, val.sermagic );
    ret = ret && deserialize( ser, val.pitch );
    ret = ret && deserialize( ser, val.tickFrameReleaseLF );
    ret = ret && deserialize( ser, val.tickFrameReleaseHF );
    ret = ret && deserialize( ser, val.curveSpeedRelease );
    ret = ret && deserialize( ser, val.oscillatorType );
    ret = ret && deserialize( ser, val.outChannel );
    for( auto& v :  val.transient) ret = ret && deserialize( ser, v );
    ret = ret && deserialize( ser, val.sustain );
    return ret;
};
// --------------------------------------------------------------------

// key:
//  <type>-<format>-<name>-<datetime>
//
struct DbKey {
    char    dbType[8];      // TSVC:01:
    char    dbName[105];    // "BLABLALALLLA       "
    char    dbVer[15];      // ":YYDDDSSSSSNNNN
};
// this will be saved to the DB
//template< uint16_t overtoneCountOfOscillator >
struct ToneShaperVector {
    static constexpr Sermagic sermagic = "TSVC:01";
    static constexpr const char * const keymagic = "00-ToneShaperVector-01:";
    static constexpr uint16_t  idlength = 127;
    inline uint16_t maxOscillatorCount(void) const { return overtoneCountOscDef; };
    ToneShaperVector()
    :   obid("-------------obid---------------")
    ,   zero('\0')
    ,   oscillatorCountUsed( overtoneCountOscDef )
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
<<<<<<< HEAD
                toneShaperVec[vi].transient[ni].targetValueLF *= onevi;
=======
                toneShaperVec[vi].transient[ni].targetValue.mult( onevi );
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
            }
        }
    }

    ToneShaper  toneShaperVec[ overtoneCountOscDef ];
    uint16_t    oscillatorCountUsed;    // how many are used with this shaper
    char        obid[ idlength ];
    char        zero;
};
// --------------------------------------------------------------------
inline void serialize( std::stringstream& ser, const ToneShaperVector& val )
{
    serializeBegin( ser );
    serialize( ser, val.sermagic );
    serialize( ser, val.oscillatorCountUsed );
    Vecindex vi;
    for( auto& v : val.toneShaperVec ) {
        serialize(ser, vi) ;
        serialize(ser, v) ;
        vi.inc();
    }
    serializeEnd(ser);
};
// --------------------------------------------------------------------
inline bool deserialize( std::stringstream& ser, ToneShaperVector& val )
{
    bool ret = true;
    deserializeBegin(ser);
    ret = ret && deserialize( ser, val.sermagic );
    ret = ret && deserialize( ser, val.oscillatorCountUsed );
    Vecindex vi;
    for( auto& v : val.toneShaperVec ) {
        ret = ret && deserialize(ser, vi) ;
        ret = ret && deserialize(ser, v) ;
  std::cout << "ind " << vi.i << std::endl;
    }
    return ret;
};
// --------------------------------------------------------------------

} // end namespace yacynth


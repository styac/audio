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


#include    "yacynth_globals.h"
#include    "Serialize.h"

#include    <array>
#include    <iostream>

namespace yacynth {

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


// envelope frequency dependency:
// map the highest 6 bits of the pitch
//  high nibble (4) - octave - 1 bit under is a half octave
//  but only 8 octave is interesting: 20Hz -- 4096 Hz
//  0 ..40
//  1 ..80
//  2 ..160
//  3 ..320
//  4 ..640
//  5 ..1280
//  6 ..2560
//  7 ---
//
// this should be somehow coded here
// TODO: envelope pitch dependency
/*

    j 40 freq2ycent 15c56c23 PitchDep 0
    j 56 freq2ycent 1641b112 PitchDep 1
    j 80 freq2ycent 16c56c23 PitchDep 2
   j 112 freq2ycent 1741b112 PitchDep 3
   j 160 freq2ycent 17c56c23 PitchDep 4
   j 224 freq2ycent 1841b112 PitchDep 5
   j 320 freq2ycent 18c56c23 PitchDep 6
   j 448 freq2ycent 1941b112 PitchDep 7
   j 640 freq2ycent 19c56c23 PitchDep 8
   j 896 freq2ycent 1a41b112 PitchDep 9
  j 1280 freq2ycent 1ac56c23 PitchDep a
  j 1792 freq2ycent 1b41b112 PitchDep b
  j 2560 freq2ycent 1bc56c23 PitchDep c
  j 3584 freq2ycent 1c41b112 PitchDep d
  j 5120 freq2ycent 1cc56c23 PitchDep e

  j 5400 freq2ycent 1cd91653 PitchDep e
  j 5430 freq2ycent 1cdb2224 PitchDep e

  j 7168 freq2ycent 1d41b112 PitchDep f
 *


*/

// --------------------------------------------------------------------
// sustain level !!!!
// check -> max 1/4 of peek : 1<<28
// TODO freq dependent amplitude !!! direct equalisation - higher less overtones !!!
// --------------------------------------------------------------------

struct AmplitudeTransient  {
    static constexpr Sermagic sermagic = "AMTR:01";
    static constexpr uint32_t minPitch = 0x155b2c3e;       // freq2ycent( 40.0 ) should be about 40 Hz
    static constexpr uint32_t maxPitch = 0x1cd91653;       // freq2ycent( 6144.0 ) should be about 5 kHz -- 5400 Hz
    static constexpr uint16_t amplEnvFreqDepRange   = 16;
    static constexpr int8_t   curveSpeedLimit       = 3;
    static constexpr uint32_t tickLimit             = 10000;

    bool clear(void)
    {
        *this = {0};
    };
    bool check(void)
    {
        // sustain level !!!! - last node node[0]
        // check -> max 1/4 of peek : 1<<28

        // valid only -3..+3
        curveSpeed = saturate<int8_t,curveSpeedLimit>(curveSpeed);

        for( auto i=0u; i < amplEnvFreqDepRange; ++i ) {
            tickFrame[i] = std::min( uint32_t(tickFrame[i]), tickLimit );
        }
        return true;
    };
    void set( const uint32_t target, uint16_t tick, int8_t curve, uint32_t freqDep = 0 )
    {
        clear();
        targetValue[0] = target;
        tickFrame[0]= std::min( uint32_t(tick), tickLimit ); // std::min wants the same types
        curveSpeed  = saturate<int8_t,curveSpeedLimit>(curve);
//        curveSpeed  = curve <= -curveSpeedLimit ? -curveSpeedLimit : curve >= curveSpeedLimit ? curveSpeedLimit : curve;
        for( auto i=1u; i < amplEnvFreqDepRange; ++i ) {
            tickFrame[i] = std::min( (freqDep * tickFrame[i-1]) >> 14, tickLimit );
        }
    };
    void update( const AmplitudeTransient& val )
    {
        *this = val;
    };
    // TODO : TEST
    // TODO : amplitude freq dependency

    inline uint16_t envelopePitchDepTick( const int8_t ind, const uint16_t dx )
    {
        if( 0 == tickFrame[0] || 0 == tickFrame[1] || ind < 0 )     // means: no frequency dependency or low freqs
            return tickFrame[0];    // otherwise all [1]..[15] MUST BE FILLED with data
        if( 15 <= ind ) {
            return tickFrame[15];
        }
        const int32_t y0  = tickFrame[ ind ];
        const int32_t res = y0 + ((( tickFrame[ ind + 1 ] - y0 ) * dx ) >> 16 );
        return res > 0 ? res : 1;
    };

    inline uint32_t envelopePitchDepAmplitude( const int8_t ind, const uint16_t dx )
    {
        if( 0 == targetValue[0] || 0 == targetValue[1] || ind < 0 )     // means: no frequency dependency or low freqs
            return targetValue[0];    // otherwise all [1]..[15] MUST BE FILLED with data
        if( 15 <= ind ) {
            return targetValue[15];
        }
        const int64_t y0  = targetValue[ ind ];
        const int64_t res = y0 + ((( targetValue[ ind + 1 ] - y0 ) * dx ) >> 16 );
        return res > 0 ? res : 0;
    };

    // input is amplitude 48 bit
    inline int32_t getDetune( int64_t amplitude ) const
    {
        return ( amplitude * amplitudeDetune ) >> 41; // 23 bit -- TODO must be tested -> max +- 1/2 octave ???
    }

    //------- data ----------
    uint32_t    targetValue[amplEnvFreqDepRange];  //freq dependent amplitude
    uint16_t    tickFrame[amplEnvFreqDepRange]; // number of frames (64 samples at 48kHz: 1.3msec) - freq dependent
    int16_t     amplitudeDetune;    // pitch + amplitudeDetune * amplitude >> ??
    int8_t      curveSpeed;     // -3 .. 0 .. +3 : curve concave-lin-convex
    int8_t      rfu1;            // padding
};
// --------------------------------------------------------------------
inline void serialize( std::stringstream& ser, const AmplitudeTransient& val )
{
    serialize(ser, val.sermagic);
    for(auto& v :  val.targetValue) serialize( ser, v );
    serializeSpace(ser);
    for(auto& v :  val.tickFrame)   serialize( ser, v );
    serializeSpace(ser);
    serialize(ser, val.amplitudeDetune);
    serialize(ser, val.curveSpeed);
};
// --------------------------------------------------------------------
inline bool deserialize( std::stringstream& ser, AmplitudeTransient& val )
{
    bool ret = true;
    ret = ret && deserialize(ser, val.sermagic);
    for(auto& v : val.targetValue)  ret = ret && deserialize(ser, v);
    for(auto& v : val.tickFrame)    ret = ret && deserialize(ser, v);
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
    uint16_t    decayCoeff;
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
    serialize( ser, val.decayCoeff );
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
    ret = ret && deserialize( ser, val.decayCoeff );
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

    int32_t             pitch;              // may be negative !! - undertones
    AmplitudeTransient  transient[transientKnotCount];
    AmplitudeSustain    sustain;
    uint16_t            tickFrameRelease;   // TODO : change
    int8_t              curveSpeedRelease;  // TODO : change
    uint8_t             oscillatorType;
};
// --------------------------------------------------------------------
inline void serialize( std::stringstream& ser, const ToneShaper& val )
{
    serialize( ser, val.sermagic );
    serialize( ser, val.pitch );
    serialize( ser, val.tickFrameRelease );
    serialize( ser, val.curveSpeedRelease );
    serialize( ser, val.oscillatorType );
    for( auto& v :  val.transient) serialize( ser, v );
    serialize( ser, val.sustain );
};
// --------------------------------------------------------------------
inline bool deserialize( std::stringstream& ser, ToneShaper& val )
{
    bool ret = true;
    ret = ret && deserialize( ser, val.sermagic );
    ret = ret && deserialize( ser, val.pitch );
    ret = ret && deserialize( ser, val.tickFrameRelease );
    ret = ret && deserialize( ser, val.curveSpeedRelease );
    ret = ret && deserialize( ser, val.oscillatorType );
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
                toneShaperVec[vi].transient[ni].targetValue[0] *= onevi;
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


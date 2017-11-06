#pragma once
/*
 * Copyright (C) 2017 ist
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
 * File:   TuningConst.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on October 5, 2017, 6:01 PM
 */

#include    <cstdint>
#include    <cmath>
#include    <algorithm>

namespace Tuning {

constexpr double centOctave         = 1200;
constexpr double ycentOctave        = 1<<24;
constexpr double cent2ycent         = ycentOctave / centOctave;
constexpr double ycent2cent         = centOctave / ycentOctave;

constexpr double ycentET12Semitone  = ycentOctave / 12;
constexpr double makeRate( double nom, double denom ) { return nom/denom; };
constexpr double interval2ycent( double interval ) { return std::log2(interval) * ycentOctave; };
constexpr double interval2ycent( double nom, double denom ) { return std::log2(nom/denom) * ycentOctave; };

constexpr double ycentTritave       = interval2ycent( 3.0 );        // Bohlen–Pierce scale
constexpr double ycentFifth         = interval2ycent( 3.0 / 2.0 );  //
constexpr double ycentFourth        = interval2ycent( 4.0 / 3.0 );  //
constexpr double ycentThirdMaj      = interval2ycent( 5.0 / 4.0 );  //
constexpr double ycentThirdMin      = interval2ycent( 6.0 / 5.0 );  //

template < typename T >
struct Ratio {
    T n; // numerator
    T d; // denumerator
};

// 21 - octave 2:1
// 31 - tritave 3:1 -- Bohlen–Pierce scale
// 54, 65, alpha,beta,delta
// ET - Equal tempered
// JI - Just intonation
// XX - Extra intonation

enum class TuningType : uint8_t {
    TM_NIL,
    
    TM_M_ET_N_USER1,      // any equal tempered
    TM_M_ET_N_USER2,      // any equal tempered
    TM_M_ET_N_USER3,      // any equal tempered
    TM_M_ET_N_USER4,      // any equal tempered

    TM_M_JI_N_USER1,      // any just interval
    TM_M_JI_N_USER2,      // any just interval
    TM_M_JI_N_USER3,      // any just interval
    TM_M_JI_N_USER4,      // any just interval

    TM_21_ET_5,
    TM_21_ET_7,
    TM_21_ET_9,
    TM_21_ET_10,
    TM_21_ET_12,
    TM_21_ET_15,
    TM_21_ET_17,
    TM_21_ET_19,
    TM_21_ET_22,
    TM_21_ET_24,
    TM_21_ET_27,
    TM_21_ET_29,
    TM_21_ET_31,
    TM_21_ET_34,
    TM_21_ET_41,
    TM_21_ET_53,
    TM_21_ET_72,

    TM_31_ET_13,  // Bohlen–Pierce scale

    // Wendy Carlos
    //  http://www.wendycarlos.com/resources/pitch.html
    TM_32_ET_9,     // alpha
    TM_32_ET_11,    // beta
    TM_32_ET_20,    // delta

    TM_43_ET_X,
    TM_54_ET_X,
    TM_65_ET_X,

    // extend with Wendy Carlos + gamelan, shruti ....
    TM_21_XX_Shruti,
    TM_21_XX_Gamelan,

    TM_M_JI_N,      // any just intonation
    TM_21_JI_PARTCH_43 = 0x40,
    TM_21_JI_PTOLEMY_12,
};

// different 12 key/octave keyboards
// MIDI keyboards are always mapped to integral index (n*microtonal)

enum class TuningVariation : uint8_t {
    TV_LINEAR,      // fill continously - may be ok for non octave centric or non keyboard usage
    TV_OPTIMAL,     // built in pattern
    TV_CUSTOM,      // given by pattern
};

// generator for any equal tempered
struct TuningGeneratorET {
    TuningGeneratorET() = delete;
    const uint32_t intervalCount;
    const double rate;
    const double periodYcent;
    const double deltaIntervalYcent;

    TuningGeneratorET( uint32_t intervalCount, uint32_t rateNom = 2, uint32_t rateDenom = 1 )
    :   intervalCount( intervalCount )
    ,   rate( double( rateNom ) / double( rateDenom ))
    ,   periodYcent(interval2ycent(rate))
    ,   deltaIntervalYcent( periodYcent / intervalCount )
    {}

    inline double getYcentN( uint32_t N ) const
    {
        return ( N % intervalCount ) * deltaIntervalYcent;
    };

    inline uint16_t getPeriod( uint32_t N ) const
    {
        return N / intervalCount;
    };
};

// generator for any just intonation
template < uint16_t s >
struct TuningGeneratorJI {
    static constexpr TuningType MyType = TuningType::TM_M_JI_N;
    static constexpr uint16_t size = s;
    double rate;
    uint16_t intervalCount;    // intervalCount <= size
    bool set( uint16_t rateNom, uint16_t rateDenom, uint16_t intervalCount )
    {
        if( intervalCount < 1 || intervalCount >= size ) {
            return false;
        }
        this->intervalCount = intervalCount;
        rate = makeRate(rateNom, rateDenom);
        return true;
    }

//    bool setInterval( uint16_t rateNom, uint16_t rateDenom, uint16_t index )
//    {
//
//        if( intervalCount < 1 || intervalCount >= size ) {
//            return false;
//        }
//        this->intervalCount = intervalCount;
//        rate = makeRate(rateNom, rateDenom);
//        return true;
//    }

    double getYcentN( uint8_t N )
    {
        const auto index = N % intervalCount;
        const auto k = notes[index];
        return interval2ycent( k.n, k.d );
    }
    Ratio<uint16_t> notes[ size ];
};

// predefined generators
template< TuningType typeIdentifier >
struct TuningGenerator {};

// octave
template<>
struct TuningGenerator<TuningType::TM_21_JI_PARTCH_43> {
    static constexpr TuningType MyType = TuningType::TM_21_JI_PARTCH_43;
    static constexpr uint16_t intervalCount = 43;
    static constexpr double rate = 2.0/1.0;
    static constexpr double getYcentN( uint8_t N ) { return notes[ N % intervalCount ]; };
    static constexpr double notes[ intervalCount ] = {
// ===========
        interval2ycent(          1.0 ),    // 00 - C
        interval2ycent(  81.0 / 80.0 ),    // 01
        interval2ycent(  33.0 / 32.0 ),    // 02
        interval2ycent(  21.0 / 20.0 ),    // 03
        interval2ycent(  16.0 / 15.0 ),    // 04 - C#
        interval2ycent(  12.0 / 11.0 ),    // 05
        interval2ycent(  11.0 / 10.0 ),    // 06
        interval2ycent(  10.0 /  9.0 ),    // 07
        interval2ycent(   9.0 /  8.0 ),    // 08 - D
        interval2ycent(   8.0 /  7.0 ),    // 09
// ===========
        interval2ycent(   7.0 /  6.0 ),    // 10
        interval2ycent(  32.0 / 27.0 ),    // 11
        interval2ycent(   6.0 /  5.0 ),    // 12 - Eb
        interval2ycent(  11.0 /  9.0 ),    // 13
        interval2ycent(   5.0 /  4.0 ),    // 14 - E
        interval2ycent(  14.0 / 11.0 ),    // 15
        interval2ycent(   9.0 /  7.0 ),    // 16
        interval2ycent(  21.0 / 16.0 ),    // 17
        interval2ycent(   4.0 /  3.0 ),    // 18 - F
        interval2ycent(  27.0 / 20.0 ),    // 19
// ===========
        interval2ycent(  11.0 /  8.0 ),    // 20
        interval2ycent(   7.0 /  5.0 ),    // 21
        interval2ycent(  10.0 /  7.0 ),    // 22
        interval2ycent(  16.0 / 11.0 ),    // 23
        interval2ycent(  40.0 / 27.0 ),    // 24
        interval2ycent(   3.0 /  2.0 ),    // 25 - G
        interval2ycent(  32.0 / 21.0 ),    // 26
        interval2ycent(  14.0 /  9.0 ),    // 27
        interval2ycent(  11.0 /  7.0 ),    // 28
        interval2ycent(   8.0 /  5.0 ),    // 29
// ===========
        interval2ycent(  18.0 / 11.0 ),    // 30
        interval2ycent(   5.0 /  3.0 ),    // 31 - A
        interval2ycent(  27.0 / 16.0 ),    // 32
        interval2ycent(  12.0 /  7.0 ),    // 33
        interval2ycent(   7.0 /  4.0 ),    // 34 - H
        interval2ycent(  16.0 /  9.0 ),    // 35
        interval2ycent(   9.0 /  5.0 ),    // 36
        interval2ycent(  20.0 / 11.0 ),    // 37
        interval2ycent(  11.0 /  6.0 ),    // 38
        interval2ycent(  15.0 /  8.0 ),    // 39
// ===========
        interval2ycent(  40.0 / 21.0 ),    // 40
        interval2ycent(  64.0 / 33.0 ),    // 41
        interval2ycent( 160.0 / 81.0 )     // 42
// ===========
    };
};

template<>
struct TuningGenerator<TuningType::TM_21_JI_PTOLEMY_12> {
    static constexpr TuningType MyType = TuningType::TM_21_JI_PTOLEMY_12;
    static constexpr uint16_t intervalCount = 12;
    static constexpr double rate = 2.0/1.0;
    static constexpr double getYcentN( uint8_t N ) { return notes[ N % intervalCount ]; };
    static constexpr double notes[ intervalCount ] = {
// ===========
        interval2ycent(          1.0 ),    // 00 - C
        interval2ycent(  16.0 / 15.0 ),    // 01
        interval2ycent(   9.0 /  8.0 ),    // 02
        interval2ycent(   6.0 /  5.0 ),    // 03
        interval2ycent(   5.0 /  4.0 ),    // 04
        interval2ycent(   4.0 /  3.0 ),    // 05
        interval2ycent(  45.0 / 32.0 ),    // 06
        interval2ycent(   3.0 /  2.0 ),    // 07
        interval2ycent(   8.0 /  5.0 ),    // 08
        interval2ycent(   5.0 /  3.0 ),    // 09
        interval2ycent(   9.0 /  5.0 ),    // 10
        interval2ycent(  15.0 /  8.0 ),    // 11
    };
};

template<>
struct TuningGenerator<TuningType::TM_21_ET_72> {
    static constexpr TuningType MyType = TuningType::TM_21_ET_72;
    static constexpr uint16_t intervalCount = 72;
    static constexpr double rate = 2.0/1.0;
    static constexpr double deltaInterval = ycentOctave / intervalCount;
    static constexpr double getYcentN( uint8_t N ) { return  (N % intervalCount) * deltaInterval;};
};
#if 0
template<>
struct TuningGenerator<TuningType::TM_21_ET_53> {
    static constexpr TuningType MyType = TuningType::TM_21_ET_53;
    static constexpr uint16_t intervalCount = 53;
    static constexpr double rate = 2.0/1.0;
    static constexpr double deltaInterval = ycentOctave / intervalCount;
    static constexpr double getYcentN( uint8_t N ) { return  (N % intervalCount) * deltaInterval;};
};

template<>
struct TuningGenerator<TuningType::TM_21_ET_41> {
    static constexpr TuningType MyType = TuningType::TM_21_ET_41;
    static constexpr uint16_t intervalCount = 41;
    static constexpr double rate = 2.0/1.0;
    static constexpr double deltaInterval = ycentOctave / intervalCount;
    static constexpr double getYcentN( uint8_t N ) { return  (N % intervalCount) * deltaInterval;};
};

template<>
struct TuningGenerator<TuningType::TM_21_ET_34> {
    static constexpr TuningType MyType = TuningType::TM_21_ET_34;
    static constexpr uint16_t intervalCount = 34;
    static constexpr double rate = 2.0/1.0;
    static constexpr double deltaInterval = ycentOctave / intervalCount;
    static constexpr double getYcentN( uint8_t N ) { return  (N % intervalCount) * deltaInterval;};
};

template<>
struct TuningGenerator<TuningType::TM_21_ET_22> {
    static constexpr TuningType MyType = TuningType::TM_21_ET_22;
    static constexpr uint16_t intervalCount = 22;
    static constexpr double rate = 2.0/1.0;
    static constexpr double deltaInterval = ycentOctave / intervalCount;
    static constexpr double getYcentN( uint8_t N ) { return  (N % intervalCount) * deltaInterval;};
};

template<>
struct TuningGenerator<TuningType::TM_21_ET_19> {
    static constexpr TuningType MyType = TuningType::TM_21_ET_19;
    static constexpr uint16_t intervalCount = 19;
    static constexpr double rate = 2.0/1.0;
    static constexpr double deltaInterval = ycentOctave / intervalCount;
    static constexpr double getYcentN( uint8_t N ) { return  (N % intervalCount) * deltaInterval;};
};

template<>
struct TuningGenerator<TuningType::TM_21_ET_17> {
    static constexpr TuningType MyType = TuningType::TM_21_ET_17;
    static constexpr uint16_t intervalCount = 17;
    static constexpr double rate = 2.0/1.0;
    static constexpr double deltaInterval = ycentOctave / intervalCount;
    static constexpr double getYcentN( uint8_t N ) { return  (N % intervalCount) * deltaInterval;};
};

template<>
struct TuningGenerator<TuningType::TM_21_ET_15> {
    static constexpr TuningType MyType = TuningType::TM_21_ET_15;
    static constexpr uint16_t intervalCount = 15;
    static constexpr double rate = 2.0/1.0;
    static constexpr double deltaInterval = ycentOctave / intervalCount;
    static constexpr double getYcentN( uint8_t N ) { return  (N % intervalCount) * deltaInterval;};
};
#endif
// tritave


} // end namespace Tuning


#if 0
notation

{

KEYS 12
KREF K.1 MIDI 65
FREF K.1 440.0

ASSIGN ET72 T.1 .. T.72
# ASSIGN YCENT 654616
# ASSIGN CENT 100.000
# ASSIGN RATE N / D

K.1.bbb     T.4
K.1.bb
K.1.b
K.1         T.7
K.1.#
K.1.##
K.1.###     T.9

K.11.bbb
K.11.bb
K.11.b
K.11
K.11.#
K.11.##
K.11.###

K.12.bbb
K.12.bb
K.12.b
K.12
K.12.#
K.12.##
K.12.###

}

#endif
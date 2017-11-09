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
#include    <array>

namespace Tuning {

constexpr double centOctave         = 1200;
constexpr double ycentOctave        = 1<<24;
constexpr double cent2ycent         = ycentOctave / centOctave;
constexpr double ycent2cent         = centOctave / ycentOctave;

constexpr double ycentET12Semitone  = ycentOctave / 12;
constexpr double makeRate( double nom, double denom ) { return nom/denom; };
constexpr double interval2ycent( double interval ) { return std::log2(interval) * ycentOctave; };
constexpr double interval2ycent( double nom, double denom ) { return std::log2(nom/denom) * ycentOctave; };


// 21 - octave 2:1
// 31 - tritave 3:1 -- Bohlen–Pierce scale
// 54, 65, alpha,beta,delta
// ET - Equal tempered
// JI - Just intonation
// XX - Extra intonation

enum class TuningType : uint16_t {
    TM_NIL,    
    TM_M_CUSTOM,        // any set by direct dump
    TM_M_ET_CUSTOM,     // any equal tempered
    TM_M_JI_CUSTOM,     // any just interval

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

    // Bohlen–Pierce scale
    TM_31_ET_13,

    // Wendy Carlos
    //  http://www.wendycarlos.com/resources/pitch.html
    TM_32_ET_9,     // alpha
    TM_32_ET_11,    // beta
    TM_32_ET_20,    // delta

    TM_43_ET_X,
    TM_54_ET_X,
    TM_65_ET_X,
    // -----------------
    TM_21_Werckmeister3,
    TM_21_Young2,       // Thomas Young 2
    TM_21_Pythagorean,
    TM_21_Ptolemy,
    TM_21_Partch43,
};

// different 12 key/octave keyboards
// MIDI keyboards are always mapped to integral index (n*microtonal)

enum class TuningVariation : uint16_t {
    TV_LINEAR,      // fill continously - may be ok for non octave centric or non keyboard usage
    TV_OPTIMAL,     // built in pattern
    TV_CUSTOM,      // given by pattern
};

// generator for any equal tempered
struct TuningGeneratorET {
    TuningGeneratorET() = delete;
    const uint16_t intervalCount;  
    const double rate;             
    const double deltaIntervalYcent; 

    TuningGeneratorET( uint32_t intervalCount, uint32_t rateNom = 2, uint32_t rateDenom = 1 )
    :   intervalCount( intervalCount )
    ,   rate( double( rateNom ) / double( rateDenom ))
    ,   deltaIntervalYcent( getYcentPeriod() / intervalCount )
    {}

    inline double getYcentN( uint16_t N ) const
    {
        return ( N % intervalCount ) * deltaIntervalYcent;
    };

    inline uint16_t getPeriod( uint16_t N ) const
    {
        return N / intervalCount;
    };
    
    inline double getYcentPeriod() const
    {
        return interval2ycent(rate);
    }
};

// generator for any just intonation
template < uint16_t arraySize >
struct TuningGenerator {
    TuningGenerator() = delete;
    static constexpr uint16_t size = arraySize;
    typedef std::array<double,size> arrayType;
    const uint16_t intervalCount;   // intervalCount <= size
    const double rate;              // cleanup sequence
    const arrayType notes;

    TuningGenerator( uint32_t rateNom, uint32_t rateDenom, uint32_t ic, const arrayType& noteList )
    :   intervalCount( ic <= size ? ic : size )
    ,   rate( double( rateNom ) / double( rateDenom ) )    
    ,   notes(noteList)
    {}

    inline double getYcentN( uint16_t N ) const
    {
        return notes[ N % intervalCount ];
    }

    inline uint16_t getPeriod( uint16_t N ) const
    {
        return N / intervalCount;
    };

    inline double getYcentPeriod() const
    {
        return interval2ycent(rate);
    }
};

typedef struct TuningGenerator<12> TuningGenerator12Notes;
typedef struct TuningGenerator<43> TuningGenerator43Notes;

} // end namespace Tuning

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

#include <cstdint>
#include <cmath>
#include <array>

#include <iostream>

namespace Tuning {

constexpr double centOctave         = 1200;
constexpr double ycentOctave        = 1<<24;
constexpr double cent2ycent         = ycentOctave / centOctave;
constexpr double ycent2cent         = centOctave / ycentOctave;

constexpr double ycentET12Semitone  = ycentOctave / 12;
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
    TM_M_JI_CUSTOM,     // any just interval
    TM_M_ET_CUSTOM,     // any equal tempered
    TM_21_JI_CUSTOM,     // any just interval
    TM_21_ET_CUSTOM,     // any equal tempered
    
    TM_21_ET_5,
    TM_21_ET_7,
    TM_21_ET_9,
    TM_21_ET_10,        // half TM_21_ET_5
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
    
    TM_21_Pelog1,
    
    // for testing 
    TM_test
};

// different 12 key/octave keyboards
// MIDI keyboards are always mapped to integral index (n*microtonal)

enum class TuningVariation : uint16_t {
    TV_CUSTOM,              // given by pattern
    TV_LINEAR_1layer,       // 1 layer -- fill continously 1 octave is 12 key or non octave contiguous
    TV_LINEAR_2layer,       // tuning/layer count specific 
    TV_LINEAR_3layer,       // tuning/layer count specific 
    TV_LINEAR_4layer,       // tuning/layer count specific 
    TV_LINEAR_5layer,       // tuning/layer count specific 
    TV_LINEAR_6layer,       // tuning/layer count specific 
    TV_LINEAR_7layer,       // tuning/layer count specific 
    TV_LINEAR_8layer,       // tuning/layer count specific     
    TV_LINEAR_24Key,        // 1 layer -- fill continously 1 octave is 24 key 
    TV_OPTIMAL,             // built in pattern
};

enum class ModalScales : uint16_t {
    MS_JustIntonation,      // just intonation approx
};

class TuningGenerator {
    static constexpr auto maxSize = 128u;
    
private:
    TuningGenerator() = delete;
    const uint32_t intervalCount;  
    const double rate;          
    const double deltaIntervalYcentET; 
    const double * const deltaIntervalYcent; 

public:
    // for stretched 
    static constexpr double octaveTolerance = 0.1;

    // ctor ET
    TuningGenerator( uint32_t ic, uint32_t rateNom, uint32_t rateDenom )
    :   intervalCount( ic )
    ,   rate(double( rateNom ) / double( rateDenom ) )
    ,   deltaIntervalYcentET( getYcentPeriod() / intervalCount )
    ,   deltaIntervalYcent(nullptr)
    {}
    
    // ctor ET
    TuningGenerator( uint32_t ic, double rate )
    :   intervalCount( ic )
    ,   rate( rate )
    ,   deltaIntervalYcentET( getYcentPeriod() / intervalCount )
    ,   deltaIntervalYcent(nullptr)
    {}

    // ctor table -- DOESN'T CONTAIN THE 0.0 element
    TuningGenerator( uint32_t ic, uint32_t rateNom, uint32_t rateDenom, const double * noteList )
    :   intervalCount( ic + 1 )
    ,   rate(double( rateNom ) / double( rateDenom ) )
    ,   deltaIntervalYcentET( 0.0 )
    ,   deltaIntervalYcent( noteList )
    {}

    // ctor table -- DOESN'T CONTAIN THE 0.0 element
    TuningGenerator( uint32_t ic, double rate, const double * noteList )
    :   intervalCount( ic + 1 )
    ,   rate( rate )
    ,   deltaIntervalYcentET( 0.0 )
    ,   deltaIntervalYcent( noteList )
    {}

    double getYcentN( uint32_t N ) const
    {
        const auto NN = N % intervalCount;
        if( NN == 0 ) { // 1st interval is always the base = 0
            return 0.0;
        }
        if( deltaIntervalYcent == nullptr ) {            
            return NN * deltaIntervalYcentET;
        } 
        return deltaIntervalYcent[ NN - 1 ];
    };
    
    inline double getYcentPeriod() const
    {
        return interval2ycent(rate);
    }

    inline auto getIntervalCount() const
    {
        return intervalCount;
    }
    
    inline auto getRate( uint32_t N ) const
    {
        return rate;
    }
    
    bool isOctave() const
    {
        return ((2.0-octaveTolerance) < rate ) && ((2.0+octaveTolerance) > rate );
    }
};


// modal scales: subset of a scale
template< uint8_t maxSize >
class ModalSteps {
private:    
    static constexpr uint8_t size = maxSize;
    const TuningGenerator& tuningGenerator;
    const uint8_t stepCount; 
    bool  ok;
    std::array<uint16_t, size> step;
    ModalSteps() = delete;

public:
    ModalSteps( const TuningGenerator& tg, uint8_t count, const uint8_t * const stepList )
    :   tuningGenerator(tg)
    ,   stepCount(count)
    ,   ok(false)
    {
        if( count > size || count < 2 ) {
            return;
        }
        uint16_t next = 0;
        for( auto i=0u; i < count; ++i ) {
            next += stepList[ i ];  // 1st step would be 0.0
            step[ i ] = next;
            std::cout
                << " i: " << i
                << " step: " << step[ i ]                    
                << " stepList: " << uint16_t(stepList[ i ])
                << " next: " << next
                << std::endl;
        }
        if( next > tuningGenerator.getIntervalCount() ) {
            return;
        }
        ok =  true;
        std::cout << " ok " << std::endl;
    }
    
    ~ModalSteps() = default;

    inline bool isOK() const { return ok; }

    // TODO : test
    double getYcentN( uint32_t N ) const
    {
        const auto NN = N % stepCount;
        if( ( NN == 0 ) || ( ! ok ) ) { // 1st interval is always the base = 0
            return 0.0;
        }
        return tuningGenerator.getYcentN( step[ NN - 1 ] ) ;
    };
};

// normal keyboard
typedef ModalSteps<12> ModalSteps12;

// 2 keyboards with 2 layers or double octave
typedef ModalSteps<24> ModalSteps24;

} // end namespace Tuning

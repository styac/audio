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
 * File:   Lfo.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on March 11, 2016, 1:42 PM
 */

// sine table: 64k x 16 bit
#include    "../oscillator/Tables.h"
#include    "../oscillator/BaseOscillator.h"
#include    "../utils/Fastsincos.h"
#include    "../utils/GaloisNoiser.h"

#include    <iostream>

using namespace noiser;

namespace yacynth {

class LfoRandom : public BaseOscillator {
public:
    LfoRandom()
    :   BaseOscillator(0)
    {};

    LfoRandom( const uint32_t delta  )
    :   BaseOscillator( delta )
    {};

    // noise phase modulation test
    inline void inc(void) {
        phase     += phaseDelta;
        phase     += rand;
//        rand        = int32_t( GALOIS_SHIFTER_SHIFT>>28 ) >> 8;
//        rand        = int32_t( GALOIS_SHIFTER_SHIFT>>24 ) >> 9;
    };
private:
    int32_t rand;
};

// TODO revisit

class Lfo : public BaseOscillator {
public:
    enum  wave_t : uint16_t {
        NIL,
        SIN,
        SAW,
        SQUARE,
        TRIANGLE,
        // new
        SIN_PHASE0,
        SIN_PHASE1,
        SIN_PHASE2,
        SIN_PHASE3,


        // COS_PHASE0,
        // COS_PHASE1,
        // COS_PHASE2,
        // COS_PHASE3,

        SAW_PHASE0,
        SAW_PHASE1,
        SAW_PHASE2,
        SAW_PHASE3,

        ANGLE3_PHASE0,
        ANGLE3_PHASE1,
        ANGLE3_PHASE2,
        ANGLE3_PHASE3,

        SQUARE_PHASE0,
        SQUARE_PHASE1,
        SQUARE_PHASE2,
        SQUARE_PHASE3,
                // etc
    };
    enum  phase_t : uint16_t {  // obsolate
        PHASE0,
        PHASE1,
        PHASE2,
        PHASE3,
    };

    Lfo()
    {};

    Lfo( const uint32_t delta, const uint16_t rng = 0 )
    :   BaseOscillator( delta )
    ,   rangeExp(rng)
    {};
    inline void set( const uint32_t delta, const wave_t typeP, const uint16_t rng = 0, const uint16_t pboost = 0 ) {
        phaseDelta  = delta;
        type        = typeP;
        rangeExp    = rng;
    };
    inline void setRange( const uint16_t rng ) {
        rangeExp    = rng < 16 ? rng : 16;
    };
    inline void reset( void ) {
        phase           = 0;
    };
    inline int64_t get( const phase_t phaseType ) {
        return get( phaseType, type );
    };
    inline int64_t get( void ) {
        return get( PHASE0, type );
    };

    // -1.0 .. +1.0
    float   getFloat( const phase_t phaseType, const wave_t type ) {
        return float( get( phaseType, type ) >> rangeExp ) * ( 1.0 / (1L<<31) );
    }
    float   getFloatOffs( const phase_t phaseType, const wave_t type ) {
        return float( get( phaseType, type ) >> rangeExp ) * ( 1.0 / (1L<<32) ) + 0.5;
    }

    int64_t get( const phase_t phaseType, const wave_t type ) {
        int32_t dphase = phase;
        switch( phaseType ) {
        case PHASE1:    dphase += phase1; break;
        case PHASE2:    dphase += phase2; break;
        case PHASE3:    dphase -= phase1; break;
        }
        switch ( type ) {
        case NIL:
            return 0;
        case SIN: {
            uint16_t ind = dphase >> 16;
            const int64_t y0 = waveSinTable[ ind ];
            return ( (  ( ( y0 << 16 ) + int64_t(( waveSinTable[ ++ind ] - y0 ) ) * uint16_t(dphase) ) << rangeExp ) );
            }
        case SAW:
            return ( int64_t(dphase) | 1 ) << rangeExp ;
        case SQUARE:
            return ( int64_t(( dphase  >> 31 ) | 1 ) << 31 ) << rangeExp;
        case TRIANGLE: {
            dphase += phase1;
            return (( int64_t(((( dphase | 1 ) * (((( dphase | 1 ) >> 31 ) << 1 ) | 1 ))) - 0x3FFFFFFF ) << (rangeExp + 1)) )  ;
            }
        }
    };

private:
    uint16_t    rangeExp;
    wave_t      type;
};

} // end namespace yacynth


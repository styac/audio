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
 * File:   Fastexp.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on March 31, 2016, 9:45 AM
 */

#include    <cstdint>
#include    <cmath>
#include    <algorithm>

namespace tables {

constexpr uint32_t   expTableSize = 1<<16;
extern float    expMinus2PI[ expTableSize + 1 ];
extern uint64_t exp2Table[   expTableSize + 1 ];


template< typename Tdata, int64_t limit>
inline Tdata saturate ( const Tdata x )
{
    return std::min( Tdata(limit), std::max( Tdata(-limit), x ));
}

class ExpTable;

extern const ExpTable& expTable;

class ExpTable {
public:
    static constexpr double     PI = 3.141592653589793238462643383279502884197169399375105820974944592307816;
    static constexpr int        precMultExp = 60;
    static constexpr float      frange = 1.0f / (1LL<<precMultExp);

    static inline const ExpTable& getInstance(void)
    {
        static ExpTable instance;
        return instance;
    }

    static inline uint64_t exp2index0(     const uint16_t index )
    {
        return exp2Table[index];
    }
    static inline uint64_t exp2index1(     const uint16_t index ) 
    {
        return exp2Table[uint32_t(index)+1];
    }
    static inline float expMinus2PIindex0( const uint16_t index )
    {
        return expMinus2PI[index];
    }
    static inline float expMinus2PIindex1( const uint16_t index )
    {
        return expMinus2PI[uint32_t(index)+1];
    }
    static inline const auto& expMinus2PIRef(void)
    {
        return expMinus2PI;
    }
    static inline const auto& exp2Ref(void)
    {
        return exp2Table;
    }

    static inline uint64_t exp2min1( uint32_t logv )
    {
        static uint64_t y0 = exp2Table[ 0 ];
        const uint16_t ind = logv >> 8;
        const uint64_t y1 = exp2Table[ ind ];

        return y1-y0;
    } // end expi2( uint32_t logv )

    static inline uint64_t expi2( uint32_t logv )
    {
        const uint16_t  ind = logv >> 8;
        uint16_t        xpo = logv >> 24;
        if( xpo > precMultExp ) {
            xpo = precMultExp ;
        }
        const uint64_t y1 = exp2Table[ ind ] ;
        const uint64_t dy = (( exp2Table[ ind + 1] - y1 ) * ( logv & 0x0FF )) >> 8;
        const uint64_t rs = ( y1 + dy ) >> ( precMultExp - xpo );
        return rs;
    } // end expi2( uint32_t logv )

// --------------------------------------------------------------------
// directly calculates deltaFi
// kills too high and low frequencies result 0 -> stop oscillator

    static inline uint32_t ycent2deltafi( const uint32_t base )
    {
        constexpr uint32_t  ref19900ycent   = 0x1ebacfd9;
        constexpr uint32_t  ref0_01ycent    = 0x9ce2e7f;
        if( ( ref19900ycent < base ) || ( ref0_01ycent > base ) )
            return 0;   //  nuke the high and  low freqs

        const uint32_t  logv = base;
        const uint16_t  ind = logv >> 8;
        const uint16_t  xpo = logv >> 24;
        const uint64_t  y1 = exp2Table[ ind ];
        const uint64_t  dy = (( exp2Table[ ind + 1 ] - y1 ) * ( logv & 0x0FF )) >> 8;
        const uint64_t  rs = ( y1 + dy ) >> ( precMultExp - xpo );
        return rs ;
    } // end ycent2deltafi    
// --------------------------------------------------------------------
// directly calculates deltaFi
// kills too high and low frequencies result 0 -> stop oscillator

    static inline uint32_t ycent2deltafi( const uint32_t base,  int32_t delta )
    {
        constexpr int32_t   maxDelta        = 0x08000000;
        constexpr uint32_t  ref19900ycent   = 0x1ebacfd9;
        constexpr uint32_t  ref0_01ycent    = 0x9ce2e7f;
        if( ( ref19900ycent < base ) || ( ref0_01ycent > base ) )
            return 0;   //  nuke the high and  low freqs

        const uint32_t  pitch = base + saturate<int32_t,maxDelta>( delta );
        const uint32_t  logv = pitch > ref19900ycent ? ref19900ycent : pitch;
        const uint16_t  ind = logv >> 8;
        const uint16_t  xpo = logv >> 24;
        const uint64_t  y1 = exp2Table[ ind ];
        const uint64_t  dy = (( exp2Table[ ind + 1 ] - y1 ) * ( logv & 0x0FF )) >> 8;
        const uint64_t  rs = ( y1 + dy ) >> ( precMultExp - xpo );
        return rs ;
    } // end ycent2deltafi

    // valid between 0 .. 0.5
    static inline float fastexpMinus2PI( const float x )
    {
        if( x <= 0.0f )
            return 1.0f;
        if( x >= 0.49999f )
            return float( std::exp( -PI ));

        return expMinus2PI[std::lround( x * ( expTableSize<<1 ))];
    };

    // MUST BE TESTED
    static inline uint32_t fastexpMinus2PI( const uint32_t x )
    {
        constexpr   uint32_t rangeExp = 31;
        if( x <= 0 )
            return 1;
        if( x >= (1L<<rangeExp) )
            return float( std::exp( -PI )) * (1L<<rangeExp);
        return std::lround( expMinus2PI[ x>>15 ] * (1L<<rangeExp) );
    };

    // valid between 0 .. 0.5
    static inline float fastexpMinus( const float x )
    {
        if( x <= 0.0f )
            return 1.0f;
        if( x >= 0.49999f )
            return float( std::exp( -0.5 ));
        return expMinus2PI[std::lround( x * ( expTableSize<<1 ) * float( 0.5 / PI ))];
    };

protected:
    ExpTable()
    {
        constexpr double delta  = -PI / expTableSize;
        constexpr double dexp   = 1.0L / double(expTableSize);
        constexpr uint64_t norm = 1LL << precMultExp;
        for( uint32_t i = 0u; i <= expTableSize; ++i ) {
            // exp( -2 * PI *x ) ; x = 0 .. 0.5
            expMinus2PI[ i ]    = std::exp( delta * i );
            // exp2 x = 1..2
            exp2Table[ i ]      = uint64_t( std::llround( std::pow( 2.0L, i * dexp ) * norm ) );
        }
    };
};
} // end namespace tables


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

#include <cstdint>
#include <cmath>
#include <algorithm>
#include <iostream>


namespace tables {

constexpr uint32_t   expTableSize = 1<<16;
constexpr uint32_t   velocityBoostTableSize = 1<<8;

extern float    expMinus2PI[ expTableSize + 1 ];
extern uint64_t exp2Table[   expTableSize + 1 ];
extern uint8_t  velocityBoostTable[];

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
    static constexpr uint8_t    precMultExp = 60;
    static constexpr float      frange = 1.0f / (1LL<<precMultExp);
    static constexpr int32_t    ref19900ycent   = 0x1ebacfd9;
    static constexpr int32_t    ref0_01ycent    = 0x9ce2e7f;

    static inline const ExpTable& getInstance()
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
    // probably not needed
    static inline float expMinus2PIindex0( const uint16_t index )
    {
        return expMinus2PI[index];
    }
    // probably not needed
    static inline float expMinus2PIindex1( const uint16_t index )
    {
        return expMinus2PI[uint32_t(index)+1];
    }
    // probably not needed
    static inline const auto& expMinus2PIRef()
    {
        return expMinus2PI;
    }
    static inline const auto& exp2Ref()
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
        const uint64_t dy = (( exp2Table[ ind + 1] - y1 ) * uint8_t( logv )) >> 8;
        const uint64_t rs = ( y1 + dy ) >> ( precMultExp - xpo );
        return rs;
    } // end expi2( uint32_t logv )

// --------------------------------------------------------------------
// directly calculates deltaFi
// kills too high and low frequencies result 0 -> stop oscillator

    static inline uint32_t ycent2deltafi( int32_t base )
    {
        if( ( ref19900ycent < base ) || ( ref0_01ycent > base ) )
            return 0;   //  nuke the high and  low freqs

        const uint32_t  logv = base;
        const uint16_t  ind = logv >> 8;
        const uint8_t   xpo = logv >> 24;
        const uint64_t  y1 = exp2Table[ ind ];
        const uint64_t  dy = (( exp2Table[ ind + 1 ] - y1 ) * uint8_t( logv )) >> 8;
        return ( y1 + dy ) >> ( precMultExp - xpo );
    } // end ycent2deltafi
// --------------------------------------------------------------------
// directly calculates deltaFi
// kills too high and low frequencies result 0 -> stop oscillator

    static inline uint32_t ycent2deltafi( int32_t base, int32_t delta )
    {
        constexpr int32_t   maxDelta = 0x0800000;
        if( ( ref19900ycent < base ) || ( ref0_01ycent > base ) )
            return 0;   //  nuke the high and  low freqs
        const uint32_t  pitch = base + saturate<int32_t,maxDelta>( delta );
        const uint32_t  logv = pitch > ref19900ycent ? ref19900ycent : pitch;
        const uint16_t  ind = logv >> 8;
        const uint8_t   xpo = logv >> 24;
        const uint64_t  y1 = exp2Table[ ind ];
        const uint64_t  dy = (( exp2Table[ ind + 1 ] - y1 ) * uint8_t( logv )) >> 8;
        return ( y1 + dy ) >> ( precMultExp - xpo );
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
    // probably not needed
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
        constexpr long double delta = -PI / expTableSize;
        constexpr long double dexp  = 1.0L / double(expTableSize);
        constexpr long double two   = 2.0;
        constexpr uint64_t norm = 1LL << precMultExp;
        for( uint32_t i = 0u; i <= expTableSize; ++i ) {

            // probably not needed
            // exp( -2 * PI *x ) ; x = 0 .. 0.5
            expMinus2PI[ i ]    = std::exp( delta * i );
            // exp2 x = 1..2
            exp2Table[ i ]      = uint64_t( std::llround( std::pow( two, i * dexp ) * norm ) );
#if 0
            if( i > 0 ) {
                uint64_t diff = exp2Table[ i ] - exp2Table[ i - 1 ];
                if( diff > (1ULL<<44) ) {
                    std::cout << std::dec
                        << "i: " << i
                        << " diff: " << diff
                        << std::endl;
                    
                }
            }
#endif
        }
    };
};


class VelocityBoostTable {
public:
    
    static inline const VelocityBoostTable& getInstance()
    {
        static VelocityBoostTable instance;
        return instance;
    }
    
    static inline uint16_t getBoost( uint8_t velocity, uint8_t booster ) 
    {
        return uint16_t(velocityBoostTable[ velocity ]) * booster;
    }
    
private:
// plot2d(  [ "x", "(x-(exp(log(2)*x/32)-1) ) ", "x - 0.6 * (x-(exp(log(2)*x/32)-1) )" ], 0, 256 ,color=4:5:6  )
    VelocityBoostTable() {
        for( uint32_t i = 0u; i < velocityBoostTableSize; ++i ) {
            const double di = i;
            const double diexp = std::exp2( di/32.0 ) - 1.0;
            velocityBoostTable[ i ] = std::round( di - diexp );
#if 0
            std::cout << std::dec
                << "i: " << di
                << " exp: " << diexp
                << " bo: " << uint16_t(velocityBoostTable[ i ]) 
                << std::endl;
#endif
        }        
    }
};

} // end namespace tables


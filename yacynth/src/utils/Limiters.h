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
 * File:   Limiters.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on March 12, 2016, 7:13 AM
 */

#include    <iostream>
#include    <array>
#include    <cmath>
#include    <type_traits>

namespace limiter {

struct CompressFloat;
extern  const CompressFloat   compressFloat;

struct CompressFloat {
    static constexpr uint16_t   tableSize4      = 4;
    static constexpr uint16_t   tableSize5      = 5;
    static constexpr int16_t    noiseFloorExp   = -23;
    CompressFloat()
    {
        // 4 grade
        cT4A8[0].k     = 2.0;
        cT4A8[0].xk    = 1.0 / 16.0;
        cT4A8[0].yk    = 4.0 / 8.0 ;
        for( int i = 1; i <= tableSize4; ++i ) { // tableSize is + 1 !!!!
            cT4A8[i].k     = cT4A8[i-1].k / 2.0;
            cT4A8[i].xk    = cT4A8[i-1].xk * 2.0;
            cT4A8[i].yk    = cT4A8[i-1].yk + 1.0 / 8.0;
        };

        // 5 grade
        cT5A4[0].k     = 2.0;
        cT5A4[0].xk    = 1.0 / 16.0;
        cT5A4[0].yk    = 2.0 / 8.0 ;
        for( int i = 1; i <= tableSize5; ++i ) { // tableSize is + 1 !!!!
            cT5A4[i].k     = cT5A4[i-1].k / 2.0;
            cT5A4[i].xk    = cT5A4[i-1].xk * 2.0;
            cT5A4[i].yk    = cT5A4[i-1].yk + 1.0 / 8.0;
        };
    };

    union   floatexp {
        float   inFl;
        struct {
            uint32_t mantisa    : 23;
            uint32_t exponent   : 8;
            uint32_t sign       : 1;
        };
    };

    struct  compressTable {
        float   xk;
        float   yk;
        float   k;
    };
    inline float compressSoft5( const float inp  ) const
    {
        const floatexp  fexpv { .inFl = inp };
        const int exponent =  fexpv.exponent - 127;
//        std::cout << "exp " << exponent << "  ";
        if( exponent < noiseFloorExp )
            return 0.0;
        if( exponent <  -tableSize4 ) // yes 4
            return  inp * 4.0;
        if( exponent >= 1 ) {
            const compressTable&   cTv = cT5A4[ tableSize5 ]; // + 1
 //      std::cout << " k " << cTv.k  << " xk "  << cTv.xk << " yk " <<  cTv.yk << "  ";
            if( inp > 0 ) {
                return cTv.yk + ( inp - cTv.xk ) * cTv.k;
            } else {
                return -cTv.yk - ( -inp - cTv.xk ) * cTv.k;
            }
        } else {
            const compressTable&   cTv = cT5A4[ exponent + tableSize4 ]; // yes 4
 //      std::cout << " k " << cTv.k  << " xk "  << cTv.xk << " yk " <<  cTv.yk << "  ";
            if( inp > 0 ) {
                return cTv.yk + ( inp - cTv.xk ) * cTv.k;
            } else {
                return -cTv.yk - ( -inp - cTv.xk ) * cTv.k;
            }
        }
    } // end compress
    //
    // this will produce overshots -- try internally
    //
    inline float compressSoft4( const float inp  ) const
    {
        const floatexp  fexpv { .inFl = inp };
        const int exponent =  fexpv.exponent - 127;
   //     std::cout << "exp " << exponent << "  ";
        if( exponent < noiseFloorExp )
            return 0.0;
        if( exponent <  -tableSize4 )
            return  inp * 8.0;
        if( exponent >= 0 ) {
            const compressTable&   cTv = cT4A8[ tableSize4 ]; // + 1
            if( inp > 0 ) {
                return cTv.yk + ( inp - cTv.xk ) * cTv.k;
            } else {
                return -cTv.yk - ( -inp - cTv.xk ) * cTv.k;
            }
        } else {
            const compressTable&   cTv = cT4A8[ exponent + tableSize4 ];
            if( inp > 0 ) {
                return cTv.yk + ( inp - cTv.xk ) * cTv.k;
            } else {
                return -cTv.yk - ( -inp - cTv.xk ) * cTv.k;
            }
        }
    } // end compress

    inline float compressHard4( const float inp  ) const
    {
        const floatexp  fexpv { .inFl = inp };
        const int exponent =  fexpv.exponent - 127;
     //   std::cout << "exp " << exponent << "  ";
        if( exponent < noiseFloorExp )
            return 0.0;
        if( exponent <  -tableSize4 )
            return  inp * 8.0;
        if( exponent >= 0 )
            return inp < 0 ? -1.0 : 1.0;
        {
            const compressTable&   cTv = cT4A8[ exponent + tableSize4 ];
//       std::cout << " k " << cTv.k  << " xk "  << cTv.xk << " yk " <<  cTv.yk << "  ";
            if( inp > 0 ) {
                return cTv.yk + ( inp - cTv.xk ) * cTv.k;
            } else {
                return -cTv.yk - ( -inp - cTv.xk ) * cTv.k;
            }
        }
    } // end compress

    compressTable   cT4A8[ tableSize4 + 1 ];
    compressTable   cT5A4[ tableSize5 + 1 ];
}; // end CompressFloat1


class  CompressInt64 {
public:
    float  compress( const int64_t inp ) const
    {
       return compressFloat.compressHard4( float(inp) * range );
    };

    void  setRange( const uint16_t val )
    {
        range = 1.0 / (1L<<val);
    };

private:
    float   range;
}; // end CompressInt64

// --------------------------------------------------------------------
//
// compress 63 bit to 32 bit quasi logaritmic --> amplitude compression
//
// s lllllll mmmm mmmm mmmm mmmm mmmm
//
// TODO TEST
//
inline int32_t logcompress( const int64_t val )
{
    const uint64_t aval = std::abs(val);
    const bool neg = val < 0;
    if( ( 1LL << 24 ) > aval )
        return static_cast<int32_t>(val) ;

    const uint8_t exp  = (64-24) - __builtin_clzll(aval);    
    const int32_t expa = exp << 24; 
    const int32_t rs = ( aval >> exp ) + expa;
    return neg ? -rs : rs;
} // end logcompress

// --------------------------------------------------------------------
// range = 0..15
template< uint8_t range=0 >
inline int64_t limitx3hard( const int64_t x )
{
    static_assert( range < 16, "limitx3hard illegal range" );
    constexpr int64_t limit = 1L<<(31-range);
    const int64_t x1 = std::min( limit, std::max( -limit, x ));
    return (x1>>(1+range)) + ( x1 - (((( x1 * x1 ) >> 32 ) * x1 ) >> 32 ))>>(2+range); // to sound TEST
} // end limitx3hard


// range = 0..15 / not checked
inline int64_t limitx3hard( const int64_t x, const uint8_t range )
{
    const int64_t limit = 1L<<(31-range);
    const int64_t x1 = std::min( limit, std::max( -limit, x ));
    return (x1>>(1+range)) + ( x1 - (((( x1 * x1 ) >> 32 ) * x1 ) >> 32 ))>>(2+range); // to sound TEST
} // end limitx3hard
// --------------------------------------------------------------------
inline float limitx3hard ( const float x )
{
    const float x1 = std::min( 1.0f, std::max( -1.0f, x ));
//    return 1.5f * x1 - 0.5f * x1 * x1 * x1;
    return x1 + std::ldexp( x1 - x1 * x1 * x1, -1 );
} // end limitx3hard
// --------------------------------------------------------------------
inline float limitx3hard ( const double x )
{
    const double x1 = std::min( 1.0, std::max( -1.0, x ));
//    return 1.5f * x1 - 0.5f * x1 * x1 * x1;
    return x1 + std::ldexp( x1 - x1 * x1 * x1, -1 );
} // end limitx3hard
// --------------------------------------------------------------------
inline float limitx3soft ( const float x )
{
    return x > 2.0f ?  x * 0.25f + 1.0f : x < -2.0f ? x * 0.25f - 1.0f :  x - 0.0625f * x * x * x;
} // end limitx3soft
// --------------------------------------------------------------------
// clip at x percent of 1.0f
template< std::size_t limit>
inline float fLimitClip ( const float x )
{
    constexpr float flim = float(limit) / 100.0;
    return std::min( flim, std::max( -flim, x ));
} // end fLimitClip

// clip at x percent of 1.0d
template< std::size_t limit>
inline double dLimitClip ( const double x )
{
    constexpr double flim = double(limit) / 100.0;
    return std::min( flim, std::max( -flim, x ));
} // end dLimitClip

// limit is value not percent
template< int64_t limit>
inline int64_t i64LimitClip ( const int64_t x )
{
    return std::min( limit, std::max( -limit, x ));
} // end i64LimitClip
template< int32_t limit>
inline int64_t i32LimitClip ( const int32_t x )
{
    return std::min( limit, std::max( -limit, x ));
} // end i64LimitClip

inline int64_t i64LimitClip ( const int64_t x, const int64_t limit )
{
    return std::min( limit, std::max( -limit, x ));
} // end i64LimitClip

// limit is value not percent
// result is normalized
template< int64_t limit>
inline float i64tofNormLimitClip ( const int64_t x )
{
    return float( std::min( limit, std::max( -limit, x ))) * (1.0f/limit);
} // end i64tofNormLimitClip

// --------------------------------------------------------------------
// linear under 0.5
// clip at 1.5 input 2.5
// smooth between 0.5..1.5 input 0.5..2.5
//
inline float limitx2hard ( const float x )
{
    if( std::fabs(x) < 0.5f ) return x;
    if( x >= 2.5f  )          return 1.5f;
    if( x <= -2.5f )          return -1.5f;
    const float xx1 = x * x * 1.0f/4.0f + 1.0f/16.0f ;
    return 5.0f/4.0f * x + (  x > 0 ? - xx1 : xx1 );
} // end limitx2hard

// --------------------------------------------------------------------


} // end namespace yacynth


#pragma once

/*
 * Copyright (C) 2016 Istvan Simon
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
 * File:   Fastsincos.h
 * Author: Istvan Simon
 *
 * Created on March 22, 2016, 6:51 PM
 */

#include    <cstdint>
#include    <cmath>
#include    <algorithm>
#include    <iostream>

// singleton class
//
// usage: sintable.fastsin
// usage: sintable.fastsinDiv2PI
// usage: sintable.fastcos
// usage: sintable.fastcosDiv2PI

namespace tables {
constexpr uint16_t  cacheLineSize               = 64;
constexpr uint32_t  sinTableSizeExp  = 16;
constexpr uint32_t  sinTableSize     = 1<<sinTableSizeExp;
extern  int16_t     waveSinTable[ sinTableSize + 1 ];
extern  float       sinTable[ sinTableSize + 1 ];

class SinTable;
extern const SinTable& sintable;

class SinTable {
public:
    static constexpr long double PIld = 3.141592653589793238462643383279502884197169399375105820974944592307816;
    static constexpr float PI   = PIld;
    static constexpr float PI2  = PIld*2.0;
    static constexpr float offs = std::sin( PIld / 2.0 / sinTableSize);
    static inline const SinTable& table(void)
    {
        static SinTable instance;
        return instance;
    }
#if  0
    inline int16_t sinWaveindex0(   const uint16_t index ) const
    {
        return waveSinTable[index];
    }
    inline int16_t sinWaveindex1(   const uint16_t index ) const
    {
        return waveSinTable[uint32_t(index)+1];
    }
    inline float sin2PIindex0(      const uint16_t index ) const
    {
        return sinTable[index];
    }
    inline float sin2PIindex1(      const uint16_t index ) const
    {
        return sinTable[uint32_t(index)+1];
    }
    inline const auto& sin2PIRef(void) const
    {
        return sinTable;
    }
    inline const int16_t *sinWave(void) const
    {
        return waveSinTable;
    }
    // --------------------------------------------------------------------
    inline float fastsin( const float fi ) const noexcept
    {
        return fastsin2PI( fi * (0.5f/PI));
    } // end fastsin

    //
    // TODO: table is now full sine -> can be simplified
    //
    // input: rad / ( 2*PI )
    // 18 bit of input
    // 2 * PI * x * ( 2^mult2exp)
    template< int mult2x >
    inline float fastsin2PImult2x( const float fi ) const noexcept
    {
        const uint64_t ifi = std::lround(fi * (1<<(18+mult2x)));
        const uint16_t ind = ifi;
        switch( ( ifi >> 16 ) & 3 ) {
        case 0: return  sinTable[ ind ];
        case 1: return  sinTable[ uint16_t( 0x0FFFF - ind ) + 1];
        case 2: return -sinTable[ ind ];
        case 3: return -sinTable[ uint16_t( 0x0FFFF - ind ) + 1];
        }
    } // end fastsin2PImult2k

    inline float fastsin2PI( const float fi ) const noexcept
    {
        const uint64_t ifi = std::lround(fi * (1<<18)) & 0x3FFFF;
        const uint16_t ind = ifi;
        switch( ( ifi >> 16 ) & 3 ) {
        case 0: return  sinTable[ ind ];
        case 1: return  sinTable[ uint16_t( 0x0FFFF - ind ) + 1];
        case 2: return -sinTable[ ind ];
        case 3: return -sinTable[ uint16_t( 0x0FFFF - ind ) + 1];
        }
    } // end fastsinDiv2PI


    inline void fastsincos( const float fi, float& sinv, float& cosv ) const noexcept
    {
        fastsincos2PI( fi * (0.5f/PI), sinv, cosv );
    } // end fastsincos

    inline void fastsincos2PI( const float fi, float& sinv, float& cosv ) const noexcept
    {
        const uint64_t ifi = std::lround(fi * (1<<18));
        const uint16_t ind = ifi;
        switch( ( ifi >> 16 ) & 3 ) {
        case 0:
            sinv = sinTable[ ind ];
            cosv = sinTable[ uint16_t( 0x0FFFF - ind ) + 1];
            return;
        case 1:
            sinv = sinTable[ uint16_t( 0x0FFFF - ind ) + 1];
            cosv = -sinTable[ ind ];
            return;
        case 2:
            sinv = -sinTable[ ind ];
            cosv = -sinTable[ uint16_t( 0x0FFFF - ind ) + 1];
            return;
        case 3:
            sinv = -sinTable[ uint16_t( 0x0FFFF - ind ) + 1];
            cosv = sinTable[ ind ];
            return;
        }
    } // end fastsinDiv2PI

    inline float fastcos( const float fi ) const noexcept
    {
        return fastcos2PI( fi * (0.5f/PI));
    } // end fastsin

    // input: rad / ( 2*PI )
    // 18 bit of input
    inline float fastcos2PI( const float fi ) const noexcept
    {
        const uint64_t ifi = std::lround( fi * (1<<18) );
        const uint16_t ind = ifi;
        switch( ( ifi >> 16 ) & 3 ) {
        case 0: return  sinTable[ uint16_t( 0x0FFFF - ind ) + 1];
        case 1: return -sinTable[ ind ];
        case 2: return -sinTable[ uint16_t( 0x0FFFF - ind ) + 1];
        case 3: return  sinTable[ ind ];
        }
    } // end fastcosDiv2PI

    inline void fastrot( const float fi, const float xi, const float yi, float xo, float yo )
    {
        fastrot2PI( fi * (0.5f/PI), xi, yi, xo, yo );
    } // end fastrot

    inline void fastrot2PI( const float fi, const float xi, const float yi, float xo, float yo )
    {
        float sinv;
        float cosv;
        fastsincos2PI( fi, sinv, cosv );
        xo  = xi * cosv - yi * sinv;
        yo  = xi * sinv + yi * cosv;
    } // end fastrot2PI

    inline float sinc( const float val ) const noexcept
    {
        if( std::abs( val ) < 1.0f / (1<<4) ) { // need more exact limit
            return 1.0f - val * val;
        }
        return fastsin( val ) / val;
    }

    inline float at( const uint16_t ival ) const noexcept
    {
        return sinTable[ival];
    } // end at

    inline float wave( const int64_t ival ) const noexcept
    {
        const uint16_t ind = ival;
        switch( ( ival >> 16 ) & 3 ) {
        case 0: return  sinTable[ uint16_t( 0x0FFFF - ind ) + 1];
        case 1: return -sinTable[ ind ];
        case 2: return -sinTable[ uint16_t( 0x0FFFF - ind ) + 1];
        case 3: return  sinTable[ ind ];
        }
    } // end wave


//**************************************
// spec dsp functions
//
// ( tan( PI * x ) - 1 ) / ( tan( PI * x ) + 1 ) === ( sin( 2 * PI * x ) - 1 ) / cos ( 2 * PI * x )
// input 0 .. 0.5
//**************************************
    // valid between 0 .. 0.5 -> Nyquist
    inline float fastSin2PIx_1p_cos2PIx( const float fpfs )  const noexcept
    {
        if( 0.0f >= fpfs )  return -1.0f;
        if( 0.5f <= fpfs )  return 1.0f;
        if( 0.25f == fpfs ) return 0.0f;
        if( 0.25f > fpfs) {
            return sin1pcos[ uint16_t( std::lround( fpfs * (1<<18) )) ];
        }
        return -sin1pcos[ uint16_t( 0x0FFFF - std::lround( (fpfs-0.25f) * (1<<18) ) ) + 1 ];
    };
#endif

private:
    SinTable()
    {
        constexpr long double dcerrcomp = 0.000000000015 / 65536.0;
        constexpr long double deltai = PIld * 2.0 / sinTableSize;
//        long double ldsumm = 0;
//        long double ldsummi = 0;
        for( uint32_t i = 0u; i <= sinTableSize; ++i ) {
            const long double d = deltai * i;
            const long double sinv = std::sin( d ) - dcerrcomp;
            waveSinTable[ i ]   = std::round( sinv * 0x7FFE );
            sinTable[ i ]       = sinv;
//            ldsumm += sinTable[ i ];
//            ldsummi += waveSinTable[ i ];
        }
        sinTable[ 0 ] = sinTable[ 32768 ] = sinTable[ 65536 ] = 0;
        waveSinTable[ 0 ] = waveSinTable[ 32768 ] = waveSinTable[ 65536 ] = 0;
#if 0
        long double summ = 0;
        long double summi = 0;
        for( uint32_t i = 0u; i <= sinTableSize; ++i ) {
            summ += sinTable[ i ];
            summi += waveSinTable[ i ];
        }
        std::cout
            << "\n\n **** sin summ " << summ
            << "\n\n **** sin summi " << summi

            << "\n\n **** sin ldsumm " << ldsumm
            << "\n\n **** sin ldsummi " << ldsummi
            << std::endl;
#endif
    };
};



} // end tables
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
 * File:   FilterComb.h
 * Author: Istvan Simon
 *
 * Created on May 13, 2016, 8:50 AM
 */

#include    <cstdint>

typedef float v4sf __attribute__((mode(SF)))  __attribute__ ((vector_size(16),aligned(16)));

namespace filter {

// 1 multiplier - 2 delay line version

template< std::size_t SlenExp >
class FilterCombInt64 {
public:
    static constexpr std::size_t storeLengthExp     = SlenExp;
    static constexpr std::size_t storeLength        = 1<<storeLengthExp;
    static constexpr std::size_t storeLengthMask    = storeLength-1;

    inline bool setLength( const uint16_t p )
    {
        if( length >= storeLength ) {
            return false;
        }
        length = p; // must be less-equal storeLength
        index  = 0;
        return true;
    }

    // 9e35 -- 40501 -- 0.61799 = 40501/65536
    inline void setCoeff( const uint16_t p = 40501 )
    {
        coeff = p;
        index = 0;
    }

    // input max 47  bit !!!!
    inline int64_t getAllPass( const int64_t x )
    {
        const uint16_t rind = ( index - length ) & storeLengthMask;
        const int64_t y = zx[ rind ] + ( coeff * ( x - zy[ rind ]) >> 16 );
        index = ( ++index ) & storeLengthMask;
        zx[ index ] = x;
        zy[ index ] = y;
        return y;
    }
    inline void getAllPass( const int64_t x, int64_t& y )
    {
        y = getAllPass(x);
    }

protected:
    int64_t     zx[ storeLength ];
    int64_t     zy[ storeLength ];
    uint16_t    coeff;
    uint16_t    length;
    uint16_t    index;
};

template< std::size_t SlenExp >
class FilterComb {
public:
    static constexpr std::size_t storeLengthExp     = SlenExp;
    static constexpr std::size_t storeLength        = 1<<storeLengthExp;
    static constexpr std::size_t storeLengthMask    = storeLength-1;

    inline bool setLength( const uint16_t p )
    {
        if( length >= storeLength ) {
            return false;
        }
        length = p; // must be less-equal storeLength
        index  = 0;
        return true;
    }

    inline void setCoeff( const float p = 0.618 )
    {
        coeff = p;
        index = 0;
    }

    inline void getAllPass( const float x, float& y )
    {
        y = getAllPass(x);
    }

    inline float getAllPass( const float x )
    {
        const uint16_t rind = ( index - length ) & storeLengthMask;
        const float y = zx[ rind ] + coeff * ( x - zy[ rind ] );
        index = ( ++index ) & storeLengthMask;
        zx[ index ] = x;
        zy[ index ] = y;
        return y;
    }

protected:
    union {
        v4sf    v[ sizeof(float) * 2 * storeLength / 16 ];
        struct {
            float   zx[ storeLength ];
            float   zy[ storeLength ];
        };
    };
    float       coeff;
    uint16_t    length;
    uint16_t    index;
};

template< std::size_t SlenExp >
class FilterComb2Ch {
public:
    static constexpr std::size_t storeLengthExp     = SlenExp;
    static constexpr std::size_t storeLength        = 1<<storeLengthExp;
    static constexpr std::size_t storeLengthMask    = storeLength-1;

    inline bool setLength( const uint16_t p )
    {
        if( length >= storeLength ) {
            return false;
        }
        length = p; // must be less-equal storeLength
        index  = 0;
        return true;
    }

    inline void setCoeff( const float p = 0.618 )
    {
        coeff = p;
        index = 0;
    }

    inline void getAllPass( const float xA, const float xB, float& yA, float& yB )
    {
        const uint16_t rind = ( index - length ) & storeLengthMask;
        yA = zxA[ rind ] + coeff * ( xA - zyA[ rind ] );
        yB = zxB[ rind ] + coeff * ( xB - zyB[ rind ] );
        index = ( ++index ) & storeLengthMask;
        zxA[ index ] = xA;
        zyA[ index ] = yA;
        zxB[ index ] = xB;
        zyB[ index ] = yB;
    }

protected:
    union {
        v4sf    v[ sizeof(float) * 4 * storeLength / 16 ];
        struct {
            float   zxA[ storeLength ];
            float   zxB[ storeLength ];
            float   zyA[ storeLength ];
            float   zyB[ storeLength ];
        };
    };
    float       coeff;
    uint16_t    length;
    uint16_t    index;
};

//
// must be revised - each channel independent taps
// for reverb
//
template< std::size_t SlenExp >
class FilterComb8x {
public:
    static constexpr std::size_t storeLengthExp     = SlenExp;
    static constexpr std::size_t storeLength        = 1<<storeLengthExp;
    static constexpr std::size_t storeLengthMask    = storeLength-1;

protected:
};

} // end namespace filter
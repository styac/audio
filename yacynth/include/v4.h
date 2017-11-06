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
 * File:   v4.h
 * Author: Istvan Simon
 *
 * Created on March 13, 2016, 11:49 AM
 */

#include    <cstdint>

typedef float       v4sf __attribute__((mode(SF)))  __attribute__ ((vector_size(16),aligned(16)));
typedef int32_t     v4si __attribute__((mode(SI)))  __attribute__ ((vector_size(16),aligned(16)));
typedef uint32_t    v4su __attribute__((mode(SI)))  __attribute__ ((vector_size(16),aligned(16)));
typedef long long   v2di __attribute__((mode(DI)))  __attribute__ ((vector_size(16),aligned(16)));

template< std::size_t N, std::size_t CH >
struct V4fMvec {
    static constexpr std::size_t sizeV2 = N;
    static constexpr std::size_t size   = N * CH;
    static constexpr std::size_t sizeV4 = (size)/4;
    inline void clear(void)
    {
        static_assert( size % 4 == 0,"illegal size");
        for( auto& vi : v ) vi = 0;
    }
    union {
        float       v[ size ];    // linear vector
        float       v2[ sizeV2 ][ CH ];
        v4sf        v4[ sizeV4 ];
    };
};

template< std::size_t N, std::size_t CH >
struct V4u32Mvec {
    static constexpr std::size_t sizeV2 = N;
    static constexpr std::size_t size   = N * CH;
    static constexpr std::size_t sizeV4 = (size)/4;
    inline void clear(void)
    {
        static_assert( size % 4 == 0,"illegal size");
        for( auto& vi : v ) vi = 0;
    }
    union {
        uint32_t    v[ size ];    // linear vector
        uint32_t    v2[ sizeV2 ][ CH ];
        v4su        v4[ sizeV4 ];
    };
};

template< std::size_t N, std::size_t CH >
struct V4i32Mvec {
    static constexpr std::size_t sizeV2 = N;
    static constexpr std::size_t size   = N * CH;
    static constexpr std::size_t sizeV4 = (size)/4;
    inline void clear(void)
    {
        static_assert( size % 4 == 0,"illegal size");
        for( auto& vi : v ) vi = 0;
    }
    union {
        int32_t     v[ size ];    // linear vector
        int32_t     v2[ sizeV2 ][ CH ];
        v4si        v4[ sizeV4 ];
    };
};

struct alignas(16) V4vf {
    static constexpr const char * const v0Name = "v4sf_v[0]";
    static constexpr const char * const v1Name = "v4sf_v[1]";
    static constexpr const char * const v2Name = "v4sf_v[2]";
    static constexpr const char * const v3Name = "v4sf_v[3]";

    inline void clear(void)
    {
        v[0] = v[1] = v[2] = v[3] = 0.0f;
    }
    inline V4vf() = default;

    inline V4vf( const v4sf & val )
    : v4(val)
    {}

    inline V4vf( float v0, float v1, float v2, float v3  )
    {
        v[0] = v0;
        v[1] = v1;
        v[2] = v2;
        v[3] = v3;
    }
    union  {
        float   v[ 4 ];
        float   v2[ 2 ][ 2 ];
        v4sf    v4; // v4[ 1 ];
    };
};

// new V4 support
template< std::size_t v4Exp>
struct V4size {
    static constexpr std::size_t varraySizeExp  = v4Exp;
    static constexpr std::size_t varraySize     = 1<<varraySizeExp;
    static constexpr std::size_t varraySizeMask = varraySize-1;

    static constexpr std::size_t arraySizeExp   = varraySizeExp+2;
    static constexpr std::size_t arraySize      = 1<<arraySizeExp;
    static constexpr std::size_t arraySizeMask  = arraySize-1;
};

// T : int32_t -- v4si
// T : float   -- v4sf
template<typename T, std::size_t v4Exp>
struct alignas(16) V4array : public V4size<v4Exp> {};

// used in controllers not very general
template<std::size_t v4Exp>
struct alignas(16) V4array<int32_t, v4Exp> : public V4size<v4Exp> {
    inline void clear(void)
    {
        for( auto& vi : v ) vi = 0;
    }

    union {
        v4si    v4[ V4size<v4Exp>::varraySize   ];
        int32_t  v[ V4size<v4Exp>::arraySize    ];
    };
};


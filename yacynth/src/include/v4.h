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

//
// http://www.delorie.com/gnu/docs/gcc/gccint_53.html
//
typedef float       v4sf __attribute__((mode(SF)))  __attribute__ ((vector_size(16),aligned(16)));
typedef int         v4si __attribute__((mode(SI)))  __attribute__ ((vector_size(16),aligned(16)));
typedef long long   v2di __attribute__((mode(DI)))  __attribute__ ((vector_size(16),aligned(16)));

using V4sf_m = struct alignas(16) V4sf
{
    float   aa;
    float   ab;
    float   ba;
    float   bb;
};

struct alignas(16) V4sfMatrix {
    void clear(void)
    {
        aa = ab = ba = bb = 0.0f;
    }
    union  {
        v4sf    v;
        struct {
            float   aa;
            float   ab;
            float   ba;
            float   bb;
        };
    };
};

struct alignas(16) V4v {
    void clear(void)
    {
        v[0] = v[1] = v[2] = v[3] = 0.0f;
    }
    union  {
        v4sf    v4;
        float   v[4];
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

template<std::size_t v4Exp>
struct alignas(16) V4array<float, v4Exp> : public V4size<v4Exp> {
    inline void clear(void)
    {
        for( auto& vi : v ) vi = 0;  
    }

    union {
        v4sf    v4[ V4size<v4Exp>::varraySize   ];
        float    v[ V4size<v4Exp>::arraySize    ];
    };
};

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

template<std::size_t v4Exp>
struct alignas(16) V4array<uint32_t, v4Exp> : public V4size<v4Exp> {
    inline void clear(void)
    {
        for( auto& vi : v ) vi = 0;  
    }

    union {
        v4si    v4[ V4size<v4Exp>::varraySize   ];
        uint32_t v[ V4size<v4Exp>::arraySize    ];
    };
};




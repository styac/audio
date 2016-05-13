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

//
// http://www.delorie.com/gnu/docs/gcc/gccint_53.html
//
typedef float       v4sf __attribute__((mode(SF)))  __attribute__ ((vector_size(16),aligned(16)));
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

struct alignas(16) V4SF {
    union {
        v4sf    v;
        float   f[4];
    };
};



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
 * File:   DelayTap.h
 * Author: Istvan Simon
 *
 * Created on April 4, 2016, 8:00 PM
 */
//#include    "Serialize.h"
//#include    "Ebuffer.h"
#include    "v4.h"

#include    <cstdint>
#include    <cmath>
#include    <algorithm>
#include    <array>

namespace yacynth {

// used in LateReverb
// delay with lowpass + highpass
template< std::size_t N >
struct alignas(16) MonoDelayBandpassTapArray {
    static constexpr std::size_t size = N;
    typedef std::array<uint32_t, N> DelayIndex;
    static constexpr std::size_t v4size = (N+3)/4;

    struct {
        union {
            float   v[N];             // feedback - output
            v4sf    v4[(N+3)/4];
        };
    } coeff;

    struct {
        union {
            float   v[N];             // feedback - output
            v4sf    v4[(N+3)/4];
        };
    } coeffLowPass;
    struct {
        union {
            float   v[N];             // feedback - output
            v4sf    v4[(N+3)/4];
        };
    } coeffHighPass;

    DelayIndex    delayIndex;             // delay of the tap
};

// used in LateReverb
// delay with lowpass
template< std::size_t N >
struct alignas(16) MonoDelayLowpassTapArray {
    static constexpr std::size_t size = N;
    typedef std::array<uint32_t, N> DelayIndex;
    static constexpr std::size_t v4size = (N+3)/4;

    struct {
        union {
            float   v[N];             // feedback - output
            v4sf    v4[(N+3)/4];
        };
    } coeff;

    struct {
        union {
            float   v[N];             // feedback - output
            v4sf    v4[(N+3)/4];
        };
    } coeffLowPass;

    DelayIndex    delayIndex;             // delay of the tap
};

// used in LateReverb
// simple delay
template< std::size_t N >
struct alignas(16) MonoDelayTapArray {
    static constexpr std::size_t size = N;
    typedef std::array<uint32_t, N> DelayIndex;
    static constexpr std::size_t v4size = (N+3)/4;
    struct {
        union {
            float   v[N];             // feedback - output
            v4sf    v4[(N+3)/4];
        };
    } coeff;

    DelayIndex    delayIndex;             // delay of the tap
};


// used in Echo
template< std::size_t N >
struct StereoDelayTapArray {
    typedef std::array<uint32_t, N> DelayIndex;
    static constexpr std::size_t size = N;
    V4vf        coeff[N];
    uint32_t    delayIndex[N][2];
};

// used in Echo
template< std::size_t N >
struct StereoDelayLowPassTapArray {
    typedef std::array<uint32_t, N> DelayIndex;
    static constexpr std::size_t size = N;
    V4vf        coeff[N];
    V4vf        coeffLowPass[N];
    uint32_t    delayIndex[N][2];
};

template< std::size_t N >
struct StereoFractionalDelayTapArray {
    typedef std::array<uint32_t, N> DelayIndex;
    static constexpr std::size_t size = N;
    V4vf        coeff[N];
    uint64_t    delayIndex[N][2];
};

// --------------------------------------------------------------------
} // end namespace yacynth

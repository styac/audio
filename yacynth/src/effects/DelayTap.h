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
//#include "Serialize.h"
//#include "Ebuffer.h"
#include "v4.h"

#include <cstdint>
#include <cmath>
#include <algorithm>
#include <array>

namespace yacynth {

// TODO
// rename MonoDelayBandpassTapArray to CombBandpassTapArray

// used in LateReverb
// delay with lowpass + highpass
template< std::size_t N, std::size_t CH >
struct alignas(16) DelayMultBandpassTapArrayNCH {
    static constexpr std::size_t size = N;
    typedef std::array<uint32_t, N> DelayIndex;
    static constexpr std::size_t v4size = (N+3)/4;

    static constexpr char const * const tapIndexName        = "tapIndex";
    static constexpr char const * const coeffName           = "coeff_";
    static constexpr char const * const coeffLowPassName    = "coeffLowPass_";
    static constexpr char const * const coeffHighPassName   = "coeffHighPass_";
    static constexpr char const * const delayName           = "delay_";

    V4fMvec<N,CH>   coeff;
    V4fMvec<N,CH>   coeffLowPass;
    V4fMvec<N,CH>   coeffHighPass;
#if 0
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
#endif
    DelayIndex    delayIndex;             // delay of the tap
};

// used in LateReverb
// delay with lowpass
template< std::size_t N, std::size_t CH >
struct alignas(16) DelayMultLowpassTapArrayNCH {
    static constexpr std::size_t size = N;
    typedef std::array<uint32_t, N> DelayIndex;
    static constexpr std::size_t v4size = (N+3)/4;
    
    static constexpr char const * const tapIndexName        = "tapIndex";
    static constexpr char const * const coeffName           = "coeff_";
    static constexpr char const * const coeffLowPassName    = "coeffLowPass_";
    static constexpr char const * const delayName           = "delay_";

    V4fMvec<N,CH>   coeff;
    V4fMvec<N,CH>   coeffLowPass;
#if 0    
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
#endif
    DelayIndex    delayIndex;             // delay of the tap
};

// used in LateReverb
// simple delay
template< std::size_t N, std::size_t CH >
struct alignas(16) DelayMultTapArrayNCH {
    static constexpr std::size_t size = N;
    typedef std::array<uint32_t, N> DelayIndex;
    static constexpr std::size_t v4size = (N+3)/4;
    static constexpr char const * const tapIndexName        = "tapIndex";
    static constexpr char const * const coeffName           = "coeff_";
    static constexpr char const * const delayName           = "delay_";

    V4fMvec<N,CH>   coeff;
#if 0    
    struct {
        union {
            float   v[N];             // feedback - output
            v4sf    v4[(N+3)/4];
        };
    } coeff;
#endif
    DelayIndex    delayIndex;             // delay of the tap
};

// used in EarlyReflection
template< std::size_t N, std::size_t CH >
struct DelayModulatedTapArrayNCH {
    static constexpr char const * const tapIndexName        = "tapIndex";
    static constexpr char const * const coeffName           = "coeffCH_";
    static constexpr char const * const modDepthName        = "modDepthCH_";
    static constexpr char const * const deltaPhaseName      = "deltaPhaseCH_";
    static constexpr char const * const delayName           = "delayCH_";
    //
    // modDepth = coeff * depth / (1<<31)
    //
    V4fMvec<N,CH>   coeff;
    V4fMvec<N,CH>   modDepth;
    V4i32Mvec<N,CH> modDeltaPhase;
    V4i32Mvec<N,CH> delayIndex;
};

template< std::size_t N, std::size_t CH >
struct DelayTapArrayNCH {    
    static constexpr char const * const tapIndexName        = "tapIndex";
    static constexpr char const * const delayName           = "delayCH_";
    V4i32Mvec<N,CH> delayIndex;
};

// used in Echo
template< std::size_t N >
struct StereoDelayTapArray {
    typedef std::array<uint32_t, N> DelayIndex;
    static constexpr std::size_t size = N;

    static constexpr char const * const coeffIndexName          = "coeffIndex";
    static constexpr char const * const coeffName               = "coeff";

    static constexpr char const * const delayIndexName          = "delayIndex";
    static constexpr char const * const delayName               = "delay";

    static constexpr char const * const delayChannelName        = "delayCH_";
    static constexpr char const * const coeffChannelName        = "coeffCH_";

    V4vf        coeff[N];
    uint32_t    delayIndex[N][2];
};

// used in Echo
template< std::size_t N >
struct StereoDelayLowPassTapArray {
    typedef std::array<uint32_t, N> DelayIndex;
    static constexpr std::size_t size = N;

    static constexpr char const * const coeffIndexName          = "coeffIndex";
    static constexpr char const * const coeffName               = "coeff";

    static constexpr char const * const coeffLowPassIndexName   = "coeffLowPassIndex";
    static constexpr char const * const coeffLowPassName        = "coeffLowPass";

    static constexpr char const * const delayIndexName          = "delayIndex";
    static constexpr char const * const delayName               = "delay";

    static constexpr char const * const delayChannelName        = "delayCH_";
    static constexpr char const * const coeffChannelName        = "coeffCH_";

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

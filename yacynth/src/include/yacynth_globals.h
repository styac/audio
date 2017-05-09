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
 * File:   yacynth_globals.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on January 26, 2016, 11:35 PM
 */

// this will go from here


#include    <cstdint>
#include    <cmath>
#include    <algorithm>

#define YAC_DEBUG    1

namespace yacynth {

#define cArrayElementCount(T)    (sizeof(T) / sizeof(T[0]))

constexpr   uint16_t    cacheLineSize               = 64;

constexpr   double      PI  = 3.141592653589793238462643383279502884197169399375105820974944592307816406286;
constexpr   double      PI2 = PI * 2.0;

constexpr   uint16_t    overtoneCountOscDefExp      = 8;         // 256 overtones
constexpr   uint16_t    overtoneCountOscDef         = 1<<overtoneCountOscDefExp;
constexpr   uint16_t    overtoneCountOscDefMask     = overtoneCountOscDef-1;

constexpr   uint16_t    voiceCount                  = 1<<7;         // 128 x poliphony
constexpr   uint16_t    lfoCount                    = 16;
//
// for different sets like: Spectrum, Envelope, ....
//
constexpr   uint16_t    settingVectorSize           = 16;

//
// point 0  : delay (with 0 - or very lazy curve )
// point 15 : sustain - can be decay for plucked instruments
//
constexpr   uint16_t    envelopeVectorSize          = 1<<8;
constexpr   uint16_t    transientKnotCount          = 1<<4;     // 16 point
constexpr   uint16_t    envelopeKnotRelease         = 0x7F;     // new> own var for release
constexpr   uint16_t    envelopeKnotUp              = transientKnotCount-1;
constexpr   uint16_t    envelopeKnotSteady          = -1;

constexpr   uint16_t    sytemOutChannel             = 2;    // mono = mono/2 stereo
constexpr   double      samplingFrequency           = 48000.0;

constexpr   uint16_t    normFactorPhaseIndexExp     = 1<<4;
constexpr   uint64_t    normFactorPhaseIndexVal     = 1LL << normFactorPhaseIndexExp;
//
// 64k - tricky addressing - only for pitch oscillator
//
constexpr   int         waveTableSize               = 1 << 16;
constexpr   int         velocityTableSize           = 1 << 8;
constexpr   uint16_t    oscillatorOutSampleCountExp = 6;        // 64 sample
constexpr   uint16_t    oscillatorOutSampleCount    = 1<<oscillatorOutSampleCountExp;
constexpr   uint16_t    effectChannelCount          = 16;
//
// ------------------------------------------------------------
//  lowest 8 bit: linear interpolation
//  next 16 bit: index into the exp2 table
//
constexpr   uint32_t    ycentNormIntExp         = 24;
constexpr   uint32_t    ycentNormInt            = 1<<ycentNormIntExp;
constexpr   double      ycentNorm               = 65536.0 * 256.0;

constexpr   double      deltaPhaseScalerBase    =
    double( normFactorPhaseIndexVal * waveTableSize ) / samplingFrequency;

constexpr inline uint64_t freq2deltaPhase( double freq )
    { return uint64_t( std::llround(  deltaPhaseScalerBase * freq )); };

constexpr inline double freq2deltaPhaseDouble( double freq )
    { return deltaPhaseScalerBase * freq; };

constexpr inline uint32_t deltaPhase2ycent( double delta )
    { return uint32_t( std::lround( std::log2( delta ) * ycentNorm) ); };

constexpr inline double deltaPhase2ycentDouble( double delta )
    { return std::log2( delta ) * ycentNorm;  };

constexpr inline uint32_t freq2ycent( double freq )
    { return uint32_t( std::lround( std::log2( deltaPhaseScalerBase * freq ) * ycentNorm) ); };

inline double freq2ycentDouble( double freq )
    { return std::log2( deltaPhaseScalerBase * freq ) * ycentNorm; };

//====================================================
// there should a constexpr function to calculate these
//
#define     freq2ycentDef(x) \
    ( std::log2( deltaPhaseScalerBase * (x) ) * ycentNorm )

// for LFO frequencies -- sampling rate is 1 / lfoRateSamplingFrequency
#define     freq2ycentLfoDef(x) freq2ycentDef( x )
//    0x193b0973;   // octave 25
constexpr double    refA440ycentDouble  =   freq2ycentDef(440.0);
constexpr uint32_t  refA440ycent        =   uint64_t(std::llround(refA440ycentDouble));
constexpr uint32_t  ref19900ycent       =   freq2ycentDef(19900.0);
constexpr uint32_t  ref0_01ycent        =   freq2ycentDef(0.01);

// LFO limits

constexpr uint32_t  refLfoycentRange    =   5; // octave up and down
constexpr double    refLfoFreq          =   1.0; // Hz

constexpr uint32_t  refLfocent          =   freq2ycentLfoDef(refLfoFreq);
constexpr uint32_t  refMaxLfoycent      =   freq2ycentLfoDef(refLfoFreq * (1<<refLfoycentRange) );
constexpr uint32_t  refMinLfoycent      =   freq2ycentLfoDef(refLfoFreq / (1<<refLfoycentRange) );
// middle : 2 *20 = 40 2/20 = 0.1


//====================================================
// amplitude normalisations ( >> N )
constexpr int normAmplitudeOscillatorExp    = 32;
constexpr int normAmplitudePanmixExp        = 36; // looks optimal


//====================================================
template< typename Tdata, Tdata limit>
inline Tdata saturate ( const Tdata x )
{
    return std::min( Tdata(limit), std::max( Tdata(-limit), x ));
}
//====================================================
template< typename Tdata, Tdata limitLow, Tdata limitHigh>
inline Tdata saturate ( const Tdata x )
{
    return std::min( Tdata(limitHigh), std::max( Tdata(limitLow), x ));
}

//====================================================
//#define     OSCILLATOR_PHASE_RANDOMIZER     1
//#define     OSCILLATOR_VELOCITY_RANDOMIZER  1
//#define     OSCILLATOR_AMPLITUDE_RANDOMIZER  1

//====================================================

#if 0
template<typename T>
T saturate(T val, T min, T max) {
    return std::min(std::max(val, min), max);
}

template<typename T>
T saturate(T val, T lim) {
    return std::min(std::max(val, -lim), lim);
}

template<typename T, std::size_t lim>
T saturate(T val) {
    return std::min(std::max(val, -lim), lim);
}
#endif

constexpr double kOnePoleLowPass( const double f )
{
    return std::exp( -PI2 * f / samplingFrequency );
}

} // end namespace yacynth


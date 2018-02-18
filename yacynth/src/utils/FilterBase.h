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
 * File:   FilterBase.h
 * Author: Istvan Simon
 *
 * Created on March 24, 2016, 2:39 PM
 */

#include    "Fastsincos.h"
#include    "Fastexp.h"
#include    "Limiters.h"

using namespace limiter;
using namespace tables;


//
//      F control parameter: pitchF
//  F logaritmic -> ycent
//  1<<32 / fsampling is the base
//
// octave
//  31 : 24000 .. 48000  not used practically
//  30 : 12000 .. 24000 only till 20000 but for allpass 12000 for 2xSV 16000
//  29 :  6000 .. 12000 for 2xSV 16000
//  28 :  3000 ..  6000
//  27 :  1500 ..  3000
//  26 :   750 ..  1500
//  25 :   375 ..   750  A440 = 19 3b 09 73 -- 19 -> 25
//  24 :   180 ..   375 ~
//  23 :    90 ..   180 ~
//  22 :    45 ..    90 ~
//  21 :    22 ..    45 ~
//  20 :    11 ..    22 ~ -- 2x oversampling - everything goes down by 1
//
// only octave 20..30 is used - 11
// each octave is split into 256 parts and lin. interpolated 1 byte index - 2 byte interpolation
//  table: y0 , m
//  res = y0 + m * dx
// table : y0,m - int32 - fc 0..0.5 * (1<<32)

//
//  float/double:  octave.fraction:   A440 - 25.nnnnf
//  int : byte 3 - octave - byte 2,1,0 fraction
//
// calculating fc = exp2( pitchF ) / ( 1<<32 ) : fc = f/fs
//
// linear iterate on the log value
//

typedef float v4sf __attribute__((mode(SF)))  __attribute__ ((vector_size(16),aligned(16)));

namespace filter {
constexpr   uint16_t    cacheLineSize               = 64;

// --------------------------------------------------------------------
//  filter state
//
// stateCount = state count of 1 filter
// channelCount = stereo=2 , mono=1
// filterCount = count of filters in 1 channel
//
//  4x4pole:
//  stateCountExp = 2   stateCount = 4
//  channelCountExp = 1 channelCount = 2
//  filterCountExp = 2  filterCount = 2     parallel
//      allStateCount = 16
//
//  SVF:
//  stateCountExp = 1   stateCount = 2
//  channelCountExp = 1 channelCount = 2
//  filterCountExp = 2  filterCount = 2     parallel
//      allStateCount = 8
//
//  4 allpass 2 order: phaser
//  stateCountExp = 1   stateCount = 2
//  channelCountExp = 1 channelCount = 2
//  filterCountExp = 2  filterCount = 2     serial
//
//      allStateCount = 8
// coeffCount = coeff count of 1 filter
// channelCount = stereo=2 , mono=1
// filterCount = count of filters in 1 channel
//
//  4x4pole:
//  coeffCountExp = 1   coeffCount = 2      (F,Q)
//


// --------------------------------------------------------------------
// --------------------------------------------------------------------
// --------------------------------------------------------------------
// --------------------------------------------------------------------
// === NEW === NEW === NEW === NEW === NEW === NEW === NEW
class FilterBase {
public :
    static constexpr float      PI  = 3.141592653589793238462643383279502884197;
    static constexpr float      PI2 = 2.0f * PI;
    static constexpr uint16_t   samplingFrequency = 48000;  // this could be a template parameter but don't bother
    static constexpr float      onepfs  = 1.0f/samplingFrequency;
    static constexpr float      freqMin = 24.0;
    static constexpr float      freqMax = 19000.0;
    static constexpr float      fcMin   = freqMin/samplingFrequency;
    static constexpr float      fcMax   = freqMax/samplingFrequency;
    // reminder
    static constexpr int        octave12000  = 30;
    static constexpr int        octave6000   = 29;
    static constexpr int        octave3000   = 28;
    static constexpr int        octave1500   = 27;

    // channel index
    enum CHX {
        CH0,
        CH1,
        CH2,
        CH3,
    };
    
    // filter index
    enum FLX {
        FL0,
        FL1,
        FL2,
        FL3,
    };
    
    // state index
    enum ZDX {
        ZD0,
        ZD1,
        ZD2,
        ZD3,
    };

    // coeff index
    enum CDX {
        CD0,
        CD1,
        CD2,
        CD3,
    }; 
    
    // SVF specials
    enum SVFZD {
        LOW,
        BND,
        HGH,
    };

    // SVF specials
    enum SVFCD {
        CDF,
        CDD,
        CDDCIN,
    };            
};

// --------------------------------------------------------------------

// NEWNEWNEWNEW

class FilterTable : public FilterBase {
public:
    FilterTable()
    :   limit(FilterTable::tableSize-2)
    {};

    static constexpr int  maxOctave         = 31;  // to 48000
    static constexpr int  minOctave         = 20;
    static constexpr int  ycentOctaveExp    = 24;
    static constexpr int  minYcent          = minOctave<<ycentOctaveExp;

    static constexpr int  octavePointExp    = 12;
    static constexpr int  octavePointCount  = 1<<octavePointExp;
    static constexpr int  valueNormExp      = 31;
    static constexpr int  tableSize         = ( maxOctave - minOctave ) * octavePointCount;
    static constexpr int64_t valueNorm      = 1LL<<valueNormExp;
    static constexpr float   valueNormf     = 1.0f / valueNorm;

    // fast
    // no interpolation
    // valid between 20hz..20khz caller must ensure
    inline int32_t getIntRaw( const int32_t valLog ) const
    {
        constexpr uint16_t rshv = ycentOctaveExp-octavePointExp; // right shift value
        const int32_t valLog0 = std::abs(valLog - minYcent);
        const int32_t idx = valLog0>>rshv;
        return y[idx];
    }

    inline float getFloatRaw( const int32_t valLog ) const
    {
        return float ( getInt( valLog ) * valueNormf );
    }

    inline int32_t getInt( const int32_t valLog ) const
    {
        constexpr uint16_t rshv = ycentOctaveExp-octavePointExp; // right shift value
        constexpr uint32_t rshvmsk = (1<<rshv) - 1;
        const int32_t valLog0 = valLog - minYcent;
        if( valLog0 <= 0 )
            return y[0];
        const int32_t idx   = valLog0>>rshv;
        if( idx > limit )
            return y[ limit ];
        const int32_t y0 = y[idx];
        const int16_t rest  = valLog0 & rshvmsk;
        return y0 + ((( y[idx + 1] - y0 ) * rest )>>rshv);
    }

    inline float getFloat( const int32_t valLog ) const
    {
        return float ( getInt( valLog ) * valueNormf );
    }

#if 0
    inline float getFloat( const float valLog )     const
    {
        const int32_t v = valLog * (1<<ycentOctaveExp);
        return float ( getInt( v ) * valueNormf );
    }
#endif
    // filter coeff functions
    static inline double logF_fc( const double logF )
    {
        constexpr double onenorm = 1.0/(1L<<32);
        return std::pow( 2.0, logF ) * onenorm;
    }
    static inline double fc_expPi2_F( const double fc )
    {
        return std::exp( -PI2 * fc );
    }
    static inline double fc_cosPi2_F( const double fc )
    {
        return std::cos( PI2 * fc );
    }
    static inline double fc_sinPi2_F( const double fc )
    {
        return std::sin( PI2 * fc );
    }
    static inline double fc_2sinPi_F( const double fc )
    {
        return 2.0 * std::sin( PI * fc );
    }
    static inline double fc_sinpercosPi2_F( const double fc )
    {
        if( std::abs( fc - 0.25 ) < 1e-15 ) {
            return 0.0;
        }
        return ( fc_sinPi2_F(fc) - 1.0 ) / fc_cosPi2_F(fc);
    }

protected:
    int32_t y[ tableSize ];
    int32_t limit;
};

// exp( - 2 * pi * f / fs ) -- 1 pole filter

class FilterTableExp2Pi final : public FilterTable {
public:
    inline static FilterTableExp2Pi& getInstance(void)
    {
        static FilterTableExp2Pi instance;
        return instance;
    }

private:
    ~FilterTableExp2Pi() = default;

    FilterTableExp2Pi()
    {
        for( auto octave = FilterTable::minOctave; octave < FilterTable::maxOctave; ++octave ) {
            const double xO = octave;
            for( auto inOctave = 0; inOctave < FilterTable::octavePointCount; ++inOctave ) {
                uint32_t  index = ((octave - FilterTable::minOctave)<<FilterTable::octavePointExp) + inOctave;
                if( index >= FilterTable::tableSize )
                    return;
                const double x = xO + double(inOctave) / double(octavePointCount);
                const double fc = logF_fc( x );
                FilterTable::y[ index ] = std::lround( fc_expPi2_F(fc) * FilterTable::valueNorm );
                if( fc > 0.46 ) {
                    FilterTable::limit = index;
                    const float lv = FilterTable::y[ index ];
                    for( ; index<FilterTable::tableSize; ++index ) {
                        FilterTable::y[ index ] = lv;
                    }
        std::cout
            << "FilterTableExp2Pi index:" << index
            << " tableSize " << tableSize
            << " limit low " << FilterTable::y[ 0 ] * valueNormf                
            << " high " << FilterTable::y[ FilterTable::limit ] * valueNormf                
            << std::endl;

                    return;
                }
            }
        }
    }
};

class FilterTableSinCosPi2 final : public FilterTable {
public:
    inline static FilterTableSinCosPi2& getInstance(void)
    {
        static FilterTableSinCosPi2 instance;
        return instance;
    }
private:
    ~FilterTableSinCosPi2() = default;

    FilterTableSinCosPi2()
    {
        for( auto octave = FilterTable::minOctave; octave < FilterTable::maxOctave; ++octave ) {
            const double xO = octave;
            for( auto inOctave = 0; inOctave < FilterTable::octavePointCount; ++inOctave ) {
                uint32_t  index = ((octave - FilterTable::minOctave)<<FilterTable::octavePointExp) + inOctave;
                if( index >= FilterTable::tableSize )
                    return;
                const double x = xO + double(inOctave) / double(octavePointCount);
                const double fc = logF_fc( x );
                FilterTable::y[ index ] = std::lround( fc_sinpercosPi2_F(fc) * FilterTable::valueNorm );
                // std::cout << "FilterTableSinCosPi2 "  << FilterTable::y[ index ] << std::endl;
                if( fc > 0.46 ) {
                    FilterTable::limit = index;
                    const float lv = FilterTable::y[ index ];
                    for( ; index<FilterTable::tableSize; ++index ) {
                        FilterTable::y[ index ] = lv;
                    }
        std::cout
            << "FilterTableSinCosPi2 index:" << index
            << " tableSize " << tableSize
            << " limit low " << FilterTable::y[ 0 ] * valueNormf                
            << " high " << FilterTable::y[ FilterTable::limit ] * valueNormf                
            << std::endl;
                    return;
                }
            }
        }
    }
};

class FilterTable2SinPi final : public FilterTable {
public:
    inline static FilterTable2SinPi& getInstance(void)
    {
        static FilterTable2SinPi instance;
        return instance;
    }
private:
    ~FilterTable2SinPi() = default;

    FilterTable2SinPi()
    {
        for( auto octave = FilterTable::minOctave; octave < FilterTable::maxOctave; ++octave ) {
            const double xO = octave;
            for( auto inOctave = 0; inOctave < FilterTable::octavePointCount; ++inOctave ) {
                uint32_t  index = ((octave - FilterTable::minOctave)<<FilterTable::octavePointExp) + inOctave;
                if( index >= FilterTable::tableSize )
                    return;
                const double x = xO + double(inOctave) / double(octavePointCount);
                const double fc = logF_fc( x );
                FilterTable::y[ index ] = std::lround( fc_2sinPi_F(fc) * FilterTable::valueNorm );
                if( fc > 0.1666 ) {
                    FilterTable::limit = index;
                    const float lv = FilterTable::y[ index ];
                    for( ; index<FilterTable::tableSize; ++index ) {
                        FilterTable::y[ index ] = lv;
                    }
        std::cout
            << "FilterTable2SinPi index:" << index
            << " tableSize " << tableSize
            << " limit low " << FilterTable::y[ 0 ] * valueNormf                
            << " high " << FilterTable::y[ FilterTable::limit ] * valueNormf                
            << std::endl;
                    return;
                }
            }
        }
    }
};

class FilterTableCos2Pi final : public FilterTable {
public:
    inline static FilterTableCos2Pi& getInstance(void)
    {
        static FilterTableCos2Pi instance;
        return instance;
    }
private:
    ~FilterTableCos2Pi() = default;
    
    FilterTableCos2Pi()
    {
        for( auto octave = FilterTable::minOctave; octave < FilterTable::maxOctave; ++octave ) {
            const double xO = octave;
            for( auto inOctave = 0; inOctave < FilterTable::octavePointCount; ++inOctave ) {
                uint32_t  index = ((octave - FilterTable::minOctave)<<FilterTable::octavePointExp) + inOctave;
                if( index >= FilterTable::tableSize )
                    return;
                const double x = xO + double(inOctave) / double(octavePointCount);
                const double fc = logF_fc( x );
                FilterTable::y[ index ] = std::lround( fc_cosPi2_F(fc) * FilterTable::valueNorm );
//        std::cout
//            << "FilterTableCos2Pi index:" << index
//            << " val " << FilterTable::y[ index ] * valueNormf                
//            << std::endl;
                if( fc > 0.46 ) {
                    FilterTable::limit = index;
                    const float lv = FilterTable::y[ index ];
                    for( ; index<FilterTable::tableSize; ++index ) {
                        FilterTable::y[ index ] = lv;
                    }
        std::cout
            << "FilterTableCos2Pi index:" << index
            << " tableSize " << tableSize
            << " limit low " << FilterTable::y[ 0 ] * valueNormf                
            << " high " << FilterTable::y[ FilterTable::limit ] * valueNormf                
            << std::endl;
                    return;
                }
            }
        }
    }
};

} // end namespace filter


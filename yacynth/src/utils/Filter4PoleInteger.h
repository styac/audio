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
 * File:   Filter4PoleInteger.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on March 30, 2016, 11:14 PM
 */
#include    "Limiters.h"
#include    "Fastsincos.h"
#include    "Fastexp.h"
#include    "FilterBase.h"

#include    <cstdint>
#include    <cmath>

using namespace limiter;
using namespace tables;

namespace filter {

// --------------------------------------------------------------------
//
// simple fast 4 pole filter
//
class Filter4PoleIntegerBase : public FilterBase {
public:
    static constexpr uint16_t   oversamplingRate= 1;
    static constexpr float      rangeO          = 1.0 / (1LL<<31);
    static constexpr float      rangeI          = 1LL << 30;
    static constexpr int        rangeOExp       = 2;
    static constexpr int64_t    clipTuning      = 300000;

    static constexpr int16_t    fNormExp        = 32;
    static constexpr int16_t    qNormExp        = 24;
    static constexpr int64_t    fcontrolMin     = (1LL<<fNormExp) * std::exp(-PI2*fcMin);
    static constexpr int64_t    fcontrolMax     = (1LL<<fNormExp) * std::exp(-PI2*fcMax);
    static constexpr int64_t    qcontrolMin     = 0;
    static constexpr int64_t    qcontrolMax     = 4 * (1<<qNormExp); // ????

    static constexpr int        qRateExp        = 48;

    enum FilterType {
        LOWPASS,
        BANDPASS1,
        BANDPASS2,
        BANDPASS3,
    };

    struct alignas(16) Channel {
        int64_t     y0a;
        int64_t     y0b;
        int64_t     y0c;
        int64_t     y0d;
    };

    struct Param {
        int64_t         fgain;
        int64_t         qgain;
        FilterType      type;
    };
    inline void setType( const FilterType t ) { param1.type=t; };

    float checkF( const int32_t fv )
    {
        return fcontrolMin <= fv  ? fcontrolMin : fcontrolMax >= fv  ? fcontrolMax : fv;
    };

    void limitQ( void )
    {
        if( param1.qgain < qcontrolMin ) {
            param1.qgain = qcontrolMin;
            return;
        }
        // experimental
//        if( param1.fgain < 0.17f  )
//            param1.qgain = std::min( param1.qgain, 3.0f );
//        else if( param1.qgain > qcontrolMax )
//            param1.qgain = qcontrolMax;
    };


    inline void setF( const uint32_t fv )
    {
        param1.fgain = checkF( fv );
        limitQ();
    };

    inline void setQ( const uint32_t qv = 0 )
    {
        param1.qgain = qv;
        limitQ();
    };

protected:
    Param       param1;
};
// --------------------------------------------------------------------
class Filter4PoleInteger : public Filter4PoleIntegerBase {
public:
    Filter4PoleInteger()
    {
        clear();
    };

    void clear(void)
    {
        channelA        = {0};
        param1          = {0};
        param1.fgain    = fcontrolMax;
        param1.qgain    = qcontrolMin;
    };
/*
 --------------- timer 1006
 ---------------- timer 994
 ---------------- timer 1000
 ---------------- timer 1002
 ---------------- timer 992
 ---------------- timer 1002
 ---------------- timer 991
 ---------------- timer 997
 ---------------- timer 987
 ---------------- timer 997
 ---------------- timer 995
 ---------------- timer 998
 ---------------- timer 988
 */
    // INPUT MUST NOT OVERFLOW : float input abs(in) <= 1.0 !!!!!!!!!!!!!
    //     anyway -- this is only for testing
    inline void getA( const float inA, float& outA  )
    {
        doFeedbackA( inA * rangeI );
        switch( param1.type ) {
        case LOWPASS:
            outA = float( channelA.y0d ) * rangeO;
            return;
        case BANDPASS1:
            outA = float( channelA.y0d - channelA.y0a ) * rangeO;
            return;
        case BANDPASS2:
            outA = float( channelA.y0d - channelA.y0b ) * rangeO;
            return;
        case BANDPASS3:
            outA = float( channelA.y0d - channelA.y0c ) * rangeO;
            return;
        }
    };

    inline void getA( const int32_t inA, int32_t& outA )
    {
        doFeedbackA( inA * rangeI );
        switch( param1.type ) {
        case LOWPASS:
            outA = channelA.y0d >> rangeOExp;
            return;
        case BANDPASS1:
            outA = ( channelA.y0d - channelA.y0a ) >> rangeOExp;
            return;
        case BANDPASS2:
            outA = ( channelA.y0d - channelA.y0b ) >> rangeOExp;
            return;
        case BANDPASS3:
            outA = ( channelA.y0d - channelA.y0c ) >> rangeOExp;
            return;
        }
    };
    inline int32_t getLowpass( const int32_t inA )
    {
        doFeedbackA( inA * rangeI );
        return channelA.y0d >> rangeOExp;
    };
    inline int32_t getBandpass1( const int32_t inA )
    {
        doFeedbackA( inA * rangeI );
        return (channelA.y0d - channelA.y0a) >> rangeOExp;
    };
    inline int32_t getBandpass2( const int32_t inA )
    {
        doFeedbackA( inA * rangeI );
        return (channelA.y0d - channelA.y0b) >> rangeOExp;
    };
    inline int32_t getBandpass3( const int32_t inA )
    {
        doFeedbackA( inA * rangeI );
        return (channelA.y0d - channelA.y0c) >> rangeOExp;
    };

private:
    inline void doFeedbackA( const int32_t inA )
    {
        const int64_t xA = inA - ( limitx3hard<1>( channelA.y0d ) * param1.qgain ) >> qNormExp;
//        const int64_t xA = inA - ( i64LimitClip<clipTuning>( channelA.y0d ) * param1.qgain )>>qNormExp;
        channelA.y0a = ((( channelA.y0a - xA           ) * param1.fgain ) >> fNormExp ) + xA;
        channelA.y0b = ((( channelA.y0b - channelA.y0a ) * param1.fgain ) >> fNormExp ) + channelA.y0a;
        channelA.y0c = ((( channelA.y0c - channelA.y0b ) * param1.fgain ) >> fNormExp ) + channelA.y0b;
        channelA.y0d = ((( channelA.y0d - channelA.y0c ) * param1.fgain ) >> fNormExp ) + channelA.y0c;
    };
    Channel     channelA;
}; // end Filter4PoleInteger
// --------------------------------------------------------------------
#if 0

class Filter4PoleIntegerStereo : public Filter4PoleIntegerBase {
public:
    Filter4PoleIntegerStereo()
    {
        clear();
    };
    void clear(void)
    {
        channelA    = {0};
        channelB    = {0};
        param1      = {0};
//        setF();
        setQ();
    };

    // INPUT MUST NOT OVERFLOW : float input abs(in) <= 1.0 !!!!!!!!!!!!!
    //     anyway -- this is only for testing
    inline void get( const float inA, const float inB, float& outA, float& outB )
    {
        doFeedback( inA * rangeI, inB * rangeI );
        switch( param1.type ) {
        case LOWPASS:
            outA = float( channelA.y0d ) * rangeO;
            outB = float( channelB.y0d ) * rangeO;
            return;
        case BANDPASS1:
            outA = float( channelA.y0d - channelA.y0a ) * rangeO;
            outB = float( channelB.y0d - channelB.y0a ) * rangeO;
            return;
        case BANDPASS2:
            outA = float( channelA.y0d - channelA.y0b ) * rangeO;
            outB = float( channelB.y0d - channelB.y0b ) * rangeO;
            return;
        case BANDPASS3:
            outA = float( channelA.y0d - channelA.y0c ) * rangeO;
            outB = float( channelB.y0d - channelB.y0c ) * rangeO;;
            return;
        }
    };

    inline void get( const int32_t inA, const int32_t inB, int32_t& outA, int32_t& outB )
    {
        doFeedback( inA * rangeI, inB * rangeI );
        switch( param1.type ) {
        case LOWPASS:
            outA = channelA.y0d >> rangeOExp;
            outB = channelB.y0d >> rangeOExp;
            return;
        case BANDPASS1:
            outA = ( channelA.y0d - channelA.y0a ) >> rangeOExp;
            outB = ( channelB.y0d - channelB.y0a ) >> rangeOExp;
            return;
        case BANDPASS2:
            outA = ( channelA.y0d - channelA.y0b ) >> rangeOExp;
            outB = ( channelB.y0d - channelB.y0b ) >> rangeOExp;
            return;
        case BANDPASS3:
            outA = ( channelA.y0d - channelA.y0c ) >> rangeOExp;
            outB = ( channelB.y0d - channelB.y0c ) >> rangeOExp;
            return;
        }
    };

private:
    inline void doFeedback( const int32_t inA, const int32_t inB )
    {
        const int64_t xA = inA - ( i64LimitClip<clipTuning>( channelA.y0d ) * param1.qgain );
        const int64_t xB = inB - ( i64LimitClip<clipTuning>( channelB.y0d ) * param1.qgain );
        channelA.y0a    = ((( channelA.y0a - xA ) * param1.fcurrent ) >> 32 ) + xA;
        channelB.y0a    = ((( channelB.y0a - xB ) * param1.fcurrent ) >> 32 ) + xB;
        channelA.y0b    = ((( channelA.y0b - channelA.y0a ) * param1.fcurrent ) >> 32 ) + channelA.y0a;
        channelB.y0b    = ((( channelB.y0b - channelB.y0a ) * param1.fcurrent ) >> 32 ) + channelB.y0a;
        channelA.y0c    = ((( channelA.y0c - channelA.y0b ) * param1.fcurrent ) >> 32 ) + channelA.y0b;
        channelB.y0c    = ((( channelB.y0c - channelB.y0b ) * param1.fcurrent ) >> 32 ) + channelB.y0b;
        channelA.y0d    = ((( channelA.y0d - channelA.y0c ) * param1.fcurrent ) >> 32 ) + channelA.y0c;
        channelB.y0d    = ((( channelB.y0d - channelB.y0c ) * param1.fcurrent ) >> 32 ) + channelB.y0c;
    };
    Channel     channelA;
    Channel     channelB;
}; // end Filter4PoleIntegerStereo
#endif

} // end namespace filter


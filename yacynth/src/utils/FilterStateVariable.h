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
 * File:   FilterStateVariable.h
 * Author: Istvan Simon
 *
 * Created on March 15, 2016, 6:59 PM
 */

#include    "FilterBase.h"

#include    <cstdint>
#include    <iostream>
#include    <iomanip>

namespace filter {
// --------------------------------------------------------------------
//
// OBSOLATE --> refactor
//
// Chamberlain state variable filter
// 2x oversampled
//
#if 0

class FilterSV : public FilterBaseOld {
public:
    static constexpr    uint16_t oversamplingRate   = 2;
    static inline float getFc(const float freq)     { return getFcOSR<oversamplingRate>(freq); };
    static inline float getFc2PI(const float freq)  { return getFc2PIOSR<oversamplingRate>(freq); };
    static inline float checkFc(const float freq)   { return checkFcOSR<oversamplingRate>(freq); };

    static constexpr    float    fMin               = 0.0004;   // 20/48000
    static constexpr    float    fMax               = 0.45;     // 20000/48000
    static constexpr    float    qMin               = 0.001;   // MUST BE CHECKED
    static constexpr    float    qMax               = 1.6;      // unstable above 1.6

    static inline float evalFc( const float fc )
    {
        const float fclim = fc < fMin ? fMin : fc > fMax ? fMax : fc;
        // good approximation for a 2* oversampled filter
        return 3.0f * fclim;
    };
    static inline float evalQ( const float q )
    {
        return q < qMin ? qMin : q > qMax ? qMax : q;
    };

    enum FilterType {
        BYPASS,                // get the input: the filter is runnning
        LOWPASS,
        HIGHPASS,
        BANDPASS,
        NOTCH,
        PEEK,
        PEEKPIN
    };

    struct Channel {
        float   band1;
        float   low1;
        float   high1;
        float   band2;
        float   low2;
        float   high2;
    };

    struct Param {
        void setEffective( void )
        {
            fEff = f.current();
            qEff = std::min( q.current(), 2.0f - f.current() );
        };

        ParamIterLinear f;
        ParamIterLinear q;
        // effective multipliers from f and q current to increase stability
        float           fEff;
        float           qEff;
        FilterType      type;
    };

    void clear(void)
    {
        channelA = {0};
        channelB = {0};
        param1.f.set( evalFc(fMax) );
        param1.q.set( qMin );
        param1.setEffective();
    };

    FilterSV()
    { clear(); };

    inline void setType( const FilterType t ) { param1.type=t; };

    // input is fc = f/fs fs =
    inline void setF( const float fc )
    {
        param1.f.setStep( evalFc( fc ) );
    };
    inline void setQ( const float qv )
    {
        param1.q.setStep( evalQ( qv ) );
    };

    // this should be called after a parameter change
    // returns true if there is something to set yet
    inline bool sweep( void )
    {
        const bool retf = param1.f.step();
        const bool retq = param1.q.step();
        if( retf || retq ) {
            param1.setEffective();
            return true;
        }
        return false;
    };

    inline void getA( const float in, float& out )
    {
        doLinA(in);
        switch( param1.type ) {
        case BYPASS:
            out = in;
            return;
        case LOWPASS:
            out = channelA.low1;
            return;
        case HIGHPASS:
            out = ( channelA.high1 + channelA.high2 ) * 0.5f;
            return;
        case BANDPASS:
            out = ( channelA.band1 + channelA.band2 );
            return;
        case PEEK:
            out = channelA.low2 - channelA.high1;
            return;
        case NOTCH:
            out = channelA.low2 + channelA.high2;
            return;
        case PEEKPIN:
            out = ( channelA.low2 - channelA.high1 + in ) * 0.03f;
            return;
        }
    };

    inline float getA( const float in )
    {
        doLinA( in );
        switch( param1.type ) {
        case BYPASS:
            return in;
        case LOWPASS:
            return channelA.low1;
        case HIGHPASS:
            return ( channelA.high1 + channelA.high2 ) * 0.5f;
        case BANDPASS:
            return ( channelA.band1 + channelA.band2 );
        case PEEK:
            return channelA.low2 - channelA.high1;
        case NOTCH:
            return channelA.low2 + channelA.high2;
        case PEEKPIN:
            return ( channelA.low2 - channelA.high1 + in ) * 0.03f;
        }
    };

    inline void getB( const float in, float& out )
    {
        doLinB( in );
        switch( param1.type ) {
        case BYPASS:
            out = in;
            return;
        case LOWPASS:
            out = channelB.low1;
            return;
        case HIGHPASS:
            out =  ( channelB.high1 + channelB.high2 ) * 0.5f;
            return;
        case BANDPASS:
            out = ( channelB.band1 + channelB.band2 );
            return;
        case PEEK:
            out = channelB.low2 - channelB.high1;
            return;
        case NOTCH:
            out = channelB.low2 + channelB.high2;
            return;
        case PEEKPIN:
            out = ( channelB.low2 - channelB.high1 + in ) * 0.03f;
            return;
        }
    };

    inline float getB( const float in )
    {
        doLinB( in );
        switch( param1.type ) {
        case BYPASS:
            return in;
        case LOWPASS:
            return channelB.low1;
        case HIGHPASS:
            return ( channelB.high1 + channelB.high2 ) * 0.5f;
        case BANDPASS:
            return ( channelB.band1 + channelB.band2 );
        case PEEK:
            return channelB.low2 - channelB.high1;
        case NOTCH:
            return channelB.low2 + channelB.high2;
        case PEEKPIN:
            return ( channelB.low2 - channelB.high1 + in ) * 0.03f;
        }
    };

// stereo
    inline void getAB( const float inA, const float inB, float& outA, float& outB )
    {
        doLinAB( inA, inB );
        switch( param1.type ) {
        case BYPASS:
            outA = inA;
            outB = inB;
            return;
        case LOWPASS:
            outA = channelA.low1;
            outB = channelB.low1;
            return;
        case HIGHPASS:
            outA = ( channelA.high1 + channelA.high2 ) * 0.5f;
            outB = ( channelB.high1 + channelB.high2 ) * 0.5f;
            return;
        case BANDPASS:
            outA = ( channelA.band1 + channelA.band2 );
            outB = ( channelB.band1 + channelB.band2 );
            return;
        case PEEK:
            outA = channelA.low2 - channelA.high1;
            outB = channelB.low2 - channelB.high1;
            return;
        case NOTCH:
            outA = channelA.low2 + channelA.high2;
            outB = channelB.low2 + channelB.high2;
            return;
        case PEEKPIN:
            outA = ( channelA.low2 - channelA.high1 + inA ) * 0.03f;
            outB = ( channelB.low2 - channelB.high1 + inB ) * 0.03f;
            return;
        }
    };

private:
    // Beat Frei: Digital Sound Generation – Part 2 chap 3.3 - page 13 - 14
    inline void doLinA( const float in )
    {
        // pass 1
        channelA.low1   = channelA.low2         + channelA.band2 * param1.fEff;
        channelA.high1  = in - channelA.low1    - channelA.band2 * param1.qEff;
        channelA.band1  = channelA.band2        + channelA.high1 * param1.fEff;
        // pass 2
        channelA.low2   = channelA.low1         + channelA.band1 * param1.fEff;
        channelA.high2  = in - channelA.low2    - channelA.band1 * param1.qEff;
        channelA.band2  = channelA.band1        + channelA.high2 * param1.fEff;
    };
    inline void doLinB( const float in )
    {
        // pass 1
        channelB.low1   = channelB.low2         + channelB.band2 * param1.fEff;
        channelB.high1  = in - channelB.low1    - channelB.band2 * param1.qEff;
        channelB.band1  = channelB.band2        + channelB.high1 * param1.fEff;
        // pass 2
        channelB.low2   = channelB.low1         + channelB.band1 * param1.fEff;
        channelB.high2  = in - channelB.low2    - channelB.band1 * param1.qEff;
        channelB.band2  = channelB.band1        + channelB.high2 * param1.fEff;
    };
    inline void doLinAB( const float inA, const float inB )
    {
        // pass1
        channelA.low1   = channelA.low2         + channelA.band2 * param1.fEff;
        channelB.low1   = channelB.low2         + channelB.band2 * param1.fEff;
        channelA.high1  = inA - channelA.low1   - channelA.band2 * param1.qEff;
        channelB.high1  = inB - channelB.low1   - channelB.band2 * param1.qEff;
        channelA.band1  = channelA.band2        + channelA.high1 * param1.fEff;
        channelB.band1  = channelB.band2        + channelB.high1 * param1.fEff;
        // pass2
        channelA.low2   = channelA.low1         + channelA.band1 * param1.fEff;
        channelB.low2   = channelB.low1         + channelB.band1 * param1.fEff;
        channelA.high2  = inA - channelA.low2   - channelA.band1 * param1.qEff;
        channelB.high2  = inB - channelB.low2   - channelB.band1 * param1.qEff;
        channelA.band2  = channelA.band1        + channelA.high2 * param1.fEff;
        channelB.band2  = channelB.band1        + channelB.high2 * param1.fEff;
    };
    Channel     channelA;
    Channel     channelB;
    Param       param1;
}; // end class FilterSV
// --------------------------------------------------------------------

template< std::size_t tableSizeX >
class  FilterSVtableF {
public:
    static constexpr std::size_t tableSizeExp = tableSizeX;
    static constexpr std::size_t tableSize = 1<<tableSizeExp;
    static constexpr double PI  = 3.141592653589793238462643383279502884197;
    FilterSVtableF()
    :   FilterSVtableF( 30.0, 16000.0, 48000.0, 2 ) // default here
    {};
    FilterSVtableF( const double minFreq, const double maxFreq, const float samplingFrequency, const int16_t oversamplingRate )
    {
        const double mult   = PI / samplingFrequency / oversamplingRate;
        const double dfreq  = std::pow( 2.0f, std::log2( maxFreq / minFreq ) / tableSize ) ;
        double freq = minFreq;
        for( int i = tableSize-1; i >= 0 ; --i ) {
            f[ i ] =  2.0f * std::sin( mult * freq );
            freq *= dfreq;
        }
    };
    float get( const uint64_t index ) { return index < tableSize ? f[ index ] : f[ tableSize - 1 ]; };

private:
    float           f[ tableSize ];
};
// --------------------------------------------------------------------

template< std::size_t tableSizeX >
class  FilterSVtableQ {
public:
    static constexpr std::size_t tableSizeExp = tableSizeX;
    static constexpr std::size_t tableSize = 1<<tableSizeExp;
    FilterSVtableQ()
    :   FilterSVtableQ( 0.001, 1.6 )
    {};
    FilterSVtableQ( const double minQ, const double maxQ  )
    {
        const double dq = std::pow( 2.0f, std::log2( maxQ / minQ ) / tableSize );
        double qval = minQ;
        for( int i = tableSize-1; i >= 0 ; --i ) {
            q[ i ] = qval;
            qval *= dq;
        }
    }
    float get( const uint64_t index ) { return index < tableSize ? q[ index ] : q[ tableSize - 1 ]; };

private:
    float   q[tableSize];
};
#endif
// --------------------------------------------------------------------
} // end namespace filter
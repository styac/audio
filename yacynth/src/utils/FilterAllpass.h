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
 * File:   FilterAllpass.h
 * Author: Istvan Simon
 *
 * Created on March 21, 2016, 6:52 PM
 */

// http://www.music.mcgill.ca/~ich/classes/FiltersChap2.pdf

// http://www.micromodeler.com/dsp/
// http://home.deib.polimi.it/bestagini/_Slides/lesson_3.pdf

#include    "FilterBase.h"

using namespace tables;

typedef float v4sf __attribute__((mode(SF)))  __attribute__ ((vector_size(16),aligned(16)));

namespace filter {


template< std::size_t channelCountExp >
class FilterAllpass1 : public FilterBase {
public:
    static constexpr uint8_t  channelCount      = 1<<channelCountExp;
    static constexpr uint8_t  channelCountMask  = channelCount-1;
    static constexpr int32_t  fMin              = 0x1e000000; // octave 30.5 -- 16 khz
    static constexpr int32_t  fMax              = 0x16100000; // octave 21  -- 45 Hz
    static constexpr float    fcontrolMin       = ( std::sin(PI2*fcMin) - 1 ) / std::cos(PI2*fcMin);
    static constexpr float    fcontrolMax       = ( std::sin(PI2*fcMax) - 1 ) / std::cos(PI2*fcMax);

    struct Channel {
        void clear(void)
        {
            for( auto j=0; j < channelCount; ++j ) {
                s[j] = 0.0f;
                kmulf[j] = 0.0f;
            }
        }
        union {
            v4sf    v4[channelCount];
            struct {
                float   s[channelCount];
                float   kmulf[channelCount];    // forward multiplier
                float   kmulb[channelCount];    // feedback multiplier
                float   gmul[channelCount];     // out gain ??
            };
        };
    };


    FilterAllpass1()
    {
        clear();
    };


    void clear(void)
    {
        channel.clear();
    };

    // change the feedback parameter
    // forward will be delayed by 1 tick
    inline void setFreq( const uint8_t index, const int32_t v )
    {
        const float freq = ftableSinCosPi2.getFloat( v );
        channel.kmulb[index&channelCountMask] = std::max( fcontrolMin, std::min( fcontrolMax, freq ));
    };

    // change the feedback parameter
    // forward will be delayed by 1 tick
    inline void setK( const uint8_t index, const float v )
    {
        channel.kmulb[index&channelCountMask] = v;
    };

    inline void setGain( const uint8_t index, const float v )
    {
        channel.gmul[index&channelCountMask] = v;
    };

    //=================================================
    //
    //      >-------- * +kmulf------v
    //      ^                       v
    //  x -->-  + --->  z-1 ----->  + --->  y
    //          ^                       v
    //          ^ ------ * -kmulb------ <
    //
    // 2 channel (STEREO)
    // "count" paralel (count/2 -> each channel)
    // 2 multiplier 1 delay
    template< std::size_t count >
    inline void set2mul( const float x0, const float x1, float& y0, float& y1 )
    {
        static_assert( 2*count <= channelCount, "count greater then channel count" );
        static_assert( count > 0, "count == zero" );
        static_assert( channelCount > 1, "channelCount small" );

        const float t0 = channel.s[0] + x0 * channel.kmulf[0];
        const float t1 = channel.s[1] + x1 * channel.kmulf[1];
        channel.s[0] = x0 - t0 * channel.kmulb[0];
        channel.s[1] = x1 - t1 * channel.kmulb[1];
        // forward multiplier delayed by 1 tick
        channel.kmulf[0] = channel.kmulb[0];
        channel.kmulf[1] = channel.kmulb[1];
        y0 = t0 * channel.gmul[0];
        y1 = t1 * channel.gmul[1];
        for( auto i=2u; i<2*count; i += 2 ) {
            const float t0 = channel.s[i] + x0 * channel.kmulf[i];
            const float t1 = channel.s[i+1] + x1 * channel.kmulf[i+1];
            channel.s[i]    = x0 - t0 * channel.kmulb[i];
            channel.s[i+1]  = x1 - t1 * channel.kmulb[i+1];
            // forward will be delayed by 1 tick
            channel.kmulf[i]    = channel.kmulb[i];
            channel.kmulf[i+1]  = channel.kmulb[i+1];
            y0 += t0 * channel.gmul[i];
            y1 += t1 * channel.gmul[i+1];
        }
    };

    //
    // 1 channel (MONO)
    // "count" paralel
    // 2 multiplier 1 delay
    //
    template< std::size_t count >
    inline void set2mul( const float x, float& y )
    {
        static_assert( count <= channelCount, "count greater then channel count" );
        static_assert( count > 0, "count == zero" );
        const float t = channel.s[0] + x * channel.kmulf[0];
        channel.s[0] = x - t * channel.kmulb[0];
        // forward multiplier delayed by 1 tick
        channel.kmulf[0] = channel.kmulb[0];
        y = t * channel.gmul[0];
        for( auto i=1u; i<count; ++i ) {
            const float t = channel.s[i] + x * channel.kmulf[i];
            channel.s[i] = x - t * channel.kmulb[i];
            // forward multiplier delayed by 1 tick
            channel.kmulf[i] = channel.kmulb[i];
            y += t * channel.gmul[i];
        }
    };

    //
    // 1 multiply 1 delay version
    // see > http://www2.units.it/ramponi/teaching/DSP/materiale/Ch6%282%29.pdf slide 40
    // Mitra http://www.ingelec.uns.edu.ar/pds2803/materiales/articulos/thedigitalallpassfilter.pdf
    // makes incredible big spikes for param change
    // check the speed !!!
    //
    template< std::size_t count >
    inline void set1mul( const float x, float& y )
    {
        const float t = ( channel.s[0] + x ) * channel.kmulb[0];
        y = ( channel.s[0] + t ) * channel.gmul[0];
        channel.s[0] = x - t;
        for( auto i=1u; i<count; ++i ) {
            const float t = ( channel.s[i] + x ) * channel.kmulb[i];
            y = ( channel.s[i] + t ) * channel.gmul[i];
            channel.s[i] = x - t;
        }
    };

protected:
    Channel     channel;
}; // end FilterAllpass1
// --------------------------------------------------------------------
//
//
// http://www.music.mcgill.ca/~ich/classes/FiltersChap2.pdf
//  page 12
//
template< std::size_t channelCountExp >
class FilterAllpass2 : public FilterBase {
public:
    static constexpr uint8_t  channelCount      = 1<<channelCountExp;
    static constexpr uint8_t  channelCountMask  = channelCount-1;
    static constexpr int32_t  fMin              = 0x1e000000; // octave 30.5 -- 16 khz
    static constexpr int32_t  fMax              = 0x16100000; // octave 21  -- 45 Hz
    static constexpr float    fcontrolMin       = std::cos(PI2*fcMin);
    static constexpr float    fcontrolMax       = std::cos(PI2*fcMax);

    // c = (  sin( 2 * pi * fbw/fs ) - 1 ) / cos ( 2 * PI * fbw/fs )
    // d = - cos (  2 * PI * fcut/fs )

    struct Channel {
        void clear(void)
        {
            for( auto j=0; j < channelCount; ++j ) {
                s[0][j] = s[1][j] = 0.0f;
            }
        }
        union {
            v4sf    v4[channelCount];
            struct {
                float   s[2][channelCount];
                float   cmul[channelCount];
                float   dmul[channelCount];
            };
        };
        float   gmul[channelCount];     // out gain ??
    };


    FilterAllpass2()
    {
        clear();
    };



    void clear(void)
    {
    };

    inline float checkF( const float fv )
    {
    };

    inline float checkQ( const float qv )
    {
    };

    inline void setQ( float qv )
    {
    };

    inline void setF( float fv )
    {
    };


    // x = 2*pi*f/fs
    // c = (sin(x1) - 1)/cos(x1)
    // d = -cosv2
    // c1 = -c
    // c2 = d*(1-c)

    inline void set1( const float x, float& y  )
    {
        const float xc = x * channel.cmul[0];
        y = xc + channel.s[0][0];
        const float yc = y * channel.cmul[0];
        // ????
        channel.s[0][0] = channel.dmul[0] * ( x - y - xc + yc ) + channel.s[1][0];
        channel.s[1][0] = x - yc;
    };

    inline void set2( const float x, float& y  )
    {
        const float s0d = channel.s[0][0] * channel.dmul[0];
        const float s1d = s0d + channel.s[1][0];
        const float s1c = s1d * channel.cmul[0];
        const float xin = x - s0d + s1c;
        y = s1d - ( xin + s0d ) * channel.cmul[0];
        channel.s[1][0] = channel.s[0][0];
        channel.s[0][0] = xin;
    };


private:
    Channel channel;

}; // end FilterAllpass2

// --------------------------------------------------------------------
// ********************** OBSOLATE

class FilterAllpassBase : public FilterBase {
public:
    static constexpr    uint16_t oversamplingRate   = 1;
    static constexpr    float    fMin               = 25.0/48000.0;   // highest freq ~19500
    static constexpr    float    fMax               = 20000.0/48000.0;     // lowest freq ~23 lower might be unstable
    static constexpr    int      qRateExp           = 48;

protected:
    // NEW

};

// --------------------------------------------------------------------
template< std::size_t tableSizeX >
class FilterAllpassTableF {
public:
    static constexpr std::size_t tableSizeExp = tableSizeX;
    static constexpr std::size_t tableSize = 1<<tableSizeExp;
    static constexpr double PI  = 3.141592653589793238462643383279502884197;
    FilterAllpassTableF()
    :   FilterAllpassTableF( 25.0, 19900.0, 48000.0, 1 )
    {};
    FilterAllpassTableF( const double minFreq, const double maxFreq, const float samplingFrequencyP, const int16_t oversamplingRateP )
    :   oversamplingRate(oversamplingRateP)
    ,   samplingFrequency(samplingFrequencyP)
    {
        const double mult   = 1.0 / samplingFrequency / oversamplingRate;
        const double dfreq  = std::pow( 2.0f, std::log2( maxFreq / minFreq ) / tableSize ) ;
        double freq = minFreq;
        for( int i = 0; i < tableSize; ++i ) {
            f[ i ] =  freq * mult;
            freq *= dfreq;
        }
    };
    float get( const uint64_t index ) { return index < tableSize ? f[ index ] : f[ tableSize - 1 ]; };

private:
    float           f[ tableSize ];
    const int16_t   oversamplingRate;
    const float     samplingFrequency;
};

template< std::size_t tableSizeX >
class FilterAllpassTableQ {
public:
    static constexpr std::size_t tableSizeExp = tableSizeX;
    static constexpr std::size_t tableSize = 1<<tableSizeExp;
    static constexpr double PI  = 3.141592653589793238462643383279502884197;
    FilterAllpassTableQ()
    :   FilterAllpassTableQ( 0.2, 5.0 )
    {};
    FilterAllpassTableQ( const double minQ, const double maxQ )
    {
        const double dq  = std::pow( 2.0f, std::log2( maxQ / minQ ) / tableSize ) ;
        double qv = minQ;
        for( int i = 0; i < tableSize; ++i ) {
            q[ i ] =  qv;
            qv *= dq;
        }
    };
    float get( const uint64_t index ) { return index < tableSize ? q[ index ] : q[ tableSize - 1 ]; };

private:
    float           q[ tableSize ];
};

} // end namespace
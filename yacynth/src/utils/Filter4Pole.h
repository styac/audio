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
 * File:   Filter4Pole.h
 * Author: Istvan Simon
 *
 * Created on March 21, 2016, 6:27 PM
 */

#include    "FilterBase.h"
#include    "Limiters.h"
#include    "v4.h"

#include    <cstdint>
#include    <iostream>
#include    <iomanip>

using namespace limiter;

namespace filter {
// --------------------------------------------------------------------
template< std::size_t channelCountExp >
class Filter4Pole : public FilterBase {
public:
    static constexpr uint8_t  channelCount      = 1<<channelCountExp;
    static constexpr uint8_t  channelCountMask  = channelCount-1;
    static constexpr uint8_t  stateCount        = 4;

    // state indices -> A,B,C,D
    static constexpr std::size_t    SA  = 0;
    static constexpr std::size_t    SB  = 1;
    static constexpr std::size_t    SC  = 2;
    static constexpr std::size_t    SD  = 3;

    static constexpr int32_t  fMin              = 0x1e000000; // octave 30.5 -- 16 khz
    static constexpr int32_t  fMax              = 0x16100000; // octave 21  -- 45 Hz
    static constexpr float    fcontrolMin       = std::exp(-PI2*fcMin);
    static constexpr float    fcontrolMax       = std::exp(-PI2*fcMax);
    static constexpr float    qcontrolMin       = 0.0;
    static constexpr float    qcontrolMax       = 3.995; // checked

    struct alignas(16) Channel {
        void clear(void)
        {
            for( auto i=0; i < stateCount; ++i )
                for( auto j=0; j < channelCount; ++j )
                    s[i][j] = 0.0f;
        }
        union {
            v4sf    v4[channelCount];
            float   s[stateCount][channelCount];

        };
        struct {
            float   fmul[channelCount];
            float   qmul[channelCount];
            float   gmul[channelCount];

            // possible audio rate interpolation with low pass filter - 20 Hz??
            // update fmul,qmul by 1pole filtering from ftarget,qtarget,gtarget
            // fmul = fmul * k + ftarget * (1-k)
//            float   ftarget[channelCount];
//            float   qtarget[channelCount];
//            float   gtarget[channelCount];
        };
    };


    Filter4Pole()
    {
        clear();
    };

    void clear(void)
    {
        channel.clear();
    };

    inline void setGain( const uint8_t ind, const float gn )
    {
        channel.gmul[ ind & channelCountMask ] = gn;
    }

    inline void setFreq( const uint8_t ind, const int32_t fv )
    {
        const float freq = ftableExp2Pi.getFloat( fv );
        channel.fmul[ ind & channelCountMask ] = std::max( fcontrolMax, std::min( fcontrolMin, freq ));

  std::cout << "ind " << uint16_t(ind)  << " f " << channel.fmul[ ind & channelCountMask ] << std::endl;
    };

    inline void setQ( const uint8_t ind, const float qv )
    {
        channel.qmul[ ind & channelCountMask ] = std::max( qcontrolMin, std::min( qcontrolMax, qv ));

  std::cout << "ind " << uint16_t(ind) << " q "  << channel.qmul[ ind & channelCountMask ] << std::endl;
    };

    // safer
    // all bandpass
    template< std::size_t S1, std::size_t S2, std::size_t N0, std::size_t L >
    inline float getBP(void) const
    {
        static_assert((N0+L) <= channelCount, "channel count too small" );
        float s = ( channel.s[S1][N0] - channel.s[S2][N0] ) * channel.gmul[N0];
        for( auto i=1u; i < L; ++i ) {
            s += ( channel.s[S1][N0+i] - channel.s[S2][N0+i] ) * channel.gmul[N0+i];
        }
        return s;
    }

    // N0 - lowpass
    // N0+1,... bandpass
    template< std::size_t S1, std::size_t S2, std::size_t N0, std::size_t L >
    inline float getLBP(void) const
    {
        static_assert((N0+L) <= channelCount, "channel count too small" );
        float s = channel.s[S1][N0] * channel.gmul[N0];
        for( auto i=1u; i<L; ++i ) {
            s += ( channel.s[S1][N0+i] - channel.s[S2][N0+i] ) * channel.gmul[N0+i];
        }
        return s;
    }

    // safer
    template< std::size_t count >
    inline void set( const float in )
    {
        static_assert( count <= channelCount, "count greater then channel count" );
        static_assert( count > 0, "count == zero" );
        for( auto i=0u; i < count; ++i ) {
            const float inx = in - channel.s[SD][i] * channel.qmul[i];
            channel.s[SA][i]  = ( channel.s[SA][i] - inx ) * channel.fmul[i] + inx;
            channel.s[SB][i]  = ( channel.s[SB][i] - channel.s[SA][i] ) * channel.fmul[i] + channel.s[SA][i];
            channel.s[SC][i]  = ( channel.s[SC][i] - channel.s[SB][i] ) * channel.fmul[i] + channel.s[SB][i];
            channel.s[SD][i]  = ( channel.s[SD][i] - channel.s[SC][i] ) * channel.fmul[i] + channel.s[SC][i];
        }
    };

    inline void set( std::size_t count, const float in )
    {
        for( auto i=0; i < count; ++i ) {
            const float inx = in - channel.s[SD][i] * channel.qmul[i];
            channel.s[SA][i]  = ( channel.s[SA][i] - inx ) * channel.fmul[i] + inx;
            channel.s[SB][i]  = ( channel.s[SB][i] - channel.s[SA][i] ) * channel.fmul[i] + channel.s[SA][i];
            channel.s[SC][i]  = ( channel.s[SC][i] - channel.s[SB][i] ) * channel.fmul[i] + channel.s[SB][i];
            channel.s[SD][i]  = ( channel.s[SD][i] - channel.s[SC][i] ) * channel.fmul[i] + channel.s[SC][i];
        }
    };

protected:
    Channel channel;
}; // end Filter4pole

// --------------------------------------------------------------------
} // end namespace filter
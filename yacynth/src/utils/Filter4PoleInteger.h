#pragma once
#if 0
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

typedef int v4si __attribute__((mode(SI)))  __attribute__ ((vector_size(16),aligned(16)));

namespace filter {

// --------------------------------------------------------------------

template< std::size_t channelCountExp >
class Filter4PoleInteger : public FilterBaseOld {
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
    static constexpr int32_t  fcontrolMin       = std::exp(-PI2*fcMin) * (1L<<31);
    static constexpr int32_t  fcontrolMax       = std::exp(-PI2*fcMax) * (1L<<31);
    static constexpr int32_t  qcontrolMin       = 0;
    static constexpr int32_t  qcontrolMax       = 3.995 * (1L<<24);

    struct alignas(16) Channel {
        void clear(void)
        {
            for( auto i=0; i < stateCount; ++i )
                for( auto j=0; j < channelCount; ++j )
                    s[i][j] = 0;
        }
        union {
            v4si        v4[channelCount];
            int32_t     s[stateCount][channelCount];
        };
        struct {
            int32_t fmul[channelCount];
            int32_t qmul[channelCount];
            int32_t gmul[channelCount];
        };
    };


    Filter4PoleInteger()
    {
        clear();
    };

    void clear(void)
    {
        channel.clear();
    };

    inline void setGain( const uint8_t ind, const int32_t gn )
    {
        channel.gmul[ ind & channelCountMask ] = gn;
    }

    inline void setFreq( const uint8_t ind, const int32_t fv )
    {
        const int32_t freq = ftableExp2Pi.getInt( fv );
        channel.fmul[ ind & channelCountMask ] = std::max( fcontrolMax, std::min( fcontrolMin, freq ));

  std::cout << "ind " << uint16_t(ind)  << " f " << channel.fmul[ ind & channelCountMask ] << std::endl;
    };

    // 24 bit -> 1.0
    inline void setQ( const uint8_t ind, const int32_t qv )
    {
        channel.qmul[ ind & channelCountMask ] = std::max( qcontrolMin, std::min( qcontrolMax, qv ));

  std::cout << "ind " << uint16_t(ind) << " q "  << channel.qmul[ ind & channelCountMask ] << std::endl;
    };

    // safer
    // all bandpass
    template< std::size_t S1, std::size_t S2, std::size_t N0, std::size_t L >
    inline int32_t getBP(void) const
    {
        static_assert((N0+L) <= channelCount, "channel count too small" );
        int32_t s = (( channel.s[S1][N0] - channel.s[S2][N0] ) * channel.gmul[N0]) >> 32;
        for( auto i=1u; i < L; ++i ) {
            s += (( channel.s[S1][N0+i] - channel.s[S2][N0+i] ) * channel.gmul[N0+i]) >> 32;
        }
        return s;
    }

    // N0 - lowpass
    // N0+1,... bandpass
    template< std::size_t S1, std::size_t S2, std::size_t N0, std::size_t L >
    inline int32_t getLBP(void) const
    {
        static_assert((N0+L) <= channelCount, "channel count too small" );
        int32_t s = ( channel.s[S1][N0] * channel.gmul[N0] ) >> 32;
        for( auto i=1u; i<L; ++i ) {
            s += (( channel.s[S1][N0+i] - channel.s[S2][N0+i] ) * channel.gmul[N0+i] ) >> 32;
        }
        return s;
    }

    // safer
    template< std::size_t count >
    inline void set( const int32_t in )
    {
        static_assert( count <= channelCount, "count greater then channel count" );
        static_assert( count > 0, "count == zero" );
        for( auto i=0u; i < count; ++i ) {
            const int64_t inx = in - (( channel.s[SD][i] * int64_t(channel.qmul[i]) ) >> 24 );
            channel.s[SA][i]  = ((( channel.s[SA][i] - inx ) * int64_t(channel.fmul[i])) >> 32 ) + inx;
            channel.s[SB][i]  = ((( channel.s[SB][i] - channel.s[SA][i] ) * int64_t(channel.fmul[i]) ) >> 32 ) + channel.s[SA][i];
            channel.s[SC][i]  = ((( channel.s[SC][i] - channel.s[SB][i] ) * int64_t(channel.fmul[i]) ) >> 32 ) + channel.s[SB][i];
            channel.s[SD][i]  = ((( channel.s[SD][i] - channel.s[SC][i] ) * int64_t(channel.fmul[i]) ) >> 32 ) + channel.s[SC][i];
        }
    };

protected:
    Channel channel;
}; // end Filter4PoleInteger


} // end namespace filter

#endif
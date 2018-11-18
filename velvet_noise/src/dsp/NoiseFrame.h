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
 * File:   NoiseFrame.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on May 30, 2016, 6:31 PM
 */

#include "GaloisNoiser.h"
#include <cstring>

template< std::size_t fexp, std::size_t chcount=1 >
struct FrameFloat {
    typedef float value_type;
    static constexpr std::size_t sectionSizeExp     = fexp;
    static constexpr std::size_t sectionSize        = 1<<sectionSizeExp;
    static constexpr std::size_t sectionSizeMask    = sectionSize-1;
    static constexpr std::size_t channelCount       = chcount;
    inline void clear(void)
    {
        memset( channel, 0, sizeof(channel) );
    }

    union  alignas(16) {
        float   channel[ channelCount ][ sectionSize ];
    };
};

// --------------------------------------------------------------------
template< std::size_t fexp, std::size_t chcount=1 >
struct FrameInt {
    typedef int32_t value_type;
    static constexpr std::size_t sectionSizeExp     = fexp;
    static constexpr std::size_t sectionSize        = 1<<sectionSizeExp;
    static constexpr std::size_t sectionSizeMask    = sectionSize-1;
    static constexpr std::size_t channelCount       = chcount;
    inline void clear(void)
    {
        memset( channel, 0, sizeof(channel) );
    }
    union  alignas(16) {
        int32_t channel[ channelCount ][ sectionSize ];
    };
};
// --------------------------------------------------------------------
template< typename T, std::size_t fexp, std::size_t chcount >
struct FrameInterleave {
    static constexpr std::size_t sectionSizeExp     = fexp;
    static constexpr std::size_t sectionSize        = 1<<sectionSizeExp;
    static constexpr std::size_t sectionSizeMask    = sectionSize-1;
    static constexpr std::size_t channelCount       = chcount;
    static constexpr std::size_t interleavedSize    = sectionSize*channelCount;
    inline void clear(void)
    {
        memset( channel, 0, sizeof(channel) );
    }
    
    inline void set( FrameInt<sectionSizeExp,channelCount> const & in, float norm )
    {
        for( uint32_t i=0u; i<sectionSize; ++i ) {
            for( uint32_t c=0u; c<channelCount; ++c) {
                channelX[i][c] = static_cast<T>(in.channel[c][i]*norm);                
            }
        }
    }

    inline void add( FrameInt<sectionSizeExp,channelCount> const & in, float norm )
    {
        for( uint32_t i=0u; i<sectionSize; ++i ) {
            for( uint32_t c=0u; c<channelCount; ++c) {
                channelX[i][c] += static_cast<T>(in.channel[c][i]*norm);
            }
        }
    }

    inline void set( FrameInt<sectionSizeExp,channelCount> const & in )
    {
        for( uint32_t i=0u; i<sectionSize; ++i ) {
            for( uint32_t c=0u; c<channelCount; ++c) {
                channelX[i][c] = in.channel[c][i];                
            }
        }
    }

    union {
        T channelX[ sectionSize ][ channelCount ];
        T channel[ interleavedSize ];        
    };
};

// --------------------------------------------------------------------

template< typename Tstore >
class NoiseFrame : public Tstore {
public:
    typedef typename Tstore::value_type storage_type;
    static constexpr std::size_t sectionSizeExp     = Tstore::sectionSizeExp;
    static constexpr std::size_t sectionSize        = Tstore::sectionSize;
    static constexpr std::size_t sectionSizeMask    = sectionSize-1;
    static constexpr std::size_t channelCount       = Tstore::channelCount;
    static constexpr std::size_t sSize              = 16;

    NoiseFrame() = delete;
    NoiseFrame( GaloisShifter& gs )
    :   noise(gs)
    {
        clear();
    }

    inline void clear(void)
    {
        Tstore::clear();
        for( auto& sref : zv ) sref = 0;
    }

    inline void fillWhiteStereo( void )
    {
        static_assert(channelCount>1, "no stereo -- channelCount < 2");
        constexpr uint8_t   poleExp  = 11; // depends on sampling freq !
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = zv[0];
            const int32_t x1 = zv[1];
            const int32_t x2 = zv[2];
            const int32_t x3 = zv[3];
            zv[0] = noise.getWhite24();
            zv[2] = noise.getWhite24();
            zv[0] += noise.getWhite24();
            zv[2] += noise.getWhite24();

            zv[1] += x0 - zv[0] - ( zv[1] >> poleExp );
            zv[3] += x2 - zv[2] - ( zv[3] >> poleExp );
            Tstore::channel[0][i] = zv[1] + x1;
            Tstore::channel[1][i] = zv[3] + x3;
        }
    }

    inline void fillWhitePrng( void )
    {
        static_assert(channelCount>1, "no stereo -- channelCount < 2");
        constexpr uint8_t   poleExp  = 11; // depends on sampling freq !
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = zv[0];
            const int32_t x1 = zv[1];
            const int32_t x2 = zv[2];
            const int32_t x3 = zv[3];
            zv[0] = noise.prng();
            zv[2] = noise.prng();
            zv[0] += noise.prng();
            zv[2] += noise.prng();

            zv[1] += x0 - zv[0] - ( zv[1] >> poleExp );
            zv[3] += x2 - zv[2] - ( zv[3] >> poleExp );
            Tstore::channel[0][i] = zv[1] + x1;
            Tstore::channel[1][i] = zv[3] + x3;

        }
    }

    template< uint8_t pulseCountExp > 
    inline void fillVelvet()    
    {
        static_assert( pulseCountExp < sectionSizeExp - 1, "pulse count too high");
        constexpr uint8_t slotSizeExp   = sectionSizeExp - pulseCountExp;
        constexpr uint8_t slotSize      = 1<<slotSizeExp;
        constexpr uint32_t slotSizeMask  = slotSize - 1;
        constexpr uint8_t normExp       = 21;
        Tstore::clear();
        for( auto pi=0u; pi < sectionSize; pi += slotSize ) {
            const int32_t rand0 = noise.getWhiteRaw();
            const int32_t sign0 = ((rand0>>31) | 1)<<normExp; // +1,-1
            const uint32_t position0 = rand0 & slotSizeMask;
            
            if( 2 == channelCount ) {
                Tstore::channel[ 1 ][ pi + position0 ] = Tstore::channel[ 0 ][ pi + position0 ] = sign0;
            } else {
                Tstore::channel[ 0 ][ pi + position0 ] = sign0;                
            }     
        }
    }

    // velvet noise stereo

    inline void fill16()
    {
        for( auto pi=0u; pi < sectionSize; ++pi ) {
            const int32_t x0 = zv[0];
            Tstore::channel[ 0 ][ pi ] = Tstore::channel[ 1 ][ pi ] = (x0 + (noise.getLow()>>12))>>1;
            zv[0] = Tstore::channel[ 0 ][ pi ];
        }
    }

    inline void fillTriangle()
    {
        Tstore::clear();
        const auto x0 = zv[0];
        const auto x1 = zv[1];
//        Tstore::channel[ 0 ][ pi + 0 ] += v;
//        Tstore::channel[ 0 ][ pi + 1 ] += v+v;

        for( auto pi=0u; pi < sectionSize-2; ++pi ) {
            const int32_t v = noise.getWhite24() >> 3;
            Tstore::channel[ 0 ][ pi + 0 ] += v;
            Tstore::channel[ 0 ][ pi + 1 ] += v+v;
            Tstore::channel[ 0 ][ pi + 2 ] += v;
            Tstore::channel[ 1 ][ pi + 0 ] = Tstore::channel[ 0 ][ pi + 0 ];
            Tstore::channel[ 1 ][ pi + 1 ] = Tstore::channel[ 0 ][ pi + 1 ];
            Tstore::channel[ 1 ][ pi + 2 ] = Tstore::channel[ 0 ][ pi + 2 ];
        }
        {
            const int32_t v = noise.getWhite24() >> 3;
            Tstore::channel[ 0 ][ 62 + 0 ] += v;
            Tstore::channel[ 0 ][ 62 + 1 ] += v+v;
            Tstore::channel[ 1 ][ 62 + 0 ] = Tstore::channel[ 0 ][ 62 + 0 ];
            Tstore::channel[ 1 ][ 62 + 1 ] = Tstore::channel[ 0 ][ 62 + 1 ];
            zv[0] = v;
        }
        {
            const int32_t v = noise.getWhite24() >> 3;
            Tstore::channel[ 0 ][ 63 + 0 ] += v;
            Tstore::channel[ 1 ][ 63 + 0 ] = Tstore::channel[ 0 ][ 63 + 0 ];
            zv[1] = v;
        }
    }

    inline void fillVelvet2CH(uint8_t pulseCountExp)
    {
        static_assert( 2 == channelCount, "must be 2 channel");
//        static_assert( pulseCountExp < sectionSizeExp - 1, "pulse count too high");
        const uint8_t slotSizeExp   = sectionSizeExp - pulseCountExp;
        const uint8_t slotSize      = 1<<slotSizeExp;
        const uint32_t slotSizeMask  = slotSize - 1;
        constexpr uint8_t normExp   = 21;
        Tstore::clear();
        for( auto pi=0u; pi < sectionSize; pi += slotSize ) {
            const int32_t rand0 = noise.getWhiteRaw();
            const int32_t rand1 = noise.getWhiteRaw();
            const int32_t sign0 = ((rand0>>31) | 1)<<normExp; // +1,-1
            const int32_t sign1 = ((rand1>>31) | 1)<<normExp; // +1,-1
            const uint32_t position0 = rand0 & slotSizeMask;
            const uint32_t position1 = rand1 & slotSizeMask;
            Tstore::channel[ 0 ][ pi + position0 ] = sign0;
            Tstore::channel[ 1 ][ pi + position1 ] = sign1;
        }
    }
    
    template< uint8_t pulseCountExp > 
    inline void fillVelvetTriangle()    
    {
        static_assert( pulseCountExp < sectionSizeExp - 3, "pulse count too high");        
        constexpr uint8_t slotSizeExp   = sectionSizeExp - pulseCountExp;
        constexpr uint8_t slotSize      = 1<<slotSizeExp;
        constexpr int32_t slotSizeMask  = (slotSize - 1) & 0xFFFCU;
        constexpr uint8_t normExp       = 21;
        Tstore::clear();
        for( auto pi=0u; pi < sectionSize; pi += slotSize ) {
            const int32_t rand0 = noise.getWhiteRaw();
            const int32_t sign0 = ((rand0>>31) | 1)<<normExp; // +1,-1
            const int32_t position0 = rand0 & slotSizeMask;
            
            if( 2 == channelCount ) {
                Tstore::channel[ 1 ][ pi + position0 ] = Tstore::channel[ 0 ][ pi + position0 ] = sign0;
                Tstore::channel[ 1 ][ pi + position0 + 1 ] = Tstore::channel[ 0 ][ pi + position0 + 1 ] = sign0 * 2;
                Tstore::channel[ 1 ][ pi + position0 + 2 ] = Tstore::channel[ 0 ][ pi + position0 + 2 ] = sign0;
            } else {
                Tstore::channel[ 0 ][ pi + position0 ] = sign0;
                Tstore::channel[ 0 ][ pi + position0 + 1 ] = sign0<<1;
                Tstore::channel[ 0 ][ pi + position0 + 2 ] = sign0;                
            }     
        }
    }
    
    inline void fillVelvetTriangle2CH(uint8_t pulseCountExp )
    {
        static_assert( 2 == channelCount, "must be 2 channel");
        static_assert( 11 >= sectionSizeExp, "section size too big (random 24 bit)");
        static_assert( 6 <= sectionSizeExp, "section size too small");
        const uint32_t slotSizeExp   = sectionSizeExp - pulseCountExp;
        const uint32_t slotSize      = 1<<slotSizeExp;
        const uint32_t slotSizeMask  = (slotSize - 1) & 0xFFFDU; // 4 dot/wavelet +0,1 random phase
        constexpr uint8_t normExp    = 21; // 24 bit signed
        Tstore::clear();
        for( auto pi=0u; pi < sectionSize; pi += slotSize ) {
            const int32_t rand0 = noise.getWhiteRaw();
            const int32_t rand1 = noise.getWhiteRaw();
            const int32_t v0 = ((rand0>>31) | 1) << normExp;
            const int32_t v1 = ((rand1>>31) | 1) << normExp;
            const uint32_t position0 = rand0 & slotSizeMask;
            const uint32_t position1 = rand1 & slotSizeMask;
            Tstore::channel[ 0 ][ pi + position0 + 0 ] = v0;
            Tstore::channel[ 0 ][ pi + position0 + 1 ] = v0 * 2;
            Tstore::channel[ 0 ][ pi + position0 + 2 ] = v0;
            
            Tstore::channel[ 1 ][ pi + position1 + 0 ] = v1;
            Tstore::channel[ 1 ][ pi + position1 + 1 ] = v1 * 2;
            Tstore::channel[ 1 ][ pi + position1 + 2 ] = v1;
        }
    }

    inline void fillVelvetDoubleTriangle2CH(uint8_t pulseCountExp )
    {
        static_assert( 2 == channelCount, "must be 2 channel");
        static_assert( 11 >= sectionSizeExp, "section size too big (random 24 bit)");
        static_assert( 6 <= sectionSizeExp, "section size too small");
        const uint16_t slotSizeExp  = sectionSizeExp - pulseCountExp;
        const uint16_t slotSize     = 1<<slotSizeExp;
        const int32_t slotSizeMask  = (slotSize - 1) & 0xFFFDU; // 4 dot/wavelet
        constexpr uint8_t normExp0  = 21; // 24 bit signed
        constexpr uint8_t normExp1  = 19; // 24 bit signed
        Tstore::clear();
        // primary pulse train
        for( auto pi=0u; pi < sectionSize; pi += slotSize ) {
            const int32_t rand0 = noise.getWhiteRaw();
            const int32_t rand1 = noise.getWhiteRaw();
            const int32_t sign0 = ((rand0>>31) | 1)<< normExp0; // +1,-1
            const int32_t sign1 = ((rand1>>31) | 1)<< normExp0; // +1,-1
            const int32_t position0 = rand0 & slotSizeMask;
            const int32_t position1 = rand1 & slotSizeMask;
            Tstore::channel[ 0 ][ pi + position0 + 0 ] = sign0;
            Tstore::channel[ 0 ][ pi + position0 + 1 ] = sign0 * 2;
            Tstore::channel[ 0 ][ pi + position0 + 2 ] = sign0;

            Tstore::channel[ 1 ][ pi + position1 + 0 ] = sign1;
            Tstore::channel[ 1 ][ pi + position1 + 1 ] = sign1 * 2;
            Tstore::channel[ 1 ][ pi + position1 + 2 ] = sign1;
        }
        // secondary pulse train
        for( auto pi=0u; pi < sectionSize; pi += slotSize ) {
            const int32_t rand0 = noise.getWhiteRaw();
            const int32_t rand1 = noise.getWhiteRaw();
            const int32_t sign0 = ((rand0>>31) | 1)<< normExp1; // +1,-1
            const int32_t sign1 = ((rand1>>31) | 1)<< normExp1; // +1,-1
            const int32_t position0 = rand0 & slotSizeMask;
            const int32_t position1 = rand1 & slotSizeMask;
            Tstore::channel[ 0 ][ pi + position0 + 1 ] += sign0;
            Tstore::channel[ 0 ][ pi + position0 + 2 ] += sign0 * 2;
            Tstore::channel[ 0 ][ pi + position0 + 3 ] += sign0;

            Tstore::channel[ 1 ][ pi + position1 + 1 ] += sign1;
            Tstore::channel[ 1 ][ pi + position1 + 2 ] += sign1 * 2;
            Tstore::channel[ 1 ][ pi + position1 + 3 ] += sign1;
        }
    }

    // primary : stereo
    // secondary : mono
    inline void fillVelvetDoubleTriangle2CH_1CH(uint8_t pulseCountExp )
    {
        static_assert( 2 == channelCount, "must be 2 channel");
        static_assert( 11 >= sectionSizeExp, "section size too big (random 24 bit)");
        static_assert( 6 <= sectionSizeExp, "section size too small");
        const uint32_t slotSizeExp  = sectionSizeExp - pulseCountExp;
        const uint32_t slotSize     = 1<<slotSizeExp;
        const uint32_t slotSizeMask = (slotSize - 1) & 0xFFFDU; // 4 dot/wavelet
        constexpr uint8_t normExp0  = 20; // 24 bit signed
        constexpr uint8_t normExp1  = 20; // 24 bit signed
        Tstore::clear();
        // mono
        for( auto pi=0u; pi < sectionSize; pi += slotSize ) {
            const int32_t rand0 = noise.getWhiteRaw();
            const int32_t sign0 = ((rand0>>31) | 1 ) << normExp1; // +1,-1
            const uint32_t position0 = rand0 & slotSizeMask;
            Tstore::channel[ 0 ][ pi + position0 + 0 ] = sign0;
            Tstore::channel[ 0 ][ pi + position0 + 1 ] = sign0 * 2;
            Tstore::channel[ 0 ][ pi + position0 + 2 ] = sign0;

            Tstore::channel[ 1 ][ pi + position0 + 0 ] = sign0;
            Tstore::channel[ 1 ][ pi + position0 + 1 ] = sign0 * 2;
            Tstore::channel[ 1 ][ pi + position0 + 2 ] = sign0;
        }

        for( auto pi=0u; pi < sectionSize; pi += slotSize ) {
            const int32_t rand0 = noise.getWhiteRaw();
            const int32_t rand1 = noise.getWhiteRaw();
            const int32_t sign0 = ((rand0 >> 31) | 1 ) << normExp0; // +1,-1
            const int32_t sign1 = ((rand1 >> 31) | 1 ) << normExp0; // +1,-1
            const uint32_t position0 = rand0 & slotSizeMask;
            const uint32_t position1 = rand1 & slotSizeMask;
            Tstore::channel[ 0 ][ pi + position0 + 0 ] += sign0;
            Tstore::channel[ 0 ][ pi + position0 + 1 ] += sign0 * 2;
            Tstore::channel[ 0 ][ pi + position0 + 2 ] += sign0;

            Tstore::channel[ 1 ][ pi + position1 + 0 ] += sign1;
            Tstore::channel[ 1 ][ pi + position1 + 1 ] += sign1 * 2;
            Tstore::channel[ 1 ][ pi + position1 + 2 ] += sign1;
        }
    }

    // primary : stereo
    // secondary : mono
    // phase smoothed
    // not ok - clicks
    inline void fillVelvetDoubleTriangle2CH_1CHsmooth(uint8_t pulseCountExp )
    {
        static_assert( 2 == channelCount, "must be 2 channel");
        static_assert( 11 >= sectionSizeExp, "section size too big (random 24 bit)");
        static_assert( 6 <= sectionSizeExp, "section size too small");
        const uint32_t slotSizeExp  = sectionSizeExp - pulseCountExp + 1;
        const uint32_t slotSize     = 1<<slotSizeExp;
        const uint32_t slotSizeMask = (slotSize - 1); // & 0xFFFDU; // 4 dot/wavelet
        constexpr uint8_t normExp0  = 20; // 24 bit signed
        constexpr uint8_t normExp1  = 20; // 24 bit signed
        Tstore::clear();
        // stereo
        for( auto pi=0u; pi < sectionSize; pi += slotSize ) {
            const int32_t rand0 = noise.getWhiteRaw();
            const int32_t rand1 = noise.getWhiteRaw();
            const int32_t sign0 = ((rand0>>31) | 1)<< normExp0; // +1,-1
            const int32_t sign1 = ((rand1>>31) | 1)<< normExp0; // +1,-1
            uint32_t position0 = rand0 & slotSizeMask;
            uint32_t position1 = rand1 & slotSizeMask;
            if( pi + position0 > sectionSize-3 ) {
                position0 = 0;
            }
            if( pi + position1 > sectionSize-3 ) {
                position1 = 0;
            }
            Tstore::channel[ 0 ][ pi + position0 + 0 ] += sign0;
            Tstore::channel[ 0 ][ pi + position0 + 1 ] += sign0 * 2;
            Tstore::channel[ 0 ][ pi + position0 + 2 ] += sign0;

            Tstore::channel[ 1 ][ pi + position1 + 0 ] += sign1;
            Tstore::channel[ 1 ][ pi + position1 + 1 ] += sign1 * 2;
            Tstore::channel[ 1 ][ pi + position1 + 2 ] += sign1;
        }
        // mono
        for( auto pi=0u; pi < sectionSize; pi += slotSize ) {
            const int32_t rand0 = noise.getWhiteRaw();
            const int32_t sign0 = ((rand0>>31) | 1) << normExp1; // +1,-1
            uint32_t position0 = rand0 & slotSizeMask;
            if( pi + position0 > sectionSize-3 ) {
                position0 = 0;
            }
            Tstore::channel[ 0 ][ pi + position0 + 0 ] += sign0;
            Tstore::channel[ 0 ][ pi + position0 + 1 ] += sign0 * 2;
            Tstore::channel[ 0 ][ pi + position0 + 2 ] += sign0;

            Tstore::channel[ 1 ][ pi + position0 + 0 ] += sign0;
            Tstore::channel[ 1 ][ pi + position0 + 1 ] += sign0 * 2;
            Tstore::channel[ 1 ][ pi + position0 + 2 ] += sign0;
        }
    }


    inline void fillVelvetTripleTriangle2CH(uint8_t pulseCountExp )
    {
        static_assert( 2 == channelCount, "must be 2 channel");
        static_assert( 11 >= sectionSizeExp, "section size too big (random 24 bit)");
        static_assert( 6 <= sectionSizeExp, "section size too small");
        const uint16_t slotSizeExp  = sectionSizeExp - pulseCountExp;
        const uint16_t slotSize     = 1<<slotSizeExp;
        const int32_t slotSizeMask  = (slotSize - 1) & 0xFFFCU; // 4 dot/wavelet
        constexpr uint8_t normExp0  = 21; // 24 bit signed
        constexpr uint8_t normExp1  = 19; // 24 bit signed
        constexpr uint8_t normExp2  = 17; // 24 bit signed
        Tstore::clear();
        for( auto pi=0u; pi < sectionSize; pi += slotSize ) {
            const int32_t rand0 = noise.getWhiteRaw();
            const int32_t rand1 = noise.getWhiteRaw();
            const int32_t sign0 = ((rand0>>31) | 1)<< normExp0; // +1,-1
            const int32_t sign1 = ((rand1>>31) | 1)<< normExp0; // +1,-1
            const int32_t position0 = rand0 & slotSizeMask;
            const int32_t position1 = rand1 & slotSizeMask;
            Tstore::channel[ 0 ][ pi + position0 + 0 ] = sign0;
            Tstore::channel[ 0 ][ pi + position0 + 1 ] = sign0 * 2;
            Tstore::channel[ 0 ][ pi + position0 + 2 ] = sign0;

            Tstore::channel[ 1 ][ pi + position1 + 0 ] = sign1;
            Tstore::channel[ 1 ][ pi + position1 + 1 ] = sign1 * 2;
            Tstore::channel[ 1 ][ pi + position1 + 2 ] = sign1;
        }
        for( auto pi=0u; pi < sectionSize; pi += slotSize ) {
            const int32_t rand0 = noise.getWhiteRaw();
            const int32_t rand1 = noise.getWhiteRaw();
            const int32_t sign0 = ((rand0>>31) | 1)<< normExp1; // +1,-1
            const int32_t sign1 = ((rand1>>31) | 1)<< normExp1; // +1,-1
            const int32_t position0 = rand0 & slotSizeMask;
            const int32_t position1 = rand1 & slotSizeMask;
            Tstore::channel[ 0 ][ pi + position0 + 1 ] += sign0;
            Tstore::channel[ 0 ][ pi + position0 + 2 ] += sign0 * 2;
            Tstore::channel[ 0 ][ pi + position0 + 3 ] += sign0;

            Tstore::channel[ 1 ][ pi + position1 + 1 ] += sign1;
            Tstore::channel[ 1 ][ pi + position1 + 2 ] += sign1 * 2;
            Tstore::channel[ 1 ][ pi + position1 + 3 ] += sign1;
        }
        for( auto pi=0u; pi < sectionSize; pi += slotSize ) {
            const int32_t rand0 = noise.getWhiteRaw();
            const int32_t sign0 = ((rand0>>31) | 1)<< normExp1; // +1,-1
            const int32_t position0 = rand0 & slotSizeMask;
            Tstore::channel[ 0 ][ pi + position0 + 1 ] += sign0;
            Tstore::channel[ 0 ][ pi + position0 + 2 ] += sign0 * 2;
            Tstore::channel[ 0 ][ pi + position0 + 3 ] += sign0;

            Tstore::channel[ 1 ][ pi + position0 + 1 ] += sign0;
            Tstore::channel[ 1 ][ pi + position0 + 2 ] += sign0 * 2;
            Tstore::channel[ 1 ][ pi + position0 + 3 ] += sign0;
        }
    }

    // run after fill.... 
    // low cut: diff (zero) + 1 pole under 20 Hz @ 192kHz
    inline void postFilterDCcut( uint8_t upsampleRate, uint8_t stateOffs )
    {        
        const uint8_t poleExp = 10 + upsampleRate; // 9..
        const int32_t round = 1 << (poleExp-1);
        for( auto pi=0u; pi < sectionSize; ++pi ) {            
            const int32_t x0 = zv[ stateOffs+0 ];
            //const int32_t x1 = zv[ stateOffs+1 ];
            zv[ stateOffs+0 ] = Tstore::channel[ 0 ][ pi ];
            zv[ stateOffs+1 ] += x0 - zv[ stateOffs+0 ] - ((zv[ stateOffs+1 ] + round )>>poleExp);
            Tstore::channel[ 0 ][ pi ] = zv[ stateOffs+1 ];
            
            const int32_t x2 = zv[ stateOffs+2 ];
            //const int32_t x3 = zv[ stateOffs+3 ];
            zv[ stateOffs+2 ] = Tstore::channel[ 1 ][ pi ];
            zv[ stateOffs+3 ] += x2 - zv[ stateOffs+2 ] - ((zv[ stateOffs+3 ] + round )>>poleExp);
            Tstore::channel[ 1 ][ pi ] = zv[ stateOffs+3 ];
        }
    }

    inline void postFilterLowPass1( uint8_t upsampleRate, uint8_t stateOffs )
    {
        const uint8_t poleExp = 7 + upsampleRate;
        const uint8_t gainExp = 5;
        const int32_t round = 1 << (poleExp-1);
        for( auto pi=0u; pi < sectionSize; ++pi ) {
            zv[ stateOffs+0 ] += (Tstore::channel[ 0 ][ pi ]>>gainExp) - ((zv[ stateOffs+0 ] + round )>>poleExp);
            Tstore::channel[ 0 ][ pi ] = zv[ stateOffs+0 ];
            zv[ stateOffs+1 ] += (Tstore::channel[ 1 ][ pi ]>>gainExp) - ((zv[ stateOffs+1 ] + round )>>poleExp);
            Tstore::channel[ 1 ][ pi ] = zv[ stateOffs+1 ];
        }
    }

    inline void postFilterLowPass2( uint8_t upsampleRate, uint8_t stateOffs )
    {
        const uint8_t poleExp = 6 + upsampleRate;
        const uint8_t gainExp0 = 5;
        const uint8_t gainExp1 = 4 + upsampleRate;
        const int32_t round = 1 << (poleExp-1);
        for( auto pi=0u; pi < sectionSize; ++pi ) {
            zv[ stateOffs+0 ] += (Tstore::channel[ 0 ][ pi ]>>gainExp0) - ((zv[ stateOffs+0 ] + round )>>poleExp);
            zv[ stateOffs+1 ] += (zv[ stateOffs+0 ]>>gainExp1) - (zv[ stateOffs+1 ]>>poleExp);
            Tstore::channel[ 0 ][ pi ] = zv[ stateOffs+1 ];

            zv[ stateOffs+2 ] += (Tstore::channel[ 1 ][ pi ]>>gainExp0) - ((zv[ stateOffs+2 ] + round )>>poleExp);
            zv[ stateOffs+3 ] += (zv[ stateOffs+2 ]>>gainExp1) - (zv[ stateOffs+3 ]>>poleExp);
            Tstore::channel[ 1 ][ pi ] = zv[ stateOffs+3 ];
        }
    }

    inline void fillVelvetRed2CH(uint8_t pulseCountExp )
    {
        static_assert( 2 == channelCount, "must be 2 channel");
        static_assert( 11 >= sectionSizeExp, "section size too big (random 24 bit)");
        static_assert( 6 <= sectionSizeExp, "section size too small");
        lowPass1PoleVelvet<4,false>(0,0,16,9);
        lowPass1PoleVelvet<4,false>(1,1,16,9);
    }

    inline void fillVelvetBandSV2CH(uint8_t pulseCountExp, uint32_t f, uint8_t qExp )
    {
        static_assert( 2 == channelCount, "must be 2 channel");
        static_assert( 11 >= sectionSizeExp, "section size too big (random 24 bit)");
        static_assert( 6 <= sectionSizeExp, "section size too small");
        bandPassSVVelvet<4,false>(0,0,21,f,qExp);
        bandPassSVVelvet<4,false>(1,2,21,f,qExp);
    }

    inline void fillVelvetBand4pole2CH(uint8_t pulseCountExp, uint32_t f, uint16_t b )
    {
        static_assert( 2 == channelCount, "must be 2 channel");
        static_assert( 11 >= sectionSizeExp, "section size too big (random 24 bit)");
        static_assert( 6 <= sectionSizeExp, "section size too small");
        bandPass4PoleVelvet<4,false>(0,0,24,f,b);
        bandPass4PoleVelvet<4,false>(1,4,24,f,b);
    }

    inline void fillPink(void)
    {
        constexpr int g = 5;
        constexpr int p = 9;
        int32_t x0 = zv[0] + zv[1] + zv[2] + zv[3] + zv[4];
        for( auto i=0u; i<sectionSize; ++i ) {
            // 14.9358 Hz
            zv[0] += ( noise.getWhiteRaw() >> ( p - 0 + 0 + g ) ) - ( zv[0]>>(p-0) );
            // 59.9192 Hz
            zv[1] += ( noise.getWhiteRaw() >> ( p - 2 + 1 + g ) ) - ( zv[1]>>(p-2) );
            // 242.549 Hz
            zv[2] += ( noise.getWhiteRaw() >> ( p - 4 + 2 + g ) ) - ( zv[2]>>(p-4) );
            // 1020.13 Hz
            zv[3] += ( noise.getWhiteRaw() >> ( p - 6 + 3 + g ) ) - ( zv[3]>>(p-6) );
            // 5295.41 Hz
            zv[4] += ( noise.getWhiteRaw() >> ( p - 8 + 4 + g ) ) - ( zv[4]>>(p-8) );
            const int32_t x1 = zv[0] + zv[1] + zv[2] + zv[3] + zv[4];
            const int32_t x2 = zv[5];
            // dc cut
            zv[5] += ( x0 - x1 ) - ( zv[5]>>(p+2) );
            // zero gain at fs/2
            Tstore::channel[0][i] = zv[5] + x2;
            x0 = x1;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
        }
    };

    inline void fillVelvetPink2CH(uint8_t pulseCountExp )
    {
        static_assert( 2 == channelCount, "must be 2 channel");
        static_assert( 11 >= sectionSizeExp, "section size too big (random 24 bit)");
        static_assert( 6 <= sectionSizeExp, "section size too small");
        constexpr int poleExpBase = 9;
        constexpr int amplExpBase = 14;
        // ch0
        lowPass1PoleVelvet<4,false>(0,0,amplExpBase+0,poleExpBase-0);
        lowPass1PoleVelvet<4,true>(0,2,amplExpBase+1,poleExpBase-2);
        lowPass1PoleVelvet<4,true>(0,4,amplExpBase+2,poleExpBase-4);
        lowPass1PoleVelvet<4,true>(0,6,amplExpBase+3,poleExpBase-6);
        lowPass1PoleVelvet<4,true>(0,8,amplExpBase+4,poleExpBase-8);
        // ch1
        lowPass1PoleVelvet<4,false>(1,1,amplExpBase+0,poleExpBase-0);
        lowPass1PoleVelvet<4,true>(1,3,amplExpBase+1,poleExpBase-2);
        lowPass1PoleVelvet<4,true>(1,5,amplExpBase+2,poleExpBase-4);
        lowPass1PoleVelvet<4,true>(1,7,amplExpBase+3,poleExpBase-6);
        lowPass1PoleVelvet<4,true>(1,9,amplExpBase+4,poleExpBase-8);
    }

private:
    // 4 pole band pass
    // 4 state is needed
    // b : feedback 8192 == 1; max 4 --> 32767 (32000 is ok above ampl must be reduced)
    inline int32_t bandPass4P( uint8_t state, uint32_t f, uint16_t b, int32_t v )
    {
        constexpr uint8_t scaleExpF     = 32;
        constexpr uint8_t scaleExpB     = 13; // 8192 --> 1.0
        const int32_t t = v - (( int64_t(zv[ state+3 ]) * b ) >> scaleExpB );
        zv[ state+0 ]   = ((int64_t( zv[ state+0 ] - t ) * f ) >> scaleExpF ) + t;
        zv[ state+1 ]   = ((int64_t( zv[ state+1 ] - zv[ state+0 ] ) * f ) >> scaleExpF ) + zv[ state+0 ];
        zv[ state+2 ]   = ((int64_t( zv[ state+2 ] - zv[ state+1 ] ) * f ) >> scaleExpF ) + zv[ state+1 ];
        const int32_t y = ((int64_t( zv[ state+3 ] - zv[ state+2 ] ) * f ) >> scaleExpF );
        zv[ state+3 ]   = y + zv[ state+2 ];
        return y;
    }

    template<uint8_t slotSizeExp, bool add >
    inline void bandPass4PoleVelvet( uint8_t ch, uint8_t state, uint8_t normExp, uint32_t f, uint16_t b )
    {
        constexpr uint16_t slotSize     = 1<<slotSizeExp;
        constexpr int32_t slotSizeMask  = (slotSize - 1) & 0xFFFFFFFDU;
        uint16_t offs = 0;
        for( uint16_t pi = 0u; pi < sectionSize; )
        {
            const int32_t rand = noise.getWhiteRaw();
            const int32_t v = (((rand>>31) | 1) << normExp); // calc
            uint32_t position = offs + (rand & slotSizeMask);
            if( add ) {
                // add
                for( ; pi < position; ++pi ) {
                    Tstore::channel[ ch ][ pi ] += bandPass4P( state, f, b, 0); // value 0
                }
                {
                    Tstore::channel[ ch ][ pi++ ] += bandPass4P( state, f, b, v);
                    Tstore::channel[ ch ][ pi++ ] += bandPass4P( state, f, b, v+v);
                    Tstore::channel[ ch ][ pi++ ] += bandPass4P( state, f, b, v);
                }
                for( ; pi < offs + slotSize; ++pi ) {
                    Tstore::channel[ ch ][ pi ] += bandPass4P( state, f, b, 0); // value 0
                }
            } else{
                // set
                for( ; pi < position; ++pi ) {
                    Tstore::channel[ ch ][ pi ] = bandPass4P( state, f, b, 0); // value 0
                }
                {
                    Tstore::channel[ ch ][ pi++ ] = bandPass4P( state, f, b, v);
                    Tstore::channel[ ch ][ pi++ ] = bandPass4P( state, f, b, v+v);
                    Tstore::channel[ ch ][ pi++ ] = bandPass4P( state, f, b, v);
                }
                for( ; pi < offs + slotSize; ++pi ) {
                    Tstore::channel[ ch ][ pi ] = bandPass4P( state, f, b, 0); // value 0
                }
            }
            offs += slotSize;
        }
    }

    // 1 pole low pass
    template<uint8_t slotSizeExp, bool add >
    inline void lowPass1PoleVelvet( uint8_t ch, uint8_t state, uint8_t normExp, uint8_t poleExp )
    {
        constexpr uint16_t slotSize     = 1<<slotSizeExp;
        constexpr int32_t slotSizeMask  = (slotSize - 1) & 0xFFFFFFFDU;
        uint16_t offs = 0;
        for( uint16_t pi = 0u; pi < sectionSize; )
        {
            const int32_t rand = noise.getWhiteRaw();
            const int32_t v = (((rand>>31) | 1) << normExp); // calc
            uint32_t position = offs + (rand & slotSizeMask);
            if( add ) {
                // add
                for( ; pi < position; ++pi ) {
                    zv[ state ] -= ((zv[ state ])>>poleExp); // value 0
                    Tstore::channel[ ch ][ pi ] += zv[ state ];
                }

                zv[ state ] += v - ((zv[ state ])>>poleExp);
                Tstore::channel[ ch ][ pi++ ] += zv[ state ];
                zv[ state ] += v*2 - ((zv[ state ])>>poleExp);
                Tstore::channel[ ch ][ pi++ ] += zv[ state ];
                zv[ state ] += v - ((zv[ state ])>>poleExp);
                Tstore::channel[ ch ][ pi++ ] += zv[ state ];

                for( ; pi < offs + slotSize; ++pi ) {
                    zv[ state ] -= ((zv[ state ])>>poleExp); // value 0
                    Tstore::channel[ ch ][ pi ] += zv[ state ];
                }
            } else{
                // set
                for( ; pi < position; ++pi ) {
                    zv[ state ] -= ((zv[ state ])>>poleExp); // value 0
                    Tstore::channel[ ch ][ pi ] = zv[ state ];
                }

                zv[ state ] += v - ((zv[ state ])>>poleExp);
                Tstore::channel[ ch ][ pi++ ] = zv[ state ];
                zv[ state ] += v*2 - ((zv[ state ])>>poleExp);
                Tstore::channel[ ch ][ pi++ ] = zv[ state ];
                zv[ state ] += v - ((zv[ state ])>>poleExp);
                Tstore::channel[ ch ][ pi++ ] = zv[ state ];

                for( ; pi < offs + slotSize; ++pi ) {
                    zv[ state ] -= ((zv[ state ])>>poleExp); // value 0
                    Tstore::channel[ ch ][ pi ] = zv[ state ];
                }
            }
            offs += slotSize;
        }
    }

    // state variable: band pass
    inline void bandPassSV( uint8_t state, int32_t f, uint8_t qExp, int32_t v )
    {
        zv[ state+0 ] += ( int64_t(zv[ state+1 ]) * f ) >> 32;
        const int64_t t = v - zv[ state+0 ] - ( zv[ state+1 ] >> qExp );
        zv[ state+1 ] += ( t * f ) >> 32;
    }

    template<uint8_t slotSizeExp, bool add >
    inline void bandPassSVVelvet( uint8_t ch, uint8_t state, uint8_t normExp, uint32_t f, uint8_t qExp )
    {
        constexpr uint16_t slotSize     = 1<<slotSizeExp;
        constexpr int32_t slotSizeMask  = (slotSize - 1) & 0xFFFFFFFDU;
        uint16_t offs = 0;
        for( uint16_t pi = 0u; pi < sectionSize; )
        {
            const int32_t rand = noise.getWhiteRaw();
            const int32_t v = (((rand>>31) | 1) << normExp);
            uint32_t position = offs + (rand & slotSizeMask);
            if( add ) {
                // add
                for( ; pi < position; ++pi ) {                    
                    bandPassSV( state, f, qExp, 0 ); // value 0
                    Tstore::channel[ ch ][ pi ] += zv[ state+1 ];
                }
                {
                    bandPassSV( state, f, qExp, v );
                    Tstore::channel[ ch ][ pi++ ] += zv[ state+1 ];
                    bandPassSV( state, f, qExp, v+v );
                    Tstore::channel[ ch ][ pi++ ] += zv[ state+1 ];
                    bandPassSV( state, f, qExp, v );
                    Tstore::channel[ ch ][ pi++ ] += zv[ state+1 ];
                }
                for( ; pi < offs + slotSize; ++pi ) {
                    bandPassSV( state, f, qExp, 0 ); // value 0
                    Tstore::channel[ ch ][ pi ] += zv[ state+1 ];
                }
            } else{
                // set
                for( ; pi < position; ++pi ) {                    
                    bandPassSV( state, f, qExp, 0 ); // value 0
                    Tstore::channel[ ch ][ pi ] = zv[ state+1 ];
                }
                {
                    bandPassSV( state, f, qExp, v );
                    Tstore::channel[ ch ][ pi++ ] = zv[ state+1 ];
                    bandPassSV( state, f, qExp, v+v );
                    Tstore::channel[ ch ][ pi++ ] = zv[ state+1 ];
                    bandPassSV( state, f, qExp, v );
                    Tstore::channel[ ch ][ pi++ ] = zv[ state+1 ];
                }
                for( ; pi < offs + slotSize; ++pi ) {
                    // value 0
                    bandPassSV( state, f, qExp, 0 );
                    Tstore::channel[ ch ][ pi ] = zv[ state+1 ];
                }
            }
            offs += slotSize;
        }
    }

    int32_t zv[sSize];
    GaloisShifter&  noise;
};


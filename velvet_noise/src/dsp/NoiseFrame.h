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
        for( uint32_t i=0; i<sectionSize; ++i ) {
            for( int c=0; c<channelCount; ++c) {
                channelX[i][c] = static_cast<T>(in.channel[c][i]*norm);                
            }
        }
    }

    inline void add( FrameInt<sectionSizeExp,channelCount> const & in, float norm )
    {
        for( uint32_t i=0; i<sectionSize; ++i ) {
            for( int c=0; c<channelCount; ++c) {
                channelX[i][c] += static_cast<T>(in.channel[c][i]*norm);
            }
        }
    }

    inline void set( FrameInt<sectionSizeExp,channelCount> const & in )
    {
        for( uint32_t i=0; i<sectionSize; ++i ) {
            for( int c=0; c<channelCount; ++c) {
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
        for( auto& sref : s ) sref = 0;
    }

    inline void fillWhiteStereo( void )
    {
        static_assert(channelCount>1, "no stereo -- channelCount < 2");
        constexpr uint8_t   poleExp  = 11; // depends on sampling freq !
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = s[0];
            const int32_t x1 = s[1];
            const int32_t x2 = s[2];
            const int32_t x3 = s[3];
            s[0] = noise.getWhite24();
            s[2] = noise.getWhite24();
            s[0] += noise.getWhite24();
            s[2] += noise.getWhite24();

            s[1] += x0 - s[0] - ( s[1] >> poleExp );
            s[3] += x2 - s[2] - ( s[3] >> poleExp );
            Tstore::channel[0][i] = s[1] + x1;
            Tstore::channel[1][i] = s[3] + x3;
        }
    };

    template< uint8_t pulseCountExp > 
    inline void fillVelvet()    
    {
        static_assert( pulseCountExp < sectionSizeExp - 1, "pulse count too high");
        constexpr uint8_t slotSizeExp   = sectionSizeExp - pulseCountExp;
        constexpr uint8_t slotSize      = 1<<slotSizeExp;
        constexpr int32_t slotSizeMask  = slotSize - 1;
        constexpr uint8_t normExp       = 21;
        Tstore::clear();
        for( auto pi=0u; pi < sectionSize; pi += slotSize ) {
            const int32_t rand0 = noise.getWhiteRaw();
            const int32_t sign0 = ((rand0>>31) | 1)<<normExp; // +1,-1
            const int32_t position0 = rand0 & slotSizeMask;
            
            if( 2 == channelCount ) {
                Tstore::channel[ 1 ][ pi + position0 ] = Tstore::channel[ 0 ][ pi + position0 ] = sign0;
            } else {
                Tstore::channel[ 0 ][ pi + position0 ] = sign0;                
            }     
        }
    }

    // velvet noise stereo

    inline void fillVelvet2CH(uint8_t pulseCountExp)
    {
        static_assert( 2 == channelCount, "must be 2 channel");
//        static_assert( pulseCountExp < sectionSizeExp - 1, "pulse count too high");
        const uint8_t slotSizeExp   = sectionSizeExp - pulseCountExp;
        const uint8_t slotSize      = 1<<slotSizeExp;
        const int32_t slotSizeMask  = slotSize - 1;
        constexpr uint8_t normExp   = 21;
        Tstore::clear();
        for( auto pi=0u; pi < sectionSize; pi += slotSize ) {
            const int32_t rand0 = noise.getWhiteRaw();
            const int32_t rand1 = noise.getWhiteRaw();
            const int32_t sign0 = ((rand0>>31) | 1)<<normExp; // +1,-1
            const int32_t sign1 = ((rand1>>31) | 1)<<normExp; // +1,-1
            const int32_t position0 = rand0 & slotSizeMask;
            const int32_t position1 = rand1 & slotSizeMask;            
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
//        static_assert( pulseCountExp < sectionSizeExp - 3, "pulse count too high");
        const uint16_t slotSizeExp   = sectionSizeExp - pulseCountExp;
        const uint16_t slotSize      = 1<<slotSizeExp;
//        static_assert( 2 <= slotSize, "slotSize too small");
//        static_assert( sectionSize / 2 >= slotSize, "slotSize too big");
        const int32_t slotSizeMask  = (slotSize - 1) & 0xFFFCU; // 4 dot/wavelet
        constexpr uint8_t normExp       = 21; // 24 bit signed
        Tstore::clear();
        for( auto pi=0u; pi < sectionSize; pi += slotSize ) {
            const int32_t rand0 = noise.getWhiteRaw();
            const int32_t rand1 = noise.getWhiteRaw();
            const int32_t sign0 = (rand0>>31) | 1; // +1,-1
            const int32_t sign1 = (rand1>>31) | 1; // +1,-1           
            const int32_t position0 = rand0 & slotSizeMask;
            const int32_t position1 = rand1 & slotSizeMask;            
            Tstore::channel[ 0 ][ pi + position0 ] = sign0 << normExp;
            Tstore::channel[ 0 ][ pi + position0 + 1 ] = Tstore::channel[ 0 ][ pi + position0 ] * 2;
            Tstore::channel[ 0 ][ pi + position0 + 2 ] = Tstore::channel[ 0 ][ pi + position0 ];
            
            Tstore::channel[ 1 ][ pi + position1 ] = sign1 << normExp;
            Tstore::channel[ 1 ][ pi + position1 + 1 ] = Tstore::channel[ 1 ][ pi + position1 ] * 2;
            Tstore::channel[ 1 ][ pi + position1 + 2 ] = Tstore::channel[ 1 ][ pi + position1 ];
        }
    }
    
    inline void fillVelvetDoubleTriangle2CH(uint8_t pulseCountExp )
    {
        static_assert( 2 == channelCount, "must be 2 channel");
        static_assert( 11 >= sectionSizeExp, "section size too big (random 24 bit)");
        static_assert( 6 <= sectionSizeExp, "section size too small");
        const uint16_t slotSizeExp  = sectionSizeExp - pulseCountExp;
        const uint16_t slotSize     = 1<<slotSizeExp;
        const int32_t slotSizeMask  = (slotSize - 1) & 0xFFFCU; // 4 dot/wavelet
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
        const uint16_t slotSizeExp  = sectionSizeExp - pulseCountExp;
        const uint16_t slotSize     = 1<<slotSizeExp;
        const int32_t slotSizeMask  = (slotSize - 1) & 0xFFFCU; // 4 dot/wavelet
        constexpr uint8_t normExp0  = 21; // 24 bit signed
        constexpr uint8_t normExp1  = 20; // 24 bit signed
        Tstore::clear();
        // stereo
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
        // mono
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



    // 16 dot/ wave
    // 0 : 0
    // 1 : 1/2
    // 2 : 1
    // 3 : 1 - k*1  k=1/8
    // 4 : 0
    // 5 : 0
    // 6 : 0
    // 7 : 0
    // 8 : 0
    // 9 : 1/2
    // A : 1/4
    // B : 1/8
    // C : 1/16
    // D : 1/32
    // F : 0

    template< uint8_t pulseCountExp >
    inline void fillVelvetDecay2CH()
    {
        constexpr uint8_t normExp       = 20;
        static_assert( 2 == channelCount, "must be 2 channel");
        static_assert( 11 >= sectionSizeExp, "section size too big (random 24 bit)");
        static_assert( 6 <= sectionSizeExp, "section size too small");
        static_assert( pulseCountExp < sectionSizeExp - 3, "pulse count too high");
        constexpr uint16_t slotSizeExp   = sectionSizeExp - pulseCountExp;
        constexpr uint16_t slotSize      = 1<<slotSizeExp;
        static_assert( 2 <= slotSize, "slotSize too small");
        static_assert( sectionSize / 2 >= slotSize, "slotSize too big");
        constexpr int32_t slotSizeMask  = (slotSize - 1) & 0xFFFEU; // 8 dot/wave
        Tstore::clear();
        for( auto pi=0u; pi < sectionSize; pi += slotSize ) {
            const int32_t rand0 = noise.getWhiteRaw();
            const int32_t rand1 = noise.getWhiteRaw();
            const int32_t sign0 = ((rand0>>31) | 1)<< normExp; // +1,-1
            const int32_t sign1 = ((rand1>>31) | 1)<< normExp; // +1,-1
            const int32_t position0 = rand0 & slotSizeMask;
            const int32_t position1 = rand1 & slotSizeMask;
            Tstore::channel[ 0 ][ pi + position0 + 1 ] = sign0;
            Tstore::channel[ 0 ][ pi + position0 + 2 ] = sign0 * 2;
            Tstore::channel[ 0 ][ pi + position0 + 3 ] = Tstore::channel[ 0 ][ pi + position0 + 2 ] + sign0;
            Tstore::channel[ 0 ][ pi + position0 + 4 ] = Tstore::channel[ 0 ][ pi + position0 + 2 ];
            Tstore::channel[ 0 ][ pi + position0 + 5 ] = sign0;


            Tstore::channel[ 1 ][ pi + position1 + 1 ] = sign1;
            Tstore::channel[ 1 ][ pi + position1 + 2 ] = sign1 * 2;
            Tstore::channel[ 1 ][ pi + position1 + 3 ] = Tstore::channel[ 1 ][ pi + position1 + 2 ] + sign1;
            Tstore::channel[ 1 ][ pi + position1 + 4 ] = Tstore::channel[ 1 ][ pi + position1 + 2 ];
            Tstore::channel[ 1 ][ pi + position1 + 5 ] = sign1;

//            Tstore::channel[ 0 ][ pi + position0 + 1 ] = sign0;
//            Tstore::channel[ 0 ][ pi + position0 + 2 ] = sign0 * 2;
//            Tstore::channel[ 0 ][ pi + position0 + 3 ] = sign0;
//            Tstore::channel[ 0 ][ pi + position0 + 4 ] = Tstore::channel[ 0 ][ pi + position0 + 3 ] >> 1;
//            Tstore::channel[ 0 ][ pi + position0 + 5 ] = Tstore::channel[ 0 ][ pi + position0 + 4 ] >> 1;
//            Tstore::channel[ 0 ][ pi + position0 + 6 ] = Tstore::channel[ 0 ][ pi + position0 + 5 ] >> 1;


//            Tstore::channel[ 1 ][ pi + position1 + 1 ] = sign1;
//            Tstore::channel[ 1 ][ pi + position1 + 2 ] = sign1 * 2;
//            Tstore::channel[ 1 ][ pi + position1 + 3 ] = sign1;
//            Tstore::channel[ 1 ][ pi + position1 + 4 ] = Tstore::channel[ 1 ][ pi + position1 + 3 ] >> 1;
//            Tstore::channel[ 1 ][ pi + position1 + 5 ] = Tstore::channel[ 1 ][ pi + position1 + 4 ] >> 1;
//            Tstore::channel[ 1 ][ pi + position1 + 6 ] = Tstore::channel[ 1 ][ pi + position1 + 5 ] >> 1;
        }
    }



    // run after fill.... 
    // low cut: diff (zero) + 1 pole under 20 Hz @ 192kHz
    inline void postFilterDCcut( uint8_t upsampleRate, uint8_t stateOffs )
    {        
        const uint8_t poleExp = 8 + upsampleRate; // 9..
        const int32_t round = 1 << (poleExp-1);
        for( auto pi=0u; pi < sectionSize; ++pi ) {            
            const int32_t x0 = s[ stateOffs+0 ];
            const int32_t x1 = s[ stateOffs+1 ];
            s[ stateOffs+0 ] = Tstore::channel[ 0 ][ pi ];
            s[ stateOffs+1 ] += x0 - s[ stateOffs+0 ] - ((s[ stateOffs+1 ] + round )>>poleExp);
            Tstore::channel[ 0 ][ pi ] = s[ stateOffs+1 ];
            
            const int32_t x2 = s[ stateOffs+2 ];
            const int32_t x3 = s[ stateOffs+3 ];
            s[ stateOffs+2 ] = Tstore::channel[ 1 ][ pi ];
            s[ stateOffs+3 ] += x2 - s[ stateOffs+2 ] - ((s[ stateOffs+3 ] + round )>>poleExp);
            Tstore::channel[ 1 ][ pi ] = s[ stateOffs+3 ];
        }
    }

    inline void postFilterLowPass1( uint8_t upsampleRate, uint8_t stateOffs )
    {
        const uint8_t poleExp = 7 + upsampleRate; // 9..
        const uint8_t gainExp = 1 + upsampleRate; // 9..
        const int32_t round = 1 << (poleExp-1);
        for( auto pi=0u; pi < sectionSize; ++pi ) {
            s[ stateOffs+0 ] += (Tstore::channel[ 0 ][ pi ]>>gainExp) - (s[ stateOffs+0 ]>>poleExp);
            Tstore::channel[ 0 ][ pi ] = s[ stateOffs+0 ];
            s[ stateOffs+1 ] += (Tstore::channel[ 1 ][ pi ]>>gainExp) - (s[ stateOffs+1 ]>>poleExp);
            Tstore::channel[ 1 ][ pi ] = s[ stateOffs+1 ];
        }
    }

    inline void postFilterLowPass2( uint8_t upsampleRate, uint8_t stateOffs )
    {
        const uint8_t poleExp = 6 + upsampleRate; // 9..
        const uint8_t gainExp = 3 + upsampleRate; // 9..
        const int32_t round = 1 << (poleExp-1);
        for( auto pi=0u; pi < sectionSize; ++pi ) {
            s[ stateOffs+0 ] += (Tstore::channel[ 0 ][ pi ]>>gainExp) - (s[ stateOffs+0 ]>>poleExp);
            s[ stateOffs+1 ] += (s[ stateOffs+0 ]>>gainExp) - (s[ stateOffs+1 ]>>poleExp);
            Tstore::channel[ 0 ][ pi ] = s[ stateOffs+1 ];

            s[ stateOffs+2 ] += (Tstore::channel[ 1 ][ pi ]>>gainExp) - (s[ stateOffs+2 ]>>poleExp);
            s[ stateOffs+3 ] += (s[ stateOffs+2 ]>>gainExp) - (s[ stateOffs+3 ]>>poleExp);
            Tstore::channel[ 1 ][ pi ] = s[ stateOffs+3 ];
        }
    }

    int32_t s[sSize];
//    int32_t decay[2][8] = {
//        0,
//        1 << normExp,
//        2 << normExp,
//        (1 << normExp) ,
//    };

private:
    GaloisShifter&  noise;
};


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
    static constexpr std::size_t vsectionSizeExp    = fexp-2;
    static constexpr std::size_t vsectionSize       = 1<<vsectionSizeExp;
    static constexpr std::size_t vsectionSizeMask   = vsectionSize-1;
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
    static constexpr std::size_t vsectionSizeExp    = fexp-2;
    static constexpr std::size_t vsectionSize       = 1<<vsectionSizeExp;
    static constexpr std::size_t vsectionSizeMask   = vsectionSize-1;
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
    {};

    inline void clear(void)
    {
        Tstore::clear();
        for( auto& sref : s ) sref = 0;
    }

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
    template< uint8_t pulseCountExp > 
    inline void fillVelvet2CH()    
    {
        static_assert( 2 == channelCount, "must be 2 channel");
        static_assert( pulseCountExp < sectionSizeExp - 1, "pulse count too high");
        constexpr uint8_t slotSizeExp   = sectionSizeExp - pulseCountExp;
        constexpr uint8_t slotSize      = 1<<slotSizeExp;
        constexpr int32_t slotSizeMask  = slotSize - 1;
        constexpr uint8_t normExp       = 21;
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
    
    template< uint8_t pulseCountExp > 
    inline void fillVelvetTriangle2CH()    
    {
        static_assert( 2 == channelCount, "must be 2 channel");
        static_assert( 11 >= sectionSizeExp, "section size too big (random 24 bit)");
        static_assert( 6 <= sectionSizeExp, "section size too small");
        static_assert( pulseCountExp < sectionSizeExp - 3, "pulse count too high");        
        constexpr uint16_t slotSizeExp   = sectionSizeExp - pulseCountExp;
        constexpr uint16_t slotSize      = 1<<slotSizeExp;
        static_assert( 2 <= slotSize, "slotSize too small");
        static_assert( sectionSize / 2 >= slotSize, "slotSize too big");        
        constexpr int32_t slotSizeMask  = (slotSize - 1) & 0xFFFCU;
        constexpr uint8_t normExp       = 21;
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
#if 0
           
            s[0] += sign0;
            s[1] += sign1;
#endif            
        }
    }
    
    // run after fill.... 
    // low cut: diff (zero) + 1 pole under 20 Hz @ 192kHz
    inline void postFilterDCcut( uint8_t poleExp=12 )    
    {        
        constexpr uint8_t   soffs    = 12; // state offset index - use high
        for( auto pi=0u; pi < sectionSize; ++pi ) {            
            const int32_t x0 = s[soffs+0];
            const int32_t x1 = s[soffs+1];
            s[soffs+0] = Tstore::channel[ 0 ][ pi ];
            s[soffs+1] += x0 - s[soffs+0] - (s[soffs+1]>>poleExp);
            Tstore::channel[ 0 ][ pi ] = s[soffs+1];
            
            const int32_t x2 = s[soffs+2];
            const int32_t x3 = s[soffs+3];            
            s[soffs+2] = Tstore::channel[ 1 ][ pi ];
            s[soffs+3] += x2 - s[soffs+2] - (s[soffs+3]>>poleExp);
            Tstore::channel[ 1 ][ pi ] = s[soffs+3];
        }
    }
    
    // TBD
    inline void postFilterLowPass( uint8_t poleExp=12 )    
    {        
        constexpr uint8_t   soffs    = 10; // state offset index - use high
        for( auto pi=0u; pi < sectionSize; ++pi ) {            
//            const int32_t x0 = s[soffs+0];
//            s[soffs+0] = Tstore::channel[ 0 ][ pi ];
//            s[soffs+1] += x0 - s[soffs+0] - (s[soffs+1]>>poleExp);
//            Tstore::channel[ 0 ][ pi ] = s[soffs+1];
//            
//            const int32_t x2 = s[soffs+2];
//            const int32_t x3 = s[soffs+3];            
//            s[soffs+2] = Tstore::channel[ 1 ][ pi ];
//            s[soffs+3] += x2 - s[soffs+2] - (s[soffs+3]>>poleExp);
//            Tstore::channel[ 1 ][ pi ] = s[soffs+3];
        }
    }

    inline const auto& getFrame(void) const
    {
        return Tstore::channel[0];
    }
    inline const auto& getVFrame(void) const
    {
        return Tstore::vchannel[0];
    }

    int32_t s[sSize];

private:
    GaloisShifter&  noise;
};


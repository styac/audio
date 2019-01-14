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

#include "utils/GaloisNoiser.h"
#include <cstring>

// http://mc2method.org/white-noise/

using namespace noiser;

namespace yacynth {

template< std::size_t fexp, std::size_t chcount=1 >
struct FrameFloat {
    static constexpr std::size_t sectionSizeExp     = fexp;
    static constexpr std::size_t sectionSize        = 1<<sectionSizeExp;
    static constexpr std::size_t sectionSizeMask    = sectionSize-1;
    static constexpr std::size_t vsectionSizeExp    = fexp-2;
    static constexpr std::size_t vsectionSize       = 1<<vsectionSizeExp;
    static constexpr std::size_t vsectionSizeMask   = vsectionSize-1;
    static constexpr std::size_t channelCount       = chcount;
    inline void clear()
    {
        memset( channel, 0, sizeof(channel) );
    }

    union  alignas(16) {
        v4sf    vchannel[ channelCount ][ vsectionSize ];
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
    inline void clear()
    {
        memset( channel, 0, sizeof(channel) );
    }
    union  alignas(16) {
        v4si    vchannel[ channelCount ][ vsectionSize ];
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
    inline void clear()
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
// ok
// state count ???
// TODO -> Blue-s from NoiseFrame
// parameters ??
template< typename Tstore, size_t upsampleRate = 1 > // oversamplingRate
class NoiseFrame : public Tstore {
public:
    static constexpr std::size_t sectionSizeExp     = Tstore::sectionSizeExp;
    static constexpr std::size_t sectionSize        = Tstore::sectionSize;
    static constexpr std::size_t sectionSizeMask    = sectionSize-1;
    static constexpr std::size_t vsectionSize       = Tstore::vsectionSize;
    static constexpr std::size_t vsectionSizeMask   = vsectionSize-1;
    static constexpr std::size_t channelCount       = Tstore::channelCount;
    static constexpr std::size_t sSize              = 16;

    NoiseFrame() = delete;
    NoiseFrame( GaloisShifter& gs )
    :   noise(gs)
    {
        clear();
    }

    inline void clear()
    {
        Tstore::clear();
        for( auto& sref : zv ) sref = 0;
    }

    // zero gain at fs/2
    // -24 dB
    inline void fillAvg()
    {
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = zv[0];
            zv[0] = noise.getWhite24();
            Tstore::channel[0][i] = zv[0] + x0;
        }
    }

    
    // pole += oversampling factor
    // shaped - low-high cut
    inline void fillWhite( void )
    {
        constexpr uint8_t   poleExp  = 9;
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = zv[0];
            const int32_t x1 = zv[1];
            zv[0] = noise.getWhite24();
            zv[1] += x0 - zv[0] - ( zv[1] >> poleExp );
            Tstore::channel[0][i] = zv[1] + x1;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
        }
    };

    // pole += oversampling factor
    // for the peeking filter
    // TODO check 2x,3x,4x oversampling and avg 
    inline void fillWhiteLowCut( void )
    {
        constexpr uint8_t poleExp   = 8;
        constexpr int32_t poleRound = 1<<(poleExp);      
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = zv[0];
            const int32_t x1 = zv[1];
//            const int32_t x2 = s[2];
            zv[0] = noise.getWhite24Avg();
            zv[1] += x0 - zv[0] - ( (zv[1] + poleRound ) >> poleExp );
            Tstore::channel[0][i] = (zv[1] + x1)>>2; // avoid overflow in filter
//            s[2] +=      s[1] - ( (s[2] + 1) >> 1 );
//            Tstore::channel[0][i] = s[2] + x2;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
        }
    };

    // pole += oversampling factor
    inline void fillWhiteBlue1( void )
    {
        constexpr uint8_t   poleExp  = 8;
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = zv[0];
            const int32_t x1 = zv[1];
            zv[0] = noise.getWhite24();
            zv[1] += x0 - zv[0] - ( (zv[1] + (1 << (poleExp-1)) )>> poleExp );
            Tstore::channel[0][i] = zv[1] + x1;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
        }
    };    

    // pole += oversampling factor
    inline void fillWhiteStereo( void )
    {
        static_assert(channelCount>1, "no stereo -- channelCount < 2");
        constexpr uint8_t   poleExp  = 9;
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = zv[0];
            const int32_t x1 = zv[1];
            const int32_t x2 = zv[2];
            const int32_t x3 = zv[3];
            zv[0] = noise.getWhite24();
            zv[1] += x0 - zv[0] - ( zv[1] >> poleExp );
            zv[2] = noise.getWhite24();
            zv[3] += x2 - zv[2] - ( zv[3] >> poleExp );
            Tstore::channel[0][i] = zv[1] + x1;
            Tstore::channel[1][i] = zv[3] + x3;
        }
    };

    // pole += oversampling factor
    // raw white
    // -24 dB
    inline void fillRaw()
    {
        for( auto i=0u; i<sectionSize; ++i ) {
            Tstore::channel[0][i] = noise.getWhite24()<<1;
        }
    }

    // pole += oversampling factor
    inline void fillBlue()
    {
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = zv[0];
            const int32_t x1 = zv[1];
            zv[0] = noise.getWhite24();
            zv[1] = x0 - zv[0];
            Tstore::channel[0][i] = zv[1] + x1;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
        }
    };

    // pole += oversampling factor
    inline void fillRed( void )
    {
        constexpr uint8_t   poleExp     = 9;
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = zv[0];
            const int32_t x2 = zv[2];
            zv[0] = noise.getWhite24();
            zv[1] += x0 - zv[0] - ( zv[1] >> poleExp );
            zv[2] += ( zv[1] >> 5 ) - ( zv[2] >> poleExp );
            Tstore::channel[0][i] = zv[2] + x2;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
        }
    };

    // pole += oversampling factor
    inline void fillPurple( void )
    {
        constexpr uint8_t   poleExp     = 9;
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = zv[0];
            const int32_t x3 = zv[3];
            zv[0] = noise.getWhite24();
            zv[1] += x0 - zv[0] - ( zv[1] >> poleExp );
            zv[2] += ( zv[1] >> 5 ) - ( zv[2] >> poleExp );
            zv[3] += ( zv[2] >> 7 ) - ( zv[3] >> poleExp );
            Tstore::channel[0][i] = zv[3] + x3;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
        }
    };

    // for fillRedVar, fillPurpleVar
    // poleExp = 0..15
    inline void setPoleExp( const uint8_t poleExp )
    {
        zv[sSize-1] = ( poleExp & 0x0F ) + 1; ;
    }

    // pole += oversampling factor
    // to test
    // s[sSize-1] - pole param -- function param
    inline void fillRedVar( void )
    {
        // gain compensation factors
        const int32_t cf1 = (zv[sSize-1]>>1) + 1;
        const int32_t cf2 = cf1 + (zv[sSize-1]&1);
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = zv[0];
            const int32_t x2 = zv[2];
            zv[0] = noise.getWhiteRaw() >> 4;
            zv[1] += x0 - zv[0] - ( zv[1] >> zv[sSize-1] );
            zv[2] += ( zv[1] >> cf1 ) + ( zv[1] >> cf2 ) - ( zv[2] >> zv[sSize-1] );
            Tstore::channel[0][i] = zv[2] + x2;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
        }
    };

    // pole += oversampling factor
    inline void fillRedVar( uint8_t pole )
    {
        // gain compensation factors
        const int32_t cf1 = (pole>>1) + 1;
        const int32_t cf2 = cf1 + (pole&1);
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = zv[0];
            const int32_t x2 = zv[2];
            zv[0] = noise.getWhiteRaw() >> 4;
            zv[1] += x0 - zv[0] - ( zv[1] >> pole );
            zv[2] += ( zv[1] >> cf1 ) + ( zv[1] >> cf2 ) - ( zv[2] >> pole );
            Tstore::channel[0][i] = zv[2] + x2;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
        }
    };
    
    // pole += oversampling factor
    // to test
    // s[sSize-1] - pole param
    inline void fillPurpleVar( uint8_t pole )
    {
        const int32_t cf1 = (pole>>1) + 1;
        const int32_t cf2 = cf1 + (pole&1);
        const int32_t cf3 = pole-2;

        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = zv[0];
            const int32_t x3 = zv[3];
            zv[0] = noise.getWhiteRaw() >> 4;
            zv[1] += x0 - zv[0] - ( zv[1] >> pole );
            zv[2] += ( zv[1] >> cf1 ) + ( zv[1] >> cf2 ) - ( zv[2] >> pole );
            zv[3] += ( zv[2] >> cf3 ) - ( zv[3] >> pole );
            Tstore::channel[0][i] = zv[3] + x3;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
        }
    };

    // pole += oversampling factor
    inline void fillPurpleVar( void )
    {
        const int32_t cf1 = (zv[sSize-1]>>1) + 1;
        const int32_t cf2 = cf1 + (zv[sSize-1]&1);
        const int32_t cf3 = zv[sSize-1]-2;

        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = zv[0];
            const int32_t x3 = zv[3];
            zv[0] = noise.getWhiteRaw() >> 4;
            zv[1] += x0 - zv[0] - ( zv[1] >> zv[sSize-1] );
            zv[2] += ( zv[1] >> cf1 ) + ( zv[1] >> cf2 ) - ( zv[2] >> zv[sSize-1] );
            zv[3] += ( zv[2] >> cf3 ) - ( zv[3] >> zv[sSize-1] );
            Tstore::channel[0][i] = zv[3] + x3;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
        }
    };

    // pole += oversampling factor
// The pinking filter consists of several one-pole low-pass filters,
// where each low-pass is spaced two octaves from its neighbors and
// filter gains are attenuated in 6dB steps.
// This configuration results in a nearly linear -3dB per octave overall
// frequency response when the low-pass filters are summed.
// http://www.firstpr.com.au/dsp/pink-noise/

    inline void fillPink()
    {
        constexpr int g = 2;
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

    // pole += oversampling factor
    // under 100 Hz pinkish above red
    inline void fillPinkLow()
    {
        constexpr int g = -2;
        constexpr int p = 15;
        int32_t x0 = zv[0] + zv[1] + zv[2] + zv[3] + zv[4];
        for( auto i=0u; i<sectionSize; ++i ) {
            zv[0] += ( noise.getWhiteRaw() >> ( p - 0 + 0 + g ) ) - ( zv[0]>>(p-0) );
            zv[1] += ( noise.getWhiteRaw() >> ( p - 2 + 1 + g ) ) - ( zv[1]>>(p-2) );
            zv[2] += ( noise.getWhiteRaw() >> ( p - 4 + 2 + g ) ) - ( zv[2]>>(p-4) );
            zv[3] += ( noise.getWhiteRaw() >> ( p - 6 + 3 + g ) ) - ( zv[3]>>(p-6) );
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
        }
    }
    
    // adapt
//    inline void fillVelvetDoubleTriangle2CH(uint8_t pulseCountExp )
//    {
//        static_assert( 2 == channelCount, "must be 2 channel");
//        static_assert( 11 >= sectionSizeExp, "section size too big (random 24 bit)");
//        static_assert( 6 <= sectionSizeExp, "section size too small");
//        const uint16_t slotSizeExp  = sectionSizeExp - pulseCountExp;
//        const uint16_t slotSize     = 1<<slotSizeExp;
//        const int32_t slotSizeMask  = (slotSize - 1) & 0xFFFCU; // 4 dot/wavelet
//        constexpr uint8_t normExp0  = 21; // 24 bit signed
//        constexpr uint8_t normExp1  = 19; // 24 bit signed
//        Tstore::clear();
//        // primary pulse train
//        for( auto pi=0u; pi < sectionSize; pi += slotSize ) {
//            const int32_t rand0 = noise.getWhiteRaw();
//            const int32_t rand1 = noise.getWhiteRaw();
//            const int32_t sign0 = ((rand0>>31) | 1)<< normExp0; // +1,-1
//            const int32_t sign1 = ((rand1>>31) | 1)<< normExp0; // +1,-1
//            const int32_t position0 = rand0 & slotSizeMask;
//            const int32_t position1 = rand1 & slotSizeMask;
//            Tstore::channel[ 0 ][ pi + position0 + 0 ] = sign0;
//            Tstore::channel[ 0 ][ pi + position0 + 1 ] = sign0 * 2;
//            Tstore::channel[ 0 ][ pi + position0 + 2 ] = sign0;
//
//            Tstore::channel[ 1 ][ pi + position1 + 0 ] = sign1;
//            Tstore::channel[ 1 ][ pi + position1 + 1 ] = sign1 * 2;
//            Tstore::channel[ 1 ][ pi + position1 + 2 ] = sign1;
//        }
//        // secondary pulse train
//        for( auto pi=0u; pi < sectionSize; pi += slotSize ) {
//            const int32_t rand0 = noise.getWhiteRaw();
//            const int32_t rand1 = noise.getWhiteRaw();
//            const int32_t sign0 = ((rand0>>31) | 1)<< normExp1; // +1,-1
//            const int32_t sign1 = ((rand1>>31) | 1)<< normExp1; // +1,-1
//            const int32_t position0 = rand0 & slotSizeMask;
//            const int32_t position1 = rand1 & slotSizeMask;
//            Tstore::channel[ 0 ][ pi + position0 + 1 ] += sign0;
//            Tstore::channel[ 0 ][ pi + position0 + 2 ] += sign0 * 2;
//            Tstore::channel[ 0 ][ pi + position0 + 3 ] += sign0;
//
//            Tstore::channel[ 1 ][ pi + position1 + 1 ] += sign1;
//            Tstore::channel[ 1 ][ pi + position1 + 2 ] += sign1 * 2;
//            Tstore::channel[ 1 ][ pi + position1 + 3 ] += sign1;
//        }
//    }
//
    
//    // primary : stereo
//    // secondary : mono
//    template< uint8_t pulseCountExp > 
//    inline void fillVelvetDoubleTriangle2CH_1CH()
//    {
//        static_assert( 2 == channelCount, "must be 2 channel");
//        static_assert( 11 >= sectionSizeExp, "section size too big (random 24 bit)");
//        static_assert( 6 <= sectionSizeExp, "section size too small");
//        constexpr uint32_t slotSizeExp  = sectionSizeExp - pulseCountExp;
//        constexpr uint32_t slotSize     = 1<<slotSizeExp;
//        constexpr uint32_t slotSizeMask = (slotSize - 1) & 0xFFFDU; // 4 dot/wavelet  odd-even
//        constexpr uint8_t normExp0  = 20; // 24 bit signed
//        constexpr uint8_t normExp1  = 20; // 24 bit signed
//        Tstore::clear();
//        for( auto pi=0u; pi < sectionSize; pi += slotSize ) {
//            const int32_t rand0 = noise.getWhiteRaw();
//            const int32_t rand1 = noise.getWhiteRaw();
//            const int32_t sign0 = ((rand0>>31) | 1)<< normExp0; // +1,-1
//            const int32_t sign1 = ((rand1>>31) | 1)<< normExp0; // +1,-1
//            const uint32_t position0 = rand0 & slotSizeMask;
//            const uint32_t position1 = rand1 & slotSizeMask;
//            Tstore::channel[ 0 ][ pi + position0 + 0 ] = sign0;
//            Tstore::channel[ 0 ][ pi + position0 + 1 ] = sign0 * 2;
//            Tstore::channel[ 0 ][ pi + position0 + 2 ] = sign0;
//
//            Tstore::channel[ 1 ][ pi + position1 + 0 ] = sign1;
//            Tstore::channel[ 1 ][ pi + position1 + 1 ] = sign1 * 2;
//            Tstore::channel[ 1 ][ pi + position1 + 2 ] = sign1;
//        }
//        // mono
//        for( auto pi=0u; pi < sectionSize; pi += slotSize ) {
//            const int32_t rand0 = noise.getWhiteRaw();
//            const int32_t sign0 = ((rand0>>31) | 1) << normExp1; // +1,-1
//            const uint32_t position0 = rand0 & slotSizeMask;
//            Tstore::channel[ 0 ][ pi + position0 + 0 ] += sign0;
//            Tstore::channel[ 0 ][ pi + position0 + 1 ] += sign0 * 2;
//            Tstore::channel[ 0 ][ pi + position0 + 2 ] += sign0;
//
//            Tstore::channel[ 1 ][ pi + position0 + 0 ] += sign0;
//            Tstore::channel[ 1 ][ pi + position0 + 1 ] += sign0 * 2;
//            Tstore::channel[ 1 ][ pi + position0 + 2 ] += sign0;
//        }
//    }
    

    
    // low cut: diff (zero) + 1 pole under 20 Hz @ 192kHz
    template<uint8_t stateOffs = 12>
    inline void postFilterDCcut( uint8_t poleExp=12 )    
    {        
        for( auto pi=0u; pi < sectionSize; ++pi ) {            
            const int32_t x0 = zv[ stateOffs+0 ];
            const int32_t x1 = zv[ stateOffs+1 ];
            zv[ stateOffs+0 ] = Tstore::channel[ 0 ][ pi ];
            zv[ stateOffs+1 ] += x0 - zv[ stateOffs+0 ] - (zv[ stateOffs+1 ]>>poleExp);
            Tstore::channel[ 0 ][ pi ] = zv[ stateOffs+1 ];
            
            const int32_t x2 = zv[ stateOffs+2 ];
            const int32_t x3 = zv[ stateOffs+3 ];            
            zv[ stateOffs+2 ] = Tstore::channel[ 1 ][ pi ];
            zv[ stateOffs+3 ] += x2 - zv[ stateOffs+2 ] - (zv[ stateOffs+3 ]>>poleExp);
            Tstore::channel[ 1 ][ pi ] = zv[ stateOffs+3 ];
        }
    }
    
    template<uint8_t stateOffs = 10>
    inline void postFilterLowPass1()
    {
        constexpr uint8_t poleExp = 7 + upsampleRate; // 9..
        constexpr uint8_t gainExp = 1 + upsampleRate; // 9..
        for( auto pi=0u; pi < sectionSize; ++pi ) {
            zv[ stateOffs+0 ] += (Tstore::channel[ 0 ][ pi ]>>gainExp) - (zv[ stateOffs+0 ]>>poleExp);
            Tstore::channel[ 0 ][ pi ] = zv[ stateOffs+0 ];
            zv[ stateOffs+1 ] += (Tstore::channel[ 1 ][ pi ]>>gainExp) - (zv[ stateOffs+1 ]>>poleExp);
            Tstore::channel[ 1 ][ pi ] = zv[ stateOffs+1 ];
        }
    }

    template<uint8_t stateOffs = 8>
    inline void postFilterLowPass2()
    {
        constexpr uint8_t poleExp = 6 + upsampleRate; // 9..
        constexpr uint8_t gainExp = 3 + upsampleRate; // 9..
        for( auto pi=0u; pi < sectionSize; ++pi ) {
            zv[ stateOffs+0 ] += (Tstore::channel[ 0 ][ pi ]>>gainExp) - (zv[ stateOffs+0 ]>>poleExp);
            zv[ stateOffs+1 ] += (zv[ stateOffs+0 ]>>gainExp) - (zv[ stateOffs+1 ]>>poleExp);
            Tstore::channel[ 0 ][ pi ] = zv[ stateOffs+1 ];

            zv[ stateOffs+2 ] += (Tstore::channel[ 1 ][ pi ]>>gainExp) - (zv[ stateOffs+2 ]>>poleExp);
            zv[ stateOffs+3 ] += (zv[ stateOffs+2 ]>>gainExp) - (zv[ stateOffs+3 ]>>poleExp);
            Tstore::channel[ 1 ][ pi ] = zv[ stateOffs+3 ];
        }
    }
    
    
    inline void fillVelvetPink2CH()
    {
        static_assert( 2 == channelCount, "must be 2 channel");
        static_assert( 11 >= sectionSizeExp, "section size too big (random 24 bit)");
        static_assert( 6 <= sectionSizeExp, "section size too small");
        constexpr int poleExpBase = 9;
        constexpr int amplExpBase = 14;
        // for 96khz maybe needed poleExpBase = 11; + 1 low filter stage
        // ch0
        lowPassVelvet<4,false>(0,0,amplExpBase+0,poleExpBase-0);
        lowPassVelvet<4,true>(0,2,amplExpBase+1,poleExpBase-2);
        lowPassVelvet<4,true>(0,4,amplExpBase+2,poleExpBase-4);
        lowPassVelvet<4,true>(0,6,amplExpBase+3,poleExpBase-6);
        lowPassVelvet<4,true>(0,8,amplExpBase+4,poleExpBase-8);
        // ch1
        lowPassVelvet<4,false>(1,1,amplExpBase+0,poleExpBase-0);
        lowPassVelvet<4,true>(1,3,amplExpBase+1,poleExpBase-2);
        lowPassVelvet<4,true>(1,5,amplExpBase+2,poleExpBase-4);
        lowPassVelvet<4,true>(1,7,amplExpBase+3,poleExpBase-6);
        lowPassVelvet<4,true>(1,9,amplExpBase+4,poleExpBase-8);
    }


    inline const auto& getFrame() const
    {
        return Tstore::channel[0];
    }
    inline const auto& getVFrame() const
    {
        return Tstore::vchannel[0];
    }

    int32_t zv[sSize];

private:

    template<uint8_t slotSizeExp, bool add >
    inline void lowPassVelvet( uint8_t ch, uint8_t state, uint8_t normExp, uint8_t poleExp )
    {
        constexpr uint16_t slotSize     = 1<<slotSizeExp;
        constexpr int32_t slotSizeMask  = (slotSize - 1) & 0xFFFFFFFDU;
        uint16_t offs = 0;
        for( auto pi = 0u; pi < sectionSize; )
        {
            const int32_t rand = noise.getWhiteRaw();
            const int32_t v = (((rand>>31) | 1) << normExp); // calc
            uint32_t position = offs + (rand & slotSizeMask);
            if( add ) {
                // add
                for( ; pi < position; ++pi ) {
                    // value 0
                    zv[ state ] -= ((zv[ state ])>>poleExp);
                    Tstore::channel[ ch ][ pi ] += zv[ state ];
                }

                zv[ state ] += v - ((zv[ state ])>>poleExp);
                Tstore::channel[ ch ][ pi++ ] += zv[ state ];
                zv[ state ] += v*2 - ((zv[ state ])>>poleExp);
                Tstore::channel[ ch ][ pi++ ] += zv[ state ];
                zv[ state ] += v - ((zv[ state ])>>poleExp);
                Tstore::channel[ ch ][ pi++ ] += zv[ state ];

                for( ; pi < offs + slotSize; ++pi ) {
                    // value 0
                    zv[ state ] -= ((zv[ state ])>>poleExp);
                    Tstore::channel[ ch ][ pi ] += zv[ state ];
                }
            } else{
                // set
                for( ; pi < position; ++pi ) {
                    // value 0
                    zv[ state ] -= ((zv[ state ])>>poleExp);
                    Tstore::channel[ ch ][ pi ] = zv[ state ];
                }

                zv[ state ] += v - ((zv[ state ])>>poleExp);
                Tstore::channel[ ch ][ pi++ ] = zv[ state ];
                zv[ state ] += v*2 - ((zv[ state ])>>poleExp);
                Tstore::channel[ ch ][ pi++ ] = zv[ state ];
                zv[ state ] += v - ((zv[ state ])>>poleExp);
                Tstore::channel[ ch ][ pi++ ] = zv[ state ];

                for( ; pi < offs + slotSize; ++pi ) {
                    // value 0
                    zv[ state ] -= ((zv[ state ])>>poleExp);
                    Tstore::channel[ ch ][ pi ] = zv[ state ];
                }
            }
            offs += slotSize;
        }
    }
    
    GaloisShifter&  noise;
};

} // end namespace yacynth

/*
k 1 f 5295.41
k 2 f 2197.79
k 3 f 1020.13 -- 1323
k 4 f 493.053
k 5 f 242.549 -- 331
k 6 f 120.312
k 7 f 59.9192 -- 82
k 8 f 29.9009
k 9 f 14.9358  -- 20.6
k 10 f 7.46425
k 11 f 3.73122 -- 5.17
k 12 f 1.86538
k 13 f 0.932633
k 14 f 0.466302
k 15 f 0.233148
k 16 f 0.116573 -> random LFO
k 17 f 0.0582862
k 18 f 0.0291431
k 19 f 0.0145715
k 20 f 0.00728575
 */

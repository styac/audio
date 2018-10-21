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
    inline void clear(void)
    {
        memset( channel, 0, sizeof(channel) );
    }

    union  alignas(16) {
        v4sf    vchannel[ channelCount ][ vsectionSize ];
        float   channel[ channelCount ][ sectionSize ];
    };
};

template< std::size_t fexp, std::size_t chcount=1 >
struct FrameFloatInterleave {
    static constexpr std::size_t sectionSizeExp     = fexp;
    static constexpr std::size_t sectionSize        = 1<<sectionSizeExp;
    static constexpr std::size_t sectionSizeMask    = sectionSize-1;
    static constexpr std::size_t channelCount       = chcount;
    static constexpr std::size_t interleavedSize    = sectionSize*channelCount;
    inline void clear(void)
    {
        memset( channel, 0, sizeof(channel) );
    }
    
    inline void set( FrameFloat<sectionSizeExp,channelCount> const & in, float norm )
    {
        for( int i=0; i<sectionSize; ++i ) {
            for( int c=0; c<channelCount; ++c) {
                channelX[i][c] = in.channel[c][i] * norm;                
            }
        }
    }
    inline void set( FrameFloat<sectionSizeExp,channelCount> const & in )
    {
        for( int i=0; i<sectionSize; ++i ) {
            for( int c=0; c<channelCount; ++c) {
                channelX[i][c] = in.channel[c][i];                
            }
        }
    }
    union {
        float channelX[ sectionSize ][channelCount ];
        float channel[ interleavedSize ];        
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
        v4si    vchannel[ channelCount ][ vsectionSize ];
        int32_t channel[ channelCount ][ sectionSize ];
    };
};
// --------------------------------------------------------------------
template< std::size_t fexp, std::size_t chcount=1 >
struct FrameIntInterleave {
    static constexpr std::size_t sectionSizeExp     = fexp;
    static constexpr std::size_t sectionSize        = 1<<sectionSizeExp;
    static constexpr std::size_t sectionSizeMask    = sectionSize-1;
    static constexpr std::size_t channelCount       = chcount;
    static constexpr std::size_t interleavedSize    = sectionSize*channelCount;
    inline void clear(void)
    {
        memset( channel, 0, sizeof(channel) );
    }
    inline void set( FrameInt<sectionSizeExp,channelCount> const & in, int norm )
    {
        for( uint32_t i=0; i<sectionSize; ++i ) {
            for( int c=0; c<channelCount; ++c) {
                channelX[i][c] = in.channel[c][i] * norm;                
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
        int32_t channelX[ sectionSize ][channelCount ];
        int32_t channel[ interleavedSize ];        
    };
};
// --------------------------------------------------------------------
// ok
// state count ???
// TODO -> Blue-s from NoiseFrame
// parameters ??
template< typename Tstore >
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
    {};

    inline void clear(void)
    {
        Tstore::clear();
        for( auto& sref : s ) sref = 0;
    }

    // zero gain at fs/2
    // -24 dB
    inline void fillAvg(void)
    {
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = s[0];
            s[0] = noise.getWhite24();
            Tstore::channel[0][i] = s[0] + x0;
        }
    }

    // shaped - low-high cut
    inline void fillWhite( void )
    {
        constexpr uint8_t   poleExp  = 9;
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = s[0];
            const int32_t x1 = s[1];
            s[0] = noise.getWhite24();
            s[1] += x0 - s[0] - ( s[1] >> poleExp );
            Tstore::channel[0][i] = s[1] + x1;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
        }
    };

    // for the peeking filter
    // TODO check 2x,3x,4x oversampling and avg 
    inline void fillWhiteLowCut( void )
    {
        constexpr uint8_t poleExp   = 8;
        constexpr int32_t poleRound = 1<<(poleExp);      
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = s[0];
            const int32_t x1 = s[1];
//            const int32_t x2 = s[2];
            s[0] = noise.getWhite24Avg();
            s[1] += x0 - s[0] - ( (s[1] + poleRound ) >> poleExp );
            Tstore::channel[0][i] = (s[1] + x1)>>2; // avoid overflow in filter
//            s[2] +=      s[1] - ( (s[2] + 1) >> 1 );
//            Tstore::channel[0][i] = s[2] + x2;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
        }
    };

    inline void fillWhiteBlue1( void )
    {
        constexpr uint8_t   poleExp  = 8;
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = s[0];
            const int32_t x1 = s[1];
            s[0] = noise.getWhite24();
            s[1] += x0 - s[0] - ( (s[1] + (1 << (poleExp-1)) )>> poleExp );
            Tstore::channel[0][i] = s[1] + x1;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
        }
    };    

    
    inline void fillWhiteStereo( void )
    {
        static_assert(channelCount>1, "no stereo -- channelCount < 2");
        constexpr uint8_t   poleExp  = 9;
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = s[0];
            const int32_t x1 = s[1];
            const int32_t x2 = s[2];
            const int32_t x3 = s[3];
            s[0] = noise.getWhite24();
            s[1] += x0 - s[0] - ( s[1] >> poleExp );
            s[2] = noise.getWhite24();
            s[3] += x2 - s[2] - ( s[3] >> poleExp );
            Tstore::channel[0][i] = s[1] + x1;
            Tstore::channel[1][i] = s[3] + x3;
        }
    };

    // raw white
    // -24 dB
    inline void fillRaw(void)
    {
        for( auto i=0u; i<sectionSize; ++i ) {
            Tstore::channel[0][i] = noise.getWhite24()<<1;
        }
    }

    inline void fillBlue(void)
    {
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = s[0];
            const int32_t x1 = s[1];
            s[0] = noise.getWhite24();
            s[1] = x0 - s[0];
            Tstore::channel[0][i] = s[1] + x1;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
        }
    };

    inline void fillRed( void )
    {
        constexpr uint8_t   poleExp     = 9;
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = s[0];
            const int32_t x2 = s[2];
            s[0] = noise.getWhite24();
            s[1] += x0 - s[0] - ( s[1] >> poleExp );
            s[2] += ( s[1] >> 5 ) - ( s[2] >> poleExp );
            Tstore::channel[0][i] = s[2] + x2;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
        }
    };

    inline void fillPurple( void )
    {
        constexpr uint8_t   poleExp     = 9;
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = s[0];
            const int32_t x3 = s[3];
            s[0] = noise.getWhite24();
            s[1] += x0 - s[0] - ( s[1] >> poleExp );
            s[2] += ( s[1] >> 5 ) - ( s[2] >> poleExp );
            s[3] += ( s[2] >> 7 ) - ( s[3] >> poleExp );
            Tstore::channel[0][i] = s[3] + x3;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
        }
    };

    // for fillRedVar, fillPurpleVar
    // poleExp = 0..15
    inline void setPoleExp( const uint8_t poleExp )
    {
        s[sSize-1] = ( poleExp & 0x0F ) + 1; ;
    }

    // to test
    // s[sSize-1] - pole param -- function param
    inline void fillRedVar( void )
    {
        // gain compensation factors
        const int32_t cf1 = (s[sSize-1]>>1) + 1;
        const int32_t cf2 = cf1 + (s[sSize-1]&1);
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = s[0];
            const int32_t x2 = s[2];
            s[0] = noise.getWhiteRaw() >> 4;
            s[1] += x0 - s[0] - ( s[1] >> s[sSize-1] );
            s[2] += ( s[1] >> cf1 ) + ( s[1] >> cf2 ) - ( s[2] >> s[sSize-1] );
            Tstore::channel[0][i] = s[2] + x2;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
        }
    };

    inline void fillRedVar( uint8_t pole )
    {
        // gain compensation factors
        const int32_t cf1 = (pole>>1) + 1;
        const int32_t cf2 = cf1 + (pole&1);
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = s[0];
            const int32_t x2 = s[2];
            s[0] = noise.getWhiteRaw() >> 4;
            s[1] += x0 - s[0] - ( s[1] >> pole );
            s[2] += ( s[1] >> cf1 ) + ( s[1] >> cf2 ) - ( s[2] >> pole );
            Tstore::channel[0][i] = s[2] + x2;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
        }
    };
    
    // to test
    // s[sSize-1] - pole param
    inline void fillPurpleVar( uint8_t pole )
    {
        const int32_t cf1 = (pole>>1) + 1;
        const int32_t cf2 = cf1 + (pole&1);
        const int32_t cf3 = pole-2;

        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = s[0];
            const int32_t x3 = s[3];
            s[0] = noise.getWhiteRaw() >> 4;
            s[1] += x0 - s[0] - ( s[1] >> pole );
            s[2] += ( s[1] >> cf1 ) + ( s[1] >> cf2 ) - ( s[2] >> pole );
            s[3] += ( s[2] >> cf3 ) - ( s[3] >> pole );
            Tstore::channel[0][i] = s[3] + x3;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
        }
    };

    inline void fillPurpleVar( void )
    {
        const int32_t cf1 = (s[sSize-1]>>1) + 1;
        const int32_t cf2 = cf1 + (s[sSize-1]&1);
        const int32_t cf3 = s[sSize-1]-2;

        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = s[0];
            const int32_t x3 = s[3];
            s[0] = noise.getWhiteRaw() >> 4;
            s[1] += x0 - s[0] - ( s[1] >> s[sSize-1] );
            s[2] += ( s[1] >> cf1 ) + ( s[1] >> cf2 ) - ( s[2] >> s[sSize-1] );
            s[3] += ( s[2] >> cf3 ) - ( s[3] >> s[sSize-1] );
            Tstore::channel[0][i] = s[3] + x3;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
        }
    };

// The pinking filter consists of several one-pole low-pass filters,
// where each low-pass is spaced two octaves from its neighbors and
// filter gains are attenuated in 6dB steps.
// This configuration results in a nearly linear -3dB per octave overall
// frequency response when the low-pass filters are summed.
// http://www.firstpr.com.au/dsp/pink-noise/

    inline void fillPink(void)
    {
        constexpr int g = 2;
        constexpr int p = 9;
        int32_t x0 = s[0] + s[1] + s[2] + s[3] + s[4];
        for( auto i=0u; i<sectionSize; ++i ) {
            // 14.9358 Hz
            s[0] += ( noise.getWhiteRaw() >> ( p - 0 + 0 + g ) ) - ( s[0]>>(p-0) );
            // 59.9192 Hz
            s[1] += ( noise.getWhiteRaw() >> ( p - 2 + 1 + g ) ) - ( s[1]>>(p-2) );
            // 242.549 Hz
            s[2] += ( noise.getWhiteRaw() >> ( p - 4 + 2 + g ) ) - ( s[2]>>(p-4) );
            // 1020.13 Hz
            s[3] += ( noise.getWhiteRaw() >> ( p - 6 + 3 + g ) ) - ( s[3]>>(p-6) );
            // 5295.41 Hz
            s[4] += ( noise.getWhiteRaw() >> ( p - 8 + 4 + g ) ) - ( s[4]>>(p-8) );
            const int32_t x1 = s[0] + s[1] + s[2] + s[3] + s[4];
            const int32_t x2 = s[5];
            // dc cut
            s[5] += ( x0 - x1 ) - ( s[5]>>(p+2) );
            // zero gain at fs/2
            Tstore::channel[0][i] = s[5] + x2;
            x0 = x1;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
        }
    };

    // under 100 Hz pinkish above red
    inline void fillPinkLow(void)
    {
        constexpr int g = -2;
        constexpr int p = 15;
        int32_t x0 = s[0] + s[1] + s[2] + s[3] + s[4];
        for( auto i=0u; i<sectionSize; ++i ) {
            s[0] += ( noise.getWhiteRaw() >> ( p - 0 + 0 + g ) ) - ( s[0]>>(p-0) );
            s[1] += ( noise.getWhiteRaw() >> ( p - 2 + 1 + g ) ) - ( s[1]>>(p-2) );
            s[2] += ( noise.getWhiteRaw() >> ( p - 4 + 2 + g ) ) - ( s[2]>>(p-4) );
            s[3] += ( noise.getWhiteRaw() >> ( p - 6 + 3 + g ) ) - ( s[3]>>(p-6) );
            s[4] += ( noise.getWhiteRaw() >> ( p - 8 + 4 + g ) ) - ( s[4]>>(p-8) );
            const int32_t x1 = s[0] + s[1] + s[2] + s[3] + s[4];
            const int32_t x2 = s[5];
            // dc cut
            s[5] += ( x0 - x1 ) - ( s[5]>>(p+2) );
            // zero gain at fs/2
            Tstore::channel[0][i] = s[5] + x2;
            x0 = x1;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
        }
    };
// #define VELVET_TEST
    // velvet noise mono
    // TODO : test alternate sign -- bad high freq sine -- low freq cut
    // TODO : test avg 1/2, 1, 1/2 -- aliasing
    
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
                Tstore::channel[ 1 ][ pi + position0 + 2] = Tstore::channel[ 0 ][ pi + position0 + 2 ] = sign0;
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
        static_assert( pulseCountExp < sectionSizeExp - 3, "pulse count too high");        
        //constexpr uint8_t pulseCountExp = pexp;
        constexpr uint8_t slotSizeExp   = sectionSizeExp - pulseCountExp;
        constexpr uint8_t slotSize      = 1<<slotSizeExp;
        constexpr int32_t slotSizeMask  = (slotSize - 1) & 0xFFFCU;
        constexpr uint8_t normExp       = 21;
        Tstore::clear();
        for( auto pi=0u; pi < sectionSize; pi += slotSize ) {
            const int32_t rand0 = noise.getWhiteRaw();
            const int32_t rand1 = noise.getWhiteRaw();
            const int32_t sign0 = ((rand0>>31) | 1)<<normExp; // +1,-1
            const int32_t sign1 = ((rand1>>31) | 1)<<normExp; // +1,-1           
            const int32_t position0 = rand0 & slotSizeMask;
            const int32_t position1 = rand1 & slotSizeMask;            
            Tstore::channel[ 0 ][ pi + position0 ]      = sign0;
            Tstore::channel[ 0 ][ pi + position0 + 1 ]  = Tstore::channel[ 0 ][ pi + position0 ] * 2;
            Tstore::channel[ 0 ][ pi + position0 + 2 ]  = Tstore::channel[ 0 ][ pi + position0 ];
            Tstore::channel[ 1 ][ pi + position1 ]      = sign1;
            Tstore::channel[ 1 ][ pi + position1 + 1 ]  = Tstore::channel[ 1 ][ pi + position1 ] * 2;
            Tstore::channel[ 1 ][ pi + position1 + 2 ]  = Tstore::channel[ 1 ][ pi + position1 ];
        }
    }

    // experimental - worse
    inline void fillVelvetSinxPx2CH()    
    {
        constexpr uint8_t pulseCountExp = 3;
        constexpr uint8_t slotSizeExp   = sectionSizeExp - pulseCountExp;
        constexpr uint8_t slotSize      = 1<<slotSizeExp;
        constexpr int32_t slotSizeMask  = (slotSize - 1) & 0xFFFCU;
        constexpr uint8_t normExp       = 21;
        constexpr double x              = PI/4.0;
        constexpr int32_t sinxpxPIp4    = (std::sin(x)/x) * (1<<(normExp-1));
        
        Tstore::clear();
        for( auto pi=0u; pi < sectionSize; pi += slotSize ) {
            const int32_t rand0 = noise.getWhiteRaw();
            const int32_t rand1 = noise.getWhiteRaw();
            const int32_t sign0 = ((rand0>>31) | 1); // +1,-1
            const int32_t sign1 = ((rand1>>31) | 1); // +1,-1           
            const int32_t position0 = rand0 & slotSizeMask;
            const int32_t position1 = rand1 & slotSizeMask;            
            Tstore::channel[ 0 ][ pi + position0 ]      = sign0*sinxpxPIp4;
            Tstore::channel[ 0 ][ pi + position0 + 1 ]  = sign0<<normExp;
            Tstore::channel[ 0 ][ pi + position0 + 2 ]  = Tstore::channel[ 0 ][ pi + position0 ];
            Tstore::channel[ 1 ][ pi + position1 ]      = sign1*sinxpxPIp4;
            Tstore::channel[ 1 ][ pi + position1 + 1 ]  = sign1<<normExp;
            Tstore::channel[ 1 ][ pi + position1 + 2 ]  = Tstore::channel[ 1 ][ pi + position1 ];
        }
    }
    
    // run after fill.... 
    // low cut: diff (zero) + 1 pole under 20 Hz @ 192kHz
    inline void postFilterDCcut( uint8_t poleExp=12 )    
    {        
//        constexpr uint8_t   poleExp  = 12; // under 20Hz 
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
    
    inline const auto& getFrame(void) const
    {
        return Tstore::channel[0];
    }
    inline const auto& getVFrame(void) const
    {
        return Tstore::vchannel[0];
    }

private:
    int32_t s[sSize];
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

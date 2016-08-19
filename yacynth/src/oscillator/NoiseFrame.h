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

<<<<<<< HEAD
/* 
=======
/*
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
 * File:   NoiseFrame.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on May 30, 2016, 6:31 PM
 */
#include    "../utils/GaloisNoiser.h"

using namespace noiser;

namespace yacynth {


template< std::size_t fexp >
struct FrameFloat {
<<<<<<< HEAD
    static constexpr std::size_t sectionSizeExp     = fexp;    
    static constexpr std::size_t sectionSize        = 1<<sectionSizeExp;    
    static constexpr std::size_t sectionSizeMask    = sectionSize-1;    
    static constexpr std::size_t vsectionSizeExp    = fexp-2;    
    static constexpr std::size_t vsectionSize       = 1<<vsectionSizeExp;    
    static constexpr std::size_t vsectionSizeMask   = vsectionSize-1;    
=======
    static constexpr std::size_t sectionSizeExp     = fexp;
    static constexpr std::size_t sectionSize        = 1<<sectionSizeExp;
    static constexpr std::size_t sectionSizeMask    = sectionSize-1;
    static constexpr std::size_t vsectionSizeExp    = fexp-2;
    static constexpr std::size_t vsectionSize       = 1<<vsectionSizeExp;
    static constexpr std::size_t vsectionSizeMask   = vsectionSize-1;
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
    static constexpr std::size_t channelCount       = 1;
    inline void clear(void)
    {
        for( auto& ichn : channel ) memset( ichn, 0, sectionSize*sizeof(float) );
    }
<<<<<<< HEAD
    
    union  alignas(16) {
        v4sf    vchannel[ channelCount ][ vsectionSize ];
        float   channel[ channelCount ][ sectionSize ];
    };     
=======

    union  alignas(16) {
        v4sf    vchannel[ channelCount ][ vsectionSize ];
        float   channel[ channelCount ][ sectionSize ];
    };
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
};
// --------------------------------------------------------------------
template< std::size_t fexp >
struct FrameInt {
<<<<<<< HEAD
    static constexpr std::size_t sectionSizeExp     = fexp;    
    static constexpr std::size_t sectionSize        = 1<<sectionSizeExp;    
    static constexpr std::size_t sectionSizeMask    = sectionSize-1;    
    static constexpr std::size_t vsectionSizeExp    = fexp-2;    
    static constexpr std::size_t vsectionSize       = 1<<vsectionSizeExp;    
    static constexpr std::size_t vsectionSizeMask   = vsectionSize-1;    
=======
    static constexpr std::size_t sectionSizeExp     = fexp;
    static constexpr std::size_t sectionSize        = 1<<sectionSizeExp;
    static constexpr std::size_t sectionSizeMask    = sectionSize-1;
    static constexpr std::size_t vsectionSizeExp    = fexp-2;
    static constexpr std::size_t vsectionSize       = 1<<vsectionSizeExp;
    static constexpr std::size_t vsectionSizeMask   = vsectionSize-1;
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
    static constexpr std::size_t channelCount       = 1;
    inline void clear(void)
    {
        for( auto& ichn : channel ) memset( ichn, 0, sectionSize*sizeof(int32_t) );
    }
    union  alignas(16) {
        v4si    vchannel[ channelCount ][ vsectionSize ];
        int32_t channel[ channelCount ][ sectionSize ];
<<<<<<< HEAD
    };         
};
// --------------------------------------------------------------------   
// ok
template< typename Tstore >
class NoiseFrame : public Tstore {
public:
    static constexpr std::size_t sectionSize        = Tstore::sectionSize;    
    static constexpr std::size_t sectionSizeMask    = sectionSize-1;    
    static constexpr std::size_t vsectionSize       = Tstore::vsectionSize;    
    static constexpr std::size_t vsectionSizeMask   = vsectionSize-1;    
    static constexpr std::size_t channelCount       = Tstore::channelCount;
        
=======
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
    static constexpr std::size_t sectionSize        = Tstore::sectionSize;
    static constexpr std::size_t sectionSizeMask    = sectionSize-1;
    static constexpr std::size_t vsectionSize       = Tstore::vsectionSize;
    static constexpr std::size_t vsectionSizeMask   = vsectionSize-1;
    static constexpr std::size_t channelCount       = Tstore::channelCount;

>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
    NoiseFrame() = delete;
    NoiseFrame( GaloisShifter& gs )
    :   galoisShifter(gs)
    {};
<<<<<<< HEAD
    
    inline void clear(void)
    {
        Tstore::clear(); 
    }
    
=======

    inline void clear(void)
    {
        Tstore::clear();
        for( auto& sref : s ) sref = 0;
    }

>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
    // zero gain at fs/2
    // -24 dB
    inline void fillAvg(void)
    {
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = s[0];
            s[0] = galoisShifter.getWhite24();
<<<<<<< HEAD
            Tstore::channel[0][i] = s[0] + x0;         
        }
    }
    
=======
            Tstore::channel[0][i] = s[0] + x0;
        }
    }

>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
    // shaped - low-high cut
    inline void fillWhite( void )
    {
        constexpr uint8_t   poleExp  = 9;
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = s[0];
<<<<<<< HEAD
            const int32_t x1 = s[1];            
            s[0] = galoisShifter.getWhite24();
            s[1] += x0 - s[0] - ( s[1] >> poleExp );
            Tstore::channel[0][i] = s[1] + x1;         
        }        
=======
            const int32_t x1 = s[1];
            s[0] = galoisShifter.getWhite24();
            s[1] += x0 - s[0] - ( s[1] >> poleExp );
            Tstore::channel[0][i] = s[1] + x1;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
        }
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
    };

    // for the peeking filter
    inline void fillWhiteBlue( void )
    {
        constexpr uint8_t   poleExp  = 7;
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = s[0];
<<<<<<< HEAD
            const int32_t x1 = s[1];            
            s[0] = galoisShifter.getWhite24();
            s[1] += x0 - s[0] - ( s[1] >> poleExp );
            Tstore::channel[0][i] = s[1] + x1;         
        }        
    };    
    
=======
            const int32_t x1 = s[1];
            s[0] = galoisShifter.getWhite24();
            s[1] += x0 - s[0] - ( s[1] >> poleExp );
            Tstore::channel[0][i] = s[1] + x1;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
        }
    };

>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
    inline void fillWhiteStereo( void )
    {
        static_assert(channelCount>1, "no stereo -- channelCount < 2");
        constexpr uint8_t   poleExp  = 9;
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = s[0];
<<<<<<< HEAD
            const int32_t x1 = s[1];            
            const int32_t x2 = s[2];
            const int32_t x3 = s[3];            
=======
            const int32_t x1 = s[1];
            const int32_t x2 = s[2];
            const int32_t x3 = s[3];
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
            s[0] = galoisShifter.getWhite24();
            s[1] += x0 - s[0] - ( s[1] >> poleExp );
            s[2] = galoisShifter.getWhite24();
            s[3] += x2 - s[2] - ( s[3] >> poleExp );
<<<<<<< HEAD
            Tstore::channel[0][i] = s[1] + x1;         
            Tstore::channel[1][i] = s[3] + x3;         
        }        
    };
    
=======
            Tstore::channel[0][i] = s[1] + x1;
            Tstore::channel[1][i] = s[3] + x3;
        }
    };

>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
    // raw white
    // -24 dB
    inline void fillRaw(void)
    {
        for( auto i=0u; i<sectionSize; ++i ) {
<<<<<<< HEAD
            Tstore::channel[0][i] = galoisShifter.getWhite24()<<1;         
        }
    }
    
=======
            Tstore::channel[0][i] = galoisShifter.getWhite24()<<1;
        }
    }

>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
    inline void fillBlue(void)
    {
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = s[0];
            const int32_t x1 = s[1];
            s[0] = galoisShifter.getWhite24();
            s[1] = x0 - s[0];
<<<<<<< HEAD
            Tstore::channel[0][i] = s[1] + x1;            
=======
            Tstore::channel[0][i] = s[1] + x1;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
        }
    };

    inline void fillRed( void )
    {
        constexpr uint8_t   poleExp     = 9;
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = s[0];
            const int32_t x2 = s[2];
            s[0] = galoisShifter.getWhite24();
            s[1] += x0 - s[0] - ( s[1] >> poleExp );
            s[2] += ( s[1] >> 5 ) - ( s[2] >> poleExp );
<<<<<<< HEAD
            Tstore::channel[0][i] = s[2] + x2;         
        }
    }; 
    
=======
            Tstore::channel[0][i] = s[2] + x2;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
        }
    };

>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
    inline void fillPurple( void )
    {
        constexpr uint8_t   poleExp     = 9;
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = s[0];
            const int32_t x3 = s[3];
<<<<<<< HEAD
            s[0] = galoisShifter.getWhite24();            
            s[1] += x0 - s[0] - ( s[1] >> poleExp );
            s[2] += ( s[1] >> 5 ) - ( s[2] >> poleExp );
            s[3] += ( s[2] >> 7 ) - ( s[3] >> poleExp );
            Tstore::channel[0][i] = s[3] + x3;         
        }
    }; 
    
=======
            s[0] = galoisShifter.getWhite24();
            s[1] += x0 - s[0] - ( s[1] >> poleExp );
            s[2] += ( s[1] >> 5 ) - ( s[2] >> poleExp );
            s[3] += ( s[2] >> 7 ) - ( s[3] >> poleExp );
            Tstore::channel[0][i] = s[3] + x3;
            if( 2 == channelCount ) {
                Tstore::channel[1][i] = Tstore::channel[0][i];
            }
        }
    };

>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
    // for fillRedVar, fillPurpleVar
    // poleExp = 0..15
    inline void setPoleExp( const uint8_t poleExp )
    {
        s[7] = ( poleExp & 0x0F ) + 1; ;
    }

    // to test
<<<<<<< HEAD
    // s[7] - pole param
=======
    // s[7] - pole param -- function param
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
    inline void fillRedVar( void )
    {
        // gain compensation factors
        const int32_t cf1 = (s[7]>>1) + 1;
<<<<<<< HEAD
        const int32_t cf2 = cf1 + (s[7]&1);        
=======
        const int32_t cf2 = cf1 + (s[7]&1);
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = s[0];
            const int32_t x2 = s[2];
            s[0] = galoisShifter.getWhiteRaw() >> 4;
            s[1] += x0 - s[0] - ( s[1] >> s[7] );
            s[2] += ( s[1] >> cf1 ) + ( s[1] >> cf2 ) - ( s[2] >> s[7] );
<<<<<<< HEAD
            Tstore::channel[0][i] = s[2] + x2;         
        }
    }; 
    
=======
            Tstore::channel[0][i] = s[2] + x2;
        }
    };

>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
    // to test
    // s[7] - pole param
    inline void fillPurpleVar( void )
    {
        const int32_t cf1 = (s[7]>>1) + 1;
        const int32_t cf2 = cf1 + (s[7]&1);
        const int32_t cf3 = s[7]-2;
<<<<<<< HEAD
        
=======

>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
        for( auto i=0u; i<sectionSize; ++i ) {
            const int32_t x0 = s[0];
            const int32_t x3 = s[3];
            s[0] = galoisShifter.getWhiteRaw() >> 4;
            s[1] += x0 - s[0] - ( s[1] >> s[7] );
            s[2] += ( s[1] >> cf1 ) + ( s[1] >> cf2 ) - ( s[2] >> s[7] );
            s[3] += ( s[2] >> cf3 ) - ( s[3] >> s[7] );
<<<<<<< HEAD
            Tstore::channel[0][i] = s[3] + x3;         
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
=======
            Tstore::channel[0][i] = s[3] + x3;
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
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
        constexpr int g = 2;
        constexpr int p = 9;
        int32_t x0 = s[0] + s[1] + s[2] + s[3] + s[4];
        for( auto i=0u; i<sectionSize; ++i ) {
            // 14.9358 Hz
            s[0] += ( galoisShifter.getWhiteRaw() >> ( p - 0 + 0 + g ) ) - ( s[0]>>(p-0) );
<<<<<<< HEAD
            // 59.9192 Hz        
=======
            // 59.9192 Hz
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
            s[1] += ( galoisShifter.getWhiteRaw() >> ( p - 2 + 1 + g ) ) - ( s[1]>>(p-2) );
            // 242.549 Hz
            s[2] += ( galoisShifter.getWhiteRaw() >> ( p - 4 + 2 + g ) ) - ( s[2]>>(p-4) );
            // 1020.13 Hz
            s[3] += ( galoisShifter.getWhiteRaw() >> ( p - 6 + 3 + g ) ) - ( s[3]>>(p-6) );
            // 5295.41 Hz
            s[4] += ( galoisShifter.getWhiteRaw() >> ( p - 8 + 4 + g ) ) - ( s[4]>>(p-8) );
            const int32_t x1 = s[0] + s[1] + s[2] + s[3] + s[4];
            const int32_t x2 = s[5];
            // dc cut
            s[5] += ( x0 - x1 ) - ( s[5]>>(p+2) );
            // zero gain at fs/2
            Tstore::channel[0][i] = s[5] + x2;
            x0 = x1;
        }
    };
<<<<<<< HEAD
    
    // under 100 Hz pinkish above red
    inline void fillPinkLow(void)
    {        
=======

    // under 100 Hz pinkish above red
    inline void fillPinkLow(void)
    {
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
        constexpr int g = -2;
        constexpr int p = 15;
        int32_t x0 = s[0] + s[1] + s[2] + s[3] + s[4];
        for( auto i=0u; i<sectionSize; ++i ) {
            s[0] += ( galoisShifter.getWhiteRaw() >> ( p - 0 + 0 + g ) ) - ( s[0]>>(p-0) );
            s[1] += ( galoisShifter.getWhiteRaw() >> ( p - 2 + 1 + g ) ) - ( s[1]>>(p-2) );
            s[2] += ( galoisShifter.getWhiteRaw() >> ( p - 4 + 2 + g ) ) - ( s[2]>>(p-4) );
            s[3] += ( galoisShifter.getWhiteRaw() >> ( p - 6 + 3 + g ) ) - ( s[3]>>(p-6) );
            s[4] += ( galoisShifter.getWhiteRaw() >> ( p - 8 + 4 + g ) ) - ( s[4]>>(p-8) );
            const int32_t x1 = s[0] + s[1] + s[2] + s[3] + s[4];
            const int32_t x2 = s[5];
            // dc cut
            s[5] += ( x0 - x1 ) - ( s[5]>>(p+2) );
            // zero gain at fs/2
            Tstore::channel[0][i] = s[5] + x2;
            x0 = x1;
        }
<<<<<<< HEAD
    };    
    
=======
    };

>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
    inline const auto& getFrame(void) const
    {
        return Tstore::channel[0];
    }
    inline const auto& getVFrame(void) const
    {
        return Tstore::vchannel[0];
    }
<<<<<<< HEAD
    
=======

>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
private:
    int32_t s[8];
    GaloisShifter&  galoisShifter;
};

} // end namespace yacynth

/*
 k 1 f 5295.41
<<<<<<< HEAD
k 2 f 2197.79 
k 3 f 1020.13 -- 1323
k 4 f 493.053
k 5 f 242.549 -- 331 
=======
k 2 f 2197.79
k 3 f 1020.13 -- 1323
k 4 f 493.053
k 5 f 242.549 -- 331
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
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
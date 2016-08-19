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
 * File:   NoiseSample.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on May 30, 2016, 6:33 PM
 */
#include    "../utils/GaloisNoiser.h"

using namespace noiser;

namespace yacynth {


class NoiseSample {
public:
        
    NoiseSample() = delete;
    NoiseSample( GaloisShifter& gs )
    :   galoisShifter(gs)
    {};
    
    inline void clear(void)
    {
        for( auto& sv : s ) sv = 0;
    }
    
    // zero gain at fs/2
    // -24 dB
    inline int32_t getAvg(void)
    {
        const int32_t x0 = s[0];
        s[0] = galoisShifter.getWhite24();            
        return s[0] + x0;         
    }

    inline int32_t getWhite( void )
    {
        constexpr uint8_t   poleExp  = 8;
        const int32_t x0 = s[0];
        const int32_t x1 = s[1];
        s[0] = galoisShifter.getWhite24();            
        s[1] += x0 - s[0] - ( s[1] >> poleExp );
        return s[1] + x1;         
    };
    
    // raw white
    // -24 dB
    inline int32_t getRaw(void)
    {
        return galoisShifter.getWhite24();            
    }


    inline int32_t getBlue(void)
    {
        const int32_t x0 = s[0];
        const int32_t x1 = s[1];
        s[0] = galoisShifter.getWhite24();            
        s[1] = x0 - s[0];
        return s[1] + x1;            
    };
    
    inline int32_t getBlue2(void)
    {
        const int32_t x0 = s[0];
        const int32_t x1 = s[1];
        const int32_t x2 = s[2];
        s[0] = galoisShifter.getWhite24();            
        s[1] = x0 - s[0];
        s[2] = x1 - s[1];         
        return s[2] + x2;            
    };
    
    inline int32_t getBlue3(void)
    {
        const int32_t x0 = s[0];
        const int32_t x1 = s[1];
        const int32_t x2 = s[2];
        const int32_t x3 = s[3];
        s[0] = galoisShifter.getWhite24();            
        s[1] = x0 - s[0];
        s[2] = x1 - s[1];         
        s[3] = (x2 - s[2])>>1;         
        return s[3] + x3;            
    };
    
    inline int32_t getBlue4(void)
    {
        const int32_t x0 = s[0];
        const int32_t x1 = s[1];
        const int32_t x2 = s[2];
        const int32_t x3 = s[3];
        const int32_t x4 = s[4];
        s[0] = galoisShifter.getWhite24();            
        s[1] = x0 - s[0];
        s[2] = x1 - s[1];         
        s[3] = (x2 - s[2])>>1;         
        s[4] = (x3 - s[3])>>1;                          
        return s[4] + x4;            
    };

    inline int32_t getBlue5(void)
    {
        const int32_t x0 = s[0];
        const int32_t x1 = s[1];
        const int32_t x2 = s[2];
        const int32_t x3 = s[3];
        const int32_t x4 = s[4];
        const int32_t x5 = s[5];
        s[0] = galoisShifter.getWhite24();            
        s[1] = x0 - s[0];
        s[2] = x1 - s[1];         
        s[3] = (x2 - s[2])>>1;         
        s[4] = (x3 - s[3])>>1;                          
        s[5] = (x4 - s[4])>>1;                          
        return s[5] + x5;            
    };
    
    inline int32_t getBlue6(void)
    {
        const int32_t x0 = s[0];
        const int32_t x1 = s[1];
        const int32_t x2 = s[2];
        const int32_t x3 = s[3];
        const int32_t x4 = s[4];
        const int32_t x5 = s[5];
        const int32_t x6 = s[6];
        s[0] = galoisShifter.getWhite24();            
        s[1] = x0 - s[0];
        s[2] = x1 - s[1];         
        s[3] = (x2 - s[2])>>1;         
        s[4] = (x3 - s[3])>>1;                          
        s[5] = (x4 - s[4])>>1;                          
        s[6] = (x5 - s[5])>>1;                          
        return s[6] + x6;            
    };   
    
    inline int32_t getBlueBlue( const uint8_t mx = 7 )
    {
        const  uint8_t m = mx&3;
        int32_t x[8];
        for( auto i=0u; i<=m; ++i ) {
            x[i] = s[i];
        }            
        s[0] = galoisShifter.getWhite24();            
        for( auto i=1u; i<=m; ++i ) {
            s[i] = ( x[i-1] - s[i-1] ) >> 1;
        }                    
        return s[m] + x[m];            
    };   
    
    inline int32_t getRed( void )
    {
        constexpr uint8_t   poleExp     = 9;
        const int32_t x0 = s[0];
        const int32_t x2 = s[2];
        s[0] = galoisShifter.getWhite24();            
        s[1] += x0 - s[0] - ( s[1] >> poleExp );
        s[2] += ( s[1] >> 6 ) - ( s[2] >> poleExp );
        return s[2] + x2;         
    }; 
            
    
    inline int32_t getPurple( void )
    {
        constexpr uint8_t   poleExp     = 9;
        const int32_t x0 = s[0];
        const int32_t x3 = s[3];
        s[0] = galoisShifter.getWhite24();            
        s[1] += x0 - s[0] - ( s[1] >> poleExp );
        s[2] += ( s[1] >> 6 ) - ( s[2] >> poleExp );
        s[3] += ( s[2] >> 8 ) - ( s[3] >> poleExp );
        return s[3] + x3;         
    }; 

    // 0..15
    inline void setPoleExp( const uint8_t poleExp )
    {
        s[7] = ( poleExp & 0x0F ) + 1; ;
    }

    // to test
    // s[7] - pole param
    inline int32_t getRedVar( void )
    {
        // gain compensation factors
        const int32_t cf1 = (s[7]>>1) + 1;
        const int32_t cf2 = cf1 + (s[7]&1);        
        const int32_t x0 = s[0];
        const int32_t x2 = s[2];
        s[0] = galoisShifter.getWhite24();            
        s[1] += x0 - s[0] - ( s[1] >> s[7] );
        s[2] += ( s[1] >> cf1 ) + ( s[1] >> cf2 ) - ( s[2] >> s[7] );
        return s[2] + x2;         
    }; 
    
    // to test
    // s[7] - pole param
    inline int32_t getPurpleVar( void )
    {
        const int32_t cf1 = (s[7]>>1) + 1;
        const int32_t cf2 = cf1 + (s[7]&1);
        const int32_t cf3 = s[7]-2;
        const int32_t x0 = s[0];
        const int32_t x3 = s[3];
        s[0] = galoisShifter.getWhite24();            
        s[1] += x0 - s[0] - ( s[1] >> s[7] );
        s[2] += ( s[1] >> cf1 ) + ( s[1] >> cf2 ) - ( s[2] >> s[7] );
        s[3] += ( s[2] >> cf3 ) - ( s[3] >> s[7] );
        return s[3] + x3;         
    }; 
    
// The pinking filter consists of several one-pole low-pass filters, 
// where each low-pass is spaced two octaves from its neighbors and 
// filter gains are attenuated in 6dB steps. 
// This configuration results in a nearly linear -3dB per octave overall 
// frequency response when the low-pass filters are summed. 
// http://www.firstpr.com.au/dsp/pink-noise/
    
    inline int32_t getPink(void)
    {        
        constexpr int g = 2;
        constexpr int p = 9;
        const int32_t x0 = s[0] + s[1] + s[2] + s[3] + s[4];
        // 14.9358 Hz
        s[0] += ( galoisShifter.getWhite24() >> ( p - 0 + 0 + g ) ) - ( s[0]>>(p-0) );
        // 59.9192 Hz        
        s[1] += ( galoisShifter.getWhite24() >> ( p - 2 + 1 + g ) ) - ( s[1]>>(p-2) );
        // 242.549 Hz
        s[2] += ( galoisShifter.getWhite24() >> ( p - 4 + 2 + g ) ) - ( s[2]>>(p-4) );
        // 1020.13 Hz
        s[3] += ( galoisShifter.getWhite24() >> ( p - 6 + 3 + g ) ) - ( s[3]>>(p-6) );
        // 5295.41 Hz
        s[4] += ( galoisShifter.getWhite24() >> ( p - 8 + 4 + g ) ) - ( s[4]>>(p-8) );
        const int32_t x1 = s[0] + s[1] + s[2] + s[3] + s[4];
        const int32_t x2 = s[5];
        // dc cut
        s[5] += ( x0 - x1 ) - ( s[5]>>(p+2) );
        // zero gain at fs/2
        return s[5] + x2;
    };
    
    // under 100 Hz pinkish above red
    inline int32_t getPinkLow(void)
    {        
        constexpr int g = -2;
        constexpr int p = 15;
        int32_t x0 = s[0] + s[1] + s[2] + s[3] + s[4];
        s[0] += ( galoisShifter.getWhite24() >> ( p - 0 + 0 + g ) ) - ( s[0]>>(p-0) );
        s[1] += ( galoisShifter.getWhite24() >> ( p - 2 + 1 + g ) ) - ( s[1]>>(p-2) );
        s[2] += ( galoisShifter.getWhite24() >> ( p - 4 + 2 + g ) ) - ( s[2]>>(p-4) );
        s[3] += ( galoisShifter.getWhite24() >> ( p - 6 + 3 + g ) ) - ( s[3]>>(p-6) );
        s[4] += ( galoisShifter.getWhite24() >> ( p - 8 + 4 + g ) ) - ( s[4]>>(p-8) );
        const int32_t x1 = s[0] + s[1] + s[2] + s[3] + s[4];
        const int32_t x2 = s[5];
        // dc cut
        s[5] += ( x0 - x1 ) - ( s[5]>>(p+2) );
        // zero gain at fs/2
        return s[5] + x2;
    };    

    inline float getWhiteFloat(void)
    {
        return static_cast<float> ( getWhite());
    }    
    inline float getAvgFloat(void)
    {
        return static_cast<float> ( getAvg() );
    }    
    inline float getRedFloat(void)
    {
        return static_cast<float> (getRed() );
    }
    inline float getRedVarFloat(void)
    {
        return static_cast<float> ( getRedVar());
    }
    inline float getPinkLowFloat(void)
    {
        return static_cast<float> ( getPinkLow());
    }
    
private:
    int32_t s[8];
    GaloisShifter&  galoisShifter;
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
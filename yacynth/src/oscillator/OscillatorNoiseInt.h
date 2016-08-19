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
 * File:   OscillatorNoiseInt.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on May 27, 2016, 8:49 PM
 */
#include    "../utils/FilterBase.h"

using namespace tables;
using namespace filter;

typedef int     v4si __attribute__((mode(SI)))  __attribute__ ((vector_size(16),aligned(16)));
typedef float   v4sf __attribute__((mode(SF)))  __attribute__ ((vector_size(16),aligned(16)));

namespace yacynth {

<<<<<<< HEAD
class OscillatorNoise : public FilterBase {
=======
class OscillatorNoise : public FilterBaseOld {
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
public:
    static constexpr    uint8_t stateCountExp   = 4;
    static constexpr    uint8_t stateCount      = 1<<stateCountExp;
    static constexpr    uint8_t feedbackSV1Exp  = 4;
    static constexpr    uint8_t feedbackSV2Exp  = 3;
    static constexpr    uint8_t feedbackSV3Exp  = 1;
    static constexpr    uint8_t feedbackSV4Exp  = 1;
    static constexpr    uint8_t feedback4pExp   = 2;
    
    OscillatorNoise()
    { clear(); };
        

    inline void clear(void) 
    {
        for( auto& x : sx ) x = 0;
    }
    
    // 4 pole --> "Moog"
    // feedback : 4x-epsilon 
    // input is ycent
    inline void setFreq4p( const int32_t ycent ) 
    {
        f = ftableExp2Pi.getInt( ycent ) >> 15; //? -> uint16
        s[0][3] = s[1][3] = s[2][3] = s[3][3] = f;
    }

    inline int32_t get4p( const int32_t sample )
    {         
        const int32_t vx = sample - ( ss[3] << feedback4pExp );
        const int32_t x0 = ss[6];
        ss[0] = (( ss[0] - vx ) >> 16 ) * f + vx;
        ss[1] = (( ss[1] - ss[0] ) >> 16 ) * f + ss[0];
        ss[2] = (( ss[2] - ss[1] ) >> 16 ) * f + ss[1];                                 
        ss[6] = (( ss[3] - ss[2] + 0x08000 ) >> 16 ) * f;  // rounding --> noise at DC !                      
        ss[3] = ss[6] + ss[2];                               
        return x0 + ss[6];  // cut at fs/2
    }; 
 
    // state variable
    // input is ycent
    inline void setFreqSv( const int32_t ycent ) 
    {
        f = ftable2SinPi.getInt( ycent ) >> 15; //? -> uint16
    }   
    inline int32_t getSv1( const int32_t sample )
    {                        
        ss[0] += (( ss[2] + 0x08000 ) >> 16 ) * f;
        ss[1] = sample - ss[0] - ( ss[2] >> feedbackSV1Exp );
        return ss[2] += (( ss[1] + 0x08000 ) >> 16 ) * f;   
    };
    
    inline int32_t getSv2( const int32_t sample )
    {                        
        ss[0] += (( ss[2] + 0x08000 ) >> 16 ) * f;
        ss[3] += (( ss[5] + 0x08000 ) >> 16 ) * f;
        ss[1] = sample - ss[0] - ( ss[2] >> feedbackSV2Exp );
        ss[4] = ss[2]  - ss[3] - ( ss[5] >> feedbackSV2Exp );
        ss[2] += (( ss[1] + 0x08000 ) >> 16 ) * f;   
        return ss[5] += (( ss[4] + 0x08000 ) >> 16 ) * f;   
    };
    inline int32_t getSv3( const int32_t sample )
    {                        
        ss[0] += (( ss[2] + 0x08000 ) >> 16 ) * f;
        ss[3] += (( ss[5] + 0x08000 ) >> 16 ) * f;
        ss[6] += (( ss[8] + 0x08000 ) >> 16 ) * f;
        ss[1] = sample - ss[0] - ( ss[2] >> feedbackSV3Exp );
        ss[4] = ss[2]  - ss[3] - ( ss[5] >> feedbackSV3Exp );
        ss[7] = ss[5]  - ss[6] - ( ss[8] >> feedbackSV1Exp );
        ss[2] += (( ss[1] + 0x08000 ) >> 16 ) * f;   
        ss[5] += (( ss[4] + 0x08000 ) >> 16 ) * f;   
        return ss[8] += (( ss[7] + 0x08000 ) >> 16 ) * f;   
    };
    
    // TODO test
    inline int32_t getSv4( const int32_t sample )
    {   
        union {
            v4si    vin;
            int32_t in[4];
        };
        
        in[0] = sample;
        in[1] = s[2][0];
        in[2] = s[2][1];
        in[3] = s[2][2];
        
        vs[0] += (( vs[2] + 0x08000 ) >> 16 ) * vs[3];
        vs[1] = vin - vs[0] - ( vs[2] >> feedbackSV4Exp );
        vs[2] += (( vs[1] + 0x08000 ) >> 16 ) * vs[3];   
        return s[2][3];
    };
              
private:
    union {
        v4si    vs[stateCount/4];   // vs[3] == f
        int32_t s[4][stateCount/4]; // s[3][x] == f
        int32_t sx[stateCount];
        struct {
            int32_t ss[stateCount-1];
            int32_t f;
        };
    };
};

#if 0
class OscillatorNoise4x {
public:
    OscillatorNoise4x()
    { clear(); };
        
    struct State {
        inline void clear(void)
        {
            sin = 0;
            vs[0] = vs[1] = vs[2] = vs[3] = v4si{0};
            g[0] = g[1] = g[2] = g[3] = 0;            
        }
        v4si        vs[4];
        union {
            v4si    vf;
            int32_t f[4];
        };     
        int32_t     sin;
        uint8_t     g[4];
    };
    
    inline void clear(void) 
    {
        state.clear();
    }
    
    inline void setF( const uint8_t index, const uint16_t f ) 
    {
        state.f[index&3] = f;
    }
    inline void setG( const uint8_t index, const uint8_t g ) 
    {
        state.g[index&3] = g;
    }

    inline int32_t get( const int32_t sample )
    {                                
        const int32_t x0 = state.sin;
        state.sin = sample;            
        const v4si vx =  state.sin + x0 - ( state.vs[3] << 2 );
        state.vs[0] = (( state.vs[0] - vx ) >> 16 ) * state.vf + vx;
        state.vs[1] = (( state.vs[1] - state.vs[0] ) >> 16 ) * state.vf + state.vs[0];
        state.vs[2] = (( state.vs[2] - state.vs[1] ) >> 16 ) * state.vf + state.vs[1];  
        
        const union {
            v4si        vs3;
            int32_t     s3[4];                                
        } vv = { .vs3 = (( state.vs[3] - state.vs[2] + 0x08000 ) >> 16 ) * state.vf  };  // rounding --> noise at DC !                      
        state.vs[3] = vv.vs3 + state.vs[2];                               
        return ( vv.s3[0] >> state.g[0] ) + ( vv.s3[1] >> state.g[1] ) + ( vv.s3[2] >> state.g[2] ) + ( vv.s3[3] >> state.g[3] );
    }; 
        
private:
    State           state;    
};
#endif

} // end namespace yacynth


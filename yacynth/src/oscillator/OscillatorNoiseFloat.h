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
 * File:   OscillatorNoiseFloat.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on May 27, 2016, 8:49 PM
 */
#include    "../utils/GaloisNoiser.h"
#include    "../utils/FilterBase.h"

using namespace noiser;
using namespace filter;

namespace yacynth {

class OscillatorNoiseFloat : public FilterBaseOld {
public:

    static constexpr float    fcontrolMin       = std::exp(-PI2*fcMin);
    static constexpr float    fcontrolMax       = std::exp(-PI2*fcMax);
    
    OscillatorNoiseFloat() = delete;
    OscillatorNoiseFloat( GaloisShifter& gs )
    :   galoisShifter(gs)
    { clear(); };
        
    struct State {
        inline void clear(void)
        {
            sin = 0;
            vs[0] = vs[1] = vs[2] = vs[3] = v4sf{0};
            g[0] = g[1] = g[2] = g[3] = 1.0f;            
        }
        v4sf        vs[4];
        union {
            v4sf    vf;
            float   f[4];
        };     
        union {
            v4sf    vg;
            float   g[4];
        };     
        int32_t     sin;
    };
    inline void clear(void) 
    {
        state.clear();
    }

    inline void setFreq( const uint8_t ind, const int32_t fv )
    {
        const float freq = ftableExp2Pi.getFloat( fv );
        state.f[index&3] = std::max( fcontrolMax, std::min( fcontrolMin, freq ));

  std::cout << "ind " << uint16_t(ind)  << " f " << state.f[index&3] << std::endl;
    };
    
    inline void setGain( const uint8_t index, const float g ) 
    {
        state.g[index&3] = g;
    }

//    inline int32_t get4x4pole( void )
    inline float get4x4pole( void )
    {                        
        const int32_t x0 = state.sin;
        state.sin = galoisShifter.getWhiteRaw() >> 3;        
        const v4sf vx = static_cast<float>( state.sin + x0 /* multiply by envelope */ ) - ( state.vs[3] * 3.9999f );
        state.vs[0] = ( state.vs[0] -          vx   ) * state.vf + vx;
        state.vs[1] = ( state.vs[1] - state.vs[0]   ) * state.vf + state.vs[0];
        state.vs[2] = ( state.vs[2] - state.vs[1]   ) * state.vf + state.vs[1];  
        const v4sf vs = ( state.vs[3] - state.vs[2] ) * state.vf;               
        state.vs[3] = vs + state.vs[2];                               

        const union {
            v4sf      vs3;
            float     s3[4];
        } vv = { .vs3 = vs * state.vg };   

        return vv.s3[0] + vv.s3[1] + vv.s3[2] + vv.s3[3]; // separate channel ??
//        return int32_t(  vv.s3[0] + vv.s3[1] + vv.s3[2] + vv.s3[3] );
    };    

    template< std::size_t count >
    inline void get4x4pole( float * out )
    {                        
        for( auto i = 0u; i < count; ++i ) {
            const int32_t x0 = state.sin;
            state.sin = galoisShifter.getWhiteRaw() >> 3;        
            const v4sf vx = static_cast<float>( state.sin + x0 /* multiply by envelope */ ) - ( state.vs[3] * 3.9999f );
            state.vs[0] = ( state.vs[0] -          vx ) * state.vf + vx;
            state.vs[1] = ( state.vs[1] - state.vs[0] ) * state.vf + state.vs[0];
            state.vs[2] = ( state.vs[2] - state.vs[1] ) * state.vf + state.vs[1];  
            const v4sf vs = ( state.vs[3] - state.vs[2] ) * state.vf;               
            state.vs[3] = vs + state.vs[2];                               
            
            const union {
                v4sf      vs3;
                float     s3[4];
            } vv = { .vs3 = vs * state.vg };   
            
            *out++ = vv.s3[0] + vv.s3[1] + vv.s3[2] + vv.s3[3];
//            *out++ += vv.s3[0] + vv.s3[1] + vv.s3[2] + vv.s3[3];
        }
    }; 

    // shifter forwarding
    inline void reset(void)
    {
        galoisShifter.reset();
    }
    inline void inc(void)
    {
        galoisShifter.inc();
    };
    inline uint64_t getState(void)
    {
        return galoisShifter.getState();
    };
    inline uint64_t get(void)
    {
        return galoisShifter.get();
    };
    
    inline int32_t getWhiteRaw(void)
    {
        return galoisShifter.getWhiteRaw();
    };    
    
private:
    State           state;    
    GaloisShifter&  galoisShifter;
};


} // end namespace yacynth


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
 * File:   EffectOscillator.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on April 17, 2016, 9:09 PM
 */
#include    "../oscillator/BaseOscillator.h"
#include    "Ebuffer.h"

namespace yacynth {

template< std::size_t sExp>
struct VsizeParam {
    static constexpr std::size_t varraySizeExp  = sExp;
    static constexpr std::size_t varraySize     = 1<<varraySizeExp;    
    static constexpr std::size_t arraySizeExp   = varraySizeExp+2;
    static constexpr std::size_t arraySize      = arraySizeExp;
    static constexpr std::size_t arraySizeMask  = arraySize-1;
    static constexpr std::size_t arraySizeExp2  = arraySizeExp+1;
    static constexpr std::size_t arraySize2     = arraySizeExp2;
    static constexpr std::size_t arraySizeMask2 = arraySize2-1;
};

template< std::size_t sExp >
struct alignas(16) ValueArray : public VsizeParam<sExp> {
    union {
        v4si        v[varraySize];
        uint32_t    i[arraySize];        
    };
};

template< typename T, typename TV, std::size_t sExp >
struct alignas(16) TargetParamVector : VsizeParam<sExp> {    
    union {
        TV vv[varraySize];
        T  v[arraySize];
    };        
};

template< std::size_t sExp>
class EffectOscillator : public VsizeParam<sExp> {
public:        
    void reset(void)
    {
        for( auto i=0u; i<varraySize; ++i ) {
            phase.v[i] = initPhase.v[i];
        }
    }
        
    void inc(void)
    {
        for( auto i=0u; i<varraySize; ++i ) {
            phase.v[i] += deltaPhase.v[i];
        }
    }
    
    uint32_t get( std::size_t ind )
    {
        return phase.i[ind];
    }
    
    inline static EffectOscillator& getInstance(void)
    {
        static EffectOscillator instance;
        return instance;
    }
    
private:
    EffectOscillator() {}
    // purple lfo <1> -> phase + am to osc
    ValueArray<sExp>     phase;
    ValueArray<sExp>     initPhase;
    ValueArray<sExp>     deltaPhase;
//    MapperControllerInt2Int controller[arraySize];
};



} // end namespace yacynth


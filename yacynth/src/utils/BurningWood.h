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
 * File:   BurningWood.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on March 29, 2016, 7:38 PM
 */

namespace yacynth {

class BurningWood {
public:
    static constexpr uint64_t seed      = 0x868485654FE84945ULL;
    static constexpr int shdownWhite    = 24; // spectrum looks white noise in this slice
    static constexpr int afilt          = 5;
    static constexpr int bfilt          = 1023;

    // carefully handmade overflow tricks
    static constexpr int afiltB         = 4;
    static constexpr int bfiltB         = 1019;
    static constexpr float rangeP       = 1.0f/(1L<<22);
    static constexpr float rangeW       = 1.0f/(1L<<32);
    
    struct Filterstate {
        int32_t     dc;
        int32_t     avg;
        int32_t     y_1;
    };    
    
    BurningWood()
    :   lfsr(seed)
    {};    
    inline void reset(void)
    {
        lfsr = seed;
    };    
    inline uint64_t get(void)
    {
        return lfsr = (lfsr << 1) ^ (uint64_t(( int64_t(lfsr & 0x8000000000000000ULL)>>63) & 0xd198000000000001ULL));
    };
    inline int32_t getWhite(void)
    {
        return get() >> shdownWhite;
    };    
    void getWhite( float& outA, float& outB )
    {
        outA = float( getWhite() ) * rangeW;
        outB = float( getWhite() ) * rangeW;
    }    
    void getWhite( int32_t& outA, int32_t& outB )
    {
        outA = getWhite();
        outB = getWhite();
    }     
    void getRed( float& outA, float& outB )
    {
        int32_t A, B;
        getRed( A, B );
        outA = float(A) * rangeP;
        outB = float(B) * rangeP;
    }
    void getRed( int32_t& outA, int32_t& outB )
    {
        filterA.dc  = getWhite() - filterA.dc;
        filterB.dc  = getWhite() - filterB.dc;
        filterA.avg = ( filterA.dc + filterA.avg ) >> 1;
        filterB.avg = ( filterB.dc + filterB.avg ) >> 1;
        outA = filterA.y_1 = (( filterA.avg >> afilt ) + bfilt * filterA.y_1 ) >> 10;
        outB = filterB.y_1 = (( filterB.avg >> afilt ) + bfilt * filterB.y_1 ) >> 10;
    }
    void getBurningWood( float& outA, float& outB )
    {
        int32_t A, B;
        getBurningWood( A, B );
        outA = float(A) * rangeP;
        outB = float(B) * rangeP;
    }
    void getBurningWood( int32_t& outA, int32_t& outB )
    {
        filterA.dc  = getWhite() - filterA.dc;
        filterB.dc  = getWhite() - filterB.dc;
        const int32_t fire = getWhite();
        filterA.avg =  ( filterA.dc + filterA.avg ) >> 1;
        filterB.avg =  ( filterB.dc + filterB.avg ) >> 1;        
        outA = filterA.y_1 = ((( fire + filterA.avg ) >> afiltB ) + bfiltB * filterA.y_1 ) >> 10;
        outB = filterB.y_1 = ((( fire + filterB.avg ) >> afiltB ) + bfiltB * filterB.y_1 ) >> 10;
    }
    
private:
    uint64_t    lfsr;
    Filterstate filterA;
    Filterstate filterB;
};

} // end namespace yacynth


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
 * File:   GaloisNoiser.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on March 12, 2016, 7:11 AM
 */

#include <cstdint>
#include <cmath>

// 0
constexpr uint64_t seedThreadEffect_noise       = 0x868485654FE84945ULL;
// i c000000000 vv 65cb8e6b9323242d
constexpr uint64_t seedThreadOscillator_noise   = 0x65cb8e6b9323242dULL;
// i 4000000000 vv e5eb475e9f173ed2 from seedThreadOscillator_noise
constexpr uint64_t seedThreadEffect_random      = 0xe5eb475e9f173ed2ULL;
// i 8000000000 vv from seedThreadOscillator_noise
constexpr uint64_t seedThreadOscillator_random  = 0x2da068af77c909abULL;


// i 8000000000 vv e4858c0e6cbc01b--

// --------------------------------------------------------------------
class GaloisShifter {
public:
    //                                      7766554433221100
//  static constexpr uint64_t feedback  = 0xd19850abe0000001ULL;
    static constexpr uint64_t feedback  = 0xd198000000000001ULL; // original max length

    static constexpr int shdownWhite    = 24; // spectrum looks white noise in this slice

    GaloisShifter( const uint64_t seed = seedThreadEffect_noise ) // default should be deleted
    :   lfsr(seed)
    {};

    inline void reset( const uint64_t seed )
    {
        lfsr = seed;
    }

    inline void inc()
    {
        lfsr = (lfsr + lfsr) ^ (uint64_t(( int64_t(lfsr)>>63) & feedback ));
    }

    inline uint64_t getState()
    {
        return lfsr;
    }

    inline uint64_t get()
    {
        inc();
        return lfsr;
    }

    inline int32_t get16()
    {
        inc();
        return lfsr16[3];   // most random part
    }

    inline int32_t getLow()
    {
        inc();
        return lfsr32[0];
    }

    inline int32_t getHigh()
    {
        inc();
        return lfsr32[1];
    }

    // 24 bit significant
    inline int32_t getWhite24()
    {
        union {
            int32_t  res;
            int16_t  res16[2];
        };
        inc();
        // +3dB at 10kHz
        res16[1] = lfsr8[6]^lfsr8[7];
        res16[0] = lfsr16[3]^lfsr16[2];
        return res;
    }
            
    // good red noise -20 db / decade
    inline int32_t getRed24()
    {
        static int64_t acc = 0;
        
        inc();
        acc += lfsr32[1]>>8;
        return acc >> 8;
    }
    
    inline int32_t getWhite24Avg()
    {
        union {
            int32_t  res;
            int16_t  res16[2];
        };
        res16[1] = lfsr8[6]^lfsr8[7];
        res16[0] = lfsr16[3]^lfsr16[2];
        const int32_t r0 = res;
        inc();
        res16[1] = lfsr8[6]^lfsr8[7];
        res16[0] = lfsr16[3]^lfsr16[2];
        return r0 + res;
    }

    inline int32_t getWhiteRaw()
    {
        inc();
        return (lfsr >> shdownWhite);
    }

    // 0..8 is BLUE
    inline int32_t getWhiteRaw( const uint8_t sh )
    {
        inc();
        return lfsr >> sh;
    }


    inline int32_t prng()
    {
        static uint32_t p = 12345;
        p = p * 1103515245 + 12345;

        // scale down
        return int32_t(p)>>8;
    }


protected:
    union alignas(sizeof(uint64_t)) {
        uint64_t        lfsr;
        int32_t         lfsr32[2];
        int16_t         lfsr16[4];
        int8_t          lfsr8[8];
    };
};


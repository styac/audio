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

    inline void inc(void)
    {
        lfsr = (lfsr + lfsr) ^ (uint64_t(( int64_t(lfsr)>>63) & feedback ));
    };
    inline uint64_t getState(void)
    {
        return lfsr;
    };

    inline uint64_t get(void)
    {
        inc();
        return lfsr;
    };

    inline uint16_t get16(void)
    {
        inc();
        return lfsr16[7];   // most random part
    };

    inline uint32_t getLow(void)
    {
        inc();
        return lfsr32[0];
    };

    inline uint32_t getHigh(void)
    {
        inc();
        return lfsr32[1];
    };

    // 24 bit significant
    inline int32_t getWhite24(void)
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
    };
            
    // good red noise -20 db / decade
    inline int32_t getRed24(void)
    {
        static int64_t acc = 0;
        
        inc();
        acc += lfsr32[1]>>8;
        return acc >> 8;
    };
    
    inline int32_t getWhite24Avg(void)
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
    };

    inline int32_t getWhiteRaw(void)
    {
        inc();
        return (lfsr >> shdownWhite);
    };

    // 0..8 is BLUE
    inline int32_t getWhiteRaw( const uint8_t sh )
    {
        inc();
        return lfsr >> sh;
    };

protected:
    union alignas(sizeof(uint64_t)) {
        uint64_t        lfsr;
        int32_t         lfsr32[2];
        int16_t         lfsr16[4];
        int8_t          lfsr8[8];
    };
};


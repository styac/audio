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
 * File:   OscillatorOutput.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on February 1, 2016, 5:25 PM
 */

#include    "yacynth_globals.h"
#include    "message/yamsg.h"
#include    <algorithm>
#include    <array>
#include    <atomic>
#include    <cstring>
#include    <iostream>

namespace yacynth {

// TODO : reduce and test
// TODO : sync mode must be made first currently 4 is the minimum if external/internal framesize = 4
constexpr   uint64_t bufCountExp    = 2;
constexpr   uint64_t bufCount       = (1LL<<bufCountExp);
constexpr   uint64_t bufCountMask   = bufCount-1LL;

class alignas(cacheLineSize) OscillatorOut {
public:
    OscillatorOut()
        { clear(); };
    void clear(void)
        {
            memset(this, 0, sizeof(OscillatorOut));
        };
    int64_t     layer[ oscOutputChannelCount ][ oscillatorFrameSize ];
    int64_t     amplitudeSumm[ oscOutputChannelCount ];  // collect the summ for the given period for the compressor
};

class OscillatorOutVector {
public:
    inline static OscillatorOutVector& getInstance(void)
    {
        static OscillatorOutVector instance;
        return instance;
    };

    uint16_t    getReadIndex(void)      { return readPtr & bufCountMask;  };
    //uint16_t    peekReadIndexNext(void) { return ( readPtr + 1 ) & bufCountMask;  };
    uint16_t    getWriteIndex(void)     { return writePtr & bufCountMask;  };
    uint16_t    getFullCount(void)      { return  ( writePtr - readPtr ); };
    bool        isFull(void)            { return bufCount  <= ( writePtr - readPtr ); };
    bool        isEmpty(void)           { return readPtr >= writePtr; };
    void        readOk(void)            { ++readPtr;  };
    void        writeOk(void)           { ++writePtr; };
    std::array<OscillatorOut, bufCount> out;

    // never wraps around in the life on earth
    std::atomic<uint64_t>               readPtr;
    std::atomic<uint64_t>               writePtr;

private:
    OscillatorOutVector()
    :   readPtr(0)
    ,   writePtr(0)
    {};
};

} // end namespace yacynth

/*
https://nativecoding.wordpress.com/2015/06/19/multithreading-multicore-programming-and-false-sharing-benchmark/
http://linux.die.net/man/3/cpu_set
http://man7.org/linux/man-pages/man3/CPU_SET.3.html

 */
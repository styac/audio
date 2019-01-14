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

#include "yacynth_globals.h"
#include <array>
#include <atomic>
#include <cstring>

namespace yacynth {

// check this
// https://www.justsoftwaresolutions.co.uk/files/ndc_oslo_2016_safety_off.pdf
// https://www.codeproject.com/Articles/43510/Lock-Free-Single-Producer-Single-Consumer-Circular
// http://www.codersblock.org/blog/2016/6/02/ditching-the-mutex

class alignas(cacheLineSize) OscillatorOut {
public:
    OscillatorOut()
        { clear(); };
    void clear()
        {
            memset(this, 0, sizeof(OscillatorOut));
        };
    int64_t     layer[ oscOutputChannelCount ][ oscillatorFrameSize ];
    int64_t     amplitudeSumm[ oscOutputChannelCount ]; // sum of amplitudes for a frame
};

class OscillatorOutVector {
public:
    static constexpr uint64_t bufCountExp    = 1;
    static constexpr uint64_t bufCount       = (1LL<<bufCountExp);
    static constexpr uint64_t bufCountMask   = bufCount-1LL;
    static constexpr uint64_t maxFillCount   = bufCount; 

    inline static auto& getInstance()
    {
        static OscillatorOutVector instance;
        return instance;
    }
    
    uint16_t getReadIndex() const
    { 
        return readIndex.load(std::memory_order_acquire) & bufCountMask;  
    }
    
    uint16_t getWriteIndex() const 
    { 
        return writeIndex.load(std::memory_order_acquire) & bufCountMask; 
    }
    
    bool isFull() const       
    { 
        return maxFillCount == ( writeIndex.load(std::memory_order_acquire) - readIndex.load(std::memory_order_acquire) ); 
    }
    
    bool isEmpty() const      
    { 
        return readIndex.load(std::memory_order_acquire) == writeIndex.load(std::memory_order_acquire); 
    }
    
    void readFinished()        
    { 
        ++readIndex;  
    }
    
    void writeFinished()       
    { 
        ++writeIndex; 
    }
    
    std::array<OscillatorOut, bufCount> out;

private:
    uint64_t                            pad0[(cacheLineSize - sizeof(uint64_t))/8];
    std::atomic<uint64_t>               readIndex;
    uint64_t                            pad1[(cacheLineSize - sizeof(uint64_t))/8];
    std::atomic<uint64_t>               writeIndex;

    OscillatorOutVector()
    : readIndex(0)
    , writeIndex(0)
    {}
};

} // end namespace yacynth

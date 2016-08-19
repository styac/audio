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
 * File:   YaIoInQueue.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on February 10, 2016, 4:43 PM
 */

#include    <cstdint>
#include    <atomic>
#include    <string.h>

namespace yacynth {

constexpr uint32_t inQueueBufSizeExp    = 10;
constexpr uint32_t inQueueBufSize       = (1<<inQueueBufSizeExp);
constexpr uint64_t inQueueBufSizeMask   = (inQueueBufSize - 1LL );

class YaIoInQueue {
public:
// --------------------------------------------------------------------
    YaIoInQueue()
    :   head(0)
    ,   tail(0)
    { memset(queue,0, sizeof(queue)); };
// --------------------------------------------------------------------
    bool        put( uint64_t data ) {
        uint64_t expect = 0;
        if( 0 == data )
            return true;    // cant put a zero
        if( queue[ head & inQueueBufSizeMask ].compare_exchange_strong( expect, data ) ) {
            ++head;
            return true;
        }
        return false;
    }
// --------------------------------------------------------------------
    uint64_t    get(void) {
        const uint64_t ptr = tail & inQueueBufSizeMask;
        const uint64_t tmp = queue[ ptr ];
        if( 0 != tmp ) {
            queue[ ptr ] = 0;
            ++tail;
        }
        return tmp ;
    }
// --------------------------------------------------------------------
    uint64_t    lng(void) { return head-tail; };
// --------------------------------------------------------------------
    void        clear(void) {
        head = 0;
        tail = 0;
        memset(queue,0, sizeof(queue));
    }
// --------------------------------------------------------------------
private:
    std::atomic<uint64_t>   queue[ inQueueBufSize ];
    uint64_t   head;
    uint64_t   tail;
};



class YaIoInQueueVector
{
public:
    static YaIoInQueueVector& getInstance(void);
    YaIoInQueue     queueControl;       // to the control thread -> GUI
    YaIoInQueue     queueBackend;       // to the backend> mixer filter effect
    YaIoInQueue     queueOscillator;    // to the oscillators


    YaIoInQueueVector(YaIoInQueueVector&)                   = delete;
    YaIoInQueueVector(YaIoInQueueVector const&)             = delete;
    YaIoInQueueVector(YaIoInQueueVector&&)                  = delete;
    void operator=( YaIoInQueueVector& )                    = delete;
    YaIoInQueueVector& operator=(YaIoInQueueVector const&)  = delete;
    YaIoInQueueVector& operator=(YaIoInQueueVector &&)      = delete;

    
protected:
    YaIoInQueueVector() = default;
    
private:
    ~YaIoInQueueVector(){};


};

} // end namespace yacynth


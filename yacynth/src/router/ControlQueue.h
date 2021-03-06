#pragma once
/*
 * Copyright (C) 2017 ist
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
 * File:   ControlQueue.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on October 3, 2017, 6:46 PM
 */

#include <cstdint>
#include <atomic>
#include <string.h>

namespace yacynth {

// TODO test sync with bufsize 2
constexpr uint32_t inQueueBufSizeExp    = 10; // less will be enough -- emptied in < 5 msec
constexpr uint32_t inQueueBufSize       = (1<<inQueueBufSizeExp);
constexpr uint64_t inQueueBufSizeMask   = (inQueueBufSize - 1LL );

class ControlQueue {
public:
    ControlQueue()
    : head(0)
    , tail(0)
    { 
        memset(queue,0, sizeof(queue)); 
    }

    bool put( uint64_t data ) 
    {
        if( 0 == data )
            return true;    // cant put a zero -- no valid data==0
        uint64_t expect = 0;
        if( queue[ head & inQueueBufSizeMask ].compare_exchange_strong( expect, data ) ) { // weak may be enough
            ++head;
            return true;
        }
        return false;
    }

    uint64_t get() 
    {
        const uint64_t ptr = tail & inQueueBufSizeMask;
        const uint64_t tmp = queue[ ptr ]; // load acquire
        if( 0 != tmp ) {
            queue[ ptr ] = 0; // store
            ++tail;
        }
        return tmp;
    }

    uint64_t  lng() 
    { 
        return head-tail; 
    }

    void clear() 
    {
        head = 0;
        tail = 0;
        memset(queue,0, sizeof(queue));
    }

private:
    std::atomic<uint64_t>   queue[ inQueueBufSize ];
    // may need padding but the frequency is low
    uint64_t   head;
    // may need padding but the frequency is low
    uint64_t   tail;
};

// --------------------------------------------------------------------

class ControlQueueVector
{
public:
    static ControlQueueVector& getInstance();
    ControlQueue     queueControl;       // to the control thread -> UDP GUI
    ControlQueue     queueBackend;       // to the backend mixer filter effect - may be not used
    ControlQueue     queueOscillator;    // to the oscillators

    ControlQueueVector(ControlQueueVector&)                   = delete;
    ControlQueueVector(ControlQueueVector const&)             = delete;
    ControlQueueVector(ControlQueueVector&&)                  = delete;
    void operator=( ControlQueueVector& )                     = delete;
    ControlQueueVector& operator=(ControlQueueVector const&)  = delete;
    ControlQueueVector& operator=(ControlQueueVector &&)      = delete;

protected:
    ControlQueueVector() = default;

private:
    ~ControlQueueVector(){};
};

} // end namespace yacynth




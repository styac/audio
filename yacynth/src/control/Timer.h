#pragma once
/*
 * Copyright (C) 2019 ist
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
 * File:   Timer.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on January 7, 2019, 5:13 PM
 */

#include <cstdint>
#include <sys/time.h>
#include <type_traits>
#include <chrono>
#include <ctime>

namespace yacynth {

// #define USE_CHRONO

#ifdef USE_CHRONO

struct NanosecTimer 
{
    int64_t dt() const
    {
        const std::chrono::duration<int64_t, std::nano> dtime = endTime - beginTime;
        return dtime.count();
    }
    void begin() {
        beginTime = std::chrono::high_resolution_clock::now();
    }
    void end() {
        endTime = std::chrono::high_resolution_clock::now();
    }
    std::chrono::time_point<std::chrono::high_resolution_clock> beginTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> endTime;
};
#else
struct NanosecTimer 
{    
    int64_t dt() const
    {
        constexpr int64_t sec2nsec = 1000*1000*1000LL;
        return int32_t( endTime.tv_sec - beginTime.tv_sec ) * sec2nsec + ( endTime.tv_nsec - beginTime.tv_nsec );
    }
    void begin() {
        clock_gettime( CLOCK_REALTIME, &beginTime);
    }
    void end() {
        clock_gettime( CLOCK_REALTIME, &endTime);
    }
    timespec beginTime;
    timespec endTime;
};
#endif


class NanosecCollector
{
    int64_t timeSumm;
    int64_t count;
    
public:
    NanosecCollector()
    : timeSumm(0)
    , count(0)    
    {}

    void clear()
    {
        timeSumm = 0;
        count = 0;
    }
    
    void add( NanosecTimer const& t )
    {
        timeSumm += t.dt();
        ++count;
    }

    void addEnd( NanosecTimer& t )
    {
        t.end();
        timeSumm += t.dt();
        ++count;
    }
    
    int64_t dt() const
    {
        return timeSumm;
    }    
    
    bool checkCount( int64_t limit ) const
    {
        return count < limit;
    }
    
    auto getClear()
    {
        const auto t = timeSumm;
        clear();
        return t;
    }
    
};

} // end namespace yacynth 



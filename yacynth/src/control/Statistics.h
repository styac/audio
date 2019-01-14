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
 * File:   Statistics.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on February 17, 2016, 1:33 PM
 */

#include "control/Timer.h"

#include <cstddef>
#include <cstdint>
#include <vector>
#include <array>
#include <cstdio>
#include <iostream>
#include <cassert>
#include <iterator>
#include <string>
#include <sys/time.h>
#include <type_traits>
#include <chrono>
#include <ctime>

namespace yacynth {

class Statistics {
public:
    Statistics()
    { clear(); setLimit( 1000 ); }; // 1000 microsec - hard limit is 1300 at 48kHz and 64 sample

    enum {
        COUNTER_INNER_LOOP,
        COUNTER_LOW_AMPLITUDE,
        COUNTER_SUSTAIN,
        COUNTER_OUTER_LOOP,
        COUNTER_INNER_LOOP_SUM,
        COUNTER_LOW_AMPLITUDE_SUM,
        COUNTER_SUSTAIN_SUM,
        COUNTER_OUTER_LOOP_SUM,
        COUNTER_ARRAY,
        COUNTER_ARRAY_POS,
        COUNTER_1,       // next
        COUNTER_2,       // next
// ==============================
        COUNTER_SIZE,
    };

    static constexpr std::size_t   counterCount    = COUNTER_SIZE;

    void    clear( void );
    void    setLimit( uint64_t val ) { cycleDeltaLimit = val; };
    void    startTimer( void );
    void    stopTimer( void );

    int64_t dt() const
    {
        return timeInfo.dt();
    }

    NanosecTimer     timeInfo;
    uint64_t    cycleBegin;
    uint64_t    cycleEnd;
    uint64_t    cycleDelta;        //  cycleEnd - cycleBegin
    uint64_t    cycleDeltaLimit;   //  under limit is acceptable

    uint64_t    cycleDeltaMax;     // max of cycleDelta
    uint64_t    cycleDeltaSumm;    // sum of all cycleDelta

    uint64_t    countSumm;         // summ of cycles
    uint64_t    countOverSumm;     // summ of cycles that were higher than limit
    uint64_t    countDisplay;      // display after count
    std::array<uint64_t, counterCount>    cycleCounter;
};

} // end namespace yacynth


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
 * File:   Statistics.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on February 17, 2016, 3:49 PM
 */

#include "Statistics.h"

namespace yacynth {

void Statistics::clear()
{
    cycleBegin      = 0;
    cycleEnd        = 0;
    cycleDelta      = 0;
    cycleDeltaMax   = 0;
    cycleDeltaSumm  = 0;
    countSumm       = 0;
    countOverSumm   = 0;
    countDisplay    = 0;
    cycleDeltaLimit = 64000.0/48.0;

    for( std::size_t i=0; i < cycleCounter.size(); ++i) {
        cycleCounter.at( i ) = 0;
    }
} // end Statistics::clear

// --------------------------------------------------------------------

// TODO refactor use timespec
void Statistics::startTimer( void )
{
    timeval tv;
    gettimeofday(&tv,NULL);
    cycleBegin      = tv.tv_sec*1000000ULL + tv.tv_usec;
    ++countDisplay;
} // end Statistics::startTimer

// --------------------------------------------------------------------

void Statistics::stopTimer( void )
{
    timeval tv;
    gettimeofday(&tv,NULL);
    cycleEnd        = tv.tv_sec*1000000ULL + tv.tv_usec;
    cycleDelta      = cycleEnd - cycleBegin;
    if( cycleDelta > cycleDeltaLimit ) {
        ++countOverSumm;
    }
    cycleDeltaSumm  += cycleDelta;
    if( cycleDelta > cycleDeltaMax ) {
        cycleDeltaMax   =  cycleDelta;
    }
    ++countSumm;
} // end Statistics::stopTimer

// --------------------------------------------------------------------

} // end namespace yacynth



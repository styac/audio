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
 * File:   ThreadControl.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on January 6, 2019, 11:35 AM
 */

#include "control/ThreadControl.h"
#include <sys/resource.h>


//    cpu_set_t cpuset;
//    CPU_ZERO(&cpuset);
//    CPU_SET(i, &cpuset);
//    int rc = pthread_setaffinity_np(threads[i].native_handle(),
//                                    sizeof(cpu_set_t), &cpuset);

namespace yacynth {

ThreadControl::ThreadControl()
: hwconcurrency(std::thread::hardware_concurrency())
{
}

void ThreadControl::assignDiffCore( std::thread& t, int8_t core )
{
    if( core < 0) {
        return;
    }
    uint8_t diffCore = hwconcurrency > 3 ? (core+2) % hwconcurrency : (core+1) % hwconcurrency;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(diffCore, &cpuset);
    pthread_setaffinity_np(t.native_handle(), sizeof(cpu_set_t), &cpuset);
}

int ThreadControl::getThreadParam( std::thread& t )
{
    auto nath = t.native_handle();
    return pthread_getschedparam(nath, &policy, &sch);   
}

int ThreadControl::setThreadParam( std::thread& t ) const
{
    auto nath = t.native_handle();
    return pthread_setschedparam(nath, policy, &sch);   
}

int ThreadControl::setPriority(int value)
{
    pid = getpid();
    return setpriority(PRIO_PROCESS, pid, value); 
}

} // end namespace yacynth 

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
 * File:   ThreadControl.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on January 6, 2019, 11:35 AM
 */

#include <thread>
#include <pthread.h>
#include <unistd.h>

/*
 system settings 
 
--- 1.  

/etc/security/limits.d/99-realtime.conf

content: 

@realtime   -  rtprio     98
@realtime   -  memlock    unlimited

--- 2.

groupadd realtime
usermod -a -G realtime yourUserID

--- 3.

echo 2048 > /sys/class/rtc/rtc0/max_user_freq
echo 2048 > /proc/sys/dev/hpet/max-user-freq
 
cat /sys/class/rtc/rtc0/max_user_freq
cat /proc/sys/dev/hpet/max-user-freq

 */

//info
//  https://news.ycombinator.com/item?id=9386994
//  https://superpowered.com/latency
//  https://source.android.com/devices/audio/latency/measurements
//  https://source.android.com/devices/audio/latency/latency

namespace yacynth {
// num_cpus = std::thread::hardware_concurrency()
struct ThreadControl {
    ThreadControl();
    
    sched_param     sch;
    int             policy; 
    pid_t           pid; 
    uint8_t const   hwconcurrency;
    uint8_t         currentCore;
//    void getThreadCore( std::thread& t );
//    void assignOwnCore( std::thread& t );
    void assignDiffCore( std::thread& t, int8_t core );
    int getThreadParam( std::thread& t );
    int setThreadParam( std::thread& t ) const;    
    int setPriority( int value );
};

} // end namespace yacynth 



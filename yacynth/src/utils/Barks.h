#pragma once

/*
 * Copyright (C) 2017 Istvan Simon -- stevens37 at gmail dot com
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
 * File:   Barks.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on May 10, 2017, 9:26 PM
 */

         
#include    <cstdint>
#include    <cmath>
#include    <algorithm>

namespace tables {

struct Barks {
    static constexpr std::size_t  BarksSize = 24;
    static constexpr float  samplingFrequency = 48000.0;

    Barks() 
    {
    }
    
    static const uint16_t barksBw_Hz[ BarksSize ];
    static const uint16_t barksCenter_Hz[ BarksSize ];

    //static const float barksBw_fc[ BarksSize ];         // TODO
    //static const float barksCenter_fc[ BarksSize ];     // TODO
    
    //static const int32_t barksBw_ycent[ BarksSize ];    // TODO
    //static const int32_t barksCenter_ycent[ BarksSize ];// TODO
};



} // end namespace tables


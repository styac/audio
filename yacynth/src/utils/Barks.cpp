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
 * File:   Barks.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on May 10, 2017, 9:26 PM
 */

// https://ccrma.stanford.edu/~jos/bbt/Bark_Frequency_Scale.html

#include "Barks.h"

namespace tables {

const uint16_t Barks::barksCenter_Hz[ BarksSize ] = {
    50,     150,    250,    350, 
    450,    570,    700,    840, 
    1000,   1170,   1370,   1600, 
    1850,   2150,   2500,   2900, 
    3400,   4000,   4800,   5800, 
    7000,   8500,   10500,  13500        
};

const uint16_t Barks::barksBw_Hz[ BarksSize ] = {
    100,    200,    300,    400, 
    510,    630,    770,    920, 
    1080,   1270,   1480,   1720, 
    2000,   2320,   2700,   3150, 
    3700,   4400,   5300,   6400, 
    7700,   9500,   12000,  15500
};

//const  float Barks::barksBw_fc[ BarksSize ];
//const  float Barks::barksCenter_fc[ BarksSize ];

//const  int32_t Barks::barksBw_ycent[ BarksSize ];
//const  int32_t Barks::barksCenter_ycent[ BarksSize ];


} // end namespace tables

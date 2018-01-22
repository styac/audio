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
 * File:   Tables.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on February 6, 2016, 5:15 PM
 */
#if 0
#include    "yacynth_globals.h"
#include    "v4.h"
#include    "utils/Fastexp.h"

#include    <iostream>
#include    <array>

namespace yacynth {
// --------------------------------------------------------------------
//
// input:
// nnnnnnnn nnnnnnnn nnnnnnnn nnnnnnnn
// 00eeeeee iiiiiiii iiiiiiii xxxxxxxx
//
// directly calculates deltaFi
// kills too high and low frequencies result 0 -> stop oscillator
constexpr int32_t refOctave = 0x1000000;
constexpr   int precMultExp = 60;

inline uint32_t ycent2deltafi( uint32_t base,  int32_t delta )
{
    if( ( ref19900ycent < base ) || ( ref0_01ycent > base ) )
       return 0;   //  nuke the high and  low freqs
    if( refOctave < delta  ) {
        delta = refOctave;
    }
    if( -refOctave > delta  ) {
        delta = -refOctave;
    }
    const uint32_t  logv = base + delta;
    const uint16_t  ind = logv >> 8;
    const uint16_t  xpo = logv >> 24;    
    const uint64_t  y1 = tables::exp2Table[ ind ];
    const uint64_t  dy = (( tables::exp2Table[ ind + 1 ] - y1 ) * uint8_t( logv )) >> 8;        
    const uint64_t  rs = ( y1 + dy ) >> ( precMultExp - xpo );
    return rs ;
} // end ycent2deltafi( uint32_t logv )


} // end namespace yacynth
#endif


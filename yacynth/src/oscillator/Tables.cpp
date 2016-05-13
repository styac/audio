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
 * File:   Tables.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on February 6, 2016, 5:15 PM
 */

#include    "Tables.h"

#include    <cstdlib>
#include    <iostream>
#include    <limits>

namespace yacynth {

uint64_t    exp2Table[      waveTableSize + 1   ];

// --------------------------------------------------------------------

void    fillExp2Table( void )
{
    constexpr long double   dexp   = 1.0L / double(waveTableSize);
    constexpr uint64_t      norm   = 1LL << precMultExp;
    for( auto i = 0; i <= waveTableSize; ++i ) {    // +1 for the interpolation
        exp2Table[ i ] = uint64_t( std::llround( std::pow( 2.0L, i * dexp ) * norm ) );
    }
} // end fillCent2freq

} // end namespace yacynth



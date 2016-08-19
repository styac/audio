/*
 * Copyright (C) 2016 Istvan Simon
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
 * File:   BaseOscillator.cpp
 * Author: Istvan Simon
 *
 * Created on March 26, 2016, 10:37 AM
 */

#include    "../oscillator/BaseOscillator.h"

#include    <iostream>

namespace yacynth {

IntegratorOscillator::IntegratorOscillator()
:   phaseDeltaRef( primes )
{
    quarter = 0;
    phase = 0;
    // ~4 Hz
//    phaseDeltaN = 47;
//    phaseDeltaP = 59;
//    phaseDelta01 = 0;

    // 40 -> 20 Hz

//    phaseDelta0 = 137;
//    phaseDelta1 = 97;
//    phaseDelta2 = 123;
//    phaseDelta3 = 113;
//    phaseDelta0 = 43;
//    phaseDelta1 = 47;
//    phaseDelta2 = 59;
//    phaseDelta3 = 53;
    depth = 2;
    decay = 100;
}


int64_t IntegratorOscillator::mod( const int64_t in )
{
    constexpr int cutpeek = 11;
    if( 0x0FF == quarter ) {
        if( decay ) {
            amplDecay = (( decay * in ) >> 28 ) + 1;
            return -amplDecay;
        }
        return 0;
    }
    if( --phase <= 0 )  {

        switch( quarter++ & 0x03 ) {
        case 0:
            phaseDelta = phaseDeltaRef.table[ 37 + (( quarter >> 2 ) & 15 ) ] + ( (quarter >>1) & 7);
            amplDelta = in / ( ( 1<<cycle ) * phaseDelta * depth );
            if( decay ) amplDecay = ( ( decay * in ) >> 28 ) + 1;
            else amplDecay = 0;
            phase = phaseDelta;
            retAmpl = amplDelta;
            break;
        case 1:
            phase = phaseDelta;
            retAmpl = -amplDelta;
            break;
        case 2:
            phaseDelta = phaseDeltaRef.table[ 41 + (( quarter >> 2 ) & 15 ) ] + ( (quarter >>1) & 7);
            amplDelta = in / ( ( 1<<cycle ) * phaseDelta * depth );
            if( decay ) amplDecay = ( ( decay * in ) >> 28 ) + 1;
            else amplDecay = 0;
            phase = phaseDelta;
            retAmpl = amplDelta;
            break;
        case 3:
            phase = phaseDelta;
            retAmpl = -amplDelta;
            quarter &= 0x7F;
            break;
        }
    }
    return retAmpl - amplDecay;
}

} // end namespace yacynth

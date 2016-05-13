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
 * File:   Echo.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on March 7, 2016, 2:05 PM
 */

#include    "Echo.h"

namespace yacynth {
// --------------------------------------------------------------------
void Echo::testvect(void)
{
    tapOutputVector.fill( echoTapsOut() );
    tapFeedbackVector.fill( echoTapsFeedback() );
//    tapOutputVector.list();
//    tapFeedbackVector.list();
}
// --------------------------------------------------------------------
void Echo::sprocess_0( void * thp )
{
    (static_cast<Echo*>(thp))->process();
}
// --------------------------------------------------------------------
bool Echo::fill( std::stringstream& ser )
{
    return false;
}

// --------------------------------------------------------------------
void Echo::process(void)
{
//    const EIObuffer& inp = *this->inp;

    out.copy( *inp );
    const uint16_t loopCountF = std::min( tapFeedbackVector.usedTapCount, tapFeedbackVector.vectorSize );
    for( auto i = 0u; i < loopCountF; ++i ) {
        auto& tv = tapFeedbackVector.dtvec[i];
        if( tv.isValid() ) {
            uint32_t ind = tv.delaySrcH + sectionSize;   // index is negated
            for( auto i = 0u; i < sectionSize; ++i ) {
                delay.multAddMixStereoNoisefloor<-24>( --ind, tv.coeff, out.channelA[i], out.channelB[i] );
            }
        }
    }
    delay.pushSection( out.channelA, out.channelB );
    if( inMix ) {
        out.copy( *inp );
    } else {
        out.clear();
    }
    const uint16_t loopCountO = std::min( tapOutputVector.usedTapCount, tapOutputVector.vectorSize );
    for( auto i = 0u; i < loopCountO; ++i ) {
        auto& tv = tapOutputVector.dtvec[i];
        if( tv.isValid() ) {
            uint32_t ind = tv.delaySrcH + sectionSize;   // index is negated
            for( auto i = 0u; i < sectionSize; ++i ) {
                delay.multAddMixStereo( --ind, tv.coeff, out.channelA[i], out.channelB[i] );
            }
        }
    }
} // end Echo:process
// --------------------------------------------------------------------


} // end namespace yacynth



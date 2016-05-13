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
 * File:   Comb.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on March 8, 2016, 9:15 AM
 */

#include "Comb.h"

namespace yacynth {
// --------------------------------------------------------------------    
void Comb::feedforward( const EIObuffer& inp, EIObuffer& out )
{
    const float multDelay   = tap.multDelay;
    const float multOut     = tap.multOut;   
    const uint32_t dlind32  = tap.dlind32; 

        const float * chnIAp = inp.channelA;
        float * chnOAp = out.channelA;
        const float * chnIBp = inp.channelB;
        float * chnOBp = out.channelB;
        for( uint16_t sami = 0; sami < sectionSize; ++sami ) {
            const uint32_t dind = delay.getIndex( dlind32 );
            const float csmplA  = *chnIAp++;
            const float csmplB  = *chnIBp++;
            //
            //      vectoring
            // delay.channelA[ dind ], delay.channelB[ dind ], csmplA, csmplB
            //      mult
            // multDelay, multDelay, multOut, multOut
            //            
            *chnOAp++ = delay.channelA[ dind ] * multDelay + csmplA * multOut;
            *chnOBp++ = delay.channelB[ dind ] * multDelay + csmplB * multOut;
            delay.push( csmplA, csmplB );
        }

} // end Comb::feedforward
// --------------------------------------------------------------------
void Comb::feedback( const EIObuffer& inp, EIObuffer& out )
{
    const float multDelay   = tap.multDelay;
    const float multOut     = tap.multOut;    
    const uint32_t dlind32  = tap.dlind32; 
    

        const float * chnIAp  = inp.channelA;
        float * chnOAp  = out.channelA;
        const float * chnIBp  = inp.channelB;
        float * chnOBp  = out.channelB;
        for( uint16_t sami = 0; sami < sectionSize; ++sami ) {
            const uint32_t dind = delay.getIndex( dlind32 );
            const float csmplA  = *chnIAp++;
            const float csmplB  = *chnIBp++;
            const float tmpA    = delay.channelA[ dind ] * multDelay + csmplA;
            const float tmpB    = delay.channelB[ dind ] * multDelay + csmplB;
            *chnOAp++ = tmpA * multOut;
            *chnOBp++ = tmpB * multOut;
            delay.push( tmpA, tmpB );
        }

} // end Comb::feedback
// --------------------------------------------------------------------
void Comb::allpass( const EIObuffer& inp, EIObuffer& out )
{
    const float multDelay   = tap.multDelay;
    const float multOut     = tap.multOut;
    const uint32_t dlind32  = tap.dlind32; 

        const float * chnIAp  = inp.channelA;
        float * chnOAp  = out.channelA;
        const float * chnIBp  = inp.channelB;
        float * chnOBp  = out.channelB;
        for( uint16_t sami = 0; sami < sectionSize; ++sami ) {
            const uint32_t dind = delay.getIndex( dlind32 );
            const float csmplA  = *chnIAp++;
            const float csmplB  = *chnIBp++;
            const float dlA     = delay.channelA[ dind ];
            const float dlB     = delay.channelB[ dind ];
            const float tmpA    = dlA * multDelay + csmplA;
            const float tmpB    = dlB * multDelay + csmplB;
            *chnOAp++ = csmplA * multOut + dlA;            
            *chnOBp++ = csmplB * multOut + dlB;
            delay.push( tmpA, tmpB );            
        }
 
} // end Comb::allpass
// --------------------------------------------------------------------
void Comb::noop( const EIObuffer& inp, EIObuffer& out )
{

        const float * chnIAp = inp.channelA;
        float * chnOAp = out.channelA;
        const float * chnIBp = inp.channelB;
        float * chnOBp = out.channelB;
        for( uint16_t sami = 0; sami < sectionSize; ++sami ) {
            *chnOAp++ = *chnIAp++;
            *chnOBp++ = *chnIBp++;
        }
} // end Comb::noop
// --------------------------------------------------------------------    
} // end namespace yacynth



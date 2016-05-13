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
 * File:   CombInterpolated.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on March 8, 2016, 10:23 PM
 */

/*



 double CosineInterpolate(
   double y1,double y2,
   double mu)
{
   double mu2;

   mu2 = (1-cos(mu*PI))/2;
   return(y1*(1-mu2)+y2*mu2);
}
 *
 *
 *
 *


double CubicInterpolate(
   double y0,double y1,
   double y2,double y3,
   double mu)
{
   double a0,a1,a2,a3,mu2;

   mu2 = mu*mu;
   a0 = y3 - y2 - y0 + y1;
   a1 = y0 - y1 - a0;
   a2 = y2 - y0;
   a3 = y1;

   return(a0*mu*mu2+a1*mu2+a2*mu+a3);
}

 *
 *
Clipping without branching

Type : Min, max and clip
References : Posted by Laurent de Soras

Notes :
It may reduce accuracy for small numbers. I.e. if you clip to [-1; 1], fractional part of the result will be quantized to 23 bits (or more, depending on the bit depth of the temporary results). Thus, 1e-20 will be rounded to 0. The other (positive) side effect is the denormal number elimination.

Code :
float max (float x, float a)
{
   x -= a;
   x += fabs (x);
   x *= 0.5;
   x += a;
   return (x);
}

float min (float x, float b)
{
   x = b - x;
   x += fabs (x)
   x *= 0.5;
   x = b - x;
   return (x);
}

float clip (float x, float a, float b)
{
   x1 = fabs (x-a);
   x2 = fabs (x-b);
   x = x1 + (a+b);
   x -= x2;
   x *= 0.5;
   return (x);
}
https://www.dsprelated.com/freebooks/pasp/Delay_Line_Interpolation_Summary.html
 *

 */


/*

 https://www.dsprelated.com/freebooks/pasp/Lagrange_Interpolation.html#sec:laginterp

 */

#include    "CombInterpolated.h"

namespace yacynth {

CombInterpolated::CombInterpolated( std::size_t length )
:   Comb( length )
{
    lfo.set( freq2deltaPhase( 1.0 ), Lfo::TRIANGLE, 7 );
};
// --------------------------------------------------------------------
CombInterpolated::CombInterpolated( std::size_t length, uint32_t dlind32, float multDelay, float multOut, uint16_t depthExp )
:   Comb( length, dlind32, multDelay, multOut )
{
    lfo.set( freq2deltaPhase( 0.25 ), Lfo::SIN, depthExp );
};
// --------------------------------------------------------------------
void CombInterpolated::feedforwardInterpolated(  const EIObuffer& inp, EIObuffer& out )
{
    const float multDelay   = tap.multDelay;
    const float multOut     = tap.multOut;
    const uint64_t dlind64  = tap.dlind64;

        const float * chnIAp = inp.channelA;
        float * chnOAp = out.channelA;
        const float * chnIBp = inp.channelB;
        float * chnOBp = out.channelB;
        for( uint16_t sami = 0; sami < sectionSize; ++sami, lfo.inc() ) {
            const float csmplA  = *chnIAp++;
            const float csmplB  = *chnIBp++;
            const float dlsmplA = delay.getInterpolatedA( dlind64 + lfo.get( Lfo::PHASE0 ));
            const float dlsmplB = delay.getInterpolatedB( dlind64 + lfo.get( Lfo::PHASE1 ));
            *chnOAp++ = dlsmplA * multDelay + csmplA * multOut;
            *chnOBp++ = dlsmplB * multDelay + csmplB * multOut;
            delay.push( csmplA, csmplB );
        }
  
} // end CombInterpolated::feedforwardInterpolated
// --------------------------------------------------------------------
void CombInterpolated::feedbackInterpolated(  const EIObuffer& inp, EIObuffer& out )
{
    const float multDelay   = tap.multDelay;
    const float multOut     = tap.multOut;
    const uint64_t dlind64  = tap.dlind64;

        const float * chnIAp = inp.channelA;
        float * chnOAp = out.channelA;
        const float * chnIBp = inp.channelB;
        float * chnOBp = out.channelB;
        for( uint16_t sami = 0; sami < sectionSize; ++sami, lfo.inc() ) {
            const float csmplA  = *chnIAp++;
            const float csmplB  = *chnIBp++;
            const float tmpA    = delay.getInterpolatedA( dlind64 + lfo.get( Lfo::PHASE0 ))
                                    * multDelay + csmplA;
            const float tmpB    = delay.getInterpolatedB( dlind64 + lfo.get( Lfo::PHASE1 ))
                                    * multDelay + csmplB;
            *chnOAp++ = tmpA * multOut;
            *chnOBp++ = tmpB * multOut;
            delay.push( tmpA, tmpB );
        }
 
} // end CombInterpolated::feedbackInterpolated
// --------------------------------------------------------------------

void CombInterpolated::allpassInterpolated(  const EIObuffer& inp, EIObuffer& out )
{
    const float multDelay   = tap.multDelay;
    const float multOut     = tap.multOut;
    const uint64_t dlind64  = tap.dlind64;

        const float * chnIAp = inp.channelA;
        float * chnOAp = out.channelA;
        const float * chnIBp = inp.channelB;
        float * chnOBp = out.channelB;
        for( uint16_t sami = 0; sami < sectionSize; ++sami, lfo.inc() ) {
            const float dlA     = delay.getInterpolatedA( dlind64 + lfo.get( Lfo::PHASE0 ));
            const float csmplA  = *chnIAp++;
            const float dlB     = delay.getInterpolatedB( dlind64 + lfo.get( Lfo::PHASE1 ));
            const float csmplB  = *chnIBp++;
            const float tmpA    = dlA * multDelay + csmplA;
            const float tmpB    = dlB * multDelay + csmplB;
            *chnOAp++ = csmplA * multOut + dlA;
            *chnOBp++ = csmplB * multOut + dlB;
            delay.push( tmpA, tmpB );
        }

} // end CombInterpolated::allpassInterpolated
// --------------------------------------------------------------------
void CombInterpolated::barberpole(  const EIObuffer& inp, EIObuffer& out )
{
    const float multDelay   = tap.multDelay;
    const float multOut     = tap.multOut;
    const uint64_t dlind64  = tap.dlind64;

        const float * chnIAp = inp.channelA;
        float * chnOAp = out.channelA;
        const float * chnIBp = inp.channelB;
        float * chnOBp = out.channelB;
        for( uint16_t sami = 0; sami < sectionSize; ++sami, lfo.inc() ) {
            const float dlA1    = delay.getInterpolatedA( dlind64 + ( lfo.get( Lfo::PHASE0, Lfo::SAW )))
                                * lfo.getFloatOffs( Lfo::PHASE1, Lfo::SIN );
            const float dlA     = delay.getInterpolatedA( dlind64 + ( lfo.get( Lfo::PHASE2, Lfo::SAW )))
                                * lfo.getFloatOffs( Lfo::PHASE3, Lfo::SIN ) + dlA1;
            const float dlB1    = delay.getInterpolatedB( dlind64 + ( lfo.get( Lfo::PHASE1, Lfo::SAW )))
                                * lfo.getFloatOffs( Lfo::PHASE2, Lfo::SIN );
            const float dlB     = delay.getInterpolatedB( dlind64 + ( lfo.get( Lfo::PHASE3, Lfo::SAW )))
                                * lfo.getFloatOffs( Lfo::PHASE0, Lfo::SIN ) + dlB1;

            const float csmplA  = *chnIAp++;
            const float csmplB  = *chnIBp++;
            const float tmpA    = dlA * multDelay + csmplA;
            const float tmpB    = dlB * multDelay + csmplB;
            *chnOAp++ = csmplA * multOut + dlA;
            *chnOBp++ = csmplB * multOut + dlB;
            delay.push( tmpA, tmpB );
        }

} // end CombInterpolated::allpassInterpolated

// --------------------------------------------------------------------
} // end namespace yacynth



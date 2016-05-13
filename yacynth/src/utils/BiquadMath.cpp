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
 * File:   BiquadMath.cpp
 * Author: Istvan Simon
 *
 * Created on March 14, 2016, 12:47 AM
 */

// see: http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt

#include    "BiquadMath.h"

namespace filter {
//
// this calculator negates the a1,a2
// the implementation DO NOT SUBSTRACT
//
// fcrel = f / fs
//
// gainLIn NOT decibel!
//
// Q is used as Q -- precalculation may be needed
//
// --------------------------------------------------------------------
void BiquadParam::eval( type_t t, Coeffs& filt, const float fcrel, const float q, const float gainLin )
{
    switch( t ) {
    case RBJ_LOWPASS:       evalRBJLowPass(    filt, fcrel, q ); return;
    case RBJ_HIGHPASS:      evalRBJHighPass(   filt, fcrel, q ); return;
    case RBJ_BANDPASSCS:    evalRBJBandPassCS( filt, fcrel, q ); return;
    case RBJ_BANDPASSCP:    evalRBJBandPassCP( filt, fcrel, q ); return;
    case RBJ_NOTCH:         evalRBJNotch(      filt, fcrel, q ); return;
    case RBJ_ALLPASS:       evalRBJAllpass(    filt, fcrel, q ); return;
    case RBJ_PEEKING:       evalRBJPeeking(    filt, fcrel, q, gainLin ); return;
    case RBJ_LOWSHELF:      evalRBJLowShelf(   filt, fcrel, q, gainLin ); return;
    case RBJ_HIGHSHELF:     evalRBJHigShelf(   filt, fcrel, q, gainLin ); return;
    };
} // end BiquadParam::eval
// --------------------------------------------------------------------
void BiquadParam::evalRBJLowPass( Coeffs& filt, const float fcrel, const float q, const float zz )
{
    const float w0      = (( fcrel < FCMIN ? FCMIN : ( fcrel > FCMAX ? FCMAX : fcrel ))) * PI2;
    const float tt      = std::cos( w0 ); // 12000.Hz @ 48000 sampling
    const float cosw0   = std::abs( tt ) < 1.0e-7 ? 0.0 : tt;
    const float sinw0   = std::sin( w0 );
    const float oneq    = 1.0 / ( q < QMIN ? QMIN  : ( q > QMAX ? QMAX : q ) );
    const float alpha   = 0.5 * sinw0 * oneq;
    const float diva0   = 1.0 / ( 1.0 + alpha );
    filt.b1             = diva0 - cosw0 * diva0;
    filt.b0 = filt.b2   = filt.b1 * 0.5;
    filt.a1             = 2.0 * cosw0 * diva0;
    filt.a2             = -( diva0 - alpha * diva0 ) ;
} // end BiquadMath::evalLowPass
// --------------------------------------------------------------------
void BiquadParam::evalRBJHighPass( Coeffs& filt, const float fcrel, const float q, const float zz )
{
    const float w0      = (( fcrel < FCMIN ? FCMIN : ( fcrel > FCMAX ? FCMAX : fcrel ))) * PI2;
    const float tt      = std::cos( w0 ); // 12000.Hz @ 48000 sampling
    const float cosw0   = std::abs( tt ) < 1.0e-7 ? 0.0 : tt;
    const float sinw0   = std::sin( w0 );
    const float oneq    = 1.0 / ( q < QMIN ? QMIN  : ( q > QMAX ? QMAX : q ) );
    const float alpha   = 0.5 * sinw0 * oneq;
    const float diva0   = 1.0 / ( 1.0 + alpha );
    filt.b1             = -( diva0 + cosw0 * diva0 );
    filt.b0 = filt.b2   = -filt.b1 * 0.5;
    filt.a1             = 2.0 * cosw0 * diva0;
    filt.a2             = -( diva0 - alpha * diva0 );
} // end BiquadMath::evalHighPass
// --------------------------------------------------------------------
void BiquadParam::evalRBJBandPassCS( Coeffs& filt, const float fcrel, const float q, const float zz )
{
    const float w0      = (( fcrel < FCMIN ? FCMIN : ( fcrel > FCMAX ? FCMAX : fcrel ))) * PI2;
    const float tt      = std::cos( w0 ); // 12000.Hz @ 48000 sampling
    const float cosw0   = std::abs( tt ) < 1.0e-7 ? 0.0 : tt;
    const float sinw0   = std::sin( w0 );
    const float oneq    = 1.0 / ( q < QMIN ? QMIN  : ( q > QMAX ? QMAX : q ) );
    const float alpha   = 0.5 * sinw0 * oneq;
    const float diva0   = 1.0 / ( 1.0 + alpha );
    filt.b1             = 0.0;
    filt.b0             = sinw0 * 0.5 * diva0;
    filt.b2             = -filt.b0;
    filt.a1             = 2.0 * cosw0 * diva0;
    filt.a2             = -( diva0 - alpha * diva0 ) ;
} // end BiquadMath::evalBandPass
// --------------------------------------------------------------------
void BiquadParam::evalRBJBandPassCP( Coeffs& filt, const float fcrel, const float q, const float zz )
{
    const float w0      = (( fcrel < FCMIN ? FCMIN : ( fcrel > FCMAX ? FCMAX : fcrel ))) * PI2;
    const float tt      = std::cos( w0 ); // 12000.Hz @ 48000 sampling
    const float cosw0   = std::abs( tt ) < 1.0e-7 ? 0.0 : tt;
    const float sinw0   = std::sin( w0 );
    const float oneq    = 1.0 / ( q < QMIN ? QMIN  : ( q > QMAX ? QMAX : q ) );
    const float alpha   = 0.5 * sinw0 * oneq;
    const float diva0   = 1.0 / ( 1.0 + alpha );
    filt.b1             = 0.0;
    filt.b0             = alpha * diva0;
    filt.b2             = -filt.b0;
    filt.a1             = 2.0 * cosw0 * diva0;
    filt.a2             = -( diva0 - alpha * diva0 ) ;
} // end BiquadMath::evalBandPass
// --------------------------------------------------------------------
void BiquadParam::evalRBJNotch( Coeffs& filt, const float fcrel, const float q, const float zz )
{
    const float w0      = (( fcrel < FCMIN ? FCMIN : ( fcrel > FCMAX ? FCMAX : fcrel ))) * PI2;
    const float tt      = std::cos( w0 ); // 12000.Hz @ 48000 sampling
    const float cosw0   = std::abs( tt ) < 1.0e-7 ? 0.0 : tt;
    const float sinw0   = std::sin( w0 );
    const float oneq    = 1.0 / ( q < QMIN ? QMIN  : ( q > QMAX ? QMAX : q ) );
    const float alpha   = 0.5 * sinw0 * oneq;
    const float diva0   = 1.0 / ( 1.0 + alpha );
    filt.b0 = filt.b2   = diva0;
    filt.b1             = -2.0 * cosw0 * diva0;
    filt.a1             = -filt.b1 ;
    filt.a2             = -( diva0 - alpha * diva0 ) ;
} // end BiquadMath::evalNotch
// --------------------------------------------------------------------
void BiquadParam::evalRBJAllpass( Coeffs& filt, const float fcrel, const float q, const float zz )
{
    const float w0      = (( fcrel < FCMIN ? FCMIN : ( fcrel > FCMAX ? FCMAX : fcrel ))) * PI2;
    const float tt      = std::cos( w0 ); // 12000.Hz @ 48000 sampling
    const float cosw0   = std::abs( tt ) < 1.0e-7 ? 0.0 : tt;
    const float sinw0   = std::sin( w0 );
    const float oneq    = 1.0 / ( q < QMIN ? QMIN  : ( q > QMAX ? QMAX : q ) );
    const float alpha   = 0.5 * sinw0 * oneq;
    const float diva0   = 1.0 / ( 1.0 + alpha );
    filt.b2             = diva0;
    filt.b0             = -2.0 * cosw0 * diva0;
    filt.a1             = -filt.b0;
    filt.a2             = -( diva0 - alpha * diva0 );
} // end BiquadMath::evalAllpass
// --------------------------------------------------------------------
void BiquadParam::evalRBJPeeking( Coeffs& filt, const float fcrel, const float q, const float gain )
{
    const float w0      = (( fcrel < FCMIN ? FCMIN : ( fcrel > FCMAX ? FCMAX : fcrel ))) * PI2;
    const float tt      = std::cos( w0 ); // 12000.Hz @ 48000 sampling
    const float cosw0   = std::abs( tt ) < 1.0e-7 ? 0.0 : tt;
    const float sinw0   = std::sin( w0 );
    const float oneq    = 1.0 / ( q < QMIN ? QMIN  : ( q > QMAX ? QMAX : q ) );
    const float biga    = gain < GMIN ? GMIN  : ( gain > GMAX ? GMAX : gain );
    const float alpha   = 0.5 * sinw0 * oneq;
    const float alpdv   = alpha / biga;
    const float alpml   = alpha * biga;
    const float diva0   = 1.0 / ( 1.0 + alpdv );
    filt.b0             = ( 1.0 + alpml )   * diva0;
    filt.b1             = -2.0 * cosw0      * diva0;
    filt.a1             = -filt.b1;
    filt.a2             = -( diva0 - alpdv ) * diva0;
    filt.b2             =  ( 1.0 - alpml )   * diva0;
} // end BiquadMath::evalPeeking
// --------------------------------------------------------------------
void BiquadParam::evalRBJLowShelf( Coeffs& filt, const float fcrel, const float q, const float gain )
{
    const float w0      = (( fcrel < FCMIN ? FCMIN : ( fcrel > FCMAX ? FCMAX : fcrel ))) * PI2;
    const float tt      = std::cos( w0 ); // 12000.Hz @ 48000 sampling
    const float cosw0   = std::abs( tt ) < 1.0e-7 ? 0.0 : tt;
    const float sinw0   = std::sin( w0 );
    const float oneq    = 1.0 / ( q < QMIN ? QMIN  : ( q > QMAX ? QMAX : q ) );
    const float biga    = gain < GMIN ? GMIN  : ( gain > GMAX ? GMAX : gain );
    const float alpha   = sinw0 * oneq * std::sqrt( biga );
    const float aP1     = biga + 1.0;
    const float aM1     = biga - 1.0;
    const float aP1c    = aP1 * cosw0;
    const float aM1c    = aM1 * cosw0;
    const float diva0   = 1.0 / ( aP1 + aM1c + alpha );
    filt.b1             = 2.0 * biga * ( aM1 - aP1c         ) * diva0;
    filt.b0             =       biga * ( aP1 - aM1c + alpha ) * diva0;
    filt.b2             =       biga * ( aP1 - aM1c - alpha ) * diva0;
    filt.a1             =        2.0 * ( aM1 + aP1c         ) * diva0;
    filt.a2             =             -( aP1 + aM1c - alpha ) * diva0;
} // end BiquadMath::evalLowShelf
// --------------------------------------------------------------------
void BiquadParam::evalRBJHigShelf( Coeffs& filt, const float fcrel, const float q, const float gain )
{
    const float w0      = (( fcrel < FCMIN ? FCMIN : ( fcrel > FCMAX ? FCMAX : fcrel ))) * PI2;
    const float tt      = std::cos( w0 ); // 12000.Hz @ 48000 sampling
    const float cosw0   = std::abs( tt ) < 1.0e-7 ? 0.0 : tt;
    const float sinw0   = std::sin( w0 );
    const float oneq    = 1.0 / ( q < QMIN ? QMIN  : ( q > QMAX ? QMAX : q ) );
    const float biga    = gain < GMIN ? GMIN  : ( gain > GMAX ? GMAX : gain );
    const float alpha   = sinw0 * oneq * std::sqrt( biga );
    const float aP1     = biga + 1.0;
    const float aM1     = biga - 1.0;
    const float aP1c    = aP1 * cosw0;
    const float aM1c    = aM1 * cosw0;
    const float diva0   = 1.0 / ( aP1 - aM1c + alpha );
    filt.b1             = -2.0 * biga * ( aM1 + aP1c         ) * diva0;
    filt.b0             =        biga * ( aP1 + aM1c + alpha ) * diva0;
    filt.b2             =        biga * ( aP1 + aM1c - alpha ) * diva0;
    filt.a1             =        -2.0 * ( aM1 - aP1c         ) * diva0;
    filt.a2             =              -( aP1 - aM1c - alpha ) * diva0;
} // end BiquadMath::evalHigShelf
// --------------------------------------------------------------------
} // end namespace filter
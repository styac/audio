#pragma once

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
 * File:   BiquadMath.h
 * Author: Istvan Simon
 *
 * Created on March 14, 2016, 12:47 AM
 */

#include    "Biquad.h"
#include    <cmath>

/*
//init
mX1 = 0;
mX2 = 0;
mY1 = 0;
mY2 = 0;
pi = 22/7;

//coefficients
cutoff = cutoff_slider;
res = res_slider;

cutoff = 2 * cutoff_slider / srate;
res = pow(10, 0.05 * -res_slider);
k = 0.5 * res * sin(pi * cutoff);
c1 = 0.5 * (1 - k) / (1 + k);
c2 = (0.5 + c1) * cos(pi * cutoff);
c3 = (0.5 + c1 - c2) * 0.25;

mA0 = 2 * c3;
mA1 = 2 * 2 * c3;
mA2 = 2 * c3;
mB1 = 2 * -c2;
mB2 = 2 * c1;

//loop
output = mA0*input + mA1*mX1 + mA2*mX2 - mB1*mY1 - mB2*mY2;

mX2 = mX1;
mX1 = input;
mY2 = mY1;
mY1 = output;
*/


/* TODO --- precalc function


    Fs (the sampling frequency)

    f0 ("wherever it's happenin', man."  Center Frequency or
        Corner Frequency, or shelf midpoint frequency, depending
        on which filter type.  The "significant frequency".)

    dBgain (used only for peaking and shelving filters)

    Q (the EE kind of definition, except for peakingEQ in which A*Q is
        the classic EE Q.  That adjustment in definition was made so that
        a boost of N dB followed by a cut of N dB for identical Q and
        f0/Fs results in a precisely flat unity gain filter or "wire".)

     _or_ BW, the bandwidth in octaves (between -3 dB frequencies for BPF
        and notch or between midpoint (dBgain/2) gain frequencies for
        peaking EQ)

     _or_ S, a "shelf slope" parameter (for shelving EQ only).  When S = 1,
        the shelf slope is as steep as it can be and remain monotonically
        increasing or decreasing gain with frequency.  The shelf slope, in
        dB/octave, remains proportional to S for all other values for a
        fixed f0/Fs and dBgain.



Then compute a few intermediate variables:

    A  = sqrt( 10^(dBgain/20) )
       =       10^(dBgain/40)     (for peaking and shelving EQ filters only)

    w0 = 2*pi*f0/Fs

    cos(w0)
    sin(w0)

    alpha = sin(w0)/(2*Q)                                       (case: Q)
          = sin(w0)*sinh( ln(2)/2 * BW * w0/sin(w0) )           (case: BW)
          = sin(w0)/2 * sqrt( (A + 1/A)*(1/S - 1) + 2 )         (case: S)

        FYI: The relationship between bandwidth and Q is
             1/Q = 2*sinh(ln(2)/2*BW*w0/sin(w0))     (digital filter w BLT)
        or   1/Q = 2*sinh(ln(2)/2*BW)             (analog filter prototype)

        The relationship between shelf slope and Q is
             1/Q = sqrt((A + 1/A)*(1/S - 1) + 2)

    2*sqrt(A)*alpha  =  sin(w0) * sqrt( (A^2 + 1)*(1/S - 1) + 2*A )
        is a handy intermediate variable for shelving EQ filters.



 */
// fcrel = f_cutoff / f_sampling
// fc = 0.001 .. 0.4
//
//
//
/*

 * freq warping
 *

" // pre-warp the cutoff- these are bilinear-transform filters
! float wd = 2*pi*m_fFc;
! float T = 1/(float)m_nSampleRate;
! float wa = (2/T)*tan(wd*T/2);
! float g = wa*T/2;
! // big combined value
! float G = g/(1.0 + g);
! // 2-channel output buffer for LPF
! float LP[2];
! // do the filter, see VA book p. 46
! //
! // form sub-node value v(n)
! float v = (pInput[0] - m_fZ1[0])*G;
! // form output of node + register
! LP[0] = v + m_fZ1[0];
! // setup for next time through
! m_fZ1[0] = LP[0] + v;

 */

namespace filter {

struct   BiquadParam {
    static constexpr float PI       = 3.141592653589793238462643383279502884197;
    static constexpr float PI2      = PI * 2.0;
    static constexpr float FCMIN    = 0.0001;
    static constexpr float FCMAX    = 0.42;
    static constexpr float QMIN     = 0.01;
    static constexpr float QMAX     = 100.0;
    static constexpr float GMIN     = 0.001;
    static constexpr float GMAX     = 100000.0;

    enum type_t {
        RBJ_LOWPASS,
        RBJ_HIGHPASS,
        RBJ_BANDPASSCS,
        RBJ_BANDPASSCP,
        RBJ_PEEKING,
        RBJ_LOWSHELF,
        RBJ_HIGHSHELF,
        RBJ_NOTCH,
        RBJ_ALLPASS,
    };

    static void eval( type_t t, Coeffs& filt,  const float fcrel, const float q, const float gainLin = 1.0  );

    static inline float   relfreq( const float freq, const float samplfreq )
    {
        return freq/samplfreq;
    };
    static inline float db2gain( const float db )
    {
        return pow( 10.0, db/40.0 );
    };
    // TODO
    static inline float qBandwidth( const float db )
    {
        return 0;
    };

    static void evalRBJLowPass(     Coeffs& filt, const float fcrel, const float q, const float gainLin = 0 );
    static void evalRBJHighPass(    Coeffs& filt, const float fcrel, const float q, const float gainLin = 0 );
    static void evalRBJBandPassCS(  Coeffs& filt, const float fcrel, const float q, const float gainLin = 0 );
    static void evalRBJBandPassCP(  Coeffs& filt, const float fcrel, const float q, const float gainLin = 0 );
    static void evalRBJNotch(       Coeffs& filt, const float fcrel, const float q, const float gainLin = 0 );
    static void evalRBJAllpass(     Coeffs& filt, const float fcrel, const float q, const float gainLin = 0 );
    static void evalRBJPeeking(     Coeffs& filt, const float fcrel, const float q, const float gainLin );
    static void evalRBJLowShelf(    Coeffs& filt, const float fcrel, const float q, const float gainLin );
    static void evalRBJHigShelf(    Coeffs& filt, const float fcrel, const float q, const float gainLin );
};

} // end namespace filter
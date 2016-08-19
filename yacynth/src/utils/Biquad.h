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
 * File:   Biquad.h
 * Author: Istvan Simon
 *
 * Created on March 13, 2016, 10:49 PM
 */
#include    "FilterBase.h"

#include    <cstdint>

// for 2nd order looks bad
// #define USE_V4S
// #define FILTER_4TH


//
// WARNING
//  coefficients a1,a2 , b0,b1,b2   -- B * X  + A * Y
//  NO SUBSTRACTION -- a1,a2 must be negated !!
//
//
// http://www.micromodeler.com/dsp/
//
namespace filter {

typedef float v4sf __attribute__((mode(SF)))  __attribute__ ((vector_size(16),aligned(16)));
//
//  for 2nd or 4th order
//

struct alignas(16) Coeffs {
    union
    {
        v4sf        v12;
        struct {
            float   b1;
            float   b2;
            float   a1;
            float   a2;
        };
    };

#ifdef  FILTER_4TH
    union
    {
        v4sf        v34;
        struct {
            float   b3;
            float   b4;
            float   a3;
            float   a4;
        };
    };
#endif

    float       b0;
    float       rfu1;
    float       rfu2;
    float       rfu3;
};

class BiquadBase {
public:
    union V4sfBiquadXY
    {
        v4sf        v;
        struct {
            float   xv1;
            float   xv2;
            float   yv1;
            float   yv2;
        };
    };

    BiquadBase( const float b0, const float b1, const float b2, const float a1, const float a2 )
    { set( b0, b1, b2, a1, a2 ); };


    BiquadBase()
    {};

    void set( const float b0, const float b1, const float b2, const float a1, const float a2 )
    {
        coeff.b0 = b0;
        coeff.b1 = b1;
        coeff.b2 = b2;
        coeff.a1 = a1;
        coeff.a2 = a2;
    };

    void set( const Coeffs& cc )
    {
        coeff = { cc };
    };

    Coeffs& getCoeffRef(void)
    { return this->coeff; };

protected:
    Coeffs  coeff;
};


class Biquad : public BiquadBase {
public:
    Biquad( const float b0, const float b1, const float b2, const float a1, const float a2 )
    :   BiquadBase( b0, b1, b2, a1, a2 )
    {};

    Biquad()
    {};

    struct Delay {
        float z1;
        float z2;
    };

    void clear(void) { delayA.z1 = delayA.z2 = 0; };

// count 100000000 endt 1458058387991294 begint 1458058387414503 diff  576791 avg 0.00576791

    inline void get( float *inout )
    {
        const float ret = *inout * coeff.b0 + delayA.z1;
        delayA.z1       = *inout * coeff.b1 + ret * coeff.a1 + delayA.z2;
        delayA.z2       = *inout * coeff.b2 + ret * coeff.a2;
        *inout          = ret;
    };
// count 100000000 endt 1458058305222917 begint 1458058304649506 diff  573411 avg 0.00573411

    inline void get( const float in, float& out )
    {
        out             = in * coeff.b0 + delayA.z1;
        delayA.z1       = in * coeff.b1 + delayA.z2 + out * coeff.a1;
        delayA.z2       = in * coeff.b2 + out * coeff.a2;
    };


// no USE_V4S   : count 100000000 endt 1458057417979701 begint 1458057417405118 diff   574583 avg 0.00574583
//  USE_V4S     : count 100000000 endt 1458057379978500 begint 1458057377618540 diff  2359960 avg 0.0235996

// count 100000000 endt 1458058458150838 begint 1458058457573584 diff  577254 avg 0.00577254

    inline float get( const float in )
    {
        const float ret = in * coeff.b0 + delayA.z1;
        delayA.z1       = in * coeff.b1 + delayA.z2 + ret * coeff.a1;
        delayA.z2       = in * coeff.b2 + ret * coeff.a2;
        return ret;
    };

    // Summation point ??
    inline float getSum( const float in )
    {
        const float ret = in * coeff.b0 + delayA.z1;
        delayA.z1       = in * coeff.b1 + delayA.z2 + ret * coeff.a1;
        delayA.z2       = in * coeff.b2 + ret * coeff.a2;
        return delayA.z1 + delayA.z2;
    };

private:
    Delay           delayA;
};


class BiquadStereo : public BiquadBase {
public:
    BiquadStereo( const float b0, const float b1, const float b2, const float a1, const float a2 )
    :   BiquadBase( b0, b1, b2, a1, a2 )
    {};

    BiquadStereo()
    {};

    struct Delay {
        float z1;
        float z2;
    };

    void clear(void) { delayA = delayB = { 0 }; };

    inline float getA( const float in )
    {
        const float ret = in * coeff.b0 + delayA.z1;
        delayA.z1       = in * coeff.b1 + ret * coeff.a1 + delayA.z2;
        delayA.z2       = in * coeff.b2 + ret * coeff.a2;
        return ret;
    };
    inline void getA( float *inout )
    {
        const float ret = *inout * coeff.b0 + delayA.z1;
        delayA.z1       = *inout * coeff.b1 + ret * coeff.a1 + delayA.z2;
        delayA.z2       = *inout * coeff.b2 + ret * coeff.a2;
        *inout          = ret;
    };
    inline void getA( const float in, float& out )
    {
        out             = in * coeff.b0 + delayA.z1;
        delayA.z1       = in * coeff.b1 + out * coeff.a1 + delayA.z2;
        delayA.z2       = in * coeff.b2 + out * coeff.a2;

    };
    inline float getB( const float in )
    {
        const float ret = in * coeff.b0 + delayB.z1;
        delayB.z1       = in * coeff.b1 + ret * coeff.a1 + delayB.z2;
        delayB.z2       = in * coeff.b2 + ret * coeff.a2;
        return ret;
    };
    inline void getB( float *inout )
    {
        const float ret = *inout * coeff.b0 + delayB.z1;
        delayB.z1       = *inout * coeff.b1 + ret * coeff.a1 + delayB.z2;
        delayB.z2       = *inout * coeff.b2 + ret * coeff.a2;
        *inout          = ret;
    };
    inline void getB( const float in, float& out )
    {
        out             = in * coeff.b0 + delayB.z1;
        delayB.z1       = in * coeff.b1 + out * coeff.a1 + delayB.z2;
        delayB.z2       = in * coeff.b2 + out * coeff.a2;
    };

// count 100000000 endt 1458058997712412 begint 1458058997133983 diff  578429 avg 0.00578429
    inline void get( const float inA, const float inB,  float& outA, float& outB )
    {
        outA            = inA * coeff.b0 + delayA.z1;
        outB            = inB * coeff.b0 + delayB.z1;
        delayA.z1       = inA * coeff.b1 + outA * coeff.a1 + delayA.z2;
        delayB.z1       = inB * coeff.b1 + outB * coeff.a1 + delayB.z2;
        delayA.z2       = inA * coeff.b2 + outA * coeff.a2;
        delayB.z2       = inB * coeff.b2 + outB * coeff.a2;
    };

    //
    // IN PLACE !!!!!!!!!!!!!!!!!!!
    //
    inline void get( float *inA, float *inB  )
    {
        const float rA  = *inA * coeff.b0 + delayA.z1;
        const float rB  = *inB * coeff.b0 + delayB.z1;
        delayA.z1       = *inA * coeff.b1 + rA * coeff.a1 + delayA.z2;
        delayB.z1       = *inB * coeff.b1 + rB * coeff.a1 + delayB.z2;
        delayA.z2       = *inA * coeff.b2 + rA * coeff.a2;
        delayB.z2       = *inB * coeff.b2 + rB * coeff.a2;
        *inA            = rA;
        *inB            = rB;
    };

    //
    // DO NOT USE IN PLACE !!!!!!!!!!!!!!!!!!!
    //
    inline void get( const float * const inA, const float * const inB, float *outA, float *outB  )
    {
        *outA           = *inA * coeff.b0 + delayA.z1;
        *outB           = *inB * coeff.b0 + delayB.z1;
        delayA.z1       = *inA * coeff.b1 + *outA * coeff.a1 + delayA.z2;
        delayB.z1       = *inB * coeff.b1 + *outB * coeff.a1 + delayB.z2;
        delayA.z2       = *inA * coeff.b2 + *outA * coeff.a2;
        delayB.z2       = *inB * coeff.b2 + *outB * coeff.a2;
    };

private:
    Delay   delayA;
    Delay   delayB;
};


} // end namespace filter


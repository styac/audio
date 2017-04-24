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
 * File:   Comb.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on March 8, 2016, 9:15 AM
 */

#include    "yacynth_globals.h"
#include    "Ebuffer.h"
#include    <array>
#include    <iostream>

//
// TODO : NESTED
//  https://ccrma.stanford.edu/~jos/pasp/Nested_Allpass_Filters.html
//
namespace yacynth {

class Comb {
public:
    static constexpr auto sectionSize = EIObuffer::sectionSize;
    struct Tap {
        Tap()
        { clear(); };

        Tap( uint64_t d32, float muDelay, float muOut )
        :   dlind32( d32 )
        ,   multDelay( muDelay )
        ,   multOut( muOut )
        ,   dlind64( d32 << 32  )
        {};
        void clear(void)
        {
            multDelay = 0;
            multOut = 0;
            dlind32 = 0;
            dlind64 = 0;
        };
        float       multDelay;  // called bM, or -aM
        float       multOut;    // called b0
        uint32_t    dlind32;    // delay index 0 => -1, 1 => -2 ...
        uint64_t    dlind64;    // for interpolation - DO NOT INDEX WITH THIS !!!
    };

    Comb() = delete;

    Comb( std::size_t length )
    :   delay(length)
    ,   bufferSizeMask( delay.bufferSizeMask )
    ,   toStereo(false)
    {};

    Comb( std::size_t length, uint32_t dlind32, float multDelay, float multOut )
    :   delay(length)
    ,   bufferSizeMask( delay.bufferSizeMask )
    ,   tap( dlind32, multDelay, multOut )
    ,   toStereo(false)
    {};

    void setStereo( bool val ) { toStereo = val; };
    void setTap( const Tap& v ) {
        tap = v;
    }

    void setTap( const uint32_t dlind,  const float multDelay, const float multOut) {
        tap.dlind32     = dlind;
        tap.dlind64     = uint64_t( dlind )  << 32;
        tap.multDelay   = multDelay;
        tap.multOut     = multOut;
    }

    // these work WITHOUT interpolation
    void feedforward(   const EIObuffer& inp, EIObuffer& out );
    void feedback(      const EIObuffer& inp, EIObuffer& out );
    void allpass(       const EIObuffer& inp, EIObuffer& out );
    void noop(          const EIObuffer& inp, EIObuffer& out );

protected:
    Tap             tap;
    EDelayLine      delay;
    const uint32_t  bufferSizeMask;
    bool            toStereo;
};

} // end namespace yacynth


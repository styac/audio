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
 * File:   CombInterpolated.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on March 8, 2016, 10:23 PM
 */

#include    "Comb.h"

#include    "../oscillator/Lfo.h"

namespace yacynth {

class CombInterpolated : public Comb {
public:
    CombInterpolated() = delete;
    CombInterpolated( std::size_t length );
    CombInterpolated( std::size_t length, uint32_t dlind32, float multDelay, float multOut, uint16_t depthExp );

    void feedforwardInterpolated(   const EIObuffer& inp, EIObuffer& out );
    void feedbackInterpolated(      const EIObuffer& inp, EIObuffer& out );
    void allpassInterpolated(       const EIObuffer& inp, EIObuffer& out );
    void barberpole(                const EIObuffer& inp, EIObuffer& out );

    void setLfoDepthExp( const uint16_t val )
    {
        lfo.setRange( val );
    };

private:
    Lfo         lfo;
};

} // end namespace yacynth


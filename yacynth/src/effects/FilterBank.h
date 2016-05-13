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
 * File:   FIlterBank.h
 * Author: Istvan Simon
 *
 * Created on March 15, 2016, 3:06 PM
 */

#include    "../utils/Biquad.h"
#include    "../utils/BiquadMath.h"
#include    "Ebuffer.h"

using namespace filter;

namespace yacynth {

class FilterBank {
public:
    static constexpr auto       sectionSize     = EIObuffer::sectionSize;

    void setCoeffs(  const float fcrel, const float q, const float gainLin = 1.0 )
    {
        biquad.clear();
        BiquadParam::evalRBJLowPass( biquad.getCoeffRef(), fcrel, q, gainLin );
        std::cout
            << "fc " << fcrel
            << " q " << q
            << " a1 " << biquad.getCoeffRef().a1
            << " a2 " << biquad.getCoeffRef().a2
            << " b0 " << biquad.getCoeffRef().b0
            << " b1 " << biquad.getCoeffRef().b1
            << " b2 " << biquad.getCoeffRef().b2
            << std::endl;
    };

    void process( const EIObuffer& in, EIObuffer& out );

private:
    Biquad          biquad;
    BiquadStereo    biquadStereo;
};

} // end namespace yacynth

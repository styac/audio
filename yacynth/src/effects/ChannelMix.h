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
 * File:   ChannelMix.h
 * Author: Istvan Simon
 *
 * Created on March 26, 2016, 10:35 PM
 */

#include    "yacynth_globals.h"
#include    "Filter.h"
#include    "Ebuffer.h"
#include    "Echo.h"
#include    "Comb.h"
#include    "CombInterpolated.h"
#include    "FilterBank.h"

#include    "../oscillator/Tables.h"
#include    "../oscillator/OscillatorOutput.h"
#include    "../control/Controllers.h"
#include    "../oscillator/Lfo.h"
#include    "../utils/GaloisNoiser.h"
#include    "../utils/Limiters.h"

#include    <cstdint>
#include    <iostream>
#include    <tgmath.h>
#include    <cstdint>
#include    <cmath>
#include    <array>

//
// this will replace the Panmix
// converts osc out (int) -> float
// calls the effects
// mixes everything
//  gives the output to the jack
//  called by the jack CB
//

using namespace limiter;

namespace yacynth {

class ChannelMix {
public:
    ChannelMix();
// main entry point
//    void process( const OscillatorOut& inp, Ebuffer64Float& out );
// or?
//    void run( const OscillatorOut& inp );
//    const Ebuffer64Float& get( void );

    void mute(  bool m = true );
    void fadeOut( void );

private:
    // this will map the oscillator outputs to the channel output --
    // this will be donw by the osc
   // std::array< uint16_t, overtoneCountOfOscillator >   mapping;
    std::array< EIObuffer, effectChannelCount >    effectChannel;
};

} // end namespace yacynth


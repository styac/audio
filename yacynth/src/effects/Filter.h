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
 * File:   Filter.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on February 18, 2016, 5:40 PM
 */

#include    "../control/Controllers.h"
#include    "../utils/Filter4Pole.h"
#include    "../utils/Filter4PoleInteger.h"
#include    "../utils/FilterStateVariable.h"
#include    "../utils/FilterAllpass.h"
#include    "../utils/GaloisNoiser.h"
#include    "FilterControl.h"
#include    "yacynth_globals.h"
#include    "Ebuffer.h"

#include    <array>
#include    <iomanip>
#include    <algorithm>


using namespace filter;
using namespace noiser;

namespace yacynth {

class ControlledFilter {
public:
    static constexpr uint8_t  channelCountExp   = 3;
    static constexpr uint8_t  channelCount      = 1<<channelCountExp;
    static constexpr uint8_t  channelCountMask  = channelCount-1;

    static constexpr uint16_t filterSampleCount     = EIObuffer::sectionSize;

    enum FilterMode {
        FLT_NOP,
        FLT_4P1xLP,         // 1x low pass
        FLT_4P1xBP1,        // 1x band pass1
        FLT_4P1xBP2,        // 1x band pass2
        FLT_4P1xBP3,        // 1x band pass3

        FLT_4P2xLPBP1,      // 1x low pass 1x band pass1
        FLT_4P2xLPBP2,      // 1x low pass 1x band pass1
        FLT_4P2xLPBP3,      // 1x low pass 1x band pass1
        FLT_4P2xBP11,       // 2x band pass1
        FLT_4P2xBP22,       // 2x band pass1
        FLT_4P2xBP33,       // 2x band pass1
        FLT_4P2xBP12,       // 2x band pass1
        FLT_4P2xBP23,       // 2x band pass1
        FLT_4P2xBP13,       // 2x band pass1

        FLT_4P3xLPBP11,
        FLT_4P3xLPBP22,
        FLT_4P3xLPBP33,
        FLT_4P3xLPBP12,
        FLT_4P3xLPBP23,
        FLT_4P3xLPBP13,

        FLT_4P3xBP111,      // 3x band pass1
        FLT_4P3xBP222,      // 3x band pass2
        FLT_4P3xBP333,      // 3x band pass3

        FLT_4P3xBP122,      // 3x band pass1
        FLT_4P3xBP123,      // 3x band pass2
        FLT_4P3xBP223,      // 3x band pass1
        FLT_4P3xBP233,      // 3x band pass2

        FLT_4P4xLPBP111,
        FLT_4P4xLPBP222,
        FLT_4P4xLPBP333,
        FLT_4P4xLPBP123,

        FLT_4P4xBP1111,     // 4x band pass1
        FLT_4P4xBP2222,     // 4x band pass2
        FLT_4P1xBP3333,     // 1x band pass3
    };

    ControlledFilter();
    void reset(void)
    {
    };


    void updateLfos(void);
    inline void control(void)
    {};

    void process( const EIObuffer& inp, EIObuffer& out );

private:


    Filter4Pole<channelCountExp>    filter;
    FilterControl<channelCountExp>  filterControl;
    ControllerIterated  ctrlF1;
    ControllerIterated  ctrlF2;
    ControllerIterated  ctrlQ;
//    GNoiseShaped                    noise;  // test -> to be removed -> connect input

    // Lfo  filter; // filter f modulation
    // Lfo  amMod   // after filter channel Amplitude modulation

    // profiling
    uint64_t    timer;
    int64_t     count;
};

} // end namespace yacynth


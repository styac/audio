#pragma once

/*
 * Copyright (C) 2017 Istvan Simon -- stevens37 at gmail dot com
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
 * File:   FxFilterParam.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on May 10, 2017, 8:49 PM
 */

#include    "Tags.h"
#include    "protocol.h"
#include    "yacynth_globals.h"
#include    "v4.h"
#include    "control/Controllers.h"

namespace yacynth {
using namespace TagEffectCollectorLevel_01;
using namespace TagEffectTypeLevel_02;

// --------------------------------------------------------------------
// FxFilterParam ------------------------------------------------------
// --------------------------------------------------------------------
// controller index
// cache
// mapper
// function
//

// modes:
//
//  1,2,3,4 input mono -> output stereo
//  2,4,6,8 input stereo ( 1,2,3,4 * 2)
//
//  1 LP | BP
//  2 LP+BP | BP+BP
//  3 LP+BP+BP | BP+BP+BP
//  4 LP+BP+BP+BP | BP+BP+BP+BP
// control : controller(1..8)( y0 + slope ), amplitude (1) ( y0 + slope ), oscillator (1..8) ( y0 + slope )
//
// key>     keyprefix+instance
// key-refix> /effect/filter/...
// data>    comment+const+var
// data-const> inputcount,maxmode,filtercount,channelcount,

// interface to save load DB !!
class FxFilterParam {
public:
    // mandatory fields
    static constexpr char const * const name    = "Filter";
    static constexpr TagEffectType  type        = TagEffectType::FxFilter;
    static constexpr std::size_t maxMode        = 9;
    static constexpr std::size_t inputCount     = 1;

    static constexpr uint8_t  filterCount       = 8;
    static constexpr uint8_t  vfilterCount      = 2;
    static constexpr uint8_t  channelCountExp   = 3;
    static constexpr uint8_t  channelCount      = 1<<channelCountExp;
    static constexpr uint8_t  channelCountMask  = channelCount-1;

    bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    // -----------------------------------------------------------
    // state variable mode
    //
    struct Mode_SVF01_2ch {
//    static constexpr uint8_t subtype         = uint8_t(TagEffectFxEarlyReflectionMode::SetParametersMode01);

        bool check()
        {
            return true;
        }

        // manual control
        ControllerIndex         fControlIndex;
        ControllerIndex         qControlIndex;
        ControllerMapLinear<1>  fMapper;

        // oscillator freq and phase diff
        ControllerIndex deltaPhaseIndex;    // to set the freq
        ControllerIndex phaseDiff00Index;   // to set the phase diff A-B

        // oscillator phase values
        ControllerIndex oscMasterIndex;     // master oscillator
        ControllerIndex oscSlave00Index;    // slave osc

    } mode_SVF01_2ch;


    // -----------------------------------------------------------
    // 4 pole mode
    //
    struct Mode_4p_2ch {
//    static constexpr uint8_t subtype         = uint8_t(TagEffectFxEarlyReflectionMode::SetParametersMode01);
        bool check()
        {
            return true;
        }

        // manual control
        ControllerIndex         fControlIndex;
        ControllerIndex         qControlIndex;
        ControllerMapLinear<1>  fMapper;

        // oscillator freq and phase diff
        ControllerIndex deltaPhaseIndex;    // to set the freq
        ControllerIndex phaseDiff00Index;   // to set the phase diff A-B

        // oscillator phase values
        ControllerIndex oscMasterIndex;     // master oscillator
        ControllerIndex oscSlave00Index;    // slave osc

    } mode_4p_2ch;

    // -----------------------------------------------------------
    // allpass mode
    //
    struct Mode_2ch_x4ap_phaser_mode01 {
//    static constexpr uint8_t subtype         = uint8_t(TagEffectFxEarlyReflectionMode::SetParametersMode01);

        bool check()
        {
            return true;
        }


        ControllerIndex feedbackGainIndex;  // to set the feedback gain
        ControllerIndex wetDryGainIndex;       // to set the wet/dry gain -- which ???

        ControllerIndex bandWidhthIndex;    // to set the bandwith of the filters (K1) - here all the same
        // LFO
        ControllerIndex deltaPhaseControlIndex;    // to set the freq - phase ch0
        ControllerIndex phaseDiff00ControlIndex;   // to set the phase diff ch1 to ch0

        // oscillator freq and phase diff
        ControllerIndex deltaPhaseIndex;    // to set the freq - phase ch0
        ControllerIndex phaseDiff00Index;   // to set the phase diff ch1 to ch0
        // oscillator phase values
        ControllerIndex oscMasterIndex;     // master oscillator
        ControllerIndex oscSlave00Index;    // slave osc

        ControllerMapLinear<1>  oscFreqMapper;      // modulator frequency
        ControllerMapLinear<1>  bandwidthMapper;    // bandwith control to ycent (allpass K1)
        ControllerMapLinear<8>  notchMapper;        // 1 slope + 8 offset or 8 slope + 8 offset?

        // ControllerMapLinear<1>  notchMapper[8]; // 1 slope + 8 offset or 8 slope + 8 offset?

    } mode_2ch_x4ap_phaser_mode01;

    // -----------------------------------------------------------

};

} // end namespace yacynth

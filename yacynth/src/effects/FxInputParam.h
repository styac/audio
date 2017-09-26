#pragma once
/*
 * Copyright (C) 2017 ist
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
 * File:   FxInputParam.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on September 16, 2017, 10:37 AM
 */



#include    "protocol.h"
#include    "Tags.h"
#include    "../control/Controllers.h"


namespace yacynth {

using namespace TagEffectTypeLevel_02;
using namespace TagEffectFxInputModeLevel_03;

class FxInputParam {
    
public:
    FxInputParam()
    {};
    // mandatory fields
    static constexpr char const * const name = "Input";
    static constexpr TagEffectType  type     = TagEffectType::FxInput;
    static constexpr std::size_t maxMode     = 1; //
    static constexpr std::size_t inputCount  = 0; // input is audio input

    // temp
    static constexpr   float   gainref = 1.0f/(1L<<24);
    static constexpr uint8_t subtype         = uint8_t(TagEffectFxInputMode::SetParametersMode01);

    bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );
};

}; // end namespace yacynth
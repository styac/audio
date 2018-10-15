/*
 * Copyright (C) 2018 Istvan Simon
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
 * File:   FxBaseTypeList.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on February 22, 2018, 11:01 PM
 */
#include "yacynth_config.h"
#include "effects/FxBase.h"

namespace yacynth {

// add more info

#define FXTYPE(cr,t) \
    { uint8_t(TagEffectType::t), cr, #t }

const EffectTypes effectTypes[] = {        
    FXTYPE(false,FxNop),
    FXTYPE(false,FxNil),
    FXTYPE(false,FxSlave),
    FXTYPE(true,FxMixer),
    FXTYPE(false,FxOscillatorMixer),
    FXTYPE(false,FxInput),
    FXTYPE(true,FxModulator),
    FXTYPE(true,FxOutNoise),
    FXTYPE(true,FxOutOscillator),
    FXTYPE(true,FxFilter),
    FXTYPE(true,FxEcho),
    FXTYPE(true,FxLateReverb),
    FXTYPE(true,FxEarlyReflection),
    FXTYPE(true,FxChorus),
    FXTYPE(true,FxFlanger),
};

} // end namespace yacynth 

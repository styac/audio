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
 * File:   AmplitudeModulator.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on April 18, 2016, 8:48 AM
 */

#include    "../oscillator/BaseOscillator.h"
#include    "EffectBase.h"
#include    "Ebuffer.h"

namespace yacynth {

template< typename Tstore >
class AmplitudeModulatorBase {
public:

protected:
    uint32_t        depth;
    uint32_t        offset;
    uint32_t        gain;
    uint32_t        stereoDepth;
    uint32_t        stereoPhaseDiff;
    
    BaseOscillator  modOscillator;
        
};

} // end namespace yacynth


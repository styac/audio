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
 * File:   StaticEffects.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on October 4, 2017, 7:01 PM
 */
// #include    "StaticEffects.h"

#include    "../effects/FxMixer.h"
#include    "../effects/FxFilter.h"
#include    "../effects/FxOutNoise.h"
#include    "../effects/FxModulator.h"
#include    "../effects/FxOutOscillator.h"
#include    "../effects/FxEcho.h"
#include    "../effects/FxLateReverb.h"
#include    "../effects/FxEarlyReflection.h"
#include    "../effects/FxChorus.h"
#include    "../effects/FxFlanger.h"


using namespace yacynth;

void createStaticEfects()
{
    FxOutNoise          * fxnoise       = new FxOutNoise();
    FxOutOscillator     * fxosc         = new FxOutOscillator();
    FxModulator         * fxmod         = new FxModulator();
    FxFilter            * fxfilt        = new FxFilter();
    FxEcho              * fxecho        = new FxEcho();
    FxLateReverb        * fxrevb        = new FxLateReverb();
    FxEarlyReflection   * fxearlyref    = new FxEarlyReflection();
    FxChorus            * fxchorus      = new FxChorus();
    FxFlanger           * fxFlanger     = new FxFlanger();
    FxMixer             * fxMixer       = new FxMixer();
}

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
 * File:   Controllers.cpp
 * Author: Istvan Simon
 *
 * Created on February 16, 2016, 10:27 AM
 */

#include    "Controllers.h"

namespace yacynth {

float InnerController::expFuncTable[midiStep];
uint32_t InnerController::phaseFuncTable[midiStep];

InnerController::InnerController() 
{
    clear();
    constexpr float dy          = 0.97;
    float y = 1.0f;
    
    for( int i=midiStep-1; i>0; --i ) {
        expFuncTable[i] = y;
        y *= dy;
        phaseFuncTable[i] = (i & 0x7E )<<(31-7); // ????
    }
    expFuncTable[0]     = 0.0f;
    phaseFuncTable[0]   = 0;
    phaseFuncTable[midiStep-1] = phaseFuncTable[midiStep-2] = 0x80000000; // ????
    value.v[InnerController::CC_PITCHBEND] = 0x2000;
}
    
MidiRangeController::MidiRangeController()
{
    clear();
    // test setup
    // novation impulse 49 -- basic
    // channel 0 -- slide 9
    index[0][0x29] = InnerController::CC_SINK;      // 0
    index[0][0x2A] = InnerController::CC_SINK;
    index[0][0x2B] = InnerController::CC_SINK;
    index[0][0x2C] = InnerController::CC_SINK;
    index[0][0x2D] = InnerController::CC_SINK;
    index[0][0x2E] = InnerController::CC_MODULATOR_FREQ1;
    index[0][0x2F] = InnerController::CC_MODULATOR_FREQ0;
    
    index[0][0x30] = InnerController::CC_MODULATOR_PHASEDIFF0;
    index[0][0x31] = InnerController::CC_MAINVOLUME; // 8
    
    index[0][0x15] = InnerController::CC_SINK;      // upper
    index[0][0x16] = InnerController::CC_SINK;
    index[0][0x17] = InnerController::CC_SINK;
    index[0][0x18] = InnerController::CC_SINK;
    
    index[0][0x19] = InnerController::CC_SINK;      // lower
    index[0][0x1A] = InnerController::CC_SINK;
    index[0][0x1B] = InnerController::CC_SINK;
    index[0][0x1C] = InnerController::CC_SINK;

    index[0][controllerAftertouch]  = InnerController::CC_CHANNEL_AFTERTOUCH;
    index[0][controllerPitchbend]   = InnerController::CC_PITCHBEND;
    
    
}


} // end namespace yacynth
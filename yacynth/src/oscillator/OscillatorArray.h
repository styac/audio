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
 * File:   OscillatorArray.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on February 1, 2016, 4:47 PM
 */

#include    "Oscillator.h"
#include    "OscillatorOutput.h"
#include    "../message/yamsg.h"
#include    "../control/Controllers.h"

#include    <array>

namespace yacynth {

class   OscillatorArray  {
public:
    OscillatorArray();
    bool initialize(  void );
        
    void clear(void) {};
    static ToneShaperMatrix& getToneShaperMatrix(void) 
        { return Oscillator::getToneShaperMatrix(); };
        
    void        generate(       OscillatorOut& out, Statistics& stat );
    void        voiceUp(        uint16_t oscNr,  uint32_t pitch,  uint16_t velocity );
    void        voiceChange(    uint16_t oscNr,  uint32_t pitch,  uint16_t velocity );

    inline void getPitch() 
    {
        constexpr  int8_t   pitchBendMaxExp     = 13 + 10;    
        constexpr  int32_t  pitchBendMax        = 1<<pitchBendMaxExp;    
        constexpr  float    octaveResolution    = 1<<24;        
        constexpr  uint32_t pitchMult           = octaveResolution / 6; // fix range 
        if( pitchBend.updateDiff() ) {
            const int64_t pitchInd = (pitchBend.getValue()<<10) - pitchBendMax;
            oscillatorParamGenerate.pitchDelta = (pitchInd * pitchMult)>>pitchBendMaxExp;
        }
    }
    
private:
    OscillatorInGenerate    oscillatorParamGenerate;
    OscillatorInChange      oscillatorParamChange;
    uint8_t                 minVoice;
    uint8_t                 maxVoice;
    bool                    monoPhone;
    bool                    enableFM;
    int16_t                 fastReleaseTick;
    int32_t                 commonFMPitch;
    ControlledValue         pitchBend;
    std::array<Oscillator,overtoneCountOscDef> array;
};

} // end namespace yacynth


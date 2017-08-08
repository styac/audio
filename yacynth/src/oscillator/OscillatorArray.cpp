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
 * File:   OscillatorArray.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on February 1, 2016, 4:47 PM
 */

#include    "OscillatorArray.h"
#include    <cstdlib>
#include    <iostream>

namespace yacynth {
// --------------------------------------------------------
OscillatorArray::OscillatorArray()
:   oscillatorParamGenerate()
,   monoPhone(false)
,   enableFM(false)
,   fastReleaseTick(1)
,   minVoice(0)
,   maxVoice(voiceCount)
,   commonFMPitch(0)
//,   toneBank(0)
{
    pitchBendIndex.setIndex( InnerController::CC_PITCHBEND );
    pitchBendIndex.setInnerValue( pitchBendRange.middleValue );
    pitchBendRange.setRange();
}
// --------------------------------------------------------
bool OscillatorArray::initialize( void )
{
    fastReleaseTick = 50;
    for( auto voiceNr = 0u; voiceNr < voiceCount; voiceNr++ ) {
        array[ voiceNr ].initialize();
    }
    return true;
} // end OscillatorArray::initialize
// --------------------------------------------------------
void OscillatorArray::generate( OscillatorOut& out, Statistics& stat )
{
    out.clear();
    getPitch();
    if( enableFM ) {
//        oscillatorParamGenerate.pitchDelta += frequencyModulator.get() & 0x7FFFFF; // max +- 1/2 octave
    }
    stat.cycleCounter[Statistics::COUNTER_INNER_LOOP_SUM]    += stat.cycleCounter[Statistics::COUNTER_INNER_LOOP];
    stat.cycleCounter[Statistics::COUNTER_LOW_AMPLITUDE_SUM] += stat.cycleCounter[Statistics::COUNTER_LOW_AMPLITUDE];
    stat.cycleCounter[Statistics::COUNTER_SUSTAIN_SUM]       += stat.cycleCounter[Statistics::COUNTER_SUSTAIN];
    stat.cycleCounter[Statistics::COUNTER_OUTER_LOOP_SUM]    += stat.cycleCounter[Statistics::COUNTER_OUTER_LOOP];
    stat.cycleCounter[Statistics::COUNTER_INNER_LOOP]    = 0;
    stat.cycleCounter[Statistics::COUNTER_LOW_AMPLITUDE] = 0;
    stat.cycleCounter[Statistics::COUNTER_SUSTAIN]       = 0;
    stat.cycleCounter[Statistics::COUNTER_OUTER_LOOP]    = 0;
    ++stat.cycleCounter[Statistics::COUNTER_ARRAY];
    Oscillator::fillWhiteNoise();  // static
    for( auto voiceNr = minVoice; voiceNr < maxVoice; voiceNr++ ) {
        auto& oscillator   = array[ voiceNr ];
        if( oscillator.generate( oscillatorParamGenerate, out, stat ) ) {
            ++stat.cycleCounter[Statistics::COUNTER_ARRAY_POS];
       }
    }
} // OscillatorArray::generate
// --------------------------------------------------------
void OscillatorArray::voiceRun(  uint16_t oscNr,  uint32_t pitch,  uint16_t velocity, uint16_t toneBank )
{
    if( oscNr >= voiceCount ) 
        return;
    
    std::cout << "voiceUp oscNr:" << oscNr << " pitch:" << oscillatorParamChange.pitch << " toneBank:" << toneBank << std::endl;
    oscillatorParamChange.pitch             = pitch;
    oscillatorParamChange.velocity          = velocity;
    oscillatorParamChange.toneShaperSelect  = toneBank;
    array[ oscNr ].voiceRun( oscillatorParamChange );
} // OscillatorArray::voiceUp
// --------------------------------------------------------
void OscillatorArray::voiceRelease( uint16_t oscNr )
{
    if( oscNr >= voiceCount ) 
        return;

    std::cout << "new voiceChange voice: " << oscNr << std::endl;

    array[ oscNr ].voiceRelease( oscillatorParamChange );
} // OscillatorArray::voiceChange
// --------------------------------------------------------
} // end namespace yacynth



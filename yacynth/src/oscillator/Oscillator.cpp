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
 * File:   Oscillator.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on January 28, 2016, 5:20 PM
 */
#include    "Oscillator.h"
#include    "ToneShaper.h"

namespace yacynth {

const ToneShaperMatrix  Oscillator::toneShaperMatrix;
GNoise                  Oscillator::gNoiser;
GaloisShifter           Oscillator::gRandom;

// --------------------------------------------------------------------
Oscillator::Oscillator()
{
    oscillatorCountUsed = overtoneCountOscDef;
    for( auto i=0; i<overtoneCountOscDef; i++ ) {
//        wavetable[i]    = waveSinTable;
        state[i].phase  = 0;
        voiceState      = VOICE_DOWN;
    }
} // end Oscillator::Oscillator
// --------------------------------------------------------------------

void Oscillator::initialize( void )
{
}
// --------------------------------------------------------------------

bool Oscillator::generate( const OscillatorInGenerate& in,  OscillatorOut& out, Statistics& stat )
{
    bool        isEnd       = false;
    switch( voiceState ) {
    case VOICE_DOWN:
        return false;
    case VOICE_DELAY_RUN:
        if( --delay > 0 )
            return false;
        voiceState = VOICE_RUN;
        return false;
    case VOICE_RELEASE:
        isEnd = true;
    case VOICE_RUN:
        const auto& tsvec     = toneShaperMatrix.toneShapers[ toneShaperSelect ];
        oscillatorCountUsed = tsvec.oscillatorCountUsed;
        if( out.overtoneCount < oscillatorCountUsed ) {
            out.overtoneCount = oscillatorCountUsed;
        }
        for( auto oscindex = 0; oscindex < oscillatorCountUsed; ++oscindex ) {
            const auto& toneshaper      = tsvec.toneShaperVec[ oscindex ];
            const auto& oscillatorType  = toneshaper.oscillatorType;

            // TODOs
            // go to fastexp !
            // -> refactor transient detune !!!
            const uint32_t deltaPhase   = ycent2deltafi( basePitch + toneshaper.pitch, in.pitchDelta );
            if( 0 == deltaPhase )
                continue;
            int64_t deltaAddAcc = 0;
            bool changeEnvPhase = false;
            auto& stateOsc      = state[ oscindex ];
            // should be mapped 256 ovetones -> 16x2 output channels
            auto& outLayer      = out.layer[ oscindex ];
            if( 0 > stateOsc.envelopePhase ) {
                if( VOICE_RELEASE == voiceState ) {
                    continue; // next oscillator -- if all is end then voice down
                }
//++++++++++++++++++++++++++++
                goto L_sustain;
            }
            // TODO release -> own values tickFrameRelease, curveSpeedRelease
            if( envelopeKnotRelease == stateOsc.envelopePhase ) { // release
                const auto& currentEnvelopeKnot  = toneshaper.transient[ envelopeKnotRelease ];
                stateOsc.envelopeTargetValueVelocity = 0 ;
                // at least 1 for release
                // normal release: input stateOsc.tickFrame == 0
                // fast release  : input stateOsc.tickFrame > 0 -- legato - envelopMultiplierExpChecked fix
                if( 0 == stateOsc.tickFrame ) { // if fastRelease then stateOsc.tickFrame == 1
                    stateOsc.tickFrame = 0 == currentEnvelopeKnot.tickFrame[0] ? 1 : currentEnvelopeKnot.tickFrame[0];
                    stateOsc.envelopMultiplierExpChecked = oscillatorOutSampleCountExp - currentEnvelopeKnot.curveSpeed;
                } else {
                    stateOsc.envelopMultiplierExpChecked = oscillatorOutSampleCountExp + 1;
                }
                stateOsc.envelopePhase = 0; // next will  be -1 -- tricky
            } else {
                while(  0 == stateOsc.tickFrame ) {
                    changeEnvPhase = true;
                    stateOsc.tickFrame = toneshaper.transient[stateOsc.envelopePhase].tickFrame[0];
                    if( 0 < stateOsc.tickFrame )
                        break;
                    if( 0 > --stateOsc.envelopePhase ) {
//++++++++++++++++++++++++++++
                        goto L_sustain;
                    }
                }
                if( changeEnvPhase ) {
                    const auto& currentEnvelopeKnot  = toneshaper.transient[stateOsc.envelopePhase];
                    // 32 * 16 = 48 bit
#ifdef OSCILLATOR_AMPLITUDE_RANDOMIZER

                    stateOsc.envelopeTargetValueVelocity =
                        stateOsc.velocity * static_cast<uint64_t>(currentEnvelopeKnot.targetValue[0])
                        + ( gRandom.getWhite() & 0x0FFFF );
#else
                    stateOsc.envelopeTargetValueVelocity =
                        stateOsc.velocity * static_cast<uint64_t>(currentEnvelopeKnot.targetValue[0]);
#endif
                    stateOsc.envelopMultiplierExpChecked = oscillatorOutSampleCountExp - currentEnvelopeKnot.curveSpeed;
                }
            }
            switch( stateOsc.tickFrame ) {
            case 0:
                break;
            case 1: // this is the last frame in this round
                deltaAddAcc = (( stateOsc.envelopeTargetValueVelocity - stateOsc.amplitudoOsc )
                    >> oscillatorOutSampleCountExp );
                --stateOsc.tickFrame;
                --stateOsc.envelopePhase;
                break;
            case 2: // this is almost last frame in this round ; divide by 2
                // TODO:  this might be also better if linear
                deltaAddAcc = (( stateOsc.envelopeTargetValueVelocity - stateOsc.amplitudoOsc )
                    >> oscillatorOutSampleCountExp );   // linear ?
//                    >> ( oscillatorOutSampleCountExp + 1 ) );
                --stateOsc.tickFrame;
                break;
            default:
                deltaAddAcc = (( stateOsc.envelopeTargetValueVelocity - stateOsc.amplitudoOsc ) / stateOsc.tickFrame )
                    >> stateOsc.envelopMultiplierExpChecked;
                --stateOsc.tickFrame;
                break;
            }
//++++++++++++++++++++++++++++
            goto L_innerloop;
//============================
L_sustain:
            deltaAddAcc = stateOsc.sustainModulator.mod( stateOsc.amplitudoOsc, toneshaper.sustain, random4SustainModulator.get() );
            ++stat.cycleCounter[Statistics::COUNTER_SUSTAIN];
//============================
L_innerloop:
            if( ( 4 > stateOsc.amplitudoOsc ) && ( 0 >= deltaAddAcc )) {
                stateOsc.amplitudoOsc = 0;
                ++stat.cycleCounter[Statistics::COUNTER_LOW_AMPLITUDE];
                continue; // next oscillator
            }
            // avoid conditional jump in inner loop
            // this could be eliminated completely
            if( (stateOsc.amplitudoOsc + ( deltaAddAcc<<oscillatorOutSampleCountExp )) < 0 ) {
                deltaAddAcc = - ( stateOsc.amplitudoOsc >> oscillatorOutSampleCountExp );
            }
            out.amplitudeSumm += stateOsc.amplitudoOsc;
            auto layp = &outLayer[ 0 ];

//---------------------------
            // envelope 32 * 16 = 48 bit
            // wave: 16 bit signed = 64 bit signed
            // 64 signed >> 32 -> 32 bit/osc
            // 256 osc -> +8 -> 40 bit
            // 256 voice  +8 -> 48 bit output
            switch( oscillatorType ) {
            case OSC_SIN:
                for( auto buffindex = 0; buffindex < oscillatorOutSampleCount; ++buffindex ) {
                    *layp++ +=  ( tables::waveSinTable[ uint16_t((( stateOsc.phase += deltaPhase ) >> normFactorPhaseIndexExp))]
                            * stateOsc.amplitudoOsc ) >> normAmplitudeOscillatorExp;
                    stateOsc.amplitudoOsc += deltaAddAcc;
                }
                break;
            case OSC_SIN_RING:
                for( auto buffindex = 0; buffindex < oscillatorOutSampleCount; ++buffindex ) {
                    *layp++ +=  ( tables::waveSinTable[ uint16_t((( stateOsc.phase += deltaPhase ) >> normFactorPhaseIndexExp))]
                            * (stateOsc.amplitudoOsc /* * ringtable[++ri & mask] >> 16 */  ) ) >> normAmplitudeOscillatorExp;
                    stateOsc.amplitudoOsc += deltaAddAcc;
                }
                break;
//---------------------------
            case OSC_SINSIN:
                for( auto buffindex = 0; buffindex < oscillatorOutSampleCount; ++buffindex ) {
                    *layp++ += ( tables::waveSinTable[ uint16_t( tables::waveSinTable[ uint16_t((stateOsc.phase += deltaPhase ) >> normFactorPhaseIndexExp)])]
                            * stateOsc.amplitudoOsc ) >> normAmplitudeOscillatorExp;
                    stateOsc.amplitudoOsc += deltaAddAcc;
                }
                break;
//---------------------------
            case OSC_WHITE_NOISE:
                for( auto buffindex = 0; buffindex < oscillatorOutSampleCount; ++buffindex ) {
                    *layp++ +=  ( int16_t(gNoiser.getWhite()) * stateOsc.amplitudoOsc ) >> normAmplitudeOscillatorExp;
                    stateOsc.amplitudoOsc += deltaAddAcc;
                }
                break;

//---------------------------
//            case OSC_PDxx     sin(sin())
//            case OSC_PAIRxx

            } // end switch( oscillatorType )

            ++stat.cycleCounter[Statistics::COUNTER_INNER_LOOP];
            isEnd = false;
        }
        if( VOICE_RELEASE == voiceState && isEnd ) {
            voiceState = VOICE_DOWN;
        }
        ++stat.cycleCounter[Statistics::COUNTER_OUTER_LOOP];
        return true;
    }
} // end Oscillator::generate(  const OscillatorInGenerate& in,  OscillatorOut& out );
//======================================================================================================================
// --------------------------------------------------------------------
// if runningCount > limit then delay the new voice by a cycle
// void Oscillator::voiceUp( uint16_t envBankSel, uint16_t spectBankSel, uint16_t runningCount, int16_t velocity )

void Oscillator::voiceUp( const OscillatorInChange& in )
{
#if 0
    if( VOICE_DOWN != voiceState ) {
        return;
    }
#endif
    voiceState          = VOICE_RUN;
    toneShaperSelect    = in.toneShaperSelect;
    basePitch           = in.pitch;
    setPitchDependency();

    // monitoring
    std::cout << "voiceUp: " << in.pitch << " depind " << uint16_t( pitchDepIndex  ) << " depdx " << pitchDepDx << std::endl;
    random4SustainModulator.reset();
    grandomOscillatorThread.get();   // randomize a bit
    for( auto oscindex = 0; oscindex < oscillatorCountUsed; ++oscindex ) {
        auto& stateOsc = state[ oscindex ];
//        const uint16_t  amplitudoRate = spectrum->overtoneSet[ spectrumBankSelect ].overtone[ oscindex ].amplitudoRate;
        //const auto& env     = envelope->knotSet[ envelopeBankSelect ][ oscindex ];

#ifdef OSCILLATOR_VELOCITY_RANDOMIZER
        stateOsc.velocity = ( amplitudoRate * in.velocity ) + ( gRandom.getWhite() & 0x0FFFF );
#else
        stateOsc.velocity = in.velocity;
#endif

#ifdef OSCILLATOR_PHASE_RANDOMIZER
        stateOsc.phase = gRandom.getWhite();
#endif
        stateOsc.sustainModulator.reset();
        stateOsc.tickFrame          = 0;                // feature: must be set to 0
        stateOsc.envelopePhase      = envelopeKnotUp;   // always the highest-1 knot for voiceUp
//        stateOsc.releaseEnd         = false;
    }
}
// --------------------------------------------------------------------
void Oscillator::voiceRelease( const OscillatorInChange& in )
{
    if( VOICE_RELEASE == voiceState ) { // or if down !!!
        return;
    }
    std::cout << "voiceRelease: " << std::endl;

    voiceState  = VOICE_RELEASE;
    for( auto oscindex = 0; oscindex < oscillatorCountUsed; ++oscindex ) {
        auto& stateOsc = state[ oscindex ];
        stateOsc.tickFrame      = in.tickFrameRelease;  // used if not 0
        stateOsc.envelopePhase  = envelopeKnotRelease;  // knot[15] -- release targetValue==ignored
    }
} // end Oscillator::voiceRelease( const OscillatorInChange& in )
// --------------------------------------------------------------------
void Oscillator::voiceDelay( const OscillatorInChange& in )
{
    delay = in.delay;
} // end Oscillator::voiceRelease( const OscillatorInChange& in )
// --------------------------------------------------------------------
} // end namespace yacynth



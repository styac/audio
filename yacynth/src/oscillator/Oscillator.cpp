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

NoiseFrame< FrameInt< oscillatorOutSampleCountExp > >   
                        Oscillator::whiteNoiseFrame( GaloisShifterSingle<seedThreadOscillator_noise>::getInstance() );

const ToneShaperMatrix  Oscillator::toneShaperMatrix;

// --------------------------------------------------------------------
Oscillator::Oscillator()
:   noiseWide(GaloisShifterSingle<seedThreadOscillator_noise>::getInstance())
{
    oscillatorCountUsed = overtoneCountOscDef;
    for( auto i=0; i<overtoneCountOscDef; i++ ) {
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
    // under this amplitude value the signal will not be generated
    constexpr uint64_t hearingThreshold = 1L<<23;
    
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
//        oscillatorCountUsed = 10;
        
        if( out.overtoneCount < oscillatorCountUsed ) {
            out.overtoneCount = oscillatorCountUsed;
        }
        for( auto oscindex = 0; oscindex < oscillatorCountUsed; ++oscindex ) {
            const auto& toneshaper      = tsvec.toneShaperVec[ oscindex ];
            // high osc only sine
//            const auto& oscillatorType  = ( oscindex & 0x80 ) ? OSC_SIN : toneshaper.oscillatorType;
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
            // TODO: envelopeKnotRelease = +128 ??
            if( 0 > stateOsc.envelopePhase ) {
                if( VOICE_RELEASE == voiceState ) {
                    continue; // next oscillator -- if all is end then voice down
                }
//++++++++++++++++++++++++++++
                goto L_sustain;
            }

            if( envelopeKnotRelease == stateOsc.envelopePhase ) { // release
                stateOsc.envelopeTargetValueVelocity = 0 ;
                // at least 1 for release
                // normal release: input stateOsc.tickFrame == 0
                // fast release  : input stateOsc.tickFrame > 0 -- legato - envelopMultiplierExpChecked fix
                if( 0 == stateOsc.tickFrame ) { // if fastRelease then stateOsc.tickFrame == 1
                    stateOsc.tickFrame = toneshaper.tickFrameRelease.get();
                    stateOsc.envelopMultiplierExpChecked = oscillatorOutSampleCountExp - toneshaper.curveSpeedRelease;
                } else {
                    stateOsc.envelopMultiplierExpChecked = oscillatorOutSampleCountExp + 1;
                }
                stateOsc.envelopePhase = 0; // next will  be -1 -- tricky
            } else {
                while( 0 == stateOsc.tickFrame ) {
                    changeEnvPhase = true;
                    stateOsc.tickFrame = toneshaper.transient[stateOsc.envelopePhase].tickFrame.get();
                    if( 0 < stateOsc.tickFrame )
                        break;
                    if( 0 > --stateOsc.envelopePhase ) {
//++++++++++++++++++++++++++++
                        goto L_sustain;
                    }
                }
                if( changeEnvPhase ) {
                    // TODO:  frequency dependency -- amplitude,time
                    const auto& currentEnvelopeKnot  = toneshaper.transient[stateOsc.envelopePhase];
                    stateOsc.envelopeTargetValueVelocity =
                        (velocity * static_cast<uint64_t>(currentEnvelopeKnot.targetValue.get() )) >> 10; // 25 bit for the signal
                    stateOsc.envelopMultiplierExpChecked = oscillatorOutSampleCountExp - currentEnvelopeKnot.curveSpeed;
                }
            } // end if( envelopeKnotRelease == stateOsc.envelopePhase )
            
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
                deltaAddAcc = (( stateOsc.envelopeTargetValueVelocity - stateOsc.amplitudoOsc )
                    >> ( oscillatorOutSampleCountExp + 1 ) );
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
            deltaAddAcc = stateOsc.sustainModulator.mod( stateOsc.amplitudoOsc, toneshaper.sustain );
            ++stat.cycleCounter[Statistics::COUNTER_SUSTAIN];
//============================
L_innerloop:                
            if( ( hearingThreshold > stateOsc.amplitudoOsc ) && ( 0 >= deltaAddAcc )) {
                stateOsc.amplitudoOsc = 0;
                stateOsc.phase += deltaPhase<<oscillatorOutSampleCountExp; // no signal but move the phase by a tick
                ++stat.cycleCounter[Statistics::COUNTER_LOW_AMPLITUDE];
                continue; // next oscillator
            }

            // adjust if negative because of the truncation to avoid going into minus
            if( 0 > deltaAddAcc) {
                ++deltaAddAcc;
            }
            out.amplitudeSumm += stateOsc.amplitudoOsc;
            auto layp = &outLayer[ 0 ];

//---------------------------
            // envelope 32 * 16 = 48 - 10 = 38 bit
            // wave: 16 bit signed = 64 bit signed
            // 64 signed >> 32 -> 32 bit/osc
            // 256 osc -> +8 -> 40 bit
            // 256 voice  +8 -> 48 bit output
            switch( oscillatorType ) {
//---------------------------
            case OSC_SIN:
                for( auto buffindex = 0; buffindex < oscillatorOutSampleCount; ++buffindex ) {
                    *layp++ +=  ( tables::waveSinTable[ uint16_t((( stateOsc.phase += deltaPhase ) >> normFactorPhaseIndexExp))]
                            * stateOsc.amplitudoOsc ) >> normAmplitudeOscillatorExp;
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
            case OSC_SIN_RING:
                for( auto buffindex = 0; buffindex < oscillatorOutSampleCount; ++buffindex ) {
                    *layp++ +=  ( tables::waveSinTable[ uint16_t((( stateOsc.phase += deltaPhase ) >> normFactorPhaseIndexExp))]
                            * (stateOsc.amplitudoOsc /* * ringtable[++ri & mask] >> 16 */  ) ) >> normAmplitudeOscillatorExp;
                    stateOsc.amplitudoOsc += deltaAddAcc;
                }
                break;
//-----------------------------------------------------------------------------
// noise: 1 narrow + 1 wide for a voice !!!!!!!!!!!!!!!!                
// only 1 for a voice !!! 
// this will not be checked in this place -- parameter setting must ensure
// the wide has own galoisshifter -- uncorrelated                
// the narrow uses the common NoiseFrame -- correlated
//                
            case OSC_NOISE_WHITE:
                for( auto buffindex = 0; buffindex < oscillatorOutSampleCount; ++buffindex ) {
                    *layp++ +=  ( noiseWide.getWhite() * stateOsc.amplitudoOsc ) >> (normAmplitudeOscillatorExp+2);
                    stateOsc.amplitudoOsc += deltaAddAcc;
                }
                break;
//---------------------------                
            case OSC_NOISE_RED:
                for( auto buffindex = 0; buffindex < oscillatorOutSampleCount; ++buffindex ) {
                    *layp++ +=  ( noiseWide.getRed() * stateOsc.amplitudoOsc ) >> (normAmplitudeOscillatorExp+2);
                    stateOsc.amplitudoOsc += deltaAddAcc;
                }
//---------------------------                
            case OSC_NOISE_PURPLE:
                for( auto buffindex = 0; buffindex < oscillatorOutSampleCount; ++buffindex ) {
                    *layp++ +=  ( noiseWide.getPurple() * stateOsc.amplitudoOsc ) >> (normAmplitudeOscillatorExp+2);
                    stateOsc.amplitudoOsc += deltaAddAcc;
                }
                break;
//---------------------------                
            case OSC_NOISE_BLUE:
                for( auto buffindex = 0; buffindex < oscillatorOutSampleCount; ++buffindex ) {
                    *layp++ +=  ( noiseWide.getBlue() * stateOsc.amplitudoOsc ) >> (normAmplitudeOscillatorExp+2);
                    stateOsc.amplitudoOsc += deltaAddAcc;
                }
                break;
//---------------------------                
            case OSC_NOISE_PEEK1:
                for( auto buffindex = 0; buffindex < oscillatorOutSampleCount; ++buffindex ) {
                    *layp++ +=  ( noiseNarrow.getSv1( whiteNoiseFrame.getFrame()[buffindex] ) * stateOsc.amplitudoOsc ) >> (normAmplitudeOscillatorExp+2);
                    stateOsc.amplitudoOsc += deltaAddAcc;
                }
//---------------------------                
            case OSC_NOISE_PEEK2:
                for( auto buffindex = 0; buffindex < oscillatorOutSampleCount; ++buffindex ) {
                    *layp++ +=  ( noiseNarrow.getSv2( whiteNoiseFrame.getFrame()[buffindex] ) * stateOsc.amplitudoOsc ) >> (normAmplitudeOscillatorExp+2);
                    stateOsc.amplitudoOsc += deltaAddAcc;
                }
//---------------------------
            case OSC_NOISE_PEEK3:
                for( auto buffindex = 0; buffindex < oscillatorOutSampleCount; ++buffindex ) {
                    *layp++ +=  ( noiseNarrow.getSv3( whiteNoiseFrame.getFrame()[buffindex] ) * stateOsc.amplitudoOsc ) >> (normAmplitudeOscillatorExp+5);
                    stateOsc.amplitudoOsc += deltaAddAcc;
                }
//---------------------------

                // OSC_4x_NOISE -- oscindex += 3 --> 4 ??
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
    toneShaperSelect    = in.toneShaperSelect;
    basePitch           = in.pitch;
    setPitchDependency();

    // monitoring
    std::cout << "voiceUp: " << in.pitch << " depdx " << pitchDepDx << std::endl;
//    random4SustainModulator.reset();
#ifdef OSCILLATOR_VELOCITY_RANDOMIZER
    velocity = in.velocity + ( gRandom.getRaw() & 0x0FFFF );
#else
    velocity = in.velocity;
#endif
//    noiseWide.clear();
//    noiseNarrow.clear();
    noiseNarrow.setFreqSv(basePitch);
    for( auto oscindex = 0; oscindex < oscillatorCountUsed; ++oscindex ) {
        auto& stateOsc = state[ oscindex ];

#ifdef OSCILLATOR_PHASE_RANDOMIZER
        // if voice is not running !!
        stateOsc.phase = gRandom.getRaw();
#endif
        stateOsc.sustainModulator.reset();
        stateOsc.tickFrame          = 0;                // feature: must be set to 0
        stateOsc.envelopePhase      = envelopeKnotUp;   // always the highest-1 knot for voiceUp
//        stateOsc.releaseEnd         = false;
    }
    voiceState          = VOICE_RUN;
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
        stateOsc.envelopePhase  = envelopeKnotRelease;  
    }
} // end Oscillator::voiceRelease( const OscillatorInChange& in )
// --------------------------------------------------------------------
void Oscillator::voiceDelay( const OscillatorInChange& in )
{
    delay = in.delay;
} // end Oscillator::voiceRelease( const OscillatorInChange& in )
// --------------------------------------------------------------------
} // end namespace yacynth



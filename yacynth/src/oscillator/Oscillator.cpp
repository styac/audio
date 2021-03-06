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

#include "yacynth_config.h"
#include "oscillator/Oscillator.h"
#include "oscillator/ToneShaper.h"

namespace yacynth {

//namespace {
//constexpr auto LogCategoryMask              = LOGCAT_net;
//constexpr auto LogCategoryMaskAlways        = LogCategoryMask | nanolog::LogControl::log_always;
//constexpr const char * const LogCategory    = "NETS";
//}

NoiseFrame< FrameInt< oscillatorFrameSizeExp > >
                        Oscillator::whiteNoiseFrame( GaloisShifterSingle<seedThreadOscillator_noise>::getInstance() );

const ToneShaperMatrix  Oscillator::toneShaperMatrix;

// --------------------------------------------------------------------
Oscillator::Oscillator()
:   toneShaperVecCurr(&toneShaperMatrix.toneShapers[ 0 ])
,   noiseWide(GaloisShifterSingle<seedThreadOscillator_noise>::getInstance())
,   noisePhaseMode(GaloisShifterSingle<seedThreadOscillator_noise>::getInstance())
,   noiseNarrow()
,   basePitch(0)
,   targetPitch(0)
,   deltaPitch(0)
,   glissandoTick(0)
,   velocity(0)
,   delay(0)
,   toneShaperSelect(0)
,   oscillatorCountUsed( overtoneCountOscDef )
,   pitchDepDx(0)
,   runCount(0)
,   voiceState(VOICE_DOWN)
{
    ;
    for( auto i=0; i<overtoneCountOscDef; i++ ) {
        state[i].phase_0  = 0;
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
    // TODO test
    constexpr int64_t  hearingThreshold = 1L<<7;
    
    // magic numbers for stereo diff frequency sound
    constexpr int32_t frDetune0Magic = 1237;   // min value
    
            
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
        switch( glissandoTick ) {
        case 0:
            // nothing to do
            break;
        case 1:
            --glissandoTick;
            basePitch = targetPitch;
            // recalculate
            setPitchDependency();
            break;
        default:
            --glissandoTick;
            basePitch += deltaPitch;
        }

        oscillatorCountUsed = toneShaperVecCurr->oscillatorCountUsed;

        for( auto oscindex = 0; oscindex < oscillatorCountUsed; ++oscindex ) {
            const auto& toneshaper  = toneShaperVecCurr->toneShaper[ oscindex ];
            auto& stateOsc          = state[ oscindex ];
            uint32_t deltaPhase_0 = 0;
            uint32_t deltaPhase_1 = 0;
            int32_t frDetune0 = frDetune0Magic + ( int32_t(toneshaper.detune2CH) << 4 );
            int32_t filterCenterFreq = 0;
            int16_t filterBandwith = 0;
            uint8_t outChannel_0 = toneshaper.outChannel & oscOutputChannelCountMsk;

            const auto oscillatorType  = toneshaper.oscillatorType;
            switch( oscillatorType ) {
            case ToneShaper::OSC_NONE:
                continue;

            case ToneShaper::OSC_NOISE_WHITE:
            case ToneShaper::OSC_NOISE_RED0:
            case ToneShaper::OSC_NOISE_RED1:
            case ToneShaper::OSC_NOISE_RED2:
            case ToneShaper::OSC_NOISE_RED3:
            case ToneShaper::OSC_NOISE_PURPLE0:
            case ToneShaper::OSC_NOISE_PURPLE1:
            case ToneShaper::OSC_NOISE_PURPLE2:
            case ToneShaper::OSC_NOISE_PURPLE3:
            case ToneShaper::OSC_NOISE_BLUE0:
            case ToneShaper::OSC_NOISE_BLUE1:
            case ToneShaper::OSC_NOISE_BLUE2:
            case ToneShaper::OSC_NOISE_BLUE3:
                break;

            case ToneShaper::OSC_NOISE_SV1x1_PEEK:
            case ToneShaper::OSC_NOISE_SV2x1_PEEK:
            case ToneShaper::OSC_NOISE_SV3x1_PEEK:
            case ToneShaper::OSC_NOISE_SV1x2_PEEK:
            case ToneShaper::OSC_NOISE_SV2x2_PEEK:
            case ToneShaper::OSC_NOISE_SV3x2_PEEK:
            case ToneShaper::OSC_NOISE_SV1x3_PEEK:
            case ToneShaper::OSC_NOISE_SV2x3_PEEK:
            case ToneShaper::OSC_NOISE_SV3x3_PEEK:
            case ToneShaper::OSC_NOISE_SV1x4_PEEK:
            case ToneShaper::OSC_NOISE_SV2x4_PEEK:
            case ToneShaper::OSC_NOISE_SV3x4_PEEK:
                filterCenterFreq = noiseNarrow.getFreqSv( basePitch + toneshaper.pitch + in.pitchDelta );
                filterBandwith = 0x7F + (toneshaper.filterBandwidth<<7); // 1...0.2
                break;

            case ToneShaper::OSC_NOISE_4Px1_PEEK:
            case ToneShaper::OSC_NOISE_4Px2_PEEK:
                filterCenterFreq = noiseNarrow.getFreq4p( basePitch + toneshaper.pitch + in.pitchDelta );
                filterBandwith = 0x7F + (toneshaper.filterBandwidth<<7); // max 3.9999 - 3.FC
                break;

            case ToneShaper::OSC_NOISE_APx1_PEEK:
            case ToneShaper::OSC_NOISE_APx2_PEEK:
            case ToneShaper::OSC_NOISE_APx3_PEEK:
            case ToneShaper::OSC_NOISE_APx4_PEEK:
                filterCenterFreq = noiseNarrow.getFreqAp2( basePitch + toneshaper.pitch + in.pitchDelta );
                filterBandwith =  0x7000 + (int16_t(toneshaper.filterBandwidth)<<4); // 0.5 ...0.9999
                break;

            default:
                deltaPhase_0 = ExpTable::ycent2deltafi( basePitch + toneshaper.pitch, in.pitchDelta );
                if( 0 == deltaPhase_0 )
                    continue; // too high sound
            }

            int64_t deltaAmpl = 0;
            bool changeEnvPhase = false;
            // TODO: envelopeKnotRelease = +128 ??
            // this should be before ycent2deltafi() ?
            if( 0 > stateOsc.envelopePhase ) {
                if( VOICE_RELEASE == voiceState ) {
                    continue; // next oscillator -- if all is end then voice down
                }
//++++++++++++++++++++++++++++
                goto L_sustain;
            }

            // check voiceState  = VOICE_RELEASE ?
            if( envelopeKnotRelease == stateOsc.envelopePhase ) { // release 7F
                stateOsc.envelopeTargetValueVelocity = 0 ;
                // at least 1 for release
                // normal release: input stateOsc.tickFrame == 0
                // fast release  : input stateOsc.tickFrame > 0 -- legato - envelopMultiplierExpChecked fix
                if( 0 == stateOsc.tickFrame ) { // if fastRelease then stateOsc.tickFrame == 1
                    stateOsc.tickFrame = toneshaper.tickFrameRelease.get(pitchDepDx);  // TODO interpolate
                    stateOsc.envelopDeltaDividerExp = oscillatorFrameSizeExp - toneshaper.tickFrameRelease.curveSpeed;
                } else {
                    stateOsc.envelopDeltaDividerExp = oscillatorFrameSizeExp + 1;
                }
                stateOsc.envelopePhase = 0; // next will  be -1 -- tricky
            } else {
                while( 0 == stateOsc.tickFrame ) {
                    changeEnvPhase = true;
                    stateOsc.tickFrame = toneshaper.transient[stateOsc.envelopePhase].tickFrame.get(pitchDepDx);

                    if( 0 < stateOsc.tickFrame )
                        break;
                    if( 0 > --stateOsc.envelopePhase ) {
//++++++++++++++++++++++++++++
                        goto L_sustain;
                    }
                }
                if( changeEnvPhase ) {
                    const auto& currentEnvelopeKnot  = toneshaper.transient[stateOsc.envelopePhase];
#ifdef TV_16bit
                    // envelope 30 bit -- reduce to int32 - always positive
                    stateOsc.envelopeTargetValueVelocity = ( int64_t(currentEnvelopeKnot.targetValue) * stateOsc.velocityBoosted ) >> 2; // 16+16-2 = 30
#else
                    // envelope 30 bit -- reduce to int32 - always positive
                    stateOsc.envelopeTargetValueVelocity = ( int64_t(currentEnvelopeKnot.targetValue) * stateOsc.velocityBoosted ) >> 18; // 32+16-18 = 30
#endif    
                    stateOsc.envelopDeltaDividerExp = oscillatorFrameSizeExp - currentEnvelopeKnot.tickFrame.curveSpeed;
                }
            } // end if( envelopeKnotRelease == stateOsc.envelopePhase )

            switch( stateOsc.tickFrame ) {
            case 0:
                break;
            case 1: // this is the last frame in this round
                deltaAmpl = (( stateOsc.envelopeTargetValueVelocity - stateOsc.amplitudoOsc )
                    >> oscillatorFrameSizeExp );
                --stateOsc.tickFrame;
                --stateOsc.envelopePhase;
                break;
            case 2: // this is almost last frame in this round ; divide by 2
                deltaAmpl = (( stateOsc.envelopeTargetValueVelocity - stateOsc.amplitudoOsc )
                    >> ( oscillatorFrameSizeExp + 1 ) );
                --stateOsc.tickFrame;
                break;
            default:
                deltaAmpl = (( stateOsc.envelopeTargetValueVelocity - stateOsc.amplitudoOsc ) / stateOsc.tickFrame )
                    >> stateOsc.envelopDeltaDividerExp;
                --stateOsc.tickFrame;
                break;
            }
//++++++++++++++++++++++++++++
            goto L_innerloop;
//============================
L_sustain:
            deltaAmpl = stateOsc.sustainModulator.decayMod( stateOsc.amplitudoOsc );
            ++stat.cycleCounter[Statistics::COUNTER_SUSTAIN];
//============================
L_innerloop:
            if( ( hearingThreshold > stateOsc.amplitudoOsc ) && ( 0 >= deltaAmpl )) {
                stateOsc.amplitudoOsc = 0;
                stateOsc.phase_0 += deltaPhase_0<<oscillatorFrameSizeExp; // no signal but move the phase by a tick
                ++stat.cycleCounter[Statistics::COUNTER_LOW_AMPLITUDE];
                continue; // next oscillator
            }

// adjust if negative because of the truncation to avoid going into minus
            if( 0 > deltaAmpl) {
                ++deltaAmpl;
            }
//---------------------------
            // envelope 30 bit
            // wave: 16 bit signed = 46 bit signed
            //  - 20 --> 26 bit
            // 256 osc -> +8 -> 34 bit
            // 256 voice  +8 -> 42 bit output

            int64_t amplitudoOsc = stateOsc.amplitudoOsc;
            uint32_t phase_0 = stateOsc.phase_0;
            out.amplitudeSumm[ outChannel_0 ] += amplitudoOsc;
            auto layp_0 = &out.layer[ outChannel_0 ][ 0 ];
            const auto outChannel_1 = (outChannel_0+1) & oscOutputChannelCountMsk;
            auto layp_1 = &out.layer[ outChannel_1 ][ 0 ]; // for 2CH tones

//---------------------------
            switch( oscillatorType ) {
            case ToneShaper::OSC_SIN:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    *layp_0++ +=  ( tables::waveSinTable[ uint16_t(((( phase_0 += deltaPhase_0 )) >> scalePhaseIndexExp))]
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
            case ToneShaper::OSC_SIN_2CH_PH:
                out.amplitudeSumm[ outChannel_1 ] = out.amplitudeSumm[ outChannel_0 ] ;
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint16_t ph0 = ( phase_0 += deltaPhase_0 ) >> scalePhaseIndexExp;
                    const uint16_t ph1 = ph0 + (1<<14); // PI / 2 shift
                    *layp_0++ +=  ( tables::waveSinTable[ ph0 ] * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    *layp_1++ +=  ( tables::waveSinTable[ ph1 ] * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
            case ToneShaper::OSC_SIN_2CH_FR:
                out.amplitudeSumm[ outChannel_1 ] = out.amplitudeSumm[ outChannel_0 ] ;
                deltaPhase_1 = deltaPhase_0 + frDetune0;
                deltaPhase_0 -= frDetune0;
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint16_t ph0 = ( phase_0 += deltaPhase_0 ) >> scalePhaseIndexExp;
                    const uint16_t ph1 = ( stateOsc.phase_1 += deltaPhase_1 ) >> scalePhaseIndexExp;
                    *layp_0++ +=  ( tables::waveSinTable[ ph0 ] * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    *layp_1++ +=  ( tables::waveSinTable[ ph1 ] * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;                
                
//---------------------------
            case ToneShaper::OSC_SINSIN:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    *layp_0++ += ( tables::waveSinTable[ uint16_t( tables::waveSinTable[ uint16_t(( phase_0 += deltaPhase_0 ) >> scalePhaseIndexExp)])]
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;

//---------------------------
            case ToneShaper::OSC_SINSIN_2CH_PH:
                out.amplitudeSumm[ outChannel_1 ] = out.amplitudeSumm[ outChannel_0 ] ;
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint16_t ph0 = ( phase_0 += deltaPhase_0 ) >> scalePhaseIndexExp;
                    const uint16_t ph1 = ph0 + (1<<14); // PI / 2 shift
                    *layp_0++ += ( tables::waveSinTable[ uint16_t( tables::waveSinTable[ ph0 ] )]
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    *layp_1++ += ( tables::waveSinTable[ uint16_t( tables::waveSinTable[ ph1 ] )]
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;

//---------------------------
            case ToneShaper::OSC_SINSIN_2CH_FR:
                out.amplitudeSumm[ outChannel_1 ] = out.amplitudeSumm[ outChannel_0 ] ;
                deltaPhase_1 = deltaPhase_0 + frDetune0;
                deltaPhase_0 -= frDetune0;
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint16_t ph0 = ( phase_0 += deltaPhase_0 ) >> scalePhaseIndexExp;
                    const uint16_t ph1 = ( stateOsc.phase_1 += deltaPhase_1 ) >> scalePhaseIndexExp;
                    *layp_0++ += ( tables::waveSinTable[ uint16_t( tables::waveSinTable[ ph0 ] )]
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    *layp_1++ += ( tables::waveSinTable[ uint16_t( tables::waveSinTable[ ph1 ] )]
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
            case ToneShaper::OSC_TRIANGLE:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    phase_0 += deltaPhase_0;
                    const int32_t a0 = int32_t(phase_0);
                    const int32_t triangle = ((a0>>31) ^ a0 ) - 0x3FFFFFFF;
                    *layp_0++ += ( triangle * amplitudoOsc ) >> ( scaleAmplitudeOscExp + 13 );
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
            case ToneShaper::OSC_TRIANGLE_2CH_PH:
                out.amplitudeSumm[ outChannel_1 ] = out.amplitudeSumm[ outChannel_0 ] ;
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint32_t ph0 = ( phase_0 += deltaPhase_0 );
                    const uint32_t ph1 = ph0 + (1<<30); // PI / 2 shift
                    const int32_t a0 = int32_t(ph0);
                    const int32_t triangle0 = ((a0>>31) ^ a0 ) - 0x3FFFFFFF;
                    const int32_t a1 = int32_t(ph1);
                    const int32_t triangle1 = ((a1>>31) ^ a1 ) - 0x3FFFFFFF;
                    *layp_0++ += ( triangle0 * amplitudoOsc ) >> ( scaleAmplitudeOscExp + 13 );
                    *layp_1++ += ( triangle1 * amplitudoOsc ) >> ( scaleAmplitudeOscExp + 13 );
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
            case ToneShaper::OSC_PD00:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint16_t phaseC = (phase_0 += deltaPhase_0 ) >> scalePhaseIndexExp;
                    *layp_0++ += ( tables::waveSinTable[ uint16_t(phaseC + (uint16_t( tables::waveSinTable[ phaseC ] ))) ]
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
            case ToneShaper::OSC_PD01:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint16_t phaseC = (phase_0 += deltaPhase_0 ) >> scalePhaseIndexExp;
                    *layp_0++ += ( tables::waveSinTable[ uint16_t(phaseC + (uint16_t( tables::waveSinTable[ phaseC ]>>1))) ]
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
            case ToneShaper::OSC_PD02:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint16_t phaseC = (phase_0 += deltaPhase_0 ) >> scalePhaseIndexExp;
                    *layp_0++ += ( tables::waveSinTable[ uint16_t(phaseC + (uint16_t( tables::waveSinTable[ phaseC ]>>2))) ]
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
            case ToneShaper::OSC_PD03:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint16_t phaseC = (phase_0 += deltaPhase_0 ) >> scalePhaseIndexExp;
                    *layp_0++ += ( tables::waveSinTable[ uint16_t(phaseC + (uint16_t( tables::waveSinTable[ phaseC ]>>3))) ]
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
            case ToneShaper::OSC_PD10:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint16_t phaseC = (phase_0 += deltaPhase_0 ) >> scalePhaseIndexExp;
                    const uint16_t phaseCPH = phase_0 >> 14;
                    *layp_0++ += ( tables::waveSinTable[ uint16_t(phaseC + (uint16_t( tables::waveSinTable[ phaseCPH ] ))) ]
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
            case ToneShaper::OSC_PD20:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint16_t phaseC = (phase_0 += deltaPhase_0 ) >> scalePhaseIndexExp;
                    const uint16_t phaseCPH = phase_0 >> 15;
                    *layp_0++ += ( tables::waveSinTable[ uint16_t(phaseC + (uint16_t( tables::waveSinTable[ phaseCPH ] ))) ]
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
            case ToneShaper::OSC_PD30:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint16_t phaseCPH = (phase_0 += deltaPhase_0 ) >> scalePhaseIndexExp;
                    const uint16_t phaseC = phase_0 >> 14;
                    *layp_0++ += ( tables::waveSinTable[ uint16_t(phaseC + (uint16_t( tables::waveSinTable[ phaseCPH ] ))) ]
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
#if 1
            case ToneShaper::OSC_PD40:
                deltaPhase_1 = deltaPhase_0 + frDetune0;
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint16_t phaseCPH = (phase_0 += deltaPhase_0 ) >> scalePhaseIndexExp;
                    const uint16_t phaseC = ( stateOsc.phase_1 += deltaPhase_1 ) >> scalePhaseIndexExp;
                    //const uint16_t phaseC = phase_0 >> 15;
                    *layp_0++ += ( tables::waveSinTable[ uint16_t(phaseCPH + (uint16_t( tables::waveSinTable[ phaseC ] >> 2 ))) ]
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
#else
            case ToneShaper::OSC_PD40:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint16_t phaseCPH = (phase_0 += deltaPhase_0 ) >> scalePhaseIndexExp;
                    const uint16_t phaseC = phase_0 >> 15;
                    *layp_0++ += ( tables::waveSinTable[ uint16_t(phaseC + (uint16_t( tables::waveSinTable[ phaseCPH ] ))) ]
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
#endif
                
//---------------------------
// check?                
            case ToneShaper::OSC_PDRED0:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint16_t phaseC = (phase_0 += deltaPhase_0 ) >> scalePhaseIndexExp;
                    *layp_0++ += ( tables::waveSinTable[ uint16_t(phaseC + (int16_t( noisePhaseMode.getRed() >> 10 ) )) ]
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
// check?                
            case ToneShaper::OSC_PDRED1:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint16_t phaseC = (phase_0 += deltaPhase_0 ) >> scalePhaseIndexExp;
                    *layp_0++ += ( tables::waveSinTable[ uint16_t(phaseC + (int16_t( noisePhaseMode.getRed() >> 11 ) )) ]
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
// check?                
            case ToneShaper::OSC_PDPURPLE0:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint16_t phaseC = (phase_0 += deltaPhase_0 ) >> scalePhaseIndexExp;
                    *layp_0++ += ( tables::waveSinTable[ uint16_t(phaseC + (int16_t( noisePhaseMode.getPurple() >> 8 ) )) ]
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
// check?                
            case ToneShaper::OSC_PDPURPLE1:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint16_t phaseC = ( phase_0 += deltaPhase_0 ) >> scalePhaseIndexExp;
                    *layp_0++ += ( tables::waveSinTable[ uint16_t(phaseC + (int16_t( noisePhaseMode.getPurple() >> 10 ) )) ]
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
// doubles                
            case ToneShaper::OSC_D12V11:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint32_t phaseC = ( phase_0 += deltaPhase_0 );
                    *layp_0++ += (( int32_t(tables::waveSinTable[ uint16_t(phaseC>>16) ]) + tables::waveSinTable[ uint16_t(phaseC>>15) ] )
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
            case ToneShaper::OSC_D12V12:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint32_t phaseC = ( phase_0 += deltaPhase_0 );
                    *layp_0++ += (( int32_t(tables::waveSinTable[ uint16_t(phaseC>>16) ]) + (tables::waveSinTable[ uint16_t(phaseC>>15) ] >> 1 ))
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;

//---------------------------
            case ToneShaper::OSC_D13V11:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint32_t phaseC = ( phase_0 += deltaPhase_0 );
                    const uint16_t phaseC1 = ( phaseC >> 16 );
                    const uint16_t phaseC3 = ( phaseC >> 15 ) + phaseC1;
                    *layp_0++ += (( int32_t(tables::waveSinTable[ phaseC1 ]) + tables::waveSinTable[ phaseC3 ] )
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
            case ToneShaper::OSC_D13V12:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint32_t phaseC = ( phase_0 += deltaPhase_0 );
                    const uint16_t phaseC1 = ( phaseC >> 16 );
                    const uint16_t phaseC3 = ( phaseC >> 15 ) + phaseC1;
                    *layp_0++ += (( int32_t(tables::waveSinTable[ phaseC1 ]) + (tables::waveSinTable[ phaseC3 ] >> 1))
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
            case ToneShaper::OSC_D23V11:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint32_t phaseC = ( phase_0 += deltaPhase_0 );
                    const uint16_t phaseC1 = ( phaseC >> 16 );
                    const uint16_t phaseC2 = ( phaseC >> 15 );
                    const uint16_t phaseC3 = phaseC2 + phaseC1;
                    *layp_0++ += (( int32_t(tables::waveSinTable[ phaseC2 ]) + tables::waveSinTable[ phaseC3 ] )
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
            case ToneShaper::OSC_D23V12:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint32_t phaseC = ( phase_0 += deltaPhase_0 );
                    const uint16_t phaseC1 = ( phaseC >> 16 );
                    const uint16_t phaseC2 = ( phaseC >> 15 );
                    const uint16_t phaseC3 = ( phaseC >> 15 ) + phaseC1;
                    *layp_0++ += (( int32_t(tables::waveSinTable[ phaseC2 ]) + (tables::waveSinTable[ phaseC3 ] >> 1))
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
// triples                
            case ToneShaper::OSC_T123V111:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint32_t phaseC = ( phase_0 += deltaPhase_0 );
                    const uint16_t phaseC1 = ( phaseC >> 16 );
                    const uint16_t phaseC2 = ( phaseC >> 15 );
                    const uint16_t phaseC3 = phaseC2 + phaseC1;
                    *layp_0++ += (( int32_t(tables::waveSinTable[ phaseC1 ]) 
                        + tables::waveSinTable[ phaseC2 ]
                        + tables::waveSinTable[ phaseC3 ] )
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
            case ToneShaper::OSC_T123V121:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint32_t phaseC = ( phase_0 += deltaPhase_0 );
                    const uint16_t phaseC1 = ( phaseC >> 16 );
                    const uint16_t phaseC2 = ( phaseC >> 15 );
                    const uint16_t phaseC3 = phaseC2 + phaseC1;
                    *layp_0++ += (( int32_t(tables::waveSinTable[ phaseC1 ]) 
                        + (tables::waveSinTable[ phaseC2 ]>>1)
                        + tables::waveSinTable[ phaseC3 ] )
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
            case ToneShaper::OSC_T123V112:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint32_t phaseC = ( phase_0 += deltaPhase_0 );
                    const uint16_t phaseC1 = ( phaseC >> 16 );
                    const uint16_t phaseC2 = ( phaseC >> 15 );
                    const uint16_t phaseC3 = phaseC2 + phaseC1;
                    *layp_0++ += (( int32_t(tables::waveSinTable[ phaseC1 ]) 
                        + tables::waveSinTable[ phaseC2 ]
                        + (tables::waveSinTable[ phaseC3 ]>>1) )
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
            case ToneShaper::OSC_T123V122:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint32_t phaseC = ( phase_0 += deltaPhase_0 );
                    const uint16_t phaseC1 = ( phaseC >> 16 );
                    const uint16_t phaseC2 = ( phaseC >> 15 );
                    const uint16_t phaseC3 = phaseC2 + phaseC1;
                    *layp_0++ += (( int32_t(tables::waveSinTable[ phaseC1 ]) 
                        + (tables::waveSinTable[ phaseC2 ]>>1)
                        + (tables::waveSinTable[ phaseC3 ]>>1) )
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
            case ToneShaper::OSC_T234V111:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint32_t phaseC = ( phase_0 += deltaPhase_0 );
                    const uint16_t phaseC1 = ( phaseC >> 16 );
                    const uint16_t phaseC2 = ( phaseC >> 15 );
                    const uint16_t phaseC3 = phaseC2 + phaseC1;
                    const uint16_t phaseC4 = ( phaseC >> 14 );
                    *layp_0++ += (( int32_t(tables::waveSinTable[ phaseC4 ]) 
                        + tables::waveSinTable[ phaseC2 ]
                        + tables::waveSinTable[ phaseC3 ] )
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
            case ToneShaper::OSC_T234V121:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint32_t phaseC = ( phase_0 += deltaPhase_0 );
                    const uint16_t phaseC1 = ( phaseC >> 16 );
                    const uint16_t phaseC2 = ( phaseC >> 15 );
                    const uint16_t phaseC3 = phaseC2 + phaseC1;
                    const uint16_t phaseC4 = ( phaseC >> 14 );
                    *layp_0++ += (( int32_t(tables::waveSinTable[ phaseC4 ]) 
                        + (tables::waveSinTable[ phaseC3 ]>>1)
                        + tables::waveSinTable[ phaseC2 ] )
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
            case ToneShaper::OSC_T234V112:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint32_t phaseC = ( phase_0 += deltaPhase_0 );
                    const uint16_t phaseC1 = ( phaseC >> 16 );
                    const uint16_t phaseC2 = ( phaseC >> 15 );
                    const uint16_t phaseC3 = phaseC2 + phaseC1;
                    const uint16_t phaseC4 = ( phaseC >> 14 );
                    *layp_0++ += (( int32_t(tables::waveSinTable[ phaseC3 ]) 
                        + tables::waveSinTable[ phaseC2 ]
                        + (tables::waveSinTable[ phaseC4 ]>>1) )
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
            case ToneShaper::OSC_T234V122:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    const uint32_t phaseC = ( phase_0 += deltaPhase_0 );
                    const uint16_t phaseC1 = ( phaseC >> 16 );
                    const uint16_t phaseC2 = ( phaseC >> 15 );
                    const uint16_t phaseC3 = phaseC2 + phaseC1;
                    const uint16_t phaseC4 = ( phaseC >> 14 );
                    *layp_0++ += (( int32_t(tables::waveSinTable[ phaseC2 ]) 
                        + (tables::waveSinTable[ phaseC4 ]>>1)
                        + (tables::waveSinTable[ phaseC3 ]>>1) )
                            * amplitudoOsc ) >> scaleAmplitudeOscExp;
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------

//---------------------------

//---------------------------
                
//-----------------------------------------------------------------------------
// noise: 1 wide for a voice !!!!!!!!!!!!!!!!
// only 1 for a voice !!!
// this will not be checked in this place -- parameter setting must ensure
// the wide has own galoisshifter -- uncorrelated
// check RED1..3 PURPLE1..3 BLUE 1..3
// TODO stereo
            case ToneShaper::OSC_NOISE_WHITE:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    *layp_0++ +=  ( noiseWide.getWhite() * amplitudoOsc ) >> (scaleAmplitudeOscExp+8);
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
// TODO stereo
            case ToneShaper::OSC_NOISE_RED1:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    *layp_0++ +=  ( noiseWide.getRed() * amplitudoOsc ) >> (scaleAmplitudeOscExp+9);
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
// TODO stereo
            case ToneShaper::OSC_NOISE_PURPLE1:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    *layp_0++ +=  ( noiseWide.getPurple() * amplitudoOsc ) >> (scaleAmplitudeOscExp+7);
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
// TODO stereo
                // TODO add diff zero
            case ToneShaper::OSC_NOISE_BLUE1:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    *layp_0++ +=  ( noiseWide.getBlue() * amplitudoOsc ) >> (scaleAmplitudeOscExp+6);
                    amplitudoOsc += deltaAmpl;
                }
                break;

//-----------------------------------------------------------------------------
// PEEK filters : uses the common NoiseFrame -- correlated
// TODO : 1 filter / voice or 1 filter / overtone ?
// TODO : default norm right shift must be tested -- mainly for boosting filters
//---------------------------
// TODO stereo
            case ToneShaper::OSC_NOISE_SV1x1_PEEK:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    *layp_0++ +=  ( noiseNarrow.getFilteredSVx1<0>( whiteNoiseFrame.getFrame()[sind], filterCenterFreq ) * amplitudoOsc ) >> (scaleAmplitudeOscExp+8);
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
// TODO stereo
            case ToneShaper::OSC_NOISE_SV2x1_PEEK:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    *layp_0++ +=  ( noiseNarrow.getFilteredSVx1<1>( whiteNoiseFrame.getFrame()[sind], filterCenterFreq ) * amplitudoOsc ) >> (scaleAmplitudeOscExp+8);
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
// TODO stereo
            case ToneShaper::OSC_NOISE_SV3x1_PEEK:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    *layp_0++ +=  ( noiseNarrow.getFilteredSVx1<2>( whiteNoiseFrame.getFrame()[sind], filterCenterFreq ) * amplitudoOsc ) >> (scaleAmplitudeOscExp+8);
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
// TODO stereo
            case ToneShaper::OSC_NOISE_SV1x2_PEEK:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    *layp_0++ +=  ( noiseNarrow.getFilteredSVx2<0>( whiteNoiseFrame.getFrame()[sind], filterCenterFreq ) * amplitudoOsc ) >> (scaleAmplitudeOscExp+8);
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
// TODO stereo
            case ToneShaper::OSC_NOISE_SV2x2_PEEK:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    *layp_0++ +=  ( noiseNarrow.getFilteredSVx2<1>( whiteNoiseFrame.getFrame()[sind], filterCenterFreq ) * amplitudoOsc ) >> (scaleAmplitudeOscExp+8);
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
// TODO stereo
            case ToneShaper::OSC_NOISE_SV3x2_PEEK:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    *layp_0++ +=  ( noiseNarrow.getFilteredSVx2<2>( whiteNoiseFrame.getFrame()[sind], filterCenterFreq ) * amplitudoOsc ) >> (scaleAmplitudeOscExp+8);
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
// TODO stereo
            case ToneShaper::OSC_NOISE_SV1x3_PEEK:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    *layp_0++ +=  ( noiseNarrow.getFilteredSVx3<0>( whiteNoiseFrame.getFrame()[sind], filterCenterFreq ) * amplitudoOsc ) >> (scaleAmplitudeOscExp+8);
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
// TODO stereo
            case ToneShaper::OSC_NOISE_SV2x3_PEEK:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    *layp_0++ +=  ( noiseNarrow.getFilteredSVx3<1>( whiteNoiseFrame.getFrame()[sind], filterCenterFreq ) * amplitudoOsc ) >> (scaleAmplitudeOscExp+8);
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
// TODO stereo
            case ToneShaper::OSC_NOISE_SV3x3_PEEK:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    *layp_0++ +=  ( noiseNarrow.getFilteredSVx3<2>( whiteNoiseFrame.getFrame()[sind], filterCenterFreq ) * amplitudoOsc ) >> (scaleAmplitudeOscExp+10);
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
// TODO stereo
            case ToneShaper::OSC_NOISE_SV1x4_PEEK:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    *layp_0++ += ( noiseNarrow.getFilteredSVx4<0>( whiteNoiseFrame.getFrame()[sind], filterCenterFreq ) * amplitudoOsc ) >> (scaleAmplitudeOscExp+8);
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
// TODO stereo
            case ToneShaper::OSC_NOISE_SV2x4_PEEK:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    *layp_0++ += ( noiseNarrow.getFilteredSVx4<1>( whiteNoiseFrame.getFrame()[sind], filterCenterFreq ) * amplitudoOsc ) >> (scaleAmplitudeOscExp+8);
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
// TODO stereo
            case ToneShaper::OSC_NOISE_SV3x4_PEEK:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    *layp_0++ += ( noiseNarrow.getFilteredSVx4<2>( whiteNoiseFrame.getFrame()[sind], filterCenterFreq ) * amplitudoOsc ) >> (scaleAmplitudeOscExp+12);
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
// TODO stereo
            case ToneShaper::OSC_NOISE_APx1_PEEK:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    *layp_0++ += ( noiseNarrow.getFilteredAP2x1( whiteNoiseFrame.getFrame()[sind], filterCenterFreq, filterBandwith ) * amplitudoOsc )
                                >> (scaleAmplitudeOscExp+5);
                    amplitudoOsc += deltaAmpl;
                }
                break;

//---------------------------
// TODO stereo
            case ToneShaper::OSC_NOISE_APx2_PEEK:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    *layp_0++ += ( noiseNarrow.getFilteredAP2x2( whiteNoiseFrame.getFrame()[sind], filterCenterFreq, filterBandwith ) * amplitudoOsc )
                                >> (scaleAmplitudeOscExp+5);
                    amplitudoOsc += deltaAmpl;
                }
                break;

//---------------------------
// TODO stereo
            case ToneShaper::OSC_NOISE_APx3_PEEK:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    *layp_0++ += ( noiseNarrow.getFilteredAP2x3( whiteNoiseFrame.getFrame()[sind], filterCenterFreq, filterBandwith ) * amplitudoOsc )
                                >> (scaleAmplitudeOscExp+5);
                    amplitudoOsc += deltaAmpl;
                }
                break;

//---------------------------
// TODO stereo
            case ToneShaper::OSC_NOISE_APx4_PEEK:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    *layp_0++ += ( noiseNarrow.getFilteredAP2x4( whiteNoiseFrame.getFrame()[sind], filterCenterFreq, filterBandwith ) * amplitudoOsc )
                                >> (scaleAmplitudeOscExp+5);
                    amplitudoOsc += deltaAmpl;
                }
                break;

//---------------------------
// TODO stereo
            case ToneShaper::OSC_NOISE_4Px1_PEEK:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    *layp_0++ +=  ( noiseNarrow.getFiltered4Px1( whiteNoiseFrame.getFrame()[sind], filterCenterFreq, filterBandwith ) * amplitudoOsc )
                                >> (scaleAmplitudeOscExp+5);
                    amplitudoOsc += deltaAmpl;
                }
                break;
                
//---------------------------
// TODO stereo
            case ToneShaper::OSC_NOISE_4Px2_PEEK:
                for( auto sind = 0; sind < oscillatorFrameSize; ++sind ) {
                    *layp_0++ +=  ( noiseNarrow.getFiltered4Px2( whiteNoiseFrame.getFrame()[sind], filterCenterFreq, filterBandwith ) * amplitudoOsc )
                                >> (scaleAmplitudeOscExp+5);
                    amplitudoOsc += deltaAmpl;
                }
                break;
            } // end switch( oscillatorType )
           
            stateOsc.amplitudoOsc = amplitudoOsc;
            stateOsc.phase_0 = phase_0;
            ++stat.cycleCounter[Statistics::COUNTER_INNER_LOOP];
            isEnd = false;
        }
        if( VOICE_RELEASE == voiceState && isEnd ) {
//            std::cout << "-- VOICE_DOWN"  << std::endl;
            voiceState = VOICE_DOWN;
        }
        ++stat.cycleCounter[Statistics::COUNTER_OUTER_LOOP];
        return true;
    }
    return false; 
} // end Oscillator::generate(  const OscillatorInGenerate& in,  OscillatorOut& out );
//======================================================================================================================
// --------------------------------------------------------------------
// if runningCount > limit then delay the new voice by a cycle
// void Oscillator::voiceUp( uint16_t envBankSel, uint16_t spectBankSel, uint16_t runningCount, int16_t velocity )

void Oscillator::voiceRun( const OscillatorInChange& in )
{
#if 0
    if( VOICE_DOWN != voiceState ) {
        return;
    }
#endif
    ++runCount;
    toneShaperSelect    = in.toneShaperSelect; // direct from router
    if( toneShaperSelect >= settingVectorSize ) {
      toneShaperSelect = settingVectorSize-1;
    }
    toneShaperVecCurr   = &toneShaperMatrix.toneShapers[ toneShaperSelect ];
    oscillatorCountUsed = toneShaperVecCurr->oscillatorCountUsed;

    if(( in.oscillatorCountUsed > 0 ) && ( in.oscillatorCountUsed < oscillatorCountUsed )) {
        oscillatorCountUsed = in.oscillatorCountUsed;
    }
    targetPitch = basePitch = in.pitch;
    deltaPitch = 0;
    glissandoTick = 0;
    setPitchDependency();

    // ------------ monitoring
    const double dph  = ExpTable::ycent2deltafi( basePitch, 0 );
    const double freq = samplingFrequency * dph / (1LL<<32) ;
    std::cout << std::dec
        << "\n  ***  depdx " << pitchDepDx
        << " tick "     << toneShaperVecCurr->toneShaper[0].tickFrameRelease.get(pitchDepDx)
        << " decay "    << toneShaperVecCurr->toneShaper[0].sustain.decayCoeff.get(pitchDepDx)
        << " depth "    << toneShaperVecCurr->toneShaper[0].sustain.modDepth
        << " delta "    << toneShaperVecCurr->toneShaper[0].sustain.modDeltaPhase
        << " freq "     << freq
        << " dph "      << dph
        << std::endl;
    // ------------

    const uint16_t velo = uint16_t(in.velocity) << 8;
    velocity = velo; // probably not needed

//    noiseWide.clear();
//    noiseNarrow.clear();

// to generate
// only for sine type - noise: switch( oscillatorType )
// wide noise : no pitch
// narrow noise: pitch from filter type table
    noiseNarrow.clear();
//    noiseNarrow.setFreqSv(basePitch);
    for( auto oscindex = 0; oscindex < oscillatorCountUsed; ++oscindex ) {
        auto& stateOsc = state[ oscindex ];
        auto& ts = toneShaperVecCurr->toneShaper[ oscindex ];
#ifdef OSCILLATOR_PHASE_RANDOMIZER
        // if voice is not running !!
        stateOsc.phase_0 = gRandom.getRaw();
#endif
        stateOsc.sustainModulator.reset( ts.sustain, pitchDepDx );
        stateOsc.tickFrame          = 0;                // feature: must be set to 0
        stateOsc.envelopePhase      = transientKnotCount-1;   // down count
//        stateOsc.releaseEnd         = false;
        // TODO check characeristic
        const uint16_t bv =  ts.veloBoost ? velo - tables::VelocityBoostTable::getBoost( in.velocity, ts.veloBoost ) : velo;
//        const uint16_t bv =  booster ? velo - getVelocityBoostXX( in.velocity, booster ) : velo;
        stateOsc.velocityBoosted = bv; //-- changed the oscillator !!!! 
//        std::cout << std::dec << " --- bv: " << bv << std::endl;
    }
    voiceState          = VOICE_RUN;
}
// --------------------------------------------------------------------
void Oscillator::voiceRelease( const OscillatorInChange& in )
{
    switch(voiceState) {
    case VOICE_RELEASE:
    case VOICE_DOWN:
        return;
    default:
        break;        
    }
    --runCount;
    std::cout << "voiceRelease: " << runCount << std::endl;

    if( runCount > 0 ) {
        return;
    } else if ( runCount < 0 ) {
        runCount = 0;
    }

    voiceState  = VOICE_RELEASE;
    for( auto oscindex = 0; oscindex < oscillatorCountUsed; ++oscindex ) {
        auto& stateOsc = state[ oscindex ];
        stateOsc.tickFrame      = in.tickFrameRelease;  // used if not 0 -- legato 1

        // check other solution ?
        stateOsc.envelopePhase  = envelopeKnotRelease;
    }
} // end Oscillator::voiceRelease( const OscillatorInChange& in )

void Oscillator::voiceChange( const OscillatorInChange& in )
{
    switch(voiceState) {
    case VOICE_RELEASE:
    case VOICE_DOWN:
        return;

    default:
        break;
    }

    std::cout << "voiceChange: " << std::endl;
    // setGlissando(  .... )

} // end Oscillator::voiceRelease( const OscillatorInChange& in )

// --------------------------------------------------------------------
void Oscillator::voiceDelay( const OscillatorInChange& in )
{
    delay = in.delay;
} // end Oscillator::voiceRelease( const OscillatorInChange& in )
// --------------------------------------------------------------------
} // end namespace yacynth



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
 * File:   Oscillator.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on January 28, 2016, 5:20 PM
 */
#include    "yacynth_globals.h"
#include    "OscillatorOutput.h"
#include    "Tables.h"
#include    "ToneShaper.h"
#include    "NoiseFrame.h"
#include    "NoiseSample.h"
#include    "OscillatorNoiseInt.h"

#include    "../control/Statistics.h"
#include    "../utils/GaloisNoiser.h"
#include    "../utils/Fastsincos.h"
#include    "../oscillator/ToneShaperMatrix.h"

#include    <cstdint>
#include    <iostream>
#include    <random>
#include    <atomic>
#include    <tgmath.h>

//
// TODO:
//  output mapper -- which overtone which outputChannel??
//  per oscillator map - pass by voiceUp
//
using namespace noiser;
using namespace tables;

namespace yacynth {
// --------------------------------------------------------------------
struct OscillatorInGenerate {
    OscillatorInGenerate()
    :   pitchDelta(0)
    {};
    int32_t pitchDelta;  // global delta -- log relative frequency
};
// --------------------------------------------------------------------
struct OscillatorInChange {
    OscillatorInChange()
    :   velocity(0)
    ,   tickFrameRelease(0)
    ,   toneShaperSelect(0)
    ,   pitch(0)
    ,   oscillatorCountUsed(0)
    {};
    
    uint32_t    pitch;
    uint16_t    velocity;           // linear velocity
    uint16_t    tickFrameRelease;   // for Release for direct input
    uint16_t    toneShaperSelect; // select the envelope table
    uint16_t    delay;              // set delay
    uint16_t    oscillatorCountUsed;
    uint8_t     outputChannel;      // considering to change the current solution apply diff effects
};
// --------------------------------------------------------------------
struct SustainModulator {
    inline void reset( const AmplitudeSustain& env, int16_t pitchDepDx )
    {
        phase           = 0;
        deltaAmplMod    = 0;
        sign            = -1;
        phaseDelta      = env.modDeltaPhase;
        modDepth        = env.modDepth;
        decayRate       = env.decayCoeff.get( pitchDepDx );
    }
           
    inline int64_t decayMod( const int64_t in )
    {
        constexpr uint8_t scaleDecay = 20+6;
        constexpr uint8_t scaleMod  = 21+6;
        if( 0 == decayRate ) {
            return 0;
        }
        phase += phaseDelta;
        const int8_t signOld = sign;
        sign = ( int16_t(phase) >> 15 ) | 1;
        switch( sign - signOld ) {
        case 2:
            deltaAmplMod = ( modDepth * in ) >> scaleMod;
            // phaseDelta += in & 15; // randomizer -- experimental
            break;
        case -2:
            deltaAmplMod = -deltaAmplMod;
            break;
        }
        const int64_t deltaAmplDecay = ( decayRate * in ) >> scaleDecay;
        return 1 + deltaAmplMod - deltaAmplDecay;
    }

    int64_t     deltaAmplMod; //  can be i32
    uint16_t    decayRate;
    uint16_t    modDepth;
    uint16_t    phaseDelta;
    uint16_t    phase;
    int8_t      sign;
};
// --------------------------------------------------------------------
struct alignas(int64_t) OscillatorState { // or cacheLineSize ?

    int64_t             amplitudoOsc;                   // ok - must be signed to handle underflows can be i32
    int64_t             envelopeTargetValueVelocity;    // ok - must be signed? to handle underflows can be i32
    uint32_t            phase;
    int32_t             tickFrame;  // int16_t ???
    int32_t             amplitudeDetunePitch;
    uint16_t             envelopDeltaDividerExp;
    int8_t              envelopePhase;
//    int8_t              rfu1;
    SustainModulator    sustainModulator;
};
// --------------------------------------------------------------------
class alignas(16) Oscillator {
public:
    static constexpr int32_t minPitchDep = 0x16000000;  // j 30 freq2ycent 155b2c3e

    enum OscType {
        OSC_SIN,
        OSC_SINSIN,  // waveSinTable[ uint16_t( waveSinTable[phase >>16] )]

        OSC_PD00,   // phase distorsion0 waveSinTable[ uint16_t((waveSinTable[phase >>16]) + (phase >>16))]
        OSC_PD01,   // phase distorsion1 waveSinTable[ uint16_t((waveSinTable[phase >>16]>>1) + (phase >>16))]
        OSC_PD02,   // phase distorsion2 waveSinTable[ uint16_t((waveSinTable[phase >>16]>>2) + (phase >>16))]
        OSC_PD03,   // phase distorsion3 waveSinTable[ uint16_t((waveSinTable[phase >>16]>>3) + (phase >>16))]

        OSC_12OV0,  // tone 1 + 2 : (waveSinTable[uint16_t(phase>>16)]+(waveSinTable[uint16_t(phase>>15)]));};
        OSC_12OV1,  // tone 1 + 2 : (waveSinTable[uint16_t(phase>>16)]+(waveSinTable[uint16_t(phase>>15)]>>1));};
        OSC_12OV2,  // tone 1 + 2 : (waveSinTable[uint16_t(phase>>16)]+(waveSinTable[uint16_t(phase>>15)]>>2));};

        OSC_13OV0,  // tone 1 + 3 : (waveSinTable[uint16_t(phase>>16)]+(waveSinTable[uint16_t(phase>>14)]));};
        OSC_13OV1,  // tone 1 + 3 : (waveSinTable[uint16_t(phase>>16)]+(waveSinTable[uint16_t(phase>>14)]>>1));};
        OSC_13OV2,  // tone 1 + 3 : (waveSinTable[uint16_t(phase>>16)]+(waveSinTable[uint16_t(phase>>14)]>>2));};

        // wide noise
        OSC_NOISE_WHITE   = 0x80,
        OSC_NOISE_RED,
        OSC_NOISE_PURPLE,
        OSC_NOISE_BLUE,

        // narrow noise - correlated (common input)
        OSC_NOISE_PEEK1,
        OSC_NOISE_PEEK2,
        OSC_NOISE_PEEK3,
        OSC_NOISE_PEEK4,

        // check: experimental
        OSC_SIN_MULT_RED_NOISE,

    };


    enum voice_state_t : uint8_t {
        VOICE_DOWN,
        VOICE_RELEASE,
        VOICE_RUN,
        VOICE_DELAY_RUN,        // ?????

//        VOICE_UP,
//        VOICE_UP_PREPARE,

//        VOICE_LEGATO_UP,            // legato ADS
//        VOICE_LEGATO_UP_REPARE,     // legato ADS
//        VOICE_LEGATO_DOWN,          // legato Release
//        VOICE_LEGATO_DOWN_PREPARE,  // legato Release

//        VOICE_AFTERTOUCH,           // aftertouch ADS
//        VOICE_SUSTAIN,
//        VOICE_TEST,
    };

    Oscillator();
    void clear(void) {};
    static ToneShaperMatrix& getToneShaperMatrix(void)
        { return const_cast<ToneShaperMatrix&>(toneShaperMatrix); };
    void        initialize( void );
    bool        generate(           const OscillatorInGenerate& in,  OscillatorOut& out, Statistics& stat );
    void        voiceRun(           const OscillatorInChange& in );
    void        voiceRelease(       const OscillatorInChange& in );
    void        voiceChange(        const OscillatorInChange& in );
    void        voiceDelay(         const OscillatorInChange& in );
    std::size_t sizeVector(void)    const { overtoneCountOscDef; };

    inline static void fillWhiteNoise(void)
    {
        whiteNoiseFrame.fillWhiteBlue(); // fillWhiteBlue ??
    }

private:

    inline void setPitchDependency(void)
    {
        // pitch:
        // bit 0..23 - in octave (24 bits)
        // bit 24..31 - octave number
        // here 8 octave is used -- 3 bit
        //  13 bit from in octave part
        //  24-13 = 11
        //
/*
  j 27 freq2ycent 15344291
  j 28 freq2ycent 1541b112
  j 29 freq2ycent 154ea6e7
  j 30 freq2ycent 155b2c3e
  j 31 freq2ycent 15674878
  j 32 freq2ycent 15730242
 */
        constexpr uint8_t   prExp = 11;
        constexpr int32_t   maxPd = 0x0F000U;
        const int32_t dp =  basePitch - minPitchDep;
        if( dp <= 0 ) {
            pitchDepDx  = 0;
            return;
        }
        if( dp >= (maxPd<<prExp) ) {
            pitchDepDx  = maxPd;
            return;
        }
        pitchDepDx  = dp >> prExp;
    }

    const ToneShaperVector        * toneShaperVecCurr;
    OscillatorState                 state[ overtoneCountOscDef ];
    NoiseSample                     noiseWide;      // only 1 for a voice
    OscillatorNoise                 noiseNarrow;    // only 1 for a voice
    int32_t                         basePitch;
    uint16_t                        velocity;
    uint16_t                        delay;
    uint16_t                        toneShaperSelect;
    uint16_t                        oscillatorCountUsed;
    uint16_t                        pitchDepDx;
    voice_state_t                   voiceState;
    // common for all voices - correlated narrow band parallel filters by noiseNarrow
    static NoiseFrame<FrameInt<oscillatorOutSampleCountExp>>
                                    whiteNoiseFrame;
    static const ToneShaperMatrix   toneShaperMatrix;
}; // end class Oscillator
// --------------------------------------------------------------------
} // end namespace yacynth


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
#include "yacynth_globals.h"
#include "OscillatorOutput.h"
#include "Tables.h"
#include "ToneShaper.h"
#include "NoiseFrame.h"
#include "NoiseSample.h"
#include "OscillatorNoiseInt.h"

#include "control/Statistics.h"
#include "utils/GaloisNoiser.h"
#include "utils/Fastsincos.h"
#include "oscillator/ToneShaperMatrix.h"

#include <cstdint>
#include <iostream>
#include <random>
#include <atomic>
#include <tgmath.h>

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
    :   pitch(0)
    ,   tickFrameRelease(0)
    ,   toneShaperSelect(0)
    ,   delay(0)
    ,   oscillatorCountUsed(0)
    ,   outputChannel(0)
    ,   velocity(0)
    {};

    uint32_t    pitch;
    uint16_t    tickFrameRelease;   // for Release for direct input
    uint16_t    toneShaperSelect;   // select the envelope table
    uint16_t    delay;              // set delay
    uint16_t    oscillatorCountUsed;
    uint8_t     outputChannel;      // considering to change the current solution apply diff effects
    uint8_t     velocity;           // linear velocity
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

    int64_t             amplitudoOsc;                   // must be signed to handle underflows can be i32
    int64_t             envelopeTargetValueVelocity;    // can be i32
    OscillatorNoise     noiseNarrow;                    // 1x overtone ?
    uint32_t            phase_0;
    uint32_t            phase_1;
    int32_t             tickFrame;  // int16_t ???
    uint16_t            velocityBoosted;
    uint8_t             envelopDeltaDividerExp;
    int8_t              envelopePhase;
    SustainModulator    sustainModulator;
};
// --------------------------------------------------------------------
class alignas(16) Oscillator {
public:
    static constexpr int32_t minPitchDep = 0x16000000;  // j 30 freq2ycent 155b2c3e

    enum voice_state_t : uint8_t {
        VOICE_DOWN,
        VOICE_RELEASE,
        VOICE_RUN,
        VOICE_DELAY_RUN,
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
    std::size_t sizeVector(void)    const { return overtoneCountOscDef; };

    inline static void fillWhiteNoise(void)
    {
        whiteNoiseFrame.fillWhiteLowCut();
    }

    inline void setGlissando( int32_t targetPitchP, int16_t deltaTick )
    {
        if( deltaTick > 0 ) {
            glissandoTick = deltaTick;
            targetPitch = targetPitchP;
            deltaPitch  = ( targetPitchP - basePitch  ) / deltaTick;
            return;
        }
        glissandoTick = 0;
    }
#if 0
    // plot2d(  [ "x * 256", "x * 256 + (( (x-128) * (x-128) - 16384 ) * 255) / 256 " ],  1, 255 ,color=4:5  )
    // velocity = 2 * midi velocity
    // booster from TS
    // TODO try with exp function
    static inline uint16_t getVelocityBoostXX( uint8_t velocity, uint8_t booster )
    {
        const int32_t v1 = int32_t(velocity) - (1<<7);
        return - ((( v1 * v1 - (1<<14) ) * booster ) >> 8 );
    }
#endif
    
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
    NoiseSample1CH                  noiseWide;          // only 1 for a voice
    NoiseSample1CH                  noisePhaseMode;     // only 1 for a voice -- TODO recheck
    OscillatorNoise                 noiseNarrow;        // only 1 for a voice -- or 1x overtone -- TODO 
    int32_t                         basePitch;
    int32_t                         targetPitch;        // for glissando
    int32_t                         deltaPitch;         // for glissando
    uint16_t                        glissandoTick;
    uint16_t                        velocity;
    uint16_t                        delay;
    uint16_t                        toneShaperSelect;
    uint16_t                        oscillatorCountUsed;
    uint16_t                        pitchDepDx;
    int16_t                         runCount;   // if multirun 
    voice_state_t                   voiceState;
    // common for all voices - correlated narrow band parallel filters by noiseNarrow
    static NoiseFrame<FrameInt<oscillatorFrameSizeExp>>
                                    whiteNoiseFrame;
    static const ToneShaperMatrix   toneShaperMatrix;
//    static const OvertoneMatrix   overtoneMatrix;
}; // end class Oscillator
// --------------------------------------------------------------------
} // end namespace yacynth


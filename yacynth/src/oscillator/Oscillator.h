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
    int32_t                 pitchDelta;     // global delta -- log relative frequency
};
// --------------------------------------------------------------------
struct OscillatorInChange {
    OscillatorInChange()
    :   velocity(0)
    ,   tickFrameRelease(0)
    ,   toneShaperSelect(0)
    ,   pitch(0)
    {};
    uint32_t    pitch;
    uint16_t    velocity;           // linear velocity
    uint16_t    tickFrameRelease;   // for Release for direct input
    uint16_t    toneShaperSelect; // select the envelope table
    uint16_t    delay;              // set delay
    uint8_t     outputChannel;      // considering to change the current solution apply diff effects
};
// --------------------------------------------------------------------

// --------------------------------------------------------------------

struct SustainModulator {
    inline void reset(void)
    {
        period      = 0;
    }
    inline int64_t decay ( const int64_t in, const AmplitudeSustain& env )
    {
        return env.decayCoeff ? -(( ( env.decayCoeff * in ) >> 28 ) + 1 ) : 0 ;
    }
    inline int64_t mod( const int64_t in, const AmplitudeSustain& env, const uint64_t gShifter )
    {
        if( 0 == env.sustainModDepth ) {
            return decay( in, env );
        }
        if( 0 >= --phase ) {
            //
            // MODULATOR: first goes up - sounds better
            //  sustain level generally less than the half of the peek
            //  may overflow in the worst case because the envelope peek is 48 bit
            //  this NEVER go up more 48 because the oscillator multiplies by 16 -> 64
            //  never use max value for sustain -> ToneShaper.check()
            //
            if( ++period & 1 ) {
                phaseDelta  = env.sustainModPeriod;
                switch( env.sustainModType ) {
                case env.MODTYPE_RAND1: // default
                    phaseDelta += ((( period ^ 0xFFFA ) >> 1 ) & 7 ) // "A" looks better
                        + ((( uint8_t( gShifter >> 25 ) * env.sustainModPeriod ) >> 10 ) & 0x0FC ); // 25 looks better
                    break;

                case env.MODTYPE_RAND2: // experimental ---
                    phaseDelta += ((( period ^ 0xFFFA ) >> 1 ) & 7 )
                        + ((( uint8_t( gShifter >> 25 ) * env.sustainModPeriod ) >> 9 ) & 0x08C );
                    break;

                case env.MODTYPE_RAND3: // experimental ---
                    phaseDelta += ((( period ^ 0xFFFA ) >> 1 ) & 7 )
                        + ((( uint8_t( gShifter >> 25 ) * env.sustainModPeriod ) >> 9 ) & 0x08C );
                    break;
                }
                // depth = sustainModDepth  / 256
                amplDelta   = (( in * env.sustainModDepth ) / phaseDelta )
                                    >> ( oscillatorOutSampleCountExp + 8 );
                amplDecay   = decay( in, env );
                phase = phaseDelta;
            } else {
                phase       = phaseDelta;
                amplDelta   = -amplDelta;
            }
        }
        return amplDelta - amplDecay;
    }
    int64_t             amplDelta;
    int64_t             amplDecay;
    int16_t             phaseDelta;
    int16_t             phase;
    uint16_t            period;
    uint16_t            sustainModPeriod;       // needed here - init by voiceUp for speeding
    uint8_t             sustainModDeltaCount;   // needed here - init by voiceUp for speeding
};
// --------------------------------------------------------------------
struct OscillatorState {
    int64_t             amplitudoOsc;                   // ok - must be signed to handle underflows
    int64_t             envelopeTargetValueVelocity;    // ok - must be signed to handle underflows
    uint32_t            phase;
    int32_t             tickFrame;
    uint16_t            velocity;                       // layer velocity: set by note on
    uint16_t            envelopMultiplierExpChecked;
    int16_t             envelopePhase;                  // run max..0  , -1 end
    SustainModulator    sustainModulator;
};
// --------------------------------------------------------------------
class alignas(16) Oscillator {
public:
    static constexpr uint32_t minPitchDep = 0x155b2c3e;       // freq2ycent( 40.0 ) should be about 40 Hz
    static constexpr uint32_t maxPitchDep = 0x1cd91653;       // freq2ycent( 6144.0 ) should be about 5 kHz -- 5400 Hz

    enum OscType {
        OSC_SIN     = 0,
        OSC_SINSIN,         // waveSinTable[ uint16_t( waveSinTable[phase >>16] )]<<15;};

        // TODO
        OSC_PD00,   // phase distorsion0 - sin(sin()) waveSinTable[ uint16_t((waveSinTable[phase >>16]>>1) + (phase >>16))]<<15;};
        OSC_PD01,   // phase distorsion1 - sin(sin()) waveSinTable[ uint16_t((waveSinTable[phase >>16]>>1) + (phase >>16))]<<15;};
        OSC_PD02,   // phase distorsion2 - sin(sin()) waveSinTable[ uint16_t((waveSinTable[phase >>16]>>2) + (phase >>16))]<<15;};
        OSC_PD03,   // phase distorsion3 - sin(sin()) waveSinTable[ uint16_t((waveSinTable[phase >>16]>>3) + (phase >>16))]<<15;};

        OSC_12OV0,  // tone 1 + 2 : (waveSinTable[uint16_t(phase>>16)]+(waveSinTable[uint16_t(phase>>15)]));};
        OSC_12OV1,  // tone 1 + 2 : (waveSinTable[uint16_t(phase>>16)]+(waveSinTable[uint16_t(phase>>15)]>>1));};
        OSC_12OV2,  // tone 1 + 2 : (waveSinTable[uint16_t(phase>>16)]+(waveSinTable[uint16_t(phase>>15)]>>2));};

        // multiply by wave ???
        OSC_SIN_RING,
        OSC_SINSIN_RING,
//        OSC_WAVE1   = 1,    // user fillable tables
//        OSC_WAVE2   = 2,    // user fillable tables
//        OSC_WAVE3   = 3,    // user fillable tables
        OSC_WHITE_NOISE   = 0x80,
        OSC_RED_NOISE,
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
    void        voiceUp(            const OscillatorInChange& in );
    void        voiceRelease(       const OscillatorInChange& in );
    void        voiceDelay(         const OscillatorInChange& in );
    std::size_t sizeVector(void)    const { overtoneCountOscDef; };

private:
    inline void setPitchDependency(void)
    {
        if( basePitch >= maxPitchDep ) {    // high freq-> no dep - tick[15]
            pitchDepIndex = 15;
            pitchDepDx  = 0;
            return;
        }
        if( basePitch <= minPitchDep ) {  // low freq-> no dep tick[0]
            pitchDepIndex = -1;
            pitchDepDx  = 0;
            return;
        }
        const uint32_t  pitchDep  = ( basePitch - minPitchDep ) >> 7;
        pitchDepDx  = pitchDep & 0x0FFFF ;
        pitchDepIndex = ( pitchDep >> 16 ) & 0x0F;
    }

    GaloisShifter                   random4SustainModulator;
    OscillatorState                 state[ overtoneCountOscDef ];
    uint32_t                        basePitch;
    uint16_t                        delay;
    uint16_t                        toneShaperSelect;
    uint16_t                        oscillatorCountUsed;
    voice_state_t                   voiceState;
    uint16_t                        pitchDepDx;
    int8_t                          pitchDepIndex;
    static GNoise                   gNoiser;
    static GaloisShifter            gRandom;
    static const ToneShaperMatrix   toneShaperMatrix;
}; // end class Oscillator
// --------------------------------------------------------------------
} // end namespace yacynth


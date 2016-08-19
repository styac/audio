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
<<<<<<< HEAD
        return env.decayCoeffLF ? -(( ( env.decayCoeffLF * in ) >> 28 ) + 1 ) : 0 ;
=======
        return env.decayCoeff.get() ? -(( ( env.decayCoeff.get() * in ) >> 28 ) + 1 ) : 0 ;
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
    }
    inline int64_t mod( const int64_t in, const AmplitudeSustain& env  )
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
                        + ((( uint8_t( GaloisShifterSingle<seedThreadOscillator_random>::getInstance().getLow() ) * env.sustainModPeriod ) >> 10 ) & 0x0FC ); // 25 looks better
                    break;

                case env.MODTYPE_RAND2: // experimental ---
                    phaseDelta += ((( period ^ 0xFFFA ) >> 1 ) & 7 )
                        + ((( uint8_t( GaloisShifterSingle<seedThreadOscillator_random>::getInstance().getLow() ) * env.sustainModPeriod ) >> 9 ) & 0x08C );
                    break;

                case env.MODTYPE_RAND3: // experimental ---
                    phaseDelta += ((( period ^ 0xFFFA ) >> 1 ) & 7 )
                        + ((( uint8_t( GaloisShifterSingle<seedThreadOscillator_random>::getInstance().getLow() ) * env.sustainModPeriod ) >> 9 ) & 0x08C );
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
    uint8_t             rfu1;
    uint8_t             rfu2;
    uint8_t             rfu3;
};
// --------------------------------------------------------------------
struct OscillatorState {
    
    int64_t             amplitudoOsc;                   // ok - must be signed to handle underflows
    int64_t             envelopeTargetValueVelocity;    // ok - must be signed to handle underflows
    uint32_t            phase;
    int32_t             tickFrame;  // int16_t ???
    uint16_t            envelopMultiplierExpChecked; // int8_t ???
<<<<<<< HEAD
    int16_t             envelopePhase;  // int8_t ???                // run max..0  , -1 end
=======
    int8_t              envelopePhase; 
    int8_t              rfu1; 
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
    SustainModulator    sustainModulator;
};
// --------------------------------------------------------------------
class alignas(16) Oscillator {
public:
<<<<<<< HEAD
      // j 30 freq2ycent 155b2c3e
=======
      // 30 Hz -- freq2ycent 155b2c3e
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
    static constexpr int32_t minPitchDep = 0x155b2c3e;  // j 30 freq2ycent 155b2c3e

    enum OscType {
        OSC_SIN     = 0,
        OSC_SINSIN,         // waveSinTable[ uint16_t( waveSinTable[phase >>16] )]<<15;};

        // TODO
        OSC_PD00,   // phase distorsion0 - sin(sin()) waveSinTable[ uint16_t((waveSinTable[phase >>16]>>1) + (phase >>16))]
        OSC_PD01,   // phase distorsion1 - sin(sin()) waveSinTable[ uint16_t((waveSinTable[phase >>16]>>1) + (phase >>16))]
        OSC_PD02,   // phase distorsion2 - sin(sin()) waveSinTable[ uint16_t((waveSinTable[phase >>16]>>2) + (phase >>16))]
        OSC_PD03,   // phase distorsion3 - sin(sin()) waveSinTable[ uint16_t((waveSinTable[phase >>16]>>3) + (phase >>16))]

        OSC_12OV0,  // tone 1 + 2 : (waveSinTable[uint16_t(phase>>16)]+(waveSinTable[uint16_t(phase>>15)]));};
        OSC_12OV1,  // tone 1 + 2 : (waveSinTable[uint16_t(phase>>16)]+(waveSinTable[uint16_t(phase>>15)]>>1));};
        OSC_12OV2,  // tone 1 + 2 : (waveSinTable[uint16_t(phase>>16)]+(waveSinTable[uint16_t(phase>>15)]>>2));};

        // multiply by wave ???
        OSC_SIN_RING,
        OSC_SINSIN_RING,
//        OSC_WAVE1   = 1,    // user fillable tables
//        OSC_WAVE2   = 2,    // user fillable tables
//        OSC_WAVE3   = 3,    // user fillable tables
        OSC_NOISE_WHITE   = 0x80,
        OSC_NOISE_RED,
        OSC_NOISE_PURPLE,
        OSC_NOISE_BLUE,        
        OSC_NOISE_PEEK1,
        OSC_NOISE_PEEK2,
        OSC_NOISE_PEEK3,
        OSC_NOISE_PEEK4,
        OSC_SIN_MULT_RED_NOISE,
        OSC_NONE     = 255,
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
<<<<<<< HEAD
 
  j 20 freq2ycent 14c56c23
  j 21 freq2ycent 14d7712c
  j 22 freq2ycent 14e89f92
  j 23 freq2ycent 14f90a6a
  j 24 freq2ycent 1508c25c
  j 25 freq2ycent 1517d605
  j 26 freq2ycent 15265247
=======
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
  j 27 freq2ycent 15344291
  j 28 freq2ycent 1541b112
  j 29 freq2ycent 154ea6e7
  j 30 freq2ycent 155b2c3e
  j 31 freq2ycent 15674878
  j 32 freq2ycent 15730242
<<<<<<< HEAD
  j 33 freq2ycent 157e5fac
  j 34 freq2ycent 15896639
  j 35 freq2ycent 15941af3
  j 36 freq2ycent 159e8277
  j 37 freq2ycent 15a8a0ff
  j 38 freq2ycent 15b27a70
  j 39 freq2ycent 15bc1261

 
 
 
=======
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
 */        
        constexpr uint8_t   prExp = 11;
        constexpr uint32_t  maxPd = 0x0FFFF;
        const int32_t dp = basePitch - minPitchDep;
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
<<<<<<< HEAD
    
    inline uint32_t interpolatePitchDependency( const int64_t lowLim, const int64_t highLim )
    {
        return lowLim + ((( highLim - lowLim ) * pitchDepDx ) >> 16 );
    }
    
    GaloisShifter                   random4SustainModulator; // obsolate
=======
        
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
    OscillatorState                 state[ overtoneCountOscDef ];
    NoiseSample                     noiseWide;      // only 1 for a voice
    OscillatorNoise                 noiseNarrow;    // only 1 for a voice
    int32_t                         basePitch;
<<<<<<< HEAD
    uint16_t                        velocity;  // NEW TODO -- velocity: set by note on
=======
    uint16_t                        velocity; 
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
    uint16_t                        delay;
    uint16_t                        toneShaperSelect;
    uint16_t                        oscillatorCountUsed;
    uint16_t                        pitchDepDx;
<<<<<<< HEAD
    static GaloisShifter            gRandom;  // obsolate
=======
    voice_state_t                   voiceState;
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
    static NoiseFrame<FrameInt<oscillatorOutSampleCountExp>>   
                                    whiteNoiseFrame;
    static const ToneShaperMatrix   toneShaperMatrix;
}; // end class Oscillator
// --------------------------------------------------------------------
} // end namespace yacynth

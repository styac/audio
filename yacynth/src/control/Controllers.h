#pragma once

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
 * File:   Controllers.h
 * Author: Istvan Simon
 *
 * Created on February 16, 2016, 10:27 AM
 */
#include    "yacynth_globals.h"
#include    "v4.h"
#include    "../utils/Fastsincos.h"
#include    "../utils/Fastexp.h"


#include    <cstdint>
#include    <string>
#include    <array>
#include    <atomic>
#include    <cstring>
//
//
// http://stackoverflow.com/questions/109710/likely-unlikely-macros-in-the-linux-kernel-how-do-they-work-whats-their
// likely unlikely
//

using namespace tables;

namespace yacynth {

constexpr   std::size_t controllerCountExp = 9; // 512 -- midi -> low 256

// internal controllers

// TODO
// class InnerSetController     -- bitmap -- 1/0
// class InnerBiController      -- logical true - false
// class InnerMultiController   -- multivalue --> bank select

// range 0 .. +N
class InnerController : public V4size<controllerCountExp> {

public:
    static constexpr std::size_t maxIndex           = arraySize;
    static constexpr std::size_t filteredRangeV4    = 8;
    static constexpr std::size_t filteredRange      = filteredRangeV4*4;

    // LFO
    static constexpr std::size_t lfoMasterCountV4   = 2;
    static constexpr std::size_t lfoMasterCount     = lfoMasterCountV4*4;
    static constexpr std::size_t lfoMasterCountMask = lfoMasterCount-1;
    // 1 master + 8 slave = 9 phase
    static constexpr std::size_t lfoSlaveSetCount   = 8;
    static constexpr std::size_t lfoSlaveSetCountMask    = lfoSlaveSetCount-1;


    static constexpr std::size_t norm               = 24;
    static constexpr std::size_t kshift             = 3;
    static constexpr std::size_t kround             = 1<<(kshift-1);
    static constexpr std::size_t midiStep           = 128;
    static constexpr std::size_t midiStepMask       = midiStep-1;

    enum  {
        CC_NULL                         = 0, // never set -- must be zero
        CC_MAINVOLUME,
        CC_PITCHBEND,
        CC_CHANNEL_AFTERTOUCH,
        CC_MODULATOR_PHASEDIFF0,
        CC_MODULATOR_FREQ0,
        CC_MODULATOR_FREQ1,
        CC_FILTER_FREQ0,
        CC_FILTER_Q0,
        // ------------- filtered range begin
        CC_BEGIN_FILTER                 = 128,
        CC_END_FILTER                   = CC_BEGIN_FILTER + filteredRange,
        // ------------- filtered range end
        CC_SINK                         = 255,  // never get -- all unused controllers go here
        // ------------- internal range begin
        CC_AMPLITUDE                    = 256,       // internal -- amplitude summ
        // ------------- internal range end
        // ------------- LFO range begin
        CC_LFO_MASTER_PHASE_BEGIN       = 260,
        CC_LFO_SLAVE_PHASE_BEGIN        = CC_LFO_MASTER_PHASE_BEGIN + lfoMasterCount,
        CC_LFO_MASTER_DELTA_PHASE_BEGIN = CC_LFO_SLAVE_PHASE_BEGIN + lfoMasterCount * lfoSlaveSetCount,
        CC_LFO_SLAVE_DELTA_PHASE_BEGIN  = CC_LFO_MASTER_DELTA_PHASE_BEGIN + lfoMasterCount,
        CC_LFO_DELTA_PHASE_END          = CC_LFO_SLAVE_DELTA_PHASE_BEGIN + lfoMasterCount * lfoSlaveSetCount,
        // ------------- LFO range end
        // ------------- filtered range begin
        CC_BEGIN_FILTERED               = 514,
        CC_END_FILTERED                 = CC_BEGIN_FILTERED + filteredRange,
        // ------------- filtered range end
        CC_END
    };

    static_assert( CC_END < arraySize, "array is too small" );

    inline uint16_t getCountMasterLfo(void)
    {
        return lfoMasterCount;
    }
    inline uint16_t getCountSlaveLfo(void)
    {
        return lfoSlaveSetCount;
    }
    inline uint16_t getIndexMasterLfoPhase( uint16_t masterLfoNumber )
    {
        return CC_LFO_MASTER_PHASE_BEGIN + ( masterLfoNumber & lfoMasterCountMask );
    }
    inline uint16_t getIndexMasterLfoDeltaPhase( uint16_t masterLfoNumber )
    {
        return CC_LFO_MASTER_DELTA_PHASE_BEGIN + ( masterLfoNumber & lfoMasterCountMask );
    }
    inline uint16_t getIndexSlaveMasterLfoPhase( uint16_t masterLfoNumber, uint16_t slaveLfoNumber )
    {
        return CC_LFO_SLAVE_PHASE_BEGIN + ( masterLfoNumber & lfoMasterCountMask ) + lfoSlaveSetCount * (slaveLfoNumber & lfoSlaveSetCountMask );
    }
    inline uint16_t getIndexSlaveMasterLfoDeltaPhase( uint16_t masterLfoNumber, uint16_t slaveLfoNumber )
    {
        return CC_LFO_SLAVE_DELTA_PHASE_BEGIN + ( masterLfoNumber & lfoMasterCountMask ) + lfoSlaveSetCount * (slaveLfoNumber & lfoSlaveSetCountMask );
    }

    inline void clear(void)
    {
        value.clear();
        for( auto i =0; i<arraySize ; ++i )
            shiftleft[i] = norm-7;
    }
    /*
k 1 f 5295.41
k 2 f 2197.79
k 3 f 1020.13 -- 1323
k 4 f 493.053
k 5 f 242.549 -- 331
k 6 f 120.312
k 7 f 59.9192 -- 82
k 8 f 29.9009
k 9 f 14.9358  -- 20.6
     */
    // one pole filter>  kshift+6 --> 9
    inline void inc(void)
    {
        for( int i=CC_BEGIN_FILTER; i<filteredRangeV4; ++i ) {
            value.v4[i+filteredRangeV4] += (value.v4[i] - value.v4[i+filteredRangeV4] + kround ) >> kshift;
        }
    }

    // generic set
    inline void set( uint16_t ind, uint32_t v )
    {
        value.v[ ind & V4size::arraySizeMask ] = v;
    }

    inline void setShift( uint16_t ind, uint8_t v )
    {
        shiftleft[ ind & V4size::arraySizeMask ] = v;
    }

    // mainly for setAmplitudeSumm
    // index is full: 2 x range
    inline void setLog( uint16_t ind, uint64_t v )
    {
        // 23 bit mantissa + 6 bit exponent (1<<63)
        constexpr std::size_t maxLog = 23+6;
        if( v < 4 ) {
            value.v[ ( ind & arraySizeMask ) ] = 0;
        }
        const float x = static_cast<float>(v);
        value.v[ ( ind & arraySizeMask ) ] = ( uint32_t(x) - (127<<23) ) >> (maxLog-norm);
    }

    inline void setAmplitudeSumm( uint64_t v )
    {
        setLog(CC_AMPLITUDE,v);
    }

    inline void setMidi( uint8_t ind, uint8_t v )
    {
        value.v[ ind & V4size::arraySizeMask ] = v << shiftleft[ind & V4size::arraySizeMask];
        std::cout << "InnerController ind "  << uint16_t(ind)
            << " val " << uint16_t(v)
            << " stored " << value.v[ ind & V4size::arraySizeMask ]
            << std::endl;
    }

    // never shifted -- pitchbend
    inline void setMidi( uint8_t ind, uint8_t vH, uint8_t vL )
    {
        value.v[ ind & V4size::arraySizeMask ] = ( vH << 7 ) + vL;
    }

    inline uint32_t get( uint16_t ind ) const
    {
        return value.v[ ind & arraySizeMask ];
    }

    inline static InnerController& getInstance(void)
    {
        static InnerController instance;
        return instance;
    }

    static float getExpValue( uint8_t v )
    {
        return expFuncTable[ v & midiStepMask ];
    }
    static uint32_t getPhaseValue( uint8_t v )
    {
        return phaseFuncTable[ v & midiStepMask ];
    }

    // LFO range -- updated 1x / frame
    //  - sampling rate = sampling rate audio / sample count in frame -- 48000/64
    //  - lfoMasterRange = n*4^k -- 4k because of v4 req.
    //  - each master has s*4^k slaves with
    //
    // MMM-PHASE                -- master current phase [mci]
    // SS0-PHASE MMM+delta0     -- slave current phase [sci]
    // SS1-PHASE MMM+delta1
    // SS2-PHASE MMM+delta2
    //
    // SS3-PHASE MMM+delta3
    // SS4-PHASE MMM+delta4
    // SS5-PHASE MMM+delta5
    // SS6-PHASE MMM+delta6
    //
    // deltaPhase MMM -- freq   -- master delta phase[mi] -- frequency
    // delta0                   -- slave delta[si] to master[mi]
    // delta1
    // delta2
    //
    // delta3
    // delta4
    // delta5
    // delta6

    // V4 processing !!
    inline void incrementFrameLFOscillatorPhases(void)
    {
        int16_t currentPhaseInd = CC_LFO_MASTER_PHASE_BEGIN/4;
        int16_t deltaPhaseInd = CC_LFO_MASTER_DELTA_PHASE_BEGIN/4;
        // update master current phase [mi]
        for( int16_t mci=0; mci<lfoMasterCountV4; ++mci, ++currentPhaseInd, ++deltaPhaseInd ) {
            value.v4[currentPhaseInd] += value.v4[deltaPhaseInd] ;
        }
        for( int16_t sci=0; sci <lfoSlaveSetCount/4; ++sci ) {
            int16_t currentPhaseMasterInd = CC_LFO_MASTER_PHASE_BEGIN/4;
            for( int16_t i=0; i<lfoMasterCountV4; ++i, ++currentPhaseInd, ++deltaPhaseInd, ++currentPhaseMasterInd ) {
                value.v4[currentPhaseInd] = value.v4[deltaPhaseInd] + value.v4[currentPhaseMasterInd];
            }
        }
    }

private:
    InnerController();
    V4array<int32_t,arraySizeExp>   value; 
    uint8_t                         shiftleft[arraySize];

    // volume control characteristic
    static float expFuncTable[midiStep];

    // lfo phase control 0 .. PI
    static uint32_t phaseFuncTable[midiStep];
};

// get sin, saw, triangle -> 16 bit
// set by ycent, (1/64 sampling)
// set phase

struct ControlledValue {
    int32_t     value;  // updated value from the InnerController
    int32_t     y0;     // y(0) for linear mapping
    int16_t     slope;  // multiplier for linear mapping
    uint16_t    index;  // index in InnerController

    // value == signed 16 bit
    inline bool updateLfoSaw(void)
    {
        value = (int32_t(InnerController::getInstance().get( index ))>>16);
        return true;
    }
    // value == signed 16 bit
    inline bool updateLfoSin(void)
    {
        value = tables::waveSinTable[ uint16_t(InnerController::getInstance().get( index )>>16) ];
        return true;
    }
    // value == signed 16 bit
    inline bool updateLfoTriangle(void)
    {
        const int32_t tmp = (int32_t(InnerController::getInstance().get( index ))>>15);
        value = ((tmp>>16) ^ tmp ) - 0x7FFF;
        return true;
    }    
    inline bool updateDiff(void)
    {
        const auto tmp = value;
        value = InnerController::getInstance().get( index );
        return tmp != value;
    }    
    inline void setShift(uint8_t shv) const
    {
        if( shv > 24 )
            shv = 24;
        InnerController::getInstance().setShift(index,shv);
    }

    // to InnerController !
    inline void set( uint32_t v  )
    {
        value = v;
        InnerController::getInstance().set( index, v );
    }

    inline int32_t getValue(void) const
    {
        return value;
    }
    inline float getExpValue(void) const
    {

        return InnerController::getInstance().getExpValue(value);
    }
    inline uint32_t getPhaseValue(void) const
    {
        return InnerController::getInstance().getPhaseValue(value);
    }

    // ycent -- max 8 octave
    // shift == 0
    inline void setYcent8Parameter( int32_t y0p, int16_t slopep )
    {
                            // 0.01:9ce2e7f
        constexpr int32_t minY =  0x8000000;
                          // 19990:1ebacfd9
        constexpr int32_t maxY = 0x1EF00000;

        InnerController::getInstance().setShift(index,5);
        slope = slopep;
        if( y0p <= minY ) {
            y0 = minY;
            return;
        }
        if( y0p >= maxY ) {
            y0 = maxY;
            return;
        }
        y0 = y0p;
    }
    // 7 + 5 == 12 --
    // 12 * 15 == 27 --
    // 27-24 == 3 --
    // 2^3 == 8
    inline int32_t getYcent8Value(void) const
    {
        return y0 + slope * int16_t(value);
    }
};


// -------------------------------------------------------------------------
// maps MIDI(channel,controller) -> index
//
// -- 120..127 should be fixed -- MidiBiController

// CC 98,99,100,101, 6,38,96,97
// -- RPN,NRPN, others
// -- switches ???? MidiBiController
// -- bank select - MidiMultiController


class MidiRangeController {
public:
    static constexpr std::size_t channelCount           = 16;
    static constexpr std::size_t channelCountMask       = channelCount-1;
    static constexpr std::size_t controllerCount        = 128;  // + 2 extra -- 96 + 2
    static constexpr std::size_t controllerCountMask    = controllerCount-1;
    static constexpr std::size_t controllerAftertouch   = controllerCount;
    static constexpr std::size_t controllerPitchbend    = controllerCount+1;
    static constexpr std::size_t controllerCountAll     = 130;  // + 2 extra -- 96 + 2

    MidiRangeController();

    void clear(void)
    {
        for(auto i=0u; i<channelCount; ++i )
            for(auto j=0u; j<controllerCount; ++j )
                index[i][j] = InnerController::CC_SINK; // all unused goes there
    }
    uint8_t get( uint8_t channel, uint8_t controller ) const
    {
        return index[ channel & channelCountMask ] [ controller & controllerCountMask ];
    }
    uint8_t getAftertouch( uint8_t channel ) const
    {
        return index[ channel & channelCountMask ] [ controllerAftertouch ];
    }
    uint8_t getPitchbend( uint8_t channel ) const
    {
        return index[ channel & channelCountMask ] [ controllerPitchbend ];
    }

    // set the mapping between the midi controllers and the innercontroller
    void set( uint8_t channel, uint8_t controller, uint8_t ind = InnerController::CC_SINK )
    {
        if( ind >= InnerController::maxIndex || ind == 0 ) {
            ind = InnerController::CC_SINK;
        }
        if( controllerCountAll <= controller ) {
            return;
        }
        index[ channel & channelCountMask ] [ controller ] = ind;
    }

private:
    uint8_t    index[channelCount][controllerCountAll];
};

// --------------------------------------------------------------------
} // end namespace yacynth

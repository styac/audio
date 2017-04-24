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
#include    "Tags.h"

#include    "protocol.h"

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


#define USE_OBSOLATE

using namespace tables;

namespace yacynth {

// page 0 - range
// page 1 - lfo
// page 2 - filtered ??
// page 3 - switch
// page 4 - extended sw ( GUI+OSC )


constexpr   std::size_t controllerPageExp   = 5; // 4 * 256
constexpr   std::size_t controllerCountExp  = controllerPageExp + 8; // 512 -- midi -> low 256


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
    static constexpr std::size_t midiStep           = 128;          // table steps exp-ampl, phase
    static constexpr std::size_t midiStepMask       = midiStep-1;

    enum  {
        CC_NULL                         = 0, // never set -- must be zero
        CC_MAINVOLUME,
        CC_PITCHBEND,
        CC_CHANNEL_AFTERTOUCH,
        CC_MODULATOR_PHASEDIFF0,
        CC_MODULATOR_FREQ0,
        CC_MODULATOR_FREQ1,
        CC_MODULATOR_INVOL,
        CC_MODULATOR_MIXVOL,
        CC_FILTER_FREQ0,
        CC_FILTER_Q0,
        // ------------- mirror to filtered range begin
        // this part 128..+32 are filtered slow change
        CC_BEGIN_FILTER                 = 128,
        CC_END_FILTER                   = CC_BEGIN_FILTER + filteredRange,
        // ------------- mirror to filtered range end
        // instead of SINK - CM DISABLE
        CC_SINK                         = 255,  // never get -- all unused controllers go here - obsolete
        // ------------- internal range begin
        CC_AMPLITUDE                    = 1<<8,       // internal -- amplitude summ
        // ------------- internal range end
        // ------------- LFO range begin
        CC_LFO_MASTER_PHASE_BEGIN       = 260,
        CC_LFO_SLAVE_PHASE_BEGIN        = CC_LFO_MASTER_PHASE_BEGIN + lfoMasterCount,
        CC_LFO_MASTER_DELTA_PHASE_BEGIN = CC_LFO_SLAVE_PHASE_BEGIN + lfoMasterCount * lfoSlaveSetCount,
        CC_LFO_SLAVE_DELTA_PHASE_BEGIN  = CC_LFO_MASTER_DELTA_PHASE_BEGIN + lfoMasterCount,
        CC_LFO_DELTA_PHASE_END          = CC_LFO_SLAVE_DELTA_PHASE_BEGIN + lfoMasterCount * lfoSlaveSetCount,
        // ------------- LFO range end
        // ------------- filtered range begin
        CC_BEGIN_FILTERED               = 516,
        CC_END_FILTERED                 = CC_BEGIN_FILTERED + filteredRange,
        // ------------- filtered range end
        // switch controllers
        CC_BEGIN_SWITCH                 = 3*(1<<8),
        // software controllers 
        CC_BEGIN_SW                     = 4*(1<<8),
        CC_END                          = 5*(1<<8)
    };

    static_assert( CC_END < arraySize, "array is too small" );

    bool parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

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
    inline void incFilter(void)
    {
        for( int i=CC_BEGIN_FILTER; i<filteredRangeV4; ++i ) {
            value.v4[i+filteredRangeV4] += (value.v4[i] - value.v4[i+filteredRangeV4] + kround ) >> kshift;
        }
    }

    // generic set
    inline void set( uint16_t ind, int32_t v )
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

    // eliminate shift
    inline void setMidi( uint8_t ind, uint8_t v )
    {
        value.v[ ind & V4size::arraySizeMask ] = v; // << shiftleft[ind & V4size::arraySizeMask];
        std::cout << "InnerController ind "  << uint16_t(ind)
            << " val " << uint16_t(v)
            << " stored " << value.v[ ind & V4size::arraySizeMask ]
            << std::endl;
    }

    // switch page
    // page 1
    inline uint32_t getMidiSwitch( uint8_t ind ) const
    {
        return value.v[ (ind & V4size::arraySizeMask ) + CC_BEGIN_SWITCH ];
    }

    inline void setMidiSwitch( uint8_t ind, uint8_t v ) // limiter?
    {
        value.v[ (ind & V4size::arraySizeMask ) + CC_BEGIN_SWITCH ] = v;
        std::cout << "setMidiSwitch ind "  << uint16_t(ind)
            << " stored " << value.v[ (ind & V4size::arraySizeMask ) + CC_BEGIN_SWITCH ]
            << std::endl;
    }
    inline void incMidiSwitch( uint8_t ind  ) // limiter?
    {
        ++value.v[ (ind & V4size::arraySizeMask ) + CC_BEGIN_SWITCH ];
        std::cout << "incMidiSwitch ind "  << uint16_t(ind)
            << " stored " << value.v[ (ind & V4size::arraySizeMask ) + CC_BEGIN_SWITCH ]
            << std::endl;
    }
    inline void decMidiSwitch( uint8_t ind  ) // limiter?
    {
        --value.v[ (ind & V4size::arraySizeMask ) + CC_BEGIN_SWITCH ];
        std::cout << "decMidiSwitch ind "  << uint16_t(ind)
            << " stored " << value.v[ (ind & V4size::arraySizeMask ) + CC_BEGIN_SWITCH ]
            << std::endl;
    }
    inline void clrMidiSwitch( uint8_t ind  )
    {
        value.v[ (ind & V4size::arraySizeMask ) + CC_BEGIN_SWITCH ] = 0;
        std::cout << "clrMidiSwitch ind "  << uint16_t(ind)
            << " stored " << value.v[ (ind & V4size::arraySizeMask ) + CC_BEGIN_SWITCH ]
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
    uint8_t                         shiftleft[arraySize]; // obsolate -- must be eliminated

    // volume control characteristic
    static float expFuncTable[midiStep];

    // lfo phase control 0 .. PI
    static uint32_t phaseFuncTable[midiStep];
};


// -------------------------------------------------------------------------
//
// -- RPN,NRPN, ??
// -- bank select - MidiMultiController


class MidiController {
public:
    static constexpr std::size_t channelCount               = 16;
    static constexpr std::size_t channelCountMask           = channelCount-1;
    static constexpr std::size_t controllerCount            = 128;  // + 2 extra -- 96 + 2
    static constexpr std::size_t controllerCountMask        = controllerCount-1;
    static constexpr std::size_t controllerPolyAftertouch   = controllerCount;
    static constexpr std::size_t controllerChanAftertouch   = controllerCount+1;
    static constexpr std::size_t controllerPRogramChange    = controllerCount+2;
    static constexpr std::size_t controllerPitchbend        = controllerCount+3;
    static constexpr std::size_t controllerCountAll         = controllerCount+4; 

    struct ControlData {
        uint8_t    index;
        uint8_t    mode;
    };

    MidiController();

    enum CMode : uint8_t {
        CM_DISABLE          = 0x0FF,            // not used
        CM_RANGE            = CM_DISABLE-1,     // normal range controller > 0..127
        CM_RANGE08          = CM_DISABLE-2,     // << 1
        CM_RANGE09          = CM_DISABLE-3,     // << 2
        CM_RANGE10          = CM_DISABLE-4,     // << 3
        CM_RANGE11          = CM_DISABLE-5,     // << 4
        CM_RANGE12          = CM_DISABLE-6,     // << 5
        CM_RANGE13          = CM_DISABLE-7,     // << 6
        CM_RANGE14          = CM_DISABLE-8,     // << 7
        CM_RANGE15          = CM_DISABLE-9,     // << 8
        CM_RANGE16          = CM_DISABLE-10,    // << 9
        CM_RANGE17          = CM_DISABLE-11,    // << 10
        CM_RANGE18          = CM_DISABLE-12,    // << 11
        CM_RANGE19          = CM_DISABLE-13,    // << 12
        CM_RANGE20          = CM_DISABLE-14,    // << 13
        CM_RANGE21          = CM_DISABLE-15,    // << 14
        CM_RANGE22          = CM_DISABLE-16,    // << 15
        CM_RANGE23          = CM_DISABLE-17,    // << 16
        CM_RANGE24          = CM_DISABLE-18,    // << 17
        CM_RANGE25          = CM_DISABLE-19,    // << 18
        CM_RANGE26          = CM_DISABLE-21,    // << 19
        CM_RANGE27          = CM_DISABLE-22,    // << 21
        CM_RANGE28          = CM_DISABLE-23,    // << 22
        CM_RANGE29          = CM_DISABLE-24,    // << 23
        CM_RANGE30          = CM_DISABLE-25,    // << 24
        CM_RANGE31          = CM_DISABLE-26,    // << 25
        CM_RANGE32          = CM_DISABLE-27,    // << 26
        CM_INC              = CM_DISABLE-28,    // increment by 1
        CM_DEC              = CM_DISABLE-29,    // decrement by 1
        CM_SET              = CM_DISABLE-30,    // set -- 0,FF
        CM_RPN_PH           = CM_DISABLE-31,    // RPN param high
        CM_RPN_PL           = CM_DISABLE-32,    // RPN param low
        CM_RPN_VH           = CM_DISABLE-33,    // RPN data high
        CM_RPN_VL           = CM_DISABLE-34,    // RPN data low
        CM_NRPN_PH          = CM_DISABLE-35,    // NRPN param high
        CM_NRPN_PL          = CM_DISABLE-36,    // NRPN param low
        CM_NRPN_VH          = CM_DISABLE-37,    // NRPN data high
        CM_NRPN_VL          = CM_DISABLE-38,    // NRPN data low
        CM_PROGCH           = CM_DISABLE-39,    // program change ?
        // direct commands
        CM_MUTE             = CM_DISABLE-40,
        CM_UNMUTE           = CM_DISABLE-41,
        CM_RESET            = CM_DISABLE-42,
        // more MIDI ????
                
        /* 0..128 direct value */
    };

    size_t getChannelCount(void) const
    {
        return channelCount;
    }

    void clearChannel( uint8_t channel )
    {
        if( channel >= channelCount ) {
            return;
        }
        for(auto j=0u; j<controllerCount; ++j ) {
                cdt[channel][j].index = InnerController::CC_SINK; // all unused goes there -- obsolete
//                cdt[i][j].mode  = CM_DISABLE; // unused  -- new
                cdt[channel][j].mode  = CM_RANGE; // TEST
        }
    }


    bool parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    void clear(void)
    {
        for(auto i=0u; i<channelCount; ++i )
            for(auto j=0u; j<controllerCount; ++j ) {
                cdt[i][j].index = InnerController::CC_SINK; // all unused goes there -- obsolete
//                cdt[i][j].mode  = CM_DISABLE; // unused  -- new
                cdt[i][j].mode  = CM_RANGE; // TEST
            }
    }

    uint8_t get( uint8_t channel, uint8_t controller ) const
    {
        return cdt[ channel & channelCountMask ] [ controller & controllerCountMask ].index;
    }

    ControlData getControlData( uint8_t channel, uint8_t controller ) const
    {
        return cdt[ channel & channelCountMask ] [ controller & controllerCountMask ];
    }

    uint8_t getAftertouch( uint8_t channel ) const
    {
        return cdt[ channel & channelCountMask ] [ controllerPolyAftertouch ].index;
    }
    uint8_t getPitchbend( uint8_t channel ) const
    {
        return cdt[ channel & channelCountMask ] [ controllerPitchbend ].index;
    }

    // set the mapping between the midi controllers and the innercontroller
    void set( uint8_t channel, uint8_t controller, uint8_t ind = InnerController::CC_SINK, CMode mode = CMode::CM_RANGE )
    {
        if( ind >= InnerController::maxIndex || ind == 0 ) {
            ind = InnerController::CC_SINK;
        }
        if( controllerCountAll <= controller ) {
            return;
        }
        cdt[ channel & channelCountMask ] [ controller ].index = ind;
        cdt[ channel & channelCountMask ] [ controller ].mode  = mode;
        std::cout << std::hex
            << "  +++++++++ channel " << uint16_t(channel) 
            << " controller " << uint16_t(controller) 
            << " index " << uint16_t(cdt[ channel & channelCountMask ] [ controller ].index) 
            << " mode " << uint16_t(cdt[ channel & channelCountMask ] [ controller ].mode)  
            << std::endl;
    }

private:
    ControlData    cdt[channelCount][controllerCountAll]; // index to InnerController

    // radio button:
    //  controller number copies the v.value to the high part of the InnerController
    //  this will be used to control e.g. the mode of th effects or anything multivalue

    //Data    v[channelCount][controllerCountAll]; // index + local value for RadioButton controller set
};

// --------------------------------------------------------------------
// new
//

// define an index 
struct ControllerIndex {
    uint16_t    index;
    bool setIndex( uint16_t ind )
    {
        if( ind < InnerController::CC_END ) {
            index = ind;
            return true;
        }
        return false;
    }
    inline void setInnerValue( int32_t v = 0 )
    {
        InnerController::getInstance().set( index, v );
    }
    inline float getExpValue(void) const
    {
        return InnerController::getInstance().getExpValue( InnerController::getInstance().get( index ));
    }
    inline uint32_t getPhaseValue(void) const
    {
        return InnerController::getInstance().getPhaseValue(InnerController::getInstance().get( index ));
    }
};

// store the value and check the diff
struct ControllerCache {
    inline bool update( const ControllerIndex ind )
    {
        const auto tmp = value;
        value = InnerController::getInstance().get( ind.index );
        return tmp != value;
    }

    inline void updateLfoSaw( const ControllerIndex ind )
    {
        value = (int32_t(InnerController::getInstance().get( ind.index ))>>16);
    }

    inline void updateLfoSin( const ControllerIndex ind )
    {
        value = tables::waveSinTable[ uint16_t(InnerController::getInstance().get( ind.index )>>16) ];
    }

    inline void updateLfoTriangle( const ControllerIndex ind )
    {
        const int32_t tmp = (int32_t(InnerController::getInstance().get( ind.index ))>>15);
        value = ((tmp>>16) ^ tmp ) - 0x7FFF;
    }

    inline float getExpValue(void) const
    {
        return InnerController::getInstance().getExpValue( value );
    }

    inline uint32_t getPhaseValue(void) const
    {
        return InnerController::getInstance().getPhaseValue( value );
    }
    inline auto getValue(void) const
    {
        return value & 0x7FFFF;
    }
    inline void trigger(void) 
    {
        value &= 0x8000; // trigger a change 
    }
    // cache is not used for oscillator -- normally midi is max 14 bit
//    int32_t     value;
    int16_t     value;
};

// always adds only a fraction of the change to the value
// compare this with filtered controller
template< int32_t lim > 
struct ControllerCacheDelta : public ControllerCache {
    static_assert( lim > 2 && lim < 30 ,"value out of limits");
    inline bool updateDelta( const ControllerIndex ind )
    {
        const auto delta = InnerController::getInstance().get( ind.index ) - value;
        value += saturate<int32_t,lim>(delta);
        return delta==0;
    }
};

/*
 * const auto cval = param.pmode_01.s1.update( controlledval );
 * const auto a0   = param.pmode_01.s1.get( controlledval, 0 );
 * const auto a1   = param.pmode_01.s1.get( controlledval, 1 );
 */

template< uint8_t acount >
struct ControllerMapLinear {
    static constexpr uint8_t offsetCount = acount;
    static constexpr int32_t maxMult  = 1<<24;   // min input 7 bit
    static constexpr int32_t maxOffs  = 1<<30;   
    inline int32_t getScaled( int32_t val ) const
    {
        return mult * val;
    }
    inline int32_t getOffseted( int32_t val ) const
    {
        static_assert( offsetCount>0,"not usable" );
        return y0[0] + val;
    }
    // MUST ind < offsetCount
    inline int32_t getOffseted( int32_t val, uint16_t ind ) const
    {
        static_assert( offsetCount>0,"not usable" );
        return y0[ind] + val;
    }
    bool check() 
    {
        const int32_t abm = std::abs(mult);
        if( abm == 0 || abm > maxMult ) {
            std::cout << "  ControllerMapLinear err mult " << mult << std::endl;            
            return false;
        } 
        for( auto &y : y0 ) {
            const int32_t aby = std::abs(y);
            if( aby > maxOffs ) {
                std::cout << "  ControllerMapLinear err offs " << aby << std::endl;            
                return false;                
            }
        }
        return true;
    }
    int32_t     mult;
    int32_t     y0[offsetCount];
};


// OBSOLATE !!

#ifdef USE_OBSOLATE

template< uint8_t fcount >
struct ControlledValue {
    // new concept -- value external
    int32_t     value;          // updated value from the InnerController
    int32_t     y0[fcount];     // y(0) for linear mapping
    int16_t     slope[fcount];  // multiplier for linear mapping
    uint16_t    index;          // index in InnerController


    // value == signed 16 bit
    inline bool updateLfoSaw(void)
    {
        value = (int32_t(InnerController::getInstance().get( index ))>>16);
        return true;
    }
    inline int16_t updateLfoSawNew(void)
    {
        return (int32_t(InnerController::getInstance().get( index ))>>16);
    }
    // value == signed 16 bit
    inline bool updateLfoSin(void)
    {
        value = tables::waveSinTable[ uint16_t(InnerController::getInstance().get( index )>>16) ];
        return true;
    }

    inline int16_t updateLfoSinNew(void)
    {
        return tables::waveSinTable[ uint16_t(InnerController::getInstance().get( index )>>16) ];

    }
    // value == signed 16 bit
    inline bool updateLfoTriangle(void)
    {
        const int32_t tmp = (int32_t(InnerController::getInstance().get( index ))>>15);
        value = ((tmp>>16) ^ tmp ) - 0x7FFF;
        return true;
    }

    inline int16_t updateLfoTriangleNew(void)
    {
        const int32_t tmp = (int32_t(InnerController::getInstance().get( index ))>>15);
        return ((tmp>>16) ^ tmp ) - 0x7FFF;
    }

    inline bool updateDiff(void)
    {
        const auto tmp = value;
        value = InnerController::getInstance().get( index );
        return tmp != value;
    }

    // new concept
    inline bool update( int16_t& val )
    {
        const auto tmp = val;
        val = InnerController::getInstance().get( index );
        return tmp != val;
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
        static_assert(fcount>0,"not usable");
                            // 0.01:9ce2e7f
        constexpr int32_t minY =  0x8000000;
                          // 19990:1ebacfd9
        constexpr int32_t maxY = 0x1EF00000;

        InnerController::getInstance().setShift(index,5);
        slope[0] = slopep;
        if( y0p <= minY ) {
            y0[0] = minY;
            return;
        }
        if( y0p >= maxY ) {
            y0[0] = maxY;
            return;
        }
        y0[0] = y0p;
    }

    inline void setYcent8Parameter( int32_t y0p, int16_t slopep, uint8_t index )
    {
                            // 0.01:9ce2e7f
        static_assert(fcount>0,"not usable");
        constexpr int32_t minY =  0x8000000;
                          // 19990:1ebacfd9
        constexpr int32_t maxY = 0x1EF00000;
        if(index >= fcount)
            return;

        InnerController::getInstance().setShift(index,5);
        slope[index] = slopep;
        if( y0p <= minY ) {
            y0[index] = minY;
            return;
        }
        if( y0p >= maxY ) {
            y0[index] = maxY;
            return;
        }
        y0[index] = y0p;
    }
    // 7 + 5 == 12 --
    // 12 * 15 == 27 --
    // 27-24 == 3 --
    // 2^3 == 8
    inline int32_t getYcent8Value(void) const
    {
        static_assert(fcount>0,"not usable");
        return y0[0] + slope[0] * int16_t(value);
    }

    inline int32_t getYcent8Value(uint8_t index) const
    {
        static_assert(fcount>0,"not usable");
        if(index >= fcount)
            return 0;
        return y0[index] + slope[index] * int16_t(value);
    }
};

#endif

// --------------------------------------------------------------------
} // end namespace yacynth

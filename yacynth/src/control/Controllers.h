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
    static constexpr std::size_t lfoMasterRangeV4   = 2;
    static constexpr std::size_t lfoMasterRange     = lfoMasterRangeV4*4;
    // 1 master + 7 slave = 8 phase
    static constexpr std::size_t lfoSlaveSet        = 7;
    static constexpr std::size_t norm               = 24;
    static constexpr std::size_t kshift             = 3;
    static constexpr std::size_t kround             = 1<<(kshift-1);
    static constexpr std::size_t midiStep           = 128;
    static constexpr std::size_t midiStepMask       = midiStep-1;

    enum  {
        CC_NULL,        // never set -- must be yero
        CC_AMPLITUDE,   // internal -- amplitude summ
        CC_MAINVOLUME,
        CC_PITCHBEND,
        CC_CHANNEL_AFTERTOUCH,       
        CC_MODULATOR_PHASEDIFF0,       
        CC_MODULATOR_FREQ0,       
        CC_MODULATOR_FREQ1,       

        CC_FILTER_FREQ0,       
        CC_FILTER_Q0,       
        
        CC_BEGIN_FILTER     = 128,
        CC_SINK             = 255,  // never get -- all unused controllers go here
        CC_BEGIN_FILTERED   = 256,
        CC_END_FILTERED     = CC_BEGIN_FILTERED + filteredRange,
          
        // -------------
        
        CC_LFO_RUNNING_PHASE_BEGIN,
        CC_LFO_DELTA_PHASE_BEGIN    = CC_LFO_RUNNING_PHASE_BEGIN + lfoMasterRange * (1+lfoSlaveSet),
        CC_LFO_DELTA_PHASE_END      = 2 * CC_LFO_DELTA_PHASE_BEGIN - CC_LFO_RUNNING_PHASE_BEGIN,
        CC_END
    };
    
    static_assert( CC_END < arraySize, "array is too small" );
    
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
    
    // 
    // MMM-PHASE
    // SS0-PHASE MMM+delta0
    // SS1-PHASE MMM+delta1
    // SS2-PHASE MMM+delta2
    //     
    // SS3-PHASE MMM+delta3
    // SS4-PHASE MMM+delta4
    // SS5-PHASE MMM+delta5
    // SS6-PHASE MMM+delta6
    // 
    // deltaPhase MMM -- freq
    // delta0
    // delta1
    // delta2
    // 
    // delta3
    // delta4
    // delta5
    // delta6
    
    inline void incLfo(void)
    {
        int iMLFO;
        int iRPB = CC_LFO_RUNNING_PHASE_BEGIN;
        int iDPB = CC_LFO_DELTA_PHASE_BEGIN;
        for( int i=0; i<lfoMasterRangeV4; ++i, ++iRPB, ++iDPB ) {
            value.v4[iRPB] += value.v4[iDPB] ;
        }
        for( int slave=0; slave <lfoSlaveSet; ++slave ) {
            iMLFO = CC_LFO_RUNNING_PHASE_BEGIN;
            for( int i=0; i<lfoMasterRangeV4; ++i, ++iRPB, ++iDPB, ++iMLFO ) {
                value.v4[iRPB] = value.v4[iDPB] + value.v4[iMLFO];
            }            
        }
    }
    
private:
    InnerController();
    V4array<int32_t,arraySizeExp>   value;   // low half > filtered, high half original
    uint8_t                         shiftleft[arraySize];   
    
    // volume control characteristic
    static float expFuncTable[midiStep];
    
    // lfo phase control 0 .. PI 
    static uint32_t phaseFuncTable[midiStep];
};


struct ControlledValue {
    int32_t     value;  // updated value from the InnerController
    int32_t     y0;     // y(0) for linear mapping
    int16_t     slope;  // multiplier for linear mapping
    uint16_t    index;  // index in InnerController
    
    inline bool update(void) 
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


#if 0

union ControllerT {
    int32_t i;
    float   f;  // not needed
};
// obsolate

class ControllerMatrix {
public:
    static constexpr uint16_t controllerCountExp    = 8;
    static constexpr uint16_t controllerCount       = 1<<controllerCountExp;
    static constexpr uint16_t controllerCountMask   = controllerCount-1;
    enum {
        C_NULL,                 // always 0 -- null controller -- shouid not be assigned
        C_FLOAT_AMPLITUDE,      // internal - current amplitude summ from osc unit
        C_INT_PITCHBEND,
        C_INT_CHANNELAFTERTOUCH,
        C_INT_FILTER_Q,
        C_INT_FILTER_FREQUENCY1,
        C_INT_FILTER_FREQUENCY2,

        // ...
        // phaser....
        // flanger...
        // reverb....
        // echo....
        C_END  // end marker -- not used as controller
    };

    static_assert( C_END <= controllerCount, "controller matrix too small");

    inline static ControllerMatrix& getInstance(void)
    {
        static ControllerMatrix instance;
        return instance;
    }

    void reset(void)
    {
        memset(value,0, sizeof(value));
        value[C_INT_PITCHBEND].i = 1<<13;  // this is the zero bend
    }

    inline void setFloat( uint16_t ind, const float v )
    {
        value[ind&controllerCountMask].f = v;
    }

    inline float getFloat( uint16_t ind ) const
    {
        return value[ind&controllerCountMask].f;
    }

    inline int32_t get( uint16_t ind ) const
    {
        return value[ind&controllerCountMask].i;
    }

    inline void setMidiHL( const uint16_t ind, const uint8_t hv, const uint8_t lv )
    {
        value[ind&controllerCountMask].i = (hv<<7) + lv;
    }

    inline void setMidiL( const uint16_t ind, const uint8_t lv )
    {
        value[ind&controllerCountMask].i = lv;
    }

    inline void setMidiH( const uint16_t ind, const uint8_t lv )
    {
        value[ind&controllerCountMask].i = (lv<<7) + lv;
    }

    // OSC : 16 bit unsigned -> as negative
    inline void setOSC( const uint16_t ind, const uint16_t v )
    {
        value[ind&controllerCountMask].i = -v;
    }

private:
    ControllerMatrix()
    { reset(); };
<<<<<<< HEAD
    
=======

>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
    ControllerT     value[controllerCount];
};
// --------------------------------------------------------------------
// obsolate

class ControllerOld {
public:
    ControllerOld( const uint16_t ind )
    :   index(ind)
    {};
    ControllerOld()
    :   index(ControllerMatrix::C_NULL)
    {};

    inline bool update(void)
    {
        const int32_t tmp = value.i;
        return tmp != ( value.i = ( ControllerMatrix::getInstance().get(index)) );
    }
    inline int32_t get(void)
    {
        return value.i;
    }
    inline float getFloat(void)
    {
        return value.f;
    }
    inline void setIndex( const uint16_t indexP )
    {
        index = indexP;
    }

protected:
    ControllerT value;
    uint16_t    index;
};
// --------------------------------------------------------------------
// TODO - normalize iterated
// 7777 6666 5555 4444 3333 2222 1111 0000
//-----------------------------------------
// 0000 0000 xxxx xxx0 0000 0000 0000 0000 7 bit MIDI simple
// 0000 0000 xxxx xxxx xxxx xx00 0000 0000 7 bit MIDI duplicated
// 0000 0000 xxxx xxxx xxxx xx00 0000 0000 14 bit MIDI
// 0000 0000 xxxx xxxx xxxx xxxx 0000 0000 16 bit osc

// on the MIDI side:
// 7777 6666 5555 4444 3333 2222 1111 0000
//-----------------------------------------
// 14 bit
// 0000 0000 0000 0000 0044 4444 4444 4444
// 7 bit -> setMidiH
// 0000 0000 0000 0000 0077 7777 7777 7777
// 0000 0000 0000 0000 0000 0000 0777 7777 -> for indexing &0x7 -> X-Y


class ControllerIterated : public ControllerOld {
public:
    ControllerIterated( const uint16_t ind )
    :   ControllerOld(ind)
    {};
    ControllerIterated()
    :   ControllerOld()
    {};
    static constexpr uint8_t  rangeExp      = 10;    // 10 -> 24 bit
    static constexpr uint8_t  normExp       = 24;
    static constexpr uint8_t  itrangeExp    = 7;
    static constexpr int32_t  dvalueLimit   = 1<<itrangeExp;

    inline static float renorm ( const int32_t v, const uint8_t unorm = 0 )
    {
        return static_cast<float>( v ) / (1<<(normExp-unorm));
    };

    // OSC TODO -> 16 bit as negative
    inline bool update(void)   // hide base class update()
    {
        if( ++xcount & 0xF )
            return false;

        const int32_t tmp = value.i;
        value.i = ControllerMatrix::getInstance().get(index)<<rangeExp;
        if( tmp == value.i ) {
            return 0 <= dcount;
        }

        const int32_t dy = value.i - current;
        dvalue  = dy >> itrangeExp ;
        if( dvalue > dvalueLimit ) {
            dcount  = 1 << itrangeExp;
            return true;
        } else if( dvalue < -dvalueLimit ) {
            ++dvalue; // round up
            dcount  = 1 << itrangeExp;
            return true;
        } else {
            current = value.i;
            dvalue  = 0;
            dcount  = 0;
            return true;
        }
    }

    inline int32_t get(void)  // hide base class get()
    {
        if( 0 < dcount ) {
            --dcount;
            return current += dvalue;
        }
        dcount = -1;
        return value.i;
    }

    // get normalized float : default: 1.0f

    inline float getNorm( const uint8_t unorm=0 )
    {
        return renorm( get(), unorm );
    }

protected:
    int32_t current;
    int32_t dvalue;
    int8_t  dcount;
    int8_t  xcount;
};

#endif
// --------------------------------------------------------------------
} // end namespace yacynth

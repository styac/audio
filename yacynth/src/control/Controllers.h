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

#include    <cstdint>
#include    <string>
#include    <array>
#include    <atomic>
#include    <cstring>

namespace yacynth {
// --------------------------------------------------------------------

//
union ControllerT {
    int32_t i;
    float   f;
};

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
    
    ControllerT     value[controllerCount];
};
// --------------------------------------------------------------------
class Controller {
public:
    Controller( const uint16_t ind )
    :   index(ind)
    {};
    Controller()
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


class ControllerIterated : public Controller {
public:
    ControllerIterated( const uint16_t ind )
    :   Controller(ind)
    {};
    ControllerIterated()
    :   Controller()
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
// --------------------------------------------------------------------
} // end namespace yacynth

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
 * File:   yamsg.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on January 29, 2016, 11:01 PM
 */
#include    <cstdint>
#include    <atomic>

namespace yacynth {

//
// message structure
//  RT msg - 8 byte long
//
// SYSTEM
//  byte7   byte6   byte5   byte4   byte3   byte2   byte1   byte0
//------------------------------------------------------------------------
//  0       0       0       0       0       0       0       1
//
//
//  MIDI
//  byte7   byte6   byte5   byte4   byte3   byte2   byte1   byte0
//------------------------------------------------------------------------
//  MIDI NOTE OFF
//  0       1       chn     note    0       0       VH      VL
//  MIDI NOTE ON
//  0       2       chn     note    0       0       VH      VL
//  MIDI POLY AFTERTOUCH
//  0       3       chn     note    0       0       VH      VL
//  MIDI CONTROL
//  0       4       chn     contr   0       0       VH      VL
//                                  diffH   diffL
//  MIDI PROGRAM CHANGE
//  0       5       chn     0       0       0       VH      VL
//  MIDI CHANNEL AFTERTOUCH
//  0       6       chn     0       0       0       VH      VL
//  MIDI PITCH BEND
//  0       7       chn     0       0       0       VH      VL
//                                  diffH   diffL
//
//
//  VH-VL   = signed> -32768...+32767 -> generally the same as in the MIDI
//  velocity: VH=0 VL=0..127
//  pitch bend -0x2000 -- 0 means no bend
//
//    1 004c 0000 0064
//    2 004c 0000 0000
// 0020 xxxx xxxx xxxx

// controller:
//  op = reset, set, add -- int 32 bit
//
enum YAMOP : uint64_t {
    YAMOP_MIDI_PREFIX       = 0x00, // obsolate - will disappear
    YAMOP_SYSTEM_STOP       = 0x0000000000000001LL,
//=============================================================================
    YAMOP_CONTROL_PREFIX    = 0x01,

//=============================================================================
// think: start a voice then change with interpolation
// glissando, legato, etc
// value FF, FFFF is the NOP/KEEP
//
//  byte7   byte6   byte5   byte4   byte3   byte2   byte1   byte0
//------------------------------------------------------------------------
//  2       oscNr   velocity(u16)   --- pitch (u32) -------------

    YAMOP_SETVOICE_NOTE     = 0x02,
//=============================================================================
//  byte7   byte6   byte5   byte4   byte3   byte2   byte1   byte0
//------------------------------------------------------------------------
//  2       oscNr   spctInd envInd  delta(int16)    velocity Hi/LO
//
//
    YAMOP_CHNGVOICE_NOTE    = 0x03,

};



enum YAMOP_MIDI : uint64_t {
    YAMOP_MIDI_NOTE_OFF     = 0x01LL,
    YAMOP_MIDI_NOTE_ON      = 0x02LL,
    YAMOP_MIDI_PLY_AFTCH    = 0x03LL,
    YAMOP_MIDI_CONTROL      = 0x04LL,
    YAMOP_MIDI_PROG         = 0x05LL,
    YAMOP_MIDI_CHN_AFTCH    = 0x06LL,
    YAMOP_MIDI_PITCHBEND    = 0x07LL,
};

//
// input module
//
//
struct Yform1 {
    int16_t     paramX;     // velocity, value...
    int16_t     paramY;     // velocity, value...
    uint8_t     select2;    // note, controller, etc
    uint8_t     select1;    // channel note, controller, etc
    uint8_t     subcode;
    uint8_t     opcode;
};

struct Yform2 {
    uint8_t     paramX_L;   // velocity, value...
    uint8_t     paramX_H;   // velocity, value...
    uint8_t     paramY_L;   // velocity, value...
    uint8_t     paramY_H;   // velocity, value...
    uint8_t     select2;    // note, controller, etc
    uint8_t     select1;    // channel note, controller, etc
    uint8_t     subcode;
    uint8_t     opcode;
};

struct Yform3 {
    int32_t     param;      // controller value
    uint8_t     select2;    // controller
    uint8_t     select1;    // channel
    uint8_t     subcode;    // ctrl reset, set, add
    uint8_t     opcode;
};

// YAMOP_SETVOICE
struct YformSV {
    uint32_t    pitch;
    uint16_t    velocity;
    uint8_t     oscNr;
    uint8_t     opcode;
};

// YAMOP_CONTROL
struct YformCC {
    int16_t     valueX;
    int16_t     valueY;
    uint8_t     param;
    uint8_t     operatorNr;
    uint8_t     controllerNr;
    uint8_t     opcode;
};

struct YformV {
    uint8_t     byte[8];
};

union Yamsgrt {
    Yamsgrt()
    :   store(0)
    {};
    uint64_t        store;
    YformV          vec;
    Yform1          f1;
    Yform2          f2;
    YformSV         setVoice;
    YformCC         setController;
};

} // end namespace yacynth


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

// message structure
//  RT msg - 8 byte long
//
//  MIDI
//  byte7   byte6   byte5   byte4   byte3   byte2   byte1   byte0
//------------------------------------------------------------------------
//  MIDI NOTE OFF
//  FF      1       chn     note    0       0       VH      VL
//  MIDI NOTE ON
//  FF      2       chn     note    0       0       VH      VL
//  MIDI POLY AFTERTOUCH
//  FF      3       chn     note    0       0       VH      VL
//  MIDI CONTROL
//  FF      4       chn     contr   0       0       VH      VL
//                                  diffH   diffL
//  MIDI PROGRAM CHANGE
//  FF      5       chn     0       0       0       VH      VL
//  MIDI CHANNEL AFTERTOUCH
//  FF      6       chn     0       0       0       VH      VL
//  MIDI PITCH BEND
//  FF      7       chn     0       0       0       VH      VL
//                                  diffH   diffL
//
//=============================================================================
// redesign:
//  bit7 = 0
//    YAMOP_SETVOICE_NOTE     = 0
//  velocity(u15)   oscNr  TS select --- pitch (u32) -------------
//  velocity: 15 bit cannot be 0 ! - highest bit = 0
// TS select 8 bit
// oscNr 8 bit

//YAMOP_VOICE_SET
//=============================================================================
//  byte7   byte6   byte5   byte4   byte3   byte2   byte1   byte0
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//  velocity(u15)  TSVector  oscNr  --- pitch (u32) -------------
//


//=============================================================================
//  byte7   byte6   byte5   byte4   byte3   byte2   byte1   byte0
//------------------------------------------------------------------------
// YAMOP_SET_OVERTONECOUNT
//  0x8x   subcode  -- maxOvertone-    ------ rfu ----------------

// mode 0 - no control
// mode 1 - velocity   dependent index 0..127
// mode 2 - controller dependent index 0..127

// array<uint8_t,127> -- for each TS ? -- probably enough for 32..64

enum YAMOP : uint8_t {
    YAMOP_VOICE_RELEASE = 0x80,
    YAMOP_VOICE_CHANGE,
    YAMOP_SET_OVERTONECOUNT,

    // last range
    YAMOP_GLOBAL = 0xFF
};

// part of YAMOP_GLOBAL space
constexpr uint64_t  YAMOP_SYSTEM_STOP = uint64_t(-1LL);

#if 0
enum YAMOP_MIDI : uint64_t {
    YAMOP_MIDI_NOTE_OFF     = 0x01LL,
    YAMOP_MIDI_NOTE_ON      = 0x02LL,
    YAMOP_MIDI_PLY_AFTCH    = 0x03LL,
    YAMOP_MIDI_CONTROL      = 0x04LL,
    YAMOP_MIDI_PROG         = 0x05LL,
    YAMOP_MIDI_CHN_AFTCH    = 0x06LL,
    YAMOP_MIDI_PITCHBEND    = 0x07LL,
};
#endif
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

// NEW
struct YformVoiceSet {
    uint32_t    pitch;
    uint8_t     oscNr;
    uint8_t     toneBank;
    uint8_t     velocity;
    int8_t      rfu;
};


struct YformVoiceChange {
    uint32_t    pitch;
    uint16_t    velocity;
    uint8_t     oscNr;
    uint8_t     opcode;
};

struct YformVoiceRelease {
    uint32_t    rfu;
    uint16_t    tickRelease;
    uint8_t     oscNr;
    uint8_t     opcode;
};

struct YformOvertone {
    uint32_t    rfu;
    uint16_t    maxOvertone;
    uint8_t     subcode;
    uint8_t     opcode;
};

union Yamsgrt {
    Yamsgrt()
    :   store(0)
    {};
    uint64_t            store;
    YformV              vec;
    Yform1              f1;
    Yform2              f2;
//    YformSV             setVoiceOld;
    YformCC             setController;

    // NEW
    YformVoiceChange    voiceChange;
    YformVoiceRelease   voiceRelease;
    YformVoiceSet       voiceSet;
    YformOvertone       overtoneControl;
};

} // end namespace yacynth


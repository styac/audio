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

//=============================================================================
// redesign:
//  bit7 = 0
//    YAMOP_SETVOICE_NOTE     = 0
//      TS select   8 bit
//      oscNr       8 bit
//      channel     4 bit -- not implemented

//YAMOP_VOICE_SET
//=============================================================================
//  byte7           byte6   byte5       byte4       byte3   byte2   byte1   byte0
//------------------------------------------------------------------------
//  [0000 channel]  oscNr   velocity    TSVector    --- pitch (u32) -------------
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
    YAMOP_VOICE_RELEASE     = 0x80,
    YAMOP_VOICE_CHANGE      = 0x90,
    YAMOP_SET_OVERTONECOUNT = 0xA0,
    YAMOP_RFU_B             = 0xB0,
    YAMOP_RFU_C             = 0xC0,
    YAMOP_RFU_D             = 0xD0,
    YAMOP_RFU_E             = 0xE0,
    YAMOP_RFU_F             = 0xF0,
};

// part of YAMOP_GLOBAL space
constexpr uint64_t  YAMOP_SYSTEM_STOP = uint64_t(-1LL);


// NEW
#if 0
struct YformVoiceSet {
    uint32_t    pitch;
    uint8_t     velocity;
    uint8_t     toneBank;
    uint8_t     oscNr;
    int8_t      rfu;    // low 4 bit can be channel
};
#endif

// obsolete
struct YformVoiceSet {
    uint32_t    pitch;
    uint8_t     oscNr;
    uint8_t     toneBank;
    uint8_t     velocity;
    int8_t      rfu;
};

struct YformVoiceChange {
    uint32_t    pitch;
    uint8_t     velocity;
    uint8_t     rfu;
    uint8_t     oscNr;
    uint8_t     opcode; // low 4 bit can be channel
};

struct YformVoiceRelease {
    uint32_t    rfu;
    uint16_t    tickRelease;
    uint8_t     oscNr;
    uint8_t     opcode; // low 4 bit can be channel
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
    uint8_t             transfer[8];
    YformVoiceChange    voiceChange;
    YformVoiceRelease   voiceRelease;
    YformVoiceSet       voiceSet;
    YformOvertone       overtoneControl;
};

// -----------------------------------------
// MING control

struct MingSync {
    uint32_t    tick;       // sample count 192kHz
    uint8_t     beat;       // beat number % 256
    uint8_t     rfu1;
    uint8_t     rfu2;
    uint8_t     opcode;
};

union MingPrefix {
    uint64_t    store;
    uint8_t     transfer[8];
    MingSync    sync;
};

struct Ming {
    MingPrefix  op0;
    Yamsgrt     op1;
};

} // end namespace yacynth


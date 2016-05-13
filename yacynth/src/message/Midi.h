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
 * File:   Midi.h
 * Author: Istvan Simon
 *
 * Created on February 14, 2016, 7:50 PM
 */
#include    "yamsg.h"



namespace yacynth {

// --------------------------------------------------------------------

enum midi_status_t : uint8_t {
    MIDI_NOTE_OFF   = 0x08,
    MIDI_NOTE_ON    = 0x09,
    MIDI_PLY_AFTCH  = 0x0A,
    MIDI_CONTR_CHNG = 0x0B,
    MIDI_PROG_CHNG  = 0x0C,
    MIDI_CHN_AFTCH  = 0x0D,
    MIDI_PITCH      = 0x0E,
    MIDI_SYS        = 0x0F
};
#if 0

// --------------------------------------------------------------------

inline uint64_t YamsgSetNoteOn( uint8_t chn, uint8_t note, uint8_t vel )
{
    Yamsgrt msg;
    msg.f1.subcode  = YAMOP_MIDI_NOTE_ON;
    msg.f1.select1  = chn;
    msg.f1.select2  = note;
    msg.f1.paramX   = vel;
    return msg.store;
}

// --------------------------------------------------------------------

inline uint64_t YamsgSetNoteOff( uint8_t chn, uint8_t note, uint8_t vel )
{
    Yamsgrt msg;
    msg.f1.subcode  = YAMOP_MIDI_NOTE_OFF;
    msg.f1.select1  = chn;
    msg.f1.select2  = note;
    msg.f1.paramX   = vel;
    return msg.store;
}

// --------------------------------------------------------------------

inline uint64_t YamsgSetPolyAftch( uint8_t chn, uint8_t note, uint8_t vel )
{
    Yamsgrt msg;
    msg.f1.subcode  = MIDI_PLY_AFTCH;
    msg.f1.select1  = chn;
    msg.f1.select2  = note;
    msg.f1.paramX   = vel;
    return msg.store;
}

// --------------------------------------------------------------------

inline uint64_t YamsgSetChnAftch( uint8_t chn, uint8_t vel )
{
    Yamsgrt msg;
    msg.f1.subcode  = MIDI_CHN_AFTCH;
    msg.f1.select1  = chn;
    msg.f1.paramX   = vel;
    return msg.store;
}

// --------------------------------------------------------------------

inline uint64_t YamsgSetProgChng( uint8_t chn, uint8_t vel )
{
    Yamsgrt msg;
    msg.f1.subcode  = YAMOP_MIDI_PROG;
    msg.f1.select1  = chn;
    msg.f1.paramX   = vel;
    return msg.store;
}
// --------------------------------------------------------------------

inline uint64_t YamsgSetPitch( uint8_t chn, uint8_t vl, uint8_t vh )
{
    Yamsgrt msg;
    msg.f1.subcode  = YAMOP_MIDI_PITCHBEND;
    msg.f1.select1  = chn;
//    msg.f1.paramX   = (vh<<7) + vl - 0x02000; // will be signed
    msg.f1.paramX   = (vh<<7) + vl ; // signed
    return msg.store;
}

// --------------------------------------------------------------------

inline uint64_t YamsgSetControllerLSB( uint8_t chn, uint8_t ccn, uint8_t vl )
{
    Yamsgrt msg;
    msg.f2.subcode  = YAMOP_MIDI_CONTROL;
    msg.f2.select1  = chn;
    msg.f2.select2  = ccn;
    msg.f2.paramX_L = vl;
    return msg.store;
}

// --------------------------------------------------------------------

inline uint64_t YamsgSetControllerMSB( uint8_t chn, uint8_t ccn, uint8_t vh )
{
    Yamsgrt msg;
    msg.f2.subcode  = YAMOP_MIDI_CONTROL;
    msg.f2.select1  = chn;
    msg.f2.select2  = ccn;
    msg.f2.paramX_H = vh;
    return msg.store;
}
#endif
// --------------------------------------------------------------------

} // end namespace yacynth


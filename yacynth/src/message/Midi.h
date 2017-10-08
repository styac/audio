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
    MIDI_NOTE_OFF           = 0x08,
    MIDI_NOTE_ON            = 0x09,
    MIDI_POLY_AFTERTOUCH    = 0x0A,
    MIDI_CONTROL_CHANGE     = 0x0B,
    MIDI_PROGRAM_CHANGE     = 0x0C,
    MIDI_CHANNEL_AFTERTOUCH = 0x0D,
    MIDI_PITCH              = 0x0E,
    MIDI_SYS                = 0x0F
};

struct  RouteIn {
    RouteIn() : all(0) {};
    union {
        uint32_t  all;
        struct {
            uint8_t op;
            uint8_t channel;
            uint8_t note_cc_val;
            uint8_t velocity_val;            
        };
    };
};

// --------------------------------------------------------------------

} // end namespace yacynth


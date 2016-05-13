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
 * File:   SimpleMidiRouter.cpp
 * Author: Istvan Simon
 *
 * Created on February 27, 2016, 10:15 PM
 */
#include    "SimpleMidiRouter.h"
#include    "../message/Midi.h"

#include    <cstdint>
#include    <iostream>
#include    <iomanip>

namespace yacynth {

// --------------------------------------------------------------------
uint32_t SimpleMidiRouter::getPitch( int32_t noteNr, uint16_t tableNr )
{
    const int32_t currNote  = ( noteNr + transposition ) * microResolution;
    const int32_t noteInd   =  currNote < 0 ? 0 : ( currNote >= tuningTableSize ? tuningTableSize-1 : currNote );
    return pitch[ tableNr & ( tuningTableCount - 1)][ noteInd ];
} // end SimpleMidiRouter::getPitch
// --------------------------------------------------------------------
// polyphonic translate
Yamsgrt SimpleMidiRouter::translate( const RouteIn& in )
{
    out.store = 0;
    switch( in.op ) {
    case MIDI_NOTE_OFF:
        out.setVoice.opcode = YAMOP_CHNGVOICE_NOTE;
        out.setVoice.oscNr      = in.note_cc_val;
        out.setVoice.velocity   = 0;    // ?
        out.setVoice.pitch      = 0;    // OFF
        break;
    case MIDI_NOTE_ON:
        if( 0 == in.velocity_val ) {
            out.setVoice.opcode = YAMOP_CHNGVOICE_NOTE;
            out.setVoice.oscNr      = in.note_cc_val;
            out.setVoice.velocity   = 0;    // ?
            out.setVoice.pitch      = 0;    // OFF
        } else {
            out.setVoice.opcode = YAMOP_SETVOICE_NOTE;
            out.setVoice.oscNr      = in.note_cc_val;
            out.setVoice.velocity   = in.velocity_val << 9; // no fine value - 0..127
            out.setVoice.pitch      = getPitch( in.note_cc_val, 0 );
        }
        break;

    case MIDI_PLY_AFTCH:
        break;
    case MIDI_CONTR_CHNG:
        switch(in.note_cc_val) {
        case 0x29:
        case 0x1b:
            ControllerMatrix::getInstance().setMidiH( ControllerMatrix::C_INT_FILTER_FREQUENCY1, in.velocity_val );
            break;
        case 0x2A:
        case 0x1A:
            ControllerMatrix::getInstance().setMidiH( ControllerMatrix::C_INT_FILTER_Q, in.velocity_val );
            break;

        }
        break;
    case MIDI_PROG_CHNG:
        break;
    case MIDI_CHN_AFTCH:
        break;
    case MIDI_PITCH:
        ControllerMatrix::getInstance().setMidiHL( ControllerMatrix::C_INT_PITCHBEND, in.velocity_val, in.note_cc_val );
        break;
    }
    return out;
} // end SimpleMidiRouter::translate

// --------------------------------------------------------------------

void SimpleMidiRouter::setTransposition( int8_t val )
{
  transposition = ( val >= noteperOctave ? noteperOctave-1 : ( val <= -noteperOctave ? -(noteperOctave-1) : val ), 0 );
} // end SimpleMidiRouter::setTransposition

// --------------------------------------------------------------------
} // end namespace yacynth

#pragma once
/*
 * Copyright (C) 2017 ist
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
 * File:   Router.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on October 4, 2017, 7:07 PM
 */

#include    "yacynth_globals.h"
#include    "../message/yamsg.h"
#include    "../message/Midi.h"
#include    "../control/Controllers.h"
#include    "../router/ControlQueue.h"
#include    "../router/TuningTables.h"

#include    <iostream>
#include    <fstream>
#include    <iomanip>

namespace yacynth {


class Router {
public:
    static constexpr uint16_t   midiChannelCount = 16;
    
    enum NotePlayMode : uint8_t {
        // 1 channel in poliphonicNote - oscNr = noteNr 0..127
        // other controlNote
        poliphonicNote,   
        
        // 0..15 channel in monophonicNote - oscNr = ( channelNr 0..15 ) + 112 highest 16 osc 
        // other controlNote
        monophonicNote,
        
        // if any channel in monophonicNote the 1 can be in monopoliphonicNote
        // 
        monopoliphonicNote,     // oscNr = noteNr 16..127 for poli channel (only 1) 0..15 mono
        controlNote             // note is control
    };
    
    // check - 1 channel poliphonicNote + 15 channel controlNote
    // check - 1 channel monopoliphonicNote + 15 channel monophonicNote or controlNote
    bool checkChannelMode() const
    {
        int poliphonicNoteCount = 0;
        int monophonicNoteCount = 0;
        int monopliphonicNoteCount = 0;
        for( auto npm : notePlayMode ) {
            switch( npm ) {
            case poliphonicNote:
                ++poliphonicNoteCount;
                break;

            case monophonicNote:            
                ++monophonicNoteCount;
                break;

            case monopoliphonicNote:
                ++monopliphonicNoteCount;
                break;                
            }
        }
        if( ( poliphonicNoteCount + monopoliphonicNote ) > 1 ) {
            return false;
        }
        if( poliphonicNoteCount > 0 && monophonicNoteCount > 0 ) {
            return false;
        }
        return true;
    }
        
    Router() = delete;
    Router( ControlQueueVector& inQueue );

    ~Router() = default;

    void clear(void);
    
    static void midiInCB( void *, RouteIn in ); // callback
   
    inline MidiController&  getMidiController(void) { return midiController; }

    bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex ); 
    
protected:
    
    void inline processMidi( RouteIn in )
    {
        Yamsgrt out;
        uint8_t channel( in.channel );
        uint8_t oscillatorNr( in.note_cc_val );
        uint8_t controllerNr( in.note_cc_val );
        uint8_t noteNr( in.note_cc_val );
        uint8_t instruction( in.op );
        uint8_t velocity( in.velocity_val * 2 );    // 8 bit
        uint8_t controllerValue( in.velocity_val );

        switch( notePlayMode[ channel ] ) {
        case poliphonicNote:
            break;
            
        case monophonicNote:            
            // osc 0..15 would ok under 20 Hz
            oscillatorNr = channel + (127-15);  // highest oscillators above 6kHz not used
            break;
            
        case monopoliphonicNote:
            if( oscillatorNr >= (127-15) ) {
                return; // reserved for monophonic
            }
            break;
            
        case controlNote:
            switch( instruction ) {
            case MIDI_NOTE_OFF:
            case MIDI_NOTE_ON:
            case MIDI_CHANNEL_AFTERTOUCH:                
            case MIDI_POLY_AFTERTOUCH:
                instruction = MIDI_CONTROL_CHANGE;
                break;
            }
            break;
            
        default:
            return;
        }
        
        switch( instruction ) {
        case MIDI_NOTE_OFF:
            out.voiceRelease.opcode         = YAMOP_VOICE_RELEASE;
            out.voiceRelease.oscNr          = oscillatorNr;
            out.voiceRelease.tickRelease    = 0;    // ?
            queueIn.queueOscillator.put( out.store );
            return;

        case MIDI_NOTE_ON:
            if( 0 == in.velocity_val ) {
                out.voiceRelease.opcode         = YAMOP_VOICE_RELEASE;
                out.voiceRelease.oscNr          = oscillatorNr;
                out.voiceRelease.tickRelease    = 0;    // ?
                queueIn.queueOscillator.put( out.store );
                return;
            }
            out.voiceSet.oscNr          = oscillatorNr;
            out.voiceSet.toneBank       = 0;
            out.voiceSet.velocity       = velocity;
            out.voiceSet.pitch          = midiTuningTables.get( noteNr,  channel );
            queueIn.queueOscillator.put( out.store );
            return;

        case MIDI_POLY_AFTERTOUCH:
    //        out.setVoice.opcode = YAMOP_CHNGVOICE_NOTE;
    //        out.setVoice.oscNr      = in.note_cc_val;
    //        out.setVoice.velocity   = velocity;    // ?
    //        out.setVoice.pitch      = 0;    // OFF
            break;

        case MIDI_CONTROL_CHANGE: {
            MidiController::ControlData cdt =
                midiController.getControlData( channel, in.note_cc_val );

            switch( cdt.mode ) {
            case MidiController::CM_DISABLE :
                break;

            case MidiController::CM_RANGE : // range page 0..255
                InnerController::getInstance().setMidi( cdt.index, controllerValue );
                break;

            case MidiController::CM_INC :   // increment by 1 -- there should be a limiter
                InnerController::getInstance().incMidiSwitch( cdt.index );
                break;

            case MidiController::CM_DEC :   // decrement by 1 -- there should be a limiter
                InnerController::getInstance().decMidiSwitch( cdt.index );
                break;

            case MidiController::CM_SET : // set value: same as setMidi on the switch page
                InnerController::getInstance().setMidiSwitch( cdt.index, controllerValue );
                break;

            default: // set value: should be 0..127 - not checked here -- radio button -- value == cdt.mode
                InnerController::getInstance().setMidiSwitch( cdt.index, cdt.mode );
                break;

            }
            break;
        }

        case MIDI_PROGRAM_CHANGE:
            break;

        case MIDI_CHANNEL_AFTERTOUCH:
            InnerController::getInstance().setMidi( midiController.getAftertouch( channel ), velocity );
            break;

        case MIDI_PITCH:
            InnerController::getInstance().setMidi( midiController.getPitchbend( channel ), controllerValue, controllerNr );
            break;
        }
        return;
    }

    ControlQueueVector&     queueIn;
    MidiController          midiController;
    MidiTuningTables        midiTuningTables;
    NotePlayMode            notePlayMode[ midiChannelCount ];
};


} // end namespace yacynth 



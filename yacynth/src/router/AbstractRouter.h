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
 * File:   AbstractRouter.h
 * Author: Istvan Simon
 *
 * Created on February 27, 2016, 9:00 PM
 */

#include    "yacynth_globals.h"
#include    "../message/yamsg.h"
#include    "../message/Midi.h"
#include    "../control/Controllers.h"
#include    "../router/ControlQueue.h"

namespace yacynth {

//
// MIDI structure
//  - op=0..15
//  - chn=0..15
//  - note_cc = note number or controller number - parameter 1
//  - velocity_val = velocity or controller value
//
// Tuning concept:
//      equal tempered: max 72 TET ( default )
//          12 * microResolution / octave
//          11 octaves
//          normal 12TET is set to 0 .. microResolution-1 ...
//
//      just intonation:
//          first N places/octave
//          0...N-1 then zero
//
//      non octave centric
//
//
//

// TODO: reduce AbstractRouter + SimpleRouter to Router 

class AbstractRouter {
public:
    
// http://xenharmonic.wikispaces.com/
    
    enum    tuned_t {
        JI_43_PARTCH,
        ET_72,      // default on table[0]
#if 0
        TET15,
        TET17,
        TET19,
        TET22,
        TET31,
        TET34,
        TET41,
        TET53,
#endif
// alpha .. delta, Bohlen Pierce, ...
    };
    
    AbstractRouter() = delete;
    AbstractRouter( ControlQueueVector& inQueue );

    virtual ~AbstractRouter() = default;

    void clear(void) {};
    bool fill(  std::stringstream& ser ) {};
    void query( std::stringstream& ser ) {};
    
    static void midiInCB( void *, RouteIn in );
   
    virtual void        processMidi( RouteIn in ) = 0;

    // get the direct addressed notes from a table
    virtual uint32_t    getPitch( int32_t noteNr, uint8_t tableNr = 0 );

    // get the notes relative to the 12 grade system : note + ( -2 ... 0 ... +2 )
    virtual uint32_t    getPitch( int32_t noteNr, int8_t mod, uint8_t tableNr );
    virtual void        setTransposition( int8_t val );
    void    setCustomTuning( tuned_t custune );
    tuned_t getCustomTune( void) const { return tuned; };

    static constexpr uint16_t   noteperOctave       =   12;
    static constexpr uint16_t   microResolution     =   6;
    static constexpr uint16_t   maxTransposition    =   noteperOctave * microResolution;
    static constexpr uint16_t   octaveCount         =   11; // because of the MIDI
    static constexpr uint16_t   A440MidiNote        =   69;
    static constexpr uint16_t   A440MidiNoteMicro   =   A440MidiNote * microResolution;
    static constexpr uint16_t   tuningTableCount    =   2;  // standard - custom : Partch43
    static constexpr uint16_t   tuningTableSize     =   noteperOctave * microResolution * octaveCount;
    static constexpr uint16_t   partch43Size        =   43;
    static constexpr double     octaveResolution    =   (1<<24);    // 64k*256
    static constexpr double     equalTemperedNote   =   octaveResolution / 12.0 / microResolution;
    static constexpr double     pitch0MidiEqual     =   refA440ycentDouble - A440MidiNoteMicro * equalTemperedNote;
    // A440 : octave 5 note 31 (0..42) - major sixth
    static constexpr double     pitch0MidiPartch43  =   pitch0MidiEqual + refA440ycentDouble - 0x1937b33a;

    static constexpr  int16_t   pitchBendMax = 8192;
    static constexpr  int16_t   pitchOffset  = -pitchBendMax;
        // 8192 =>  2 semitones
    //  1 octave = octaveResolution = 1<<24
    //  2 semitones =
    // NO RPN here
    static constexpr  uint32_t pitchMult    =  (octaveResolution/6) / pitchBendMax;

    void    setMode( int16_t val ) { monoPhone = val != 0; };
    void    fillEqualTempered(  uint8_t tableNr );
    void    fillPartch43(       uint8_t tableNr );

    inline MidiController&  getMidiController(void) { return midiController; }

    virtual bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex ); 

    void  sendToneControl();
    
protected:
    ControlQueueVector& queueIn;
    MidiController      midiController;
    Yamsgrt             out; // function local
    std::array< std::array<uint32_t, tuningTableSize>, tuningTableCount> pitch;
    tuned_t             tuned;
    int16_t             toneBank;
    int8_t              transposition;
    bool                monoPhone;
};


} // end namespace yacynth

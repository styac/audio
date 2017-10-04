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
 * File:   TuningTables.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on October 4, 2017, 7:07 PM
 */

// next generation 

#include    "yacynth_globals.h"
#include    "protocol.h"

#include    <array>

namespace yacynth {

class TunerSet {
public:
    enum    tuned_t {
        // JI_n just intonation
        // ET_n equal tempered
        
        JI_43_PARTCH,
        ET_72,          // default on table[0] also ET_12,ET_24,ET_36
        ET_53,
        ET_41,
        ET_34,
        ET_31,
        ET_22,
        ET_19,
        ET_17,
        ET_15,
// alpha .. delta, Bohlen Pierce, ...
    };
    
    TunerSet();    

    ~TunerSet() = default;

    void clear(void) {};

    // get the direct addressed notes from a table
    uint32_t    getPitch( int32_t noteNr, uint8_t tableNr = 0 );

    // get the notes relative to the 12 grade system : note + ( -2 ... 0 ... +2 )
    uint32_t    getPitch( int32_t noteNr, int8_t mod, uint8_t tableNr );
    void        setTransposition( int8_t val );
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

    // void    setMode( int16_t val ) { monoPhone = val != 0; };
    void    fillEqualTempered(  uint8_t tableNr );
    void    fillPartch43(       uint8_t tableNr );  
    virtual bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex ); 

    void  sendToneControl();
    
protected:
    std::array< std::array<uint32_t, tuningTableSize>, tuningTableCount> pitch;
    tuned_t             tuned;
    int16_t             toneBank;
    int8_t              transposition;

};

struct MidiNote {    
    uint8_t octave;
    uint8_t semitone;
};

struct MidiNoteVector {    
    static constexpr uint16_t size = 128;
    MidiNote    table[ size ];
};

struct TuneTable {
    
//    int32_t table[];
};

} // end namespace yacynth 



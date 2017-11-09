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

#include    "yacynth_globals.h"
#include    "protocol.h"
#include    "TuningConst.h"
#include    "../control/Controllers.h"

#include    <array>
#include    <iostream>
#include    <fstream>
#include    <iomanip>

using namespace Tuning;

namespace yacynth {

class TuningTable {
public:
    // absolute values (relative to sampling frequency)
    static constexpr double ycent1Hz_48000          = freq2ycentDouble(1.0);    // 1Hz
    static constexpr double ycentA400_48000         = freq2ycentDouble(440.0);  // MIDI 69
    static constexpr double ycentMidi0_48000_ET12   = ycentA400_48000 - 69 * ycentET12Semitone;
    static constexpr double ycentMidi127_48000_ET12 = ycentA400_48000 + (127-69) * ycentET12Semitone;

    static constexpr uint16_t noteCountExp = 7;                 // 128 MIDI note
    static constexpr uint16_t noteCount = 1 << noteCountExp;
    static constexpr uint16_t noteCountMask = noteCount-1;
    static constexpr uint16_t modifierCountExp = 3;             // 0 + 7 modified scale
    static constexpr uint16_t modifierCount = 1 << modifierCountExp;
    static constexpr uint16_t modifierCountMask = modifierCount-1;
    static constexpr uint16_t size = noteCount  * modifierCount;
    static constexpr uint16_t sizeMask = size-1;

    TuningTable();

    ~TuningTable() = default;
    
    bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    inline int32_t get( uint8_t baseNote, uint8_t modifier ) const
    {
        uint16_t note = uint16_t( baseNote & noteCountMask ) << modifierCountExp;
        return relativeYcent[ note | ( modifier & modifierCountMask ) ];
    }

    // linear
    inline int32_t get( uint16_t note ) const
    {
        return relativeYcent[ note & sizeMask ];
    }
    
private:
    void clear();

    void setTransposition( int32_t ycent )
    {
        transientTransposition = ycent;
    }
    
    void setBaseTransposition( int32_t ycent );

    bool fill( TuningType ttype, TuningVariation tv );
    
    bool fill( int32_t * src, uint8_t layer );

    // fill cont. all keys : ET12, ET13, alpha, beta, delta
    void fillETContinuous( uint32_t intervalCount, uint32_t rateNom, uint32_t rateDenom, uint8_t step );

    // fill cont. white keys : ET 5,6,7
    void fillETContinuous1_7( uint32_t intervalCount, uint32_t rateNom, uint32_t rateDenom );

    // fill cont. : ET 8..11
    void fillETContinuous8_11( uint32_t intervalCount, uint32_t rateNom, uint32_t rateDenom );

    void fill( const TuningGenerator12Notes& table );
    
    void setBaseTransposition_MIDI69_A440();

    int32_t         relativeYcent[ size ];    
    int32_t         transientTransposition; // = detune + transposition --> controller ? and microlayer ?
    // ControllerIndex transpositionControllerIndex;
    TuningType      tuningType;
    TuningVariation tuningVariation;
};

class TuningManager {
public:
    inline static TuningManager& getInstance(void)
    {
        static TuningManager instance;
        return instance;
    }

    ~TuningManager() = default;

    inline int32_t get( uint8_t baseNote, uint8_t modifier ) const
    {
        return tuningTable.get( baseNote, modifier );
    }

    inline bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
    {
        return tuningTable.parameter( message, tagIndex, paramIndex );
    }

private:
    TuningManager() = default;
    TuningTable     tuningTable;
};

} // end namespace yacynth



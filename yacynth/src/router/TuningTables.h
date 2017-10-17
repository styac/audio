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

#include    <array>
#include    <iostream>
#include    <fstream>
#include    <iomanip>

using namespace Tuning;

namespace yacynth {

// 1 ChannelTable for each MIDI channel: 16
struct ChannelTable {    
    ChannelTable()
    :   transientTransposition(0)
    ,   tuningTableSelect(0)
    ,   currentMicroModifier(0)
    {}
    
    bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );
        
    int32_t     transientTransposition; // = detune + transposition
    uint8_t     tuningTableSelect;      // tuning table index
    //  encoded     bbb, bb, b,     0,      #,  ##, ### 
    //              7    6   5     0,4      1   2   3
    uint8_t     currentMicroModifier;   // index of microtonal modifier   
};

// any number of TuningTable for fast change : lets 4 or 8
struct TuningTable {    
    // absolute values (relative to sampling frequency)
    static constexpr double ycentA400_48000    = freq2ycentDouble(440.0);  // MIDI 69
    static constexpr double ycentMidi0_48000   = ycentA400_48000 - 69 * ycentET12Semitone;
    static constexpr double ycentMidi127_48000 = ycentA400_48000 + (127-69) * ycentET12Semitone;

    TuningTable()
    :   baseTransposition(ycentMidi0_48000)
    {}
    
    static constexpr uint16_t noteCountExp = 7;                 // 128 MIDI note
    static constexpr uint16_t noteCount = 1 << noteCountExp;
    static constexpr uint16_t noteCountMask = noteCount-1;
    static constexpr uint16_t modifierCountExp = 3;             // for each 6 mod value -3...+3 (0,4 base value)
    static constexpr uint16_t modifierCount = 1 << modifierCountExp;
    static constexpr uint16_t modifierCountMask = modifierCount-1;
    static constexpr uint16_t size = noteCount  * modifierCount;

    bool fill( TuningTypes ttype );
    
    bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );
        
    // modifier = 0     : 0
    // modifier = 1     : 1
    // modifier = 2     : 2
    // modifier = 3     : 3
    // modifier = 4     : 0
    // modifier = 5     : -1
    // modifier = 6     : -2
    // modifier = 7     : -3

    inline int32_t get( uint8_t baseNote, uint8_t modifier ) const 
    {
        uint16_t note = uint16_t(baseNote) << modifierCountExp;
        return relativeYcent[ note | ( modifier & modifierCountMask ) ];
    }
    
    double  baseTransposition;  // base of the table for the generator
    int32_t relativeYcent[  size  ];
};

class TuningTableArray {
public:
    static constexpr uint16_t tuningTableCountExp = 4;
    static constexpr uint16_t tuningTableCount = 1 << tuningTableCountExp;
    static constexpr uint16_t tuningTableCountMask = tuningTableCount - 1;
    
    inline static TuningTableArray& getInstance(void)
    {
        static TuningTableArray instance;
        return instance;
    }

    inline int32_t get( uint8_t baseNote, uint8_t modifier, uint8_t tableIndex ) const 
    {
        return tuningTables[ tableIndex & tuningTableCountMask ].get( baseNote, modifier );
    }    
    
private:
    TuningTableArray();
    TuningTable     tuningTables [ tuningTableCount ];
};

class MidiTuningTables {
public:   
    MidiTuningTables();
    
    static constexpr uint16_t channelTableCountExp = 4; 
    static constexpr uint16_t channelTableCount = 1 << channelTableCountExp;
    static constexpr uint16_t channelTableCountMask = channelTableCount-1;
    static constexpr uint16_t tuningTableCountExp = 4;
    static constexpr uint16_t tuningTableCount = 1 << tuningTableCountExp;
    static constexpr uint8_t  oneShotMicromodifier = 0x80; 
    
    bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );
    
    inline int32_t get( uint8_t baseNote, uint8_t channel ) 
    {
        const uint8_t ch = channel & channelTableCountMask;
        const uint8_t tuningTableIndex = channelTable[ ch  ].tuningTableSelect;
        const uint8_t microModifier = channelTable[ ch  ].currentMicroModifier;
        const int32_t baseYcent = channelTable[ ch  ].transientTransposition;        
        if( microModifier & oneShotMicromodifier ) {
            channelTable[ ch  ].currentMicroModifier = 0;            
        }
        const int32_t relYcent = TuningTableArray::getInstance().get( baseNote, microModifier, tuningTableIndex );
        return relYcent + baseYcent;
    }

    void setTransposition( int32_t ycent, uint8_t channel ) 
    {
        const uint8_t ch = channel & channelTableCountMask;
        channelTable[ ch  ].transientTransposition = ycent;
    }

    void setTuningTableSelect( uint8_t tableIndex, uint8_t channel ) 
    {
        const uint8_t ch = channel & channelTableCountMask;
        channelTable[ ch  ].tuningTableSelect = tableIndex;
    }
    
    void setMicromodifier( uint8_t microModifier, uint8_t channel ) 
    {
        const uint8_t ch = channel & channelTableCountMask;
        channelTable[ ch  ].currentMicroModifier = microModifier;
    }

    void setOneShotMicromodifier( uint8_t microModifier, uint8_t channel ) 
    {
        const uint8_t ch = channel & channelTableCountMask;
        channelTable[ ch  ].currentMicroModifier = microModifier | oneShotMicromodifier;
    }

private:    
    ChannelTable    channelTable[ channelTableCount ];
};
    
} // end namespace yacynth 



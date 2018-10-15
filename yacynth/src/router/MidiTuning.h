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
 * File:   MidiTuning.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on November 6, 2017, 5:37 PM
 */

#include "TuningTables.h"

namespace yacynth {

// 1 ChannelTable for each MIDI channel: 16
struct MidiChannelTable {
    MidiChannelTable()
    :   currentMicroModifier(0)
    ,   baseMicroModifier(0)
    {}
    uint8_t     currentMicroModifier;   // one shot or same as baseMicroModifier index of microtonal modifier -- controller? // page 3 - switch
    uint8_t     baseMicroModifier;      // static index of microtonal modifier -- controller? // page 3 - switch
};

class MidiTuning {
public:
    MidiTuning();

    static constexpr uint16_t channelTableCountExp = 4;
    static constexpr uint16_t channelTableCount = 1 << channelTableCountExp;
    static constexpr uint16_t channelTableCountMask = channelTableCount-1;

    bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    inline int32_t get( uint8_t baseNote, uint8_t channel )
    {
        auto& ch = channelTable[ channel & channelTableCountMask ];
        const int32_t relYcent = TuningManager::getInstance().get( baseNote, ch.currentMicroModifier );
        ch.currentMicroModifier = ch.baseMicroModifier;
        
        std::cout 
            << "\t\t\t\t\t ** cent: " <<  ( relYcent * ycent2cent )
            << std::endl;
        return relYcent;
    }

    void setMicromodifier( uint8_t microModifier, uint8_t channel )
    {
        auto& ch = channelTable[ channel & channelTableCountMask ];
        ch.currentMicroModifier = ch.baseMicroModifier = microModifier;
    }

    void setOneShotMicromodifier( uint8_t microModifier, uint8_t channel )
    {
        auto& ch = channelTable[ channel & channelTableCountMask ];
        ch.currentMicroModifier = microModifier ;
    }

private:
    void clear();
    MidiChannelTable channelTable[ channelTableCount ];
};

} // end namespace yacynth




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
 * File:   TuningTables.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on October 4, 2017, 7:07 PM
 */
#include    "TuningTables.h"
#include    <iostream>
#include    <fstream>
#include    <iomanip>


//  http://xenharmonic.wikispaces.com/
//  http://www.microtonal-synthesis.com/scales.html
//  http://www.wendycarlos.com/resources.html#tables



//  http://www.tonalsoft.com/monzo/partch/scale/partch43-lattice.aspx
//  http://self.gutenberg.org/articles/bohlen%E2%80%93pierce_scale
//  http://www.huygens-fokker.org/docs/modename.html
//  https://en.wikipedia.org/wiki/Five-limit_tuning

// Bohlen-Pierce Scale 
// Pierce 3579b scale,  3 root of 13 
// https://en.wikipedia.org/wiki/Bohlen%E2%80%93Pierce_scale
// https://www.youtube.com/watch?v=z0Xpn4G9_R4

// need def for arrays of "JI" types
constexpr double Tuning::TuningGenerator<TuningTypes::TM_21_JI_PTOLEMY_12>::notes[];
constexpr double Tuning::TuningGenerator<TuningTypes::TM_21_JI_PARTCH_43>::notes[];

namespace yacynth {

// 1 ChannelTable for each MIDI channel: 16

bool ChannelTable::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    
}

bool TuningTable::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    
}

bool MidiTuningTables::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    
}


MidiTuningTables::MidiTuningTables()
{
}


bool TuningTable::fill( TuningTypes ttype )
{
    double ycentBase = baseTransposition;
    uint16_t index = 0;
    switch( ttype ) {
    case TuningTypes::TM_21_JI_PTOLEMY_12:
        for( auto octave = 0u; octave < 11; ++octave ) {
            for( auto note = 0u; note < 12; ++note ) {
                double ycentRel = TuningGenerator<TuningTypes::TM_21_JI_PTOLEMY_12>::getYcentN( note );
                int32_t ycent = std::round( ycentRel + ycentBase );
                for( auto micro = 0u; micro < 8; ++micro ) {
                    relativeYcent[ index++ ] = ycent;
#if 0
    std::cout
        << "index "     << index
        << " octave "   << octave
        << " note "     << note
        << " micro "    << micro
        << " ycent "    << ycent
        << std::endl;
#endif
                    if( index >= size ) {
                        return true;                                
                    }
                }            
            }    
            ycentBase += ycentOctave;
        }        
        return true;        

    case TuningTypes::TM_21_ET_72:
        for( auto octave = 0u; octave < 11; ++octave ) {
            for( auto note = 0u; note < 12; ++note ) {
                double ycentRel = TuningGenerator<TuningTypes::TM_21_ET_72>::getYcentN( 6 * note );
                int32_t ycent = std::round( ycentRel + ycentBase );
                for( auto micro = 0u; micro < 8; ++micro ) {
                    relativeYcent[ index++ ] = ycent;
#if 0
      std::cout
        << "index "     << index
        << " octave "   << octave
        << " note "     << note
        << " micro "    << micro
        << " ycent "    << ycent
        << std::endl;
#endif    
                    if( index >= size ) {
                        return true;                                
                    }
                }            
            }    
            ycentBase += ycentOctave;
        }        
        return true;        
    }
    return false;        
}

TuningTableArray::TuningTableArray()
{
    tuningTables[0].fill( TuningTypes::TM_21_ET_72 );
    tuningTables[1].fill( TuningTypes::TM_21_JI_PTOLEMY_12 );
}

} // end namespace yacynth 



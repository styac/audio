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
 * File:   TuningTablesFill.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on November 9, 2017, 5:46 PM
 */
#include    "TuningTables.h"

namespace Tuning {

extern TuningGenerator12Notes  table_TM_21_Werckmeister3;
extern TuningGenerator12Notes  table_TM_21_Young2;
extern TuningGenerator12Notes  table_TM_21_Pythagorean;
extern TuningGenerator12Notes  table_TM_21_Ptolemy;
extern TuningGenerator43Notes  table_TM_21_Partch43;

};

namespace yacynth {


void TuningTable::fill( const TuningGenerator12Notes& etg )
{
    uint16_t index = 0;
    double ycentBase = 0;
    const double ycentPeriod = etg.getYcentPeriod();
    while( index < size ) {
        uint16_t note = 0;
        while(( index < size ) && ( note < etg.intervalCount )) {
            const int32_t ycent = std::round( etg.getYcentN( note ) + ycentBase );
            for( auto micro = 0u; micro < modifierCount; ++micro ) {
                relativeYcent[ index++ ] = ycent;
            }
            note++;
        }
        ycentBase += ycentPeriod;
    }
}

void TuningTable::fillETContinuous( uint32_t intervalCount, uint32_t rateNom, uint32_t rateDenom, uint8_t step )
{
    TuningGeneratorET etg( intervalCount, rateNom, rateDenom );
    uint16_t index = 0;
    double ycentBase = 0;
    const double ycentPeriod = etg.getYcentPeriod();
    while( index < size ) {
        uint16_t note = 0;
        while(( index < size ) && ( note < etg.intervalCount )) {
            const int32_t ycent = std::round( etg.getYcentN( note ) + ycentBase );
            for( auto micro = 0u; micro < modifierCount; ++micro ) {
                relativeYcent[ index++ ] = ycent;
            }
            note += step;
        }
        ycentBase += ycentPeriod; // cleanup
    }
}

// only white keys - black may be duplicated or half?

void TuningTable::fillETContinuous1_7( uint32_t intervalCount, uint32_t rateNom, uint32_t rateDenom )
{
    const int restOctave = 7 - intervalCount;
    if( restOctave < 0 ) {
        return;
    }
    uint8_t whiteKeys[] = { 2,2,1,2,2,2,1 }; // C,D,E,F,G,A,H --> start: MIDI 0 == C
    TuningGeneratorET etg( intervalCount, rateNom, rateDenom );
    const double ycentPeriod = etg.getYcentPeriod();

    uint16_t index = 0;
    double ycentBase = 0;
    int32_t ycent = 0;
    while( index < size ) {
        uint16_t note = 0;
        while(( index < size ) && ( note < etg.intervalCount )) {
            ycent = std::round( etg.getYcentN( note ) + ycentBase );
            for( auto micro = 0u; micro < modifierCount; ++micro ) {
                relativeYcent[ index + micro ] = ycent;
            }
            index += whiteKeys[ note++ ] << modifierCountExp;
        }
        for( auto i=0; i < restOctave; ++i ) {
            for( auto micro = 0u; micro < modifierCount; ++micro ) {
                relativeYcent[ index + micro ] = ycent;
            }            
            index += whiteKeys[ note++ ] << modifierCountExp;
        }
        ycentBase += ycentPeriod; // cleanup
    }
}

void TuningTable::fillETContinuous8_11( uint32_t intervalCount, uint32_t rateNom, uint32_t rateDenom )
{
    const int restOctave = 12 - intervalCount;
    if( restOctave < 0 ) {
        return;
    }
    TuningGeneratorET etg( intervalCount, rateNom, rateDenom );
    const double ycentPeriod = etg.getYcentPeriod();

    uint16_t index = 0;
    double ycentBase = 0;
    int32_t ycent = 0;
    while( index < size ) {
        uint16_t note = 0;
        while(( index < size ) && ( note < etg.intervalCount )) {
            ycent = std::round( etg.getYcentN( note ) + ycentBase );
            for( auto micro = 0u; micro < modifierCount; ++micro ) {
                relativeYcent[ index + micro ] = ycent;
            }
            index += 1 << modifierCountExp;
#if 0
        std::cout
            << "index "     << index
            << " note "     << note
            << " ycent "    << ycent
            << std::endl;
#endif
        
        }
        for( auto i=0; i < restOctave; ++i ) {
            for( auto micro = 0u; micro < modifierCount; ++micro ) {
                relativeYcent[ index + micro ] = ycent;
            }            
            index += 1 << modifierCountExp;
        }
        ycentBase += ycentPeriod; // cleanup
    }
}


// apply baseTransposition
//  1 fill
//  baseTransposition = ycentMidi0_48000_ET12 - note69
//  add baseTransposition to each elements
//
bool TuningTable::fill( TuningType ttype, TuningVariation tv )
{
//    double ycentBase = 0;
//    uint16_t index = 0;
    clear();
    switch( ttype ) {
    case TuningType::TM_21_ET_12:
        fillETContinuous( 12, 2, 1, 1 );
        break;

    case TuningType::TM_21_ET_72:
        fillETContinuous( 72, 2, 1, 6 );
        break;

    case TuningType::TM_21_ET_5:
        fillETContinuous1_7( 5, 2, 1 );
        break;        
        
    case TuningType::TM_21_ET_7:
        fillETContinuous1_7( 7, 2, 1 );
        break;        

    case TuningType::TM_21_ET_9:
        fillETContinuous( 9, 2, 1, 1 ); // need a new filler for less than 12
       // fillETContinuous8_11( 9, 2, 1); to test
        break;        
        
    case TuningType::TM_31_ET_13:
        fillETContinuous( 13, 3, 1, 1 );
        break;
        
    case TuningType::TM_32_ET_9:
        fillETContinuous( 9, 3, 2, 1 );
        break;

    case TuningType::TM_21_Ptolemy:
        fill( Tuning::table_TM_21_Ptolemy );
        break;

    case TuningType::TM_21_Pythagorean:
        fill( Tuning::table_TM_21_Pythagorean );
        break;

    case TuningType::TM_21_Young2:
        fill( Tuning::table_TM_21_Young2 );
        break;

    case TuningType::TM_21_Werckmeister3:
        fill( Tuning::table_TM_21_Werckmeister3 );
        break;

    default:
        return false;
    }
    tuningType = ttype;
    setBaseTransposition_MIDI69_A440();
    return true;
}

bool TuningTable::fill( int32_t * src, uint8_t layer )
{
    auto l = layer & modifierCountMask; // checked before calling
    for( auto i = 0u; i < noteCount; ++i ) {
        relativeYcent[ ( i << modifierCountExp ) + l ] = *src++;
    }
    tuningType = TuningType::TM_M_CUSTOM;
    return true;
}

} // end namespace yacynth 

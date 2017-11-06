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
#include    "protocol.h"
#include    "Tags.h"
#include    <iostream>
#include    <fstream>
#include    <iomanip>

//  http://xenharmonic.wikispaces.com/
//  http://www.microtonal-synthesis.com/scales.html
//  http://www.wendycarlos.com/resources.html#tables


//  http://www.tonalsoft.com/monzo/partch/scale/partch43-lattice.aspx
//  http://www.tonalsoft.com/monzo/blackjack/blackjack.aspx
//  http://self.gutenberg.org/articles/bohlen%E2%80%93pierce_scale
//  http://www.huygens-fokker.org/docs/modename.html
//  https://en.wikipedia.org/wiki/Five-limit_tuning
//  https://www.dynamictonality.com/about.htm
//  https://groups.yahoo.com/neo/groups/tuning/info
//  https://www.revolvy.com/main/index.php?s=12%20equal%20temperament&item_type=topic
//  http://www.precisionstrobe.com/apps/pianotemp/temper.html
//  http://lumma.org/tuning/erlich/erlich-decatonic.pdf

// Bohlen-Pierce Scale
// Pierce 3579b scale,  3 root of 13
// https://en.wikipedia.org/wiki/Bohlen%E2%80%93Pierce_scale
// https://www.youtube.com/watch?v=z0Xpn4G9_R4

// need def for arrays of "JI" types
constexpr double Tuning::TuningGenerator<TuningType::TM_21_JI_PTOLEMY_12>::notes[];
constexpr double Tuning::TuningGenerator<TuningType::TM_21_JI_PARTCH_43>::notes[];

namespace yacynth {
using namespace TagTunerLevel_01;

bool TuningTable::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    TAG_DEBUG(message.getTag(tagIndex), tagIndex, paramIndex, "TuningTable" );
    const uint8_t tag = message.getTag(tagIndex);
    switch( TagTuner( tag ) ) {
    case TagTuner::Clear:
        TAG_DEBUG(TagTuner::Clear, tagIndex, paramIndex, "TuningTable" );
        message.setStatusSetOk();
        return true;

    case TagTuner::SetInternalTuning: {
            TAG_DEBUG(TagTuner::SetInternalTuning, tagIndex, paramIndex, "TuningTable" );
//            const uint16_t controller = message.params[paramIndex];
            message.setStatusSetOk();
        }
        return true;

    case TagTuner::SetCustomTuningET: {
            TAG_DEBUG(TagTuner::SetCustomTuningET, tagIndex, paramIndex, "TuningTable" );
//            const uint16_t controller = message.params[paramIndex];
            message.setStatusSetOk();
        }
        return true;

    case TagTuner::SetCustomTuningJI: {
            TAG_DEBUG(TagTuner::SetCustomTuningJI, tagIndex, paramIndex, "TuningTable" );
//            const uint16_t controller = message.params[paramIndex];
            message.setStatusSetOk();
        }
        return true;

    default:
        break;
    }
    message.setStatus( yaxp::MessageT::illegalTag );
    return false; // error
}


void TuningTable::fillETContinuous( uint32_t intervalCount, uint32_t rateNom, uint32_t rateDenom, uint8_t step )
{
    TuningGeneratorET etg( intervalCount, rateNom, rateDenom );
    uint16_t index = 0;
    double ycentBase = 0;
    while( index < size ) {
        uint16_t note = 0;
        while(( index < size ) && ( note < etg.intervalCount )) {
            const int32_t ycent = std::round( etg.getYcentN( note ) + ycentBase );
            for( auto micro = 0u; micro < modifierCount; ++micro ) {
                relativeYcent[ index++ ] = ycent;
            }
            note += step;
        }
        ycentBase += etg.periodYcent;
    }
}

// only white keys - black may be duplicated or half?

void TuningTable::fillETWhitesContinuous( uint32_t intervalCount, uint32_t rateNom, uint32_t rateDenom )
{
    uint8_t whiteKeys[] = { 0,2,4,5,7,9,11 }; // C,D,E,F,G,A,H --> start: MIDI 0 == C
    TuningGeneratorET etg( intervalCount, rateNom, rateDenom );

    uint16_t index = 0;
    uint16_t keyInd = 0;
    double ycentBase = 0;
    while( index < size ) {
        uint16_t note = 0;
        while(( index < size ) && ( note < etg.intervalCount )) {
            const int32_t ycent = std::round( etg.getYcentN( note++ ) + ycentBase );
            for( auto micro = 0u; micro < modifierCount; ++micro ) {
                relativeYcent[ index++ ] = ycent;
            }
            index += whiteKeys[ keyInd++ ];
            if( keyInd >= sizeof(whiteKeys) ) {
                keyInd = 0;
            }
        }
        ycentBase += etg.periodYcent;
    }
}

// apply baseTransposition
//  1 fill
//  baseTransposition = ycentMidi0_48000_ET12 - note69
//  add baseTransposition to each elements
//
bool TuningTable::fill( TuningType ttype, TuningVariation tv )
{
    double ycentBase = 0;
    uint16_t index = 0;
    switch( ttype ) {
    case TuningType::TM_21_ET_12:
        fillETContinuous( 12, 2, 1, 1 );
        break;

    case TuningType::TM_21_ET_72:
        fillETContinuous( 72, 2, 1, 6 );
        break;

    case TuningType::TM_31_ET_13:
        fillETContinuous( 13, 3, 1, 1 );
        break;

    case TuningType::TM_21_JI_PTOLEMY_12: {
            for( auto octave = 0u; octave < 11; ++octave ) {
                for( auto note = 0u; note < 12; ++note ) {
                    double ycentRel = TuningGenerator<TuningType::TM_21_JI_PTOLEMY_12>::getYcentN( note );
                    int32_t ycent = std::round( ycentRel + ycentBase );
                    for( auto micro = 0u; micro < modifierCount; ++micro ) {
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
        }
        break;

    default:
        return false;
    }
    tuningType = ttype;
    setBaseTransposition();
    return true;
}

void TuningTable::setBaseTransposition()
{
    const int32_t offsetTransposition = ycentA400_48000 - relativeYcent[ 69 * modifierCount ];
    for( auto i=0u; i < size; ++i ) {
        relativeYcent[ i ] += offsetTransposition;
    }
}


bool TuningManager::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    TAG_DEBUG(message.getTag(tagIndex), tagIndex, paramIndex, "TuningTableArray" );

    const uint8_t tag = message.getTag(tagIndex);
    switch( TagTuner( tag ) ) {
    case TagTuner::Clear:
        TAG_DEBUG(TagTuner::Clear, tagIndex, paramIndex, "TuningTableArray" );
//        clear();
        message.setStatusSetOk();
        return true;

    case TagTuner::SetCustomTuningET: {
            TAG_DEBUG(TagTuner::SetCustomTuningET, tagIndex, paramIndex, "TuningTableArray" );
//            const uint16_t controller = message.params[paramIndex];
            message.setStatusSetOk();
        }
        return true;

    case TagTuner::SetCustomTuningJI: {
            TAG_DEBUG(TagTuner::SetCustomTuningJI, tagIndex, paramIndex, "TuningTableArray" );
//            const uint16_t controller = message.params[paramIndex];
            message.setStatusSetOk();
        }
        return true;

    default:
        break;
    }
    message.setStatus( yaxp::MessageT::illegalTag );
    return false; // error
}

TuningManager::TuningManager()
{
    tuningTables.fill( TuningType::TM_21_ET_72, TuningVariation::TV_LINEAR );
//    tuningTables.fill( TuningType::TM_31_ET_13, TuningVariation::TV_LINEAR );
//    tuningTables.fill( TuningType::TM_21_ET_72, TuningVariation::TV_LINEAR );
//    tuningTables[1].fill( TuningType::TM_21_JI_PTOLEMY_12, TuningVariation::TV_LINEAR );
}


} // end namespace yacynth



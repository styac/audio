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

#define REF_TM_TABLE(p) \
    extern TuningGenerator table_ ## p;

REF_TM_TABLE(TM_21_Werckmeister3);
REF_TM_TABLE(TM_21_Young2);
REF_TM_TABLE(TM_21_Pythagorean);
REF_TM_TABLE(TM_21_Ptolemy);
REF_TM_TABLE(TM_21_Partch43);
REF_TM_TABLE(TM_21_Pelog1);

};

namespace yacynth {

// TODO implement layer
void TuningTable::fill( const TuningGenerator& etg, uint8_t layer )
{
    uint16_t index = 0;
    double ycentBase = 1; // will be renormalized
    int32_t ycent = 0;
    const double ycentPeriod = etg.getYcentPeriod();
    const auto intervalCount = etg.getIntervalCount();
    if( etg.isOctave() ) {
        // fill "white" leys 1 layer - extend the rest
        if( intervalCount < 8 ) {
            const int restOctave = 7 - intervalCount;
            const uint8_t whiteKeys[] = { 2,2,1,2,2,2,1 }; // C,D,E,F,G,A,H --> start: MIDI 0 == C
            while( index < size ) {
                uint16_t note = 0;
                while(( index < size ) && ( note < intervalCount )) {
                    ycent = std::round( etg.getYcentN( note ) + ycentBase );
                    for( auto micro = 0u; micro < modifierCount; ++micro ) {
                        relativeYcent[ index + micro ] = ycent;
                    }
                    index += (whiteKeys[ note++ ] << modifierCountExp);
                }
                for( auto i=0; i < restOctave; ++i ) {
                    for( auto micro = 0u; micro < modifierCount; ++micro ) {
                        relativeYcent[ index + micro ] = ycent;
                    }
                    index += (whiteKeys[ note++ ] << modifierCountExp);
                }
                ycentBase += ycentPeriod;
            }
            return;
        }

        // fill all leys 1 layer - extend the rest
        if( intervalCount < 12 ) {
            const int restOctave = 12 - intervalCount;
            const auto indexInc = (1 << modifierCountExp);
            while( index < size ) {
                uint16_t note = 0;
                while(( index < size ) && ( note < intervalCount )) {
                    ycent = std::round( etg.getYcentN( note ) + ycentBase );
                    for( auto micro = 0u; micro < modifierCount; ++micro ) {
                        relativeYcent[ index + micro ] = ycent;
                    }
                    index += indexInc;
                    ++note;
                }
                for( auto i=0; i < restOctave; ++i ) {
                    for( auto micro = 0u; micro < modifierCount; ++micro ) {
                        relativeYcent[ index + micro ] = ycent;
                    }
                    index += indexInc;
                }
                ycentBase += ycentPeriod;
            }
            return;
        }
    }

    // fill all keys 1layer
    while( index < size ) {
        uint16_t note = 0;
        while(( index < size ) && ( note < etg.getIntervalCount() )) {
            ycent = std::round( etg.getYcentN( note ) + ycentBase );
            for( auto micro = 0u; micro < modifierCount; ++micro ) {
                relativeYcent[ index++ ] = ycent;
            }
            note++;
        }
        ycentBase += ycentPeriod;
    }
}
#if 0
void TuningTable::fillETContinuous( uint32_t intervalCount, uint32_t rateNom, uint32_t rateDenom, uint32_t step )
{
    TuningGenerator etg( intervalCount, rateNom, rateDenom );
    fill( etg );
#if 0
    uint16_t index = 0;
    double ycentBase = 0;
    const double ycentPeriod = etg.getYcentPeriod();
    while( index < size ) {
        uint16_t note = 0;
        while(( index < size ) && ( note < etg.getIntervalCount() )) {
            const int32_t ycent = std::round( etg.getYcentN( note ) + ycentBase );
            for( auto micro = 0u; micro < modifierCount; ++micro ) {
                relativeYcent[ index++ ] = ycent;
            }
            note += step;
        }
        ycentBase += ycentPeriod; // cleanup
    }
#endif
}

// only white keys - black may be duplicated or half?

void TuningTable::fillETContinuous1_7( uint32_t intervalCount, uint32_t rateNom, uint32_t rateDenom )
{
    const int restOctave = 7 - intervalCount;
    if( restOctave < 0 ) {
        return;
    }
    uint8_t whiteKeys[] = { 2,2,1,2,2,2,1 }; // C,D,E,F,G,A,H --> start: MIDI 0 == C
    TuningGenerator etg( intervalCount, rateNom, rateDenom );
    const double ycentPeriod = etg.getYcentPeriod();

    uint16_t index = 0;
    double ycentBase = 0;
    int32_t ycent = 0;
    while( index < size ) {
        uint16_t note = 0;
        while(( index < size ) && ( note < etg.getIntervalCount() )) {
            ycent = std::round( etg.getYcentN( note ) + ycentBase );
            for( auto micro = 0u; micro < modifierCount; ++micro ) {
                relativeYcent[ index + micro ] = ycent;
            }
#if 0
        std::cout
            << "index "     << index
            << " note "     << note
            << " ycent "    << ycent
            << std::endl;
#endif
            index += (whiteKeys[ note++ ] << modifierCountExp);
        }
        for( auto i=0; i < restOctave; ++i ) {
            for( auto micro = 0u; micro < modifierCount; ++micro ) {
                relativeYcent[ index + micro ] = ycent;
            }
#if 0
        std::cout
            << "index "     << index
            << " note "     << note
            << " ycent "    << ycent
            << std::endl;
#endif
            index += (whiteKeys[ note++ ] << modifierCountExp);
        }
        ycentBase += ycentPeriod;
    }
}

void TuningTable::fillETContinuous8_11( uint32_t intervalCount, uint32_t rateNom, uint32_t rateDenom )
{
    const int restOctave = 12 - intervalCount;
    if( restOctave < 0 ) {
        return;
    }
    TuningGenerator etg( intervalCount, rateNom, rateDenom );
    const double ycentPeriod = etg.getYcentPeriod();
    const auto indexInc = (1 << modifierCountExp);

    uint16_t index = 0;
    double ycentBase = 0;
    int32_t ycent = 0;
    while( index < size ) {
        uint16_t note = 0;
        while(( index < size ) && ( note < etg.getIntervalCount() )) {
            ycent = std::round( etg.getYcentN( note ) + ycentBase );
            for( auto micro = 0u; micro < modifierCount; ++micro ) {
                relativeYcent[ index + micro ] = ycent;
            }
            index += indexInc;
            ++note;
        }
        for( auto i=0; i < restOctave; ++i ) {
            for( auto micro = 0u; micro < modifierCount; ++micro ) {
                relativeYcent[ index + micro ] = ycent;
            }
            index += indexInc;
        }
        ycentBase += ycentPeriod;
    }
}
#endif
void TuningTable::fillET( uint32_t intervalCount, uint32_t rateNom, uint32_t rateDenom, uint8_t layer )
{
    TuningGenerator etg( intervalCount, rateNom, rateDenom );
    fill( etg, layer );
}

// apply baseTransposition
//  1 fill
//  baseTransposition = ycentMidi0_48000_ET12 - note69
//  add baseTransposition to each elements
//
bool TuningTable::fill( TuningType ttype, TuningVariation tv )
{
    clear();
    switch( ttype ) {
    case TuningType::TM_21_ET_12:
        fillET( 12, 2, 1 );
        break;

//    case TuningType::TM_21_ET_72:
//        fillETContinuous( 72, 2, 1, 6 );
//        break;

    case TuningType::TM_21_ET_5:
        fillET( 5, 2, 1 );
        break;

    case TuningType::TM_21_ET_7:
        fillET( 7, 2, 1 );
        break;

    case TuningType::TM_21_ET_9:
        fillET( 9, 2, 1 );
        break;

    case TuningType::TM_21_ET_10:
        fillET( 10, 2, 1 );
        break;

    case TuningType::TM_31_ET_13:
        fillET( 13, 3, 1 );
        break;

    case TuningType::TM_32_ET_9:
        fillET( 9, 3, 2 );
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

    case TuningType::TM_21_Pelog1:
        fill( Tuning::table_TM_21_Pelog1 );
        break;

    // test case
    case TuningType::TM_test:
//        fill( Tuning::table_TM_21_test_ET12 );
//        break;

    default:
        return false;
    }
    tuningType = ttype;
    setBaseTransposition_MIDI69_A440();
    return true;
}

void TuningTable::fill( int32_t * src, uint8_t layer )
{
    auto l = layer & modifierCountMask; // checked before calling
    for( auto i = 0u; i < noteCount; ++i ) {
        relativeYcent[ ( i << modifierCountExp ) + l ] = *src++;
    }
}

} // end namespace yacynth

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
#include "yacynth_config.h"

#include "TuningTables.h"
#include "protocol.h"
#include "Tags.h"
#include <iostream>
#include <fstream>
#include <iomanip>

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

// http://www.aawmjournal.com/articles/2015b/Garzoli_AAWM_Vol_4_2.pdf

// http://tundra.cnx.rice.edu:8888/contents/6c32f21d-65ac-4a40-8205-11226fb6a327@16/indian-classical-music-tuning-and-ragas
// http://tundra.cnx.rice.edu:8888/contents/37a22b6b-d06d-480a-9c69-eb9daf025e38@27
// http://www.kylegann.com/histune.html

// http://www.huygens-fokker.org/docs/modename.html
// http://www.piano-tuners.org/edfoote/well_tempered_piano.html
// http://www.flutopedia.com/csc_7tone_alt.htm
// http://www.nonoctave.com/tuning/scales/Ptolemy's_Scale_Catalog.html
// http://www.flutopedia.com/

// http://sethares.engr.wisc.edu/consemi.html

// http://www.oddmusic.com/
// http://barthopkin.com/
//
//


namespace yacynth {

using namespace TagTunerLevel_01;

bool TuningTable::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    TAG_DEBUG(message.getTag(tagIndex), tagIndex, paramIndex, "TuningTable" );
    const uint8_t tag = message.getTag(tagIndex);
    switch( TagTuner( tag ) ) {
    case TagTuner::Clear:
        TAG_DEBUG(TagTuner::Clear, tagIndex, paramIndex, "TuningTable" );
        clear();
        message.setStatusSetOk();
        return true;

    case TagTuner::SetInternalTuning: {
            TAG_DEBUG(TagTuner::SetInternalTuning, tagIndex, paramIndex, "TuningTable" );
            const Tuning::TuningType tuningType = Tuning::TuningType( message.params[ 0 ] );
            const Tuning::TuningVariation tuningVariation = Tuning::TuningVariation( message.params[ 1 ] );
            if( fill( tuningType, tuningVariation ) ) {
                message.setStatusSetOk();
                return true;                
            }
        }
        message.setStatus( yaxp::MessageT::illegalParam );
        return false;

    case TagTuner::SetAbsolute: {
            TAG_DEBUG(TagTuner::SetAbsolute, tagIndex, paramIndex, "TuningTable" );
            const uint16_t layer = message.params[ 0 ];
            if( layer >= modifierCount ) {
                message.setStatus( yaxp::MessageT::illegalParam );
                return false;
            }
            if( message.length != ( sizeof(int32_t) * noteCount ) ) {
                message.setStatus( yaxp::MessageT::illegalDataLength );
                return false;
            }
            // no clear
            fill( (int32_t * )message.data, layer );
            tuningType = TuningType::TM_M_CUSTOM;
            // no transposition
        }
        message.setStatusSetOk();
        return true;                

    case TagTuner::SetTuningET: {
            TAG_DEBUG(TagTuner::SetTuningET, tagIndex, paramIndex, "TuningTable" );
            const uint16_t intervalCount = message.params[ 0 ];
            const uint16_t nom = message.params[ 1 ];
            const uint16_t demom = message.params[ 2 ];
            if( intervalCount == 0 || nom == 0 || demom == 0 ) {
                message.setStatus( yaxp::MessageT::illegalParam );
                return false;                                
            }
            clear();
            fillET( intervalCount, nom, demom );
            tuningType = TuningType::TM_M_ET_CUSTOM;
            setBaseTransposition_MIDI69_A440();                
        }
        message.setStatusSetOk();
        return true;                

    case TagTuner::SetTuningJI: {
            TAG_DEBUG(TagTuner::SetTuningJI, tagIndex, paramIndex, "TuningTable" );
        }
        message.setStatus( yaxp::MessageT::illegalParam );
        return false;

    default:
        break;
    }
    message.setStatus( yaxp::MessageT::illegalTag );
    return false; // error
}

void TuningTable::setBaseTransposition_MIDI69_A440()
{
    setBaseTransposition( ycentA400_48000 - relativeYcent[ 69 * modifierCount ] );
}

void TuningTable::setBaseTransposition( int32_t ycent )
{
    for( auto i=0u; i < size; ++i ) {
        if( relativeYcent[ i ] > 0 ) {
            relativeYcent[ i ] += ycent;            
        }
    }
}

void TuningTable::clear()
{
    memset( &relativeYcent, 0, sizeof(relativeYcent) );
    transientTransposition = 0;
    tuningType = TuningType::TM_NIL;    
}

TuningTable::TuningTable()
:   transientTransposition(0)
,   tuningType(TuningType::TM_NIL)
,   tuningVariation(TuningVariation::TV_LINEAR_1layer)
{
    fill( TuningType::TM_21_ET_12, TuningVariation::TV_LINEAR_1layer );
//    fill( TuningType::TM_31_ET_13, TuningVariation::TV_LINEAR_1layer );
//    fill( TuningType::TM_32_ET_9, TuningVariation::TV_LINEAR_1layer );
//    fill( TuningType::TM_test, TuningVariation::TV_LINEAR_1layer );
    
//    tuningTable.fill( TuningType::TM_21_ET_72, TuningVariation::TV_LINEAR );

}


} // end namespace yacynth



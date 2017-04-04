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
 * File:   AbstractRouter.cpp
 * Author: Istvan Simon
 *
 * Created on February 27, 2016, 9:00 PM
 */

#include    "AbstractRouter.h"
#include    "../oscillator/Tables.h"
#include    <iomanip>

//  http://www.microtonal-synthesis.com/scale_partch.html
//  http://www.tonalsoft.com/monzo/partch/scale/partch43-lattice.aspx
//  http://self.gutenberg.org/articles/bohlen%E2%80%93pierce_scale
//  http://www.huygens-fokker.org/docs/modename.html
//  http://xenharmonic.wikispaces.com/Regular+Temperaments
//  https://en.wikipedia.org/wiki/Five-limit_tuning
//  http://xenharmonic.wikispaces.com/Regular+Temperaments

namespace yacynth {
using namespace TagRouterLevel_01;
// --------------------------------------------------------------------

AbstractRouter::AbstractRouter()
:   transposition(0)
,   monoPhone(false)
{
    out.store = 0;
    fillEqualTempered(0);
    setCustomTuning(PARTCH43);
} // end AbstractRouter::AbstractRouter

// --------------------------------------------------------------------


// --------------------------------------------------------------------
uint32_t AbstractRouter::getPitch( int32_t noteNr, uint8_t tableNr )
{
    const int32_t currNote  =  noteNr + transposition;
    const int32_t noteInd   =  currNote < 0 ? 0 : ( currNote >= tuningTableSize ? tuningTableSize-1 : currNote );
    return pitch[ tableNr & ( tuningTableCount - 1)][ noteInd ];
} // end AbstractRouter::getPitch

uint32_t AbstractRouter::getPitch( int32_t noteNr, int8_t mod, uint8_t tableNr )
{
    const int8_t    modLim      = mod < -microResolution ? -microResolution : ( mod > microResolution ? microResolution : mod );
    const int32_t   currNote    =  noteNr + modLim + transposition;
    const int32_t   noteInd     =  currNote < 0 ? 0 : ( currNote >= tuningTableSize ? tuningTableSize-1 : currNote );
    return pitch[ tableNr & ( tuningTableCount - 1)][ noteInd ];
} // end AbstractRouter::getPitch

// --------------------------------------------------------------------

void AbstractRouter::fillEqualTempered( uint8_t tableNr )
{
    for( auto i = 0; i < tuningTableSize; ++i ) {
        pitch[ tableNr ][ i ]  = round( pitch0MidiEqual + equalTemperedNote * i );
#if 0
            std::cout
                << std::dec
                << "i " << i
                << " nt " << (i/microResolution)
                << " oct " << (i/maxTransposition)
                << std::hex
                << " pitch " << pitch[ tableNr ][ i ]
                << std::endl;
#endif
    }

#if 0

    std::array<double, tuningTableSize>  noteRe;
    for( auto i=0; i < tuningTableSize; ++i ) {
        noteRe[i] = ycent2deltafi( pitch[ tableNr ][ i ] , 0 );
    }
    double maxerr = 0;
    long double rate = pow( 2.0L, 1.0/72.0 );
    long double rrt = rate;
    for( auto i=1; i < tuningTableSize; ++i ) {
        const double rr = noteRe[i]/noteRe[0];
        const double err = 1 - (rr/rrt);
        if( std::abs(err) > maxerr)  maxerr = std::abs(err);
            std::cout
                << std::dec << std::setprecision(8)
                << "i " << i
                << " maxerr " << maxerr
                << " err " << err
                << " rate re " << rr
                << " rate orig " << rrt

                << std::endl;
        rrt *= rate;
    }


#endif
} // end AbstractRouter::EqualTemperedNoteRates(void)
// --------------------------------------------------------------------


void AbstractRouter::fillPartch43( uint8_t tableNr )
{
    std::array<double, partch43Size>  note;
// ===========
    note[42] =  160.0 / 81.0;
    note[41] =   64.0 / 33.0;
    note[40] =   40.0 / 21.0;
// ===========
    note[39] =   15.0 /  8.0;
    note[38] =   11.0 /  6.0;
    note[37] =   20.0 / 11.0;
    note[36] =    9.0 /  5.0;
    note[35] =   16.0 /  9.0;
    note[34] =    7.0 /  4.0;   // H
    note[33] =   12.0 /  7.0;
    note[32] =   27.0 / 16.0;
    note[31] =    5.0 /  3.0;   // A
    note[30] =   18.0 / 11.0;
// ===========
    note[29] =    8.0 /  5.0;
    note[28] =   11.0 /  7.0;
    note[27] =   14.0 /  9.0;
    note[26] =   32.0 / 21.0;
    note[25] =    3.0 /  2.0;   // G
    note[24] =   40.0 / 27.0;
    note[23] =   16.0 / 11.0;
    note[22] =   10.0 /  7.0;
    note[21] =    7.0 /  5.0;
    note[20] =   11.0 /  8.0;
// ===========
    note[19] =   27.0 / 20.0;
    note[18] =    4.0 /  3.0;   // F
    note[17] =   21.0 / 16.0;
    note[16] =    9.0 /  7.0;
    note[15] =   14.0 / 11.0;
    note[14] =    5.0 /  4.0;   // E
    note[13] =   11.0 /  9.0;
    note[12] =    6.0 /  5.0;
    note[11] =   32.0 / 27.0;
    note[10] =    7.0 /  6.0;
// ===========
    note[9] =     8.0 /  7.0;
    note[8] =     9.0 /  8.0;   // D
    note[7] =    10.0 /  9.0;
    note[6] =    11.0 / 10.0;
    note[5] =    12.0 / 11.0;
    note[4] =    16.0 / 15.0;   // C#
    note[3] =    21.0 / 20.0;
    note[2] =    33.0 / 32.0;
    note[1] =    81.0 / 80.0;
    note[0] =     1.0;          // C
// ===========
    for( auto i = 0; i < tuningTableSize; ++i ) {
        pitch[ tableNr ][ i ]  = 0;
    }
    for( auto octave = 0; octave < octaveCount; ++octave ) {
        const int       baseind = octave * noteperOctave * microResolution;     // the same octave index as for equal
        for( auto nt = 0; nt < note.size(); ++nt ) {
            const double pt = pitch0MidiPartch43 + ( std::log2( note[ nt ] ) + octave ) * octaveResolution;
            pitch[ tableNr ][ baseind + nt ] = std::lround( pt );
#if 0
            std::cout
                << std::dec
                << "octave " << octave
                << " nt " << nt
                << std::hex
                << " pitch " << pitch[ tableNr ][ baseind + nt  ]
                << std::dec << std::setprecision(8)
                << " pt " << ( std::log2( note[ nt ] ) + octave )
                << " rate " << note[ nt ] * (1<<octave)
                << std::endl;
#endif
        }
    }

#if 0

    std::array<double, partch43Size>  noteRe;
    for( auto i=0; i < partch43Size; ++i ) {
        noteRe[i] = ycent2deltafi( pitch[ tableNr ][ i ] , 0 );
    }
    double maxerr = 0;
    for( auto i=1; i < partch43Size; ++i ) {
        const double rr = noteRe[i]/noteRe[0];
        const double err = 1 - (rr/note[i]);
        if( std::abs(err) > maxerr)  maxerr = std::abs(err);
            std::cout
                << std::dec << std::setprecision(8)
                << "i " << i
                << " maxerr " << maxerr
                << " err " << err
                << " rate re " << rr
                << " rate orig " << note[i]

                << std::endl;
    }


#endif

} // end AbstractRouter::EqualTemperedNoteRates(void)

// --------------------------------------------------------------------

void AbstractRouter::setTransposition( int8_t val )
{
  transposition = ( val > maxTransposition ? maxTransposition : ( val < -maxTransposition ? -maxTransposition : val ) );
}
// --------------------------------------------------------------------

void AbstractRouter::setCustomTuning( tuned_t custune )
{
    switch( custune ) {
    case PARTCH43:
        fillPartch43(1);
        break;
    default:
        return;
    }
    tuned = custune;
} // end AbstractRouter::setCustomTune

bool AbstractRouter::parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    const uint8_t tag = message.getTag(tagIndex);
    switch( TagRouter( tag ) ) {
    case TagRouter::Clear :
        return true;
    }
            
    message.setStatus( Yaxp::MessageT::illegalTag, tag );
    return false;    
}

} // end namespace yacynth

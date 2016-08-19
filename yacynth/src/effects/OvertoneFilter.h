#pragma once

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
 * File:   OvertoneFilter.h
 * Author: Istvan Simon
 *
 * Created on March 17, 2016, 10:02 PM
 */
#include    "yacynth_globals.h"
#include    "Ebuffer.h"
#include    "../control/Controllers.h"
#include    "../utils/GaloisNoiser.h"

#include    <array>
#include    <iomanip>
#include    <algorithm>

namespace yacynth {


class OvertoneFilter {
public:

    // this must be completely reworked

    // 4..8 range - formant
    //  for(range)
    //      for ( i = beg; ... )
    //          out = in * level
    //

    struct Range {
        uint16_t    beg;    // relative or absolute to each other ???
        uint16_t    level;  // how to interpolate if changes ???
    };

    static constexpr    int16_t unitV = 0x7FFF;
    OvertoneFilter()
    {
        setUnit();
    };

    using   OvertoneCoeffs = std::array<int16_t, overtoneCountOscDef>;

    int64_t filter(int64_t sample, uint16_t index )
    {
        return ( sample * coeffs[index] ) >> 15;
    };

    void    setCoeffs( const OvertoneCoeffs& oc )
    {
        coeffs = oc;
    };

    void    setUnit( void )
    {
        for( auto i = 0u; i < overtoneCountOscDef; ++i ) {
            coeffs[i] = unitV;
        }
    };

    // group handling !!! group of overtones equal

    void    setLowHighpass( uint16_t pos1, uint16_t pos2, uint16_t group )
    {
        groups = group; // not developed yet
        if( pos1 < pos2 ) { // lowpass
            if( pos2 >= overtoneCountOscDef )
                pos2 = overtoneCountOscDef;
            pos1s = pos1;
            pos2s = pos2;
            for( auto i = 1u; i < pos1; ++i ) {
                coeffs[i] = unitV;
            }
            const uint16_t slope = unitV / ( pos2 - pos1 );
            for( auto i = pos1; i < pos2; ++i ) {
                coeffs[i] = (pos2 - i) * slope;
            }
            for( auto i = pos2; i < overtoneCountOscDef; ++i ) {
                coeffs[i] = 0;
            }

        } else if( pos1 > pos2 ) { // highpass
            if( pos1 >= overtoneCountOscDef )
                pos1 = overtoneCountOscDef;
            pos1s = pos1;
            pos2s = pos2;
            for( auto i = 1u; i < pos2; ++i ) {
                coeffs[i] = 0;
            }
            const uint16_t slope = unitV / ( pos1 - pos2 );
            for( auto i = pos2; i < pos1; ++i ) {
                coeffs[i] = ( i - pos2 ) * slope;
            }
            for( auto i = pos1; i < overtoneCountOscDef; ++i ) {
                coeffs[i] = unitV;
            }

        } else { // lowpass
            if( pos1 >= overtoneCountOscDef )
                pos1 = overtoneCountOscDef;
            pos2s = pos1s = pos1;
            for( auto i = 1u; i < pos1; ++i ) {
                coeffs[i] = unitV;
            }
            for( auto i = pos1; i < overtoneCountOscDef; ++i ) {
                coeffs[i] = 0;
            }
        }
    };

    // sweep up - down
    void sweep( int16_t pos )
    {
        constexpr int16_t zero(0);
        pos1s += pos;
        pos2s += pos;
        setLowHighpass( std::min(zero,pos1s), std::min(zero,pos2s), groups );
    };

private:
    Range           range[8];
    int16_t         pos1s;
    int16_t         pos2s;
    uint16_t        groups;
    OvertoneCoeffs  coeffs;

};
// --------------------------------------------------------------------

} // end namespace yacynth

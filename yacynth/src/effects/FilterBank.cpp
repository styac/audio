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
 * File:   FIlterBank.cpp
 * Author: Istvan Simon
 *
 * Created on March 15, 2016, 3:06 PM
 */
#include    "FilterBank.h"
#include    "Ebuffer.h"

namespace yacynth {
// --------------------------------------------------------------------
void FilterBank::process( const EIObuffer& in, EIObuffer& out )
{
    for( uint32_t samplei = 0; samplei < sectionSize; ++samplei ) {

        out.channelA[ samplei ] = biquad.get(  in.channelA[ samplei ] );
#if 0
        if( in.channelA[ samplei ] > 0.000001f )
            std::cout
                << "samplei " << samplei
                << " in " << in.channelA[ samplei ]
                << " out " << out.channelA[ samplei ]
                << " z1 " << biquad.delayA.z1
                << " z2 " << biquad.delayA.z2
                << std::endl;
#endif
    }
}
// --------------------------------------------------------------------
} // end namespace yacynth
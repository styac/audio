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
 * File:   ToneShaperMatrix.cpp
 * Author: Istvan Simon
 *
 * Created on April 4, 2016, 11:52 AM
 */

#include    "../oscillator/ToneShaperMatrix.h"

namespace yacynth {

// --------------------------------------------------------------------

bool ToneShaperMatrix::fill( std::stringstream& ser )
{
    // for testing
    bool ret = deserialize ( ser, toneShapers[0] );
    return ret;
}

// --------------------------------------------------------------------

void ToneShaperMatrix::query( std::stringstream& ser )
{
    ;
}

// --------------------------------------------------------------------

void ToneShaperMatrix::dump( std::stringstream& ser )
{
    // for testing
    serialize ( ser, toneShapers[0] );
}
// --------------------------------------------------------------------

void ToneShaperMatrix::clear(void)
{
    ;
}

// --------------------------------------------------------------------


} // end namespace yacynth
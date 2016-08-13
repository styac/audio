/*
 * Copyright (C) 2016 Istvan Simon -- stevens37 at gmail dot com
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
 * File:   FxFilter.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on June 21, 2016, 9:57 PM
 */

#include "FxFilter.h"

namespace yacynth {

    // 00 is always clear for output or bypass for in-out
void FxFilter::sprocess_00( void * thp )
{
//        static_cast< MyType * >(thp)->clear();
}
void FxFilter::sprocess_01( void * thp )
{
    static_cast< MyType * >(thp)->test_allpass2();
}
void FxFilter::sprocess_02( void * thp )
{
    static_cast< MyType * >(thp)->test_allpass2();
}


} // end namespace yacynth



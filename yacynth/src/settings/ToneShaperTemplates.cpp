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
 * File:   ToneShaperTemplates.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on February 4, 2016, 9:19 AM
 */

#include            "../oscillator/ToneShaper.h"

namespace yacynth {

ToneShaper::ToneShaper()
{
//    count = envelopeKnotCount;
// these are for testing -- same for each overtone

    for( auto i = 0; i < transientKnotCount; i++ ) {
        transient[ i ].tickFrame[0]    = 0;
        transient[ i ].curveSpeed   = 0;
        transient[ i ].targetValue[0]  = 0;
    }
    transient[ envelopeKnotRelease ].tickFrame[0]    = 100;
    transient[ envelopeKnotRelease ].curveSpeed   = 2;
    transient[ envelopeKnotRelease ].targetValue[0]  = 0;      // ignored

    transient[ 2 ].tickFrame[0] = 10;
    transient[ 2 ].curveSpeed   = -1;
    transient[ 2 ].targetValue[0]  = 65534 * (1L<<16); // NEW uin32

    transient[ 1 ].tickFrame[0] = 300;
    transient[ 1 ].curveSpeed   = 1;
    transient[ 1 ].targetValue[0]  = 20000 * (1L<<16); // NEW uin32

    sustain.decayCoeff          = 1000;
    sustain.sustainModDepth     = 0;      // 100 / 256
//    sustain.sustainModDepth     = 170;      // 100 / 256
    sustain.sustainModPeriod    = 300;
    sustain.sustainModType      = 1;

    // this should be diff for each ovwertone
    // but for testing we use the same values - so suspend the activation
    // sustainModMagic[] = {}; -


}
// --------------------------------------------------------------------






//



} // end namespace yacynth



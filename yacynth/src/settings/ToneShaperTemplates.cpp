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
        transient[ i ].clear();
    }
#if 0       
    // NEW
    // standard test
    tickFrameRelease.set(1000);
    curveSpeedRelease   = 2;
    
    transient[ 2 ].tickFrame.set(10);
    transient[ 2 ].curveSpeed       = 2;
    transient[ 2 ].targetValue.set(65534 * (1L<<16)); // NEW uin32

    transient[ 1 ].tickFrame.set(300);
    transient[ 1 ].curveSpeed       = 1;
    transient[ 1 ].targetValue.set(20000 * (1L<<16)); // NEW uin32

    sustain.decayCoeff.set( 1000 );
    sustain.sustainModDepth     = 0;      // 100 / 256
//    sustain.sustainModDepth     = 170;      // 100 / 256
    sustain.sustainModPeriod    = 300;
    sustain.sustainModType      = 1;
#else
    // allpass test
    tickFrameRelease.lowBase =100;
    curveSpeedRelease   = 2;
    
    transient[ 2 ].tickFrame.lowBase = 2;
    transient[ 2 ].curveSpeed       = 2;
    transient[ 2 ].targetValue.lowBase = 65534 ; // NEW uin32

    transient[ 1 ].tickFrame.lowBase = 30;
    transient[ 1 ].curveSpeed       = 1;
    transient[ 1 ].targetValue.lowBase = 60000 ; // NEW uin32

    sustain.decayCoeff.lowBase = 0;
    sustain.sustainModDepth     = 0;      // 100 / 256
//    sustain.sustainModDepth     = 170;      // 100 / 256
    sustain.sustainModPeriod    = 300;
    sustain.sustainModType      = 1;
    
#endif
    // this should be diff for each ovwertone
    // but for testing we use the same values - so suspend the activation
    // sustainModMagic[] = {}; -


}
// --------------------------------------------------------------------






//



} // end namespace yacynth



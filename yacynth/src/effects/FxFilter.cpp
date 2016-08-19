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

FxFilterParam::FxFilterParam()
{
    const uint16_t masterLfo = 0;
    const uint16_t slaveLfo = 0;
    // test lfo 0, 0
    InnerController& inco = InnerController::getInstance();
    testMasterLfo1.index = inco.getIndexMasterLfoPhase(masterLfo);
    testSlaveLfo1.index = inco.getIndexSlaveMasterLfoPhase(masterLfo,slaveLfo);

   // set test values for delta phase
    testMasterLfoDeltaPhase1.index = inco.getIndexMasterLfoDeltaPhase(masterLfo);
    testSlaveLfoDeltaPhase1.index = inco.getIndexSlaveMasterLfoDeltaPhase(masterLfo,slaveLfo);

    uint32_t masterDDP = freq2deltaPhase(0.2f) << 6; // 1 Hz - 1/64 sampling freq
    uint32_t slaveDDP = 0x80000000 ;

    testMasterLfoDeltaPhase1.set( masterDDP );
    testSlaveLfoDeltaPhase1.set( slaveDDP );
    
    const int32_t freq_01 = freq2ycent(200.0f);
    testMasterLfo1.setYcent8Parameter(freq_01 , 600);
    testSlaveLfo1.setYcent8Parameter(freq_01 , 600);
  

}
    // 00 is always clear for output or bypass for in-out
void FxFilter::sprocess_00( void * thp )
{
    static_cast< MyType * >(thp)->clear();
}
void FxFilter::sprocess_01( void * thp )
{
    static_cast< MyType * >(thp)->test_allpass2();
}
void FxFilter::sprocess_02( void * thp )
{
    static_cast< MyType * >(thp)->test_lfoController();
}


} // end namespace yacynth



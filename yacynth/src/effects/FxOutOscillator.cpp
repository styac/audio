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
 * File:   FxOutOscillator.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on June 23, 2016, 7:46 PM
 */

#include    "FxOutOscillator.h"
#include    "yacynth_globals.h"

namespace yacynth {

const char slavename[]  = " slave";

FxOutOscillatorParam::FxOutOscillatorParam()
{
    //phaseDiff = 1<<30;
    //phaseDelta = freq2deltaPhase(1);
    phaseDiff.index = InnerController::CC_MODULATOR_PHASEDIFF0;
    phaseDelta0.index = InnerController::CC_MODULATOR_FREQ0;
    phaseDelta1.index = InnerController::CC_MODULATOR_FREQ1;
    phaseDiff.setShift(0);
    const int32_t freq_01 = freq2ycent(0.1f);
    phaseDelta0.setYcent8Parameter(freq_01 , 0x7FFF);
}

bool FxOutOscillator::connect( const FxBase * v, uint16_t ind ) 
{
    doConnect(v,ind);
}; 


void FxOutOscillator::sprocessTransient( void * thp )
{
    auto& th = *static_cast< MyType * >(thp);
    switch( th.fadePhase ) {
    // 1 phase
    case FadePhase::FPH_fadeNo:
        th.sprocessp = th.sprocesspSave =  th.sprocessv[ th.procMode ];
        th.sprocesspSave(thp);
        return;

    // clear then switch to nop
    case FadePhase::FPH_fadeOutClear:
        th.clear();
        th.procMode = 0;
        th.sprocessp = th.sprocesspSave = sprocessNop;
        return;

    case FadePhase::FPH_fadeOutSimple:
        th.sprocesspSave(thp);
        th.fadeOut();   // then clear -- then nop
        th.sprocessp = th.sprocesspSave =  th.sprocessv[ th.procMode ];
        return;

    // 1 phase
    case FadePhase::FPH_fadeInSimple:
        th.sprocessp = th.sprocesspSave =  th.sprocessv[ th.procMode ];
        th.sprocesspSave(thp);
        th.fadeIn();
        return;

    // 1 of 2 phase
    case FadePhase::FPH_fadeOutCross:
        th.sprocesspSave(thp);
        th.fadeOut();
        th.sprocesspSave =  th.sprocessv[ th.procMode ];
        th.fadePhase = FadePhase::FPH_fadeInCross;
        return;

    // 2 of 2 phase
    case FadePhase::FPH_fadeInCross: // the same as FPH_fadeInSimple ???
        th.sprocessp = th.sprocesspSave =  th.sprocessv[ th.procMode ];
        th.sprocesspSave(thp);
        th.fadeIn();
        return;
    }

}

// 00 is always clear for output or bypass for in-out
void FxOutOscillator::sprocess_00( void * thp )
{
//        static_cast< MyType * >(thp)->clear();
}
void FxOutOscillator::sprocess_01( void * thp )
{
    static_cast< MyType * >(thp)->updateParam();
    static_cast< MyType * >(thp)->processSine();
}
void FxOutOscillator::sprocess_02( void * thp )
{
    static_cast< MyType * >(thp)->updateParam();
    static_cast< MyType * >(thp)->processSinePd0();
}
void FxOutOscillator::sprocess_03( void * thp )
{
    static_cast< MyType * >(thp)->updateParam();
    static_cast< MyType * >(thp)->processSinePd1();
}

void FxOutOscillator::sprocess_04( void * thp )
{
    static_cast< MyType * >(thp)->updateParam();
    static_cast< MyType * >(thp)->processSinePd2();
}

void FxOutOscillator::sprocess_05( void * thp )
{
    static_cast< MyType * >(thp)->updateParam();
    static_cast< MyType * >(thp)->processSinePd3();
}


} // end namespace yacynth



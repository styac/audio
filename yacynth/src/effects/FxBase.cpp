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
 * File:   EffectBase.h
 * Author: Istvan Simon
 *
 * Created on April 6, 2016, 11:22 PM
 */
#include "yacynth_config.h"
#include "FxBase.h"
#include "yaio/CycleCount.h"

#include <chrono>
#include <sys/time.h>
#include <ctime>
#include <time.h>

// TODO split the file to 
//  FxBase.cpp
//  FxBaseRunner.cpp
//  FxBaseCollector.cpp

namespace yacynth {
using namespace TagEffectRunnerLevel_01;
using namespace TagEffectCollectorLevel_01;
using namespace TagEffectCollectorLevel_01;

// --------------------------------------------------------------------

FxBase      fxNil("Nil",0,0,TagEffectType::FxNil);
uint16_t    FxBase::count = -1;
// template <typename T>  uint16_t S<T>::something_relevant = 0;

// --------------------------------------------------------------------

FxNode::FxNode()
:   thp(&fxNil)
{};

// --------------------------------------------------------------------
FxBase::~FxBase()
{
};

// --------------------------------------------------------------------

void FxBase::sprocessNop( void * data )
{
    return;
}
// --------------------------------------------------------------------
void FxBase::sprocessClear2Nop( void * data )
{
    FxBase& thp = * static_cast<FxBase *> ( data );
    thp.sprocessp = sprocessNop;
    thp.EIObuffer::clear();
}
// --------------------------------------------------------------------
void FxBase::sprocessFadeOut( void * data )
{
    FxBase& thp = * static_cast<FxBase *> ( data );
    thp.sprocessp = sprocessClear2Nop;
    thp.sprocesspCurr( data );
    thp.EIObuffer::fadeOut();
}
// --------------------------------------------------------------------
void FxBase::sprocessFadeIn( void * data )
{
    FxBase& thp = * static_cast<FxBase *> ( data ) ;
    thp.clearState(); // clears the internal state but not the output - new mode starts
    SpfT sprocessX( thp.sprocesspNext );
    thp.sprocessp = sprocessX;
    thp.sprocesspCurr = sprocessX;
    sprocessX( data );
    thp.EIObuffer::fadeIn();
}
// --------------------------------------------------------------------
// not real cross fade at the moment but fade out - fade in
void FxBase::sprocessCrossFade( void * data )
{
    FxBase& thp = * static_cast<FxBase *> ( data );
    thp.sprocessp = sprocessFadeIn;
    thp.sprocesspCurr( data );
    thp.EIObuffer::fadeOut();
}
// --------------------------------------------------------------------
// X -> 0:
//  k:      run old function    :  sprocessX2FadeOut
//          fade out
//  k+1:    clear               :  sprocessClear2Nop
//  k+2:    nop
//          wait 4 cycle

// 0 -> X
//  k:      run new function    :  sprocessFadeIn2X
//          fade in
//  k+1:    new function
//          wait 3 cycle

// X -> Y
//  k:      run old function    :  sprocessX2FadeOut
//          fade out
//  k+1     run new function    :  sprocessFadeIn2X
//          fade in
//  k+2:    new function
//          wait 4 cycle

bool FxBase::setProcessingMode( uint16_t mode )
{
    constexpr int waitCycle = 10;
    if( procMode == mode ) {
        return true; // no change
    }

    // check cycleCount if low then wait 10msec
    // set cycleCount
    auto currCycle = CycleCount::getInstance().get();
    if( currCycle < nextSetPocModeCycle ) {
        timespec req{0,1000LL*1000*10};
        timespec rem;
        // sleep(10sec)
        int r = nanosleep( &req, &rem );
        if( r != 0 ) {
            // TODO temp
            std::cout << " *** nanosleep error: " << errno << std::endl;
        }
    }
    nextSetPocModeCycle = CycleCount::getInstance().get() + waitCycle;
    return setSprocessNext( mode );
}

// --------------------------------------------------------------------

bool FxBase::connect( const FxBase * v, uint16_t ind )
{
     std::cout
         << "FxBase::connect - should never be called - override "
         << std::endl;
    return false;
}

// --------------------------------------------------------------------
// clear transient data - NOT settings

void FxBase::clearState()
{
    // EIObuffer::clear();
}

// --------------------------------------------------------------------

bool FxBase::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    TAG_DEBUG( TagEffectCollector::Nop, tagIndex, paramIndex,"FxBase/FxNil" );
    message.setStatus( yaxp::MessageT::noParameter ); // nothing to do here
    return false;
}

// --------------------------------------------------------------------

bool FxBase::setSprocessNext( uint16_t mode )
{
    sprocessp = sprocesspNext = FxBase::sprocessClear2Nop;
    return true;
}; 

// --------------------------------------------------------------------
} // end namespace yacynth
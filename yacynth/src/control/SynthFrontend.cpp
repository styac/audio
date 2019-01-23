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
 * File:   SynthFrontend.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on February 17, 2016, 7:45 AM
 */

#include "yacynth_config.h"
#include "control/SynthFrontend.h"
#include "control/global.h"
#include "control/Nsleep.h"
#include "logDefs.h"

#include <unistd.h>
#include <atomic>
#include <thread>

namespace yacynth {

namespace {
constexpr auto LogCategoryMask              = LOGCAT_synthfe;
constexpr auto LogCategoryMaskAlways        = LogCategoryMask | nanolog::LogControl::log_always;
constexpr const char * const LogCategory    = "SYFE";
}

SynthFrontend::SynthFrontend (
        ControlQueueVector&      queueinP,
        OscillatorOutVector&    outVectorP,
        OscillatorArray       * oscArrayP
    )
:   queuein( queueinP )
,   outVector(outVectorP )
,   oscArray( oscArrayP )
,   runFe(true)
{
} // end SynthFrontend::SynthFrontend

// --------------------------------------------------------------------
bool SynthFrontend::initialize( void )
{
    statistics.clear();
    return oscArray->initialize(  );

} // end SynthFrontend::initialize
// --------------------------------------------------------------------

bool SynthFrontend::evalMEssage( void )
{
    Yamsgrt msg;
    while( (msg.store = queuein.queueOscillator.get()) ) {
        std::cout << std::hex << " msg:" << msg.store << " cpu:" << sched_getcpu()   << std::endl;
        if( int64_t(msg.store) > 0 ) {
            oscArray->voiceRun( msg.voiceSet.oscNr, msg.voiceSet.pitch, msg.voiceSet.velocity, msg.voiceSet.toneBank );
            if( 0 == ++cycleNoise ) { // reset after 2^32 cycles
                GaloisShifterSingle<seedThreadOscillator_noise>::getInstance().reset();
                GaloisShifterSingle<seedThreadOscillator_random>::getInstance().reset();
            }
            continue;
        }
        switch( msg.voiceChange.opcode ) {
        case YAMOP_VOICE_RELEASE :
            oscArray->voiceRelease( msg.voiceRelease.oscNr );
            continue;
        case YAMOP_VOICE_CHANGE :
            continue;
        default:
            if( int64_t(msg.store) == -1LL ) {
                return false;
            }
        }
    }
    return true;
} // end SynthFrontend::evalMEssage

// --------------------------------------------------------------------

// main oscillator loop
bool SynthFrontend::run( void )
{
    statistics.startTimer();
    statistics.stopTimer();
    LOG_TRACE_CAT(LogCategoryMask,LogCategory) << "SynthFrontend::run";

#ifdef IO_DEBUG    
    NanosecTimer timer;
#endif
    
    while( runFe ) {
        if( ! evalMEssage() ) {
            return false;
        }
        
        if( ! outVector.isFull() ) {
            statistics.startTimer();        
            
#ifdef IO_DEBUG    
            timer.begin();    
#endif
            const int wi = outVector.getWriteIndex();
            oscArray->generate( outVector.out[ wi ], statistics );
            outVector.writeFinished();        
            statistics.stopTimer();
            
#ifdef IO_DEBUG    
            constexpr int countExp = 12;
            timer.end();
            nanosecCollector.addEnd(timer);
            if( ! nanosecCollector.checkCount(1<<countExp) ) {
                LOG_DEBUG_CAT(LogCategoryMask,LogCategory)
                    << ( nanosecCollector.getClear() >> countExp ) 
                    << " nanosec; cpu:" << sched_getcpu()
                    << " over " << statistics.countOverSumm
                    << " max " << statistics.cycleDeltaMax
                    ;            
                statistics.countDisplay  = 0;
                statistics.cycleDeltaSumm = 0;
            }
#endif
        } else {
            nsleep(1000); // wait a bit nothing to do          
        }
    }
    return true;
} // end SynthFrontend::run
// --------------------------------------------------------------------

void SynthFrontend::exec( void * data )
{
    SynthFrontend * thp = static_cast<SynthFrontend *> (data);
    LOG_TRACE_CAT(LogCategoryMask,LogCategory) << "SynthFrontend::exec";
    thp->run();

} // end  SynthFrontend::exec
// --------------------------------------------------------------------

} // end namespace yacynth

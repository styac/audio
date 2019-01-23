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
 * File:   IOthread.cpp
 * Author: Istvan Simon
 *
 * Created on February 15, 2016, 4:59 PM
 */

#include "yacynth_config.h"
#include "yaio/IOthread.h"
#include "yaio/CycleCount.h"
#include "logDefs.h"

#include <sys/time.h>
#include <ctime>
#include <atomic>

namespace yacynth {

namespace {
constexpr auto LogCategoryMask              = LOGCAT_iothread;
constexpr auto LogCategoryMaskAlways        = LogCategoryMask | nanolog::LogControl::log_always;
constexpr const char * const LogCategory    = "IOTH";
}

IOThread:: IOThread( OscillatorOutVector& out )
: queueOut(out)
, fxEndMixer()
, fxOscillatorMixer()
, fxInput()
, fxRunner( fxEndMixer, fxOscillatorMixer, fxInput )
, bufferSizeRate(1)
, toClearFxInput(true)
, cycleNoise(0)
, coreNr(-1) // not set
{
    fxEndMixer.setProcessingMode(1); // TODO > endMixed mode -- muted function
}

// --------------------------------------------------------------------

void IOThread::audioInOutCB( void *data, uint32_t nframes, float *outp1, float *outp2, float *inp1, float *inp2 )
{
    IOThread& thp = *static_cast<IOThread *>(data);
    thp.setCore();    
    
#ifdef IO_DEBUG    
    NanosecTimer timer;
#endif
    
    for( auto fi=0u; fi < thp.bufferSizeRate; ++fi ) {
        CycleCount::getInstance().inc( 1 );
        thp.fxInput.process( inp1, inp2 );
        inp1 += thp.fxEndMixer.sectionSize;
        inp2 += thp.fxEndMixer.sectionSize;
        InnerController::getInstance().incrementFrameLFOscillatorPhases();

        while( thp.queueOut.isEmpty() ) 
            ; // often empty if not much to do in the effect chain
                
#ifdef IO_DEBUG    
        timer.begin();    
#endif
        
        const int ri = thp.queueOut.getReadIndex();
        thp.fxOscillatorMixer.process( thp.queueOut.out[ri] );
        
        thp.queueOut.readFinished();
        
        thp.fxRunner.run();
        thp.fxEndMixer.exec();                  // must be the last
        thp.fxEndMixer.dump( outp1, outp2 );    // get the result
        outp1 += thp.fxEndMixer.sectionSize;
        outp2 += thp.fxEndMixer.sectionSize;
        
#ifdef IO_DEBUG    
        constexpr int countExp = 12;
        timer.end();
        thp.nanosecCollector.addEnd(timer);
        if( ! thp.nanosecCollector.checkCount(1<<countExp) ) {
            LOG_DEBUG_CAT(LogCategoryMask,LogCategory)
                << ( thp.nanosecCollector.getClear() >> countExp ) 
                << " nanosec; cpu:" << sched_getcpu() 
                ;            
        }
#endif
    }

    if( 0 == ++thp.cycleNoise ) { // reset after 2^32 cycles
        GaloisShifterSingle<seedThreadEffect_noise>::getInstance().reset();
        GaloisShifterSingle<seedThreadEffect_random>::getInstance().reset();
    }
}

// --------------------------------------------------------------------

void IOThread::audioOutCB( void *data, uint32_t nframes, float *outp1, float *outp2 )
{
    IOThread& thp = *static_cast<IOThread *>(data);
    thp.setCore();
    
#ifdef IO_DEBUG    
    NanosecTimer timer;
#endif
    
    // if muted was changed from unmuted -- create var
    if( thp.toClearFxInput ) {
        thp.fxInput.clear();
        thp.toClearFxInput = false;
    }
    
    for( auto fi=0u; fi < thp.bufferSizeRate; ++fi ) {
        while( thp.queueOut.isEmpty() )
            ;

#ifdef IO_DEBUG    
        timer.begin();    
#endif
        
        InnerController::getInstance().incrementFrameLFOscillatorPhases();
        const int ri = thp.queueOut.getReadIndex();
        thp.fxOscillatorMixer.process( thp.queueOut.out[ri] );
        thp.queueOut.readFinished();
        thp.fxRunner.run();
        thp.fxEndMixer.exec();                  // must be the last
        thp.fxEndMixer.dump( outp1, outp2 );    // get the result
        outp1 += thp.fxEndMixer.sectionSize;
        outp2 += thp.fxEndMixer.sectionSize;

#ifdef IO_DEBUG    
        constexpr int countExp = 12;
        timer.end();
        thp.nanosecCollector.addEnd(timer);
        if( ! thp.nanosecCollector.checkCount(1<<countExp) ) {
            LOG_DEBUG_CAT(LogCategoryMask,LogCategory) 
                << ( thp.nanosecCollector.getClear() >> countExp ) 
                << " nanosec; cpu:" << sched_getcpu() 
                ;            
        }
#endif
    }

    if( 0 == ++thp.cycleNoise ) { // reset after 2^32 cycles
        GaloisShifterSingle<seedThreadEffect_noise>::getInstance().reset();
        GaloisShifterSingle<seedThreadEffect_random>::getInstance().reset();
    }
} // end IOThread::audioOutCB

// --------------------------------------------------------------------

} // end namespace yacynth

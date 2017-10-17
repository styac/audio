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

#include    "../yaio/IOthread.h"
#include    "../yaio/CycleCount.h"
#include    <sys/time.h>
#include    <ctime>
#include    <atomic>

namespace yacynth {

IOThread:: IOThread( OscillatorOutVector& out )
:   queueOut(out)
,   cycleNoise(0)
,   fxEndMixer()
,   fxOscillatorMixer()
,   fxInput()
,   fxRunner(fxEndMixer)
,   toClearFxInput(true)
{
    fxEndMixer.setProcessingMode(1); // TODO > endMixed mode -- muted function
};

// --------------------------------------------------------------------

void IOThread::audioInOutCB( void *data, uint32_t nframes, float *outp1, float *outp2, float *inp1, float *inp2 )
{
    // ------------------- profiling
    timeval tv;
    IOThread& thp = *static_cast<IOThread *>(data);
    // ------------------- profiling

    // TODO> relax this condition and make sync mode to reduce latency
    if( thp.queueOut.getFullCount() < thp.bufferSizeRate ) {
        // wait for a short time with nanosleep
        // synth thread doesn't run
        for( auto i=0u; i < nframes; ++i ) {
            outp1[i] = 0.0f;
            outp2[i] = 0.0f;
        }
        return;
    }

    // ------------------- profiling
    const int64_t  maxC = 1000;
    if( --thp.count < 0 ) {
        std::cout << std::dec << " ---new in-out  iothread " << ( thp.timer / thp.bufferSizeRate ) << " cpu:"<< sched_getcpu() << std::endl;
        thp.count = maxC;
        thp.timer = 0;
    }
    gettimeofday(&tv,NULL);
    const uint64_t begint   = tv.tv_sec*1000000ULL + tv.tv_usec;
    // ------------------- profiling

    for( auto fi=0u; fi < thp.bufferSizeRate; ++fi ) {
        CycleCount::getInstance().inc( 1 );
        thp.fxInput.process( inp1, inp2 );
        inp1 += thp.fxEndMixer.sectionSize;
        inp2 += thp.fxEndMixer.sectionSize;
        InnerController::getInstance().incrementFrameLFOscillatorPhases();
//        if( thp.queueOut.getFullCount() < 1 ) {
//          wait 1 microsec
//        }
        const int ri = thp.queueOut.getReadIndex();
        thp.fxOscillatorMixer.process( thp.queueOut.out[ri] );
        thp.queueOut.readOk();
        thp.fxRunner.run();
        thp.fxEndMixer.exec();                  // must be the last
        thp.fxEndMixer.dump( outp1, outp2 );    // get the result
        outp1 += thp.fxEndMixer.sectionSize;
        outp2 += thp.fxEndMixer.sectionSize;
    }

    if( 0 == ++thp.cycleNoise ) { // reset after 2^32 cycles
        GaloisShifterSingle<seedThreadEffect_noise>::getInstance().reset();
        GaloisShifterSingle<seedThreadEffect_random>::getInstance().reset();
    }

    // ------------------- profiling
    gettimeofday(&tv,NULL);
    thp.timer += tv.tv_sec*1000000ULL + tv.tv_usec - begint;
    // ------------------- profiling
}

// --------------------------------------------------------------------

void IOThread::audioOutCB( void *data, uint32_t nframes, float *outp1, float *outp2 )
{
    // ------------------- profiling
    timeval tv;
    IOThread& thp = *static_cast<IOThread *>(data);
    // ------------------- profiling

    // TODO> relax this condition and make sync mode to reduce latency
    if( thp.queueOut.getFullCount() < thp.bufferSizeRate ) {
        // wait for a short time with nanosleep
        // synth thread doesn't run
        for( auto i=0u; i < nframes; ++i ) {
            outp1[i] = 0.0f;
            outp2[i] = 0.0f;
        }
        return;
    }

    // ------------------- profiling
    const int64_t  maxC = 1000;
    if( --thp.count < 0 ) {
        std::cout << std::dec << " ---new out iothread " << ( thp.timer / thp.bufferSizeRate ) << " cpu:"<< sched_getcpu() << std::endl;
        thp.count = maxC;
        thp.timer = 0;
    }
    gettimeofday(&tv,NULL);
    const uint64_t begint   = tv.tv_sec*1000000ULL + tv.tv_usec;
    // ------------------- profiling
    // if muted was changed from unmuted -- create var
    if( thp.toClearFxInput ) {
        thp.fxInput.clear();
        thp.toClearFxInput = false;
    }

    for( auto fi=0u; fi < thp.bufferSizeRate; ++fi ) {
        InnerController::getInstance().incrementFrameLFOscillatorPhases();
//        if( thp.queueOut.getFullCount() < 1 ) {
//          wait 1 microsec
//        }
        const int ri = thp.queueOut.getReadIndex();
        thp.fxOscillatorMixer.process( thp.queueOut.out[ri] );
        thp.queueOut.readOk();
        thp.fxRunner.run();
        thp.fxEndMixer.exec();                  // must be the last
        thp.fxEndMixer.dump( outp1, outp2 );    // get the result
        outp1 += thp.fxEndMixer.sectionSize;
        outp2 += thp.fxEndMixer.sectionSize;
    }

    if( 0 == ++thp.cycleNoise ) { // reset after 2^32 cycles
        GaloisShifterSingle<seedThreadEffect_noise>::getInstance().reset();
        GaloisShifterSingle<seedThreadEffect_random>::getInstance().reset();
    }

    // ------------------- profiling
    gettimeofday(&tv,NULL);
    thp.timer += tv.tv_sec*1000000ULL + tv.tv_usec - begint;
    // ------------------- profiling
} // end IOThread::audioOutCB

// --------------------------------------------------------------------

} // end namespace yacynth

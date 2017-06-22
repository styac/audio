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

#include    <sys/time.h>
#include    <ctime>
#include    <atomic>

namespace yacynth {

IOThread:: IOThread(
    YaIoInQueueVector&      in,
    OscillatorOutVector&    out,
    AbstractRouter&         router )
:   queueIn(in)
,   queueOut(out)
,   midiRouter(router)
,   cycleNoise(0)
,   fxEndMixer()
,   fxOscillatorMixer()
,   fxRunner(fxEndMixer)

{
    fxEndMixer.setProcMode(1); // TODO > endMixed mode
};
// --------------------------------------------------------------------

void IOThread::printEvent( uint8_t *eventp, uint32_t eventSize, bool lastEvent)
{
    std::cout << "ev: " << std::hex;
    for( auto i=0; i< eventSize; ++i, ++eventp ){
        const uint16_t val = *eventp;
        std::cout << " " <<  val ;
    }
    if( lastEvent ) {
        std::cout << " ****";
    }
    std::cout << std::endl;
} // end IOThread::printEvent

// --------------------------------------------------------------------
// TODO what about the routing ??
// is it possible to be called 4x 64 / sample ?

void IOThread::midiInCB( void *data, uint8_t *eventp, uint32_t eventSize, bool lastEvent )
{
    bool res;
    IOThread& thp = *static_cast<IOThread *>(data);

// diag
    thp.printEvent( eventp, eventSize, lastEvent );
// diag

    Yamsgrt     ymsg;
    RouteIn     midi;
    midi.op             = ( *eventp ) >> 4;
    midi.chn            = ( *eventp ) & 0x0F;
    midi.note_cc_val    = ( 1 < eventSize ) ? *(eventp+1) : 0;
    midi.velocity_val   = ( 2 < eventSize ) ? *(eventp+2) : 0;
    
    /*
     switch( eventSize ) {
     case 0:
        return;
     case 1:
        break;
     default:
        break;
     */

    ymsg = thp.midiRouter.translate( midi );
    if( 0 == ymsg.store )
        return;
    res     = thp.queueIn.queueOscillator.put( ymsg.store );
    return;

} // end IOThread::midiInCB

// --------------------------------------------------------------------

void IOThread::audioOutCB( void *data, uint32_t nframes, float *outp1, float *outp2, int16_t bufferSizeMult )
{
    timeval tv; // profiling
    IOThread& thp = *static_cast<IOThread *>(data);

    if( thp.queueOut.getFullCount() < bufferSizeMult ) {
        // synth thread doesn't run
        for( auto i=0u; i < nframes; ++i ) {
            outp1[i] = 0.0f;
            outp2[i] = 0.0f;
        }
        return;
    }

    // looks ok
    // gains are not complete
    const int64_t  maxC = 1000;
    if( --thp.count < 0 ) {
        std::cout << std::dec << " ---new  iothread " << ( thp.timer / bufferSizeMult ) << std::endl;
        thp.count = maxC;
        thp.timer = 0;
    }

    // profiling
    gettimeofday(&tv,NULL);
    const uint64_t begint   = tv.tv_sec*1000000ULL + tv.tv_usec;

    for( auto fi=0u; fi < bufferSizeMult; ++fi ) {
        // main cycle for the effect stage -- called according to the frames rate:
        // dev state: 64 sample internal, 256 external
        // one frame cycle
        InnerController::getInstance().incrementFrameLFOscillatorPhases();
        const int ri = thp.queueOut.getReadIndex();
        thp.fxOscillatorMixer.process( thp.queueOut.out[ri] ); // this has a special interface
        thp.queueOut.readOk();
        thp.fxRunner.run();

        // TODO > in 1 step - mix direct to the output
        // dump should call exec() ?
        // or dump set the pointers and a use special endMixer type

        thp.fxEndMixer.exec();  // must be the last
        thp.fxEndMixer.dump( outp1, outp2 ); // get the result
        outp1 += thp.fxEndMixer.sectionSize;
        outp2 += thp.fxEndMixer.sectionSize;
    }

    if( 0 == ++thp.cycleNoise ) { // reset after 2^32 cycles
        GaloisShifterSingle<seedThreadEffect_noise>::getInstance().reset();
        GaloisShifterSingle<seedThreadEffect_random>::getInstance().reset();
    }
    // profiling
    gettimeofday(&tv,NULL);
    thp.timer += tv.tv_sec*1000000ULL + tv.tv_usec - begint;

} // end IOThread::audioOutCB

// --------------------------------------------------------------------

} // end namespace yacynth

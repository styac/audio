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

#include    "SynthFrontend.h"

#include    <unistd.h>
#include    <atomic>

// for writing
#include    <sndfile.h>


namespace yacynth {

SNDFILE * sndf;
float filebuf[1024];
SF_INFO sf_info;

void wrsndfile( OscillatorOut *out )
{

        auto k = 0;
        for( auto j=0; j < oscillatorOutSampleCount; ++j  ) {
            filebuf[k++]    = float( out->layer[0][j] ) * 1.0e-11;
            filebuf[k++]    = float( out->layer[1][j] ) * 1.0e-11;
        }

        sf_write_float(sndf, filebuf , 2 * oscillatorOutSampleCount );
}

SynthFrontend::SynthFrontend (
        YaIoInQueueVector&      queueinP,
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
    Yamsgrt     msg;
    uint16_t    velocityLin;
    while( msg.store = queuein.queueOscillator.get() ) {
        std::cout << std::hex << "msg: " << msg.store << std::endl;
        if( YAMOP_SYSTEM_STOP == msg.store) {
            return false;
        }

        switch(  msg.f1.opcode ) {
        case YAMOP_SETVOICE_NOTE:
            oscArray->voiceUp(      msg.setVoice.oscNr, msg.setVoice.pitch, msg.setVoice.velocity );
            if( 0 == ++cycleNoise ) { // reset after 2^32 cycles
                GaloisShifterSingle<seedThreadOscillator_noise>::getInstance().reset();
                GaloisShifterSingle<seedThreadOscillator_random>::getInstance().reset();
            }

            break;
        case YAMOP_CHNGVOICE_NOTE:
            oscArray->voiceChange(  msg.setVoice.oscNr, msg.setVoice.pitch, msg.setVoice.velocity );
            break;
        }
    }
    return true;
} // end SynthFrontend::evalMEssage
// --------------------------------------------------------------------
bool SynthFrontend::generate( void )
{
    if( ! outVector.isFull() ) {
        const int wi = outVector.getWriteIndex();
        oscArray->generate( outVector.out[ wi ], statistics );
//        wrsndfile( &outVector->out[ wi ]);
        outVector.writeOk();
#if 0
        std::cout
               << "end SynthFrontend   " << outVector->getFullCount()
               << " wi " << wi
               << " writePtr " << outVector->writePtr
               << std::endl;
#endif
        return true;
    }
    return false;
} // end SynthFrontend::generate
// --------------------------------------------------------------------
bool SynthFrontend::run( void )
{
//    GaloisShifterSingle& gs1         = GaloisShifterSingle::getInstance();
//    GaloisShifterSingle& gs2         = GaloisShifterSingle::getInstance();
//    std::cout
//        << "gs1 " << static_cast< void *>( &(gs1 ) )
 //       << " gs2 " << static_cast< void *>( &(gs2 ) )
 //       << std::endl;

    statistics.startTimer();
    statistics.stopTimer();

#if 0
    sf_info.channels    = 2;
    sf_info.format      = SF_FORMAT_WAV | SF_FORMAT_FLOAT  ;
    sf_info.samplerate  = 48000;
    sf_info.seekable    = 0;
    sf_info.sections    = 0;
    sf_info.frames      = 200000;
    sndf = sf_open( "testsound.wav", SFM_WRITE, &sf_info ) ;
    if( sndf == nullptr ) {
        std::cout << "testsound.wav open error" << std::endl;
        exit(-1);
    }
//    queuein.queueOscillator.put( 0x2002600000049 );

#endif
     std::cout << "SynthFrontend::run " << std::endl;

    while( runFe ) {
        statistics.startTimer();
        if( ! evalMEssage() ) {
            return false;
        }
        const bool genRes  = generate();
        statistics.stopTimer();

        if( 10000 <= statistics.countDisplay ) {
           std::cout << std::dec
               << "--- osc " << statistics.cycleDeltaSumm
               << " over " << statistics.countOverSumm
               << " max " << statistics.cycleDeltaMax
               << std::endl;
           statistics.countDisplay  = 0;
           statistics.cycleDeltaSumm = 0;
        }
        // it should wait if the loop is too fast
        // countCycles[0] + countCycles[1] < limit
        if( ! genRes  ) {
            usleep(100);    // wait a bit nothing to do
        }
// this should be delayed by 1 complete cycle
//        outVector->writeOk();
    }
    return true;
} // end SynthFrontend::run
// --------------------------------------------------------------------

void SynthFrontend::exec( void * data )
{
    SynthFrontend * thp = static_cast<SynthFrontend *> (data);
    std::cout << "SynthFrontend::exec " << std::endl;
    const bool res = thp->run();

} // end  SynthFrontend::exec
// --------------------------------------------------------------------





} // end namespace yacynth



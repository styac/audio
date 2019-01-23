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
 * File:   main.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on January 26, 2016, 11:09 PM
 */
#include "yacynth_config.h"

#include "utils/Limiters.h"
#include "utils/GaloisNoiser.h"
#include "yaio/YaIoJack.h"
#include "yaio/IOthread.h"
#include "net/Server.h"
// #include "router/SimpleMidiRouter.h"
#include "router/Router.h"
#include "control/Controllers.h"
#include "control/Sysman.h"
#include "control/SynthFrontend.h"
#include "yacynth_globals.h"
#include "v4.h"
#include "Setting.h"
#include "ThreadControl.h"

#include "logDefs.h"
#include "Nsleep.h"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <thread>
#include <iomanip>
#include <unistd.h>
#include <pthread.h>
#include <csignal>

using namespace yacynth;
using namespace filter;
using namespace tables;
using namespace noiser;


namespace {
constexpr auto LogCategoryMask              = LOGCAT_main;
constexpr auto LogCategoryMaskAlways        = LogCategoryMask | nanolog::LogControl::log_always;
constexpr const char * const LogCategory    = "MAIN";
}

void preset0( Sysman  * sysman, uint8_t mode );
void createStaticEfects();

// to catch 
// std::exception_ptr globalException = nullptr;

// --------------------------------------------------------------------
//
// this is not the normal stop method
//

static void signal_handler( int sig )
{
    switch( sig ) {
    case SIGTERM:
    case SIGINT:
        YaIoJack::getInstance().shutdown();
        exit(-1);
    }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"

static void teststuff()
{
    std::cout << std::dec
        << "\nOscillatorArray size:     "   << sizeof(OscillatorArray)
        << "\nOscillator size:          "   << sizeof(Oscillator)
        << "\nToneShaperMatrix size:    "   << sizeof(ToneShaperMatrix)
        << "\nToneShaperVector size:    "   << sizeof(ToneShaperVector)
        << "\nInnerController size:     "   << sizeof(InnerController)
        << "\nMidiRangeController size: "   << sizeof(MidiController)
        << "\nToneShaper size:          "   << sizeof(ToneShaper)
        << "\nAmplitudeSustain size:    "   << sizeof(AmplitudeSustain)
        << "\nAmplitudeTransient size:  "   << sizeof(AmplitudeTransient)
        << "\nInterpolatedTick size:    "   << sizeof(InterpolatedTick)
        << "\nInterpolatedDecay size:   "   << sizeof(InterpolatedDecay)
        << "\nyaxp::Header size:        "   << sizeof(yaxp::Header)
        << "\ncontroller CC_END         "   << InnerController::CC_END
        << "\ncontroller arraySize      "   << InnerController::arraySize

        << "\n--------------------"
        << "\nrefA440ycent:             "   << refA440ycent
        << "\nrefA440ycentDouble:       "   << uint64_t(refA440ycentDouble*10)
        << "\nrefA440ycentDouble:       "   << refA440ycentDouble
        << "\n--------------------"
        << std::hex
        << "\n refA440ycent:            "   << refA440ycent
        << "\n refA440ycentDouble:      "   << uint64_t(std::llround(refA440ycentDouble))
        << "\n 19990:                   "   << ref19900ycent
        << "\n 0.01:                    "   << ref0_01ycent
        << "\n ycent1Hz_48000:          "   << int32_t(TuningTable::ycent1Hz_48000)
        << "\n ycentA400_48000:         "   << int32_t(TuningTable::ycentA400_48000)
        << "\n ycentMidi0_48000:        "   << int32_t(TuningTable::ycentMidi0_48000_ET12)
        << "\n ycentMidi127_48000:      "   << int32_t(TuningTable::ycentMidi127_48000_ET12)
        << "\n deltaPhaseA400_48000:    "   << ExpTable::ycent2deltafi(TuningTable::ycentA400_48000)
        << "\n deltaPhaseMidi0_48000:   "   << ExpTable::ycent2deltafi(TuningTable::ycentMidi0_48000_ET12)
        << "\n deltaPhaseMidi127_48000: "   << ExpTable::ycent2deltafi(TuningTable::ycentMidi127_48000_ET12)
        << "\n cent2ycent:              "   << cent2ycent
        << "\n ycent2cent:              "   << ycent2cent
        << "\n midi res:                "   << (100.0/16384.0)
        << "\n\n"
        << std::dec
        << "\n freq2deltaPhase( 1.0 )   "   << freq2deltaPhase( 1.0 )
        << "\n freq2deltaPhase( 0.125 ) "   << freq2deltaPhase( 0.125 )
        << "\n freq2deltaPhase( 8.0 )   "   << freq2deltaPhase( 8.0 )
        << std::endl;
}

// --------------------------------------------------------------------
//  -- MAIN --
// --------------------------------------------------------------------

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

int main( int argc, char** argv )
{
    try {        
        nanolog::initialize(nanolog::NonGuaranteedLogger(10), "/tmp/", "", 1);
        nanolog::set_logFormat(nanolog::LogFormat::LF_NONE);
        nanolog::set_logLevel(nanolog::LogLevel::TRACE);
    } catch (...) {
        std::cerr << "init error: FATALs" << std::endl;
        exit(-1);
    }
    
    try {
        GaloisShifterSingle<seedThreadEffect_noise>::getInstance();
        GaloisShifterSingle<seedThreadOscillator_noise>::getInstance();
        GaloisShifterSingle<seedThreadEffect_random>::getInstance();
        GaloisShifterSingle<seedThreadOscillator_random>::getInstance();

        ExpTable::getInstance();
        FilterTableExp2Pi::getInstance();
        FilterTableSinCosPi2::getInstance();
        FilterTable2SinPi::getInstance();
        FilterTableCos2Pi::getInstance();
        SinTable::table();
        VelocityBoostTable::getInstance();
        YaIoJack::getInstance();
        TuningManager::getInstance();        
    } catch (...) {        
        std::cerr << "init error: FATALs" << std::endl;
        exit(-1);
    }
    
    // init singletons

    ControlQueueVector&     queuein     = ControlQueueVector::getInstance();
    OscillatorOutVector&    oscOutVec   = OscillatorOutVector::getInstance();
    InnerController&        controller  = InnerController::getInstance();
    FxCollector::getInstance();

    struct sigaction sigact;
    memset( &sigact, 0, sizeof(sigact) );
    sigfillset( &sigact.sa_mask );
    sigact.sa_handler = signal_handler;
	sigaction( SIGTERM, &sigact, NULL );
    sigaction( SIGINT, &sigact, NULL );
    // uint16_t   port( yaxp::defaultPort ); // from param

    
    Setting settings;
    if( ! settings.initialize( argc, argv ) ) {
        exit(-1);
    }

    std::cout << "\n\n============ settings =============="
        << "\n homedir  : "  << settings.getHomeDir()
        << "\n confir   : "  << settings.getConfDir()
        << "\n auth     : "  << settings.getAuthKeyFile()
        << std::endl;

    // teststuff();

    // inter thread communication
    // TODO check nullptr
    
    OscillatorArray * oscArray   = new OscillatorArray();
    // router - can be singleton
    Router          * midiRouter = new Router(queuein);

    // threads
    // TODO IOThread: separate audio and midi

    IOThread        * iOThread   = new IOThread( oscOutVec );
    SynthFrontend   * synthFe    = new SynthFrontend( queuein, oscOutVec, oscArray );
    Sysman          * sysman     = new Sysman( *midiRouter, *oscArray, *iOThread ); // + oscOutVec to control

    // auto& fxRunner  = iOThread->getFxRunner();

    createStaticEfects(); // default sound

    try {
        preset0( sysman, 1 );
        //-------------------------
        // start jack thread
        YaIoJack::getInstance().registerAudioProcessor( iOThread, IOThread::audioOutCB, IOThread::audioInOutCB );
        YaIoJack::getInstance().registerMidiProcessor( midiRouter, Router::midiInCB );

        if( ! synthFe->initialize() )
            exit(-1);
        if( ! YaIoJack::getInstance().initialize() )
            exit(-1);

        iOThread->setBufferSizeRate( YaIoJack::getInstance().getBufferSizeRate() );
        //-------------------------
        // start synth fe thread
        std::thread  synthFrontendThread( SynthFrontend::exec, synthFe );
        nsleep(1000*1000*10,0);
                
        if( ! YaIoJack::getInstance().run() )
            exit(-1);
        
        nsleep(1000*1000*10,0);
        
        ThreadControl threadControl;        
        int res = threadControl.getThreadParam( synthFrontendThread );
        if( res != 0 ) {
            std::cout << "\n\n============ synthFrontendThread getThreadParam error " << res << std::endl;            
        } else {            
            threadControl.policy = SCHED_FIFO;
            threadControl.sch.sched_priority = YaIoJack::getInstance().getRealTimePriority();
            res = threadControl.setThreadParam( synthFrontendThread );
            if( res != 0 ) {
                std::cout << "\n\n============ synthFrontendThread setThreadParam error " << res << std::endl;            
            } else {
                std::cout << "\n\n============ synthFrontendThread setThreadParam prio: " << threadControl.sch.sched_priority << std::endl;                                        
            }            
        }
        
        LOG_DEBUG_CAT(LogCategoryMask,LogCategory) << "============LETS GO==============";
        
        std::cout << " main thread id " << std::this_thread::get_id() << std::endl;

        if( ! net::Server::create( *sysman, settings ) ) {
            std::cout << "\n\n============END ERROR> server failed \n\n" << std::endl;
            exit(-1);
        }
        
        YaIoJack::getInstance().unmute(); // in - out running
        nsleep(1000*1000*50,0);
        
        int8_t ioThreadCoreNr = iOThread->getCoreNr();
        if( ioThreadCoreNr < 0 ) {
            nsleep(1000*1000*50,0);
            ioThreadCoreNr = iOThread->getCoreNr();            
        }
        
        std::cout << "\n\n============ ioThreadCoreNr: " << int16_t(ioThreadCoreNr)  << std::endl;                                        
        threadControl.assignDiffCore(synthFrontendThread, ioThreadCoreNr);
        
        net::Server::getInstance()->run(); // will block
        LOG_DEBUG_CAT(LogCategoryMask,LogCategory) << "\n\n============END==============\n\n";
        YaIoJack::getInstance().mute();
        synthFe->stop();
        synthFrontendThread.join();
        YaIoJack::getInstance().shutdown();
    } catch (...) {
        std::cout << "runtime error" << std::endl;
        YaIoJack::getInstance().mute();
        synthFe->stop();
        YaIoJack::getInstance().shutdown();
        return -1;
    }
//    synthFrontendThread.join();
//-----------------------------------------------------------
    return 0;
};

#pragma GCC diagnostic pop

//-----------------------------------------------------------

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

#include    "../utils/Biquad.h"
#include    "../utils/BiquadMath.h"
#include    "../utils/Limiters.h"
#include    "../utils/GaloisNoiser.h"
#include    "../utils/FilterBase.h"
#include    "../oscillator/Tables.h"
#include    "../oscillator/Oscillator.h"
#include    "../oscillator/OscillatorArray.h"
#include    "../oscillator/ToneShaper.h"
#include    "../yaio/YaIoJack.h"
#include    "../yaio/IOthread.h"
#include    "../net/Server.h"
#include    "../router/SimpleMidiRouter.h"
#include    "../effects/Filter.h"
#include    "../effects/DelayTap.h"
#include    "../effects/FxFilter.h"
#include    "../effects/FxOutNoise.h"
#include    "../effects/FxModulator.h"
#include    "../effects/FxOutOscillator.h"

#include    "../control/Controllers.h"
#include    "../control/Sysman.h"
#include    "../control/SynthFrontend.h"
#include    "yacynth_globals.h"
#include    "v4.h"

#include    <cstdlib>
#include    <iostream>
#include    <fstream>
#include    <thread>
#include    <type_traits>
#include    <iomanip>

#include    <unistd.h>
#include    <pthread.h>
#include    <sndfile.h>
#include    <bitset>
#include    <map>
#include    <list>
#include    <csignal>
#include    <sys/time.h>
#include    <chrono>

using namespace yacynth;
using namespace filter;
using namespace tables;
using namespace noiser;

using namespace TagMainLevel_00;
using namespace TagToneShaperLevel_01;

using namespace TagEffectFactoryLevel_01;
using namespace TagEffectRunnerLevel_01;
using namespace TagMidiControllerLevel_01;
using namespace TagInnerControllerLevel_01;
using namespace TagRouterLevel_01;
using namespace TagTunerLevel_01;
using namespace TagEffectCollectorLevel_01;
using namespace TagEffectTypeLevel_02;
using namespace TagEffectFxFilterModeLevel_03;
using namespace TagEffectFxOscillatorMixerModeLevel_03;
using namespace TagEffectFxMixerModeLevel_03;
using namespace TagEffectFxModulatorModeLevel_03;
using namespace TagEffectFxOutNoiseModeLevel_03;
using namespace TagEffectFxOutOscillatorModeLevel_03;
using namespace TagEffectFxEchoModeLevel_03;
using namespace TagEffectFxReverbModeLevel_03;

// put to main()
YaIoJack&    jack = YaIoJack::getInstance();
// --------------------------------------------------------------------
bool initialize(void)
{
    fillExp2Table();  // obsolate
    SinTable::table();
    return true;
}

// --------------------------------------------------------------------

void signal_handler(int sig)
{
    // YaIoJack::getInstance().shutdown();
    jack.shutdown();
    fprintf(stderr, "yacynth -- signal received, exiting\n");
    exit(0);
}

void setupEffects(Sysman  * sysman)
{    
    Yaxp::Message msgBuffer;
    
/* the first 3 element must be in this sequence : 1 instance
---- ind  0 id 0 type 1 maxMode 0 inputCount 0 masterId 0  Nil
---- ind  1 id 1 type 3 maxMode 1 inputCount 4 masterId 0  Mixer4
---- ind  2 id 2 type 4 maxMode 1 inputCount 0 masterId 0  OscillatorMixer
 * optional but first the output: noise, low freq osc : maybe more instance
---- ind  3 id 3 type 6 maxMode 2 inputCount 0 masterId 0  NoiseSource
---- ind  4 id 4 type 7 maxMode 5 inputCount 0 masterId 0  Oscillator
---- ind  5 id 5 type 2 maxMode 0 inputCount 0 masterId 4   ^OscillatorSlave
---- ind  6 id 6 type 2 maxMode 0 inputCount 0 masterId 4   ^OscillatorSlave
---- ind  7 id 7 type 2 maxMode 0 inputCount 0 masterId 4   ^OscillatorSlave
 * some always used : maybe more instance
---- ind  8 id 8 type 5 maxMode 4 inputCount 2 masterId 0  Modulator
---- ind  9 id 9 type 8 maxMode 9 inputCount 1 masterId 0  Filter
 */
    
    // put the standard components into here
    // sequence important !!
    FxOutNoise      * fxnoise = new FxOutNoise();
    FxOutOscillator * fxosc   = new FxOutOscillator();
    FxModulator     * fxmod   = new FxModulator();
    FxFilter        * fxfilt  = new FxFilter();

    // temp
    fxmod->setProcMode(1);
    fxosc->setProcMode(1);
    fxfilt->setProcMode(2);
    fxnoise->setProcMode(1);    

    std::cout << "\n---- Collector get list\n" << std::endl;

    msgBuffer.clear();
    msgBuffer.setLength(50000);    
    msgBuffer.setTags( uint8_t( TagMain::Effect )
            ,uint8_t(TagEffectCollector::GetEffectList )    
            );

    sysman->evalParameterMessage(msgBuffer);
    
    if( msgBuffer.messageType == 0 ) {
        
        EffectListEntry *data = static_cast<EffectListEntry *>((void *)(msgBuffer.data));
        for( uint16_t ind = 0; ind < msgBuffer.length / sizeof(EffectListEntry); ++ind, ++data ) {
            std::cout 
                << "---- ind  "     << uint16_t(data->fxIndex)  
                << " id "           << uint16_t(data->id)                     
                << " type "         << uint16_t(data->fxType) 
                << " maxMode "      << uint16_t(data->fxMaxMode) 
                << " inputCount "   << uint16_t(data->inputCount ) 
                << " masterId "     << uint16_t(data->masterId)                     
                << "  "             << data->name
                << std::endl;            
        }
    } else {
        std::cout << "---- error " <<uint16_t(msgBuffer.messageType) << std::endl;
    }
    
    std::cout << "\n---- Fill runner\n" << std::endl;
    
    EffectRunnerFill effectFill[] = {4,8};    
    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectRunner )
                    ,   uint8_t( TagEffectRunner::Fill )    
                    );    
    msgBuffer.setPar(cArrayElementCount(effectFill));   
    msgBuffer.getTargetData(effectFill);
    sysman->evalParameterMessage(msgBuffer);    
    if( msgBuffer.messageType == 0 ) {        
        std::cout << "---- fill ok" << std::endl;                        
    } else {
        std::cout << "---- error " <<uint16_t(msgBuffer.messageType) << std::endl;
    }       
    
// connect   
    std::cout << "\n---- Connect effects\n" << std::endl;
    
    EffectRunnerSetConnections  effectRunnerSetConnections[] = {
        { 2, 2, 0 },    // audio osc out to modulator 0
        { 4, 2, 1 },    // low freq osc to  modulator 1
        { 8, 0, 0 }     // modulator to output 0
    };
                                                                           
    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectRunner )
                    ,   uint8_t( TagEffectRunner::SetConnections )    
                    );
    msgBuffer.setPar(cArrayElementCount(effectRunnerSetConnections));   
    msgBuffer.getTargetData(effectRunnerSetConnections);
    sysman->evalParameterMessage(msgBuffer);
    
    if( msgBuffer.messageType == 0 ) {        
        std::cout << "---- Connect ok" << std::endl;                        
    } else {
        std::cout << "---- Connect error " <<uint16_t(msgBuffer.messageType) << std::endl;
    }       


    std::cout << "\n---- Runner get list\n" << std::endl;
    
    msgBuffer.clear();
    msgBuffer.setLength(50000);    
    msgBuffer.setTags( uint8_t( TagMain::EffectRunner )
            ,uint8_t(TagEffectRunner::GetEffectList )    
            );
    sysman->evalParameterMessage(msgBuffer);
    
    if( msgBuffer.messageType == 0 ) {
        
        EffectListEntry *data = static_cast<EffectListEntry *>((void *)(msgBuffer.data));
        for( uint16_t ind = 0; ind < msgBuffer.length / sizeof(EffectListEntry); ++ind, ++data ) {
            std::cout 
                << "---- ind  "     << uint16_t(data->fxIndex)  
                << " id "           << uint16_t(data->id)                     
                << " type "         << uint16_t(data->fxType) 
                << " maxMode "      << uint16_t(data->fxMaxMode) 
                << " inputCount "   << uint16_t(data->inputCount ) 
                << " masterId "     << uint16_t(data->masterId)                     
                << "  "             << data->name
                << std::endl;            
        }                        
    } else {
        std::cout << "---- error " <<uint16_t(msgBuffer.messageType) << std::endl;
    }    
 //   std::cout << "\n---- End setup\n" << std::endl;
}


// --------------------------------------------------------------------

void teststuff(void)
{

    std::cout << std::dec
        << "\nOscillatorArray size: "       << sizeof(OscillatorArray)
        << "\nToneShaperMatrix size: "      << sizeof(ToneShaperMatrix)
        << "\nToneShaperVector size: "      << sizeof(ToneShaperVector)
        << "\nToneShaper size: "            << sizeof(ToneShaper)
        << "\nInnerController size: "       << sizeof(InnerController)
        << "\nMidiRangeController size: "   << sizeof(MidiController)
        << "\nYax::Header size: "           << sizeof(Yaxp::Header)
        << "\n--------------------"
        << "\nsize: v4sf "                  << sizeof(v4sf)
        << "\nsize: V4sf_m "                << sizeof(V4sf_m)
        << std::hex
        << "\nsize: v4sf_u "                << sizeof(V4sfMatrix)
        << "\nfreq2deltaPhase( 1.0 ) "      << freq2deltaPhase( 1.0 )
        << "\nrefA440ycent:       "         << refA440ycent
        << "\nrefA440ycentDouble: "         << uint64_t(std::llround(refA440ycentDouble))
        << "\n19990:"                       << ref19900ycent
        << "\n0.01:"                        << ref0_01ycent
        << std::dec
        << "\n refA440ycent: "              << refA440ycent
        << "\n refA440ycentDouble: "        << uint64_t(refA440ycentDouble*10)
        << "\n refA440ycentDouble: "        << refA440ycentDouble
        << "\n Filter4Pole: "               << sizeof(Filter4PoleOld<3>)
        << "\n wchar_t: "                   << sizeof(wchar_t)
        << std::endl;
}

// --------------------------------------------------------------------
//  -- MAIN --
// --------------------------------------------------------------------
int main( int argc, char** argv )
{
    uint16_t            port( 7373 );         // from param
    std::string         host( "127.0.0.1" );    // allowed from
    struct sigaction sigact;
    memset (&sigact, '\0', sizeof(sigact));
    static_assert( 8 == sizeof(Yamsgrt), "sizeof(Yamsgrt) must be 8" );
    static_assert( 0x193b0973 == refA440ycent, "refA440ycent must be 0x193b0973" );
    if( ! initialize() ) {
        exit(-1);
    }
    teststuff();
    //  singletons first
    // random generators
    GaloisShifterSingle<seedThreadEffect_noise>& gs0        = GaloisShifterSingle<seedThreadEffect_noise>::getInstance();
    GaloisShifterSingle<seedThreadOscillator_noise>& gs1    = GaloisShifterSingle<seedThreadOscillator_noise>::getInstance();
    GaloisShifterSingle<seedThreadEffect_random>& gs2       = GaloisShifterSingle<seedThreadEffect_random>::getInstance();
    GaloisShifterSingle<seedThreadOscillator_random>& gs3   = GaloisShifterSingle<seedThreadOscillator_random>::getInstance();

    FilterTableExp2Pi::getInstance();
    FilterTableSinCosPi2::getInstance();
    FilterTable2SinPi::getInstance();
    FilterTableCos2Pi::getInstance();


    // inter thread communication
    YaIoInQueueVector&      queuein     = YaIoInQueueVector::getInstance();
    OscillatorOutVector&    oscOutVec   = OscillatorOutVector::getInstance();
    InnerController&        controller  = InnerController::getInstance();

    LowOscillatorArray::getInstance().reset();
    FxCollector::getInstance();

    OscillatorArray     *oscArray   = new OscillatorArray();
    SimpleMidiRouter    *midiRoute  = new SimpleMidiRouter();

    // threads
    IOThread            *iOThread   = new IOThread(      queuein, oscOutVec, *midiRoute );
    SynthFrontend       *synthFe    = new SynthFrontend( queuein, oscOutVec, oscArray );

    Sysman              *sysman     = new Sysman( *oscArray, *iOThread );
    Server              uiServer( host, port, *sysman );

    std::cout << std::hex
        << "queuein: " << (void *)&queuein
        << " oscOutVec: " << (void *)&oscOutVec
        << " oscArray: " << (void *)oscArray
        << " iOThread: " << (void *)iOThread
        << " synthFe: " << (void *)synthFe
        << std::endl;

// start going up

    // FxCollector::getInstance().check();

    auto& fxRunner = iOThread->getFxRunner();
    
#if 0
#if 1

    fxRunner.add(3);
    fxRunner.add(4);
//    fxRunner.add(8);
    fxRunner.add(9);
//    fxRunner.add(5);
//    fxRunner.list();


    //fxRunner.connect(2,0);    // connect fxOscillatorMixer to fxEndMixer,0

    fxRunner.connect(2,3,0);
    //fxRunner.connect(4,2,1);
    //fxRunner.connect(8,0); // modulator
    fxRunner.connect(9,0);

#else
    fxRunner.add(3);
    fxRunner.add(4);
//    fxRunner.add(8);
    fxRunner.add(9);
//    fxRunner.add(5);
    fxRunner.list();
    fxmod->setProcMode(1);
    fxosc->setProcMode(1);
    fxfilt->setProcMode(2);
    fxnoise->setProcMode(1);

    //fxRunner.connect(2,0);    // connect fxOscillatorMixer to fxEndMixer,0

    fxRunner.connect(3,3,0);
    //fxRunner.connect(4,2,1);
    //fxRunner.connect(8,0); // modulator
    fxRunner.connect(9,0);

#endif
    
#endif

    // load initial setup

    setupEffects(sysman);
    
    try {
        //-------------------------
        // start jack thread
        jack.setProcessCB( iOThread, IOThread::midiInCB,  IOThread::audioOutCB );
        if( ! synthFe->initialize() )
            exit(-1);
        if( !jack.initialize() )
            exit(-1);
        if( !jack.run() )
            exit(-1);
        //-------------------------
        // start synth fe thread
        std::thread   synthFrontendThread( SynthFrontend::exec, synthFe );

        // -------------------------------------
        std::cout << "\n\n save ToneShaper[0]\n\n" << std::endl;
        ToneShaperMatrix& ts = oscArray->getToneShaperMatrix();
        
        Yaxp::Message yms;
        
        yms.setTags( 
            uint8_t(TagMainLevel_00::TagMain::ToneShaper),
            uint8_t(TagToneShaperLevel_01::TagToneShaper::SetOvertone)
        );
        
        std::ofstream file_tsout;
        file_tsout.open("toneshaper_out");

        YsifOutStream * yser= new YsifOutStream();
        
        serialize(*yser, yms);
        
        ts.dump( *yser );
        file_tsout << yser->rdbuf();
        file_tsout.close();

        std::ifstream file_tsin;
        file_tsin.open("toneshaper_in");

        YsifInpStream * ydeser=new YsifInpStream();

        if( file_tsin.good() ) {
            std::cout << "\n\n read a new  ToneShaper[0]\n\n" << std::endl;
            ydeser->seekp(0);
            *ydeser << file_tsin.rdbuf();
            file_tsin.close();
        
            deserialize(*ydeser, yms);
            
            if( !ts.fill( *ydeser ) ) {
                std::cerr << "error in loading toneshaper_in" << std::endl;
            }
        }

       std::cout << "\n\n============LETS GO==============\n\n" << std::endl;
       // std::cout << "\n\n============STOP==============\n\n" << std::endl;

    // run without thread
    //   synthFe->run();
       //exit(0);

       jack.unmute();

        //-------------------------
        // start ui server -- cmd processor thread
       uiServer.run();  // command processor
       jack.mute();
       synthFrontendThread.join();
       jack.shutdown();
    } catch (...) {
       jack.shutdown();
       return -1;
    }
//-----------------------------------------------------------
    return 0;
};


//-----------------------------------------------------------

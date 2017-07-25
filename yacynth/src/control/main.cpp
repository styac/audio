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
#include    "../effects/DelayTap.h"
#include    "../effects/FxFilter.h"
#include    "../effects/FxOutNoise.h"
#include    "../effects/FxModulator.h"
#include    "../effects/FxOutOscillator.h"
#include    "../effects/FxEcho.h"
#include    "../effects/FxLateReverb.h"
#include    "../effects/FxEarlyReflection.h"
#include    "../effects/FxChorus.h"
#include    "../effects/FxFlanger.h"

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
using namespace TagEffectFxLateReverbModeLevel_03;
using namespace TagEffectFxEarlyReflectionModeLevel_03;
using namespace TagEffectFxChorusModeLevel_03;
using namespace TagEffectFxFlangerModeLevel_03;

#include    "../control/global.h"
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

static int32_t relFreq2pitch( double relf ) {
    return std::round( std::log2( relf ) * (1<<24) );
}; 

static void basicInit( Sysman  * sysman )
{
    yaxp::Message msgBuffer;    
    // sequence important !!
    constexpr int   EffectInstance_Nil                  = 0;
    constexpr int   EffectInstance_Mixer4               = EffectInstance_Nil + 1;
    constexpr int   EffectInstance_OscillatorMixer      = EffectInstance_Nil + 2;
    FxOutNoise      * fxnoise = new FxOutNoise();
    constexpr int   EffectInstance_FxOutNoise           = EffectInstance_Nil + 3;
    FxOutOscillator * fxosc   = new FxOutOscillator();
    constexpr int   EffectInstance_FxOutOscillator      = EffectInstance_Nil + 4;
    constexpr int   EffectInstance_FxOutO_Slave1        = EffectInstance_Nil + 5;
    constexpr int   EffectInstance_FxOutO_Slave2        = EffectInstance_Nil + 6;
    constexpr int   EffectInstance_FxOutO_Slave3        = EffectInstance_Nil + 7;
    FxModulator     * fxmod   = new FxModulator();
    constexpr int   EffectInstance_FxModulator          = EffectInstance_Nil + 8;
    FxFilter        * fxfilt  = new FxFilter();
    constexpr int   EffectInstance_FxFilter             = EffectInstance_Nil + 9;
    FxEcho          * fxecho  = new FxEcho();
    constexpr int   EffectInstance_FxEcho               = EffectInstance_Nil + 10;
    FxLateReverb    * fxrevb  = new FxLateReverb();
    constexpr int   EffectInstance_FxLateReverb         = EffectInstance_Nil + 11;
    FxEarlyReflection  * fxearlyref  = new FxEarlyReflection();
    constexpr int   EffectInstance_FxEarlyReflection    = EffectInstance_Nil + 12;
    constexpr int   EffectInstance_FxEarlyReflection_Slave1 = EffectInstance_Nil + 13;
    FxChorus  * fxchorus  = new FxChorus();
    constexpr int   EffectInstance_FxChorus             = EffectInstance_Nil + 14;
    FxFlanger  * fxFlanger  = new FxFlanger();
    constexpr int   EffectInstance_FxFlanger            = EffectInstance_Nil + 15;    

    constexpr   int overToneCount = 16;
    
    ToneShaper ts;
    ts.clear();
    for( auto vi=0u; vi < overToneCount; ++vi ) {
        const float onevi = 1.0f/float(vi+1);
        ts.pitch = relFreq2pitch( vi+1 );
        ts.amplitudeDetune = 0;      
        ts.curveSpeedRelease = 2;
        ts.sustain.decayCoeff.setDecayPar( 0, 0 ); 
        ts.sustain.sustainModDepth  = 0;     // 100 / 256
        ts.sustain.sustainModPeriod = 300;
        ts.sustain.sustainModType   = 1;                
        ts.tickFrameRelease.setTickPar( 100, 0 );    
        ts.transient[ 2 ].tickFrame.setTickPar( 20, 0 );
        ts.transient[ 2 ].curveSpeed    = 2;
        ts.transient[ 2 ].targetValue.setPar( 65534 * onevi, 0 ); 
        ts.transient[ 1 ].tickFrame.setTickPar( 300, 0 );
        ts.transient[ 1 ].curveSpeed    = 1;  
        ts.transient[ 1 ].targetValue.setPar( 20000 * onevi, 0 ); 
        
        msgBuffer.clear();
        msgBuffer.setPar( 0, vi );
        msgBuffer.setTags( uint8_t(TagMain::ToneShaper), uint8_t(TagToneShaper::SetOvertone) );
        msgBuffer.getTargetData( ts );        
        sysman->evalMessage(msgBuffer);
        if( msgBuffer.messageType != yaxp::MessageT::responseSetOK ) {
            std::cout << "---- send TS error " << uint16_t(msgBuffer.messageType) << std::endl;
            exit(-1);
        }        
    }
    
    msgBuffer.clear();
    msgBuffer.setPar( 0, overToneCount );
    msgBuffer.setTags( uint8_t(TagMain::ToneShaper), uint8_t(TagToneShaper::SetOvertoneCount) );
    sysman->evalMessage(msgBuffer);
    if( msgBuffer.messageType != yaxp::MessageT::responseSetOK ) {
        std::cout << "---- send TS count ok error " << uint16_t(msgBuffer.messageType) << std::endl;
        exit(-1);
    }
        
    EffectRunnerSetConnections  effectRunnerSetConnections[] = {
        { EffectInstance_OscillatorMixer, 0, 0 },     // audio osc out to output mixer
    };

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectRunner )
                    ,   uint8_t( TagEffectRunner::SetConnections )
                    );
    msgBuffer.setPar(cArrayElementCount(effectRunnerSetConnections));
    msgBuffer.getTargetData(effectRunnerSetConnections);
    sysman->evalMessage(msgBuffer);
    if( msgBuffer.messageType != yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Connect error " << uint16_t(msgBuffer.messageType) << std::endl;
        exit(-1);
    }

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::SetProcessingMode )
                    );
    msgBuffer.setPar( 1,1 ); 
    sysman->evalMessage(msgBuffer);
    if( msgBuffer.messageType != yaxp::MessageT::responseSetOK  ) {
        std::cout << "---- set mode : mixer error " <<uint16_t(msgBuffer.messageType) << std::endl;
        exit(-1);
    }

    MidiControllerSetting  midiSetting[] = {
        {  110, 0, 0x31,  MidiController::CM_RANGE,   InnerController::CC_MAINVOLUME  },// volume - start with low
    };

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::MidiController )
                    ,   uint8_t( TagMidiController::SetController )
                    );
    msgBuffer.setPar(cArrayElementCount(midiSetting));
    msgBuffer.getTargetData(midiSetting);
    sysman->evalMessage(msgBuffer);
    if( msgBuffer.messageType != yaxp::MessageT::responseSetOK ) {
        std::cout << "---- MidiSetting error " <<uint16_t(msgBuffer.messageType) << std::endl;
        exit(-1);
    }

    float range0 = 1.0f;
    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::EffectInstance )
                    ,   uint8_t( TagEffectType::FxMixer )
                    ,   uint8_t( TagEffectFxMixerMode::SetVolumeRange )
                    );
    msgBuffer.setPar( EffectInstance_Mixer4, 0 ); // channel
    msgBuffer.getTargetData( range0 );
    sysman->evalMessage(msgBuffer);
    if( msgBuffer.messageType != yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Volume control range0 error " <<uint16_t(msgBuffer.messageType) << std::endl;
    }    

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::SetProcessingMode )
                    );
    msgBuffer.setPar( 1,3 ); // endmixer - mode 3 - 3 channel with 1 contreoller
    sysman->evalMessage(msgBuffer);
    if( msgBuffer.messageType != yaxp::MessageT::responseSetOK  ) {
        std::cout << "---- set mode : mixer error " <<uint16_t(msgBuffer.messageType) << std::endl;
        exit(-1);
    }

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::EffectInstance )
                    ,   uint8_t( TagEffectType::FxMixer )
                    ,   uint8_t( TagEffectFxMixerMode::SetVolumeControllerIndex )
                    );

    msgBuffer.setPar( EffectInstance_Mixer4, 0  );// channel
    uint16_t cindex0 = InnerController::CC_MAINVOLUME;
    msgBuffer.getTargetData( cindex0 );
    sysman->evalMessage(msgBuffer);
    if( msgBuffer.messageType != yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Volume control index0 error " <<uint16_t(msgBuffer.messageType) << std::endl;
    }

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::EffectInstance )
                    ,   uint8_t( TagEffectType::FxMixer )
                    ,   uint8_t( TagEffectFxMixerMode::SetVolumeRange )
                    );
    msgBuffer.setPar( EffectInstance_Mixer4, 0 ); // channel
    msgBuffer.getTargetData( range0 );
    sysman->evalMessage(msgBuffer);
    if( msgBuffer.messageType != yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Volume control range0 error " <<uint16_t(msgBuffer.messageType) << std::endl;
    }    
}
// --------------------------------------------------------------------

static void teststuff(void)
{
    std::cout << std::dec
        << "\nOscillatorArray size: "       << sizeof(OscillatorArray)
        << "\nToneShaperMatrix size: "      << sizeof(ToneShaperMatrix)
        << "\nToneShaperVector size: "      << sizeof(ToneShaperVector)
        << "\nInnerController size: "       << sizeof(InnerController)
        << "\nMidiRangeController size: "   << sizeof(MidiController)
        << "\nYax::Header size: "           << sizeof(yaxp::Header)
        << "\n--------------------"
        << "\nsize: v4sf "                  << sizeof(v4sf)
        << std::hex
        << "\nfreq2deltaPhase( 1.0 ) "      << freq2deltaPhase( 1.0 )
        << "\nrefA440ycent:       "         << refA440ycent
        << "\nrefA440ycentDouble: "         << uint64_t(std::llround(refA440ycentDouble))
        << "\n19990:"                       << ref19900ycent
        << "\n0.01:"                        << ref0_01ycent
        << std::dec
        << "\n refA440ycent: "              << refA440ycent
        << "\n refA440ycentDouble: "        << uint64_t(refA440ycentDouble*10)
        << "\n refA440ycentDouble: "        << refA440ycentDouble
        << "\n Header: "                  << sizeof(yaxp::Header)
        << "\nToneShaper size:          " << sizeof(ToneShaper)
        << "\nAmplitudeSustain size:    " << sizeof(AmplitudeSustain)
        << "\nAmplitudeTransient size:  " << sizeof(AmplitudeTransient)
        << "\nInterpolatedu32 size:     " << sizeof(InterpolatedAmplitudeU32)
        << "\nInterpolatedu16 size:     " << sizeof(InterpolatedDecreaseU16)
        << "\n\n"
        << std::endl;
#if 0
    for( float i=1.0e+30f; i>std::numeric_limits<float>::min(); i /= 10 ) {
        float v = noisefloor(i);
        uint32_t b = *reinterpret_cast<uint32_t*>(&i);
        if( v != i )
            std::cout << std::dec
                << "i=" << i
                << " b=" << std::hex << b
                << " val=" << v
                << std::endl;
    }
    exit(0);
#endif

#if 0
    // 13 d0 0000
    for( int32_t ycent = 0x01000000; ycent < 0x1F000000; ycent += 0x00100000 ) {
        std::cout << std::hex
            << "ycent=" << ycent
            << " f=" <<  FilterTable2SinPi::getInstance().getFloat( ycent )
            << std::endl;
    }
    exit(0);
#endif


}

// --------------------------------------------------------------------
//  -- MAIN --
// --------------------------------------------------------------------
int main( int argc, char** argv )
{
    // init singletons    
    GaloisShifterSingle<seedThreadEffect_noise>& gs0        = GaloisShifterSingle<seedThreadEffect_noise>::getInstance();
    GaloisShifterSingle<seedThreadOscillator_noise>& gs1    = GaloisShifterSingle<seedThreadOscillator_noise>::getInstance();
    GaloisShifterSingle<seedThreadEffect_random>& gs2       = GaloisShifterSingle<seedThreadEffect_random>::getInstance();
    GaloisShifterSingle<seedThreadOscillator_random>& gs3   = GaloisShifterSingle<seedThreadOscillator_random>::getInstance();

    FilterTableExp2Pi::getInstance();
    FilterTableSinCosPi2::getInstance();
    FilterTable2SinPi::getInstance();
    FilterTableCos2Pi::getInstance();
    SinTable::table();

    YaIoJack::getInstance();
    
    // OBSOLETE
    LowOscillatorArray::getInstance().reset();

    YaIoInQueueVector&      queuein     = YaIoInQueueVector::getInstance();
    OscillatorOutVector&    oscOutVec   = OscillatorOutVector::getInstance();
    InnerController&        controller  = InnerController::getInstance();
    FxCollector::getInstance();

    
    struct sigaction sigact;
    memset( &sigact, 0, sizeof(sigact) );
    sigfillset( &sigact.sa_mask );
    sigact.sa_handler = signal_handler;
	sigaction( SIGTERM, &sigact, NULL );
    sigaction( SIGINT, &sigact, NULL );
    
    
    uint16_t   port( yaxp::defaultPort ); // from param

    const char *homedir = getenv("HOME");

    teststuff();

    // inter thread communication

    OscillatorArray     *oscArray   = new OscillatorArray();
    SimpleMidiRouter    *midiRoute  = new SimpleMidiRouter();

    // threads
    IOThread            *iOThread   = new IOThread(      queuein, oscOutVec, *midiRoute );
    SynthFrontend       *synthFe    = new SynthFrontend( queuein, oscOutVec, oscArray );

    Sysman              *sysman     = new Sysman( *oscArray, *iOThread );
    Server              uiServer( *sysman, port );
    auto& fxRunner      = iOThread->getFxRunner();

    //
    // authentication
    // const char * const confDir  = ".yacynth";
    // const char * const seedFile = ".yaxp.seed";
    // open $HOME/.yacynth/.yaxp.seed
    // read and uiServer.setAuthSeed( );
    //
    
    try {
        basicInit( sysman );
        //-------------------------
        // start jack thread
        YaIoJack::getInstance().setProcessCB( iOThread, IOThread::midiInCB,  IOThread::audioOutCB );
        if( ! synthFe->initialize() )
            exit(-1);
        if( !YaIoJack::getInstance().initialize() )
            exit(-1);
        if( !YaIoJack::getInstance().run() )
            exit(-1);
        //-------------------------
        // start synth fe thread
        std::thread   synthFrontendThread( SynthFrontend::exec, synthFe );
        std::cout << "\n\n============LETS GO==============\n\n" << std::endl;
        YaIoJack::getInstance().unmute();

        //-------------------------
        // start ui server -- cmd processor thread
        uiServer.run();  // command processor
        std::cout << "\n\n============END==============\n\n" << std::endl;
        YaIoJack::getInstance().mute();
        synthFe->stop();
        synthFrontendThread.join();
        YaIoJack::getInstance().shutdown();
    } catch (...) {
        YaIoJack::getInstance().mute();
        synthFe->stop();
        YaIoJack::getInstance().shutdown();
        return -1;
    }
//    synthFrontendThread.join();
//-----------------------------------------------------------
    return 0;
};


//-----------------------------------------------------------

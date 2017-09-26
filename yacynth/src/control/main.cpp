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
// #include    <sndfile.h>
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
#include "Setting.h"
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
    FxOutNoise          * fxnoise       = new FxOutNoise();
    FxOutOscillator     * fxosc         = new FxOutOscillator();
    FxModulator         * fxmod         = new FxModulator();
    FxFilter            * fxfilt        = new FxFilter();
    FxEcho              * fxecho        = new FxEcho();
    FxLateReverb        * fxrevb        = new FxLateReverb();
    FxEarlyReflection   * fxearlyref    = new FxEarlyReflection();
    FxChorus            * fxchorus      = new FxChorus();
    FxFlanger           * fxFlanger     = new FxFlanger();

    ToneShaper ts;
    ts.clear();
    ts.pitch = 0;
    ts.amplitudeDetune = 0;
    ts.sustain.decayCoeff.setPar( 0, 0 );
    ts.sustain.modDepth  = 0;     // 100 / 256
    ts.sustain.modDeltaPhase = 300;
    ts.tickFrameRelease.setPar( 100, 0, 2 );
    ts.transient[ 2 ].tickFrame.setPar( 20, 0, 2 );
    ts.transient[ 2 ].targetValue = uint32_t( 32000.0 * 65535.0 );
    ts.transient[ 1 ].tickFrame.setPar( 300, 0, 1 );
    ts.transient[ 1 ].targetValue = uint32_t( 20000.0 * 65535.0 );

    ts.oscillatorType = ToneShaper::OSC_SIN;
    // filter test
    // ts.oscillatorType = ToneShaper::OSC_NOISE_SV3x4_PEEK;
    ts.filterBandwidth = 0x60;
    ts.veloBoost = 0;
    ts.outChannel = 0;

    msgBuffer.clear();
    msgBuffer.setPar( 0, 0 );
    msgBuffer.setTags( uint8_t(TagMain::ToneShaper), uint8_t(TagToneShaper::SetOvertone) );
    msgBuffer.getTargetData( ts );
    sysman->evalMessage(msgBuffer);
    if( msgBuffer.messageType != yaxp::MessageT::responseSetOK ) {
        std::cout << "---- send TS error " << uint16_t(msgBuffer.messageType) << std::endl;
        exit(-1);
    }

    // velo BOOST test
    constexpr   int overToneCount = 8;
    for( auto vi=1u; vi < overToneCount; ++vi ) {
//        const float onevi = 1.0f/float(vi+1);
//        ts.pitch = relFreq2pitch( vi+1 );
        const float onevi = 1.0f/float( vi + 1 );
        ts.pitch = relFreq2pitch( vi + 1 );
        ts.amplitudeDetune = 0;
        ts.sustain.decayCoeff.setPar( 0, 0 );
        ts.sustain.modDepth  = 0;     // 100 / 256
        ts.sustain.modDeltaPhase = 300;
        ts.tickFrameRelease.setPar( 100, 0, 2 );
        ts.transient[ 2 ].tickFrame.setPar( 20, 0, 2 );
        ts.transient[ 2 ].targetValue = uint32_t( 32000.0 * 65535.0 * onevi );
        ts.transient[ 1 ].tickFrame.setPar( 300, 0, 1 );
        ts.transient[ 1 ].targetValue = uint32_t( 20000.0 * 65535.0 * onevi );

        ts.oscillatorType = ToneShaper::OSC_SIN;
        if( vi > 3 ) {
            ts.veloBoost = 0;// 255;
            ts.outChannel = 0;
        } else {
            ts.veloBoost = 0; //255;
            ts.outChannel = 0;
        }

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

    // get
    msgBuffer.clear();
    msgBuffer.setTags( uint8_t( TagMain::EffectCollector )
            ,uint8_t(TagEffectCollector::GetEffectList )
            );

    EffectMap   effectMap;
    sysman->evalMessage(msgBuffer);
    if( msgBuffer.messageType == yaxp::MessageT::responseGetOK  ) {
        EffectListEntry *data = static_cast<yacynth::EffectListEntry *>((void *)(msgBuffer.data));
        for( uint16_t ind = 0; ind < msgBuffer.params[0]; ++ind ) {
            effectMap.emplace( data->fullName, data->id );
            std::cout
                << "++ ind  "           << uint16_t(data->fxIndex)
                << " id "               << uint16_t(data->id)
                << " type "             << uint16_t(data->fxType)
                << " maxMode "          << uint16_t(data->fxMaxMode)
                << " inputCount "       << uint16_t(data->inputCount )
                << " masterId "         << uint16_t(data->masterId)
                << " instanceIndex "    << uint16_t(data->instanceIndex)
                << "  "                 << data->name
                << "  "                 << data->fullName
                << std::endl;
            ++data;
        }
        for( auto& fxid : effectMap ) {
            std::cout
                << " id "               << uint16_t( fxid.second )
                << "  "                 << fxid.first
                << std::endl;
        }

    } else {
        std::cout << "---- error " <<uint16_t(msgBuffer.messageType) << std::endl;
        exit(-1);
    }

    const uint8_t EffectInstance_Mixer4             = effectMap["EndMixer"];
    const uint8_t EffectInstance_OscillatorMixer    = effectMap["OscillatorMixer.00:00"];

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectRunner )
                    ,   uint8_t( TagEffectRunner::Preset0 )
                    );
    sysman->evalMessage(msgBuffer);
    if( msgBuffer.messageType != yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Connect error " << uint16_t(msgBuffer.messageType) << std::endl;
        exit(-1);
    }

    MidiControllerSetting  midiSetting[] = {
        {  110, 0, 0x31,  MidiController::CM_RANGE, InnerController::CC_MAINVOLUME  },// volume - start with low
    };

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::MidiController )
                    ,   uint8_t( TagMidiController::SetController )
                    );

    msgBuffer.setPar(cArrayElementCount(midiSetting));
    msgBuffer.getTargetData(midiSetting);
    sysman->evalMessage(msgBuffer);
    if( msgBuffer.messageType != yaxp::MessageT::responseSetOK ) {
        std::cout << "---- MidiSetting error " << uint16_t(msgBuffer.messageType) << std::endl;
        exit(-1);
    }

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::EffectInstance )
                    ,   uint8_t( TagEffectType::FxMixer )
                    ,   uint8_t( TagEffectFxMixerMode::Preset0 )
                    );
    msgBuffer.setPar( EffectInstance_Mixer4, 0 ); // channel
    sysman->evalMessage(msgBuffer);
    if( msgBuffer.messageType != yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Volume control Preset0 error " << uint16_t(msgBuffer.messageType) << std::endl;
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
        std::cout << "---- Volume control index0 error " << uint16_t(msgBuffer.messageType) << std::endl;
    }    
}

static void teststuff(void)
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
        << "\nfreq2deltaPhase( 1.0 )    "   << freq2deltaPhase( 1.0 )
        << "\nrefA440ycent:             "   << refA440ycent
        << "\nrefA440ycentDouble:       "   << uint64_t(std::llround(refA440ycentDouble))
        << "\n19990:                    "   << ref19900ycent
        << "\n0.01:                     "   << ref0_01ycent
        << std::dec
        << "\n\n"
        << std::endl;

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

    ExpTable::getInstance();
    FilterTableExp2Pi::getInstance();
    FilterTableSinCosPi2::getInstance();
    FilterTable2SinPi::getInstance();
    FilterTableCos2Pi::getInstance();
    SinTable::table();
    VelocityBoostTable::getInstance();
    YaIoJack::getInstance();

    // OBSOLETE
    // LowOscillatorArray::getInstance().reset();

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

    Setting settings;

    std::cout << "\n\n============ settings =============="
        << "\n homedir  : "  << settings.getHomeDir()
        << "\n confir   : "  << settings.getConfDir()
        << "\n auth     : "  << settings.getAuthKey()
        << std::endl;

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
        // use :   int nanosleep(const struct timespec *req, struct timespec *rem);
        sleep(1); // wait to relax
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

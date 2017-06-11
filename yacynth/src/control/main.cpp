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
    yaxp::Message msgBuffer;


/*
---- ind  0 id 0 type 1 maxMode 0 inputCount 0 masterId 0  Nil
---- ind  1 id 1 type 3 maxMode 3 inputCount 4 masterId 0  Mixer4
---- ind  2 id 2 type 4 maxMode 1 inputCount 0 masterId 0  OscillatorMixer
---- ind  3 id 3 type 6 maxMode a inputCount 0 masterId 0  NoiseSource
---- ind  4 id 4 type 7 maxMode 5 inputCount 0 masterId 0  Oscillator4x
---- ind  5 id 5 type 2 maxMode 0 inputCount 0 masterId 4   ^OscillatorSlave
---- ind  6 id 6 type 2 maxMode 0 inputCount 0 masterId 4   ^OscillatorSlave
---- ind  7 id 7 type 2 maxMode 0 inputCount 0 masterId 4   ^OscillatorSlave
---- ind  8 id 8 type 5 maxMode 6 inputCount 2 masterId 0  Modulator
---- ind  9 id 9 type 8 maxMode 9 inputCount 1 masterId 0  Filter
---- ind  a id a type 9 maxMode 1 inputCount 1 masterId 0  Echo
---- ind  b id b type a maxMode 3 inputCount 1 masterId 0  FxReverb
 */

    // put the standard components into here
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


    // temp
    fxmod->setProcMode(3);

    fxosc->setProcMode(10);


    fxfilt->setProcMode(2);

    fxnoise->setProcMode(2);
    fxrevb->setProcMode(1);
    fxearlyref->setProcMode(2);
    fxecho->setProcMode(2);
    fxchorus->setProcMode(1);
    fxFlanger->setProcMode(3);

// ------------------------------------------------------------
    std::cout << "\n---- REMOTE LOAD \n" << std::endl;
    return; // for the remote setup
// ------------------------------------------------------------

    std::cout << "\n---- Collector get list\n" << std::endl;

    msgBuffer.clear();
    msgBuffer.setLength(50000);
    msgBuffer.setTags( uint8_t( TagMain::EffectCollector )
            ,uint8_t(TagEffectCollector::GetEffectList )
            );

    sysman->evalMessage(msgBuffer);

    if( msgBuffer.messageType == yaxp::MessageT::responseGetOK  ) {

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
        exit(-1);
    }

    std::cout << "\n---- set mode : mixer" << std::endl;
    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::SetProcessingMode )
                    );
    msgBuffer.setPar( 1,3 ); // endmixer - mode 3 - 3 channel with 1 contreoller
    sysman->evalMessage(msgBuffer);

    if( msgBuffer.messageType == yaxp::MessageT::responseSetOK  ) {
        std::cout << "---- set mode : mixer ok" << std::endl;
    } else {
        std::cout << "---- set mode : mixer error " <<uint16_t(msgBuffer.messageType) << std::endl;
        exit(-1);
    }

// ------------------------------------------------------------

    std::cout << "\n---- Fill runner\n" << std::endl;

#if 0
    // filter  test
    EffectRunnerFill effectFill[] = {
        EffectInstance_FxOutNoise,
//        EffectInstance_FxOutOscillator,
        EffectInstance_FxFilter
    };    // osc + reverb
#endif

#if 1
    // flanger  test
    EffectRunnerFill effectFill[] = {
//        EffectInstance_FxOutOscillator,
        EffectInstance_FxFlanger
    };    // osc + reverb
#endif


#if 0
    // chorus  test
    EffectRunnerFill effectFill[] = {
        EffectInstance_FxOutOscillator,
        EffectInstance_FxChorus
    };    // osc + reverb
#endif

#if 0
    // echo  test
    EffectRunnerFill effectFill[] = {
        EffectInstance_FxOutOscillator,
        EffectInstance_FxEcho
    };    // osc + reverb
#endif


#if 0
    // latereverb osc test
    EffectRunnerFill effectFill[] = {
        EffectInstance_FxOutOscillator,
        EffectInstance_FxLateReverb
    };    // osc + reverb
#endif

#if 0
    EffectRunnerFill effectFill[] = {
        EffectInstance_FxEarlyReflection,
        EffectInstance_FxLateReverb,
    };    // osc + reverb
#endif

    //EffectRunnerFill effectFill[] = {4,8};    // osc + mod
    //EffectRunnerFill effectFill[] = {3};    // noise

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectRunner )
                    ,   uint8_t( TagEffectRunner::Fill )
                    );
    msgBuffer.setPar(cArrayElementCount(effectFill));
    msgBuffer.getTargetData(effectFill);
    sysman->evalMessage(msgBuffer);
    if( msgBuffer.messageType == yaxp::MessageT::responseSetOK ) {
        std::cout << "---- fill ok" << std::endl;
    } else {
        std::cout << "---- error " <<uint16_t(msgBuffer.messageType) << std::endl;
        exit(-1);
    }
// ------------------------------------------------------------

    msgBuffer.clear();
    msgBuffer.setLength(50000);
    msgBuffer.setTags(  uint8_t( TagMain::EffectRunner )
                    ,   uint8_t( TagEffectRunner::GetEffectList )
                    );
    sysman->evalMessage(msgBuffer);
    if( msgBuffer.messageType == yaxp::MessageT::responseGetOK ) {
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
        std::cout << "---- TagEffectRunner::GetEffectList error " <<uint16_t(msgBuffer.messageType) << std::endl;
        exit(-1);
    }


// ------------------------------------------------------------

// connect
    std::cout << "\n---- Connect effects\n" << std::endl;


#if 0
    // flanger test
    EffectRunnerSetConnections  effectRunnerSetConnections[] = {

        { EffectInstance_FxOutNoise, 2, 0 },    // noise osc out to filter 0
//        { EffectInstance_FxOutOscillator, 2, 0 },    // audio osc out to reverb 0
        //{ EffectInstance_OscillatorMixer, 2, 0 },     // audio osc out to filter 0
        { EffectInstance_FxFilter, 0, 0 }           // filter to output 0
    };
#endif


#if 1
    // flanger test
    EffectRunnerSetConnections  effectRunnerSetConnections[] = {
//        { EffectInstance_FxOutOscillator, 2, 0 },    // audio osc out to reverb 0
        { EffectInstance_OscillatorMixer, 1, 0 },     // audio osc out to reverb 0
        { EffectInstance_FxFlanger, 0, 0 }           // reverb to output 0
    };
#endif

#if 0
    EffectRunnerSetConnections  effectRunnerSetConnections[] = {
        { 2, 2, 0 },    // audio osc out to modulator 0
        { 4, 2, 1 },    // low freq osc to  modulator 1
        { 8, 0, 0 }     // modulator to output 0
    };
#endif



#if 0
    // chorus test
    EffectRunnerSetConnections  effectRunnerSetConnections[] = {
        { EffectInstance_FxOutOscillator, 2, 0 },    // audio osc out to reverb 0
        { EffectInstance_FxChorus, 0, 0 }           // chorus to output 0
    };
#endif

#if 0
    // echo test
    EffectRunnerSetConnections  effectRunnerSetConnections[] = {
        { EffectInstance_FxOutOscillator, 2, 0 },    // audio osc out to reverb 0
        { EffectInstance_FxEcho, 0, 0 }     // echo to output 0
    };
#endif

#if 0
    // reverb test
    EffectRunnerSetConnections  effectRunnerSetConnections[] = {
        { EffectInstance_FxOutOscillator, 2, 0 },    // audio osc out to reverb 0
        { EffectInstance_FxLateReverb, 0, 0 }     // reverb to output 0
    };
#endif


#if 0
    // reverb test
    EffectRunnerSetConnections  effectRunnerSetConnections[] = {
        { EffectInstance_OscillatorMixer,           1, 0 },     // audio osc out to reverb 0
        { EffectInstance_FxEarlyReflection,         0, 1 },     // reverb to output mixer 1
        { EffectInstance_FxLateReverb,              0, 2 },     // reverb to output mixer 2
        { EffectInstance_FxEarlyReflection_Slave1,  2, 0 },     // early slave to late reverb
        { EffectInstance_OscillatorMixer,           0, 0 },     // audio osc to output mixer 0
//        { EffectInstance_FxReverb, 0, 0 },     // reverb to output 0
    };
#endif
#if 0
// noise test
    EffectRunnerSetConnections  effectRunnerSetConnections[] = {
        { 3, 0, 0 }     // modulator to output 0
    };
#endif

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectRunner )
                    ,   uint8_t( TagEffectRunner::SetConnections )
                    );
    msgBuffer.setPar(cArrayElementCount(effectRunnerSetConnections));
    msgBuffer.getTargetData(effectRunnerSetConnections);
    sysman->evalMessage(msgBuffer);

    if( msgBuffer.messageType == yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Connect ok" << std::endl;
    } else {
        std::cout << "---- Connect error " <<uint16_t(msgBuffer.messageType) << std::endl;
        exit(-1);
    }

// ------------------------------------------------------------

    std::cout << "\n---- Runner get list\n" << std::endl;

    msgBuffer.clear();
    msgBuffer.setLength(50000);
    msgBuffer.setTags( uint8_t( TagMain::EffectRunner )
            ,uint8_t(TagEffectRunner::GetEffectList )
            );
    sysman->evalMessage(msgBuffer);

    if( msgBuffer.messageType == yaxp::MessageT::responseGetOK ) {

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
        exit(-1);
    }
// ------------------------------------------------------------

    std::cout << "\n---- MIDI controllers\n" << std::endl;

    // Impulse48 - channel 0
    MidiSetting  midiSetting[] = {
        { 0, 0x2E,  MidiController::CM_RANGE,   InnerController::CC_MODULATOR_FREQ1,        0   },// modulator
        { 0, 0x2F,  MidiController::CM_RANGE,   InnerController::CC_MODULATOR_FREQ0,        0   },// modulator
        { 0, 0x30,  MidiController::CM_RANGE,   InnerController::CC_MODULATOR_PHASEDIFF0,   0   },// modulator
        { 0, 0x31,  MidiController::CM_RANGE,   InnerController::CC_MAINVOLUME,             110  },// volume - start with low
        { 0, 0x2D,  MidiController::CM_RANGE,   InnerController::CC_MODULATOR_INVOL,        99  },// modulator IN volume
        { 0, 0x2C,  MidiController::CM_RANGE,   InnerController::CC_MODULATOR_MIXVOL,       99  },// modulator MIX volume

        // filter test
        { 0, 0x4B,  MidiController::CM_RANGE,   InnerController::CC_FILTER_FREQ0,       12  },// modulator MIX volume
        { 0, 0x4C,  MidiController::CM_RANGE,   InnerController::CC_FILTER_Q0,          12  },// modulator MIX volume

        { 0, 0,     MidiController::CM_DISABLE, InnerController::CC_SINK,  0 }    //


    };

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::MidiController )
                    ,   uint8_t( TagMidiController::SetController )
                    );
    msgBuffer.setPar(cArrayElementCount(midiSetting));
    msgBuffer.getTargetData(midiSetting);
    sysman->evalMessage(msgBuffer);

    if( msgBuffer.messageType == yaxp::MessageT::responseSetOK ) {
        std::cout << "---- MidiSetting ok" << std::endl;
    } else {
        std::cout << "---- MidiSetting error " <<uint16_t(msgBuffer.messageType) << std::endl;
        exit(-1);
    }

// ------------------------------------------------------------


    std::cout << "\n---- Inner controllers\n" << std::endl;

    InnerControllerSetting  innerControllerSetting[] = {
        { 0, 0 },    //
        { 0, 0 },    //
        { 0, 0 },    //
        { 0, 0 },    //
        { 0, 0 },    //
        { 0, 0 },    //
        { 0, 0 }     //
    };

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::InnerController )
                    ,   uint8_t( TagInnerController::SetController )
                    );
    msgBuffer.setPar(cArrayElementCount(innerControllerSetting));
    msgBuffer.getTargetData(innerControllerSetting);
    sysman->evalMessage(msgBuffer);

    if( msgBuffer.messageType == yaxp::MessageT::responseSetOK ) {
        std::cout << "---- InnerControllerSetting ok" << std::endl;
    } else {
        std::cout << "---- InnerControllerSetting error " <<uint16_t(msgBuffer.messageType) << std::endl;
        exit(-1);
    }
// ------------------------------------------------------------

    std::cout << "\n---- Volume control\n" << std::endl;
    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::EffectInstance )
                    ,   uint8_t( TagEffectType::FxMixer )
                    ,   uint8_t( TagEffectFxMixerMode::SetVolumeControllerIndex )
                    );

    msgBuffer.setPar(   EffectInstance_Mixer4 // effect instance
                    ,   0  );// channel
    uint16_t cindex0 = InnerController::CC_MAINVOLUME;
    msgBuffer.getTargetData( cindex0 );
    sysman->evalMessage(msgBuffer);
    if( msgBuffer.messageType == yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Volume control index0 ok" << std::endl;
    } else {
        std::cout << "---- Volume control index0 error " <<uint16_t(msgBuffer.messageType) << std::endl;
    }

    float range0 = 1.0f;
    float range1 = 0.3f;
    float range2 = 0.3f;

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::EffectInstance )
                    ,   uint8_t( TagEffectType::FxMixer )
                    ,   uint8_t( TagEffectFxMixerMode::SetVolumeRange )
                    );
    msgBuffer.setPar(   EffectInstance_Mixer4 // effect instance
                    ,   0 ); // channel

    msgBuffer.getTargetData( range0 );
    sysman->evalMessage(msgBuffer);
    if( msgBuffer.messageType == yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Volume control range0 ok" << std::endl;
    } else {
        std::cout << "---- Volume control range0 error " <<uint16_t(msgBuffer.messageType) << std::endl;
    }
// ------------------------------------------------------------

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::EffectInstance )
                    ,   uint8_t( TagEffectType::FxMixer )
                    ,   uint8_t( TagEffectFxMixerMode::SetVolumeRange )
                    );
    msgBuffer.setPar(   EffectInstance_Mixer4 // effect instance
                    ,   1 ); // channel

    msgBuffer.getTargetData( range1 );
    sysman->evalMessage(msgBuffer);
    if( msgBuffer.messageType == yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Volume control range1 ok" << std::endl;
    } else {
        std::cout << "---- Volume control range1 error " <<uint16_t(msgBuffer.messageType) << std::endl;
    }

// ------------------------------------------------------------

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::EffectInstance )
                    ,   uint8_t( TagEffectType::FxMixer )
                    ,   uint8_t( TagEffectFxMixerMode::SetVolumeRange )
                    );
    msgBuffer.setPar(   EffectInstance_Mixer4 // effect instance
                    ,   2 ); // channel

    msgBuffer.getTargetData( range2 );
    sysman->evalMessage(msgBuffer);
    if( msgBuffer.messageType == yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Volume control range2 ok" << std::endl;
    } else {
        std::cout << "---- Volume control range2 error " <<uint16_t(msgBuffer.messageType) << std::endl;
    }

// ------------------------------------------------------------

    std::cout << "\n---- Outoscillator control\n" << std::endl;
    FxOutOscillatorParam fxOutOscillatorParam = {
        // 1x offset + 1xslope
        .freqMapper = {
        .slope = 1<<(27-7),
        .shift = 0,
        .y0 = { int32_t(freq2ycent(0.1)) },
        },
        InnerController::CC_MODULATOR_FREQ0,
        // 4 index
        { InnerController::CC_MODULATOR_PHASEDIFF0,
        0,
        0,
        0 }
    };

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::EffectInstance )
                    ,   uint8_t( TagEffectType::FxOutOscillator )
                    ,   uint8_t( TagEffectFxOutOscillatorMode::SetParameters )
                    );

    msgBuffer.setPar(   4 // effect instance
                     );

    msgBuffer.getTargetData(fxOutOscillatorParam);
    sysman->evalMessage(msgBuffer);

    if( msgBuffer.messageType == yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Outoscillator control ok" << std::endl;
    } else {
        std::cout << "---- Outoscillator control error " <<uint16_t(msgBuffer.messageType) << std::endl;
        exit(-1);
    }

    std::cout << "\n---- Modulator control\n" << std::endl;

    FxModulatorParam fxModulatorParam = {
        InnerController::CC_MODULATOR_INVOL,
        InnerController::CC_MODULATOR_MIXVOL,
    };

// ------------------------------------------------------------
    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::EffectInstance )
                    ,   uint8_t( TagEffectType::FxModulator )
                    ,   uint8_t( TagEffectFxModulatorMode::SetParameters )
                    );

    msgBuffer.setPar(   8 // effect instance
                     );

    msgBuffer.getTargetData(fxModulatorParam);
    sysman->evalMessage(msgBuffer);

    if( msgBuffer.messageType == yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Modulator control ok" << std::endl;
    } else {
        std::cout << "---- Modulator control error " <<uint16_t(msgBuffer.messageType) << std::endl;
        exit(-1);
    }

// ------------------------------------------------------------

    std::cout << "\n---- Noise generator \n" << std::endl;

    FxOutNoiseParam fxOutNoiseParam = {
        1,
        1,
        1,
    };


    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::EffectInstance )
                    ,   uint8_t( TagEffectType::FxOutNoise )
                    ,   uint8_t( TagEffectFxOutNoiseMode::SetParameters )
                    );

    msgBuffer.setPar(   3 // effect instance
                     );

    msgBuffer.getTargetData(fxOutNoiseParam);
    sysman->evalMessage(msgBuffer);

    if( msgBuffer.messageType == yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Noise generator ok" << std::endl;
    } else {
        std::cout << "---- Noise generator error " <<uint16_t(msgBuffer.messageType) << std::endl;
        exit(-1);
    }

    std::cout << "\n---- Reverb \n" << std::endl;


    /*
   , 23       , 29       , 31       , 37       , 41       , 43       , 47       , 53
       , 59       , 61       , 67       , 71       , 73       , 79       , 83       , 89
       , 97      , 101      , 103      , 107      , 109      , 113      , 127      , 131
      , 137      , 139      , 149      , 151      , 157      , 163      , 167      , 173
      , 179      , 181      , 191      , 193      , 197      , 199      , 211      , 223
      , 227      , 229      , 233      , 239      , 241      , 251      , 257      , 263
      , 269      , 271      , 277      , 281      , 283      , 293      , 307      , 311
      , 313      , 317      , 331      , 337      , 347      , 349      , 353      , 359
      , 367      , 373      , 379      , 383      , 389      , 397      , 401      , 409
      , 419      , 421      , 431      , 433      , 439      , 443      , 449      , 457
      , 461      , 463      , 467      , 479      , 487      , 491      , 499      , 503
      , 509      , 521      , 523      , 541      , 547      , 557      , 563      , 569
      , 571      , 577      , 587      , 593      , 599      , 601      , 607      , 613
      , 617      , 619      , 631      , 641      , 643      , 647      , 653      , 659


     , 1009     , 1013     , 1019     , 1021     , 1031     , 1033     , 1039     , 1049
     , 1051     , 1061     , 1063     , 1069     , 1087     , 1091     , 1093     , 1097
     , 1103     , 1109     , 1117     , 1123     , 1129     , 1151     , 1153     , 1163
     , 1171     , 1181     , 1187     , 1193     , 1201     , 1213     , 1217     , 1223
     , 1229     , 1231     , 1237     , 1249     , 1259     , 1277     , 1279     , 1283
     , 1289     , 1291     , 1297     , 1301     , 1303     , 1307     , 1319     , 1321
     , 1327     , 1361     , 1367     , 1373     , 1381     , 1399     , 1409     , 1423
     , 1427     , 1429     , 1433     , 1439     , 1447     , 1451     , 1453     , 1459
     , 1471     , 1481     , 1483     , 1487     , 1489     , 1493     , 1499     , 1511
     , 1523     , 1531     , 1543     , 1549     , 1553     , 1553     , 1567     , 1571
     , 1579     , 1583     , 1597     , 1601     , 1607     , 1609     , 1613     , 1619
     , 1621     , 1627     , 1637     , 1657     , 1663     , 1667     , 1669     , 1693
     , 1697     , 1699     , 1709     , 1721     , 1723     , 1733     , 1741     , 1747
     , 1753     , 1759     , 1777     , 1783     , 1787     , 1789     , 1801     , 1811
     , 1823     , 1831     , 1847     , 1861     , 1867     , 1871     , 1873     , 1877
     , 1879     , 1889     , 1901     , 1907     , 1913     , 1931     , 1933     , 1949
     , 1951     , 1973     , 1979     , 1987     , 1993     , 1997     , 1999     , 2003
     , 2011     , 2017     , 2027     , 2029     , 2039     , 2053     , 2063     , 2069
     , 2081     , 2083     , 2087     , 2089     , 2099     , 2111     , 2113     , 2129
     , 2131     , 2137     , 2141     , 2143     , 2153     , 2161     , 2179     , 2203
     , 2207     , 2213     , 2221     , 2237     , 2239     , 2243     , 2251     , 2267

     *
            , 59       , 61       , 67       , 71       , 73       , 79       , 83       , 89
       , 97      , 101      , 103      , 107      , 109      , 113      , 127      , 131
      , 137      , 139      , 149      , 151      , 157      , 163      , 167      , 173
      , 179      , 181      , 191      , 193      , 197      , 199      , 211      , 223
      , 227      , 229      , 233      , 239      , 241      , 251      , 257      , 263
      , 269      , 271      , 277      , 281      , 283      , 293      , 307      , 311
      , 313      , 317      , 331      , 337      , 347      , 349      , 353      , 359
      , 367      , 373      , 379      , 383      , 389      , 397      , 401      , 409
      , 419      , 421      , 431      , 433      , 439      , 443      , 449      , 457
      , 461      , 463      , 467      , 479      , 487      , 491      , 499      , 503
      , 509      , 521      , 523      , 541      , 547      , 557      , 563      , 569
      , 571      , 577      , 587      , 593      , 599      , 601      , 607      , 613
      , 617      , 619      , 631      , 641      , 643      , 647      , 653      , 659
*
     */


    FxLateReverbParam::Mode01 mode01_Housholder = {

    //  MonoDelayBandpassTapArray<combCount> tapFeedback;

        // coeff
        0.115, 0.115, 0.115, 0.115,
        0.115, 0.115, 0.115, 0.115,

        // low pass k
        0.79, 0.79, 0.79, 0.79,
        0.79, 0.79, 0.79, 0.79,

        // high pass k
        0.95, 0.95, 0.95, 0.95,
        0.95, 0.95, 0.95, 0.95,

        // delay index
        1327,   1427,   1511,   1567,
        1019,   1151,   1213,   1277,

    //  MonoDelayTapArray<combCount>         tapFeedbackInternal;

        -0.111, -0.111, -0.111, -0.111,
        -0.111, -0.111, -0.111, -0.111,

        0.91, 0.91, 0.91, 0.91,
        0.91, 0.91, 0.91, 0.91,

        // delay index
        211,    137,    193,    199,
        101,    233,    173,    191,

    //  MonoDelayLowpassTapArray<combCount>  tapOutput;
        // coeff
        0.05, 0.05, 0.05, 0.05,
        0.05, 0.05, 0.05, 0.05,

        // low pass k
        0.90, 0.90, 0.90, 0.90,
        0.90, 0.90, 0.90, 0.90,

        // delay index
        771,    891,    929,    571,
        771,    891,    929,    571,
    };

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::EffectInstance )
                    ,   uint8_t( TagEffectType::FxLateReverb )
                    ,   uint8_t( TagEffectFxLateReverbMode::SetParametersMode01 )
                    );

    msgBuffer.setPar(   EffectInstance_FxLateReverb );

    msgBuffer.getTargetData(mode01_Housholder);
    sysman->evalMessage(msgBuffer);

    if( msgBuffer.messageType == yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Reverb ok" << std::endl;
    } else {
        std::cout << "---- Reverb error " <<uint16_t(msgBuffer.messageType) << std::endl;
        exit(-1);
    }
// ------------------------------------------------------------
    std::cout << "\n---- Early Reflection \n" << std::endl;

    FxEarlyReflectionParam::Mode01 fxEarlyReflection_mode01 = {
    // delayLateReverb
    1024, 1024,
    // delaysEarlyreflection
    3809,   3809,
    4777,   4477,
    5547,   5947,
    6977,   6177,
    7109,   7809,
    8877,   8177,
    9147,   9947,
    10977,  10977,
    11809,  11809,
    12777,   12477,
    13547,   13947,
    14977,   14177,
    15109,   15809,
    16877,   16177,
    17147,   17947,
    23000,  23000,
    // coeffsEarlyreflection
    0.8000000, 0.7200000,
    0.6400000, 0.5760000,
    0.5120000, 0.4608000,
    0.4096000, 0.3686400,
    0.3276800, 0.2949120,
    0.2621440, 0.2359296,
    0.2097152, 0.1887437,
    0.1677722, 0.1509950,
    0.1342178, 0.1207960,
    0.1073742, 0.0966368,
    0.0858994, 0.0773094,
    0.0687195, 0.0618475,
    0.0549756, 0.0494780,
    0.0439805, 0.0395824,
    0.0351844, 0.0316659,
    0.0281475, 0.0253328,

    0.7693853, 0.7260897,
    0.6155083, 0.5808718,
    0.4924066, 0.4646974,
    0.3939253, 0.3717580,
    0.3151402, 0.2974064,
    0.2521122, 0.2379251,
    0.2016897, 0.1903401,
    0.1613518, 0.1522721,
    0.1290814, 0.1218177,
    0.1032652, 0.0974541,
    0.0826121, 0.0779633,
    0.0660897, 0.0623706,
    0.0528718, 0.0498965,
    0.0422974, 0.0399172,
    0.0338379, 0.0319338,
    0.0270703, 0.0255470,

    0.7434315, 0.7434315,
    0.5947452, 0.5947452,
    0.4757962, 0.4757962,
    0.3806370, 0.3806370,
    0.3045096, 0.3045096,
    0.2436077, 0.2436077,
    0.1948861, 0.1948861,
    0.1559089, 0.1559089,
    0.1247271, 0.1247271,
    0.0997817, 0.0997817,
    0.0798254, 0.0798254,
    0.0638603, 0.0638603,
    0.0510882, 0.0510882,
    0.0408706, 0.0408706,
    0.0326965, 0.0326965,
    0.0261572, 0.0261572,

    0.7260897, 0.7693853,
    0.5808718, 0.6155083,
    0.4646974, 0.4924066,
    0.3717580, 0.3939253,
    0.2974064, 0.3151402,
    0.2379251, 0.2521122,
    0.1903401, 0.2016897,
    0.1522721, 0.1613518,
    0.1218177, 0.1290814,
    0.0974541, 0.1032652,
    0.0779633, 0.0826121,
    0.0623706, 0.0660897,
    0.0498965, 0.0528718,
    0.0399172, 0.0422974,
    0.0319338, 0.0338379,
    0.0255470, 0.0270703,

    0.7200000, 0.8000000,
    0.5760000, 0.6400000,
    0.4608000, 0.5120000,
    0.3686400, 0.4096000,
    0.2949120, 0.3276800,
    0.2359296, 0.2621440,
    0.1887437, 0.2097152,
    0.1509950, 0.1677722,
    0.1207960, 0.1342178,
    0.0966368, 0.1073742,
    0.0773094, 0.0858994,
    0.0618475, 0.0687195,
    0.0494780, 0.0549756,
    0.0395824, 0.0439805,
    0.0316659, 0.0351844,
    0.0253328, 0.0281475,

    0.7260897, 0.8306147,
    0.5808718, 0.6644918,
    0.4646974, 0.5315934,
    0.3717580, 0.4252748,
    0.2974064, 0.3402198,
    0.2379251, 0.2721759,
    0.1903401, 0.2177407,
    0.1522721, 0.1741926,
    0.1218177, 0.1393541,
    0.0974541, 0.1114832,
    0.0779633, 0.0891866,
    0.0623706, 0.0713493,
    0.0498965, 0.0570794,
    0.0399172, 0.0456635,
    0.0319338, 0.0365308,
    0.0255470, 0.0292247,

    0.7434315, 0.8565685,
    0.5947452, 0.6852548,
    0.4757962, 0.5482039,
    0.3806370, 0.4385631,
    0.3045096, 0.3508505,
    0.2436077, 0.2806804,
    0.1948861, 0.2245443,
    0.1559089, 0.1796355,
    0.1247271, 0.1437084,
    0.0997817, 0.1149667,
    0.0798254, 0.0919734,
    0.0638603, 0.0735787,
    0.0510882, 0.0588630,
    0.0408706, 0.0470904,
    0.0326965, 0.0376723,
    0.0261572, 0.0301378,

    0.7693853, 0.8739104,
    0.6155083, 0.6991283,
    0.4924066, 0.5593027,
    0.3939253, 0.4474421,
    0.3151402, 0.3579537,
    0.2521122, 0.2863630,
    0.2016897, 0.2290904,
    0.1613518, 0.1832723,
    0.1290814, 0.1466178,
    0.1032652, 0.1172943,
    0.0826121, 0.0938354,
    0.0660897, 0.0750683,
    0.0528718, 0.0600547,
    0.0422974, 0.0480437,
    0.0338379, 0.0384350,
    0.0270703, 0.0307480,

    0.8000000, 0.8800001,
    0.6400000, 0.7040001,
    0.5120000, 0.5632001,
    0.4096000, 0.4505601,
    0.3276800, 0.3604481,
    0.2621440, 0.2883584,
    0.2097152, 0.2306868,
    0.1677722, 0.1845494,
    0.1342178, 0.1476395,
    0.1073742, 0.1181116,
    0.0858994, 0.0944893,
    0.0687195, 0.0755914,
    0.0549756, 0.0604732,
    0.0439805, 0.0483785,
    0.0351844, 0.0387028,
    0.0281475, 0.0309623,

    0.8306147, 0.8739104,
    0.6644918, 0.6991283,
    0.5315934, 0.5593027,
    0.4252748, 0.4474421,
    0.3402198, 0.3579537,
    0.2721759, 0.2863630,
    0.2177407, 0.2290904,
    0.1741926, 0.1832723,
    0.1393541, 0.1466178,
    0.1114832, 0.1172943,
    0.0891866, 0.0938354,
    0.0713493, 0.0750683,
    0.0570794, 0.0600547,
    0.0456635, 0.0480437,
    0.0365308, 0.0384350,
    0.0292247, 0.0307480,

    0.8565685, 0.8565685,
    0.6852548, 0.6852548,
    0.5482039, 0.5482039,
    0.4385631, 0.4385631,
    0.3508505, 0.3508505,
    0.2806804, 0.2806804,
    0.2245443, 0.2245443,
    0.1796355, 0.1796355,
    0.1437084, 0.1437084,
    0.1149667, 0.1149667,
    0.0919734, 0.0919734,
    0.0735787, 0.0735787,
    0.0588630, 0.0588630,
    0.0470904, 0.0470904,
    0.0376723, 0.0376723,
    0.0301378, 0.0301378,

    0.8739104, 0.8306147,
    0.6991283, 0.6644918,
    0.5593027, 0.5315934,
    0.4474421, 0.4252748,
    0.3579537, 0.3402198,
    0.2863630, 0.2721759,
    0.2290904, 0.2177407,
    0.1832723, 0.1741926,
    0.1466178, 0.1393541,
    0.1172943, 0.1114832,
    0.0938354, 0.0891866,
    0.0750683, 0.0713493,
    0.0600547, 0.0570794,
    0.0480437, 0.0456635,
    0.0384350, 0.0365308,
    0.0307480, 0.0292247,

    0.8800001, 0.8000000,
    0.7040001, 0.6400000,
    0.5632001, 0.5120000,
    0.4505601, 0.4096000,
    0.3604481, 0.3276800,
    0.2883584, 0.2621440,
    0.2306868, 0.2097152,
    0.1845494, 0.1677722,
    0.1476395, 0.1342178,
    0.1181116, 0.1073742,
    0.0944893, 0.0858994,
    0.0755914, 0.0687195,
    0.0604732, 0.0549756,
    0.0483785, 0.0439805,
    0.0387028, 0.0351844,
    0.0309623, 0.0281475,

    0.8739104, 0.7693853,
    0.6991283, 0.6155083,
    0.5593027, 0.4924066,
    0.4474421, 0.3939253,
    0.3579537, 0.3151402,
    0.2863630, 0.2521122,
    0.2290904, 0.2016897,
    0.1832723, 0.1613518,
    0.1466178, 0.1290814,
    0.1172943, 0.1032652,
    0.0938354, 0.0826121,
    0.0750683, 0.0660897,
    0.0600547, 0.0528718,
    0.0480437, 0.0422974,
    0.0384350, 0.0338379,
    0.0307480, 0.0270703,

    0.8565685, 0.7434314,
    0.6852548, 0.5947452,
    0.5482039, 0.4757961,
    0.4385631, 0.3806369,
    0.3508505, 0.3045095,
    0.2806804, 0.2436076,
    0.2245443, 0.1948861,
    0.1796355, 0.1559089,
    0.1437084, 0.1247271,
    0.1149667, 0.0997817,
    0.0919734, 0.0798253,
    0.0735787, 0.0638603,
    0.0588630, 0.0510882,
    0.0470904, 0.0408706,
    0.0376723, 0.0326965,
    0.0301378, 0.0261572,

    0.8306147, 0.7260897,
    0.6644918, 0.5808718,
    0.5315934, 0.4646974,
    0.4252748, 0.3717580,
    0.3402198, 0.2974064,
    0.2721759, 0.2379251,
    0.2177407, 0.1903401,
    0.1741926, 0.1522721,
    0.1393541, 0.1218177,
    0.1114832, 0.0974541,
    0.0891866, 0.0779633,
    0.0713493, 0.0623706,
    0.0570794, 0.0498965,
    0.0456635, 0.0399172,
    0.0365308, 0.0319338,
    0.0292247, 0.0255470,
    // modulatorPeriod
    5, 7, 9, 11,
    13, 16, 17, 19,
    23, 29, 31, 37,
    41, 43, 47, 51,


    };

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::EffectInstance )
                    ,   uint8_t( TagEffectType::FxEarlyReflection )
                    ,   uint8_t( TagEffectFxEarlyReflectionMode::SetParametersMode01 )
                    );

    msgBuffer.setPar( EffectInstance_FxEarlyReflection );

    msgBuffer.getTargetData( fxEarlyReflection_mode01 );
    sysman->evalMessage(msgBuffer);

    if( msgBuffer.messageType == yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Early Reflection ok" << std::endl;
    } else {
        std::cout << "---- Early Reflection error " <<uint16_t(msgBuffer.messageType) << std::endl;
        exit(-1);
    }

// ------------------------------------------------------------
    std::cout << "\n---- SetOvertoneCOunt \n" << std::endl;

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::ToneShaper )
                    ,   uint8_t( TagToneShaper::SetOvertoneCount )
                    );
    msgBuffer.setPar( 0, 12 ); // number of overtones of vector 0

    sysman->evalMessage(msgBuffer);

    if( msgBuffer.messageType == yaxp::MessageT::responseSetOK ) {
        std::cout << "---- SetOvertoneCOunt ok" << std::endl;
    } else {
        std::cout << "---- SetOvertoneCOunt error " <<uint16_t(msgBuffer.messageType) << std::endl;
        exit(-1);
    }

#if 0
    for( auto i=100.f; i<5000.0f; i += 200.0f  ) {
        std::cout
            << "---- low pass i:" << i
            << " k:" << f2FilterOnePole_F( i )
            << std::endl;

    }
#endif
// ------------------------------------------------------------
    std::cout << "\n---- Echo \n" << std::endl;

    FxEchoParam fxEchoParam = {
        .tapOutput = {

            // output
//                  AA      AB      BA      BB
            V4vf(   0.9f,   0.0f,   0.0f,   -0.9f ),
            V4vf(   0.0f,   0.0f,   0.0f,   0.0f ),
            V4vf(   0.0f,   0.0f,   0.0f,   0.0f ),
            V4vf(   0.0f,   0.0f,   0.0f,   0.0f ),

            V4vf(   0.0f,   0.0f,   0.0f,   0.0f ),
            V4vf(   0.0f,   0.0f,   0.0f,   0.0f ),
            V4vf(   0.0f,   0.0f,   0.0f,   0.0f ),
            V4vf(   0.0f,   0.0f,   0.0f,   0.0f ),

            2048, 2048,
            2048, 2048,
            2048, 2048,
            2048, 2048,

            2048, 2048,
            2048, 2048,
            2048, 2048,
            2048, 2048,
        },

        .tapFeedback = {

            // feedback
//                  AA      AB      BA      BB
            V4vf(   0.0f,   0.8f,   0.6f,   0.0f),
            V4vf(   0.0f,   0.0f,   0.0f,   0.0f),
            V4vf(   0.0f,   0.0f,   0.0f,   0.0f),
            V4vf(   0.0f,   0.0f,   0.0f,   0.0f),

            V4vf(   0.0f,   0.0f,   0.0f,   0.0f),
            V4vf(   0.0f,   0.0f,   0.0f,   0.0f),
            V4vf(   0.0f,   0.0f,   0.0f,   0.0f),
            V4vf(   0.0f,   0.0f,   0.0f,   0.0f),

            65536+64, 65536+64,
            2048, 2048,
            2048, 2048,
            2048, 2048,

            2048, 2048,
            2048, 2048,
            2048, 2048,
            2048, 2048,
        },

        .tapOutputLP = {

            // output
//                  AA      AB      BA      BB
            V4vf(   0.1f,   0.1f,   -0.1f,   0.1f),
            V4vf(   0.0f,   0.0f,   0.0f,   0.0f),
            V4vf(   0.0f,   0.0f,   0.0f,   0.0f),
            V4vf(   0.0f,   0.0f,   0.0f,   0.0f),

            V4vf(   0.0f,   0.0f,   0.0f,   0.0f),
            V4vf(   0.0f,   0.0f,   0.0f,   0.0f),
            V4vf(   0.0f,   0.0f,   0.0f,   0.0f),
            V4vf(   0.0f,   0.0f,   0.0f,   0.0f),

            // low pass
            V4vf(   0.89f,  0.89f,  0.89f,  0.89f),
            V4vf(   0.9f,   0.9f,   0.9f,   0.9f),
            V4vf(   0.9f,   0.9f,   0.9f,   0.9f),
            V4vf(   0.9f,   0.9f,   0.9f,   0.9f),

            V4vf(   0.9f,   0.9f,   0.9f,   0.9f),
            V4vf(   0.9f,   0.9f,   0.9f,   0.9f),
            V4vf(   0.9f,   0.9f,   0.9f,   0.9f),
            V4vf(   0.9f,   0.9f,   0.9f,   0.9f),

            2048, 2048,
            2048, 2048,
            2048, 2048,
            2048, 2048,

            2048, 2048,
            2048, 2048,
            2048, 2048,
            2048, 2048,
        },
        .tapFeedbackLP = {

            // feedback
//                  AA      AB      BA      BB
            V4vf(   0.1f,   0.0f,   0.0f,   0.1f),
            V4vf(   0.0f,   0.0f,   0.0f,   0.0f),
            V4vf(   0.0f,   0.0f,   0.0f,   0.0f),
            V4vf(   0.0f,   0.0f,   0.0f,   0.0f),

            V4vf(   0.0f,   0.0f,   0.0f,   0.0f),
            V4vf(   0.0f,   0.0f,   0.0f,   0.0f),
            V4vf(   0.0f,   0.0f,   0.0f,   0.0f),
            V4vf(   0.0f,   0.0f,   0.0f,   0.0f),

            // low pass
            V4vf(   0.8f,   0.8f,   0.8f,   0.8f),
            V4vf(   0.9f,   0.9f,   0.9f,   0.9f),
            V4vf(   0.9f,   0.9f,   0.9f,   0.9f),
            V4vf(   0.9f,   0.9f,   0.9f,   0.9f),

            V4vf(   0.9f,   0.9f,   0.9f,   0.9f),
            V4vf(   0.9f,   0.9f,   0.9f,   0.9f),
            V4vf(   0.9f,   0.9f,   0.9f,   0.9f),
            V4vf(   0.9f,   0.9f,   0.9f,   0.9f),

            65536+64, 65536+64,
            65536, 65536,
            2048, 2048,
            2048, 2048,

            2048, 2048,
            2048, 2048,
            2048, 2048,
            2048, 2048,
        },

        .tapOutputCount=0,
        .tapFeedbackCount=0,
        .tapOutputLPCount=1,
        .tapFeedbackLPCount=1,

        .dry={0.0f,0.0f},
    };

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::EffectInstance )
                    ,   uint8_t( TagEffectType::FxEcho )
                    ,   uint8_t( TagEffectFxEchoMode::SetParameters )
                    );

    msgBuffer.setPar( EffectInstance_FxEcho );

    msgBuffer.getTargetData( fxEchoParam );

    sysman->evalMessage(msgBuffer);

    if( msgBuffer.messageType == yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Echo ok" << std::endl;
    } else {
        std::cout << "---- Echo error " <<uint16_t(msgBuffer.messageType) << std::endl;
        exit(-1);
    }

// ------------------------------------------------------------
    std::cout << "\n---- Chorus \n" << std::endl;

    FxChorusParam::Mode01 chorusParam = {

        .baseDelay  = 2048LL<<32,
        .wetgain  =  0.4,
        .sineRange = 1<<23,     // max
        .noiseRange = 1<<30,    // max

        .depthIndex         = { 0 },
        .deltaPhaseIndex    = { InnerController::CC_LFO_MASTER_DELTA_PHASE_BEGIN },
        .phaseDiffIndex     = { InnerController::CC_LFO_SLAVE_DELTA_PHASE_BEGIN },
        .oscMasterIndex     = { InnerController::CC_LFO_MASTER_PHASE_BEGIN },
        .oscSlaveIndex      = { InnerController::CC_LFO_SLAVE_PHASE_BEGIN },

        .basaeDepthNoiseExp = 5,
        .tapCount   = 4,


    };

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::EffectInstance )
                    ,   uint8_t( TagEffectType::FxChorus )
                    ,   uint8_t( TagEffectFxChorusMode::SetParametersMode01 )
                    );

    msgBuffer.setPar( EffectInstance_FxChorus );

    msgBuffer.getTargetData( chorusParam );

    sysman->evalMessage(msgBuffer);

    if( msgBuffer.messageType == yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Chorus ok" << std::endl;
    } else {
        std::cout << "---- Chorus error " <<uint16_t(msgBuffer.messageType) << std::endl;
        exit(-1);
    }


// ------------------------------------------------------------
    std::cout << "\n---- Flanger \n" << std::endl;

    FxFlangerParam::Mode01 flangerParam = {

        .baseDelay      = 65LL<<32,
        .gain           = 0.7,
        .feedbackGain   = 0.7,
        .depth          = 1<<27,    // limit

        .feedbackIndex      = { 0 }, // TODO
        .depthIndex         = { 0 }, // TODO
        .deltaPhaseIndex    = { InnerController::CC_LFO_MASTER_DELTA_PHASE_BEGIN },
        .phaseDiffIndex     = { InnerController::CC_LFO_SLAVE_DELTA_PHASE_BEGIN },
        .oscMasterIndex     = { InnerController::CC_LFO_MASTER_PHASE_BEGIN },
        .oscSlaveIndex      = { InnerController::CC_LFO_SLAVE_PHASE_BEGIN },

    };

    flangerParam.deltaPhaseIndex.setInnerValue( 0x300000 );
    flangerParam.phaseDiffIndex.setInnerValue( 0x40000000 ); // cos

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::EffectInstance )
                    ,   uint8_t( TagEffectType::FxFlanger )
                    ,   uint8_t( TagEffectFxFlangerMode::SetParametersMode01 )
                    );

    msgBuffer.setPar( EffectInstance_FxFlanger );

    msgBuffer.getTargetData( flangerParam );

    sysman->evalMessage(msgBuffer);

    if( msgBuffer.messageType == yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Flanger ok" << std::endl;
    } else {
        std::cout << "---- Flanger error " <<uint16_t(msgBuffer.messageType) << std::endl;
        exit(-1);
    }

// ------------------------------------------------------------
    std::cout << "\n---- Filter ap - phaser \n" << std::endl;

    FxFilterParam::Mode_2ch_x4ap_phaser_mode01 mode_2ch_x4ap_phaser_mode01 = {

        // TODO
        .feedbackGainIndex          = { InnerController::CC_PHASER_FEEDBACK_CTRL },
        .wetDryGainIndex            = { InnerController::CC_PHASER_WETDRY_CTRL },

        .bandWidhthIndex            = { InnerController::CC_PHASER_BANDWIDTH_CTRL },
        // 2xLFO control
        .deltaPhaseControlIndex     = { InnerController::CC_PHASER_LFO_FREQ_CTRL },
        .phaseDiff00ControlIndex    = { InnerController::CC_PHASER_LFO_PHASEDIFF_CTRL },
        // LFO delta phase
        .deltaPhaseIndex            = { InnerController::CC_LFO_MASTER_DELTA_PHASE_BEGIN },
        .phaseDiff00Index           = { InnerController::CC_LFO_SLAVE_DELTA_PHASE_BEGIN },
        // LFO phase
        .oscMasterIndex             = { InnerController::CC_LFO_MASTER_PHASE_BEGIN },
        .oscSlave00Index            = { InnerController::CC_LFO_SLAVE_PHASE_BEGIN },

        .oscFreqMapper = {
            .slope = 1<<(27-7),
            .shift = 0,
            .y0 = {
                int32_t(freq2ycent(0.1))
            }
         },

        .bandwidthMapper = { // ???
            .slope = 1<<20,
            .shift = 0,
            .y0 = {
                int32_t(freq2ycent(100))
            }
         },

        .notchMapper = {
            .slope = 1<<9,
            .shift = 0,
            .y0 = {

                int32_t(freq2ycent(200.0)),
                int32_t(freq2ycent(200.0)),
                int32_t(freq2ycent(200.0)),
                int32_t(freq2ycent(200.0)),

                int32_t(freq2ycent(200.0)),
                int32_t(freq2ycent(200.0)),
                int32_t(freq2ycent(200.0)),
                int32_t(freq2ycent(200.0))
#if 0
                int32_t(freq2ycent(300.0)),
                int32_t(freq2ycent(600.0)),
                int32_t(freq2ycent(1200.0)),
                int32_t(freq2ycent(2400.0)),

                int32_t(freq2ycent(200.0)),
                int32_t(freq2ycent(400.0)),
                int32_t(freq2ycent(800.0)),
                int32_t(freq2ycent(1600.0))
#endif
            }
         },

    };

    mode_2ch_x4ap_phaser_mode01.deltaPhaseIndex.setInnerValue( freq2deltaPhaseControlLfo(0.5) ); // 1Hz
    mode_2ch_x4ap_phaser_mode01.phaseDiff00Index.setInnerValue( 0x40000000 ); // cos
    mode_2ch_x4ap_phaser_mode01.bandWidhthIndex.setInnerValue( 0 ); // cos

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::EffectInstance )
                    ,   uint8_t( TagEffectType::FxFilter )
                    ,   uint8_t( TagEffectFxFilterMode::SetMode_2ch_x4ap_phaser_mode01 )
                    );

    msgBuffer.setPar( EffectInstance_FxFilter );
    msgBuffer.getTargetData( mode_2ch_x4ap_phaser_mode01 );

    sysman->evalMessage(msgBuffer);

    if( msgBuffer.messageType == yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Filter ap - phaser ok" << std::endl;
    } else {
        std::cout << "---- Filter ap - phaser error " <<uint16_t(msgBuffer.messageType) << std::endl;
        exit(-1);
    }

    // ------------------------------------------------------------
//  3 e0 0000
//  2 d0 0000
// 00 00 0000

// ------------------------------------------------------------
    std::cout << "\n---- Filter SVF \n" << std::endl;

    FxFilterParam::Mode_SVF01_2ch mode_SVF01_2ch = {
        .fControlIndex = { InnerController::CC_FILTER_FREQ0 },
        .qControlIndex = { InnerController::CC_FILTER_Q0 },
        .fMapper = {
            .slope = 1 << (27-6),
            .shift = 0,
            .y0 = { 0x12000000 } // 2x oversamplng 1 octave lower
        },

    };

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::EffectInstance )
                    ,   uint8_t( TagEffectType::FxFilter )
                    ,   uint8_t( TagEffectFxFilterMode::SetMode_SVF01_2ch_mode01 )
                    );

    msgBuffer.setPar( EffectInstance_FxFilter );
    msgBuffer.getTargetData( mode_SVF01_2ch );

    sysman->evalMessage(msgBuffer);

    if( msgBuffer.messageType == yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Filter SVF ok" << std::endl;
    } else {
        std::cout << "---- Filter SVF error " <<uint16_t(msgBuffer.messageType) << std::endl;
        exit(-1);
    }

    // ------------------------------------------------------------
    std::cout << "\n---- Filter 4p \n" << std::endl;

    /*
    staring at 40 Hz - octave 22
            .y0 = { 0x16000000 }

     */
    FxFilterParam::Mode_4p_2ch mode_4p_2ch = {
        .fControlIndex = { InnerController::CC_FILTER_FREQ0 },
        .qControlIndex = { InnerController::CC_FILTER_Q0 },
        .fMapper = {
            .slope = 1 << 20,
            .shift = 0,
            .y0 = { 0x16000000 }
        },

    };

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::EffectInstance )
                    ,   uint8_t( TagEffectType::FxFilter )
                    ,   uint8_t( TagEffectFxFilterMode::SetMode_4p_2ch )
                    );

    msgBuffer.setPar( EffectInstance_FxFilter );
    msgBuffer.getTargetData( mode_4p_2ch );

    sysman->evalMessage(msgBuffer);

    if( msgBuffer.messageType == yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Filter 4p ok" << std::endl;
    } else {
        std::cout << "---- Filter 4p error " <<uint16_t(msgBuffer.messageType) << std::endl;
        exit(-1);
    }

// ------------------------------------------------------------
}



// --------------------------------------------------------------------

void teststuff(void)
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
    uint16_t            port( yaxp::defaultPort ); // from param
    struct sigaction sigact;
    memset (&sigact, '\0', sizeof(sigact));
    static_assert( 8 == sizeof(Yamsgrt), "sizeof(Yamsgrt) must be 8" );
    static_assert( 0x193b0973 == refA440ycent, "refA440ycent must be 0x193b0973" );
    if( ! initialize() ) {
        exit(-1);
    }

    const char *homedir = getenv("HOME");

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
    Server              uiServer( *sysman, port );

    //
    // authentication
    // const char * const confDir  = ".yacynth";
    // const char * const seedFile = ".yaxp.seed";
    // open $HOME/.yacynth/.yaxp.seed
    // read and uiServer.setAuthSeed( );
    //

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

//    generator_FxEarlyReflectionParam(1.0f);
//    exit(0);



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

        yaxp::Message yms;

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
        std::cout << "\n\n============END==============\n\n" << std::endl;
        jack.mute();
        synthFe->stop();
        synthFrontendThread.join();
        jack.shutdown();
    } catch (...) {
        jack.mute();
        synthFe->stop();
        jack.shutdown();
        return -1;
    }
//    synthFrontendThread.join();
//-----------------------------------------------------------
    return 0;
};


//-----------------------------------------------------------

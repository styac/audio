/*
 * Copyright (C) 2017 ist
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
 * File:   Presets.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on October 2, 2017, 9:38 PM
 */
// #include "Presets.h"
#include "yacynth_config.h"

#include "oscillator/Tables.h"
#include "oscillator/Oscillator.h"
#include "oscillator/OscillatorArray.h"
#include "oscillator/ToneShaper.h"
#include "control/Controllers.h"
#include "control/Sysman.h"
#include "TuningConst.h"

using namespace yacynth;
using namespace TagMainLevel_00;
using namespace TagToneShaperLevel_01;

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
using namespace Tuning;

void preset0( Sysman  * sysman )
{
    yaxp::Message msgBuffer;

#if 0
    std::cout << "\n---------------------------- test factory Effect begin" << std::endl;
    // test factory
    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::DeleteEffects )
                    );
    sysman->evalMessage(msgBuffer);
    if( msgBuffer.messageType != yaxp::MessageT::responseSetOK ) {
        std::cout << "---- DeleteEffects Effect error " << uint16_t(msgBuffer.messageType) << std::endl;
        //exit(-1);
    }

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::CreateEffect )
                    );
    msgBuffer.setPar( uint16_t(TagEffectType::FxEarlyReflection) ); 
    sysman->evalMessage(msgBuffer);
    if( msgBuffer.messageType != yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Create Effect error " << uint16_t(msgBuffer.messageType) << std::endl;
        //exit(-1);
    }

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::DeleteEffects )
                    );
    sysman->evalMessage(msgBuffer);
    if( msgBuffer.messageType != yaxp::MessageT::responseSetOK ) {
        std::cout << "---- DeleteEffects Effect error " << uint16_t(msgBuffer.messageType) << std::endl;
        //exit(-1);
    }

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::CreateEffect )
                    );
    msgBuffer.setPar( uint16_t(TagEffectType::FxEarlyReflection) ); 
    sysman->evalMessage(msgBuffer);
    if( msgBuffer.messageType != yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Create Effect error " << uint16_t(msgBuffer.messageType) << std::endl;
        //exit(-1);
    }

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::CreateEffect )
                    );
    msgBuffer.setPar( uint16_t(TagEffectType::FxEarlyReflection) ); 
    sysman->evalMessage(msgBuffer);
    if( msgBuffer.messageType != yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Create Effect error " << uint16_t(msgBuffer.messageType) << std::endl;
        //exit(-1);
    }

    std::cout << "\n---------------------------- test factory Effect end" << std::endl;
#endif

    ToneShaper ts;
    ts.clear();
    ts.pitch = 0;
    ts.sustain.decayCoeff.setPar( 2000, 0 );
    ts.sustain.modDepth  = 0;     // 100 / 256
    ts.sustain.modDeltaPhase = 0; //300;
    ts.tickFrameRelease.setPar( 100, 0, 2 );
    ts.transient[ 2 ].tickFrame.setPar( 5, 0, 2 );
    ts.transient[ 2 ].targetValue = uint32_t( 32000.0 * 65535.0 );
    ts.transient[ 1 ].tickFrame.setPar( 100, 0, 1 );
    ts.transient[ 1 ].targetValue = uint32_t( 7000.0 * 65535.0 );

    ts.oscillatorType = ToneShaper::OSC_SIN; // OSC_SIN;
    ts.detune2CH = 10<<12;
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
    int32_t overtoneDetune = 10000;
    constexpr   int overToneCount = 10;
    for( auto vi=1u; vi < overToneCount; ++vi ) {
//        const float onevi = 1.0f/float(vi+1);
//        ts.pitch = relFreq2pitch( vi+1 );
//        const double onevi = 1.0/double( vi );
        const double onevi = 1.0/double( vi + 1 );
        ts.pitch = interval2ycent( vi + 1 ) + overtoneDetune * vi;
        ts.sustain.decayCoeff.setPar( 1000, 0 );
        ts.sustain.modDepth  = 0;     // 100 / 256
        ts.sustain.modDeltaPhase = 0; // 300;
        ts.tickFrameRelease.setPar( 100, 0, 2 );
        ts.transient[ 2 ].tickFrame.setPar( 2, 0, 2 );
        ts.transient[ 2 ].targetValue = uint32_t( 52000.0 * 65535.0 * onevi );
        ts.transient[ 1 ].tickFrame.setPar( 100, 0, 1 );
        ts.transient[ 1 ].targetValue = uint32_t( 6000.0 * 65535.0 * onevi );

        ts.oscillatorType = ToneShaper::OSC_SIN;
        if( vi & 1) {
            ts.detune2CH = 127<<4;            
        } else {
            ts.detune2CH =-(127<<4) ;            
        }
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
                << " dynamic "          << uint16_t(data->dynamic)
                << " maxMode "          << uint16_t(data->fxMaxMode)
                << " inputCount "       << uint16_t(data->inputCount )
                << " refId "            << uint16_t(data->refId)
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
    
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

    const uint8_t EffectInstance_EndMixer           = effectMap["EndMixer.00:00"];
    const uint8_t EffectInstance_Input              = effectMap["Input.00:00"];
    const uint8_t EffectInstance_OscillatorMixer    = effectMap["OscillatorMixer.00:00"];
    
#pragma GCC diagnostic pop

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectRunner )
                    ,   uint8_t( TagEffectRunner::Preset )
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
                    ,   uint8_t( TagEffectFxMixerMode::Preset )
                    );
    msgBuffer.setPar( EffectInstance_EndMixer, 0 ); // channel
    sysman->evalMessage(msgBuffer);
    if( msgBuffer.messageType != yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Volume control Preset error " << uint16_t(msgBuffer.messageType) << std::endl;
    }

    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectCollector )
                    ,   uint8_t( TagEffectCollector::EffectInstance )
                    ,   uint8_t( TagEffectType::FxMixer )
                    ,   uint8_t( TagEffectFxMixerMode::SetVolumeControllerIndex )
                    );

    msgBuffer.setPar( EffectInstance_EndMixer, 0  );// channel
    uint16_t cindex0 = InnerController::CC_MAINVOLUME;
    msgBuffer.getTargetData( cindex0 );
    sysman->evalMessage(msgBuffer);
    if( msgBuffer.messageType != yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Volume control index0 error " << uint16_t(msgBuffer.messageType) << std::endl;
    }    
#if 0    
    // test FxInput
    EffectRunnerSetConnections  effectRunnerSetConnections[] = {
        { EffectInstance_Input,  0, 0 },
    };
    msgBuffer.clear();
    msgBuffer.setTags(  uint8_t( TagMain::EffectRunner )
                    ,   uint8_t( TagEffectRunner::SetConnections )
                    );
    msgBuffer.setPar(cArrayElementCount(effectRunnerSetConnections));
    msgBuffer.getTargetData(effectRunnerSetConnections);
    sysman->evalMessage(msgBuffer);
    if( msgBuffer.messageType != yaxp::MessageT::responseSetOK ) {
        std::cout << "---- Input connect error " << uint16_t(msgBuffer.messageType) << std::endl;
    }
#endif    
    
}

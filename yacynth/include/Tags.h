#pragma once

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
 * File:   Tags.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on October 3, 2016, 9:58 PM
 */

#include    <cstdint>
#include    <string>
#include    <array>
#include    <atomic>
#include    <cstring>

#define TAG_DEBUG_ON 1

#ifdef TAG_DEBUG_ON

#define TAG_DEBUG( tagname, tagi, pari, comment )\
std::cout \
    << "tag: " << #tagname \
    << " " << int16_t(tagname) \
    << " t-ind: " << int16_t(tagi)\
    << " p-ind: " << int16_t(pari) \
    << "  " << comment \
    << std::endl;
#else
#define TAG_DEBUG( tagname, tag, tagc, comment );
#endif


namespace yacynth {

struct EffectListEntry
{
    static constexpr const char * const typeName = "EffectListEntry";
    static constexpr const char * const fxIndexName = "fxIndex";
    static constexpr const char * const idName = "id";
    static constexpr const char * const fxTypeName = "fxType";
    static constexpr const char * const fxMaxModeName = "fxMaxMode";
    static constexpr const char * const inputCountName = "inputCount";
    static constexpr const char * const masterIdName = "masterId";
    static constexpr const char * const nameIdName = "name";
    
    static constexpr uint8_t nameLength = 32;
    uint8_t fxIndex;
    uint8_t id;
    uint8_t fxType;
    uint8_t fxMaxMode;
    uint8_t inputCount;
    uint8_t masterId;
    char    name[nameLength];
};

// level 0

namespace TagMainLevel_00 {
    enum class TagMain : uint8_t {
        Nop,
        Clear,
        Mute,           // mute immediately
        UnMute,         // unmute
        ToneShaper,
        InnerController,
        MidiController,
        EffectRunner,
        EffectCollector,
        Router,
        Tuner,
        MidiMessage,    // direct MIDI
        Yamsg,          // Y real time message
        System,         // unused
    };
} // end namespace

namespace TagToneShaperLevel_01 {
    enum class TagToneShaper : uint8_t {
        Nop,
        Clear,
        // first set
        SetOvertone,                // parameter: ToneShaperIndex, OvertoneIndex
        SetOvertoneVector,          // parameter: ToneShaperIndex, count max 256 - size:184
        SetOvertoneCount,           // parameter: ToneShaperIndex, OvertoneCount
        SetPitchVector,             // parameter: ToneShaperIndex, count max 256 (OvertoneIndex 0..n-1)
        // then get
        GetOvertone,                // parameter: ToneShaperIndex, OvertoneIndex
        GetOvertoneVector,          // parameter: ToneShaperIndex, count max 256 - size:184
    };
} // end namespace

namespace TagEffectFactoryLevel_01 {
    enum class EffectFactory : uint8_t {
        Nop,
        Clear,
        Create,     // create a new effect
    };
} // end namespace

namespace TagEffectRunnerLevel_01 {
    enum class TagEffectRunner : uint8_t {
        Nop,
        Clear,
        Fill,
        SetConnections,
        GetEffectList,
    };

    // parameter structures
    struct EffectRunnerFill
    {
        static constexpr const char * const typeName = "EffectRunnerFill";        
        static constexpr const char * const fxIdOfFxCollectorName = "fxIdOfFxCollector";        
        uint8_t fxIdOfFxCollector;
    };

    struct EffectRunnerSetConnections
    {
        static constexpr const char * const typeName = "EffectRunnerSetConnections";        
        static constexpr const char * const fxIdOfFxCollectorOutputName = "fxIdOfFxCollectorOutput";        
        static constexpr const char * const fxIdOfFxRunnerInputName = "fxIdOfFxRunnerInput";        
        static constexpr const char * const inputIdName = "inputId";        
        uint8_t fxIdOfFxCollectorOutput;
        uint8_t fxIdOfFxRunnerInput;
        uint8_t inputId;
    };

} // end namespace


namespace TagMidiControllerLevel_01 {
    enum class TagMidiController : uint8_t {
        Nop,
        Clear,                      // clear all
        ClearChannelVector,         // parameter: channel : 0 .. channelCount-1
        SetChannelVector,           // parameter: channel + all controller of channel
        SetController,              // parameter: count of MidiSetting
    };

    struct MidiControllerSetting
    {
        static constexpr const char * const typeName = "MidiControllerSetting";         
        static constexpr const char * const initValueName = "initValue";         
        static constexpr const char * const channelName = "channel";         
        static constexpr const char * const midiCCName = "midiCC";         
        static constexpr const char * const midiModeName = "midiMode";         
        static constexpr const char * const innerIndexName = "innerIndex";         
        int32_t initValue;
        uint8_t channel;
        uint8_t midiCC;
        uint8_t midiMode;
        uint8_t innerIndex;
    };
} // end namespace

// to clarify
namespace TagInnerControllerLevel_01 {
    enum class TagInnerController : uint8_t {
        Nop,
        Clear,
        ClearController,            // parameter : controller number
        SetController,              // parameter : InnerControllerSetting
    };

    struct InnerControllerSetting
    {
        static constexpr const char * const typeName = "InnerControllerSetting";
        static constexpr const char * const valueName = "InnerControllerValue";
        static constexpr const char * const indexName = "InnerControllerIndex";
        int32_t     value;
        uint16_t    index;
        uint16_t    rfu;
    };
} // end namespace

namespace TagRouterLevel_01 {
    enum class TagRouter : uint8_t {
        Nop,
        Clear,
        SetToneBank,
    };
} // end namespace

namespace TagTunerLevel_01 {
    enum class TagTuner : uint8_t {
        Nop,
        Clear,
        SetTuneTable,       // main rate table: eq tuned, natural tuned
        SetTuneSubTable,    // sub table select from main table
    };
} // end namespace

namespace TagEffectCollectorLevel_01 {
    enum class TagEffectCollector : uint8_t {
        Nop,
        Clear,
        EffectInstance,             // parameter: index,
        SetProcessingMode,          // parameter: index, mode
        GetEffectList,              // parameter: -
    };
} // end namespace

//
// all Fx types from 0x40
// 0 .. 3F are the different other type
// used in DATABASE records
//
namespace TagEffectTypeLevel_02 {
    enum class TagEffectType : uint8_t {
        FxNop   = 0x40,
        FxNil,
        FxSlave,
        FxMixer,
        FxOscillatorMixer,
        FxModulator,
        FxOutNoise,
        FxOutOscillator,
        FxFilter,
        FxEcho,
        FxLateReverb,
        FxEarlyReflection,
        FxChorus,
        FxFlanger,
    };
} // end namespace
// /Effect/TagFxFilter/TagMode_01_ap22x4x  FxCollectorid, struct Mode_01_ap22x4x

// level 2

namespace TagEffectFxFilterModeLevel_03 {
    enum class TagEffectFxFilterMode : uint8_t {
        Nop,
        Clear,                      // clear all
        GetFeatures,
        SetMode_2ch_x4ap_phaser_mode01,
        SetMode_SVF01_2ch_mode01,
        SetMode_4p_2ch,


    };
} // end namespace

namespace TagEffectFxMixerModeLevel_03 {
    enum class TagEffectFxMixerMode : uint8_t {
        Nop,
        Clear,                      // clear all
        GetFeatures,
        SetParametersMode01,        // all param - must be implemented
        SetVolumeControllerIndex,
        SetVolumeRange,
    };
} // end namespace

namespace TagEffectFxOscillatorMixerModeLevel_03 {
    enum class TagEffectFxOscillatorMixerMode : uint8_t {
        Nop,
        Clear,                      // clear all
        GetFeatures,
        SetParametersMode01,
    };
} // end namespace

namespace TagEffectFxModulatorModeLevel_03 {
    enum class TagEffectFxModulatorMode : uint8_t {
        Nop,
        Clear,
        GetFeatures,
        SetParametersMode01,
//        SetMode_01_amplitudeModulation,
//        GetMode_01_amplitudeModulation,
    };
} // end namespace

namespace TagEffectFxOutNoiseModeLevel_03 {
    enum class TagEffectFxOutNoiseMode : uint8_t {
        Nop,
        Clear,                      // clear all
        GetFeatures,
        SetParametersMode01,
    };
} // end namespace

namespace TagEffectFxOutOscillatorModeLevel_03 {
    enum class TagEffectFxOutOscillatorMode : uint8_t {
        Nop,
        Clear,                      // clear all
        GetFeatures,
        SetParametersMode01,
    };
} // end namespace


namespace TagEffectFxEchoModeLevel_03 {
    enum class TagEffectFxEchoMode : uint8_t {
        Nop,
        Clear,                      // clear all
        GetFeatures,
        SetParametersMode01,              // all params
        SetTapOutput,               // select params
        SetTapFeedback,
        SetTapOutputLP,
        SetTapFeedbackLP,
        SetDryCoeffs,
        SetTapOutputCount,
        SetTapFeedbackCount,
        SetTapOutputLPCount,
        SetTapFeedbackLPCount,
    };
} // end namespace

namespace TagEffectFxLateReverbModeLevel_03 {
    enum class TagEffectFxLateReverbMode : uint8_t {
        Nop,
        Clear,                      // clear all
        GetFeatures,
        SetParametersMode01,
    };
} // end namespace

namespace TagEffectFxEarlyReflectionModeLevel_03 {
    enum class TagEffectFxEarlyReflectionMode : uint8_t {
        Nop,
        Clear,                      // clear all
        GetFeatures,
        SetParametersMode01,
    };
} // end namespace

namespace TagEffectFxChorusModeLevel_03 {
    enum class TagEffectFxChorusMode : uint8_t {
        Nop,
        Clear,                      // clear all
        GetFeatures,
        SetParametersMode01,
    };
} // end namespace

namespace TagEffectFxFlangerModeLevel_03 {
    enum class TagEffectFxFlangerMode : uint8_t {
        Nop,
        Clear,                      // clear all
        GetFeatures,
        SetParametersMode01,
    };
} // end namespace

namespace TagEffect__ModeLevel_03 {
    enum class TagEffect__Mode : uint8_t {
        Nop,
        Clear,                      // clear all
        GetFeatures,
        SetParametersMode01,
        SetMode_01___,
        GetMode_01___,
    };
} // end namespace


} // end namespace yacynth



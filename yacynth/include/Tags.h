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
#define TAG_DEBUG( tagname, tag, tagc, comment )
#endif


namespace yacynth {

struct  __attribute__ ((packed)) EffectListEntry
{
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
        Hello,          // connect
        Bye,            // disconnect
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

// level 1
#if 0
namespace TagSystemLevel_01 {
    enum class TagSystem : uint8_t {
        Nop,
        Mute,
        Unmute,
    };
} // end namespace
#endif

namespace TagToneShaperLevel_01 {
    enum class TagToneShaper : uint8_t {
        Nop,
        Clear,
        SetOvertoneCount,           // parameter: ToneShaperIndex, OvertoneCount
        SetOvertone,                // parameter: ToneShaperIndex, OvertoneIndex
        GetOvertone,                // parameter: ToneShaperIndex, OvertoneIndex
    };
} // end namespace

namespace TagEffectFactoryLevel_01 {
    enum class EffectFactory : uint8_t {
        Nop,
        Clear,
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
    struct  __attribute__ ((packed)) EffectRunnerFill
    {
        uint8_t fxIdOfFxCollector;
    };

    struct  __attribute__ ((packed)) EffectRunnerSetConnections
    {
        uint8_t fxIdOfFxCollector;
        uint8_t fxIdOfFxRunner;
        uint8_t inputIdOfFxRunner;
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

    struct  __attribute__ ((packed)) MidiSetting
    {
        uint8_t channel;
        uint8_t midiCC;
        uint8_t midiMode;
        uint8_t innerIndex;
        int32_t initValue;
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

    struct  __attribute__ ((packed)) InnerControllerSetting
    {
        uint16_t    index;
        int32_t     value;
    };
} // end namespace

namespace TagRouterLevel_01 {
    enum class TagRouter : uint8_t {
        Nop,
        Clear,
    };
} // end namespace

namespace TagTunerLevel_01 {
    enum class TagTuner : uint8_t {
        Nop,
        Clear,
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

namespace TagEffectTypeLevel_02 {
    enum class TagEffectType : uint8_t {
        Nop,
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
    };
} // end namespace
// /Effect/TagFxFilter/TagMode_01_ap22x4x  FxCollectorid, struct Mode_01_ap22x4x

// level 2

namespace TagEffectFxFilterModeLevel_03 {
    enum class TagEffectFxFilterMode : uint8_t {
        Nop,
        Clear,                      // clear all
        SetMode_01_ap4x,
        SetMode_02_ap4x,
        SetMode_03_ap4x,
        SetMode_04_ap4x,
        SetMode_05_ap4x,
        SetMode_06_ap4x,
        SetMode_07_ap4x,
        SetMode_08_ap4x,
    };
} // end namespace

namespace TagEffectFxMixerModeLevel_03 {
    enum class TagEffectFxMixerMode : uint8_t {
        Nop,
        Clear,                      // clear all
        SetVolumeControllerIndex,    
        SetVolumeRange,
    };
} // end namespace

namespace TagEffectFxOscillatorMixerModeLevel_03 {
    enum class TagEffectFxOscillatorMixerMode : uint8_t {
        Nop,
        Clear,                      // clear all
        SetMode_01___,
        GetMode_01___,
    };
} // end namespace

namespace TagEffectFxModulatorModeLevel_03 {
    enum class TagEffectFxModulatorMode : uint8_t {
        Nop,
        Clear,
        SetParameters,
//        SetMode_01_amplitudeModulation,
//        GetMode_01_amplitudeModulation,
    };
} // end namespace

namespace TagEffectFxOutNoiseModeLevel_03 {
    enum class TagEffectFxOutNoiseMode : uint8_t {
        Nop,
        Clear,                      // clear all
        SetParameters,
    };
} // end namespace

namespace TagEffectFxOutOscillatorModeLevel_03 {
    enum class TagEffectFxOutOscillatorMode : uint8_t {
        Nop,
        Clear,                      // clear all
        SetParameters,
    };
} // end namespace

namespace TagEffectFxEchoModeLevel_03 {
    enum class TagEffectFxEchoMode : uint8_t {
        Nop,
        Clear,                      // clear all
        SetParameters,
        SetMode_01___,
        GetMode_01___,
    };
} // end namespace

namespace TagEffectFxLateReverbModeLevel_03 {
    enum class TagEffectFxLateReverbMode : uint8_t {
        Nop,
        Clear,                      // clear all
        SetParametersMode01,
    };
} // end namespace

namespace TagEffectFxEarlyReflectionModeLevel_03 {
    enum class TagEffectFxEarlyReflectionMode : uint8_t {
        Nop,
        Clear,                      // clear all
        SetParametersMode01,
    };
} // end namespace

namespace TagEffect__ModeLevel_03 {
    enum class TagEffect__Mode : uint8_t {
        Nop,
        Clear,                      // clear all
        SetParameters,
        SetMode_01___,
        GetMode_01___,
    };
} // end namespace


} // end namespace yacynth



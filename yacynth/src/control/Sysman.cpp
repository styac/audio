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
 * File:   Sysman.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on February 21, 2016, 8:31 AM
 */

#include    "Sysman.h"
#include    "../control/global.h"

namespace yacynth {
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

// --------------------------------------------------------------------

Sysman::Sysman(
    OscillatorArray&    oscillatorArrayP,
    IOThread&           iOThreadP  )
:   oscillatorArray(    oscillatorArrayP )
,   iOThread(           iOThreadP )
,   toneShaperMatrix(   oscillatorArrayP.getToneShaperMatrix() )
{

} // end Sysman::Sysman

// --------------------------------------------------------------------
//
bool Sysman::evalMessage( yaxp::Message& message )
{
    std::string str;
    message.print(str);
    std::cout << "Sysman::evalMessage  " << str << std::endl;

    return parameter( message, 0, 0 );
}

// --------------------------------------------------------------------
bool Sysman::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    const uint8_t tag = message.getTag(tagIndex);
    switch( TagMain( tag ) ) {
    case TagMain::ToneShaper:
        return toneShaperMatrix.parameter( message,++tagIndex, paramIndex );

    case TagMain::EffectRunner:
        return iOThread.getFxRunner().parameter( message,++tagIndex, paramIndex );

    case TagMain::EffectCollector:
        return FxCollector::getInstance().parameter( message,++tagIndex, paramIndex );

    case TagMain::MidiController:
        return iOThread.getRouter().getMidiController().parameter( message,++tagIndex, paramIndex );

    case TagMain::InnerController:
        return InnerController::getInstance().parameter( message,++tagIndex, paramIndex );

    case TagMain::Tuner: // TODO
        return iOThread.getRouter().parameter( message,++tagIndex, paramIndex ); // getTuner() -refactor

    case TagMain::Router:
        return iOThread.getRouter().parameter( message,++tagIndex, paramIndex );

    case TagMain::Mute:
        YaIoJack::getInstance().mute();
        message.setStatusSetOk();
        return true;

    case TagMain::UnMuteOutput:
        YaIoJack::getInstance().unmuteOutput();
        message.setStatusSetOk();
        return true;

    case TagMain::Clear:
    case TagMain::ClearState:
    case TagMain::Preset: 
        // TODO : iterate through all units
        // 1. stop all oscillaotors - wait and clear osc output
        message.setStatus( yaxp::MessageT::illegalTag );
//        message.setStatusSetOk();
        return false;
    }
    TAG_DEBUG(message.getTag(tagIndex), tagIndex, paramIndex, "Sysman" );
    message.setStatus( yaxp::MessageT::illegalTag );
    return false; // error
}

// --------------------------------------------------------------------

} // end namespace yacynth



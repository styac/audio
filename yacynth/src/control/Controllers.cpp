/*
 * Copyright (C) 2016 Istvan Simon
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
 * File:   Controllers.cpp
 * Author: Istvan Simon
 *
 * Created on February 16, 2016, 10:27 AM
 */

#include    "Controllers.h"

namespace yacynth {

using namespace TagInnerControllerLevel_01;
using namespace TagMidiControllerLevel_01;

float InnerController::expFuncTable[midiStep];
uint32_t InnerController::phaseFuncTable[midiStep];

InnerController::InnerController()
{
    clear();
    constexpr float dy          = 0.97;
    float y = 1.0f;

    for( int i=midiStep-1; i>0; --i ) {
        expFuncTable[i] = y;
        y *= dy;
        phaseFuncTable[i] = (i & 0x7E )<<(31-7); // ????
    }
    expFuncTable[0]     = 0.0f;
    phaseFuncTable[0]   = 0;
    phaseFuncTable[midiStep-1] = phaseFuncTable[midiStep-2] = 0x80000000; // ????
    value.v[InnerController::CC_PITCHBEND] = 0x2000;
}

bool InnerController::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    const uint8_t tag = message.getTag(tagIndex);
    switch( TagInnerController( tag ) ) {
    case TagInnerController::Clear:
        TAG_DEBUG(TagInnerController::Clear, tagIndex, paramIndex, "InnerController" );
        clear();
        message.setStatusSetOk();
        return true;

    case TagInnerController::ClearController: {
            TAG_DEBUG(TagInnerController::ClearController, tagIndex, paramIndex, "InnerController" );
            const uint16_t controller = message.params[paramIndex];
            if( controller >= maxIndex ) {
                message.setStatus( yaxp::MessageT::illegalTargetIndex );
                return false;
            }
            value.v[controller] = 0;
            message.setStatusSetOk();
            return true;
        }

    case TagInnerController::SetController: {
            TAG_DEBUG(TagInnerController::SetController, tagIndex, paramIndex, "InnerController" );
            const uint16_t countParam = message.params[paramIndex];
            if( countParam * sizeof(InnerControllerSetting) != message.length ) {
                message.setStatus( yaxp::MessageT::illegalDataLength );
                return false;
            }
            InnerControllerSetting *data = static_cast<InnerControllerSetting *>((void *)(message.data));
            for( uint16_t ind = 0; ind < countParam; ++ind, ++data ) {
                set( data->index, data->value );
            }
            message.setStatusSetOk();
            return true;
        }
    }

    message.setStatus( yaxp::MessageT::illegalTag );
    return false;
}

MidiController::MidiController()
{
    clear();
    // test setup
    // novation impulse 49 -- basic
    // channel 0 -- slide 9
    cdt[0][0x29].index = InnerController::CC_SINK;      // 0
    cdt[0][0x2A].index = InnerController::CC_SINK;
    cdt[0][0x2B].index = InnerController::CC_SINK;
    cdt[0][0x2C].index = InnerController::CC_SINK;
    cdt[0][0x2D].index = InnerController::CC_SINK;
    cdt[0][0x2E].index = InnerController::CC_MODULATOR_FREQ1;
    cdt[0][0x2F].index = InnerController::CC_MODULATOR_FREQ0;

    cdt[0][0x30].index = InnerController::CC_MODULATOR_PHASEDIFF0;
    cdt[0][0x31].index = InnerController::CC_MAINVOLUME; // 8

    cdt[0][0x15].index = InnerController::CC_SINK;      // upper
    cdt[0][0x16].index = InnerController::CC_SINK;
    cdt[0][0x17].index = InnerController::CC_SINK;
    cdt[0][0x18].index = InnerController::CC_SINK;

    cdt[0][0x19].index = InnerController::CC_SINK;      // lower
    cdt[0][0x1A].index = InnerController::CC_SINK;
    cdt[0][0x1B].index = InnerController::CC_SINK;
    cdt[0][0x1C].index = InnerController::CC_SINK;

    cdt[0][controllerPolyAftertouch].index  = InnerController::CC_CHANNEL_AFTERTOUCH;
    cdt[0][controllerPitchbend].index   = InnerController::CC_PITCHBEND;
}

// needs 3 param:

bool MidiController::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    const uint8_t tag = message.getTag(tagIndex);
    if( !message.checkParamIndex(paramIndex) )
        return false;

    const uint8_t channel = message.params[paramIndex];
    switch(  TagMidiController( tag ) ) {
    case  TagMidiController::Clear:
        TAG_DEBUG(TagMidiController::Clear, tagIndex, paramIndex, "MidiController" );
        clear();
        return true;

    case  TagMidiController::ClearChannelVector:
        TAG_DEBUG(TagMidiController::ClearChannelVector, tagIndex, paramIndex, "MidiController" );
        if( channel >= getChannelCount() ) {
            message.setStatus( yaxp::MessageT::illegalParam );
            return false;
        }
        clearChannel( channel );
        return true;

    case  TagMidiController::SetChannelVector:
        TAG_DEBUG(TagMidiController::SetChannelVector, tagIndex, paramIndex, "MidiController" );
        if( channel >= getChannelCount() ) {
            message.setStatus( yaxp::MessageT::illegalParam );
            return false;
        }
        if( message.length != sizeof(cdt[channel]) ) {
            message.setStatus( yaxp::MessageT::illegalDataLength );
            return false;
        }
        std::memcpy( &cdt[channel][0], message.data, sizeof(cdt[channel]) );
        return true;

    case TagMidiController::SetController: {
        TAG_DEBUG(TagMidiController::SetController, tagIndex, paramIndex, "MidiController" );
            const uint16_t countParam = message.params[paramIndex];
            if( countParam * sizeof(MidiSetting) != message.length ) {
                message.setStatus( yaxp::MessageT::illegalDataLength );
                return false;
            }
            MidiSetting *data = static_cast<MidiSetting *>((void *)(message.data));
            for( uint16_t ind = 0; ind < countParam; ++ind, ++data ) {
                set( data->channel, data->midiCC, data->innerIndex, CMode(data->midiMode) );
                InnerController::getInstance().set( data->innerIndex, data->initValue );
            }
            message.setStatusSetOk();
            return true;
        }
        return false;
    }
    message.setStatus( yaxp::MessageT::illegalTag );
    return false;
}


} // end namespace yacynth
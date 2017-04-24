/*
 * Copyright (C) 2016 Istvan Simon -- stevens37 at gmail dot com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without() even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/*
 * File:   FxEcho.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on March 7, 2016, 2:05 PM
 */

#include    "FxEcho.h"

namespace yacynth {
using namespace TagEffectFxEchoModeLevel_03;


bool FxEchoParam::parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    const uint8_t tag = message.getTag(tagIndex);    
#if 0    
    switch( TagEffectFxMixerMode( tag ) ) {
    case TagEffectFxMixerMode::Clear :
        TAG_DEBUG(TagEffectFxMixerMode::Clear, tagIndex, paramIndex, "FxMixerParam" );
        clear();
        return true;
        
    case TagEffectFxMixerMode::SetVolumeControllerIndex :
        TAG_DEBUG(TagEffectFxMixerMode::SetVolumeControllerIndex, tagIndex, paramIndex, "FxMixerParam" );
        if( !message.checkParamIndex(paramIndex+1) )                          
            return false;        
        const uint16_t channel = message.params[paramIndex];
        if(channel < inputCount) {            
            if( gainIndex[channel].setIndex( message.params[paramIndex+1] ) ) {
                return true;            
            }
            message.setStatus( Yaxp::MessageT::illegalTargetIndex, tag );                
            return false;
        }
        message.setStatus( Yaxp::MessageT::illegalTargetIndex, tag );            
        return false;
    }            
    TAG_DEBUG(TagEffectFxMixerMode::Nop, tagIndex, paramIndex, "FxMixerParam" );
    message.setStatus( Yaxp::MessageT::illegalTag, tag );    
    return false;
#endif
}



// --------------------------------------------------------------------
void FxEcho::testvect(void)
{
    tapOutputVector.fill( echoTapsOut() );
    tapFeedbackVector.fill( echoTapsFeedback() );
//    tapOutputVector.list();
//    tapFeedbackVector.list();
}
// --------------------------------------------------------------------

void FxEcho::clearTransient()
{
    EIObuffer::clear();
    delay.clear();
}

bool FxEcho::connect( const FxBase * v, uint16_t ind )
{
    doConnect(v,ind);
};

bool FxEcho::parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    // 1st tag is tag effect type
    const uint8_t tagType = message.getTag(tagIndex);    
    if( uint8_t(param.type) != tagType ) {
        message.setStatus( Yaxp::MessageT::illegalTagEffectType, uint8_t(param.type) );
        TAG_DEBUG(Yaxp::MessageT::illegalTagEffectType, uint8_t(param.type), tagType, "FxEcho" );
        return false;        
    }
    // 2nd tag is tag operation
    const uint8_t tag = message.getTag(++tagIndex);
    if( uint8_t(TagEffectFxEchoMode::Clear) == tag ) {
        clearTransient(); // this must be called to cleanup
    }
    // forward to param
    return param.parameter( message, tagIndex, paramIndex );    
}

void FxEcho::sprocess_00( void * thp )
{
//    static_cast< MyType * >(thp)->processCopy0();
}
void FxEcho::sprocess_01( void * thp )
{
//    static_cast< MyType * >(thp)->processModulation();
}

// --------------------------------------------------------------------
void FxEcho::process(void)
{
    out().copy( inp<0>() );
    // do this in preset
    const uint16_t loopCountF = std::min( tapFeedbackVector.usedTapCount, tapFeedbackVector.vectorSize );
    for( auto i = 0u; i < loopCountF; ++i ) {
        auto& tv = tapFeedbackVector.dtvec[i];
        // normalize in setup
        if( tv.isValid() ) {
            uint32_t ind = tv.delaySrcH + sectionSize;   // index is negated
            for( auto i = 0u; i < sectionSize; ++i ) {
                delay.multAddMixStereoNoisefloor<-24>( --ind, tv.coeff, out().channel[EbufferPar::chA][i], out().channel[EbufferPar::chB][i] );
            }
        }
    }
    delay.pushSection( out().channel[EbufferPar::chA], out().channel[EbufferPar::chB] );
    if( inMix ) {
        out().copy( inp<0>() );
    } else {
        out().clear();
    }
    // do this in preset
    const uint16_t loopCountO = std::min( tapOutputVector.usedTapCount, tapOutputVector.vectorSize );
    for( auto i = 0u; i < loopCountO; ++i ) {
        // normalize in setup
        auto& tv = tapOutputVector.dtvec[i];
        if( tv.isValid() ) {
            uint32_t ind = tv.delaySrcH + sectionSize;   // index is negated
            for( auto i = 0u; i < sectionSize; ++i ) {
                delay.multAddMixStereo( --ind, tv.coeff, out().channel[EbufferPar::chA][i], out().channel[EbufferPar::chB][i] );
            }
        }
    }
} // end FxEcho:process
// --------------------------------------------------------------------


} // end namespace yacynth



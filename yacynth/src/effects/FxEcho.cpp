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
#include "yacynth_config.h"
#include "FxEcho.h"

namespace yacynth {
using namespace TagEffectFxEchoModeLevel_03;

bool FxEchoParam::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    const uint8_t tag = message.getTag(tagIndex);
    switch( TagEffectFxEchoMode( tag ) ) {
    case TagEffectFxEchoMode::Clear :
        TAG_DEBUG(TagEffectFxEchoMode::Clear, tagIndex, paramIndex, "FxEchoParam" );
//        clear();
        message.setStatusSetOk();
        return true;

    case TagEffectFxEchoMode::SetParametersMode01 :
        TAG_DEBUG(TagEffectFxEchoMode::SetParametersMode01, tagIndex, paramIndex, "FxEchoParam" );
        if( ! message.checkTargetData(*this) ) {
            message.setStatus( yaxp::MessageT::illegalData);
            return false;
        }
        if( ! message.setTargetData(*this) ) {
            message.setStatus( yaxp::MessageT::illegalData);
            return false;
        }
        message.setStatusSetOk();
        return true;

    case TagEffectFxEchoMode::SetDryCoeffs :
        TAG_DEBUG(TagEffectFxEchoMode::SetDryCoeffs, tagIndex, paramIndex, "FxEchoParam" );

        if( check() ) {
            message.setStatusSetOk();
            return true;
        }
        message.setStatus( yaxp::MessageT::illegalTargetIndex);
        return false;

    case TagEffectFxEchoMode::SetTapOutput :
        TAG_DEBUG(TagEffectFxEchoMode::SetTapOutput, tagIndex, paramIndex, "FxEchoParam" );

        if( check() ) {
            message.setStatusSetOk();
            return true;
        }
        message.setStatus( yaxp::MessageT::illegalTargetIndex);
        return false;

    case TagEffectFxEchoMode::SetTapFeedback :
        TAG_DEBUG(TagEffectFxEchoMode::SetTapFeedback, tagIndex, paramIndex, "FxEchoParam" );

        if( check() ) {
            message.setStatusSetOk();
            return true;
        }
        message.setStatus( yaxp::MessageT::illegalTargetIndex);
        return false;

    case TagEffectFxEchoMode::SetTapOutputLP :
        TAG_DEBUG(TagEffectFxEchoMode::SetTapOutputLP, tagIndex, paramIndex, "FxEchoParam" );

        if( check() ) {
            message.setStatusSetOk();
            return true;
        }
        message.setStatus( yaxp::MessageT::illegalTargetIndex);
        return false;

    case TagEffectFxEchoMode::SetTapFeedbackLP :
        TAG_DEBUG(TagEffectFxEchoMode::SetTapFeedbackLP, tagIndex, paramIndex, "FxEchoParam" );

        if( check() ) {
            message.setStatusSetOk();
            return true;
        }
        message.setStatus( yaxp::MessageT::illegalTargetIndex);
        return false;

    case TagEffectFxEchoMode::SetTapOutputCount :
        TAG_DEBUG(TagEffectFxEchoMode::SetTapOutputCount, tagIndex, paramIndex, "FxEchoParam" );
        if( message.checkParamIndex(paramIndex) ) {
            tapOutputUsed = message.getParam(paramIndex);
            if( tapOutputUsed > tapOutputCount ) {
                tapOutputUsed = tapOutputCount;
            }
            message.setStatusSetOk();
            return true;
        }
        message.setStatus( yaxp::MessageT::illegalParamIndex );
        return false;

    case TagEffectFxEchoMode::SetTapFeedbackCount :
        TAG_DEBUG(TagEffectFxEchoMode::SetTapFeedbackCount, tagIndex, paramIndex, "FxEchoParam" );
        if( message.checkParamIndex(paramIndex) ) {
            tapFeedbackUsed = message.getParam(paramIndex);
            if( tapFeedbackUsed > tapFeedbackCount ) {
                tapFeedbackUsed = tapFeedbackCount;
            }
            message.setStatusSetOk();
            return true;
        }

        message.setStatus( yaxp::MessageT::illegalTargetIndex);
        return false;

    case TagEffectFxEchoMode::SetTapOutputLPCount :
        TAG_DEBUG(TagEffectFxEchoMode::SetTapOutputLPCount, tagIndex, paramIndex, "FxEchoParam" );
        if( message.checkParamIndex(paramIndex) ) {
            tapOutputLPUsed = message.getParam(paramIndex);
            if( tapOutputLPUsed > tapOutputLPCount ) {
                tapOutputLPUsed = tapOutputLPCount;
            }
            message.setStatusSetOk();
            return true;
        }

        message.setStatus( yaxp::MessageT::illegalTargetIndex);
        return false;

    case TagEffectFxEchoMode::SetTapFeedbackLPCount :
        TAG_DEBUG(TagEffectFxEchoMode::SetTapFeedbackLPCount, tagIndex, paramIndex, "FxEchoParam" );
        if( message.checkParamIndex(paramIndex) ) {
            tapFeedbackLPUsed = message.getParam(paramIndex);
            if( tapFeedbackLPUsed > tapFeedbackLPCount ) {
                tapFeedbackLPUsed = tapFeedbackLPCount;
            }
            message.setStatusSetOk();
            return true;
        }

        message.setStatus( yaxp::MessageT::illegalTargetIndex);
        return false;
    default:
        break;        
        
    }
    TAG_DEBUG(TagEffectFxEchoMode::Nop, tagIndex, paramIndex, "FxEchoParam" );
    message.setStatus( yaxp::MessageT::illegalTag );
    return false;
}

// --------------------------------------------------------------------

void FxEcho::clearState()
{
    // out().clear();    
    delay.clear();
}

bool FxEcho::connect( const FxBase * v, uint16_t ind )
{
    return doConnect(v,ind);
};

bool FxEcho::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    // 1st tag is tag effect type
    const uint8_t tagType = message.getTag(tagIndex);
    if( uint8_t(param.type) != tagType ) {
        message.setStatus( yaxp::MessageT::illegalTagEffectType );
        TAG_DEBUG(yaxp::MessageT::illegalTagEffectType, uint8_t(param.type), tagType, "FxEcho" );
        return false;
    }
    // 2nd tag is tag operation
    const uint8_t tag = message.getTag(++tagIndex);
    switch( TagEffectFxEchoMode( tag ) ) {
    case TagEffectFxEchoMode::ClearState:
        clearState(); // this must be called to cleanup
        message.setStatusSetOk();
        return true;
        
    case TagEffectFxEchoMode::Clear:
        clearState(); // this must be called to cleanup
        break;
    default:
        break;        
    }
    // forward to param
    return param.parameter( message, tagIndex, paramIndex );
}

void FxEcho::sprocess_01( void * thp )
{
    static_cast< MyType * >(thp)->process_clear_dry();
    static_cast< MyType * >(thp)->process_echo_wet();
}

void FxEcho::sprocess_02( void * thp )
{
    static_cast< MyType * >(thp)->process_add_dry();
    static_cast< MyType * >(thp)->process_echo_wet();
}

bool FxEcho::setSprocessNext( uint16_t mode ) 
{
    switch( mode ) {
    case 0:
        procMode = 0;
        sprocesspNext = FxBase::sprocessClear2Nop;
        sprocessp = FxBase::sprocessFadeOut;        
        return true;
    case 1:
        sprocesspNext = sprocess_01;
        break;
    case 2:
        sprocesspNext = sprocess_02;
        break;
    default:
        return false;
    }
    bool fadeIn = 0 == procMode;
    procMode = mode;
    if( fadeIn ) {
        sprocesspCurr = sprocesspNext;
        sprocessp = FxBase::sprocessFadeIn;
        return true;
    }
    sprocessp = FxBase::sprocessCrossFade;
    return true;
}

} // end namespace yacynth



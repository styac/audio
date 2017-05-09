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
    switch( TagEffectFxEchoMode( tag ) ) {
    case TagEffectFxEchoMode::Clear :
        TAG_DEBUG(TagEffectFxEchoMode::Clear, tagIndex, paramIndex, "FxEchoParam" );
//        clear();
        return true;

    case TagEffectFxEchoMode::SetParameters :
        TAG_DEBUG(TagEffectFxEchoMode::SetParameters, tagIndex, paramIndex, "FxEchoParam" );
        if( ! message.checkTargetData(*this) ) {
            message.setStatus( Yaxp::MessageT::illegalData, tag );
            return false;
        }
        if( ! message.setTargetData(*this) ) {
            message.setStatus( Yaxp::MessageT::illegalData, tag );
            return false;
        }
        return true;

    case TagEffectFxEchoMode::SetDryCoeffs :
        TAG_DEBUG(TagEffectFxEchoMode::SetDryCoeffs, tagIndex, paramIndex, "FxEchoParam" );

        if( check() ) {
            return true;
        }
        message.setStatus( Yaxp::MessageT::illegalTargetIndex, tag );
        return false;

    case TagEffectFxEchoMode::SetTapOutput :
        TAG_DEBUG(TagEffectFxEchoMode::SetTapOutput, tagIndex, paramIndex, "FxEchoParam" );

        if( check() ) {
            return true;
        }
        message.setStatus( Yaxp::MessageT::illegalTargetIndex, tag );
        return false;

    case TagEffectFxEchoMode::SetTapFeedback :
        TAG_DEBUG(TagEffectFxEchoMode::SetTapFeedback, tagIndex, paramIndex, "FxEchoParam" );

        if( check() ) {
            return true;
        }
        message.setStatus( Yaxp::MessageT::illegalTargetIndex, tag );
        return false;

    case TagEffectFxEchoMode::SetTapOutputLP :
        TAG_DEBUG(TagEffectFxEchoMode::SetTapOutputLP, tagIndex, paramIndex, "FxEchoParam" );

        if( check() ) {
            return true;
        }
        message.setStatus( Yaxp::MessageT::illegalTargetIndex, tag );
        return false;

    case TagEffectFxEchoMode::SetTapFeedbackLP :
        TAG_DEBUG(TagEffectFxEchoMode::SetTapFeedbackLP, tagIndex, paramIndex, "FxEchoParam" );

        if( check() ) {
            return true;
        }
        message.setStatus( Yaxp::MessageT::illegalTargetIndex, tag );
        return false;

    case TagEffectFxEchoMode::SetTapOutputCount :
        TAG_DEBUG(TagEffectFxEchoMode::SetTapOutputCount, tagIndex, paramIndex, "FxEchoParam" );
        if( message.checkParamIndex(paramIndex) ) {
            tapOutputCount = message.getParam(paramIndex);
            if( tapOutputCount > tapOutputSize ) {
                tapOutputCount = tapOutputSize;
            }
            return true;
        }
        message.setStatus( Yaxp::MessageT::illegalParamIndex, tag );
        return false;

    case TagEffectFxEchoMode::SetTapFeedbackCount :
        TAG_DEBUG(TagEffectFxEchoMode::SetTapFeedbackCount, tagIndex, paramIndex, "FxEchoParam" );
        if( message.checkParamIndex(paramIndex) ) {
            tapFeedbackCount = message.getParam(paramIndex);
            if( tapFeedbackCount > tapFeedbackSize ) {
                tapFeedbackCount = tapFeedbackSize;
            }
            return true;
        }

        message.setStatus( Yaxp::MessageT::illegalTargetIndex, tag );
        return false;

    case TagEffectFxEchoMode::SetTapOutputLPCount :
        TAG_DEBUG(TagEffectFxEchoMode::SetTapOutputLPCount, tagIndex, paramIndex, "FxEchoParam" );
        if( message.checkParamIndex(paramIndex) ) {
            tapOutputLPCount = message.getParam(paramIndex);
            if( tapOutputLPCount > tapOutputLPSize ) {
                tapOutputLPCount = tapOutputLPSize;
            }
            return true;
        }

        message.setStatus( Yaxp::MessageT::illegalTargetIndex, tag );
        return false;

    case TagEffectFxEchoMode::SetTapFeedbackLPCount :
        TAG_DEBUG(TagEffectFxEchoMode::SetTapFeedbackLPCount, tagIndex, paramIndex, "FxEchoParam" );
        if( message.checkParamIndex(paramIndex) ) {
            tapFeedbackLPCount = message.getParam(paramIndex);
            if( tapFeedbackLPCount > tapFeedbackLPSize ) {
                tapFeedbackLPCount = tapFeedbackLPSize;
            }
            return true;
        }

        message.setStatus( Yaxp::MessageT::illegalTargetIndex, tag );
        return false;
    }
    TAG_DEBUG(TagEffectFxEchoMode::Nop, tagIndex, paramIndex, "FxEchoParam" );
    message.setStatus( Yaxp::MessageT::illegalTag, tag );
    return false;
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
//    static_cast< MyType * >(thp)->clearTransient();
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

} // end namespace yacynth



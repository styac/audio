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
 * File:   FxFilter.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on June 21, 2016, 9:57 PM
 */

#include "FxFilter.h"

namespace yacynth {
using namespace TagEffectFxFilterModeLevel_03;

bool FxFilterParam::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    const uint8_t tag = message.getTag(tagIndex);
    switch( TagEffectFxFilterMode( tag ) ) {
    case TagEffectFxFilterMode::SetModeAP01 :
        TAG_DEBUG(TagEffectFxFilterMode::SetModeAP01, tagIndex, paramIndex, "FxFilterParam - SetMode_2ch_x4ap_phaser_mode01" );
        if( ! message.checkTargetData(modeAP01) ) {
            message.setStatus( yaxp::MessageT::illegalData);
            return false;
        }
        if( message.setTargetData(modeAP01) ) {
            message.setStatusSetOk();      
            return true;
        }
        message.setStatus( yaxp::MessageT::illegalDataLength );
        return false;
        
    case TagEffectFxFilterMode::SetModeSV01 :
        TAG_DEBUG(TagEffectFxFilterMode::SetModeSV01, tagIndex, paramIndex, "FxFilterParam - SetMode_SVF01_2ch_mode01" );
        if( ! message.checkTargetData(modeSV01) ) {
            message.setStatus( yaxp::MessageT::illegalData);
            return false;
        }
        if( message.setTargetData(modeSV01) ) {
            message.setStatusSetOk();      
            return true;
        }
        message.setStatus( yaxp::MessageT::illegalDataLength );
        return false;
        
    case TagEffectFxFilterMode::SetMode4P01 :
        TAG_DEBUG(TagEffectFxFilterMode::SetMode4P01, tagIndex, paramIndex, "FxFilterParam - SetMode_4p_2ch" );
        if( ! message.checkTargetData(modeSV01) ) {
            message.setStatus( yaxp::MessageT::illegalData);
            return false;
        }
        if( message.setTargetData(mode4P01) ) {
            message.setStatusSetOk();      
            return true;
        }
        message.setStatus( yaxp::MessageT::illegalDataLength );
        return false;
    // more modes
    }
            
    message.setStatus( yaxp::MessageT::illegalTag );
    return false;    
}

void FxFilter::clearTransient()
{
    EIObuffer::clear();
}

bool FxFilter::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    // 1st tag is tag effect type
    const uint8_t tagType = message.getTag(tagIndex);    
    if( uint8_t(param.type) != tagType ) {
        message.setStatus( yaxp::MessageT::illegalTagEffectType );
        TAG_DEBUG(yaxp::MessageT::illegalTagEffectType, uint8_t(param.type), tagType, "FxFilter" );
        return false;        
    }
    // 2nd tag is tag operation
    const uint8_t tag = message.getTag(++tagIndex);
    if( uint8_t(TagEffectFxFilterMode::Clear) == tag ) {
        clearTransient(); // this must be called to cleanup
        return true;
    }
    clearTransient();
    // forward to param
    return param.parameter( message, tagIndex, paramIndex );    
}

    // 00 is always clear for output or bypass for in-out
void FxFilter::sprocess_00( void * thp )
{
    static_cast< MyType * >(thp)->clear();
}
void FxFilter::sprocess_01( void * thp )
{
    static_cast< MyType * >(thp)->process_01_ap4x();
}
void FxFilter::sprocess_02( void * thp )
{
    static_cast< MyType * >(thp)->process_02_svf_1x();
}
void FxFilter::sprocess_03( void * thp )
{
    static_cast< MyType * >(thp)->process_03_pole4_1x();
}
void FxFilter::sprocess_04( void * thp )
{
    static_cast< MyType * >(thp)->process_01_ap4x();
}
void FxFilter::sprocess_05( void * thp )
{
    static_cast< MyType * >(thp)->process_01_ap4x();
}
void FxFilter::sprocess_06( void * thp )
{
    static_cast< MyType * >(thp)->process_01_ap4x();
}
void FxFilter::sprocess_07( void * thp )
{
    static_cast< MyType * >(thp)->process_01_ap4x();
}
void FxFilter::sprocess_08( void * thp )
{
    static_cast< MyType * >(thp)->process_01_ap4x();
}
void FxFilter::sprocess_09( void * thp )
{
    static_cast< MyType * >(thp)->process_01_ap4x();
}

} // end namespace yacynth



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

// obsolate if setting is ready
FxFilterParam::FxFilterParam()
{
    const uint16_t masterLfo = 0;
    const uint16_t slaveLfo = 0;


    // test lfo 0, 0
    InnerController& inco = InnerController::getInstance();

    // OBSOLATE
    testMasterLfo1.index = inco.getIndexMasterLfoPhase(masterLfo);
    testSlaveLfo1.index = inco.getIndexSlaveMasterLfoPhase(masterLfo,slaveLfo);

   // set test values for delta phase
    testMasterLfoDeltaPhase1.index = inco.getIndexMasterLfoDeltaPhase(masterLfo);
    testSlaveLfoDeltaPhase1.index = inco.getIndexSlaveMasterLfoDeltaPhase(masterLfo,slaveLfo);

    uint32_t masterDDP = freq2deltaPhase(0.1f) << 6; // 1 Hz - 1/64 sampling freq
    uint32_t slaveDDP = 0x40000000; // phase diff

    testMasterLfoDeltaPhase1.set( masterDDP ); // ???
    testSlaveLfoDeltaPhase1.set( slaveDDP );

    const int32_t freq_01 = freq2ycent(200.0f);
    const int32_t freq_02 = freq2ycent(200.0f);
    testMasterLfo1.setYcent8Parameter(freq_01 , 512);
    testSlaveLfo1.setYcent8Parameter(freq_02 , 300);

    // NEW
    // defaults
    // mode 01
    mode_01_ap4x.cindex_masterLfo.index              = inco.getIndexMasterLfoPhase(masterLfo);
    mode_01_ap4x.cindex_masterLfoDeltaPhase.index    = inco.getIndexMasterLfoDeltaPhase(masterLfo);
    mode_01_ap4x.cmaplin_masterLfo.mult      = 512;
    mode_01_ap4x.cmaplin_masterLfo.y0[0]     = freq2ycent(80.0f);
    mode_01_ap4x.cmaplin_masterLfo.y0[1]     = freq2ycent(160.0f);
    mode_01_ap4x.cmaplin_masterLfo.y0[2]     = freq2ycent(320.0f);
    mode_01_ap4x.cmaplin_masterLfo.y0[3]     = freq2ycent(640.0f);
    mode_01_ap4x.cmaplin_masterLfo.y0[4]     = freq2ycent(1280.0f);
    mode_01_ap4x.cmaplin_masterLfo.y0[5]     = freq2ycent(2560.0f);
    mode_01_ap4x.cmaplin_masterLfo.y0[6]     = freq2ycent(5120.0f);
    mode_01_ap4x.cmaplin_masterLfo.y0[7]     = freq2ycent(10240.0f);

}

bool FxFilterParam::parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    const uint8_t tag = message.getTag(tagIndex);
    switch( TagEffectFxFilterMode( tag ) ) {
    case TagEffectFxFilterMode::SetMode_01_ap4x :
        TAG_DEBUG(TagEffectFxFilterMode::SetMode_01_ap4x, tagIndex, paramIndex, "FxFilterParam - SetMode_01_ap4x" );
        if( message.setTargetData(mode_01_ap4x) ) {
            return true;
        }
        message.setStatus( Yaxp::MessageT::illegalDataLength, tag );
        return false;
        
    case TagEffectFxFilterMode::SetMode_02_ap4x :
        TAG_DEBUG(TagEffectFxFilterMode::SetMode_02_ap4x, tagIndex, paramIndex, "FxFilterParam - SetMode_02_ap4x" );
        if( message.setTargetData(mode_02_ap4x) ) {
            return true;
        }
        message.setStatus( Yaxp::MessageT::illegalDataLength, tag );
        return false;        
    // more modes
    }
            
    message.setStatus( Yaxp::MessageT::illegalTag, tag );
    return false;    
}



void FxFilter::clearTransient()
{
    EIObuffer::clear();
}

bool FxFilter::parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    // 1st tag is tag effect type
    const uint8_t tagType = message.getTag(tagIndex);    
    if( uint8_t(param.type) != tagType ) {
        message.setStatus( Yaxp::MessageT::illegalTagEffectType, uint8_t(param.type) );
        TAG_DEBUG(Yaxp::MessageT::illegalTagEffectType, uint8_t(param.type), tagType, "FxFilter" );
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
    static_cast< MyType * >(thp)->process_01_ap4x();
}
void FxFilter::sprocess_03( void * thp )
{
    static_cast< MyType * >(thp)->process_01_ap4x();
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



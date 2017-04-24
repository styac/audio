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
 * File:   Mixer.cpp
 * Author: Istvan Simon
 *
 * Created on March 15, 2016, 6:23 PM
 */

#include "FxMixer.h"

namespace yacynth {
using namespace TagEffectFxMixerModeLevel_03;


bool FxMixerParam::parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    uint16_t index = 0;
    const uint8_t tag       = message.getTag(tagIndex);    
    if( !message.checkParamIndex(paramIndex) ) {
        message.setStatus( Yaxp::MessageT::illegalParamIndex, tag );            
        return false;  
    }                         
    const uint16_t channel  = message.params[paramIndex];
    if(channel >= inputCount) { 
        message.setStatus( Yaxp::MessageT::illegalTargetIndex, tag );                
        return false;
    }
    switch( TagEffectFxMixerMode( tag ) ) {
    case TagEffectFxMixerMode::Clear :
        TAG_DEBUG(TagEffectFxMixerMode::Clear, tagIndex, paramIndex, "FxMixerParam" );
        clear();
        return true;
        
    case TagEffectFxMixerMode::SetVolumeControllerIndex :
        TAG_DEBUG(TagEffectFxMixerMode::SetVolumeControllerIndex, tagIndex, paramIndex, "FxMixerParam" );        
        if( message.setTargetData( index ) ) {
            gainIndex[channel].setIndex(index);
            return true;            
        }        
        message.setStatus( Yaxp::MessageT::illegalDataLength, tag );            
        return false;
        
    case TagEffectFxMixerMode::SetVolumeRange : 
        TAG_DEBUG(TagEffectFxMixerMode::SetVolumeRange, tagIndex, paramIndex, "FxMixerParam" );
        if( message.setTargetData( gainRange[channel] ) ) {
            std::cout <<  "--- channel " << channel <<  " volume " << gainRange[channel] << std::endl;
            return true;            
        }        
        message.setStatus( Yaxp::MessageT::illegalDataLength, tag );            
        return false;
     
    } // end switch
    TAG_DEBUG(TagEffectFxMixerMode::Nop, tagIndex, paramIndex, "FxMixerParam" );
    message.setStatus( Yaxp::MessageT::illegalTag, tag );    
    return false;
    
} // end FxMixerParam::parameter

bool FxMixer::connect( const FxBase * v, uint16_t ind )
{
    doConnect(v,ind);
};

void FxMixer::clearTransient()
{
    EIObuffer::clear();
}

bool FxMixer::setProcMode( uint16_t ind )
{
    std::cout << "---- setProcMode mixer " << ind << std::endl;
    if( procMode == ind ) {
        return true; // no change
    }
    if( getMaxMode() < procMode ) {
        return false; // illegal
    }
    if( 0 == procMode ) {
        fadePhase = FadePhase::FPH_fadeInSimple;
    } else if( 0 == ind ) {
        fadePhase = FadePhase::FPH_fadeOutSimple;
    } else {
        fadePhase = FadePhase::FPH_fadeOutCross;
    }

    procMode = ind;
    sprocessp = sprocesspSave = sprocessv[ind];
    // sprocesspSave =  sprocessv[ th.procMode ];
    // sprocessp = sprocessTransient;
    return true;
}

bool FxMixer::parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    // 1st tag is tag effect type
    const uint8_t tagType = message.getTag(tagIndex);    
    if( uint8_t(param.type) != tagType ) {
        message.setStatus( Yaxp::MessageT::illegalTagEffectType, uint8_t(param.type) );
        TAG_DEBUG(Yaxp::MessageT::illegalTagEffectType, uint8_t(param.type), tagType, "FxMixer" );
        return false;        
    }
    // 2nd tag is tag operation
    const uint8_t tag = message.getTag(++tagIndex);
    if( uint8_t(TagEffectFxMixerMode::Clear) == tag ) {
        clearTransient(); // this must be called to cleanup
    }
    // forward to param
    return param.parameter( message, tagIndex, paramIndex );    
}

    // 00 is always clear for output or bypass for in-out
    void FxMixer::sprocess_00( void * thp )
    {
       // static_cast< FxOutNoise * >(thp)->clear();
    }

    void FxMixer::sprocess_01( void * thp )
    {
        static_cast< MyType * >(thp)->mix_01();
    }

    void FxMixer::sprocess_02( void * thp )
    {
        static_cast< MyType * >(thp)->mix_02();
    }

    void FxMixer::sprocess_03( void * thp )
    {
        static_cast< MyType * >(thp)->mix_03();
    }

    void FxMixer::sprocess_04( void * thp )
    {
        static_cast< MyType * >(thp)->mix_04();
    }

} // end namespace yacynth

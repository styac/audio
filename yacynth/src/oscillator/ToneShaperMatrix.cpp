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
 * File:   ToneShaperMatrix.cpp
 * Author: Istvan Simon
 *
 * Created on April 4, 2016, 11:52 AM
 */

#include    "../oscillator/ToneShaperMatrix.h"
#include    "memory.h"

namespace yacynth {
using namespace TagToneShaperLevel_01;

// --------------------------------------------------------------------
// obsolete
bool ToneShaperMatrix::fill( YsifInpStream& ser )
{
    // for testing
    bool ret = deserialize ( ser, toneShapers[0] );
    return ret;
}

// --------------------------------------------------------------------
// obsolete
void ToneShaperMatrix::dump( YsifOutStream& ser )
{
    // for testing
    serialize ( ser, toneShapers[0] );
}
// --------------------------------------------------------------------

void ToneShaperMatrix::clear(void)
{
    memset((void*)this,0,sizeof(ToneShaperMatrix)); // check
}

// --------------------------------------------------------------------

bool ToneShaperMatrix::parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    const uint8_t tag = message.getTag(tagIndex);
    switch( TagToneShaper( tag ) ) {
    case  TagToneShaper::Clear :
        TAG_DEBUG(TagToneShaper::Clear, tagIndex, paramIndex, "ToneShaperMatrix" );
        clear();
        return true;

    case  TagToneShaper::SetOvertoneCount :
        TAG_DEBUG(TagToneShaper::SetOvertoneCount, tagIndex, paramIndex, "ToneShaperMatrix" );
        {
            const uint16_t vectorIndex = message.getParam(paramIndex);
            const uint16_t overtoneCount = message.getParam(++paramIndex);
            if( overtoneCount > overtoneCountOscDef ) {
                return false;
            }
            toneShapers[vectorIndex].oscillatorCountUsed = overtoneCount;
        }
        return true;

    case  TagToneShaper::SetOvertone :
        TAG_DEBUG(TagToneShaper::SetOvertoneCount, tagIndex, paramIndex, "ToneShaperMatrix" );
        if( message.length == sizeof(ToneShaper) ) {
            const uint16_t vectorIndex      = message.getParam(paramIndex);
            const uint16_t overtoneIndex    = message.getParam(++paramIndex);
            if( overtoneIndex > overtoneCountOscDef ) {
                message.setStatus( Yaxp::MessageT::illegalTargetIndex, 1 );
                return false;
            }
            if( vectorIndex > settingVectorSize-1 ) {
                message.setStatus( Yaxp::MessageT::illegalTargetIndex, 2 );
                return false;
            }
            if( !message.setTargetData(toneShapers[vectorIndex].toneShaperVec[overtoneIndex] ) ) {
                message.setStatus( Yaxp::MessageT::illegalDataLength );
                return false;                
            }                
            if( toneShapers[vectorIndex].toneShaperVec[overtoneIndex].check() ) {
                return true;
            }
            message.setStatus( Yaxp::MessageT::illegalData );
            return false;            
        }
        message.setStatus( Yaxp::MessageT::illegalDataLength );
        return false;

    case  TagToneShaper::GetOvertone :
        TAG_DEBUG(TagToneShaper::SetOvertoneCount, tagIndex, paramIndex, "ToneShaperMatrix" );
        if( message.length >= sizeof(ToneShaper) ) {
            const uint16_t vectorIndex      = message.getParam(paramIndex);
            const uint16_t overtoneIndex    = message.getParam(++paramIndex);
            if( overtoneIndex > overtoneCountOscDef ) {
                message.setStatus( Yaxp::MessageT::illegalTargetIndex, 1 );
                return false;
            }
            if( vectorIndex > settingVectorSize-1 ) {
                message.setStatus( Yaxp::MessageT::illegalTargetIndex, 2 );
                return false;
            }
            message.getTargetData(toneShapers[vectorIndex].toneShaperVec[overtoneIndex] );
            return true;            
        }
        message.setStatus( Yaxp::MessageT::illegalDataLength );
        return false;
    }    
    message.setStatus( Yaxp::MessageT::illegalTag, tag );
    return false;    
}

} // end namespace yacynth
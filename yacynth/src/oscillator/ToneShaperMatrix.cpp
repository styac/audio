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

#include    "oscillator/ToneShaperMatrix.h"
#include    "memory.h"

namespace yacynth {
using namespace TagToneShaperLevel_01;

// --------------------------------------------------------------------

void ToneShaperMatrix::clear(void)
{
    memset((void*)this,0,sizeof(ToneShaperMatrix)); // check
}

// --------------------------------------------------------------------

bool ToneShaperMatrix::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    const uint8_t tag = message.getTag(tagIndex);
    switch( TagToneShaper( tag ) ) {
    case  TagToneShaper::Clear :
        TAG_DEBUG(TagToneShaper::Clear, tagIndex, paramIndex, "ToneShaperMatrix" );
        clear();
        message.setStatusSetOk();
        return true;

    case  TagToneShaper::SetOvertoneCount :
        TAG_DEBUG(TagToneShaper::SetOvertoneCount, tagIndex, paramIndex, "ToneShaperMatrix" );
        {
            const uint16_t vectorIndex = message.getParam(paramIndex);
            const uint16_t overtoneCount = message.getParam(++paramIndex);
            if( vectorIndex >= settingVectorSize ) {
                message.setStatus( yaxp::MessageT::illegalTargetIndex );
                return false;
            }
            if( overtoneCount > overtoneCountOscDef ) {
                message.setStatus( yaxp::MessageT::illegalParam );
                return false;
            }
            toneShapers[vectorIndex].oscillatorCountUsed = overtoneCount;
        }
        message.setStatusSetOk();
        return true;

    case  TagToneShaper::SetOvertone :
        TAG_DEBUG(TagToneShaper::SetOvertone, tagIndex, paramIndex, "ToneShaperMatrix" );
        if( message.length == sizeof(ToneShaper) ) {
            const uint16_t vectorIndex      = message.getParam(paramIndex);
            const uint16_t overtoneIndex    = message.getParam(++paramIndex);
            if( overtoneIndex > overtoneCountOscDef ) {
                message.setStatus( yaxp::MessageT::illegalTargetIndex );
                return false;
            }
            if( vectorIndex > settingVectorSize-1 ) {
                message.setStatus( yaxp::MessageT::illegalTargetIndex );
                return false;
            }
            if( !message.setTargetData(toneShapers[vectorIndex].toneShaper[overtoneIndex] ) ) {
                message.setStatus( yaxp::MessageT::illegalDataLength );
                return false;
            }
            if( toneShapers[vectorIndex].toneShaper[overtoneIndex].check() ) {
                message.setStatusSetOk();
                return true;
            }
            message.setStatus( yaxp::MessageT::illegalData );
            return false;
        }
        message.setStatus( yaxp::MessageT::illegalDataLength );
        return false;

    case  TagToneShaper::GetOvertone :
        TAG_DEBUG(TagToneShaper::GetOvertone, tagIndex, paramIndex, "ToneShaperMatrix" );
        if( message.size >= sizeof(ToneShaper) ) {
            const uint16_t vectorIndex      = message.getParam(paramIndex);
            const uint16_t overtoneIndex    = message.getParam(++paramIndex);
            if( overtoneIndex > overtoneCountOscDef ) {
                message.setStatus( yaxp::MessageT::illegalTargetIndex );
                return false;
            }
            if( vectorIndex > settingVectorSize-1 ) {
                message.setStatus( yaxp::MessageT::illegalTargetIndex );
                return false;
            }
            message.getTargetData(toneShapers[vectorIndex].toneShaper[overtoneIndex] );
            message.setStatusGetOk();
            return true;
        }
        message.setStatus( yaxp::MessageT::illegalDataLength );
        return false;

    // array of int32_t * pitchCount
    case  TagToneShaper::SetPitchVector :
        TAG_DEBUG(TagToneShaper::SetPitchVector, tagIndex, paramIndex, "ToneShaperMatrix" );
        {
            const uint16_t vectorIndex = message.getParam(paramIndex);
            const uint16_t pitchCount = message.getParam(++paramIndex);

            // call ToneShaperMatrix::setPitchVector ?
            
            if( vectorIndex >= settingVectorSize ) {
                message.setStatus( yaxp::MessageT::illegalTargetIndex );
                return false;
            }
            if( pitchCount > overtoneCountOscDef ) {
                message.setStatus( yaxp::MessageT::illegalParam );
                return false;
            }
            if( message.length != sizeof(int32_t) * pitchCount ) {
                message.setStatus( yaxp::MessageT::illegalParam );
                return false;
            }
            int32_t *pitch = (int32_t * )(&message.data[0]);
            for( auto vi=0u; vi < pitchCount; ++vi, ++pitch ) {
                toneShapers[vectorIndex].toneShaper[ vi ].pitch = *pitch;
            }
        }
        message.setStatusSetOk();
        return true;

#if 0
    case  TagToneShaper::SetOvertoneVector :
        TAG_DEBUG(TagToneShaper::SetOvertoneCount, tagIndex, paramIndex, "ToneShaperMatrix" );
        if( message.length == sizeof(ToneShaper) ) {
            const uint16_t vectorIndex      = message.getParam(paramIndex);
            const uint16_t overtoneCount    = message.getParam(++paramIndex);
            if( vectorIndex >= settingVectorSize ) {
                message.setStatus( yaxp::MessageT::illegalTargetIndex );
                return false;
            }
            auto &dst = toneShapers[vectorIndex].toneShaper;

            for( auto vi=0u; vi<overtoneCount; ++vi ) {
                dst[ vi ] = message
            }
            if( !message.setTargetData( dst, overtoneCount ) ) {
                message.setStatus( yaxp::MessageT::illegalDataLength );
                return false;
            }
            toneShapers[vectorIndex].oscillatorCountUsed = overtoneCount;
            message.setStatusSetOk();
            return false;
        }
        message.setStatus( yaxp::MessageT::illegalDataLength );
        return false;

    case  TagToneShaper::GetOvertoneVector :
        TAG_DEBUG(TagToneShaper::SetOvertoneCount, tagIndex, paramIndex, "ToneShaperMatrix" );
        if( message.size >= sizeof(ToneShaper) ) {
            const uint16_t vectorIndex      = message.getParam(paramIndex);
            const uint16_t overtoneIndex    = message.getParam(++paramIndex);
            if( overtoneIndex > overtoneCountOscDef ) {
                message.setStatus( yaxp::MessageT::illegalTargetIndex );
                return false;
            }
            if( vectorIndex > settingVectorSize-1 ) {
                message.setStatus( yaxp::MessageT::illegalTargetIndex );
                return false;
            }
            message.getTargetData(toneShapers[vectorIndex].toneShaper[overtoneIndex] );
            message.setStatusGetOk();
            return true;
        }
        message.setStatus( yaxp::MessageT::illegalDataLength );
        return false;
#endif
    default:
        break;
    }
    message.setStatus( yaxp::MessageT::illegalTag );
    return false;
}

bool ToneShaperMatrix::setPitchVector( int32_t *pitchVector, uint16_t pitchCount, uint16_t vectorIndex )
{
    if( vectorIndex >= settingVectorSize ) {
        return false;
    }
    if( pitchCount >= ToneShaperVector::toneShaperVectorSize ) {
        return false;
    }

    auto & tsv = toneShapers[ vectorIndex ];
    for( auto i=0u; i < pitchCount; ++i ) {
        tsv.toneShaper[ i ].pitch = pitchVector[ i ];
    }
    return true;
}


} // end namespace yacynth
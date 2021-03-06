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
 * File:   EffectBase.h
 * Author: Istvan Simon
 *
 * Created on April 6, 2016, 11:22 PM
 */
#include "yacynth_config.h"
#include "FxBase.h"
#include "yaio/CycleCount.h"

#include <chrono>
#include <sys/time.h>
#include <ctime>
#include <time.h>

// TODO split the file to 
//  FxBase.cpp
//  FxBaseRunner.cpp
//  FxBaseCollector.cpp

namespace yacynth {
using namespace TagEffectRunnerLevel_01;
using namespace TagEffectCollectorLevel_01;

// --------------------------------------------------------------------

bool FxRunner::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    const uint8_t tag = message.getTag(tagIndex);
    switch( TagEffectRunner(tag) ) {
        case TagEffectRunner::Clear : {
            TAG_DEBUG(TagEffectRunner::Clear, tagIndex, paramIndex, "FxRunner" );
            clear();
            message.setStatusSetOk();
            return true;
        }

        case TagEffectRunner::Fill : {
            TAG_DEBUG(TagEffectRunner::Fill, tagIndex, paramIndex, "FxRunner" );
            if( !message.checkParamIndex(paramIndex) )
                return false;
            const uint16_t countParam = message.getParam(paramIndex);
            if( countParam >= FxRunner::nodeCount ) {
                message.setStatus( yaxp::MessageT::illegalTargetIndex ); // TODO more specific
                return false;
            }
            if( countParam*sizeof(EffectRunnerFill) != message.length ) {
                message.setStatus( yaxp::MessageT::illegalDataLength ); // TODO more specific
                return false;
            }
            clear();
//            std::cout << "---- fill begin message.length " << message.length << std::endl;
            EffectRunnerFill *data = static_cast<EffectRunnerFill *>((void *)(message.data));
            for( uint16_t ind = 0; ind < countParam; ++ind, ++data ) {
                FxRunner::RET ret = add( data->fxIdOfFxCollector );
                if( RET::OK != ret ) {
//                    std::cout << "len " << uint16_t(message.data[ind])  << std::endl;
                    TAG_DEBUG( TagEffectRunner::Fill, tagIndex, paramIndex, "FxRunner -- TagEffectRunner::Fill  param data" );
                    message.setStatus( yaxp::MessageT::targetRetCode );
                    return false;
                }
            }
            std::cout << "---- fill end" << std::endl;
            message.setStatusSetOk();
            return true;
        }

        case TagEffectRunner::SetConnections : {
            TAG_DEBUG(TagEffectRunner::SetConnections, tagIndex, paramIndex, "FxRunner" );
            if( !message.checkParamIndex(paramIndex) )
                return false;
            const uint16_t countParam = message.getParam(paramIndex);
            if( countParam*sizeof(EffectRunnerSetConnections) != message.length ) {
                std::cout << "message.length " << message.length << std::endl;
                message.setStatus( yaxp::MessageT::illegalDataLength ); // TODO more specific
                return false;
            }

            const EffectRunnerSetConnections *data = static_cast<EffectRunnerSetConnections *>((void *)(&message.data[0]));

            clearConnections();
            std::cout << "TagEffectRunner::SetConnections message.length " << message.length << std::endl;
            std::cout << "TagEffectRunner::SetConnections countParam " << countParam << std::endl;
            for( uint16_t ind = 0; ind < countParam; ++ind, ++data ) {
                FxRunner::RET ret = connect( data->fxIdOfFxCollectorOutput, data->fxIdOfFxRunnerInput, data->inputId );
                if( RET::OK != ret  ) {
                    TAG_DEBUG( TagEffectRunner::Fill, tagIndex, paramIndex, "FxRunner connect error" );
                    message.setStatus( yaxp::MessageT::targetRetCode );
                    std::cout << "TagEffectRunner::SetConnections ind " << ind << std::endl;
                    return false;
                }
            }
            message.setStatusSetOk();
            return true;
        }

    case TagEffectRunner::GetEffectList : {
            TAG_DEBUG(TagEffectRunner::GetEffectList, tagIndex, paramIndex, "FxRunner" );
            const uint16_t listLength = count() * sizeof(EffectListEntry);
            if( listLength > message.size ) {
                message.setStatus( yaxp::MessageT::illegalDataLength );
                return false;
            }
            message.params[0] = count();
            EffectListEntry *data = static_cast<EffectListEntry *>((void *)(message.data));
            
            memset(message.data, 0, sizeof(message.data));
            uint16_t ind = 1;
            for( ; ind < count(); ++ind, ++data ) {
                data->fxIndex       = ind-1;                            // useless
                data->id            = nodes[ind].thp->id(); // needed
                data->fxType        = uint8_t( nodes[ind].thp->getType() ); // useless
                data->fxMaxMode     = nodes[ind].thp->getMaxMode();     // useless
                data->inputCount    = nodes[ind].thp->getInputCount();  // useless
                data->outputCount   = 1;            
                data->refId         = ind;  // needed
                data->instanceIndex = nodes[ind].thp->myInstanceIndex();// useless
            }
            // endMixer > ind=max, refId=0
            data->fxIndex       = ind-1;
            data->id            = nodes[0].thp->id();  // needed
            data->fxType        = uint8_t( nodes[0].thp->getType() );
            data->fxMaxMode     = nodes[0].thp->getMaxMode();
            data->inputCount    = nodes[0].thp->getInputCount();
            data->outputCount   = 0;
            data->refId         = 0;  // needed
            data->instanceIndex = nodes[0].thp->myInstanceIndex();
            
            message.setStatusGetOk(listLength);
            return true;
        }

    case TagEffectRunner::Preset : {
            TAG_DEBUG(TagEffectRunner::Preset, tagIndex, paramIndex, "FxRunner" );
            clearConnections();
            connect( 2, 0, 0 ); // oscillatormixer.00:00 to endmixer.00:00
            message.setStatusSetOk();
            return true;
        }
        return true;
    default:
        break;        
    }
    TAG_DEBUG( TagEffectRunner::Nop, tagIndex, paramIndex, "FxRunner -- illegal tag" );
    message.setStatus( yaxp::MessageT::illegalTag );
    return false;
}

// --------------------------------------------------------------------
} // end namespace yacynth
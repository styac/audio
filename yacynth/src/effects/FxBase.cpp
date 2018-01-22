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

#include    "FxBase.h"
#include    "yaio/CycleCount.h"

#include    <chrono>
#include    <sys/time.h>
#include    <ctime>
#include    <time.h>

namespace yacynth {
using namespace TagEffectRunnerLevel_01;
using namespace TagEffectCollectorLevel_01;

// --------------------------------------------------------------------

FxBase      fxNil("Nil",0,0,TagEffectType::FxNil);
uint16_t    FxBase::count = -1;
// template <typename T>  uint16_t S<T>::something_relevant = 0;

// --------------------------------------------------------------------

FxNode::FxNode()
:   thp(&fxNil)
{};

// --------------------------------------------------------------------
FxBase::~FxBase()
{
};

// --------------------------------------------------------------------

void FxBase::sprocessNop( void * data )
{
    return;
}
// --------------------------------------------------------------------
void FxBase::sprocessClear2Nop( void * data )
{
    FxBase& thp = * static_cast<FxBase *> ( data );
    thp.sprocessp = sprocessNop;
    thp.EIObuffer::clear();
}
// --------------------------------------------------------------------
void FxBase::sprocessFadeOut( void * data )
{
    FxBase& thp = * static_cast<FxBase *> ( data );
    thp.sprocessp = sprocessClear2Nop;
    thp.sprocesspCurr( data );
    thp.EIObuffer::fadeOut();
}
// --------------------------------------------------------------------
void FxBase::sprocessFadeIn( void * data )
{
    FxBase& thp = * static_cast<FxBase *> ( data ) ;
    thp.clearState(); // clears the internal state but not the output - new mode starts
    SpfT sprocessX( thp.sprocesspNext );
    thp.sprocessp = sprocessX;
    thp.sprocesspCurr = sprocessX;
    sprocessX( data );
    thp.EIObuffer::fadeIn();
}
// --------------------------------------------------------------------
// not real cross fade at the moment but fade out - fade in
void FxBase::sprocessCrossFade( void * data )
{
    FxBase& thp = * static_cast<FxBase *> ( data );
    thp.sprocessp = sprocessFadeIn;
    thp.sprocesspCurr( data );
    thp.EIObuffer::fadeOut();
}
// --------------------------------------------------------------------
// X -> 0:
//  k:      run old function    :  sprocessX2FadeOut
//          fade out
//  k+1:    clear               :  sprocessClear2Nop
//  k+2:    nop
//          wait 4 cycle

// 0 -> X
//  k:      run new function    :  sprocessFadeIn2X
//          fade in
//  k+1:    new function
//          wait 3 cycle

// X -> Y
//  k:      run old function    :  sprocessX2FadeOut
//          fade out
//  k+1     run new function    :  sprocessFadeIn2X
//          fade in
//  k+2:    new function
//          wait 4 cycle

bool FxBase::setProcessingMode( uint16_t mode )
{
    constexpr int waitCycle = 10;
    if( procMode == mode ) {
        return true; // no change
    }

    // check cycleCount if low then wait 10msec
    // set cycleCount
    auto currCycle = CycleCount::getInstance().get();
    if( currCycle < nextSetPocModeCycle ) {
        timespec req{0,1000LL*1000*10};
        timespec rem;
        // sleep(10sec)
        int r = nanosleep( &req, &rem );
        if( r != 0 ) {
            // TODO temp
            std::cout << " *** nanosleep error: " << errno << std::endl;
        }
    }
    nextSetPocModeCycle = CycleCount::getInstance().get() + waitCycle;
    return setSprocessNext( mode );
}

// --------------------------------------------------------------------

bool FxBase::connect( const FxBase * v, uint16_t ind )
{
     std::cout
         << "FxBase::connect - should never be called - override "
         << std::endl;
    return false;
}

// --------------------------------------------------------------------
// clear transient data - NOT settings

void FxBase::clearState()
{
    // EIObuffer::clear();
}

// --------------------------------------------------------------------

bool FxBase::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    TAG_DEBUG( TagEffectCollector::Nop, tagIndex, paramIndex,"FxBase/FxNil" );
    message.setStatus( yaxp::MessageT::noParameter ); // nothing to do here
    return false;
}

// --------------------------------------------------------------------

bool FxBase::setSprocessNext( uint16_t mode )
{
    sprocessp = sprocesspNext = FxBase::sprocessClear2Nop;
    return true;
}; 

// --------------------------------------------------------------------

// create a fullname : name + '.' + outputIndex ':' + instanceIndex
void FxCollector::getFullName( const FxBase &fxmaster, const FxBase &fxcurr, char * fullName, size_t nameLength )
{
    std::string resname(fxmaster.name());
    switch( fxcurr.id() ) {
    case 0:
        resname = "Nil";
        break;
    case 1:
        resname = "EndMixer";
        break;
    default:
        if( fxcurr.getMasterId() == 0 ) {
            resname += ".00:";
        } else {
            // slave
            resname += ".";
            int slaveOut = fxcurr.id() - fxmaster.id();
            if( slaveOut < 10 ) {
                resname += "0";
            }
            resname += std::to_string( slaveOut );
            resname += ":";
        }
        if( fxmaster.myInstanceIndex() < 10 ) {
            resname += "0";
        }
        resname += std::to_string(fxmaster.myInstanceIndex() );
    }
    strncpy( fullName, resname.data(), nameLength-1 );
}

// --------------------------------------------------------------------

// chop the 1st parameter as index in FxCollector

bool FxCollector::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    const uint8_t tag = message.getTag(tagIndex);
    switch( TagEffectCollector( tag ) ) {
    case TagEffectCollector::Clear :
        TAG_DEBUG(TagEffectCollector::Clear, tagIndex, paramIndex, "FxCollector" );
        message.setStatusSetOk();
        return true;

    case TagEffectCollector::SetProcessingMode :
        std::cout << "---- FxCollector setProcMode" << std::endl;
        TAG_DEBUG(TagEffectCollector::SetProcessingMode, tagIndex, paramIndex, "FxCollector" );
        if( message.checkParamIndex(paramIndex) && message.checkParamIndex(paramIndex+1) ) {
            const uint8_t effectInd = message.getParam(paramIndex);
            if( count() <= effectInd ) {
                message.setStatus( yaxp::MessageT::illegalParam );
                return false;
            }
            std::cout
                    << "---- FxCollector setProcMode call " << uint16_t(effectInd )
                    << " mode " << message.getParam(paramIndex+1)
                    <<  std::endl;
//            if( get(effectInd)->setProcMode(message.getParam(paramIndex+1)) ) {
//                message.setStatusSetOk();
//                return true;
//            }
            
            if( get(effectInd)->setProcessingMode(message.getParam(paramIndex+1)) ) {
                message.setStatusSetOk();
                return true;
            }
            
            
        }
        message.setStatus( yaxp::MessageT::illegalProcMode );
        return false;

    case TagEffectCollector::GetEffectList : {
            TAG_DEBUG(TagEffectCollector::GetEffectList, tagIndex, paramIndex, "FxCollector" );
            const uint16_t listLength = count() * sizeof(EffectListEntry);
            if( listLength > message.size ) {
                message.setStatus( yaxp::MessageT::illegalDataLength );
                return false;
            }
            message.params[0] = count();
            EffectListEntry *data = static_cast<EffectListEntry *>((void *)(message.data));
            int lastMaster = 0;
            for( uint16_t ind = 0; ind < count(); ++ind, ++data ) {
                data->fxIndex       = ind;
                data->id            = nodes[ind]->id();
                data->fxType        = uint8_t( nodes[ind]->getType() );
                data->dynamic       = nodes[ind]->isDynamic();
                data->fxMaxMode     = nodes[ind]->getMaxMode();
                data->inputCount    = nodes[ind]->getInputCount();
                data->masterId      = nodes[ind]->getMasterId();
                data->instanceIndex = nodes[ind]->myInstanceIndex();
                std::memset( data->name,0,data->nameLength );
                std::strncpy( data->name, nodes[ind]->name().data(), data->nameLength-1);
                if( nodes[ind]->getMasterId() == 0 ) {
                    getFullName( *nodes[ind], *nodes[ind], data->fullName, sizeof(data->fullName) );
                    lastMaster = ind;
                } else {
                    getFullName( *nodes[lastMaster], *nodes[ind], data->fullName, sizeof(data->fullName) );
                }
            }
            message.setStatusGetOk(listLength);
            return true;
        }

    case TagEffectCollector::EffectInstance : {
            TAG_DEBUG(TagEffectCollector::EffectInstance, tagIndex, paramIndex, "FxCollector" );
            if( !message.checkParamIndex(paramIndex) )
                return false;
            const uint8_t effectInd = message.getParam(paramIndex);
            if( count() <= effectInd ) {
                message.setStatus( yaxp::MessageT::illegalParam );
                return false;
            }
            return get(effectInd)->parameter( message, ++tagIndex, ++paramIndex ); // tag will be dispatched in the effect
        }

    case TagEffectCollector::CreateEffect : {
            if( factory( TagEffectType( message.getParam( paramIndex ) ) ) ) {
                message.setStatusSetOk();
                return true;
            }
            message.setStatus( yaxp::MessageT::illegalTagEffectType );
            return false;
        }

    case TagEffectCollector::DeleteEffects : {
            if( cleanup() ) {
                message.setStatusSetOk();
                return true;
            }
            message.setStatus( yaxp::MessageT::nothingToDo );
            return false;
        }
    default:
        break;        
    }
    message.setStatus( yaxp::MessageT::illegalTag );
    return false;
}

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

            EffectRunnerSetConnections *data = static_cast<EffectRunnerSetConnections *>((void *)(&message.data[0]));

            clearConnections();
            for( uint16_t ind = 0; ind < countParam; ++ind, ++data ) {
                FxRunner::RET ret = connect( data->fxIdOfFxCollectorOutput, data->fxIdOfFxRunnerInput, data->inputId );
                if( RET::OK != ret  ) {
                    TAG_DEBUG( TagEffectRunner::Fill, tagIndex, paramIndex, "FxRunner connect error" );
                    message.setStatus( yaxp::MessageT::targetRetCode );
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
            for( uint16_t ind = 0; ind < count(); ++ind, ++data ) {
                data->fxIndex       = ind;
                data->id            = nodes[ind].thp->id();
                data->fxType        = uint8_t( nodes[ind].thp->getType() );
                data->fxMaxMode     = nodes[ind].thp->getMaxMode();
                data->inputCount    = nodes[ind].thp->getInputCount();
                data->masterId      = nodes[ind].thp->getMasterId();
                data->instanceIndex = nodes[ind].thp->myInstanceIndex();
                std::memset( data->name,'\0',data->nameLength);
                std::strncpy( data->name,nodes[ind].thp->name().data() ,data->nameLength-1);
            }
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
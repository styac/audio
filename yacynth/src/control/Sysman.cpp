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
 * File:   Sysman.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on February 21, 2016, 8:31 AM
 */

#include    "Sysman.h"
#include    "Serialize.h"

namespace yacynth {
using namespace TagMainLevel_00;
using namespace TagToneShaperLevel_01;

using namespace TagEffectFactoryLevel_01;
using namespace TagEffectRunnerLevel_01;
using namespace TagMidiControllerLevel_01;
using namespace TagInnerControllerLevel_01;
using namespace TagRouterLevel_01;
using namespace TagTunerLevel_01;
using namespace TagEffectCollectorLevel_01;
using namespace TagEffectTypeLevel_02;
using namespace TagEffectFxFilterModeLevel_03;
using namespace TagEffectFxOscillatorMixerModeLevel_03;
using namespace TagEffectFxMixerModeLevel_03;
using namespace TagEffectFxModulatorModeLevel_03;
using namespace TagEffectFxOutNoiseModeLevel_03;
using namespace TagEffectFxOutOscillatorModeLevel_03;
using namespace TagEffectFxEchoModeLevel_03;
using namespace TagEffectFxReverbModeLevel_03;

// --------------------------------------------------------------------

Sysman::Sysman(
    OscillatorArray&    oscillatorArrayP,
    IOThread&           iOThreadP  )
:   oscillatorArray(    oscillatorArrayP )
,   iOThread(           iOThreadP )
,   toneShaperMatrix(   oscillatorArrayP.getToneShaperMatrix() )
{

} // end Sysman::Sysman

// --------------------------------------------------------------------

bool Sysman::eval( ip::tcp::socket&   socketAccept )
{
    boost::system::error_code   asioError;
    boost::asio::streambuf      inStreambuf;
    boost::asio::streambuf      outStreambuf(1<<14) ;
    std::ostream                ost(&outStreambuf);

    Yaxp::Message msgBuffer;

    try {


        // write( socketAccept, boost::asio::buffer(Yaxp::helloClient), asioError );


        // new
        // read 4 byte ProtoHeader
        // read max 64k according to length
        //
        //

        // obsolete
        while( boost::system::errc::success == asioError ) {
            std::size_t n = read_until( socketAccept, inStreambuf, "\n", asioError );
    //        std::size_t n = read_until( socketAccept, inStreambuf, "}", asioError );
            streambuf::const_buffers_type bufs = inStreambuf.data();
            std::string line( buffers_begin(bufs), buffers_begin(bufs) + n);
            inStreambuf.consume(n);


            std::cout << line << std::endl;
            ost << "\ngot\n" << line;
            // write back test
            boost::asio::write( socketAccept, outStreambuf, asioError );

        }
        return true;
    } catch (...) {
        return false;        
    }
} // end Sysman::exec



// --------------------------------------------------------------------
// msg format> LSB
// min 2 tags are mandatory
//
bool Sysman::evalParameterMessage( Yaxp::Message& msg )
{
    return parameter( msg, 0, 0 );
}

void Sysman::testParameter()
{
    Yaxp::Message msgBuffer;
    msgBuffer.setLength(50000);
    std::cout << "---- Sysman::testParameter begin -----\n" << std::endl;

#if 0    
        // test the tags
    for( auto i=7; i<13; ++i ) {
        for( auto j=1; j<6; ++j ) {
            for( auto k=1; k<6; ++k ) {
                msgBuffer.setTags(i,j,k,1);
                parameter( msgBuffer, 0, 0 );
            }
        }
    }
#endif
    
#if 0      
    std::cout << "---- Sysman::Effect -----\n" << std::endl;
    
    for( auto i=1; i<11; ++i ) {
        std::cout << " **** " << i << std::endl;
        msgBuffer.setTags( uint8_t( TagMain::Effect ), 4, i, 1 );
        msgBuffer.setPar(i);
        parameter( msgBuffer, 0, 0 );
    }
#endif
    
    msgBuffer.clear();
    msgBuffer.setLength(50000);    
    msgBuffer.setTags( uint8_t( TagMain::EffectRunner )
            ,uint8_t(TagEffectRunner::GetEffectList )    
            );

    parameter( msgBuffer, 0, 0 );
    
    if( msgBuffer.messageType == 0 ) {
        
        EffectListEntry *data = static_cast<EffectListEntry *>((void *)(msgBuffer.data));
        for( uint16_t ind = 0; ind < msgBuffer.length / sizeof(EffectListEntry); ++ind, ++data ) {
            std::cout 
                << "---- ind  "     << uint16_t(data->fxIndex)  
                << " id "           << uint16_t(data->id)                     
                << " type "         << uint16_t(data->fxType) 
                << " maxMode "      << uint16_t(data->fxMaxMode) 
                << " inputCount "   << uint16_t(data->inputCount ) 
                << " masterId "     << uint16_t(data->masterId)                     
                << "  "             << data->name
                << std::endl;            
        }
                
        
    } else {
        std::cout << "---- error " << msgBuffer.messageType<< std::endl;
    }

    msgBuffer.clear();
    msgBuffer.setLength(50000);    
    msgBuffer.setTags( uint8_t( TagMain::Effect )
            ,uint8_t(TagEffectCollector::GetEffectList )    
            );

    parameter( msgBuffer, 0, 0 );
    
    if( msgBuffer.messageType == 0 ) {
        
        EffectListEntry *data = static_cast<EffectListEntry *>((void *)(msgBuffer.data));
        for( uint16_t ind = 0; ind < msgBuffer.length / sizeof(EffectListEntry); ++ind, ++data ) {
            std::cout 
                << "---- ind  "     << uint16_t(data->fxIndex)  
                << " id "           << uint16_t(data->id)                     
                << " type "         << uint16_t(data->fxType) 
                << " maxMode "      << uint16_t(data->fxMaxMode) 
                << " inputCount "   << uint16_t(data->inputCount ) 
                << " masterId "     << uint16_t(data->masterId)                     
                << "  "             << data->name
                << std::endl;            
        }
                
        
    } else {
        std::cout << "---- error " << msgBuffer.messageType<< std::endl;
    }
    
    msgBuffer.clear();
    msgBuffer.setLength(50000);    
    msgBuffer.setTags( uint8_t( TagMain::Effect )
            ,uint8_t(TagEffectCollector::EffectInstance )    
            ,uint8_t( 1 )       // type
            ,uint8_t( 0 )    
//            ,uint8_t( ) 
            );
    msgBuffer.setPar(0);        // index
    msgBuffer.setStatus();
    parameter( msgBuffer, 0, 0 );
    
    
    msgBuffer.setTags( uint8_t( TagMain::Effect )
            ,uint8_t(TagEffectCollector::EffectInstance )    
            ,uint8_t( 3 )    
            ,uint8_t( 0 )    
//            ,uint8_t( ) 
            );
    msgBuffer.setPar(1);       
    msgBuffer.setStatus();
    parameter( msgBuffer, 0, 0 );
    
    msgBuffer.setTags( uint8_t( TagMain::Effect )
            ,uint8_t(TagEffectCollector::EffectInstance )    
            ,uint8_t( 4 )    
            ,uint8_t( 0 )    
//            ,uint8_t( ) 
            );
    msgBuffer.setPar(2);       
    msgBuffer.setStatus();
    parameter( msgBuffer, 0, 0 );

    msgBuffer.setTags( uint8_t( TagMain::Effect )
            ,uint8_t(TagEffectCollector::EffectInstance )    
            ,uint8_t( 6 )    
            ,uint8_t( 0 )    
//            ,uint8_t( ) 
            );
    msgBuffer.setPar(3);       
    msgBuffer.setStatus();
    parameter( msgBuffer, 0, 0 );

    msgBuffer.setTags( uint8_t( TagMain::Effect )
            ,uint8_t(TagEffectCollector::EffectInstance )    
            ,uint8_t( 7 )    
            ,uint8_t( 0 )    
//            ,uint8_t( ) 
            );
    msgBuffer.setPar(4);       
    msgBuffer.setStatus();
    parameter( msgBuffer, 0, 0 );
    
    msgBuffer.setTags( uint8_t( TagMain::Effect )
            ,uint8_t(TagEffectCollector::EffectInstance )    
            ,uint8_t( 2 )    
            ,uint8_t( 0 )    
//            ,uint8_t( ) 
            );
    msgBuffer.setPar(5);       
    msgBuffer.setStatus();
    parameter( msgBuffer, 0, 0 );

    msgBuffer.setTags( uint8_t( TagMain::Effect )
            ,uint8_t(TagEffectCollector::EffectInstance )    
            ,uint8_t( 2 )    
            ,uint8_t( 0 )    
//            ,uint8_t( ) 
            );
    msgBuffer.setPar(6);       
    msgBuffer.setStatus();
    parameter( msgBuffer, 0, 0 );
    
    msgBuffer.setTags( uint8_t( TagMain::Effect )
            ,uint8_t(TagEffectCollector::EffectInstance )    
            ,uint8_t( 2 )    
            ,uint8_t( 0 )    
//            ,uint8_t( ) 
            );
    msgBuffer.setPar(7);       
    msgBuffer.setStatus();
    parameter( msgBuffer, 0, 0 );
    
    msgBuffer.setTags( uint8_t( TagMain::Effect )
            ,uint8_t(TagEffectCollector::EffectInstance )    
            ,uint8_t( 5 )    
            ,uint8_t( 0 )    
//            ,uint8_t( ) 
            );
    msgBuffer.setPar(8);       
    msgBuffer.setStatus();
    parameter( msgBuffer, 0, 0 );
    
    msgBuffer.setTags( uint8_t( TagMain::Effect )
            ,uint8_t(TagEffectCollector::EffectInstance )    
            ,uint8_t( 8 )   // filter
            ,uint8_t( 1 )   // clear filter
//            ,uint8_t( ) 
            );
    msgBuffer.setPar(9);    // effect index=9 
    msgBuffer.setStatus();
    parameter( msgBuffer, 0, 0 );

    msgBuffer.setTags( uint8_t( TagMain::Effect )
            ,uint8_t(TagEffectCollector::EffectInstance )    
            ,uint8_t( 8 )   // filter
            ,uint8_t( 2 )   // SetMode_01_ap22x4x filter
//            ,uint8_t( ) 
            );
    msgBuffer.setPar(9);    // effect index=9 
    msgBuffer.setStatus();
    parameter( msgBuffer, 0, 0 );

    std::cout << "---- Sysman::ToneShaper test  -----\n" << std::endl;
    
    msgBuffer.setTags( uint8_t( TagMain::ToneShaper )
            ,uint8_t(TagToneShaper::SetOvertone )    
            );
    msgBuffer.setPar(0,0);    // voice,overtone
    
    
    ToneShaper ts;
    msgBuffer.getTargetData(ts);    
    msgBuffer.setStatus();
    bool res1 = parameter( msgBuffer, 0, 0 );
    
    msgBuffer.length += 1;
    
    msgBuffer.setStatus();
    bool res2 = parameter( msgBuffer, 0, 0 );
    
    msgBuffer.length -= 2;
    
    msgBuffer.setStatus();
    bool res3 = parameter( msgBuffer, 0, 0 );
    
    std::cout << "---- res1 " << res1 << " res2 " << res2 << " res3 " << res3 << std::endl;

    std::cout << "---- Sysman::testParameter exit -----\n" << std::endl;
    exit(0);    
}
    
    
// --------------------------------------------------------------------
bool Sysman::parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex ) 
{
    const uint8_t tag = message.getTag(tagIndex);
    switch( TagMain( tag ) ) {

    case TagMain::Hello:
        TAG_DEBUG(TagMain::Hello, tagIndex, paramIndex, "Sysman" );
        // call send back Hello - enable other msgs
        return true;
        
    case TagMain::Bye:
        TAG_DEBUG(TagMain::Hello, tagIndex, paramIndex, "Sysman" );
        // call send back Bye - do shutdown TCP disable other msgs
        return true;
        
    case TagMain::ToneShaper:
        TAG_DEBUG(TagMain::ToneShaper, tagIndex, paramIndex, "Sysman" );
        return toneShaperMatrix.parameter( message,++tagIndex, paramIndex );

    case TagMain::EffectRunner:
        TAG_DEBUG(TagMain::EffectRunner, tagIndex, paramIndex, "Sysman" );
        return iOThread.getFxRunner().parameter( message,++tagIndex, paramIndex );

    case TagMain::Effect:
        TAG_DEBUG(TagMain::Effect, tagIndex, paramIndex, "Sysman" );
        return FxCollector::getInstance().parameter( message,++tagIndex, paramIndex );

    case TagMain::MidiController:
        TAG_DEBUG(TagMain::MidiController, tagIndex, paramIndex, "Sysman" );
        return iOThread.getRouter().getMidiController().parameter( message,++tagIndex, paramIndex );

    case TagMain::InnerController:
        TAG_DEBUG(TagMain::InnerController, tagIndex, paramIndex, "Sysman" );
        return InnerController::getInstance().parameter( message,++tagIndex, paramIndex );

    case TagMain::Router:
        TAG_DEBUG(TagMain::Router, tagIndex, paramIndex, "Sysman" );
        return iOThread.getRouter().parameter( message,++tagIndex, paramIndex );

    case TagMain::Mute:
        TAG_DEBUG(TagMain::Mute, tagIndex, paramIndex, "Sysman" );
        YaIoJack::getInstance().mute();
        return true;

    case TagMain::UnMute:
        TAG_DEBUG(TagMain::UnMute, tagIndex, paramIndex, "Sysman" );
        YaIoJack::getInstance().unmute();
        return true;

    case TagMain::Tuner: // TODO
        TAG_DEBUG(TagMain::Tuner, tagIndex, paramIndex, "Sysman" );
        return false;
    }
    TAG_DEBUG(message.getTag(tagIndex), tagIndex, paramIndex, "Sysman" );
    message.setStatus( Yaxp::MessageT::illegalTag, tag );
    return false; // error  
}

// --------------------------------------------------------------------

} // end namespace yacynth



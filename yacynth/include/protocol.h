#pragma once

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
 * File:   protocol.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on February 20, 2016, 10:15 AM
 */

#include    <cstdint>
#include    <string>
#include    <cstring>
#include    <array>
// #include    <iomanip>
// #include    <iostream>

namespace yacynth {
//
// yacynth exchange protocol
// read:
// 1. read Yaxp::Header
// 2. switch( cmd )
// 3. read data length=Header.length (ReqSet)
//
namespace yaxp {

constexpr uint16_t minCOntrolPort = 5000;
constexpr uint16_t maxCOntrolPort = 50000;
constexpr uint16_t seedLength = 32;
constexpr uint16_t remoteDefaultPort = 7373;

const char * const configPathName = ".local/share/";
const char * const configDirName  = ".yacconfig";
const char * const profileDirName = "default";
const char * const authKeyName    = ".yyauth.key";
const char * const confName       = "conf";
const char * const dbDir          = "db";

const char * const localDefaultPort = "/tmp/.yacynth_connport";
const char * const localPortControlSuffix = "_control";   // local AF_LOCAL suffix for control
const char * const localPortStatusSuffix  = "_status";    // local AF_LOCAL suffix for status

//
// remote:
//      control port = status port
//      synth engine TCP server for control (requestC2E)
//      gui UDP server for status   (adviceE2C)
//      IP4,IP6 must be configured by options (IP6 not implemented)
//
// local
//      control port = port + localPortControlSuffix
//      status port = port + localPortStatusSuffix
//      synth engine SOCK_STREAM server for control (requestC2E)
//      gui SOCK_DGRAM server for status    (adviceE2C)
//      IP4,IP6 must be configured by options (IP6 not implemented)
//
//

enum class CONN_MODE {
    CONNECTION_LOCAL,      // AF_UNIX or PIPE
    CONNECTION_REMOTE,     // AF_INET
};

enum class MessageT : uint8_t {
    nop,                // 0 is illegal    
    // responses
    responseSetOK,    
    heartbeatResponse,              // nop
    // authentication response from client to server
    authRequestExpected,            // by Controller from Engine
    // error responses
    noParameter,                    // 130
    targetRetCode,                  // 131
    illegalTag,                     // 132
    illegalTagEffectType,           // 133
    illegalParamIndex,              // 134
    illegalParam,                   // 135
    illegalDataLength,              // 136
    illegalData,                    // 137
    illegalTargetIndex,             // 138
    illegalEffectCollectorIndex,    // 139
    illegalEffectRunnerIndex,       // 140  - too high
    illegalEffectInputIndex,        // 141  - no such input
    illegalProcMode,                // 142
    dataCheckError,                 // 143
    illegalContext,                 // 144
    nothingToDo,                    // 145
    internalError,                  // 146

    // short messages - only header : length is not used
    shortCommands = 0x40,
    stopServer,         // rename stopEngine
    heartbeatRequest,
    
    // full messages
    validLength = 0x80, // the length field is valid from this
    responseGetOK  = 0x81,          // data length is valid

    authRequest,    // authentication request from server to client
    authResponse,   // authentication response from client to server
    requestC2E,     // request  : controller to engine
    requestE2C,     // request  : engine to controller -- not needed
    adviceC2E,      // advice   : controller to engine -- not needed
    adviceE2C,      // advice   : engine to controller
    
    // UDP channel -- maybe adviceE2C subcommands
    statusE2C = 0x84,
    errorStatusE2C, // 
    logStrE2C,      // log string 
    statisticsE2C,  // fix struct
    amplitudeE2C,   // fix struct 
    mididataE2C,    // fix struct 
};

struct alignas(16) Header
{
    static constexpr std::uint8_t  maxTagCount  = 6;
    static constexpr std::uint8_t  maxParCount  = 3;

    void clear()
    {
        std::memset(this,0,sizeof(*this));
    }

    void setStatus( MessageT stCode = MessageT::nop )
    {
        messageType = stCode ;
    }

    void setStatusSetOk()
    {
        messageType = MessageT::responseSetOK;
        length      = 0;
    }

    void setStatusGetOk( uint16_t val = 0 )
    {
        messageType = MessageT::responseGetOK;
        if( val ) {
            length  = val;
        }
    }

    uint8_t getTag( uint8_t index )
    {
        if( index < maxTagCount )
            return tags[index];
        setStatus( MessageT::illegalTag );
        return 0;
    }
    uint16_t getParam( uint8_t index ) {
        return params[index];
    }

    // index : current value
    // count : additional needed params
    bool checkParamIndex( uint8_t index, uint8_t count=0 )
    {
        if( (index+count)  < maxParCount )
            return true;
        setStatus(MessageT::illegalParamIndex);
        return false;
    }

    inline void setLength( uint16_t len )
    {
        length = len;
    }

    inline void setTags( uint8_t tag0 )
    {
        tags[0] = tag0;
        tags[1] = 0;
    }

    inline void setTags( uint8_t tag0, uint8_t tag1 )
    {
        tags[0] = tag0;
        tags[1] = tag1;
        tags[2] = 0;
    }

    inline void setTags( uint8_t tag0, uint8_t tag1, uint8_t tag2 )
    {
        tags[0] = tag0;
        tags[1] = tag1;
        tags[2] = tag2;
        tags[3] = 0;
    }
    inline void setTags( uint8_t tag0, uint8_t tag1, uint8_t tag2, uint8_t tag3 )
    {
        tags[0] = tag0;
        tags[1] = tag1;
        tags[2] = tag2;
        tags[3] = tag3;
        tags[4] = 0;
    }

    inline void setPar( uint16_t par0 )
    {
        params[0] = par0;
    }

    inline void setPar( uint16_t par0, uint16_t par1 )
    {
        params[0] = par0;
        params[1] = par1;
    }
    inline void setPar( uint16_t par0, uint16_t par1, uint16_t par2 )
    {
        params[0] = par0;
        params[1] = par1;
        params[2] = par2;
    }
    std::string& print( std::string& str )
    {
        str = "msg: seq ";
        str += std::to_string(sequenceNr);
        str += " mT ";
        str += std::to_string(uint8_t(messageType));
        str += " tag ";
        str += std::to_string(tags[0]);
        str += " ";
        str += std::to_string(tags[1]);
        str += " ";
        str += std::to_string(tags[2]);
        str += " ";
        str += std::to_string(tags[3]);
        str += " ";
        str += std::to_string(tags[4]);
        str += " ";
        str += std::to_string(tags[5]);
        str += " par ";
        str += std::to_string(params[0]);
        str += " ";
        str += std::to_string(params[1]);
        str += " ";
        str += std::to_string(params[2]);
        str += " len ";
        str += std::to_string(length);
        return str;
    }
    // members
    uint8_t     sequenceNr;     // always inc by requester
    MessageT    messageType;    // tag (-1 :-)
    uint8_t     tags[ maxTagCount ];
    union {
        uint64_t        shortmsg;       // single 8 byte message
        struct {
            uint16_t    saddr[ 2 ];    // set a single param with max 2 addresses
            float       sparam;
        };
        struct {
            uint16_t    params[ maxParCount ];
            uint16_t    length;
        };
    };
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

struct Message : public Header
{
    static constexpr std::size_t    size = 1<<16;
    static constexpr std::uint8_t   headerSize = sizeof(Header);

    template<typename T>
    void getTargetData(const T& d)
    {
        constexpr std::size_t tsize = sizeof(T);
 //       std::cout << "-----  getTargetData sizeof " << std::dec << tsize << std::endl;
        static_assert(tsize < size ,"data size too big" );
        length = tsize;
        *((T*)data)  = d;
    }

    // to init with an array of something
    template<typename T, std::size_t N>
    void getTargetData(const T(&d)[N])
    {
        constexpr std::size_t tsize = sizeof(T)*N;
        using tarray = std::array<uint8_t,tsize>; // avoid const-ness of input type - maybe dirty
//        std::cout << "-----  getTargetData sizeof " << std::dec << tsize << std::endl;
        static_assert(tsize < size ,"data size too big" );
        length = tsize;
        *((tarray *)data)  = *((tarray*)&d);
    }

    template<typename T>
    bool setTargetData(T& d)
    {
        constexpr std::size_t tsize = sizeof(T);
 //       std::cout << "-----  setTargetData sizeof " << std::dec << tsize << " length " << length << std::endl;
        static_assert(tsize < size ,"data size too big" );
        if( length != tsize ) {
            return false;
        }
        d = *((T*)data);
        return true;
    }

    // if T has a check() method
    // check in the message
    // d is not used
    template<typename T>
    bool checkTargetData(const T& d)
    {
        constexpr std::size_t tsize = sizeof(T);
//        std::cout << "-----  checkTargetData sizeof " << std::dec << tsize << " length " << length << std::endl;
        static_assert(tsize < size ,"data size too big" );
        if( length != tsize ) {
            return false;
        }
        return (*((T*)data)).check();
    }

    template<typename T>
    bool addMessageData(T& d, bool first=false)
    {
        constexpr std::size_t tsize = sizeof(T);
//        std::cout << "-----  getTargetData sizeof " << std::dec << tsize << std::endl;
        static_assert(tsize < size ,"data size too big" );
        if( first ) {
            length=0;
        }
        if( (length + tsize) >= size ) {
            return false;
        }
        *((T*)&data[length])  = d;
        length += tsize;
        return true;
    }

    template<typename T1, typename T2>
    bool setTargetData(T1& d1, T1& d2)
    {
        constexpr std::size_t tsize = sizeof(T1) + sizeof(T2);
//        std::cout << "-----  setTargetData sizeof " << std::dec << tsize << " length " << length << std::endl;
        static_assert(tsize < size ,"data size too big" );
        if( length != tsize ) {
            return false;
        }
        d1 = *((T1*)data);
        d2 = *((T2*)(data+sizeof(T1)));
        return true;
    }

    uint8_t     data[ size ];
};

#pragma GCC diagnostic pop

struct alignas(16) StatusMessage : public Header
{
    union {
        char    strBuf[ 256 ];  // null terminated string
        float   amplitude[ 32 ][ 2 ]; // 32 stereo amplitude
        // statistics
        // midi control
    };       
};

} // end namespace Yaxp
} // end namespace yacynth


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

constexpr uint16_t  defaultPort = 7373;
const char * const homeDir      = ".yacynth";       // $HOME/.yacynth
const char * const dbDir        = "db";             // $HOME/.yacynth/db
const char * const seedFile     = ".yaxp.seed";     // $HOME/.yacynth/.yaxp.seed
const char * const config       = "config";         // $HOME/.yacynth/config

enum class MessageT : uint8_t {
    // short messages - only header > length is not used
    nop,                // 0 is illegal
    stopServer,         // stop
    heartbeatRequest,
    heartbeatResponse,

    // full messages
    validLength = 0x40, // the length field is valid from this
    
    authRequest,    // authentication request from server to client
    authResponse,   // authentication response from client to server
    authError,      // authentication response from client to server
    requestC2E,     // request  : controller to engine
    requestE2C,     // request  : engine to controller
    adviceC2E,      // advice   : controller to engine
    adviceE2C,      // advice   : engine to controller

    // responses
    responseSetOK = 0x80,           // data length = 0 -- not used yet "nop" is used
    responseGetOK,                  // data length is valid
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
        uint64_t        smsg;
        struct {
            uint16_t    params[ maxParCount ];
            uint16_t    length;
        };
    };
};

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


} // end namespace Yaxp
} // end namespace yacynth


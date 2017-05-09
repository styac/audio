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
#include    "Serialize.h"


namespace yacynth {
//
// yacynth exchange protocol
// read:
// 1. read Yaxp::Header
// 2. switch( cmd )
// 3. read data length=Header.length (ReqSet)
//
namespace Yaxp {

enum class MessageT : uint8_t {
    nop,
    requestC2E,     // request controller to engine
    requestE2C,     // request engine to controller
    adviceC2E,      // advice controller to engine
    adviceE2C,      // advice engine to controller

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
};

struct Header
{
    static constexpr std::uint8_t  version = 1;
    static constexpr std::uint8_t  maxTagCount = 6; // 4 might be enough
    static constexpr std::uint8_t  maxParCount = 6; // 2 might be enough

    void clear()
    {
        *this = {0};
    }

    void setStatus( MessageT stCode = MessageT::nop, uint8_t hint=0 )
    {
        messageType = uint8_t(stCode) ;
        statusHint = hint;
    }

    uint8_t getTag( uint8_t index )
    {
        if( index < maxTagCount )
            return tags[index];
        setStatus( MessageT::illegalTag, index );
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
#if 0

    inline void setTags( uint8_t tag0, uint8_t tag1, uint8_t tag2, uint8_t tag3, uint8_t tag4 )
    {
        tags[0] = tag0;
        tags[1] = tag1;
        tags[2] = tag2;
        tags[3] = tag3;
        tags[4] = tag4;
        tags[5] = 0;
    }
#endif
    inline void setPar( uint16_t par0 )
    {
        params[0] = par0;
    }

    inline void setPar( uint16_t par0, uint16_t par1 )
    {
        params[0] = par0;
        params[1] = par1;
    }
#if 0
    inline void setPar( uint16_t par0, uint16_t par1, uint16_t par2 )
    {
        params[0] = par0;
        params[1] = par1;
        params[2] = par2;
    }

    inline void setPar( uint16_t par0, uint16_t par1, uint16_t par2, uint16_t par3 )
    {
        params[0] = par0;
        params[1] = par1;
        params[2] = par2;
        params[3] = par3;
    }

    inline void setPar( uint16_t par0, uint16_t par1, uint16_t par2, uint16_t par3, uint16_t par4 )
    {
        params[0] = par0;
        params[1] = par1;
        params[2] = par2;
        params[3] = par3;
        params[4] = par4;
    }

    inline void setPar( uint16_t par0, uint16_t par1, uint16_t par2, uint16_t par3, uint16_t par4, uint16_t par5 )
    {
        params[0] = par0;
        params[1] = par1;
        params[2] = par2;
        params[3] = par3;
        params[4] = par4;
        params[5] = par5;
    }
#endif
    // members
    uint8_t     messageType;    // -> enum  - ok
    uint8_t     statusHint;     // eg. tag index
    uint8_t     tags[maxTagCount];
    uint16_t    params[maxParCount];
    uint16_t    length;
};

struct Message : public Header
{
    static constexpr std::size_t  size = 1<<16;

    template<typename T>
    void getTargetData(const T& d)
    {
        constexpr std::size_t tsize = sizeof(T);
        std::cout << "-----  getTargetData sizeof " << std::dec << tsize << std::endl;
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
        std::cout << "-----  getTargetData sizeof " << std::dec << tsize << std::endl;
        static_assert(tsize < size ,"data size too big" );
        length = tsize;
        *((tarray *)data)  = *((tarray*)&d);
    }

    template<typename T>
    bool setTargetData(T& d)
    {
        constexpr std::size_t tsize = sizeof(T);
        std::cout << "-----  setTargetData sizeof " << std::dec << tsize << " length " << length << std::endl;
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
        std::cout << "-----  checkTargetData sizeof " << std::dec << tsize << " length " << length << std::endl;
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
        std::cout << "-----  getTargetData sizeof " << std::dec << tsize << std::endl;
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
        std::cout << "-----  setTargetData sizeof " << std::dec << tsize << " length " << length << std::endl;
        static_assert(tsize < size ,"data size too big" );
        if( length != tsize ) {
            return false;
        }
        d1 = *((T1*)data);
        d2 = *((T2*)(data+sizeof(T1)));
        return true;
    }

    uint8_t     data[size];
};

inline void serialize( YsifOutStream& ser, const Message& val )
{
    serializeObjBeg( ser, "Yaxp::Message" );
    for( uint32_t index = 0; (index < Message::maxTagCount)  && (val.tags[index] > 0 ); ++index ) {
        serializeVecBeg( ser, index, "indexTag", Message::maxTagCount-1 );
        serialize(ser, val.tags[index], "commandTag");
        serializeVecEnd( ser, val.tags[index] == 0 );
    }
    serializeObjEnd( ser );
};

inline bool deserialize( YsifInpStream& ser, Message& val )
{
    bool ret    = true;
    bool endVec = false;
    uint32_t index = 0;
    uint32_t x = 0;
    val.data[0] = 0;
    ret = ret && deserializeObjBeg( ser, "Yaxp::Message" );
    while( !endVec && (++index < Message::maxTagCount) && ret ) {
        // std::cerr << "deserialize  " <<  index << " " <<  x << " " << endVec << " ret: " << ret <<  std::endl;
        ret = ret && deserializeVecBeg( ser, x, "indexTag", Message::maxTagCount );
        ret = ret && deserialize(ser, val.data[index], "commandTag");
        ret = ret && deserializeVecEnd( ser, endVec );
    }
    val.data[0] = index;
    ret = ret && deserializeObjEnd( ser );
    return ret;
};


} // end namespace Yaxp
} // end namespace yacynth


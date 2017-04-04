#pragma once

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
 * File:   Serialize.h
 * Author: Istvan Simon
 *
 * Created on April 3, 2016, 9:45 AM
 */

#include    <cstdint>
#include    <string>
#include    <array>
#include    <map>
#include    <sstream>
#include    <iostream>
#include    <iomanip>
#include    <limits>
#include    <ios>


namespace yacynth {

//#define DEBUG_SER_ON
#ifdef DEBUG_SER_ON

#define DEBUG_SER(name) \
    std::cerr << "** ser: " << name << std::endl;

#define DEBUG_DESER(name) \
    std::cerr << "** deser: " << name << std::endl;

#else

#define DEBUG_SER(name)
#define DEBUG_DESER(name)

#endif

constexpr int spaceStrSize = 64;
extern const char spaceStr[spaceStrSize+1];
extern const char *const lastVecStr;
extern const char *const contVecStr;
extern const char *const localSym;
extern const char *const emptyToken;


typedef std::map<std::string,int64_t> SymbolMapT;


template< class StreamT >
class YsifOutStreamT : public StreamT {
public:
    static constexpr int16_t  maxTab = 32;
    YsifOutStreamT()
    :   StreamT()
    ,   tlen(0)
    {}

    ~YsifOutStreamT()
    {}

    void ObjBeg( const char *name )
    { this->write(spaceStr,inct()); *this << "{ " << std::setw(0) << name << std::endl; };

    void ObjEnd()
    { this->write(spaceStr,dect()); *this << "}" << std::endl; };

    void VecBegin(uint16_t ind, const char * const name, uint32_t maxIndex )
    {
        this->write(spaceStr,inct());
        *this << "[ " << name << " " << ind << " # maxIndex " << maxIndex << std::endl;
    };

    void VecEnd( bool last )
    { 
        this->write(spaceStr,dect());  
        if(last)
            *this << lastVecStr << std::endl; 
        else
            *this << contVecStr << std::endl; 
    };

    YsifOutStreamT& OutValue( const char *name )
    { this->write(spaceStr,tlen);  *this << ": " << name << " " ; return *this; };

private:
    uint8_t dect()
    { return tlen <= 0 ? tlen=0:--tlen; }

    uint8_t inct()
    { return tlen >= maxTab ? tlen=maxTab:tlen++; }

    int16_t tlen; // tab length
};


typedef YsifOutStreamT<std::stringstream> YsifOutStream;

template< class StreamT >
class YsifInpStreamT : public StreamT {
public:
    YsifInpStreamT()
    :   StreamT()
    ,   prefixToken()
    ,   name()
    ,   symbolMap()
    {}

    ~YsifInpStreamT()
    {}

    // if status good the goto end of line
    bool toEndl()
    {
        if(this->good()) {
            this->ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return true;
        }
        return false;
    }

    //
    // '#' comment
    //
    const std::string& getPrefix()
    {
        while(1) {
            *this >> prefixToken;
            if( this->eof() ) {
                prefixToken = emptyToken;
                return prefixToken;
            }
            if( prefixToken[0] == '#' ) {
                toEndl();
                continue;
            }
            if( prefixToken == localSym ) {
                if( !storeSymbol() ) {
                    prefixToken = emptyToken;
                    return prefixToken;
                }
                continue;
            } 
            return prefixToken;                       
        }
    }

    // % name value
    bool storeSymbol()
    {
        int64_t tmp;
        *this >> name;
        *this >> tmp;  
        std::cerr << "storeSymbol  " << name << " " << tmp << std::endl;
        if( toEndl() ) {
            try {
                symbolMap.insert(std::make_pair(name,tmp));
            } catch(...) {
                std::cerr << "error  " <<  prefixToken << " " << name << " " << tmp << std::endl;                
            }
            for( auto x : symbolMap ) std::cerr<< "map " << x.first << " "  << x.second <<std::endl;
            return true;
        }
        std::cerr << "error  " << name << " " << tmp << std::endl;                
        return false;
    }

    bool checkName( const char * const val )
    {
        *this >> name;
        // std::cerr << "in checkName " <<  name << std::endl;
        if( this->eof() ) {
            return false;
        }
        //std::cerr << "out checkName " <<  name << std::endl;
        return name == val;
    }

    bool checkCmd( const char * const prefix, const char * const name )
    {
        return (getPrefix() == prefix) && (checkName(name)) ;
    }

    bool checkCmd( const char * const prefix )
    {
        return getPrefix() == prefix ;
    }

    bool checkPrefix( const char * const prefix )
    {
        return prefixToken == prefix ;
    }

    bool VecEnd( bool& last )
    { 
        getPrefix(); 
        // std::cerr << "VecEnd  " <<  prefixToken  << std::endl;  
        if( prefixToken == lastVecStr ) {
            last = true;
            return toEndl();            
        } else if( prefixToken == contVecStr ){
            last = false;
            return toEndl();
        } else {
            return false;
        }
    };

 private:
    std::string prefixToken;
    std::string name;
    SymbolMapT  symbolMap;
};

typedef YsifInpStreamT<std::stringstream> YsifInpStream;
// -------- serialize --------


inline YsifOutStream& serializeComment( YsifOutStream& ser )
{
    ser << " # "; return ser;
};

inline void serializeObjBeg( YsifOutStream& ser, const char * const name )
{
    ser.ObjBeg(name);
};

inline void serializeObjEnd( YsifOutStream& ser )
{
    ser.ObjEnd();
};

inline void serializeVecBeg( YsifOutStream& ser, const uint32_t val, const char * const name, uint32_t maxIndex )
{
    ser.VecBegin(val,name,maxIndex);
};

inline void serializeVecEnd( YsifOutStream& ser, bool last )
{
    ser.VecEnd(last);
};

inline void serialize( YsifOutStream& ser, const uint8_t& val, const char * const name  )
{
    ser.OutValue(name) << uint16_t(val) << std::endl;
};

inline void serialize( YsifOutStream& ser, const int8_t& val, const char * const name )
{
    ser.OutValue(name) << int16_t(val) << std::endl;
};

inline void serialize( YsifOutStream& ser, const uint16_t& val, const char * const name )
{
    ser.OutValue(name) << val << std::endl;
};

inline void serialize( YsifOutStream& ser, const int16_t& val, const char * const name )
{
    ser.OutValue(name) << val << std::endl;
};

inline void serialize( YsifOutStream& ser, const uint32_t& val, const char * const name )
{
    ser.OutValue(name) << val << std::endl;
};

inline void serialize( YsifOutStream& ser, const int32_t& val, const char * const name )
{
    ser.OutValue(name) << val << std::endl;
};

inline void serialize( YsifOutStream& ser, const uint64_t& val, const char * const name )
{
    ser.OutValue(name) << val << std::endl;
};

inline void serialize( YsifOutStream& ser, const int64_t& val, const char * const name )
{
    ser.OutValue(name) << val << std::endl;
};

inline void serialize( YsifOutStream& ser, const float& val, const char * const name )
{
    ser.OutValue(name) << val << std::endl;
};

inline void serialize( YsifOutStream& ser, const double& val, const char * const name )
{
    ser.OutValue(name) << val << std::endl;
};

inline void serialize( YsifOutStream& ser, const std::string& val, const char * const name )
{
    ser.OutValue(name) << val << std::endl;
};

// -------- deserialize --------
inline bool deserializeObjBeg( YsifInpStream& ser, const char * const name )
{
    DEBUG_DESER(name);
    return ser.checkCmd("{",name) && ser.toEndl();
};

inline bool deserializeObjEnd( YsifInpStream& ser )
{
    DEBUG_DESER("ObjEnd");
    return ser.checkCmd("}") && ser.toEndl();
};

// false may mean that there is no more vector element

inline bool deserializeVecBeg( YsifInpStream& ser, uint32_t& val, const char * const name, uint32_t maxIndex )
{
    DEBUG_DESER(name);
    if( ser.checkCmd("[",name) ) {
        ser >> val;
        if( ser.good() ) {
            if( val<=maxIndex ) {
                return ser.toEndl();
            }
        }
    }
    return false;
};

inline bool deserializeVecEnd( YsifInpStream& ser, bool& last )
{
    // std::cerr << "deserializeVecEnd  "   << std::endl;  
    return ser.VecEnd(last);
};

inline void deserializeBegin( YsifInpStream& ser, const char * const name )
{   ser.seekg(0); };


inline bool deserialize( YsifInpStream& ser, uint8_t& val, const char * const name )
{
    DEBUG_DESER(name);
    if( !ser.checkCmd(":",name))
        return false;
    uint16_t tmp;
    ser >> tmp;
    val = uint8_t(tmp);
    return ser.toEndl();
};

inline bool deserialize( YsifInpStream& ser, int8_t& val, const char * const name)
{
    DEBUG_DESER(name);
    if( !ser.checkCmd(":",name))
        return false;
    int16_t tmp;
    ser >> tmp;
    val = int8_t(tmp);
    return ser.toEndl();
};

inline bool deserialize( YsifInpStream& ser, uint16_t& val, const char * const name )
{
    DEBUG_DESER(name);
    if( !ser.checkCmd(":",name))
        return false;
    ser >> val;
    return ser.toEndl();
};

inline bool deserialize( YsifInpStream& ser, int16_t& val, const char * const name )
{
    DEBUG_DESER(name);
    if( !ser.checkCmd(":",name))
        return false;
    ser >> val;
    return ser.toEndl();
};

inline bool deserialize( YsifInpStream& ser, uint32_t& val, const char * const name )
{
    DEBUG_DESER(name);
    if( !ser.checkCmd(":",name))
        return false;
    ser >> val;
    return ser.toEndl();
};

inline bool deserialize( YsifInpStream& ser, int32_t& val, const char * const name )
{
    DEBUG_DESER(name);
    if( !ser.checkCmd(":",name))
        return false;
    ser >> val;
    return ser.toEndl();
};

inline bool deserialize( YsifInpStream& ser, uint64_t& val, const char * const name )
{
    DEBUG_DESER(name);
    if( !ser.checkCmd(":",name))
        return false;
    ser >> val;
    return ser.toEndl();
};

inline bool deserialize( YsifInpStream& ser, int64_t& val, const char * const name )
{
    DEBUG_DESER(name);
    if( !ser.checkCmd(":",name))
        return false;
    ser >> val;
    return ser.toEndl();
};

inline bool deserialize( YsifInpStream& ser, float& val, const char * const name )
{
    DEBUG_DESER(name);
    if( !ser.checkCmd(":",name))
        return false;
    ser >> val;
    return ser.toEndl();
};

inline bool deserialize( YsifInpStream& ser, double& val, const char * const name )
{
    DEBUG_DESER(name);
    if( !ser.checkCmd(":",name))
        return false;
    ser >> val;
    return ser.toEndl();
};

inline bool deserialize( YsifInpStream& ser, std::string& val, const char * const name )
{
    DEBUG_DESER(name);
    if( !ser.checkCmd(":",name))
        return false;
    ser >> val;
    return ser.toEndl();
};


#if 1
// -------------------------------------
// old part1 -- DelayTap

inline bool deserialize( std::stringstream& ser, uint8_t& val )
{
    uint16_t tmp;
    ser >> tmp;
    val = uint8_t(tmp);
    return ser.good();
};

inline bool deserialize( std::stringstream& ser, int8_t& val )
{
    int16_t tmp;
    ser >> tmp;
    val = int8_t(tmp);
    return ser.good();
};

inline bool deserialize( std::stringstream& ser, uint16_t& val )
{   ser >> val; return ser.good(); };
inline bool deserialize( std::stringstream& ser, int16_t& val )
{   ser >> val; return ser.good(); };
inline bool deserialize( std::stringstream& ser, uint32_t& val )
{   ser >> val; return ser.good(); };
inline bool deserialize( std::stringstream& ser, int32_t& val )
{   ser >> val; return ser.good(); };
inline bool deserialize( std::stringstream& ser, uint64_t& val )
{   ser >> val; return ser.good(); };
inline bool deserialize( std::stringstream& ser, int64_t& val )
{   ser >> val; return ser.good(); };
inline bool deserialize( std::stringstream& ser, float& val )
{   ser >> val; return ser.good(); };
inline bool deserialize( std::stringstream& ser, double& val )
{   ser >> val; return ser.good(); };
inline bool deserialize( std::stringstream& ser, std::string& val )
{   ser >> val; return ser.good(); };


} // end namespace yacynth

#endif


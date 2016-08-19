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
#include    <sstream>

namespace yacynth {

typedef const char * const Sermagic;

struct Vecindex {
    static constexpr const char * const id = "VX";
    Vecindex()
    :   i(0)
    {};
    inline void inc(void) {++i;};
    uint32_t i;
};

// -------- serialize --------
inline void serializeBegin( std::stringstream& ser )
{   ser.seekp(0); ser << "\n"; };
inline void serializeEnd( std::stringstream& ser )
{   ser << "\n\0"; };
inline void serializeBreak( std::stringstream& ser )
{   ser << "\n"; };
inline void serializeSpace( std::stringstream& ser )
{   ser << "   "; };
inline void serialize( std::stringstream& ser, const Sermagic& val )
{   ser << "\n" << val; };
inline void serialize( std::stringstream& ser, const Vecindex& val )
{   ser << "\n" << val.id << " " << val.i; };
inline void serialize( std::stringstream& ser, const uint8_t& val )
{   ser << " " << uint16_t(val); };
inline void serialize( std::stringstream& ser, const int8_t& val )
{   ser << " " << int16_t(val); };
inline void serialize( std::stringstream& ser, const uint16_t& val )
{   ser << " " << val; };
inline void serialize( std::stringstream& ser, const int16_t& val )
{   ser << " " << val; };
inline void serialize( std::stringstream& ser, const uint32_t& val )
{   ser << " " << val; };
inline void serialize( std::stringstream& ser, const int32_t& val )
{   ser << " " << val; };
inline void serialize( std::stringstream& ser, const uint64_t& val )
{   ser << " " << val; };
inline void serialize( std::stringstream& ser, const int64_t& val )
{   ser << " " << val; };
inline void serialize( std::stringstream& ser, const float& val )
{   ser << " " << val; };
inline void serialize( std::stringstream& ser, const double& val )
{   ser << " " << val; };
inline void serialize( std::stringstream& ser, const std::string& val )
{   ser << " " << val; };

// -------- deserialize --------

inline void deserializeBegin( std::stringstream& ser )
{   ser.seekg(0); };
inline void deserializeEnd( std::stringstream& ser )
{  ; };
inline bool deserialize( std::stringstream& ser, const Sermagic& val )
{
    std::string tmp;
    ser >> tmp;
    return ser.good() && ( 0 == tmp.compare( val ));
};

inline bool deserialize( std::stringstream& ser, Vecindex& val )
{
    std::string tmp;
    ser >> tmp;
    ser >> val.i;
    return ser.good() && ( 0 == tmp.compare( val.id ));
};

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
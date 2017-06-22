#pragma once
/*
 * Copyright (C) 2017 ist
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
 * File:   TypeNames.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on June 17, 2017, 8:01 PM
 */

#include    <cstdint>

// for gcc 64 bit

namespace tnames {

enum typeids {
    NIL,
    UINT8,
    UINT16,
    UINT32,
    UINT64,
    INT8,
    INT16,
    INT32,
    INT64,
    FLOAT,
    DOUBLE,
};
  
template<typename T>
struct TypeName{
  static constexpr char const * const   name    = " undef";
  static constexpr uint16_t             id      = NIL;
  static constexpr uint16_t             size    = 0;  
};

// uint
template<>
struct TypeName<unsigned char>{
  static constexpr char const * const   name    = "uint  8";
  static constexpr uint16_t             id      = typeids::UINT8;
  static constexpr uint16_t             size    = sizeof(unsigned char);  
};

template<>
struct TypeName<unsigned short>{
  static constexpr char const * const   name    = "uint 16";
  static constexpr uint16_t             id      = typeids::UINT16;
  static constexpr uint16_t             size    = sizeof(short int);  
};

template<>
struct TypeName<unsigned int>{
  static constexpr char const * const   name    = "uint 32";
  static constexpr uint16_t             id      = typeids::UINT32;
  static constexpr uint16_t             size    = sizeof(unsigned int);  
};

template<>
struct TypeName<long unsigned int>{
  static constexpr char const * const   name    = "uint 64";
  static constexpr uint16_t             id      = typeids::UINT64;
  static constexpr uint16_t             size    = sizeof(long unsigned int);  
};



// int
template<>
struct TypeName<signed char>{
  static constexpr char const * const   name    = " int  8";
  static constexpr uint16_t             id      = typeids::INT8;
  static constexpr uint16_t             size    = sizeof(signed char);  
};

template<>
struct TypeName<short>{
  static constexpr char const * const   name    = " int 16";
  static constexpr uint16_t             id      = typeids::INT16;
  static constexpr uint16_t             size    = sizeof(short);  
};

template<>
struct TypeName<int>{
  static constexpr char const * const   name    = " int 32";
  static constexpr uint16_t             id      = typeids::INT32;
  static constexpr uint16_t             size    = sizeof(int);  
};


template<>
struct TypeName<long int>{
  static constexpr char const * const   name    = " int 64";
  static constexpr uint16_t             id      = typeids::INT64;
  static constexpr uint16_t             size    = sizeof(long int);  
};

// floats

template<>
struct TypeName<float>{     
  static constexpr char const * const   name    = "  float";
  static constexpr uint16_t             id      = typeids::FLOAT;
  static constexpr uint16_t             size    = sizeof(float);  
};

template<>
struct TypeName<double>{
  static constexpr char const * const   name    = " double";
  static constexpr uint16_t             id      = typeids::DOUBLE;
  static constexpr uint16_t             size    = sizeof(double);  
};

} // end namespace tnames 



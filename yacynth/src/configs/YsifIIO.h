#pragma once

#include <cstdlib>
#include <ios>

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
 * File:   YsifIIO.h
 * Author: Istvan Simon
 *
 * Created on March 4, 2017, 9:31 PM
 */
#include    <array>
#include    <sstream>

namespace yacynth {

constexpr std::size_t YsifSize =  1<<16;

struct YsifBuf : public std::array< uint8_t, YsifSize > {
    static constexpr std::size_t size =  YsifSize;
    void seekp(std::size_t notused=0);
    uint16_t tagCount;
};

//
// Tio is the internal class that must be serialized
// Tpre the additional prefix structure: tags + addressing parameters
//

template < class Tio, class Tpre >
class YsifIIO {
public:
    
    YsifIIO()
    {}
    
    ~YsifIIO()
    {}

    // ascii
    bool ysifOut( std::stringstream& ser )
    {
        ser.seekp(0);
        // print tags additional parameters           
        prefix.ysifOut(ser);
        // print data
        data.ysifOut(ser);
    }

    // ascii
    bool ysifInp( std::stringstream& ser )
    {
        
    }
    
    // bin
    bool ysifOut( YsifBuf& ser )
    {
        ser.seekp(0);
        // print tags additional parameters           
        prefix.ysifOut(ser);
        // print data
        data.ysifOut(ser);        
    }

    // bin
    bool ysifInp( YsifBuf& ser )
    {
        
    }
    
private:
    Tpre    prefix;
    //Tio&     data; 
    Tio     data;
};




} // end namespace yacynth

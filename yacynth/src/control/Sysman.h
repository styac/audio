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
 * File:   Sysman.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on February 21, 2016, 8:31 AM
 */
#include    "../../include/protocol.h"
#include    "../oscillator/Tables.h"
#include    "../oscillator/Oscillator.h"
//#include    "../oscillator/BaseSetting.h"
#include    "../oscillator/OscillatorArray.h"
#include    "../oscillator/ToneShaper.h"
#include    "../yaio/YaIoJack.h"
#include    "../yaio/IOthread.h"
#include    "Tags.h"

#include    <boost/array.hpp>
#include    <boost/asio.hpp>
#include    <boost/utility/string_ref.hpp>

#include    <cstdlib>
#include    <iostream>
#include    <fstream>
#include    <type_traits>
#include    <iomanip>
#include    <unistd.h>
#include    <map>
#include    <list>

using namespace boost::asio;

namespace yacynth {

class Sysman {
public:
    explicit Sysman(
        OscillatorArray&    oscillatorArrayP,
        IOThread&           iOThreadP );

    Sysman() = delete;
    Sysman(const Sysman&) = delete;
    Sysman& operator=(const Sysman&) = delete;
    bool eval( ip::tcp::socket&   socketAccept );
    
    // for testing 
    void testParameter();
    bool evalParameterMessage( Yaxp::Message& msg );
    
private:
    
    bool parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );     
    OscillatorArray&            oscillatorArray;
    ToneShaperMatrix&           toneShaperMatrix;
    IOThread&                   iOThread;
    boost::asio::streambuf      inStreambuf;
};

} // end namespace yacynth


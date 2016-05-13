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

namespace yacynth {
// --------------------------------------------------------------------

Sysman::Sysman( 
        OscillatorArray&    oscillatorArrayP,
        IOThread&           iOThreadP  )
:   oscillatorArray(    oscillatorArrayP )
,   iOThread(           iOThreadP )
,   toneShaperMatrix( oscillatorArrayP.getToneShaperMatrix() )
{
    
} // end Sysman::Sysman

// --------------------------------------------------------------------

bool Sysman::eval( ip::tcp::socket&   socketAccept )
{
    boost::system::error_code   asioError;
    boost::asio::streambuf      inStreambuf;
    boost::asio::streambuf      outStreambuf(1<<14) ;
    std::ostream                ost(&outStreambuf);
    
    std::cout << "Sysman::eval " << std::endl;    

    write(socketAccept, boost::asio::buffer(Yax::helloClient), asioError );

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
} // end Sysman::exec

// --------------------------------------------------------------------

bool Sysman::queryUnit( const unitType ut, std::stringstream& ser )
{
    std::cout << "Sysman::queryUnit " << std::endl;
    switch(ut) {
    case UNIT_SYSTEM:
        break;
    case UNIT_OSCILLATOR:
        toneShaperMatrix.query( ser );
        break;
    case UNIT_EFFECT:
        break;
    case UNIT_TUNING:
        break;
    case UNIT_CONTROLLER:
        break;
    case UNIT_MIDIROUTER:
        break;
    default:
        return false;
    }
    return true;
} // end Sysman::queryUnit

// --------------------------------------------------------------------

bool Sysman::fillUnit( const unitType ut, std::stringstream& ser )
{
    std::cout << "Sysman::fillUnit " << std::endl;
    switch(ut) {
    case UNIT_SYSTEM:
        break;
    case UNIT_OSCILLATOR:
        return toneShaperMatrix.fill( ser );
    case UNIT_EFFECT:
        break;
    case UNIT_TUNING:
        break;
    case UNIT_CONTROLLER:
        break;
    case UNIT_MIDIROUTER:
        break;
    }
    return false;
} // end Sysman::fillUnit
// --------------------------------------------------------------------

bool Sysman::evalAscii(void)
{
    std::cout << "Sysman::evalAscii " << std::endl;
    
} // end Sysman::exec

// --------------------------------------------------------------------

bool Sysman::evalUnitSystem(void)
{
    std::cout << "Sysman::evalUnitSystem " << std::endl;
    
} // end Sysman::evalUnitSystem

// --------------------------------------------------------------------

bool Sysman::evalUnitTuning(void)
{
    std::cout << "Sysman::evalUnitTuning " << std::endl;
    
} // end Sysman::evalUnitTuning

// --------------------------------------------------------------------

bool Sysman::evalUnitController(void)
{
    std::cout << "Sysman::evalUnitController " << std::endl;
    
} // end Sysman::evalUnitController

// --------------------------------------------------------------------

bool Sysman::evalUnitMidirouter(void)
{
    std::cout << "Sysman::evalUnitMidirouter " << std::endl;
    
} // end Sysman::evalUnitMidirouter

// --------------------------------------------------------------------

bool Sysman::evalUnitOscillator(void)
{
    std::cout << "Sysman::evalUnitOscillator " << std::endl;
    
} // end Sysman::evalUnitOscillator

// --------------------------------------------------------------------

bool Sysman::evalUnitEffect(void)
{
    std::cout << "Sysman::evalUnitEffect " << std::endl;
    
} // end Sysman::evalUnitEffect

// --------------------------------------------------------------------


} // end namespace yacynth



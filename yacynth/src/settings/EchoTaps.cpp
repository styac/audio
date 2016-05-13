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
 * File:   EchoTaps.cpp
 * Author: Istvan Simon
 *
 * Created on April 6, 2016, 4:38 PM
 */

#include    "../effects/DelayTap.h"
#include    "../settings/EchoTaps.h"

#include    <cstdint>
#include    <string>
#include    <array>
#include    <sstream>

namespace yacynth {

std::stringstream   echotapsOut;
std::stringstream   echotapsFeedback;

std::stringstream& echoTapsOut(void)
{
     echotapsOut
//        << "DTAP:01  0.5 0.5 0.1 0.1 " << 777*64+113 << " " <<  0
//        << "\n"
//        << "DTAP:01  0.1 0.1 0.5 0.5 " << 1103*64+117 << " " <<  0
//        << "\n"
        << "DTAP:01  0.7 0.7 0.2 0.2 " << 2000*64+3 << " " <<  0
        << "\n"
        << "DTAP:01 0.2 0.2 0.7 0.7  " << 2000*64+7 << " " <<  0
        << "\n";

     return echotapsOut;
}

std::stringstream& echoTapsFeedback(void)
{
     echotapsFeedback
 //       << "DTAP:01 0.5 0.5 0.2 0.2 " << 2000*64+7 << " " <<  0
 //       << "\n"
 //       << "DTAP:01 0.2 0.2 0.5 0.5  " << 1000*64+13 << " " <<  0
 //       << "\n"
        << "DTAP:01 0.6 0.6 0.3 0.3 " << 4000*64+11 << " " <<  0
        << "\n"
        << "DTAP:01 0.3 0.3 0.7 0.7 " << 4000*64+13 << " " <<  0
        << "\n";

     return echotapsFeedback;
}


}


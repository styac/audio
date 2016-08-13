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
 * File:   EffectBase.h
 * Author: Istvan Simon
 *
 * Created on April 6, 2016, 11:22 PM
 */

#include    "FxBase.h"
#include    <chrono>
#include    <sys/time.h>
#include    <ctime>


namespace yacynth {

FxBase      fxNil("Nil");
uint16_t    FxBase::count = -1;




FxNode::FxNode()
:   thp(&fxNil)
{};

bool FxBase::connect( const FxBase * v, uint16_t ind )
{
     std::cout
         << "FxBase::connect"
         << std::endl;
    return false;
}

bool FxBase::setProcMode( uint16_t ind )
{
     std::cout
         << "FxBase::setProcMode"
         << std::endl;
    return false;
}; // might be non virtual


void FxCollector::check(void) {
    for( auto& it : nodes ) {
        std::cout
            << "Effects: " << it->id()
            << "  " << it->name()
            << " input count:" << it->getInputCount()
            << " max mode:" << it->getMaxMode()
            << " master id:" << it->getMasterId()
            << " max id:" << it->getMaxId()
            << std::endl;
    }
}


void FxCollector::initialize(void)
{
    const std::string nameprefix("slave-");
    for( auto i=0u; i<8; ++i ) {
        std::string name = nameprefix + std::to_string(i);
        auto j = new FxBase( name.data() );
    }
}

// --------------------------------------------------------------------

} // end namespace yacynth
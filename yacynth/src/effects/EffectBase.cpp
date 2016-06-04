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

#include    "EffectBase.h"
#include    <chrono>
#include    <sys/time.h>
#include    <ctime>


namespace yacynth {

Effects&  effects   = Effects::getInstance();

void Effects::put( EffectBase& node )
{
    nodes.push_back(&node);
}

void Effects::check(void) {
    for( auto& it : nodes ) {
        std::cout
            << "Effects: " << it->myId()
            << "  " << it->myType()
            << "  " << it->myName()
            
            << std::endl;
    }
}
// --------------------------------------------------------------------
//GNoiseShaped    EffectBase::gNoiseShaped;
uint32_t        EffectBase::count = 0;
EffectBase      effectNil("Nil");
EffectNoise     effectNoise;

// --------------------------------------------------------------------
bool EffectBase::setMode( const uint8_t mode ) 
{ 
    pprocess = sprocessNop;
    return false;
};

void EffectBase::sprocessNop( void * thp )
{
    ;
}
bool EffectBase::fill( std::stringstream& ser ) 
{ 
    return true;
};
void EffectBase::query( std::stringstream& ser ) 
{ 
    ;
};
void EffectBase::connect( const EIObuffer& v, const uint16_t index ) 
{ 
    ; 
};
void EffectBase::process(void) 
{ 
    ; 
};
void EffectBase::clear(void) 
{ 
    ; 
}

// --------------------------------------------------------------------

bool EffectIOBase::fill( std::stringstream& ser )  
{ 
    return true;
};

void EffectIOBase::process(void) 
{ 
    ; 
};
void EffectIOBase::connect( const EIObuffer& v, const uint16_t index ) 
{ 
    inp=&v; 
};

 
// --------------------------------------------------------------------
EffectNoise::EffectNoise()
:   EffectBase("WhiteNoise")
{
};
EffectNoise::~EffectNoise() 
{
};
bool EffectNoise::fill( std::stringstream& ser ) 
{ 
    return true;
};
void EffectNoise::process(void)
{
    static uint64_t count = 0;
    timeval tv;
    gettimeofday(&tv,NULL);

    constexpr   float range = 1.0f/(1L<<31);
    const float level = gain * range;
    for( auto i=0u; i <out.sectionSize; ++i ) {
        int32_t A;
        int32_t B;
//        gNoiseShaped.getWhite( A, B );
        out.channelA[i] = A * level;
        out.channelB[i] = B * level;
        if ( 0 == ( ++count & 0x0FFFFF  ) ) {
            std::cout << std::hex
                << "count " << count
//                << " gNoiseShaped " << gNoiseShaped.get()
                << " sec " << tv.tv_sec
                << std::endl;
        }
    }
};

// --------------------------------------------------------------------

} // end namespace yacynth
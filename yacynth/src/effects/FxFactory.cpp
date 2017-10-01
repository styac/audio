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
 * File:   FxFactory.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on October 1, 2017, 9:00 AM
 */
#include    "FxOscillatorMixer.h"
#include    "FxMixer.h"
#include    "FxInput.h"
#include    "FxEcho.h"
#include    "FxModulator.h"
#include    "FxOutNoise.h"
#include    "FxOutOscillator.h"
#include    "FxFilter.h"
#include    "FxLateReverb.h"
#include    "FxEarlyReflection.h"
#include    "FxChorus.h"
#include    "FxFlanger.h"

namespace yacynth {

// --------------------------------------------------------------------

bool FxCollector::factory( TagEffectType effectType )
{
    // check if can be created
    FxBase * fxBasep;
    switch( effectType ) {
    case TagEffectType::FxNop:
    case TagEffectType::FxNil:
    case TagEffectType::FxSlave:
    case TagEffectType::FxOscillatorMixer:
    case TagEffectType::FxInput:
        return  false;

    case TagEffectType::FxMixer: {
        FxMixer * p = new FxMixer();
        fxBasep = static_cast<FxBase *> (p);
        }
        break;
        
    case TagEffectType::FxModulator: {
        FxModulator * p = new FxModulator();
        fxBasep = static_cast<FxBase *> (p);
        }
        break;
        
    case TagEffectType::FxOutNoise: {
        FxOutNoise * p = new FxOutNoise();
        fxBasep = static_cast<FxBase *> (p);
        }
        break;
        
    case TagEffectType::FxOutOscillator: {
        FxOutOscillator * p = new FxOutOscillator();
        fxBasep = static_cast<FxBase *> (p);
        }
        break;
        
    case TagEffectType::FxFilter: {
        FxFilter * p = new FxFilter();
        fxBasep = static_cast<FxBase *> (p);
        }
        break;
        
    case TagEffectType::FxEcho: {
        FxEcho * p = new FxEcho();
        fxBasep = static_cast<FxBase *> (p);
        }
        break;
        
    case TagEffectType::FxLateReverb: {
        FxLateReverb * p = new FxLateReverb();
        fxBasep = static_cast<FxBase *> (p);
        }
        break;
        
    case TagEffectType::FxEarlyReflection: {
        FxEarlyReflection * p = new FxEarlyReflection();
        fxBasep = static_cast<FxBase *> (p);
        }
        break;
        
    case TagEffectType::FxChorus: {
        FxChorus * p = new FxChorus();
        fxBasep = static_cast<FxBase *> (p);
        }
        break;
        
    case TagEffectType::FxFlanger: {
        FxFlanger * p = new FxFlanger();
        fxBasep = static_cast<FxBase *> (p);
        }
        break;
    }

    if( 0 == firstDynamicInstance ) {
        firstDynamicInstance = fxBasep->id();
    }
    fxBasep->setDynamic();
    return true;
} // end FxCollector::factory
    
// --------------------------------------------------------------------
// return false if nothing was deleted
//
// first FxRunner must be reset
// destroy everything from current to "staticInstanceBase"
bool FxCollector::cleanup()
{
    if( 0 == firstDynamicInstance ) {
        return false; // nothing to delete
    }
    if( firstDynamicInstance > nodes.size() ) {
        return false; // nothing to delete
    }
    bool res = false;
    auto firstNodeToDelete = nodes.size() - 1;
    auto lastNodeToDelete = firstDynamicInstance;
    for( auto nodeind = firstNodeToDelete; nodeind >= lastNodeToDelete; --nodeind ) {  
        FxBase * node = nodes[ nodeind ];
        if( 0 != firstDynamicInstance ) {
            node->setCount( firstDynamicInstance - 1 );
            firstDynamicInstance = 0;
        }
        if( node->getType() != TagEffectType::FxSlave ) {
            delete node;            
        }
    }    
    for( auto nodeind = firstNodeToDelete; nodeind >= lastNodeToDelete; --nodeind ) {  
        nodes.pop_back();
    }
    return true;
}

} // end namespace yacynth 

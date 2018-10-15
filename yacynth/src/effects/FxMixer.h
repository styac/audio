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
 * File:   Mixer.h
 * Author: Istvan Simon
 *
 * Created on March 15, 2016, 6:23 PM
 */

#include "FxMixerParam.h"
#include "Ebuffer.h"
#include "yacynth_globals.h"
//#include "protocol.h"
#include "effects/FxBase.h"

#include <array>
#include <iostream>

namespace yacynth {

class FxMixer : public Fx<FxMixerParam>  {
public:
    using MyType = FxMixer;
    FxMixer()
    :   Fx<FxMixerParam>()
    {
    }

    inline void dump( float * channel0,  float * channel1 )
    {
        out().dump( channel0, channel1 );
    }

    virtual bool connect( const FxBase * v, uint16_t ind ) override;

    virtual bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex ) override;

    virtual void clearState() override;
    
    virtual bool setSprocessNext( uint16_t mode ) override;

private:

    static void sprocess_01( void * thp );

    inline void process()
    {
        // channel 0 - MASTER VOLUME
        const bool isMasterVolumeChanged = gainCache[ 0 ].update( param.gainIndex[ 0 ] );
        if( isMasterVolumeChanged ) {
            const float cgainChA = gainCache[ 0 ].getExpValueFloat() * param.gainRange[ 0 ][ chA ];
            const float cgainChB = gainCache[ 0 ].getExpValueFloat() * param.gainRange[ 0 ][ chB ];
            out().fadeV4( inp<0>(), gain[ 0 ][ chA ], gain[ 0 ][ chB ],
                ( cgainChA - gain[ 0 ][ chA ] ), ( cgainChB - gain[ 0 ][ chB ] ) );
        } else {
            out().mult( inp<0>(), gain[ 0 ][ chA ], gain[ 0 ][ chB ] );
        }

        // channel k = MASTER VOLUME * channel volume
        for( auto cin = 1u; cin < param.effectiveInputCount; ++cin ) {
            if( param.gainZero[ cin ] ) {
                if( gainCache[ cin ].update( param.gainIndex[ cin ] ) || isMasterVolumeChanged ) {
                    const float cgainChA = gainCache[ cin ].getExpValueFloat() * param.gainRange[ cin ][ chA ] * gain[ 0 ][ chA ];
                    const float cgainChB = gainCache[ cin ].getExpValueFloat() * param.gainRange[ cin ][ chB ] * gain[ 0 ][ chB ];
                    // TODO check add
                    out().fadeAddV4( inp( cin ), gain[ cin ][ chA ], gain[ cin ][ chB ],
                        ( cgainChA - gain[ cin ][ chA ] ), ( cgainChB - gain[ cin ][ chB ] ) );
                } else {
                    out().multAdd( inp( cin ), gain[ cin ][ chA ], gain[ cin ][ chB ] );
                }                
            }
        }
    }

    float   gain[ FxMixerParam::inputCount ][ 2 ]; // for each stereo channel
    ControllerCache gainCache[ FxMixerParam::inputCount ];
//    ControllerCacheRate<8> gainCache[FxMixerParam::inputCount];
};

// --------------------------------------------------------------------
} // end namespace yacynth


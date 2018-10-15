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
 * File:   FxOscillatorMixer.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on June 23, 2016, 9:41 PM
 */

#include "yacynth_globals.h"
#include "effects/FxBase.h"
#include "v4.h"
#include "FxOscillatorMixerParam.h"
#include "oscillator/OscillatorOutput.h"

namespace yacynth {
class FxOscillatorMixer : public Fx<FxOscillatorMixerParam>  {
public:
    using MyType = FxOscillatorMixer;
    FxOscillatorMixer()
    :   Fx<FxOscillatorMixerParam>()
    {
        for( auto& si : slaves ) si.setrefId(id());
    }

    virtual bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex ) override;

    virtual void clearState() override;

    virtual bool connect( const FxBase * v, uint16_t ind ) override;

    virtual bool setSprocessNext( uint16_t mode ) override;

    void process( const OscillatorOut& inp )
    {
        // main output
        if( inp.amplitudeSumm[ 1 ] ) {
            // stereo
            // setAmplitudeSumm( ci, inp.amplitudeSumm[ 0 ] + inp.amplitudeSumm[ 1 ] );
            for( uint16_t si = 0u; si < oscillatorFrameSize; ++si ) {
                out().channel[ chA ][ si ] = inp.layer[ 0 ][ si ] * param.gain[ 0 ];
                out().channel[ chB ][ si ] = inp.layer[ 1 ][ si ] * param.gain[ 1 ];
            }
        } else {
            // mono
            // setAmplitudeSumm( ci, inp.amplitudeSumm[ 0 ] << 1 );
            for( uint16_t si = 0u; si < oscillatorFrameSize; ++si ) {
                out().channel[ chA ][ si ] = out().channel[ chB ][ si ] = inp.layer[0][si] * param.gain[0];
            }
        }

        for( uint16_t ci = 0u; ci < FxOscillatorMixerParam::slaveCount; ++ci ) {
            const uint16_t ci2_0 = (ci+1)<<1;
            const uint16_t ci2_1 = ci2_0 + 1;
            if( param.gainZero[ ci ] == 0 ) {
                // clearAmplitudeSumm( ci );
                continue;
            }
            if( inp.amplitudeSumm[ ci2_1 ] ) {
                // setAmplitudeSumm( ci, inp.amplitudeSumm[ ci2_0 ] + inp.amplitudeSumm[ ci2_1 ] );
                // stereo
                for( uint16_t si = 0u; si < oscillatorFrameSize; ++si ) {
                    slaves[ ci ].out().channel[ chA ][ si ] = inp.layer[ ci2_0 ][ si ] * param.gain[ ci2_0 ];
                    slaves[ ci ].out().channel[ chB ][ si ] = inp.layer[ ci2_1 ][ si ] * param.gain[ ci2_1 ];
                }
            } else {
                // setAmplitudeSumm( ci, inp.amplitudeSumm[ ci2_0 ] << 1 );
                // mono
                for( uint16_t si = 0u; si < oscillatorFrameSize; ++si ) {
                    slaves[ ci ].out().channel[ chA ][ si ] = slaves[ ci ].out().channel[ chB ][ si ] = inp.layer[ ci2_0 ][si] * param.gain[ ci2_0 ];
                }
            }
        }
    }

private:
    // 16 layers: 1 base + 15 slaves
    FxSlave<FxOscillatorMixerParam>   slaves[ FxOscillatorMixerParam::slaveCount ];

};

} // end namespace yacynth


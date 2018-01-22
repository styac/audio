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
 * File:   FxModulator.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on June 23, 2016, 6:14 PM
 */

#include    "FxModulatorParam.h"
#include    "effects/FxBase.h"

namespace yacynth {
class FxModulator : public Fx<FxModulatorParam>  {
public:
    using MyType = FxModulator;
    FxModulator()
    :   Fx<FxModulatorParam>()
    {
    }

    virtual bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex ) override;

    virtual void clearState() override;

    virtual bool setSprocessNext( uint16_t mode ) override;

    virtual bool connect( const FxBase * v, uint16_t ind ) override;

private:
    static void sprocess_01( void * thp );
    static void sprocess_02( void * thp );
    static void sprocess_03( void * thp );
    static void sprocess_04( void * thp );
    static void sprocess_05( void * thp );
    static void sprocess_06( void * thp );

    inline void processModulationMix(void)
    {
        inMultIn.updateDelta(param.inMultIndex);
        mixMultIn.updateDelta(param.mixMultIndex);
        const float inMult = inMultIn.getExpValueFloat();
        const float mixMult = mixMultIn.getExpValueFloat();
        for( auto si=0u; si < vsectionSize; ++si ) {
            out().vchannel[0][si] = inp<0>().vchannel[0][si] * inp<1>().vchannel[0][si] * mixMult + inp<0>().vchannel[0][si] * inMult;
            out().vchannel[1][si] = inp<0>().vchannel[1][si] * inp<1>().vchannel[1][si] * mixMult + inp<0>().vchannel[1][si] * inMult;
        }
    }

    inline void processModulation(void)
    {
        mixMultIn.updateDelta( param.mixMultIndex );
        const float mixMult = mixMultIn.getExpValueFloat();
        for( auto si=0u; si < vsectionSize; ++si ) {
            out().vchannel[0][si] = inp<0>().vchannel[0][si] * ( inp<1>().vchannel[0][si] * mixMult + 1.0f );
            out().vchannel[1][si] = inp<0>().vchannel[1][si] * ( inp<1>().vchannel[1][si] * mixMult + 1.0f );
        }
    }

    inline void processRingVolColtrol(void)
    {
        inMultIn.updateDelta(param.inMultIndex);
        const float inMult = inMultIn.getExpValueFloat();
        for( auto si=0u; si < vsectionSize; ++si ) {
            out().vchannel[0][si] = inp<0>().vchannel[0][si] * inp<1>().vchannel[0][si] * inMult;
            out().vchannel[1][si] = inp<0>().vchannel[1][si] * inp<1>().vchannel[1][si] * inMult;
        }
    }

    inline void processRing(void)
    {
        for( auto si=0u; si < vsectionSize; ++si ) {
            out().vchannel[0][si] = inp<0>().vchannel[0][si] * inp<1>().vchannel[0][si];
            out().vchannel[1][si] = inp<0>().vchannel[1][si] * inp<1>().vchannel[1][si];
        }
    }


    inline void processModulationMono(void)
    {
        mixMultIn.updateDelta( param.mixMultIndex );
        const float mixMult = mixMultIn.getExpValueFloat();
        for( auto si=0u; si < vsectionSize; ++si ) {
            out().vchannel[0][si] = inp<0>().vchannel[0][si] * ( inp<1>().vchannel[0][si] * mixMult + 1.0f );
        }
    }

    inline void processRingMono(void)
    {
        for( auto si=0u; si < vsectionSize; ++si ) {
            out().vchannel[0][si] = inp<0>().vchannel[0][si] * inp<1>().vchannel[0][si];
        }
    }
#if 0
    // 2x oversampling
    // https://christianfloisand.wordpress.com/tag/resampling/
    inline void processRingMono2xOvS(void)
    {
        for( auto si=0u; si < vsectionSize; ++si ) {
            out().vchannel[0][si] = inp<0>().vchannel[0][si] * inp<1>().vchannel[0][si];
        }
    }
#endif
    ControllerCacheRate<10> inMultIn;      // TODO test by hearing if limit is hearable
    ControllerCacheRate<10> mixMultIn;     // TODO test by hearing
};


} // end namespace yacynth


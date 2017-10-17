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
 * File:   FxOutNoise.h
 * Author: Istvan Simon
 *
 * Created on June 18, 2016, 1:06 PM
 */

#include    "FxOutNoiseParam.h"
#include    "../oscillator/NoiseFrame.h"
#include    "../utils/GaloisNoiser.h"
#include    "../effects/FxBase.h"

using namespace noiser;

namespace yacynth {

class FxOutNoise : public NoiseFrame<Fx<FxOutNoiseParam>>  {
public:
    using MyType = FxOutNoise;
    FxOutNoise()
    :   NoiseFrame<Fx<FxOutNoiseParam>>( GaloisShifterSingle<seedThreadEffect_noise>::getInstance() )
    {
    }

    virtual bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex ) override;

    virtual void clearState() override;

    virtual bool setSprocessNext( uint16_t mode ) override;

    virtual bool connect( const FxBase * v, uint16_t ind ) override;

private:
    // 00 is always clear for output or bypass for in-out
    // clearState should clear and this would be a nop
    static void sprocess_01( void * thp )
    {
    //    constexpr float gain = 1.0f/(1<<26);
        const float gain = static_cast< MyType * >(thp)->param.gains[ 1 - 1 ];
        static_cast< MyType * >(thp)->fillWhite();
        static_cast< MyType * >(thp)->mult(gain);
    }
    static void sprocess_02( void * thp )
    {
    //    constexpr float gain = 1.0f/(1<<26);
        const float gain = static_cast< MyType * >(thp)->param.gains[ 2 - 1 ];
        static_cast< MyType * >(thp)->fillWhiteStereo();
        static_cast< MyType * >(thp)->mult(gain);
    }

    static void sprocess_03( void * thp )
    {
    //    constexpr float gain = 1.0f/(1<<26);
        const float gain = static_cast< MyType * >(thp)->param.gains[ 3 - 1 ];
        static_cast< MyType * >(thp)->fillWhiteLowCut();
        static_cast< MyType * >(thp)->mult(gain);
    }
    static void sprocess_04( void * thp )
    {
    //    constexpr float gain = 1.0f/(1<<26);
        const float gain = static_cast< MyType * >(thp)->param.gains[ 4 - 1 ];
        static_cast< MyType * >(thp)->fillBlue();
        static_cast< MyType * >(thp)->mult(gain);
    }
    static void sprocess_05( void * thp )
    {
    //    constexpr float gain = 1.0f/(1<<24);
        const float gain = static_cast< MyType * >(thp)->param.gains[ 5 - 1 ];
        static_cast< MyType * >(thp)->fillRed();
        static_cast< MyType * >(thp)->mult(gain);
    }
    static void sprocess_06( void * thp )
    {
    //    constexpr float gain = 1.0f/(1<<21);
        const float gain = static_cast< MyType * >(thp)->param.gains[ 6 - 1 ];
        static_cast< MyType * >(thp)->fillPurple();
        static_cast< MyType * >(thp)->mult(gain);
    }
    static void sprocess_07( void * thp )
    {
    //    constexpr float gain = 1.0f/(1<<27);
        const float gain = static_cast< MyType * >(thp)->param.gains[ 7 - 1 ];
        static_cast< MyType * >(thp)->fillPink();
        static_cast< MyType * >(thp)->mult(gain);
    }
    static void sprocess_08( void * thp )
    {
    //    constexpr float gain = 1.0f/(1<<26);
        const float gain = static_cast< MyType * >(thp)->param.gains[ 8 - 1 ];
        static_cast< MyType * >(thp)->fillPinkLow();
        static_cast< MyType * >(thp)->mult(gain);
    }
    static void sprocess_09( void * thp )
    {
    //    constexpr float gain = 1.0f/(1<<21);
        const float gain = static_cast< MyType * >(thp)->param.gains[ 9 - 1 ];
        static_cast< MyType * >(thp)->fillPurpleVar( static_cast< MyType * >(thp)->param.purplePole );
        static_cast< MyType * >(thp)->mult(gain);
    }
    static void sprocess_10( void * thp )
    {
//        constexpr float gain = 1.0f/(1<<24);
        const float gain = static_cast< MyType * >(thp)->param.gains[ 10 - 1 ];
        static_cast< MyType * >(thp)->fillRedVar( static_cast< MyType * >(thp)->param.redPole );
        static_cast< MyType * >(thp)->mult(gain);
    }
};

// --------------------------------------------------------------------
} // end namespace yacynth
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
        fillSprocessv<0>(sprocess_00);
        fillSprocessv<1>(sprocess_01);
        fillSprocessv<2>(sprocess_02);
        fillSprocessv<3>(sprocess_03);
        fillSprocessv<4>(sprocess_04);
        fillSprocessv<5>(sprocess_05);
        fillSprocessv<6>(sprocess_06);
        fillSprocessv<7>(sprocess_07);
        fillSprocessv<8>(sprocess_08);
        fillSprocessv<9>(sprocess_09);
        fillSprocessv<10>(sprocess_10);
    }

    virtual bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex ) override;

    virtual void clearTransient() override;


    // go up to Fx ??
    // might change -> set sprocessTransient
    // FIRST TEST WITHOUT TRANSIENT
    // THEN  WITH TRANSIENT -> all types > out,
    // 00 is always clear for output or bypass for in-out == effect OFF
    bool setProcMode( uint16_t ind )  override
    {
        if( procMode == ind ) {
            return true; // no change
        }
        if( getMaxMode() < ind ) {
            return false; // illegal
        }
        if( 0 == procMode ) {
            fadePhase = FadePhase::FPH_fadeInSimple;
        } else if( 0 == ind ) {
            fadePhase = FadePhase::FPH_fadeOutSimple;
        } else {
            fadePhase = FadePhase::FPH_fadeOutCross;
        }

        procMode = ind;
        sprocessp = sprocesspSave = sprocessv[ind];
        // sprocesspSave = sprocessv[ind];
        // sprocessp = sprocessTransient;
        return true;
    }
#if 0
    // go up to Fx ?? virtual ?
    SpfT getProcMode( uint16_t ind ) const override
    {
        switch( ind ) {
        case 0:
            return sprocess_00;
        case 1:
            return sprocess_01;
        case 2:
            return sprocess_02;
        default:
            return sprocessp; // illegal index no change
        }
    }
#endif
    virtual bool connect( const FxBase * v, uint16_t ind ) override
    {
        doConnect(v,ind);
    };

private:
    // go up to Fx ???
    static void sprocessTransient( void * thp )
    {
        auto& th = *static_cast< MyType * >(thp);
        switch( th.fadePhase ) {
        // 1 phase
        case FadePhase::FPH_fadeNo:
            th.sprocessp = th.sprocesspSave =  th.sprocessv[ th.procMode ];
            th.sprocesspSave(thp);
            return;

        // clear then switch to nop
        case FadePhase::FPH_fadeOutClear:
            th.clear();
            th.procMode = 0;
            th.sprocessp = th.sprocesspSave = sprocessNop;
            return;

        case FadePhase::FPH_fadeOutSimple:
            th.sprocesspSave(thp);
            th.fadeOut();   // then clear -- then nop
            th.sprocessp = th.sprocesspSave =  th.sprocessv[ th.procMode ];
            return;

        // 1 phase
        case FadePhase::FPH_fadeInSimple:
            th.sprocessp = th.sprocesspSave =  th.sprocessv[ th.procMode ];
            th.sprocesspSave(thp);
            th.fadeIn();
            return;

        // 1 of 2 phase
        case FadePhase::FPH_fadeOutCross:
            th.sprocesspSave(thp);
            th.fadeOut();
            th.sprocesspSave =  th.sprocessv[ th.procMode ];
            th.fadePhase = FadePhase::FPH_fadeInCross;
            return;

        // 2 of 2 phase
        case FadePhase::FPH_fadeInCross: // the same as FPH_fadeInSimple ???
            th.sprocessp = th.sprocesspSave =  th.sprocessv[ th.procMode ];
            th.sprocesspSave(thp);
            th.fadeIn();
            return;
        }

    }

    // 00 is always clear for output or bypass for in-out
    static void sprocess_00( void * thp )
    {
        static_cast< MyType * >(thp)->clear();
    }
    static void sprocess_01( void * thp )
    {
        constexpr float gain = 1.0f/(1<<26);
        static_cast< MyType * >(thp)->fillWhite();
        static_cast< MyType * >(thp)->mult(gain);
    }
    static void sprocess_02( void * thp )
    {
        constexpr float gain = 1.0f/(1<<26);
        static_cast< MyType * >(thp)->fillWhiteStereo();
        static_cast< MyType * >(thp)->mult(gain);
    }

    static void sprocess_03( void * thp )
    {
        constexpr float gain = 1.0f/(1<<26);
        static_cast< MyType * >(thp)->fillWhiteBlue();
        static_cast< MyType * >(thp)->mult(gain);
    }
    static void sprocess_04( void * thp )
    {
        constexpr float gain = 1.0f/(1<<26);
        static_cast< MyType * >(thp)->fillBlue();
        static_cast< MyType * >(thp)->mult(gain);
    }
    static void sprocess_05( void * thp )
    {
        constexpr float gain = 1.0f/(1<<24);
        static_cast< MyType * >(thp)->fillRed();
        static_cast< MyType * >(thp)->mult(gain);
    }
    static void sprocess_06( void * thp )
    {
        constexpr float gain = 1.0f/(1<<21);
        static_cast< MyType * >(thp)->fillPurple();
        static_cast< MyType * >(thp)->mult(gain);
    }
    static void sprocess_07( void * thp )
    {
        constexpr float gain = 1.0f/(1<<27);
        static_cast< MyType * >(thp)->fillPink();
        static_cast< MyType * >(thp)->mult(gain);
    }
    static void sprocess_08( void * thp )
    {
        constexpr float gain = 1.0f/(1<<26);
        static_cast< MyType * >(thp)->fillPinkLow();
        static_cast< MyType * >(thp)->mult(gain);
    }
    static void sprocess_09( void * thp )
    {
        constexpr float gain = 1.0f/(1<<21);
        static_cast< MyType * >(thp)->fillPurpleVar( static_cast< MyType * >(thp)->param.purplePole );
        static_cast< MyType * >(thp)->mult(gain);
    }
    static void sprocess_10( void * thp )
    {
        constexpr float gain = 1.0f/(1<<24);
        static_cast< MyType * >(thp)->fillRedVar( static_cast< MyType * >(thp)->param.redPole );
        static_cast< MyType * >(thp)->mult(gain);
    }

    /*

 fillRedVar
 * fillPurpleVar
 */

};


// --------------------------------------------------------------------
} // end namespace yacynth
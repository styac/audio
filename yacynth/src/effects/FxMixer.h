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

#include    "FxBase.h"
#include    "Ebuffer.h"
#include    "yacynth_globals.h"
#include    "protocol.h"


#include    <array>
#include    <iostream>

namespace yacynth {
using namespace TagEffectTypeLevel_02;
// --------------------------------------------------------------------

class FxMixerParam {
public:
    FxMixerParam();
    // mandatory fields
    static constexpr char const * const name    = "Mixer4";
    static constexpr TagEffectType  type        = TagEffectType::FxMixer;
    static constexpr std::size_t maxMode        = 1;
    static constexpr std::size_t inputCount     = 4;

    inline void clear()
    {

    }
    
    bool parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex ); 
    
    ControlledValue<1> gainTarget;
};

class FxMixer : public Fx<FxMixerParam>  {
public:
    using MyType = FxMixer;
    FxMixer()
    :   Fx<FxMixerParam>()

    {
        gain[0] = 1.0f;
        fillSprocessv<0>(sprocess_00);
        fillSprocessv<1>(sprocess_01);
//        fillSprocessv<2>(sprocess_02);
//        fillSprocessv<3>(sprocess_03);
//        fillSprocessv<4>(sprocess_04);
//        fillSprocessv<5>(sprocess_05);
    }
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
        if( getMaxMode() < procMode ) {
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
        // sprocesspSave =  sprocessv[ th.procMode ];
        // sprocessp = sprocessTransient;
        return true;
    }

    // go up to Fx ?? virtual ?
#if 0
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
    inline void dump( float * channelA,  float * channelB )
    {
        out().dump( channelA, channelB );
    }

    virtual bool connect( const FxBase * v, uint16_t ind ) override;

    virtual bool parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex ) override; 
    
    virtual void clearTransient() override;

private:

    // go up to Fx ???
    static void sprocessTransient( void * thp )
    {
        FxMixer& th = *static_cast< FxMixer * >(thp);
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
       // static_cast< FxOutNoise * >(thp)->clear();
    }

    static void sprocess_01( void * thp )
    {
        static_cast< MyType * >(thp)->mix_01();
    }

    static void sprocess_02( void * thp )
    {
        // static_cast< FxOutNoise * >(thp)->fillWhiteStereo();
    }


    // TODO check ControllerCacheDelta usage 
    inline void mix_01(void)
    {
        constexpr float fadeGain =  (1.0f/(1<<6));
        if( param.gainTarget.updateDiff() ) {
            out().fade( inp(), gain[0], ( param.gainTarget.getExpValue() - gain[0] ) * fadeGain );
        } else {
            out().mult( inp(), gain[0] );
        }
    }

    float   gain[ FxMixerParam::inputCount ];
};

// --------------------------------------------------------------------
} // end namespace yacynth


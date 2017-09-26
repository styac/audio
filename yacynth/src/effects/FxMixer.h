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

#include    "FxMixerParam.h"
#include    "Ebuffer.h"
#include    "yacynth_globals.h"
//#include    "protocol.h"
#include    "../effects/FxBase.h"

#include    <array>
#include    <iostream>

namespace yacynth {

class FxMixer : public Fx<FxMixerParam>  {
public:
    using MyType = FxMixer;
    FxMixer()
    :   Fx<FxMixerParam>()
    {
        fillSprocessv<0>(sprocess_00);
        fillSprocessv<1>(sprocess_01);
    }
    // go up to Fx ??
    // might change -> set sprocessTransient
    // FIRST TEST WITHOUT TRANSIENT
    // THEN  WITH TRANSIENT -> all types > out,
    // 00 is always clear for output or bypass for in-out == effect OFF
    virtual bool setProcMode( uint16_t ind )  override;

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
    inline void dump( float * channel0,  float * channel1 )
    {
        out().dump( channel0, channel1 );
    }

    virtual bool connect( const FxBase * v, uint16_t ind ) override;

    virtual bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex ) override;

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

    static void sprocess_00( void * thp );
    static void sprocess_01( void * thp );

    inline void process()
    {
        // channel 0 - MASTER VOLUME
        const bool isMasterVolumeChanged = gainCache[ 0 ].update( param.gainIndex[ 0 ] );
        if( isMasterVolumeChanged ) {
            const float cgainChA = gainCache[ 0 ].getExpValueFloat() * param.gainRange[ 0 ][ chA ];
            const float cgainChB = gainCache[ 0 ].getExpValueFloat() * param.gainRange[ 0 ][ chB ];
            out().fadeAddV4( inp<0>(), gain[ 0 ][ chA ], gain[ 0 ][ chB ], 
                ( cgainChA - gain[ 0 ][ chA ] ), ( cgainChB - gain[ 0 ][ chB ] ) );
        } else {
            out().multSet( inp<0>(), gain[ 0 ][ chA ], gain[ 0 ][ chB ] );
        }

        // channel k = MASTER VOLUME * channel volume
        for( auto cin = 1u; cin < param.effectiveInputCount; ++cin ) {
            if( param.gainZero[ cin ] ) {
                if( gainCache[ cin ].update( param.gainIndex[ cin ] ) || isMasterVolumeChanged ) {
                    const float cgainChA = gainCache[ cin ].getExpValueFloat() * param.gainRange[ cin ][ chA ] * gain[ 0 ][ chA ];
                    const float cgainChB = gainCache[ cin ].getExpValueFloat() * param.gainRange[ cin ][ chB ] * gain[ 0 ][ chB ];
                    out().fadeAddV4( inp( cin ), gain[ cin ][ chA ], gain[ cin ][ chB ], 
                        ( cgainChA - gain[ cin ][ chA ] ), ( cgainChB - gain[ cin ][ chB ] ) );
                } else {
                    out().multSet( inp( cin ), gain[ cin ][ chA ], gain[ cin ][ chB ] );
                }
            }
        }        
    }
    
    float   gain[ FxMixerParam::inputCount ][ 2 ]; // for each stereo channel
    ControllerCache gainCache[FxMixerParam::inputCount];
//    ControllerCacheRate<8> gainCache[FxMixerParam::inputCount];
};

// --------------------------------------------------------------------
} // end namespace yacynth


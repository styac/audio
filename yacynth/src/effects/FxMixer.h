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
    static constexpr char const * const name    = "Mixer4";
    static constexpr TagEffectType  type        = TagEffectType::FxMixer;
    static constexpr std::size_t maxMode        = 4;
    static constexpr std::size_t inputCount     = 4;

    inline void clear()
    {
        for( auto &p : gainIndex ) {
            p.setIndex( InnerController::CC_NULL );
        }
    }
    
    bool parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex ); 
        
    ControllerIndex     gainIndex[inputCount];
     // range : n * -6 dB step
    float   gainRange[ inputCount ] = {0.5f, 0.5f, 0.5f, 0.5f };
};

class FxMixer : public Fx<FxMixerParam>  {
public:
    using MyType = FxMixer;
    FxMixer()
    :   Fx<FxMixerParam>()
    {
        fillSprocessv<0>(sprocess_00);
        fillSprocessv<1>(sprocess_01);
        fillSprocessv<2>(sprocess_02);
        fillSprocessv<3>(sprocess_03);
        fillSprocessv<4>(sprocess_04);
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

    static void sprocess_00( void * thp );
    static void sprocess_01( void * thp );
    static void sprocess_02( void * thp );
    static void sprocess_03( void * thp );
    static void sprocess_04( void * thp );

    // TODO check ControllerCacheDelta usage vs fadeV4
    inline void mix_01(void)
    {
        if( gainCache[0].update( param.gainIndex[0] )) {            
            out().fadeV4( inp<0>(), gain[0], ( gainCache[ 0 ].getExpValueFloat() * param.gainRange[ 0 ] - gain[ 0 ] ) );
        } else {
            out().mult( inp<0>(), gain[0] );
        }
    }
    
    // use master controller only = 0
    inline void mix_02(void)
    {
        if( gainCache[0].update( param.gainIndex[0] )) {  
            float cval0 = gainCache[ 0 ].getExpValueFloat();
            out().fadeV4( inp<0>(), inp<1>(), 
                    gain[0], gain[1], 
                    ( cval0 * param.gainRange[ 0 ] - gain[ 0 ] ), 
                    ( cval0 * param.gainRange[ 1 ] - gain[ 1 ] ) ); // controller 0 !!
                    
        } else {
            out().mult( inp<0>(), inp<1>(), gain[0], gain[1] );
        }
    }

    // use master controller only = 0
    inline void mix_03(void)
    {
        if( gainCache[0].update( param.gainIndex[0] )) {            
            float cval0 = gainCache[ 0 ].getExpValueFloat();
            out().fadeV4( inp<0>(), inp<1>(), inp<2>(), 
                    gain[0], gain[1], gain[2], 
                    ( cval0 * param.gainRange[ 0 ] - gain[ 0 ] ), 
                    ( cval0 * param.gainRange[ 1 ] - gain[ 1 ] ), // controller 0 !!
                    ( cval0 * param.gainRange[ 2 ] - gain[ 2 ] ) ); // controller 0 !!
        } else {
            out().mult( inp<0>(), inp<1>(), inp<2>(), gain[0], gain[1], gain[2] );
        }
    }

    // use master controller only = 0
    inline void mix_04(void)
    {
        if( gainCache[0].update( param.gainIndex[0] )) {
            float cval0 = gainCache[ 0 ].getExpValueFloat();            
            out().fadeV4( inp<0>(), inp<1>(), inp<2>(), inp<3>(), 
                    gain[0], gain[1], gain[2], gain[3], 
                    ( cval0 * param.gainRange[ 0 ] - gain[ 0 ] ), 
                    ( cval0 * param.gainRange[ 1 ] - gain[ 1 ] ), // controller 0 !!
                    ( cval0 * param.gainRange[ 2 ] - gain[ 2 ] ), // controller 0 !!
                    ( cval0 * param.gainRange[ 3 ] - gain[ 3 ] ) ); // controller 0 !!
        } else {
            out().mult( inp<0>(), inp<1>(), inp<2>(), inp<3>(), gain[0], gain[1], gain[2], gain[3] );
        }
    }

    // TODO - set by output and use this to output
    // float * outchannel[2];
    
    float   gain[ FxMixerParam::inputCount ];
    
    ControllerCache gainCache[FxMixerParam::inputCount];
};

// --------------------------------------------------------------------
} // end namespace yacynth


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

#include    "FxBase.h"
#include    "protocol.h"

namespace yacynth {
using namespace TagEffectTypeLevel_02;

class FxModulatorParam {
public:
    // mandatory fields
    static constexpr char const * const name    = "Modulator";
    static constexpr TagEffectType  type        = TagEffectType::FxModulator;
    static constexpr std::size_t maxMode        = 4; // 0 is always bypass
    static constexpr std::size_t inputCount     = 2; // 0 : base signal; 1 : modulation
    static constexpr char const * const modeName[maxMode+1] = 
    {   "copy"
    ,   "processModulation"
    ,   "processRing"
    ,   "processModulationMix"
    ,   "processRingVolColtrol"        
    };
      

    FxModulatorParam();
    
    bool parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex ); 
        
    // optional fields
    // AM:      inMult = gain   ; mixMult = gain * modIndex
    // RING:                    ; mixMult = gain 

    struct Mode_01_amplitudeModulation {
        ControllerIndex inMultIndex;
        ControllerIndex mixMultIndex;        
    } mode_01_amplitudeModulation;

    struct Mode_02_ringModulation {        
        ControllerIndex inMultIndex;           
    } mode_02_ringModulation;
            
    // obsolate
    float inMult;
    float mixMult;
};

class FxModulator : public Fx<FxModulatorParam>  {
public:
    using MyType = FxModulator;
    FxModulator()
    :   Fx<FxModulatorParam>()
    {
        fillSprocessv<0>(sprocess_00);
        fillSprocessv<1>(sprocess_01);
        fillSprocessv<2>(sprocess_02);
        fillSprocessv<3>(sprocess_03);
        fillSprocessv<4>(sprocess_04);
    }

    virtual bool parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex ) override; 
    
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
    virtual bool connect( const FxBase * v, uint16_t ind ) override;

private:
    // go up to Fx ???
    static void sprocessTransient( void * thp );

    static void sprocess_00( void * thp );  // bypass > inp<0> -> out
    static void sprocess_01( void * thp );
    static void sprocess_02( void * thp );
    static void sprocess_03( void * thp );
    static void sprocess_04( void * thp );

    
    /**
     *  if( param.gainTarget.updateDiff() ) {
            out().fade( inp(), gain, ( param.gainTarget.getExpValue() - gain ) * (1.0f/(1<<6)) );
        } else {
            out().mult( inp(), gain );
        }
     * 
     */
#if 0
    inline void processModulation(void)
    {
        inMult = param.mode_01_amplitudeModulation.inMultIndex.getExpValue();
        mixMult = inMult * param.mode_01_amplitudeModulation.mixMultIndex.getExpValue();
        for( auto si=0u; si < vsectionSize; ++si ) {
            out().vchannel[0][si] = inp<0>().vchannel[0][si] * inp<1>().vchannel[0][si] * param.mixMult + inp<0>().vchannel[0][si] * param.inMult;
            out().vchannel[1][si] = inp<0>().vchannel[1][si] * inp<1>().vchannel[1][si] * param.mixMult + inp<0>().vchannel[1][si] * param.inMult;
        }
    }
    
    inline void processRing(void)
    {
        inMult = param.mode_02_ringModulation.inMultIndex.getExpValue();
        for( auto si=0u; si < vsectionSize; ++si ) {
            out().vchannel[0][si] = inp<0>().vchannel[0][si] * inp<1>().vchannel[0][si] * param.mixMult; // inMult
            out().vchannel[1][si] = inp<0>().vchannel[1][si] * inp<1>().vchannel[1][si] * param.mixMult; // inMult
        }
    }
#endif      

    inline void processModulationMix(void)
    {
        inMultIn.updateDelta(param.mode_01_amplitudeModulation.inMultIndex);
        mixMultIn.updateDelta(param.mode_01_amplitudeModulation.mixMultIndex);
        const float inMult = inMultIn.getExpValue();
        const float mixMult = mixMultIn.getExpValue();
        for( auto si=0u; si < vsectionSize; ++si ) {
            out().vchannel[0][si] = inp<0>().vchannel[0][si] * inp<1>().vchannel[0][si] * mixMult + inp<0>().vchannel[0][si] * inMult;
            out().vchannel[1][si] = inp<0>().vchannel[1][si] * inp<1>().vchannel[1][si] * mixMult + inp<0>().vchannel[1][si] * inMult;
        }
    }
    
    inline void processModulation(void)
    {
        mixMultIn.updateDelta( param.mode_01_amplitudeModulation.mixMultIndex );
        const float mixMult = mixMultIn.getExpValue();
        for( auto si=0u; si < vsectionSize; ++si ) {
//            std::cout << "in0 " << inp<0>().channel[0][si] << " in1 " <<  inp<1>().channel[0][si] << std::endl;
            out().vchannel[0][si] = inp<0>().vchannel[0][si] * ( inp<1>().vchannel[0][si] * mixMult + 1.0f );
            out().vchannel[1][si] = inp<0>().vchannel[1][si] * ( inp<1>().vchannel[1][si] * mixMult + 1.0f );
        }
    }

    inline void processRingVolColtrol(void)
    {
        inMultIn.updateDelta(param.mode_02_ringModulation.inMultIndex);
        const float inMult = inMultIn.getExpValue();
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
        
    ControllerCacheDelta<10> inMultIn;      // TODO test by hearing if limit is hearable
    ControllerCacheDelta<10> mixMultIn;     // TODO test by hearing
};


} // end namespace yacynth


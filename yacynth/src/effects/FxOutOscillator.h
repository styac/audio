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
 * File:   FxOutOscillator.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on June 23, 2016, 7:46 PM
 */

#include    "FxBase.h"
#include    "../utils/Fastsincos.h"

using namespace tables;

namespace yacynth {


class FxOutOscillatorParam {
public:
    FxOutOscillatorParam();
    // mandatory fields
    static constexpr char const * const name = "Oscillator";
    static constexpr std::size_t maxMode     = 5; // 0 is always exist> 0,1,2
    static constexpr std::size_t inputCount  = 0; //  0-base signal 1-modulation
    static constexpr std::size_t slaveCount  = 3; //  0-base signal 1-modulation
    static constexpr char const * const slavename = " ^OscillatorSlave";


    // optional fields

//    uint32_t        phaseDelta;
    ControlledValue phaseDelta0;
    ControlledValue phaseDelta1;
    ControlledValue phaseDiff;
    
};

class FxOutOscillator : public Fx<FxOutOscillatorParam>  {
public:
    using MyType = FxOutOscillator;
    FxOutOscillator()
    :   Fx<FxOutOscillatorParam>()
    {
        for( auto& si : slaves ) si.setMasterId(id());

        fillSprocessv<0>(sprocess_00);
        fillSprocessv<1>(sprocess_01);
        fillSprocessv<2>(sprocess_02);
        fillSprocessv<3>(sprocess_03);
        fillSprocessv<4>(sprocess_04);
        fillSprocessv<5>(sprocess_05);
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
        case 3:
            return sprocess_03;
        case 4:
            return sprocess_04;
        case 5:
            return sprocess_05;
        default:
            return sprocessp; // illegal index no change
        }
    }
#endif
    virtual bool connect( const FxBase * v, uint16_t ind ) override;

    // bool connectSlaves( const FxBase * v, uint16_t ind );


private:
    // go up to Fx ???
    // this should interpolate the old and new
    static void sprocessTransient( void * thp );

    static void sprocess_00( void * thp );  // bypass > inp<0> -> out
    static void sprocess_01( void * thp );
    static void sprocess_02( void * thp );
    static void sprocess_03( void * thp );
    static void sprocess_04( void * thp );
    static void sprocess_05( void * thp );
    

    inline void processSine(void)
    {
        for( auto si=0u; si < sectionSize; ++si ) {
            inc();
            out().channel[0][si] = sinTable[(phase[0])>>16];
            out().channel[1][si] = sinTable[(phase[1])>>16];
        }
    }

    inline void processSinePd0(void)
    {
        for( auto si=0u; si < sectionSize; ++si ) {
            inc();
            const uint16_t phase0 = (phase[0])>>16;
            const uint16_t phase1 = (phase[1])>>16;
            out().channel[0][si] = sinTable[ uint16_t( (waveSinTable[phase0]>>1) + phase0 ) ];
            out().channel[1][si] = sinTable[ uint16_t( (waveSinTable[phase1]>>1) + phase1 ) ];
        }
    }

    inline void processSinePd1(void)
    {
        for( auto si=0u; si < sectionSize; ++si ) {
            inc();
            const uint16_t phase0 = (phase[0])>>16;
            const uint16_t phase1 = (phase[1])>>16;
            out().channel[0][si] = sinTable[ uint16_t( (waveSinTable[phase0]>>2) + phase0 ) ];
            out().channel[1][si] = sinTable[ uint16_t( (waveSinTable[phase1]>>2) + phase1 ) ];
        }
    }

    inline void processSinePd2(void)
    {
        for( auto si=0u; si < sectionSize; ++si ) {
            inc();
            const uint16_t phase0 = (phase[0])>>16;
            const uint16_t phase1 = (phase[1])>>16;
            out().channel[0][si] = sinTable[ uint16_t( (waveSinTable[phase0]>>3) + phase0 ) ];
            out().channel[1][si] = sinTable[ uint16_t( (waveSinTable[phase1]>>3) + phase1 ) ];
        }
    }

    inline void processSinePd3(void)
    {
        for( auto si=0u; si < sectionSize; ++si ) {
            inc();
            const uint16_t phase0 = (phase[0])>>16;
            const uint16_t phase1 = (phase[1])>>16;
            out().channel[0][si] = sinTable[ uint16_t( (waveSinTable[phase0])) ];
            out().channel[1][si] = sinTable[ uint16_t( (waveSinTable[phase1])) ];
        }
    }

    inline void updateParam(void)
    {
        if( param.phaseDiff.update() ) {    
            phase[1] -= phaseDiff;            
            phaseDiff = param.phaseDiff.getPhaseValue();
            phase[1] += phaseDiff;            
        }
        
        if( param.phaseDelta0.update() ) {            
            phaseDelta0 = tables::ExpTable::getInstance().ycent2deltafi( param.phaseDelta0.getYcent8Value() );
        }        
        if( param.phaseDelta1.update() ) {            
            phaseDelta1 = tables::ExpTable::getInstance().ycent2deltafi( param.phaseDelta1.getYcent8Value() );
        }        
    }

    inline void inc(void)
    {
        phase[0] += phaseDelta0;
        phase[1] += phaseDelta1;
    }

    
    uint32_t                        phaseDiff;
    uint32_t                        phaseDelta0;
    uint32_t                        phaseDelta1;
    
    uint32_t                        phase[ FxOutOscillatorParam::slaveCount*2 + 2 ];
    FxSlave<FxOutOscillatorParam>   slaves[ FxOutOscillatorParam::slaveCount ];
};

} // end namespace yacynth


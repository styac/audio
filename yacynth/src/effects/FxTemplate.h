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
 * File:   FxTemplate.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on June 23, 2016, 7:46 PM
 */

#include    "FxBase.h"
#include    "../utils/Fastsincos.h"

using namespace tables;

namespace yacynth {
using namespace TagEffectTypeLevel_02;

class FxTemplateParam {
public:
    // mandatory fields
    static constexpr char const * const name    = "Template"; // 1 main + 3 slave
    static constexpr TagEffectType  type        = TagEffectType::FxTemplate;
    static constexpr std::size_t maxMode        = 1; // 0 is always exist> 0,1,2
    static constexpr std::size_t inputCount     = 0; 
    //static constexpr std::size_t slaveCount     = 3; // 0-base signal 1-modulation
    //static constexpr char const * const slavename = " ^OscillatorSlave";

    bool parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );     
};

class FxTemplate : public Fx<FxTemplateParam>  {
public:
    using MyType = FxTemplate;
    FxTemplate()
    :   Fx<FxTemplateParam>()
    {
        fillSprocessv<0>(sprocess_00);
    }
    
    virtual bool parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex ); 
    

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

private:
    virtual void clearTransient(void) override;    

    static void sprocessTransient( void * thp );

    static void sprocess_00( void * thp );  // bypass > inp<0> -> out

    inline void process(void)
    {
    }
};


} // end namespace yacynth


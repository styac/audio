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
#include    "oscillator/NoiseFrame.h"
#include    "utils/GaloisNoiser.h"
#include    "effects/FxBase.h"

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
    static void sprocess_01( void * thp );
    static void sprocess_02( void * thp );
    static void sprocess_03( void * thp );
    static void sprocess_04( void * thp );
    static void sprocess_05( void * thp );
    static void sprocess_06( void * thp );
    static void sprocess_07( void * thp );
    static void sprocess_08( void * thp );
    static void sprocess_09( void * thp );
    static void sprocess_10( void * thp );
    static void sprocess_11( void * thp );
    static void sprocess_12( void * thp );
    static void sprocess_13( void * thp );
    static void sprocess_14( void * thp );
    static void sprocess_15( void * thp );
};

// --------------------------------------------------------------------
} // end namespace yacynth
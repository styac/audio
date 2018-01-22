#pragma once
/*
 * Copyright (C) 2017 ist
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
 * File:   FxInput.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on September 16, 2017, 10:37 AM
 */

#include    "yacynth_globals.h"
#include    "effects/FxBase.h"
#include    "v4.h"
#include    "FxInputParam.h"

namespace yacynth {

class FxInput : public Fx<FxInputParam>  {
public:
    using MyType = FxInput;
    FxInput()
    :   Fx<FxInputParam>()
    {
    }

    virtual bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex ) override;

    virtual void clearState() override;

    virtual bool setSprocessNext( uint16_t mode ) override;

    virtual bool connect( const FxBase * v, uint16_t ind ) override;

    // real process - direct called by IOThread
    inline void process( float *ch0, float *ch1 )
    {
        out().load( ch0, ch1 );
    }

private:

};

} // end namespace yacynth


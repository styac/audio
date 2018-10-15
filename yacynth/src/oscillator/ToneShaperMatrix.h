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
 * File:   ToneShaperMatrix.h
 * Author: Istvan Simon
 *
 * Created on April 4, 2016, 11:52 AM
 */
#define TONESHAPER_CTOR

#include "ToneShaper.h"
#include "Tags.h"
#include "protocol.h"


namespace yacynth {

class ToneShaperMatrix {
    static constexpr const char * const typeName = "ToneShaperMatrix";

private:

public:
    ToneShaperMatrix() = default;
    void clear( void );
    bool setPitchVector( int32_t *pitchVector, uint16_t pitchCount, uint16_t vectorIndex );
    bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );
    std::array< ToneShaperVector,  settingVectorSize>   toneShapers;
};


}

// --------------------------------------------------------------------
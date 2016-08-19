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
 * File:   FilterControl.h
 * Author: Istvan Simon
 *
 * Created on May 5, 2016, 4:17 PM
 */

#include    <cstdint>
#include    <cmath>
#include    <algorithm>

namespace yacynth {
// gain, q preset ? not dynamic
// controls - frequency:
//  1.  MIDI,OSC CC - per channel min,max, range
// LFO updated 1x / frame
//  2.  LFO - per channel: phase, amplitude, midpoint
//  3.  amplitude - per channel, min-point, amplitude

// LFO updated 1x / sample
//  per output amplitude modulation -> multiphase, amplitude


// controller <= minPoint : value = minValue
// controller >= maxPoint : value = maxValue
// value = min + (max-min) * controller / (maxPoint-minPoint)



struct FilterChannelControl {

    void    setGain( const float v )
    { gain=v;};
    void    setQ( const float v )
    { q=v;};

    float       q;
    float       gain;
    uint32_t    pitch;  // log frequency

    uint32_t    minPitch;
    uint32_t    maxPitch;

    // manual control limits
    uint32_t    minPointPitch;
    uint32_t    maxPointPitch;

    // filter freq modulator
    uint32_t    filterLfoAmplitude;
    uint32_t    filterLfoPitch;
    uint32_t    filterLfoPhase;

    // AM for the input??
    float       aModLfoAmplitude;
    uint32_t    aModLfoDeltaPhase;
    uint32_t    aModLfoPhase;      // start phase
    uint8_t     changed;   // ???

    // TODO: amplitude follower is missing
};

template< std::size_t channelCountExp >
struct FilterControl {
    static constexpr uint8_t  channelCount      = 1<<channelCountExp;
    static constexpr uint8_t  channelCountMask  = channelCount-1;

    void setMode( const uint8_t mode  )
    {
        filterMode = mode;
        // activeFilterCount = afc[filterMode]; // ???
    };
    FilterChannelControl    channelParam[ channelCount ];
    uint8_t                 filterMode;         // -> ENUM
    uint8_t                 controlMode;        // -> ENUM
    uint8_t                 activeFilterCount;  // max is compile time
    uint8_t                 changedBitmap;      // bit 1 -> changed ???
};

} // end namespace yacynth


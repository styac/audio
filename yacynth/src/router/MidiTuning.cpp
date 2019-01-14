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
 * File:   MidiTuning.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on November 6, 2017, 5:37 PM
 */
#include "yacynth_config.h"

#include "MidiTuning.h"
#include "protocol.h"
#include "Tags.h"
#include <iostream>
#include <fstream>
#include <iomanip>

namespace yacynth {

//namespace {
//constexpr auto LogCategoryMask              = LOGCAT_net;
//constexpr auto LogCategoryMaskAlways        = LogCategoryMask | nanolog::category_mask_t::log_always;
//constexpr const char * const LogCategory    = "NETS";
//}

using namespace TagTunerLevel_01;

bool MidiTuning::parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex )
{
    const uint8_t tag = message.getTag(tagIndex);
    switch( TagTuner( tag ) ) {
    case TagTuner::Clear:
    case TagTuner::ClearState:
        TAG_DEBUG(TagTuner::Clear, tagIndex, paramIndex, "MidiTuningTables" );
        clear();
        break;

    default:
        break;
    }
    return TuningManager::getInstance().parameter( message, tagIndex, paramIndex );
}


MidiTuning::MidiTuning()
{
    clear();
}

void MidiTuning::clear()
{
    memset( &channelTable, 0, sizeof(channelTable) );
}


} // end namespace yacynth 

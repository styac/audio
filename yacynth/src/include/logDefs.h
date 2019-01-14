#pragma once
/*
 * Copyright (C) 2019 ist
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
 * File:   logDefs.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on January 12, 2019, 4:38 PM
 */

#include "NanoLog.hpp"

namespace yacynth {

enum LOG_CATEGORY_ID : nanolog::category_mask_t::value_type {
    LOGCAT_main         = 1LL << 0,
    LOGCAT_net          = 1LL << 1,
    LOGCAT_synthfe      = 1LL << 2,
    LOGCAT_iothread     = 1LL << 3,
    LOGCAT_midi         = 1LL << 4,
    LOGCAT_osc          = 1LL << 5,
};

/*
#define TAG_DEBUG( tagname, tagi, pari, comment )\
std::cout \
    << "tag: " << #tagname \
    << " " << int16_t(tagname) \
    << " t-ind: " << int16_t(tagi)\
    << " p-ind: " << int16_t(pari) \
    << "  " << comment \
    << std::endl;
 
 */

} // end namespace yacynth 



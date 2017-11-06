#pragma once

/*
 * Copyright (C) 2017 Istvan Simon
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
 * File:   Osc.h
 * Author: Istvan Simon
 *
 * Created on October 31, 2017, 6:41 PM
 */
// http://cnmat.berkeley.edu/content/open-sound-control-11-encoding-specification

namespace yacynth {
namespace net {
namespace osc {

struct Osc {
    // /yxp/<address>/ming,b <blob>
    static constexpr char * const root      = "/yxp";       // Yacynth eXchange Protocol
    static constexpr char * const ming      = "/ming,b";    // Music Interface Next Generation with blob
};

// e.g.: /yxp/ming,b length ming sequence n x 8 x byte

} // end namespace osc
} // end namespace yacnet
} // end namespace yacynth


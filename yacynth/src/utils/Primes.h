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
 * File:   Primes.h
 * Author: Istvan Simon
 *
 * Created on March 25, 2016, 11:32 PM
 */

#include <cstdint>
#include <array>

namespace tables {

struct Primes {
    static constexpr  uint32_t count = (1<<16)+4096+1;
    static uint32_t   table[count];
};

extern const  Primes primes;

} // end namespace tables
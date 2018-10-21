#pragma once

/*
 * Copyright (C) 2018 Istvan Simon -- stevens37 at gmail dot com
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

constexpr double PI = 3.141592653589793238462643383279502884197169399375105820974944592307816;

#include "NoiseFrame.h"

using namespace yacynth;
using namespace noiser;
using namespace std;


constexpr std::size_t frameSizeExp = 10;
constexpr std::size_t pulseCountExp = 6;

typedef FrameInt<frameSizeExp,2> FrameType;
typedef NoiseFrame<FrameType> NoiseFrameType;
typedef FrameIntInterleave<frameSizeExp,2> FrameInterleaveType;





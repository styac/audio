#pragma once
/*
 * Copyright (C) 2018 ist
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
 * File:   yacynth_config.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on October 15, 2018, 6:36 PM
 */

#ifdef YCONF_OVERSAMPLING_RATE
#if YCONF_OVERSAMPLING_RATE==2
// 96000Hz
constexpr int  oversamplingRate     = 2;
#elif YCONF_OVERSAMPLING_RATE==4
// 192000Hz
constexpr int  oversamplingRate     = 4;
#else
constexpr int  oversamplingRate     = 1;
#endif
#else
constexpr int  oversamplingRate     = 1;
#endif
 
#ifdef YCONF_FRAME_SIZEEXP
#if YCONF_FRAME_SIZEEXP>3 && YCONF_FRAME_SIZEEXP<11
constexpr int  frameSizeExp         = YCONF_FRAME_SIZE;
#else
constexpr int  frameSizeExp         = 6;
#endif
#else
constexpr int  frameSizeExp         = 6;
#endif



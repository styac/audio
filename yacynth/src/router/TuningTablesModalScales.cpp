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
 * File:   TuningTablesModalScales.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on November 14, 2017, 9:20 PM
 */

#include    "TuningConst.h"

namespace Tuning {

#define MAKE_MS12_TABLE(p,n) \
    ModalSteps12 modalTable_ ## p ## _ ## n \
        ( tuningGenerator_ ## p,sizeof(modalTable_ ## p ## _ ## n ## __), modalTable_ ## p ## _ ## n ## __ );

TuningGenerator tuningGenerator_TM_21_ET_72(72,2,1);

// 72 tone equal modes:
// 7 5 7 4 7 5 7 7 4 7 5 7 	"Just" Chromatic
// 

static uint8_t modalTable_TM_21_ET_72_JustChromatic__ [] =
    {
        7, 5, 7, 4, 7, 5, 7, 7, 4, 7, 5, 7,        
    };

// name modalTable_TM_21_ET_72_JustChromatic

MAKE_MS12_TABLE(TM_21_ET_72,JustChromatic);

} // end namespace Tuning 


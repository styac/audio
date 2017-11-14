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
 * File:   TuningTablesInternal.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on November 8, 2017, 7:25 PM
 */

#include    "TuningConst.h"

namespace Tuning {

#define COUNT_ELEMENT(t) ( sizeof(t)/sizeof(double) )

#define MAKE_TM_TABLE(n,d,p) \
    TuningGenerator table_ ## p \
        ( COUNT_ELEMENT(table_ ## p ## __), n, d, table_ ## p ## __ );
        
static double table_TM_21_Werckmeister3__[] =
    {
        90.225  * cent2ycent,   // 01
        192.18  * cent2ycent,   // 02
        294.135 * cent2ycent,   // 03
        390.225 * cent2ycent,   // 04
        498.045 * cent2ycent,   // 05
        588.27  * cent2ycent,   // 06
        696.09  * cent2ycent,   // 07
        792.18  * cent2ycent,   // 08
        888.27  * cent2ycent,   // 09
        996.09  * cent2ycent,   // 10        
        1092.18 * cent2ycent,   // 11
    };

static double table_TM_21_Young2__[] =
    {
        93.9    * cent2ycent,   // 01
        195.8   * cent2ycent,   // 02
        297.8   * cent2ycent,   // 03
        391.7   * cent2ycent,   // 04
        499.9   * cent2ycent,   // 05
        591.9   * cent2ycent,   // 06
        697.9   * cent2ycent,   // 07
        795.8   * cent2ycent,   // 08
        893.8   * cent2ycent,   // 09
        999.8   * cent2ycent,   // 10        
        1091.8  * cent2ycent,   // 11
    };

static double table_TM_21_Pythagorean__[] =
    {
        interval2ycent(  2187.0 / 2048.0 ),    // 01
        interval2ycent(     9.0 /    8.0 ),    // 02
        interval2ycent(    32.0 /   27.0 ),    // 03
        interval2ycent(    81.0 /   64.0 ),    // 04
        interval2ycent(     4.0 /    3.0 ),    // 05
        interval2ycent(   729.0 /  512.0 ),    // 06
        interval2ycent(     3.0 /    2.0 ),    // 07
        interval2ycent(   128.0 /   81.0 ),    // 08
        interval2ycent(    27.0 /   16.0 ),    // 09
        interval2ycent(    16.0 /    9.0 ),    // 10
        interval2ycent(   243.0 /  128.0 ),    // 11
    };

static double table_TM_21_Ptolemy__[] =
    {
        interval2ycent(  16.0 / 15.0 ),    // 01
        interval2ycent(   9.0 /  8.0 ),    // 02
        interval2ycent(   6.0 /  5.0 ),    // 03
        interval2ycent(   5.0 /  4.0 ),    // 04
        interval2ycent(   4.0 /  3.0 ),    // 05
        interval2ycent(  45.0 / 32.0 ),    // 06
        interval2ycent(   3.0 /  2.0 ),    // 07
        interval2ycent(   8.0 /  5.0 ),    // 08
        interval2ycent(   5.0 /  3.0 ),    // 09
        interval2ycent(   9.0 /  5.0 ),    // 10
        interval2ycent(  15.0 /  8.0 ),    // 11
    };

static double table_TM_21_Partch43__[] =
    {
// ===========
        interval2ycent(  81.0 / 80.0 ),    // 01
        interval2ycent(  33.0 / 32.0 ),    // 02
        interval2ycent(  21.0 / 20.0 ),    // 03
        interval2ycent(  16.0 / 15.0 ),    // 04 - C#
        interval2ycent(  12.0 / 11.0 ),    // 05
        interval2ycent(  11.0 / 10.0 ),    // 06
        interval2ycent(  10.0 /  9.0 ),    // 07
        interval2ycent(   9.0 /  8.0 ),    // 08 - D
        interval2ycent(   8.0 /  7.0 ),    // 09
// ===========
        interval2ycent(   7.0 /  6.0 ),    // 10
        interval2ycent(  32.0 / 27.0 ),    // 11
        interval2ycent(   6.0 /  5.0 ),    // 12 - Eb
        interval2ycent(  11.0 /  9.0 ),    // 13
        interval2ycent(   5.0 /  4.0 ),    // 14 - E
        interval2ycent(  14.0 / 11.0 ),    // 15
        interval2ycent(   9.0 /  7.0 ),    // 16
        interval2ycent(  21.0 / 16.0 ),    // 17
        interval2ycent(   4.0 /  3.0 ),    // 18 - F
        interval2ycent(  27.0 / 20.0 ),    // 19
// ===========
        interval2ycent(  11.0 /  8.0 ),    // 20
        interval2ycent(   7.0 /  5.0 ),    // 21
        interval2ycent(  10.0 /  7.0 ),    // 22
        interval2ycent(  16.0 / 11.0 ),    // 23
        interval2ycent(  40.0 / 27.0 ),    // 24
        interval2ycent(   3.0 /  2.0 ),    // 25 - G
        interval2ycent(  32.0 / 21.0 ),    // 26
        interval2ycent(  14.0 /  9.0 ),    // 27
        interval2ycent(  11.0 /  7.0 ),    // 28
        interval2ycent(   8.0 /  5.0 ),    // 29
// ===========
        interval2ycent(  18.0 / 11.0 ),    // 30
        interval2ycent(   5.0 /  3.0 ),    // 31 - A
        interval2ycent(  27.0 / 16.0 ),    // 32
        interval2ycent(  12.0 /  7.0 ),    // 33
        interval2ycent(   7.0 /  4.0 ),    // 34 - H
        interval2ycent(  16.0 /  9.0 ),    // 35
        interval2ycent(   9.0 /  5.0 ),    // 36
        interval2ycent(  20.0 / 11.0 ),    // 37
        interval2ycent(  11.0 /  6.0 ),    // 38
        interval2ycent(  15.0 /  8.0 ),    // 39
// ===========
        interval2ycent(  40.0 / 21.0 ),    // 40
        interval2ycent(  64.0 / 33.0 ),    // 41
        interval2ycent( 160.0 / 81.0 ),    // 42
// ===========
    };

static double table_TM_21_Pelog1__[] =
    {
        122     * cent2ycent,   // 01
        271     * cent2ycent,   // 02
        571     * cent2ycent,   // 03
        677     * cent2ycent,   // 04
        785     * cent2ycent,   // 05
        947     * cent2ycent,   // 06
    };
        
MAKE_TM_TABLE(2.0,1.0,TM_21_Werckmeister3);
MAKE_TM_TABLE(2.0,1.0,TM_21_Young2);
MAKE_TM_TABLE(2.0,1.0,TM_21_Pythagorean);
MAKE_TM_TABLE(2.0,1.0,TM_21_Ptolemy);
MAKE_TM_TABLE(2.0,1.0,TM_21_Partch43);
MAKE_TM_TABLE(2.0,1.0,TM_21_Pelog1);

} // end namespace xxxx 

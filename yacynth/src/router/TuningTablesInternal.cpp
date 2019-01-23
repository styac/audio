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
#include "yacynth_config.h"

#include "TuningConst.h"

namespace Tuning {

//namespace {
//constexpr auto LogCategoryMask              = LOGCAT_net;
//constexpr auto LogCategoryMaskAlways        = LogCategoryMask | nanolog::LogControl::log_always;
//constexpr const char * const LogCategory    = "NETS";
//}

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

// 17 tone modes of Arabic Pythagorean scale:
static double table_TM_21_Arabic__[] =
    {
        256.0       / 243.0,
        65536.0     / 59049.0,
        9.0         / 8.0,
        32.0        / 27.0,
        8192.0      / 6561.0,
        81.0        / 64.0,
        4.0         / 3.0,
        1024.0      / 729.0,
        262144.0    / 177147.0,
        3.0         / 2.0,
        128.0       / 81.0,
        32768.0     / 19683.0,
        27.0        / 16.0,
        16.0        / 9.0,
        4096.0      / 2187.0,
        1048576.0   / 531441.0,
    };

// 17 tone modes of Persian scale:
static double table_TM_21_Persian__[] =
    {
        256.0   /   243.0,
        27.0    /   25.0,
        9.0     /   8.0,
        32.0    /   27.0,
        243.0   /   200.0,
        81.0    /   64.0,
        4.0     /   3.0,
        25.0    /   18.0,
        36.0    /   25.0,
        3.0     /   2.0,
        128.0   /   81.0,
        81.0    /   50.0,
        27.0    /   16.0,
        16.0    /   9.0,
        729.0   /   400.0,
        243.0   /   128.0,
    };

static double table_TM_21_Shruti__[] =
    {
        256.0   /   243.0,
        16.0    /   15.0,
        10.0    /   9.0,
        9.0     /   8.0,
        32.0    /   27.0,
        6.0     /   5.0,
        5.0     /   4.0,
        81.0    /   64.0,
        4.0     /   3.0,
        27.0    /   20.0,
        45.0    /   32.0,
        729.0   /   512.0,                 // (or 64/45)
        3.0     /   2.0,
        128.0   /   81.0,
        8.0     /   5.0,
        5.0     /   3.0,
        27.0    /   16.0,
        16.0    /   9.0,
        9.0     /   5.0,
        15.0    /   8.0,
        243.0   /   128.0,
    };

MAKE_TM_TABLE(2.0,1.0,TM_21_Werckmeister3);
MAKE_TM_TABLE(2.0,1.0,TM_21_Young2);
MAKE_TM_TABLE(2.0,1.0,TM_21_Pythagorean);
MAKE_TM_TABLE(2.0,1.0,TM_21_Ptolemy);
MAKE_TM_TABLE(2.0,1.0,TM_21_Partch43);
MAKE_TM_TABLE(2.0,1.0,TM_21_Pelog1);
MAKE_TM_TABLE(2.0,1.0,TM_21_Arabic);
MAKE_TM_TABLE(2.0,1.0,TM_21_Persian);
MAKE_TM_TABLE(2.0,1.0,TM_21_Shruti);

} // end namespace xxxx

#if 0

17 tone modes of Arabic Pythagorean scale:
256/243
65536/59049
9/8
32/27
8192/6561
81/64
4/3
1024/729
262144/177147
3/2
128/81
32768/19683
27/16
16/9
4096/2187
1048576/531441

2/1



17 tone modes of Persian scale:
256/243
27/25
9/8
32/27
243/200
81/64
4/3
25/18
36/25
3/2
128/81
81/50
27/16
16/9
729/400
243/128

2/1


http://www.plainsound.org/pdfs/srutis.pdf

http://www.ias.ac.in/article/fulltext/reso/020/06/0515-0531

22 tone modes of Indian shruti scale:
256/243
16/15
10/9
9/8
32/27
6/5
5/4
81/64
4/3
27/20
45/32
729/512 (or 64/45)
3/2
128/81
8/5
5/3
27/16
16/9
9/5
15/8
243/128

2/1

http://www.anaphoria.com/indianintro.html


https://github.com/brummer10/gxtuner

#endif
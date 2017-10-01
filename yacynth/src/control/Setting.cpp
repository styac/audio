/*
 * Copyright (C) 2016 Istvan Simon -- stevens37 at gmail dot com
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
 * File:   Setting.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on February 6, 2016, 10:42 PM
 */

#include "Setting.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace yacynth {


static int is_regular_file( const char *path )
{
    struct stat path_stat;
    lstat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

Setting::Setting()
:   homeDir(".")
,   ok(false)
{
    const char *hdp = getenv("HOME");
    if( hdp != nullptr) {
        homeDir = hdp;
    }
    // check if necessary files exist and type is ok
    // if not try to fix
}


} // end namespace yacynth



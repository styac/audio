#pragma once

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
 * File:   Setting.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on February 6, 2016, 10:42 PM
 */

#include    "yacynth_globals.h"

#include    <array>
#include    <string>


namespace yacynth {

class Setting {
public:
    static constexpr const char * const configDirName = "/.yacconfig/";
    static constexpr const char * const authKeyName = "yyauth.key";

    Setting();

    std::string getHomeDir() const
    { return homeDir; };

    std::string getConfDir() const
    { return homeDir + configDirName; };

    std::string getAuthKey() const
    { return homeDir + configDirName + authKeyName; };

private:
    std::string homeDir;
};


} // end namespace yacynth


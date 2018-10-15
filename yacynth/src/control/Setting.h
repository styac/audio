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

#include "yacynth_globals.h"
#include "protocol.h"

#include <array>
#include <map>
#include <string>

namespace yacynth {
//
// authentication
// read and uiServer.setAuthSeed( );
//

// parameters:
// -option1 value1a value1b -option2 value2a .. value2n -option3 value3 -option4 value4 ....
//  -confd /home/x/y -port 7777 -midi jack


// $base/.yacconfig/<profile>/conf
// $base/.yacconfig/<profile>/.yyauth.key
//

class Setting {
public:
    enum class OptionTag {
        CONFD,
        PROFILE,
        MIDI,
        CONN,
        IPPORT,
        LOCPORT
    };

    typedef struct { OptionTag tag; const char * usage; } OptionData;
    typedef std::map<std::string, OptionData> OptionType;

    // options  with selection
    enum class OptMidi {
        JACK,
        ALSARAW,
    };

    Setting();

    bool initialize( int argc, char** argv );

    const std::string& getHomeDir() const
    {
        return homeDir;
    }

    std::string getConfDir() const
    {
        return homeDir + "/" + yaxp::configDirName;
    }

    std::string getAuthKeyFile() const
    {
        return getConfDir() + "/" + profile + "/" + yaxp::authKeyName;
    }

    bool isOk() const
    {
        return ok;
    }

    uint16_t getControlPortRemote() const
    {
        return controlPortRemote;
    }

    const char * getControlPortLocal() const
    {
        return controlPortLocal.data();
    }

    yaxp::CONN_MODE getConnMode() const
    {
        return connMode;
    }

private:
    void usage( const char* option, const char* info ) const;

    // TODO extend to accept multiple values
    bool setOption( Setting::OptionTag tag, const std::string& optionValue );

    bool readConfigFile();
    bool readOptions( int argc, char** argv );
    bool createConfigFile( const std::string& config );
    bool createAuthFile( const std::string& config );

    std::string     homeDir;
    std::string     profile;
    std::string     configDir;
    std::string     controlPortLocal;
    OptMidi         optMidi;
    OptionType      options;
    uint16_t        controlPortRemote;
    yaxp::CONN_MODE connMode;
    bool            ok;
};


} // end namespace yacynth


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
// check if key exists if not create one with random data

Setting::Setting()
:   homeDir(".")
,   optMidi(OptMidi::JACK)
,   controlPort(defaultCOntrolPort)
,   ok(false)
{
    static const char * usageCONFD = "configuration directory: path";
    static const char * usageMIDI = "midi interface: jack (default) or alsaraw";

    options.emplace( "confd", OptionData{OptionTag::CONFD,usageCONFD} );
    options.emplace( "midi", OptionData{OptionTag::MIDI,usageMIDI} );

    const char *hdp = getenv("HOME");
    if( hdp != nullptr) {
        homeDir = hdp;
    }
}

void Setting::usage( const char* option, const char* info ) const
{

}

// TODO extend to accept multiple values
bool Setting::setOption( Setting::OptionTag tag, const std::string& optionValue )
{
    switch( tag ) {
    case OptionTag::CONFD:
        return true;

    case OptionTag::MIDI:
        if( optionValue == "jack" ) {
            return true;
        }
        if( optionValue == "alsaraw" ) {
            optMidi = OptMidi::ALSARAW;
            return true;
        }
        usage( optionValue.data(), "illegal option value for -midi");
        return false;

    case OptionTag::PORT:
        return true;
        
    }
    usage( "may not happen", "internal error" );
    return false;
}

bool Setting::initialize( int argc, char** argv )
{
    for( auto oind=1u; oind<argc; ++oind ) {
        const char * par = argv[oind];
        if( argc < (oind+1) ) {
            usage( par, "missing value" );
            return false;
        }

        // check duplicates -- only 1 is accepted

        if( *par == '-' ) {
            auto found =  options.find(par);
            if( found == options.end() ) {
                usage( par, "illegal option" );
                return false;
            }
            if( ! setOption( found->second.tag, argv[oind+1] ) ) {
                usage( par, "illegal option value" );
                return false;
            }
            ++oind; // jump over option value
        } else {
            usage( par, "not an option: missing '-'" );
            return false;
        }
    }
    return true;
}

} // end namespace yacynth



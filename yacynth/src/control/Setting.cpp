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
#include "yacynth_config.h"

#include "Setting.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/random.h>
#include <iostream>
#include <fstream>

namespace yacynth {

//namespace {
//constexpr auto LogCategoryMask              = LOGCAT_net;
//constexpr auto LogCategoryMaskAlways        = LogCategoryMask | nanolog::LogControl::log_always;
//constexpr const char * const LogCategory    = "NETS";
//}

static bool isRegularFile( const char *path )
{
    struct stat path_stat;
    if( 0 != lstat(path, &path_stat) ) {
        return false;
    }
    return S_ISREG(path_stat.st_mode) != 0;
}

static bool isDirectory( const char *path )
{
    struct stat path_stat;
    if( 0 != lstat(path, &path_stat) ) {
        return false;
    }
    return S_ISDIR(path_stat.st_mode) != 0;
}

Setting::Setting()
:   homeDir(".")
,   profile(yaxp::profileDirName)
,   controlPortLocal(yaxp::localDefaultPort)
,   optMidi(OptMidi::JACK)
,   options()
,   controlPortRemote(yaxp::remoteDefaultPort)
,   connMode(yaxp::CONN_MODE::CONNECTION_REMOTE)
,   ok(false)
{
    static const char * usageCONFD      = "configuration directory: path";
    static const char * usageMIDI       = "midi interface: jack (default) or alsaraw";
    static const char * usageCONN       = "connection: remote (default) or local";
    static const char * usageIPPORT     = "remote(ip) port (int): 5000..50000";
    static const char * usageLOCALPORT  = "local port : valid filename";
    static const char * usagePROFILE    = "profile name: valid file name";

    options.emplace( "confd",   OptionData{OptionTag::CONFD,usageCONFD} );
    options.emplace( "profile", OptionData{OptionTag::PROFILE,usagePROFILE} );
    options.emplace( "midi",    OptionData{OptionTag::MIDI,usageMIDI} );
    options.emplace( "conn",    OptionData{OptionTag::CONN,usageCONN} );
    options.emplace( "ipport",  OptionData{OptionTag::IPPORT,usageIPPORT} );
    options.emplace( "locport", OptionData{OptionTag::LOCPORT,usageLOCALPORT} );

    const char *hdp = getenv("HOME");
    if( hdp != nullptr) {
        homeDir = hdp;
    }
}

void Setting::usage( const char* option, const char* info ) const
{
    std::cout << option << info << std::endl;
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
        usage( optionValue.data(), " : illegal option value for -midi");
        return false;

    case OptionTag::CONN:
        if( optionValue == "remote" ) {
            return true;
        }
        if( optionValue == "local" ) {
            connMode = yaxp::CONN_MODE::CONNECTION_LOCAL;
            return true;
        }
        usage( optionValue.data(), " : illegal option value for -conn");
        return false;

    case OptionTag::IPPORT:
        return true;

    case OptionTag::LOCPORT:
        return true;

    default:
        return false;
    }
    usage( "may not happen", " : internal error" );
    return false;
}

bool Setting::initialize( int argc, char** argv )
{
    if( ! readOptions( argc, argv )) {
        return false;
    }
    if( ! readConfigFile()) {
        return false;
    }
    // reread to override file options
    if( ! readOptions( argc, argv )) {
        return false;
    }
    return  true;
}

bool Setting::readConfigFile()
{
    mode_t dirmode = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
    configDir = homeDir + "/" + yaxp::configDirName;
    if( ! isDirectory( configDir.data()) ) {
        // no directory -- try to make if
        if( mkdir(configDir.data(), dirmode ) != 0 ) {
            std::cerr << "can't create config dir:" << configDir << " errno: " << errno << std::endl;
            return false;
        }
    }

    configDir += "/" + profile;
    if( ! isDirectory( configDir.data()) ) {
        if( mkdir(configDir.data(), dirmode ) != 0 ) {
            std::cerr << "can't create config dir:" << configDir << " errno: " << errno << std::endl;
            return false;
        }
    }

    std::string config( configDir + "/" + yaxp::confName );
    if( ! isRegularFile(config.data()) ) {
        return createConfigFile(configDir);
    }

    // read content

    return  true;
}

bool Setting::createAuthFile( const std::string& configDir )
{
    char seedAuth[ yaxp::seedLength ];
    std::string config( getAuthKeyFile() );
    std::ofstream conf ( config );
    if ( ! conf.is_open() ) {
        std::cerr << "can't create auth file:" << config << " errno: " << errno << std::endl;
        return false;
    }
    {
        mode_t authmode = S_IRUSR | S_IWUSR;
        chmod( config.data(), authmode );
    }

    std::ifstream random ( "/dev/urandom" );
    if( random.fail() ) {
        random.close();
        conf.close();
        return false;
    }

    random.read( seedAuth, yaxp::seedLength );
    if( random.fail() ) {
        random.close();
        conf.close();
        return false;
    }
    random.close();
    conf.write( seedAuth, yaxp::seedLength);
    {
        mode_t authmode = S_IRUSR;
        chmod( config.data(), authmode );
    }
    if ( conf.fail() ) {
        std::cerr << "can't create auth file:" << config << " errno: " << errno << std::endl;
        return false;
    }
    conf.close();
    return  true;
}

bool Setting::createConfigFile( const std::string& configDir )
{
    std::string config( configDir + "/" + yaxp::confName );
    std::ofstream conf ( config );
    if ( ! conf.is_open() ) {
        std::cerr << "can't create config file:" << config << " errno: " << errno << std::endl;
        return false;
    }
    conf << "# config file" << std::endl;
    conf.close();

    return  createAuthFile( configDir );
}

bool Setting::readOptions( int argc, char** argv )
{
    for( auto oind=1; oind<argc; ++oind ) {
        const char * par = argv[oind];
        if( argc <= (oind+1) ) {
            usage( par, " : missing value" );
            return false;
        }

        // check duplicates -- only 1 is accepted
        // -conf must be first if exists
        // -profile must be next after -conf if exists

        if( *par == '-' ) {
            auto found =  options.find(par+1);
            if( found == options.end() ) {
                usage( par, " : illegal option" );
                return false;
            }
            if( ! setOption( found->second.tag, argv[oind+1] ) ) {
                usage( par, " : illegal option value" );
                return false;
            }
            ++oind; // jump over option value
        } else {
            usage( par, " : not an option: missing '-'" );
            return false;
        }
    }
    return true;
}

} // end namespace yacynth



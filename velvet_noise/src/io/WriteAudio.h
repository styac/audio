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

#include <sndfile.h>
#include <cstdint>

class SoundFile
{
public:
    SoundFile( uint32_t samplerate = 48000, int format = SF_FORMAT_WAV | SF_FORMAT_PCM_24, uint8_t channel = 2 );

    ~SoundFile()
    {
        sf_close(file);
    }

    bool open( const char *path, int mode )
    {
        file = sf_open( path, mode, &info );
        return file > 0;
    }

    bool close()
    {
        return 0 == sf_close(file);
    }

    bool read( short * p, sf_count_t count )
    {
        rcnt = sf_readf_short(file, p, count);
        return rcnt == count;
    }
    bool read( int * p, sf_count_t count )
    {
        rcnt = sf_readf_int(file, p, count);
        return rcnt == count;
    }

    bool read( float * p, sf_count_t count )
    {
        rcnt = sf_readf_float(file, p, count);
        return rcnt == count;
    }

    bool read( double * p, sf_count_t count )
    {
        rcnt = sf_readf_double(file, p, count);
        return rcnt == count;
    }

    bool write( short const * p, sf_count_t count )
    {
        rcnt = sf_writef_short(file, p, count);
        return rcnt == count;
    }

    bool write( int const * p, sf_count_t count )
    {
        rcnt = sf_writef_int(file, p, count);
        return rcnt == count;
    }

    bool write( float const * p, sf_count_t count )
    {
        rcnt = sf_writef_float(file, p, count);
        return rcnt == count;
    }

    bool write( double const * p, sf_count_t count )
    {
        rcnt = sf_writef_double(file, p, count);
        return rcnt == count;
    }

    int err() const
    {
        return sf_error(file);
    }

    uint32_t sampleRate() const
    {
        return info.samplerate;
    }

private:
    SNDFILE *file;
    sf_count_t rcnt;
    SF_INFO info;
};

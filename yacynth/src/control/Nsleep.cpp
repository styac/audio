/*
 * Copyright (C) 2019 Istvan Simon
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

#include "Nsleep.h"

#include <time.h>
#include <errno.h>

// --------------------------------------------------------------------

uint64_t nsleep( uint64_t nsec, uint64_t sec )
{
    timespec req;
    timespec rem;
    req.tv_nsec = nsec;
    req.tv_sec = sec;
    auto res = nanosleep(&req,&rem);
    if( res == -1 && errno == EINTR ) {
        return rem.tv_nsec + rem.tv_sec * 1000 * 1000 * 1000L;
    }
    return 0;
}




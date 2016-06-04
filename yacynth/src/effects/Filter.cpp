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
 * File:   Filter.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on February 18, 2016, 10:53 PM
 */

#include    "Filter.h"
#include    "Ebuffer.h"
#include    "../utils/Limiters.h"

#include    <iostream>
#include    <iomanip>
#include    <csignal>
#include    <sys/time.h>
#include    <ctime>


namespace yacynth {
// --------------------------------------------------------------------
ControlledFilter::ControlledFilter( )
:   ctrlQ(ControllerMatrix::C_INT_FILTER_Q)
,   ctrlF1(ControllerMatrix::C_INT_FILTER_FREQUENCY1)
,   ctrlF2(ControllerMatrix::C_INT_FILTER_FREQUENCY2)

{ reset(); };
// --------------------------------------------------------------------


void ControlledFilter::process( const EIObuffer& __restrict inp, EIObuffer& __restrict out )
{
    timeval tv; // profiling
    // profiling
    const int64_t  maxC = 1000;
    if( --count < 0 ) {
        std::cout << std::dec << " ------- filter " << timer << std::endl;
        count = maxC;
        timer = 0;
    }
    const float gain = inp.channelGain;
//    const float gain = 1.0f;
    // next> Amplitude modulation
    for( auto i=0; i<filterControl.activeFilterCount; ++i ) {
          filterControl.channelParam[i].gain = gain;
    }

//    const float gain = 1.0f;
    filter.setGain( 0, gain );
    filter.setGain( 1, gain );
    filter.setGain( 2, gain );
    filter.setGain( 3, gain );


    filter.setGain( 4, gain );
    filter.setGain( 5, gain );
    filter.setGain( 6, gain );
    filter.setGain( 7, gain );

    out.channelGain = 1.0f;

#if 1
    if( ctrlF1.update() ) {
        const int64_t cc = ctrlF1.get();
        const int32_t f = filter.fMin + ((( filter.fMax-filter.fMin ) * cc  )>>24) ;
        std::cout << "cc " << cc << " f "  << f  << std::endl;

        filter.setFreq( 0, f  );
        filter.setFreq( 1, f - 0x1000000 );
        filter.setFreq( 2, f - 0x2000000 );
        filter.setFreq( 3, f - 0x3000000 );
        filter.setFreq( 4, f - 0x0800000 );
        filter.setFreq( 5, f - 0x1800000 );
        filter.setFreq( 6, f - 0x2800000 );
        filter.setFreq( 7, f - 0x3800000 );

    }
#endif
    if( ctrlQ.update() ) {
        const float q = ctrlQ.getNorm(2);  // max value == 4
        std::cout << "q " << q << std::endl;

        for( auto i=0; i<filterControl.activeFilterCount; ++i ) {
              filterControl.channelParam[i].q = q;
        }

        filter.setQ( 0, q );
        filter.setQ( 1, q );
        filter.setQ( 2, q );
        filter.setQ( 3, q );

        filter.setQ( 4, q );
        filter.setQ( 5, q );
        filter.setQ( 6, q );
        filter.setQ( 7, q );

    }


    // profiling
    gettimeofday(&tv,NULL);
    const uint64_t begint   = tv.tv_sec*1000000ULL + tv.tv_usec;

    for( auto i = 0u; i < filterSampleCount; ++i ) {
//        const float in = noise.getWhitef();
//        filter.set<4>( in );
        filter.set<4>( inp.channel[out.chA][i] );
//        out.channel[out.chA][i] = out.channel[out.chB][i] = filter.getLBP<filter.SD,filter.SC,0,2>();
// update gain for AM
        out.channel[out.chA][i] = out.channel[out.chB][i] = filter.getLBP<filter.SD,filter.SC,0,4>();
    }

    // profiling
    gettimeofday(&tv,NULL);
    timer += tv.tv_sec*1000000ULL + tv.tv_usec - begint;
} // end SVFControlled::filter

} // end namespace yacynth





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
 * File:   SynthFrontend.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on February 17, 2016, 7:44 AM
 */

#include "Statistics.h"
#include "oscillator/Tables.h"
#include "oscillator/Oscillator.h"
#include "oscillator/OscillatorArray.h"
#include "oscillator/ToneShaper.h"
#include "yaio/YaIoJack.h"
#include "yaio/IOthread.h"
#include "control/Controllers.h"
#include "oscillator/ToneShaperMatrix.h"


/*
     std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    std::cout << "f(42) = " << fibonacci(42) << '\n';
    end = std::chrono::system_clock::now();

    std::chrono::duration<double> elapsed_seconds = end-start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);


 */

namespace yacynth {

class SynthFrontend {
public:
    SynthFrontend() = delete;
    SynthFrontend(
        ControlQueueVector&      queueinP,
        OscillatorOutVector&    outVectorP,
        OscillatorArray       * oscArrayP
        );
    bool            initialize( void );
    bool            run( void );
    void            stop()
    {
        runFe = false;
    }
    static  void    exec( void * data );

private:
    bool        cycleBegin( void );
    bool        cycleEnd( void );
    bool        evalMEssage( void );
    bool        generate( void );

    ControlQueueVector&     queuein;
    OscillatorOutVector&    outVector;
    OscillatorArray       * oscArray;
    uint64_t                waitLimit;          // if the cycle was too fast wait a bit to ceck
    Statistics              statistics;
    uint32_t                cycleNoise;
    bool                    runFe;

};

} // end namespace yacynth


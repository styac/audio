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

#include <iostream>
#include "io/WriteAudio.h"
#include "dsp/GenerateNoise.h"
#include <iostream>
#include <cmath>

using namespace std;

bool generate_velvet_white( SoundFile& sf, size_t second )
{
    GaloisShifter gs;
    NoiseFrameType noiseFrame(gs);
    FrameInterleaveType frameInterleave;

    // warm up the DC filter
    int64_t count = 1000;
    while(--count) {
        noiseFrame.fillVelvetTriangle2CH<pulseCountExp>();
        noiseFrame.postFilterDCcut();
    }

    count = 1 + float((second+1) * sf.sampleRate()) / NoiseFrameType::sectionSize;
    while(--count) {
        noiseFrame.fillVelvetTriangle2CH<pulseCountExp>();
        noiseFrame.postFilterDCcut();
        frameInterleave.set(noiseFrame,256.0f);
        bool res = sf.write( frameInterleave.channel, frameInterleave.interleavedSize/FrameInterleaveType::channelCount );
        if( !res ) {
            return false;
        }
//        std::cout
//                << " dc: " << noiseFrame.s[0]
//                << " " << noiseFrame.s[1]
//                << std::endl;
    }
    return true;
}

//
// -o filename
// -l length time in sec
// -t type - white, pink, red
// -f sampling frquency 48000 96000 ...
//

int main( int argc, char *argv[] )
{
    int samplingFrequency( 4*44100 );
    int fileType( SF_FORMAT_FLAC | SF_FORMAT_PCM_24 );
    size_t lengthSec(60);
    std::string fileext( ".flac" );
    std::string filename( "velvet_white_noise" );

    int tmp;
    for( auto i=1u; i < argc; i += 2 ) {
        char * ap = argv[i];
        if( ap[ 0 ] != '-') {
            std::cerr << "\nillegal option " << ap
                << std::endl;
            exit(-1);
        }
        if( (i+1) > (argc-1) ) {
            std::cerr << "\nmissing value for: " << ap
                << std::endl;
            exit(-1);
        }

        switch( ap[ 1 ] ) {
        case 'o' :
            filename = argv[i+1];
            break;

        case 'l' :
            tmp = std::stoi(argv[i+1]);
            if( (tmp<1) || (tmp>( 3600*10))) { // 10hours
                std::cerr << "\nillegal length " << tmp << " use [sec]: " << lengthSec
                    << std::endl;
            } else {
                lengthSec = tmp;
            }
            break;

        case 't' : // ignored -- only white
            break;

        case 'f' :
            tmp = std::stoi(argv[i+1]);
            if( (tmp<44100) || (tmp>(16*48000))) {
                std::cerr << "\nillegal sampling frequency " << tmp << " use [Hz]: " << samplingFrequency
                    << std::endl;
            } else {
                samplingFrequency = tmp;
            }
            break;

        default:
            std::cerr << "\nillegal parameter " << ap
                << std::endl;
            exit(-1);
        }
    }


    SoundFile soundFile(samplingFrequency, fileType);
    filename += fileext;

    if( soundFile.open(filename.data(), SFM_READ) ) {
        std::cerr << "\n" << filename << " file exist"
            << std::endl;
        exit(-1);
    }
    if( !soundFile.open(filename.data(), SFM_WRITE) )
        exit(-1);

    generate_velvet_white(soundFile, lengthSec);
    return 0;
}

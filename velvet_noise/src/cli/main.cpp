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

    count = 1 + (second * sf.sampleRate()) / NoiseFrameType::sectionSize;
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

// TBD
// -o filename
// -l length time in sec
// -t type - white, pink, red
// -f sampling frquency 48000 96000 ...
//

int main( int argc, char *argv[] )
{
    int samplingFrequency( 192000 );
    int fileType( SF_FORMAT_FLAC | SF_FORMAT_PCM_24 );
    size_t lengthSec(60);
    std::string fileext( ".flac" );
    std::string filename( "velvet_noise" );


    SoundFile soundFile(4*44100, fileType);

    if( !soundFile.open("velvet_noise_generated_176k-test.flac", SFM_WRITE) )
        exit(-1);

    generate_velvet_white(soundFile, 36);
    return 0;
}

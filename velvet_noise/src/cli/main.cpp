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

//
// -o filename
// -l length time in sec
// -t type - white, pink, red
// -f sampling base frquency C(ommercial) (44100), P(rofessional) (48000)
// -u upsamplerate 1,2,..16
// -g generator : velvet, galois
// -n white, red, deepred
//

enum NoiseType {
    NT_white,
    NT_red,
    NT_deepred,
    NT_max
};

const char * const noiseTypes[] = {
    "white",
    "red",
    "deepred"
};

void help()
{
    std::cout <<
        "usage: velvet_noise [ -option value ]\n"
        "  options\n"
        " -o output_filename - default: velvet_white_noise [format is flac] - NO OVERWRITE!\n"
        " -l length(sec) - default: 10\n"
        " -f sampling frequency C(ommercial)=44100 P(rofessional)=48000 - default: 44100\n"
        "   noises: red,deepred differs for 48000 44100\n"
        " -u oversampling rate - default: 4 (-> 4*44100)\n"
        " -n white, red, deepred\n"
        << std::endl;
}

bool generate_velvet_white( SoundFile& sf, size_t second, size_t upsampleRate )
{
    GaloisShifter gs;
    NoiseFrameType noiseFrame(gs);
    FrameInterleaveType frameInterleave;
    noiseFrame.clear();
    int64_t count = 1 + float((second+1) * sf.sampleRate()) / NoiseFrameType::sectionSize;
    std::cout << "velvet white" << std::endl;

    while(--count) {
        noiseFrame.fillVelvetTriangle2CH<pulseCountExp>();
        noiseFrame.postFilterDCcut(upsampleRate, 12);
        frameInterleave.set(noiseFrame,256.0f);
        bool res = sf.write( frameInterleave.channel, frameInterleave.interleavedSize/FrameInterleaveType::channelCount );
        if( !res ) {
            help();
            return false;
        }
    }
    return true;
}

bool generate_velvet_red( SoundFile& sf, size_t second, size_t upsampleRate )
{
    GaloisShifter gs;
    NoiseFrameType noiseFrame(gs);
    FrameInterleaveType frameInterleave;
    noiseFrame.clear();
    int64_t count = 1 + float((second+1) * sf.sampleRate()) / NoiseFrameType::sectionSize;
    std::cout << "velvet red" << std::endl;

    while(--count) {
        noiseFrame.fillVelvetTriangle2CH<pulseCountExp>();
        noiseFrame.postFilterDCcut(upsampleRate, 12);
        noiseFrame.postFilterLowPass1(upsampleRate, 10);
        frameInterleave.set(noiseFrame,256.0f);
        bool res = sf.write( frameInterleave.channel, frameInterleave.interleavedSize/FrameInterleaveType::channelCount );
        if( !res ) {
            help();
            return false;
        }
    }
    return true;
}

bool generate_velvet_deepred( SoundFile& sf, size_t second, size_t upsampleRate )
{
    GaloisShifter gs;
    NoiseFrameType noiseFrame(gs);
    FrameInterleaveType frameInterleave;
    noiseFrame.clear();
    int64_t count = 1 + float((second+1) * sf.sampleRate()) / NoiseFrameType::sectionSize;
    std::cout << "velvet deep red" << std::endl;

    while(--count) {
        noiseFrame.fillVelvetTriangle2CH<pulseCountExp>();
        noiseFrame.postFilterDCcut(upsampleRate, 12);
        noiseFrame.postFilterLowPass2(upsampleRate, 8 );
        frameInterleave.set(noiseFrame,256.0f);
        bool res = sf.write( frameInterleave.channel, frameInterleave.interleavedSize/FrameInterleaveType::channelCount );
        if( !res ) {
            help();
            return false;
        }
    }
    return true;
}

bool generate_galois_white( SoundFile& sf, size_t second, size_t upsampleRate )
{
    GaloisShifter gs;
    NoiseFrameType noiseFrame(gs);
    FrameInterleaveType frameInterleave;
    noiseFrame.clear();
    int64_t count = 1 + float((second+1) * sf.sampleRate()) / NoiseFrameType::sectionSize;
    while(--count) {
        noiseFrame.fillWhiteStereo();
//        noiseFrame.postFilterDCcut();
        frameInterleave.set(noiseFrame,32.0f);
        bool res = sf.write( frameInterleave.channel, frameInterleave.interleavedSize/FrameInterleaveType::channelCount );
        if( !res ) {
            return false;
        }
    }
    return true;
}


int main( int argc, char *argv[] )
{
    NoiseType nt = NT_white;
    int samplingFrequency( 44100 );
    int fileType( SF_FORMAT_FLAC | SF_FORMAT_PCM_24 );
    size_t lengthSec(10);
    size_t upsampleRate(4);
    std::string fileext( ".flac" );
    std::string filename( "velvet_white_noise" );

    int tmp;
    for( auto i=1u; i < argc; i += 2 ) {
        char * ap = argv[i];
        if( ap[ 0 ] != '-') {
            std::cerr << "\nillegal option " << ap
                << std::endl;
            help();
            exit(-1);
        }
        if( (i+1) > (argc-1) ) {
            std::cerr << "\nmissing value for: " << ap
                << std::endl;
            help();
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
            switch(*argv[i+1]) {
            case 'c':
            case 'C':
                samplingFrequency = 44100;
                break;
            case 'p':
            case 'P':
                samplingFrequency = 48000;
                break;
            default:
                std::cerr << "\nillegal sampling frequency use P or C"
                    << std::endl;
                help();
                exit(-1);
            }
            break;

        case 'u' :
            tmp = std::stoi(argv[i+1]);
            if( (tmp<1) || (tmp>16)) {
                std::cerr << "\nillegal upsample rate " << tmp << " use: " << upsampleRate
                    << std::endl;
            } else {
                upsampleRate = tmp;
            }
            break;

        case 'n':
            for( int t = int(NT_white); t < int(NT_max); ++t ) {
                if( 0==strncmp( noiseTypes[t], argv[i+1], std::min( strlen(noiseTypes[t]), strlen(argv[i+1]) )) ) {
                    nt = NoiseType(t);
                }
            }
            break;

        default:
            std::cerr << "\nillegal parameter " << ap
                << std::endl;
            help();
            exit(-1);
        }
    }

    SoundFile soundFile(samplingFrequency*upsampleRate, fileType);
    filename += fileext;

    if( soundFile.open(filename.data(), SFM_READ) ) {
        std::cerr << "\n" << filename << " file exist"
            << std::endl;
        help();
        exit(-1);
    }
    if( !soundFile.open(filename.data(), SFM_WRITE) ) {
        help();
        exit(-1);
    }

    switch (nt) {
    case NT_white:
        generate_velvet_white(soundFile, lengthSec, upsampleRate);
        break;
    case NT_red:
        generate_velvet_red(soundFile, lengthSec, upsampleRate);
        break;
    case NT_deepred:
        generate_velvet_deepred(soundFile, lengthSec, upsampleRate);
        break;
    default:
        help();
        exit(-1);
    }
    return 0;
}

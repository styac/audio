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
#include "dsp/NoiseFrame.h"
#include <iostream>
#include <cmath>


using namespace std;

//
// -o filename
// -l length time in sec
// -f sampling base frquency C(ommercial) (44100), P(rofessional) (48000)
// -u upsamplerate 1,2,3,4
// -g generator : velvet, galois
// -n white, red, deepred, rwhite
// -s framesize : 0 = big 1 = small
//

enum NoiseType {
    NT_white,
    NT_red,
    NT_deepred,
    NT_rwhite,
    NT_dwhite,
    NT_gwhite,

    NT_swhite,
    NT_sred,
    NT_sdeepred,
    NT_srwhite,
    NT_sdwhite,

    NT_sgwhite,

    NT_max
};

const char * const noiseTypes[] = {
    "white",
    "red",
    "deepred",
    "rwhite",
    "dwhite",
    "gwhite",
    "swhite",
    "sred",
    "sdeepred",
    "srwhite",
    "sdwhite",
    "sgwhite",
};

void help()
{
    std::cout <<
        "\n\nusage: velvet_noise [ -option value ]\n"
        "  options\n"
        " -o output_filename - default: velvet_white_noise [format is flac] - NO OVERWRITE!\n"
        " -l length(sec) - default: 10\n"
        " -f sampling frequency C(ommercial)=44100 P(rofessional)=48000 - default: 44100\n"
        "   noises: red,deepred differs for 48000 44100\n"
        " -u oversampling rate - default: 4 (-> 4*44100)\n"
        " -n white, red, deepred, rwhite\n"
        " r... noises (raw) are not band limited"
        << std::endl;
}

// upsampleRate pulseCountExp
//  1               7
//  2               6
//  3               5
//  4               4

namespace BigFrame {

constexpr std::size_t frameSizeExp = 10;    // 2<<10
typedef FrameInt<frameSizeExp,2> FrameType;
typedef NoiseFrame<FrameType> NoiseFrameType;
typedef FrameInterleave<int,frameSizeExp,2> FrameInterleaveType;


bool generate_velvet_white_blt( SoundFile& sf, size_t second, size_t upsampleRate )
{
    // check the minimum for upsample 1,2,4

    std::size_t pulseCountExp = 7-upsampleRate;
    if( pulseCountExp < 4 ) {
        pulseCountExp = 4;
    }
    GaloisShifter gs;
    NoiseFrameType noiseFrame(gs);
    FrameInterleaveType frameInterleave;
    noiseFrame.clear();
    int64_t count = 1 + float((second+1) * sf.sampleRate()) / NoiseFrameType::sectionSize;
    std::cout << "velvet white BLT" << std::endl;

    while(--count) {
        noiseFrame.fillVelvetTriangle2CH(pulseCountExp);
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

bool generate_velvet_white_double_blt( SoundFile& sf, size_t second, size_t upsampleRate )
{
    std::size_t pulseCountExp = 6-upsampleRate;
    if( pulseCountExp < 3 ) {
        pulseCountExp = 3;
    }
    GaloisShifter gs;
    NoiseFrameType noiseFrame(gs);
    FrameInterleaveType frameInterleave;
    noiseFrame.clear();
    int64_t count = 1 + float((second+1) * sf.sampleRate()) / NoiseFrameType::sectionSize;
    std::cout << "velvet white double BLT" << std::endl;

    while(--count) {
//        noiseFrame.fillVelvetDoubleTriangle2CH_1CH(pulseCountExp);
        noiseFrame.fillVelvetDoubleTriangle2CH(pulseCountExp);

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

bool generate_velvet_red_blt( SoundFile& sf, size_t second, size_t upsampleRate )
{
    const std::size_t pulseCountExp = 8-upsampleRate;
    GaloisShifter gs;
    NoiseFrameType noiseFrame(gs);
    FrameInterleaveType frameInterleave;
    noiseFrame.clear();
    int64_t count = 1 + float((second+1) * sf.sampleRate()) / NoiseFrameType::sectionSize;
    std::cout << "velvet red BLT" << std::endl;

    while(--count) {
        noiseFrame.fillVelvetTriangle2CH(pulseCountExp);
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

bool generate_velvet_deepred_blt( SoundFile& sf, size_t second, size_t upsampleRate )
{
    const std::size_t pulseCountExp = 8-upsampleRate;
    GaloisShifter gs;
    NoiseFrameType noiseFrame(gs);
    FrameInterleaveType frameInterleave;
    noiseFrame.clear();
    int64_t count = 1 + float((second+1) * sf.sampleRate()) / NoiseFrameType::sectionSize;
    std::cout << "velvet deep red BLT" << std::endl;

    while(--count) {
        noiseFrame.fillVelvetTriangle2CH(pulseCountExp);
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


bool generate_velvet_white( SoundFile& sf, size_t second, size_t upsampleRate )
{
    const std::size_t pulseCountExp = 7-upsampleRate;
    GaloisShifter gs;
    NoiseFrameType noiseFrame(gs);
    FrameInterleaveType frameInterleave;
    noiseFrame.clear();
    int64_t count = 1 + float((second+1) * sf.sampleRate()) / NoiseFrameType::sectionSize;
    std::cout << "velvet white raw" << std::endl;

    while(--count) {
        noiseFrame.fillVelvet2CH(pulseCountExp);
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


} // end namespace BigFrame

namespace SmallFrame {

constexpr std::size_t frameSizeExp = 6;    // 64
typedef FrameInt<frameSizeExp,2> FrameType;
typedef NoiseFrame<FrameType> NoiseFrameType;
typedef FrameInterleave<int,frameSizeExp,2> FrameInterleaveType;

bool generate_velvet_white_blt( SoundFile& sf, size_t second, size_t upsampleRate )
{
    // check the minimum for upsample 1,2,4

    // upscale = 4
    // pulseCountExp = 2 (4) of 64 is ok

      std::size_t pulseCountExp = 2;

//    std::size_t pulseCountExp = 4-upsampleRate;
//    if( pulseCountExp < 2 ) {
//        pulseCountExp = 2;
//    }
    GaloisShifter gs;
    NoiseFrameType noiseFrame(gs);
    FrameInterleaveType frameInterleave;
    noiseFrame.clear();
    int64_t count = 1 + float((second+1) * sf.sampleRate()) / NoiseFrameType::sectionSize;
    std::cout << "velvet white BLT SmallFrame" << std::endl;

    while(--count) {
        noiseFrame.fillVelvetTriangle2CH(pulseCountExp);
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

bool generate_velvet_white_double_blt( SoundFile& sf, size_t second, size_t upsampleRate )
{

    // upscale = 2
    // pulseCountExp = 2 (4) of 64 is ok

    std::size_t pulseCountExp = 2;

//    std::size_t pulseCountExp = 3-upsampleRate;
//    if( pulseCountExp < 2 ) {
//        pulseCountExp = 2;
//    }
    GaloisShifter gs;
    NoiseFrameType noiseFrame(gs);
    FrameInterleaveType frameInterleave;
    noiseFrame.clear();
    int64_t count = 1 + float((second+1) * sf.sampleRate()) / NoiseFrameType::sectionSize;
    std::cout << "velvet white double BLT SmallFrame" << std::endl;

    while(--count) {
//        noiseFrame.fillVelvetDoubleTriangle2CH_1CH(pulseCountExp);
        noiseFrame.fillVelvetDoubleTriangle2CH(pulseCountExp);

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

bool generate_velvet_red_blt( SoundFile& sf, size_t second, size_t upsampleRate )
{
    std::size_t pulseCountExp = 2;
    GaloisShifter gs;
    NoiseFrameType noiseFrame(gs);
    FrameInterleaveType frameInterleave;
    noiseFrame.clear();
    int64_t count = 1 + float((second+1) * sf.sampleRate()) / NoiseFrameType::sectionSize;
    std::cout << "velvet red BLT SmallFrame" << std::endl;

    while(--count) {
        noiseFrame.fillVelvetTriangle2CH(pulseCountExp);
        noiseFrame.postFilterLowPass1(upsampleRate, 10);
        noiseFrame.postFilterDCcut(upsampleRate, 12);  // filter out the prev rounding errors also
        frameInterleave.set(noiseFrame,256.0f);
        bool res = sf.write( frameInterleave.channel, frameInterleave.interleavedSize/FrameInterleaveType::channelCount );
        if( !res ) {
            help();
            return false;
        }
    }
    return true;
}

bool generate_velvet_deepred_blt( SoundFile& sf, size_t second, size_t upsampleRate )
{
    std::size_t pulseCountExp = 2;
    GaloisShifter gs;
    NoiseFrameType noiseFrame(gs);
    FrameInterleaveType frameInterleave;
    noiseFrame.clear();
    int64_t count = 1 + float((second+1) * sf.sampleRate()) / NoiseFrameType::sectionSize;
    std::cout << "velvet deep red BLT SmallFrame" << std::endl;

    while(--count) {
        noiseFrame.fillVelvetTriangle2CH(pulseCountExp);
        noiseFrame.postFilterLowPass2(upsampleRate, 8 );
        noiseFrame.postFilterDCcut(upsampleRate, 12); // filter out the prev rounding errors also
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
    std::cout << "galois white SmallFrame" << std::endl;
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


bool generate_velvet_white( SoundFile& sf, size_t second, size_t upsampleRate )
{
    const std::size_t pulseCountExp = 2;
//    const std::size_t pulseCountExp = 7-upsampleRate;
    GaloisShifter gs;
    NoiseFrameType noiseFrame(gs);
    FrameInterleaveType frameInterleave;
    noiseFrame.clear();
    int64_t count = 1 + float((second+1) * sf.sampleRate()) / NoiseFrameType::sectionSize;
    std::cout << "velvet white raw SmallFrame"  << std::endl;

    while(--count) {
        noiseFrame.fillVelvet2CH(pulseCountExp);
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


} // end namespace SmallFrame


int main( int argc, char *argv[] )
{
    NoiseType nt = NT_white;
    int samplingFrequency( 44100 );
    int fileType( SF_FORMAT_FLAC | SF_FORMAT_PCM_24 );
    size_t lengthSec(30);
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

        std::string const argvalue(argv[i+1]);
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
            if( (tmp<1) || (tmp>4)) {
                std::cerr << "\nillegal upsample rate " << tmp << " use: " << upsampleRate
                    << std::endl;
            } else {
                upsampleRate = tmp;
            }
            break;

        case 'n':
            for( int t = 0; t < int(NT_max); ++t ) {
                if( argvalue.compare(noiseTypes[t]) == 0 ) {
                    nt = NoiseType(t);
                    break;
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
        BigFrame::generate_velvet_white_blt(soundFile, lengthSec, upsampleRate);
        break;
    case NT_dwhite:
        BigFrame::generate_velvet_white_double_blt(soundFile, lengthSec, upsampleRate);
        break;
    case NT_rwhite:
        BigFrame::generate_velvet_white(soundFile, lengthSec, upsampleRate);
        break;
    case NT_red:
        BigFrame::generate_velvet_red_blt(soundFile, lengthSec, upsampleRate);
        break;
    case NT_deepred:
        BigFrame::generate_velvet_deepred_blt(soundFile, lengthSec, upsampleRate);
        break;

    case NT_gwhite:
        BigFrame::generate_galois_white(soundFile, lengthSec, upsampleRate);
        break;



    case NT_swhite:
        SmallFrame::generate_velvet_white_blt(soundFile, lengthSec, upsampleRate);
        break;
    case NT_sdwhite:
        SmallFrame::generate_velvet_white_double_blt(soundFile, lengthSec, upsampleRate);
        break;
    case NT_srwhite:
        SmallFrame::generate_velvet_white(soundFile, lengthSec, upsampleRate);
        break;
    case NT_sred:
        SmallFrame::generate_velvet_red_blt(soundFile, lengthSec, upsampleRate);
        break;
    case NT_sdeepred:
        SmallFrame::generate_velvet_deepred_blt(soundFile, lengthSec, upsampleRate);
        break;
    case NT_sgwhite:
        SmallFrame::generate_galois_white(soundFile, lengthSec, upsampleRate);
        break;
    default:
        help();
        exit(-1);
    }
    return 0;
}

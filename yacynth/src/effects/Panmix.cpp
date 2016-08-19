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
 * File:   Panmix.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on February 18, 2016, 6:24 PM
 */

#include    "Panmix.h"
#include    "../utils/Limiters.h"

#include    <sys/time.h>
#include    <ctime>

//
// TODO: collect the osc output channels to diff multitimbre channels
// need a map to describe which channels to which part
//
//
namespace yacynth {
#if 0
// --------------------------------------------------------------------
Panmix::Panmix()
:   enableEffectFilter(false)
,   controlledFilter()
,   noiseFrame(GaloisShifterSingle<seedThreadEffect_noise>::getInstance())
,   noiseFrameF(GaloisShifterSingle<seedThreadEffect_noise>::getInstance())
,   noiseFrameE(GaloisShifterSingle<seedThreadEffect_noise>::getInstance())
,   noiseSample(GaloisShifterSingle<seedThreadEffect_noise>::getInstance())
<<<<<<< HEAD
=======
//,   galoisShifterCascade(seedThreadEffect_noise)
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262

//,   noiseFilter(noiseFrame)

//,   noiseFilter(GaloisShifterSingle<seedThreadEffect_noise>::getInstance())
,   oscillatorNoise(GaloisShifterSingle<seedThreadEffect_noise>::getInstance())
//,   oscillatorNoise1(GaloisShifterSingle<seedThreadEffect_noise>::getInstance())
,   oscillatorNoiseFloat(GaloisShifterSingle<seedThreadEffect_noise>::getInstance())
{
    filterComb1.setLength(511);
    filterComb2.setLength(473);
    filterComb3.setLength(389);
    filterComb4.setLength(277);

    filterComb1.setCoeff();
    filterComb2.setCoeff();
    filterComb3.setCoeff();
    filterComb4.setCoeff();

    allpass1.clear();
    allpass1.setGain( 0, 1.0f );
    allpass1.setGain( 1, 1.0f );
    allpass1.setK( 0, allpassk[allpasskind & 1] );
    allpass1.setK( 1, allpassk[allpasskind & 1] );

    allpass2.clear();
    allpass2.setGain( 0, 1.0f );
    allpass2.setGain( 1, 1.0f );
    allpass2.setK( 0, allpassk[allpasskind & 1] );
    allpass2.setK( 1, allpassk[allpasskind & 1] );

    allpassk[0] = -0.9f;
    allpassk[1] = -0.2f;
    allpasskind = 0;
    allpasskindcng = 0;
    
    noiselength = 1000;
    noiseinterleave = 1;
<<<<<<< HEAD
=======
    
//    fxOutNoise.setProcMode(2);
//    fxOutNoiseNode.set(fxOutNoise);

>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
};


// --------------------------------------------------------------------
void Panmix::process( OscillatorOut& inp )
{
<<<<<<< HEAD
    noiseFrame.fillWhite ();
=======
    //LowOscillatorArray::getInstance().inc();    // increment lfo's
    
    //noiseFrame.fillWhite ();
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
    
    // put the amplitude value to the global controller
    amplitudeFilter( inp.amplitudeSumm );
//    ControllerMatrix::getInstance().setFloat( ControllerMatrix::C_FLOAT_AMPLITUDE, static_cast<float>( amplitude>>40 ));

//    enableEffectFilter = true;
    summOscillatorOut( inp );
    if( enableEffectFilter ) {
        controlledFilter.process( inFilter, out );
    }
} // end  Panmix::process
// --------------------------------------------------------------------
void Panmix::summOscillatorOut( OscillatorOut& inp ) {
    constexpr   float   gainref = 1.0f/(1L<<25);
//    constexpr   float   gainref = 1.0f/(1L<<16);
    AddVector  addbuff;
    float * outApf;
    float * outBpf;

    timeval tv; // profiling
    
    if( enableEffectFilter ) {
        outApf  = inFilter.channelA;
        outBpf  = inFilter.channelB;
        inFilter.setGain( gainref );
    } else {
        outApf  = out.channelA;
        outBpf  = out.channelB;
        out.setGain( gainref );
    }


    for( uint16_t si = 0u; si < oscillatorOutSampleCount; ++si  ) {
        addbuff.v[si] = inp.layer[0][si] + inp.layer[1][si];
    }

    for( uint16_t ovi = 2u; ovi < inp.overtoneCount; ++ovi ) {
        for( uint16_t si = 0u; si < oscillatorOutSampleCount; ++si ) {
            addbuff.v[si] += inp.layer[ovi][si];
        }
    }
    if( ++allpasskindcng == 256) {
        allpasskind++;
        allpasskindcng = 0;
        allpass1.setK( 0, allpassk[ allpasskind & 1] );
        allpass1.setK( 1, allpassk[ allpasskind & 1] );
        allpass2.setK( 0, allpassk[ allpasskind & 1] );
        allpass2.setK( 1, allpassk[ allpasskind & 1] );
    }

    if ( noiselength++ > 3000 ) {
        noiselength = 0;
        noiseinterleave++;
    }
        
    if( 0 == noiselength) {
        std::cout  << noiseinterleave << std::endl;
    }

    const int64_t  maxC = 1000;


    constexpr float fb = 200.0;

    constexpr uint16_t ff0 = std::round( std::exp( -2.0 * PI * fb / 48000.0 ) * double(0x10000 ) );
    constexpr uint16_t ff1 = std::round( std::exp( -2.0 * PI * fb * 2  / 48000.0 ) * double(0x10000) );
    constexpr uint16_t ff2 = std::round( std::exp( -2.0 * PI * fb * 6  / 48000.0 ) * double(0x10000) );
    constexpr uint16_t ff3 = std::round( std::exp( -2.0 * PI * fb * 9  / 48000.0 ) * double(0x10000) );
    constexpr float ff0f = std::exp( -2.0 * 3.1415 * fb / 48000.0  );
    constexpr float ff1f = std::exp( -2.0 * 3.1415 * fb * 3  / 48000.0 );
    constexpr float ff2f = std::exp( -2.0 * 3.1415 * fb * 5  / 48000.0 );
    constexpr float ff3f = std::exp( -2.0 * 3.1415 * fb * 7  / 48000.0 );

    constexpr uint16_t ffsv1 = std::round( 2.0f * std::sin( 3.1415 * fb / 48000.0 ) * double(0x10000 ) );
    constexpr uint16_t ffsv2 = std::round( 2.0f * std::sin( 3.1415 * fb * 2 / 48000.0 ) * double(0x10000 ) );

//    oscillatorNoise.setF( 0, ff0 );
//    oscillatorNoise.setF( 2, ff2 );
//    oscillatorNoise.setF( 3, ff3 );
    oscillatorNoise.setF( 0, ffsv1 );
    oscillatorNoise.setF( 1, ffsv2 );
    
//    oscillatorNoise.setF( 1, ffsv2 );
    oscillatorNoiseFloat.setF( 0, ff0f );
//    oscillatorNoiseFloat.setF( 1, ff1f );
//    oscillatorNoiseFloat.setF( 2, ff2f );
//    oscillatorNoiseFloat.setF( 3, ff3f );
    int32_t A,B;

    if( --count < 0 ) {
        std::cout << std::dec << " ------- panmix " 
            << timer 
            << " " << timer1
            << std::endl;
        count = maxC;
        timer = 0;
        timer1 = 0;
    }

    // profiling
    
    int32_t noise[oscillatorOutSampleCount];
    float noiseFloat[oscillatorOutSampleCount];

    gettimeofday(&tv,NULL);
    const uint64_t begint   = tv.tv_sec*1000000ULL + tv.tv_usec;
    
    oscillatorNoiseFloat.get1x4pole<oscillatorOutSampleCount>(noiseFloat);        
//    noiseFrame.setPoleExp(9);
//    oscillatorNoise.get1x4pole<oscillatorOutSampleCount>(noise);        

    gettimeofday(&tv,NULL);
    const uint64_t begint1 = tv.tv_sec*1000000ULL + tv.tv_usec;

//    oscillatorNoise.get1x4pole<oscillatorOutSampleCount>(noise);        

        
//    noiseFrameF.fillPurple();
<<<<<<< HEAD
=======
    //fxOutNoiseNode.exec();
//    fxOutNoise.exec();
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
    
    for( uint16_t si = 0u; si < oscillatorOutSampleCount; ++si ) {
//        *outBpf++ = *outApf++ = static_cast<float>( addbuff.v[si] );
        
<<<<<<< HEAD
        *outBpf++ = *outApf++ = static_cast<float>( noiseSample.getPink() );
=======
//        *outBpf++ = *outApf++ = static_cast<float>( noiseSample.getPink() );
//        *outBpf++ = *outApf++ = static_cast<float>( galoisShifterCascade.getWhite24()  );
//        *outApf++ = fxOutNoise.channelA[si];
//        *outBpf++ = fxOutNoise.channelB[si];
>>>>>>> ba07e31dc2378caab3f0e381e4c636f8e4c63262
        
//        const int64_t x =  addbuff.v[si];
//        *outApf++ = static_cast<float>(filterComb4.getAllPass( filterComb3.getAllPass( filterComb2.getAllPass( filterComb1.getAllPass( x ) ))));
//        *outApf++ = static_cast<float>(filterComb3.getAllPass( filterComb2.getAllPass( filterComb1.getAllPass( x ) )));
//        *outBpf++ = static_cast<float>( x );

//        const float x =  static_cast<float>( addbuff.v[si] );
//        float A,B;
//        allpass1.set2mul<1>(x,A);
//        allpass2.set1mul<1>(x,B);

//        *outApf++ = A;
//        *outBpf++ = B;

//        *outApf++ = filterComb4.getAllPass( filterComb3.getAllPass( filterComb2.getAllPass( filterComb1.getAllPass( x ) )));
//        A = noiseFilter.getCrimson( 13 );
//        B = A = noiseFilter.getColor( ff0 );
        // A = noiseFilter.getColor();
//        B = A = noiseFilter.getColorU16( ff0 );
//        B = A = noiseFilter.getColor4( ff0, ff1,ff2, ff3 );
//        B = A = noiseFilter.getColor2( ff0, ff1 );
//        A = noiseFilter.getRed();
       
    //    if( 10 < noiselength ) {
  //          for( auto i=0; i<noiseinterleave; ++i ) {
//                B = lowNoise.getWhite();    
      //      }             
        //}
//        lowNoise.getCrimson(A,B);

//        *outApf++ = *outBpf++ = static_cast<float>( noise[si] ) * 4.0f;
//        *outApf++ = *outBpf++ = noiseFloat[si] * 4.0f;
//        *outApf++ = *outBpf++ = static_cast<float>(
            
            
//            oscillatorNoise.get1xsv( noiseFrame.getFrame()[si] ) 
//           + oscillatorNoise1.get1xsv( noiseFrame.getFrame()[si] ) 
            
            
//            ) ;
//        *outApf++ = static_cast<float>(noiseFrame.getFrame()[si]);
//        *outApf++ = *outBpf++ = static_cast<float>(oscillatorNoise.get1x4pole( noiseFrame.getFrame()[si] )  ) * 4.0f;
//        *outApf++ = *outBpf++ = noiseFrame.getFrame()[si];// * 4.0f;
        
//        *outApf++ = *outBpf++ = static_cast<float>( galoisShifter.getWhiteRaw() );// * 4.0f;
//        *outApf++ = static_cast<float>( B );
    }
    
    // profiling
    gettimeofday(&tv,NULL);
    const uint64_t endt =  tv.tv_sec*1000000ULL + tv.tv_usec;
    timer += endt - begint1;
    timer1 += begint1 - begint;
};
#endif
// --------------------------------------------------------------------

} // end namespace yacynth



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
 * File:   main.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on January 26, 2016, 11:09 PM
 */

#include    "../utils/Biquad.h"
#include    "../utils/BiquadMath.h"
#include    "../utils/Limiters.h"
#include    "../utils/GaloisNoiser.h"
#include    "../utils/FilterBase.h"
#include    "../oscillator/Tables.h"
#include    "../oscillator/Oscillator.h"
#include    "../oscillator/OscillatorArray.h"
#include    "../oscillator/ToneShaper.h"
#include    "../yaio/YaIoJack.h"
#include    "../yaio/IOthread.h"
#include    "../net/Server.h"
#include    "../router/SimpleMidiRouter.h"
#include    "../effects/Filter.h"
#include    "../effects/DelayTap.h"

#include    "../control/Controllers.h"
#include    "../control/Sysman.h"
#include    "../control/SynthFrontend.h"
#include    "yacynth_globals.h"
#include    "v4.h"

#include    <cstdlib>
#include    <iostream>
#include    <fstream>
#include    <thread>
#include    <type_traits>
#include    <iomanip>

#include    <unistd.h>
#include    <pthread.h>
#include    <sndfile.h>
#include    <bitset>
#include    <map>
#include    <list>
#include    <csignal>
#include    <sys/time.h>
#include    <chrono>

using namespace yacynth;
using namespace filter;
using namespace tables;
using namespace noiser;


// beacuse of the signal handler -- must be revisited
YaIoJack    jack;
// --------------------------------------------------------------------
bool initialize(void)
{
    fillExp2Table();  // obsolate
    return true;
}

// --------------------------------------------------------------------

void signal_handler(int sig)
{
    jack.shutdown();
    fprintf(stderr, "yacynth -- signal received, exiting\n");
    exit(0);
}

// --------------------------------------------------------------------

void teststuff(void)
{
    std::chrono::time_point<std::chrono::system_clock> start, end;
    std::chrono::duration<double> elapsed_seconds;

    std::cout
        << "\nOscillatorArray size: "       << sizeof(OscillatorArray)
        << "\nEnvelope size: "              << sizeof(ToneShaperMatrix)
        << "\nToneShaperVector size: "      << sizeof(ToneShaperVector)
        << "\nToneShaper size: "            << sizeof(ToneShaper)
        << "\nsize: v4sf "                  << sizeof(v4sf)
        << "\nsize: V4sf_m "                << sizeof(V4sf_m)
        << std::hex
        << "\nsize: v4sf_u "                << sizeof(V4sfMatrix)
        << "\nfreq2deltaPhase( 1.0 ) "      << freq2deltaPhase( 1.0 )
        << "\nrefA440ycent:       "         << refA440ycent
        << "\nrefA440ycentDouble: "         << uint64_t(std::llround(refA440ycentDouble))
        << "\n19990:"                       << ref19900ycent
        << "\n0.01:"                        << ref0_01ycent
        << std::dec
        << "\n refA440ycent: "              << refA440ycent
        << "\n refA440ycentDouble: "        << uint64_t(refA440ycentDouble*10)
        << "\n refA440ycentDouble: "        << refA440ycentDouble

        << "\n Filter4Pole: "               << sizeof(Filter4Pole<3>)
        << std::endl;
    
    double f0 = 20.0;
    const double df = 1.2;
#if 0   
    for( auto k=0; k<32; k++ ) {
        const uint16_t ff = std::round( std::exp( -2.0 * 3.1415 * f0 / 48000.0 ) * double(0xFFFF) );
        std::cout 
            << "f0 " << f0
            << " ff " << ff
            << std::endl;
            f0 *= df;

    }
           
    
    for( auto k=1; k<16; k++ ) {
        constexpr double      PI2  = 2.0 * 3.141592653589793238462643383279502884197;

        const double x = 1L<<k;    
        const double x1 = 1L<<(k+1);    
        const double x2 = 1L<<(k+3);    

        const double y2k  = -std::log( 1.0 - 1.0 / x  ) / PI2 * 48000.0;                
        const double y2ka  = -std::log( 1.0 - 1.0 / x - 1.0 / x1 ) / PI2 * 48000.0;                
        const double y2kb  = -std::log( 1.0 - 1.0 / x - 1.0 / x2 ) / PI2 * 48000.0;                
        
        const double y2k48000_1  = -std::log( 1.0 - 1.0 / x ) / PI2 * 48000.0;                
        const double y2k44100_1  = -std::log( 1.0 - 1.0 / x ) / PI2 * 44100.0;                
        const double y2k48000_3  = -std::log( 1.0 - 3.0 / x ) / PI2 * 48000.0;                
        const double y2k44100_3  = -std::log( 1.0 - 3.0 / x ) / PI2 * 44100.0;                
        const double y2k48000_5  = -std::log( 1.0 - 5.0 / x ) / PI2 * 48000.0;                
        const double y2k44100_5  = -std::log( 1.0 - 5.0 / x ) / PI2 * 44100.0;                
        const double y2k48000_7  = -std::log( 1.0 - 7.0 / x ) / PI2 * 48000.0;                
        const double y2k44100_7  = -std::log( 1.0 - 7.0 / x ) / PI2 * 44100.0;                
        const double y2k48000_9  = -std::log( 1.0 - 9.0 / x ) / PI2 * 48000.0;                
        const double y2k44100_9  = -std::log( 1.0 - 9.0 / x ) / PI2 * 44100.0;                
        std::cout 
            << "k " << k
            << " y2k " << y2k
            << " y2ka " << y2ka
            << " y2kb " << y2kb
            
            << " y2k48000_1 " << y2k48000_1
            << " y2k48000_3 " << y2k48000_3
            << " y2k48000_5 " << y2k48000_5
            << " y2k48000_7 " << y2k48000_7
            << " y2k48000_9 " << y2k48000_9
            << " y2k44100_1 " << y2k44100_1
            << " y2k44100_3 " << y2k44100_3
            << " y2k44100_5 " << y2k44100_5

            
            << std::endl;        
    }
#endif
#if 0    
    GaloisShifter gs;
    for( uint64_t k=0; k < (1L<<33); k++ ) {
        const uint64_t st = gs.get();
        if( k > 0xFFFFFFFAL && k < 0x100000004L ) {
            std::cout << std::hex
                << "k " << k
                << " st " << st
                << " cycle " << gs.cycle
                << " ls 0 " << gs.lfsr[0]
                << " ls 1 " << gs.lfsr[1]
            
                << std::endl;        
            
        }

    }

    exit(0);
#endif
    
#if 0
        double maxerr = 0.0;
        double err = 0.0;
#if 0
    FilterBase::Ftable2SinPi tablesi;
        std::cout
            << " ======================================  "
            << std::endl;


        for( auto octave = 29; octave < 30; ++octave ) {
            for( auto inOctave = 0; inOctave < 4096; ++inOctave ) {
                const int32_t   val =  ( octave << 24 ) + ( inOctave << 12 );
                const float     valf =  float(val) / (1<<24) ;
                const float     fc =  FilterBase::logF_fc(valf ) ;
                const float     freq =  fc * 48000;
                const int64_t   resi =  tablesi.getInt(val);
                const float     resf =  tablesi.getFloat(val);
                const float     resf1 =  tablesi.getFloat(valf);
                const float     chekf =  2.0 * std::sin( PI * fc );
                if( std::abs ( chekf ) > 1.0e-20  ) {
                    err = ( chekf - resf  ) /  chekf;
                    if( err > maxerr)
                        maxerr = err;
                }
                std::cout
                    << "val   " <<  val
                    << " valf   " <<  valf
                    << " fc   " <<  fc
                    << " freq   " <<  freq
                    << "  resi  " <<  resi
                    << "  resf  " <<  resf
                    << "  resf1  " <<  resf1
                    << "  chekf  " <<  chekf
                    << "  err  " <<  err
                    << "  maxerr  " <<  maxerr
                    << std::endl;

            }
        }

        exit(0);

#endif
#if 0
    FilterBase::FtableSinCosPi2 tablesc;

        std::cout
            << " ======================================  "
            << std::endl;

        maxerr = 0.0;
        err = 0.0;

        for( auto octave = 29; octave < 31; ++octave ) {
            for( auto inOctave = 0; inOctave < 4096; ++inOctave ) {
                const int32_t   val =  ( octave << 24 ) + ( inOctave << 12 );
                const float     valf =  float(val) / (1<<24) ;
                const float     fc =  FilterBase::logF_fc(valf ) ;
                const float     freq =  fc * 48000;
                const int64_t   resi =  tablesc.getInt(val);
                const float     resf =  tablesc.getFloat(val);
                const float     chekf =  ( std::sin( 2 * PI * fc) - 1.0)/ std::cos( 2 * PI * fc) ;
                if( std::abs ( chekf ) > 1.0e-20  ) {
                    err = ( chekf - resf  ) /  chekf;
                    if( err > maxerr)
                        maxerr = err;
                }
                std::cout
                    << "val   " <<  val
                    << " valf   " <<  valf
                    << " fc   " <<  fc
                    << " freq   " <<  freq
                    << "  resi  " <<  resi
                    << "  resf  " <<  resf
                    << "  chekf  " <<  chekf
                    << "  err  " <<  err
                    << "  maxerr  " <<  maxerr
                    << std::endl;

            }
        }

        std::cout
            << " ======================================  "
            << std::endl;
exit(0);
#endif

    FilterBase::FtableExp2Pi table;
        for( auto octave = 28; octave < 30; ++octave ) {
            for( auto inOctave = 0; inOctave < 4096; ++inOctave ) {
                const int32_t   val =  ( octave << 24 ) + ( inOctave <<12 );
                const float     valf =  float(val) / (1<<24) ;
                const float     fc =  FilterBase::logF_fc(valf ) ;
                const float     freq =  fc * 48000;
                const int32_t   resi =  table.getInt(val);
                const float     resf =  table.getFloat(val);
                const float     chekf =  std::exp( - 2 * PI * fc);
                std::cout << std::hex
                    << "val   " <<  val
                    << " valf   " <<  valf
                    << " fc   " <<  fc
                    << " freq   " <<  freq
                    << "  resi  " <<  resi
                    << "  resf  " <<  resf
                    << "  chekf  " <<  chekf
                    << "  diff  " <<  ( chekf - resf  )/  chekf
                    << std::endl;

            }
        }

        std::cout
            << " ======================================  "
            << std::endl;



    exit(0);

#endif

#if 0
    std::stringstream& tout = echoTapsFeedback();
    std::cout
        << "  " << tout.str()
//        << "  " << echoTapsFeedback()

        << std::endl;
    exit(0);

#endif

#if 0
    Chronos stat;

    StereoDelayTapVar  ttap;

    StereoDelayTap  ttapdb;

    StereoDelayTapVector<6,8> tapvect;
    tapvect.setDelayLength(1L<<18);


    ttap.coeff0.aa = ttapdb.coeff.aa = 11.1;
    ttap.coeff0.ab = ttapdb.coeff.ab = 22.2;
    ttap.coeff0.ba = ttapdb.coeff.ba = 33.3;
    ttap.coeff0.bb = ttapdb.coeff.bb = 44.4;

    ttap.delaySrc = ttapdb.delaySrc = 123456789;
    ttap.delayDst = ttapdb.delayDst = 987654321;

    ttap.mult( 100.0 );

    EIObuffer  inp;
    EIObuffer  out;
    EIObuffer  delay(10);

    //-----------------------------------

    for( auto i=0u; i< 64; ++i ) {
        inp.channelA[i] = (i+1)*11.0;
        inp.channelB[i] = (i+1)*13.0;
    }
    for( auto i=0u; i< delay.bufferSize; ++i ) {
        delay.channelA[i] = (i+1)*23.0;
        delay.channelB[i] = (i+1)*37.0;
    }

    std::stringstream   srts;


    srts
        << "DTAP:01 111.112 123.3 0.111 5555.5 500 0"
        << "DTAP:01 222.222 456.3 0.222 5555.5 1000 0"
        << "DTAP:01 333.333 789.3 0.333 5555.5 2000 0"
        << "DTAP:01 444.444 999.3 0.444 5555.5 50000 0"
        << "DTAP:01 555.555 888.3 0.555 5555.5 60000 0"
        << "\n";

    uint16_t cnt = tapvect.fill(srts);

        std::cout
        << " cnt " << cnt
        << std::endl;


        tapvect.list();

        exit(0);

    stat.start();
    start = std::chrono::system_clock::now();
    for( auto i =0; i< 10000000; ++i) {
        delay.multAdd( ttap.delaySrcH + (i&63), ttap.coeff, out.channelA[i & 63], out.channelB[ i & 63 ] );
        delay.pushSection(out.channelA, out.channelB );
    }
    end = std::chrono::system_clock::now();
    stat.end();

    elapsed_seconds = end-start;

    std::cout
        << " dt " << stat.dt()
        << "\n\nelapsed time: delay.multAdd " << elapsed_seconds.count()
        << std::endl;
    //-----------------------------------

    //-----------------------------------

    for( auto i=0u; i< 64; ++i ) {
        inp.channelA[i] = (i+1)*11.0;
        inp.channelB[i] = (i+1)*13.0;
    }
    for( auto i=0u; i< delay.bufferSize; ++i ) {
        delay.channelA[i] = (i+1)*23.0;
        delay.channelB[i] = (i+1)*37.0;
    }


    start = std::chrono::system_clock::now();
    for( auto i =0; i< 10000000; ++i) {
        delay.multAdd( ttap.delaySrcH + (i&63), ttap.coeff, out.channelA[i & 63], out.channelB[ i & 63 ] );
        delay.pushSection(out.channelA, out.channelB );
    }
    end = std::chrono::system_clock::now();

    elapsed_seconds = end-start;

    std::cout
        << "\n\nelapsed time: delay.multAdd " << elapsed_seconds.count()
        << std::endl;
    //-----------------------------------
    for( auto i=0u; i< 64; ++i ) {
        inp.channelA[i] = (i+1)*11.0;
        inp.channelB[i] = (i+1)*13.0;
    }
    for( auto i=0u; i< delay.bufferSize; ++i ) {
        delay.channelA[i] = (i+1)*23.0;
        delay.channelB[i] = (i+1)*37.0;
    }
    start = std::chrono::system_clock::now();
    for( auto i =0; i< 10000000; ++i) {
        delay.multAddnoisefloor( ttap.delaySrcH + (i&63), ttap.coeff, out.channelA[i & 63], out.channelB[ i & 63 ] );
        delay.pushSection(out.channelA, out.channelB );
    }
    end = std::chrono::system_clock::now();

    elapsed_seconds = end-start;

    std::cout
        << "\n\nelapsed time: multAddnoisefloor " << elapsed_seconds.count()
        << std::endl;
    //-----------------------------------
    //-----------------------------------
    for( auto i=0u; i< 64; ++i ) {
        inp.channelA[i] = (i+1)*11.0;
        inp.channelB[i] = (i+1)*13.0;
    }
    for( auto i=0u; i< delay.bufferSize; ++i ) {
        delay.channelA[i] = (i+1)*23.0;
        delay.channelB[i] = (i+1)*37.0;
    }
    start = std::chrono::system_clock::now();
    for( auto i =0; i< 10000000; ++i) {
        delay.multAddnoisefloor( ttap.delaySrcH + (i&63), ttap.coeff, out.channelA[i & 63], out.channelB[ i & 63 ] );
        delay.pushSection(out.channelA, out.channelB );
    }
    end = std::chrono::system_clock::now();

    elapsed_seconds = end-start;

    std::cout
        << "\n\nelapsed time: multAddnoisefloor " << elapsed_seconds.count()
        << std::endl;
    //-----------------------------------

    //-----------------------------------




    ttap.mult( 100.0 );

//    for( auto i=0u; i< 64; ++i ) {
        std::cout
            << "aa "      << ttap.coeff.aa
            << " ab "     << ttap.coeff.ab
            << " ba "     << ttap.coeff.ba
            << " bb "     << ttap.coeff.bb
            << std::endl;

  //  }


    exit(0);
#endif

#if 0
    for( int x = 1; x <  10; ++x ) {
        std::cout << std::hex
            << "x "     <<  x
            << "  "     <<  Spectrum::relFreq2pitch( x )
        << std::endl;

    }
    exit(0);
#endif


#if 0

    ToneShaperVector * tsp = new ToneShaperVector;
    std::string key;
    std::stringstream ssr; // (300000);

    serialize( ssr, *tsp, key );

    std::cout << key << std::endl;
    std::cout << ssr.str() << std::endl;

    // ssr.seekg(0);
    bool ret = deserialize( ssr, *tsp, key );
    std::cout << "ret " << ret << std::endl;

    exit(0);

#endif
#if 0

    long double maxerr = 0;
    long double err;
    for( double x =0.499; x < 0.5001; x += 0.000001 ) {
        const float ex  = std::exp( - PI2 * x );
        const float fex = expTable.fastexpMinus2PI(x); // expTable.fastexpMinus2PI(x);


        err = std::abs( ( ex-fex ) / ex );
        if( maxerr < err ) maxerr = err;

        std::cout
            << "x  "        << x
            << "  exp "     << ex
            << "  fex "     << fex
            << "  err "     << err
            << "  maxerr "  << maxerr
            << std::endl;

    }
    exit(0);
#endif

#if 0

    long double maxerr = 0;
    long double err;
    for( double fi =0.0001; fi < 0.5; fi += 0.000001 ) {
//    for( double fi =0.255; fi > 0.249; fi -= 0.0001 ) {

        const double sinv = ( std::sin( sintable.PI2 * fi ) - 1.0 ) / std::cos( sintable.PI2 * fi );
        double fsinv = sintable.fastSin2PIx_1p_cos2PIx( fi );
//        const float sinv = std::sin(fi) / fi;
//        float fsinv = sintable.sinc( fi );

        if( fi <= 0.249999 || fi >= 0.25000001 ) {
            err = 1.0 - fsinv / sinv  ;
            if( maxerr < std::abs( err )  ) maxerr = std::abs( err ) ;
        }

        std::cout
            << "fi "        << fi
            << " sin "      << sinv
            << " fsin "     << fsinv
            << " err "      << err
            << " maxerr "   << maxerr
            << " X"
            << std::endl;

     }



        exit(0);
#endif

#if 0

    long double maxerr = 0;
    long double err;
    for( float fi =3.14f; fi < 3.15f; fi += 0.0001f ) {

        const float sinv = std::sin( fi );
        const float cosv = std::cos( fi );
        float fsinv;
        float fcosv;
        sintable.fastsincos( fi, fsinv, fcosv );

        if( sinv != 0.0 && fsinv != 0.0 ) {
            err = std::abs( ( sinv-fsinv )/sinv  );
          if( maxerr < err ) maxerr =err;
        }

        std::cout
            << "fi  "    << fi
            << "  sin "    << sinv
           << "  fsin "    << fsinv
            << " err " << err
            << " maxerr " << maxerr
            << "  cos "    << cosv
            << "  fcos "    << fcosv

            << std::endl;

     }



        exit(0);
#endif
#if 0
        biquadStereo.clear();
        BiquadParam::evalRBJLowPass( biquadStereo.getCoeffRef(), 0.25 , 10, 1 );


        float inbuffA[1000];
        float outbuffA[1000];
        float inbuffB[1000];
        float outbuffB[1000];

        for( uint64_t i = 0; i<1000; ++i ) {
            inbuffA[ i ] = float( i );
            inbuffB[ i ] = float( -i );
        }

        uint64_t count = 100000000;
        gettimeofday(&tv,NULL);
        uint64_t begint   = tv.tv_sec*1000000ULL + tv.tv_usec;

        for( uint64_t i = 0; i<count; ++i ) {
            const int ind = i % 500;
//            biquadStereo.get( inbuff[ ind ], outbuff[ ind ] );
//            biquadStereo.get( &inbuff[ ind ] );
//             biquadStereo.get( inbuffA[ ind ], inbuffB[ ind ],  outbuffA[ ind ], outbuffB[ ind ] );
//             biquadStereo.get( &inbuffA[ ind ], &inbuffB[ ind ] );
             biquadStereo.get( inbuffA[ ind ], inbuffB[ ind ], outbuffA[ ind ], outbuffB[ ind ] );
        }


        gettimeofday(&tv,NULL);
        uint64_t endt   = tv.tv_sec*1000000ULL + tv.tv_usec;

        std::cout
            << "count " << count
            << " endt "     << endt
            << " begint "     << begint
            << " diff  "     << endt - begint
            << " avg "     << float( endt - begint ) / count
            << std::endl;

        exit(0);
#endif



#if 0
    filter::Coeffs ccff;
    filter::BiquadParam par;
    for( int i = 0; i < 50; ++i ) {
        const float fc = ( 11000.0 + i * 100 ) ;
        par.evalPeeking( ccff, fc/48000.0, 2,  par.db2gain(40.0) );
        std::cout
            << " f " << fc
            << " b1 "     << ccff.a1
            << " b2 "     << ccff.a2
            << " a0 "     << ccff.b0
            << " a1 "     << ccff.b1
            << " a2 "     << ccff.b2
            << std::endl;

    }
       std::cout
           << " 0 db "  << par.db2gain(0.0)
           << " 6 db "  << filter::BiquadParam::db2gain(6.0)
           << " 20 db " << filter::BiquadParam::db2gain(20.0)
           << std::endl;

    exit(0);
#endif
    // TODO: to replace signal
    //   boost::signals2 ??

//    std::signal(SIGQUIT, signal_handler);
//    std::signal(SIGTERM, signal_handler);
//    std::signal(SIGHUP,  signal_handler);
//    std::signal(SIGINT,  signal_handler);



    //FilterCoeffs    fco;

//    fco.fill();

#if 0

    for( int64_t i = 0; i < 2100; ++i ) {
        const float inp = float(i) / 512.0  ;
        const float out = compressFloat.compressSoft5( inp );

        std::cout << std::setprecision(8)
            << "i " << i
            << " in " << inp
            << " out " << out
            << std::endl;

        if( 0 == ( i % 16 )  ) {
            std::cout << " ------------ " << std::endl;
        }
    }
    exit(0);
#endif

#if 0

    std::cout << std::hex
        << " freq2ycent "
        << "\n   440 " << freq2ycent(440)
        << "\n    20 " << freq2ycent(20)
        << "\n    40 " << freq2ycent(40)
        << "\n  1000 " << freq2ycent(1000)
        << "\n  2000 " << freq2ycent(2000)
        << "\n  4000 " << freq2ycent(4000)
        << std::endl;


    for( int i = 20; i < 40; ++i  ) {
        double jd = i;
        uint32_t ycent = freq2ycent(jd) ;

        std::cout << std::hex
            << "  j " << jd
            << " freq2ycent " << ycent
            << std::endl;
    }


#endif

#if 0

    RouteIn routA440;
    Yamsgrt result;

    routA440.chn            = 0;
    routA440.note_cc_val        = 69;
    routA440.op             = MIDI_NOTE_ON;
    routA440.velocity_val   = 64;



    result  = midiRoute->translate( routA440 );
// 0x193b0973
    std::cout << std::hex
        << "69 result: " << result.store
        << " deltaPhase: " << result.setVoice.pitch
        << " velocity: " << result.setVoice.velocity

        << std::endl;

    routA440.chn            = 0;
    routA440.note_cc_val        = 57;
    routA440.op             = MIDI_NOTE_ON;
    routA440.velocity_val   = 127;

    result  = midiRoute->translate( routA440 );
// 0x193b0973
    std::cout << std::hex

        << "57  result: " << result.store
        << " deltaPhase: " << result.setVoice.pitch
        << " velocity: " << result.setVoice.velocity

        << std::endl;

//    exit(0);

    // user interface server
//    uiServer.run();

       std::cout << " 1" << std::endl;

//   exit(0);
#endif


           
//    GNoise<galoisShifter, 2> gNoise;  
}


// --------------------------------------------------------------------
//  -- MAIN --
// --------------------------------------------------------------------
int main( int argc, char** argv )
{
    uint16_t            port( 7373 );         // from param
    std::string         host( "127.0.0.1" );    // allowed from
    struct sigaction sigact;
    memset (&sigact, '\0', sizeof(sigact));
    static_assert( 8 == sizeof(Yamsgrt), "sizeof(Yamsgrt) must be 8" );
    static_assert( 0x193b0973 == refA440ycent, "refA440ycent must be 0x193b0973" );
    if( ! initialize() ) {
        exit(-1);
    }

    // to test things begin
    teststuff();
    // to test things end

    //  singletons first    
    // random generators
    GaloisShifterSingle<seedThreadEffect_noise>& gs0        = GaloisShifterSingle<seedThreadEffect_noise>::getInstance();
    GaloisShifterSingle<seedThreadOscillator_noise>& gs1    = GaloisShifterSingle<seedThreadOscillator_noise>::getInstance();
    GaloisShifterSingle<seedThreadEffect_random>& gs2       = GaloisShifterSingle<seedThreadEffect_random>::getInstance();
    GaloisShifterSingle<seedThreadOscillator_random>& gs3   = GaloisShifterSingle<seedThreadOscillator_random>::getInstance();

    std::cout 
        << "\ngs0 " << static_cast< void *>(&gs0)
        << "\ngs1 " << static_cast< void *>(&gs1)
        << "\ngs2 " << static_cast< void *>(&gs2)
        << "\ngs3 " << static_cast< void *>(&gs3)
        << std::endl;
    
#if 0
    for( uint64_t i=0u; i<0x1000000000000UL; ++i) {

        const uint64_t vv = static_cast<GaloisShifter&>(GaloisShifterSingle<seedThreadOscillator_random>::getInstance()).get();
        if( 0 == ( i & 0x3FFFFFFFFFUL ) ) {
            std::cout << std::hex
                << "i " << i
                << " vv " << vv
                << std::endl;
            
        }
    }
#endif
    
    // inter thread communication
    YaIoInQueueVector&   queuein    = YaIoInQueueVector::getInstance();
    OscillatorOutVector& oscOutVec  = OscillatorOutVector::getInstance();
    ControllerMatrix&    contrVect  = ControllerMatrix::getInstance();

    OscillatorArray     *oscArray   = new OscillatorArray();
    SimpleMidiRouter    *midiRoute  = new SimpleMidiRouter();

    // threads
    IOThread            *iOThread   = new IOThread(      queuein, oscOutVec, *midiRoute );
    SynthFrontend       *synthFe    = new SynthFrontend( queuein, oscOutVec, oscArray );

    Sysman              *sysman     = new Sysman( *oscArray, *iOThread );
    Server              uiServer( host, port, *sysman );

    std::cout << std::hex

        << "queuein: " << (void *)&queuein
        << " oscOutVec: " << (void *)&oscOutVec
        << " oscArray: " << (void *)oscArray
        << " iOThread: " << (void *)iOThread
        << " synthFe: " << (void *)synthFe

        << std::endl;
// start going up
//    effects.check();

    jack.setProcessCB( iOThread, IOThread::midiInCB,  IOThread::audioOutCB );
    if( ! synthFe->initialize() )
        exit(-1);
    if( !jack.initialize() )
        exit(-1);
    if( !jack.run() )
        exit(-1);
    std::thread   synthFrontendThread( SynthFrontend::exec, synthFe );

    // -------------------------------------
    std::cout << "\n\n save ToneShaper[0]\n\n" << std::endl;
    ToneShaperMatrix& ts = oscArray->getToneShaperMatrix();
    std::stringstream ser;
    ts.dump( ser );
    std::ofstream file_tsout;
    file_tsout.open("toneshaper_out");
    file_tsout << ser.rdbuf();
    file_tsout.close();

    std::ifstream file_tsin;
    file_tsin.open("toneshaper_in");
    if( file_tsin.good() ) {
        std::cout << "\n\n read a new  ToneShaper[0]\n\n" << std::endl;
        ser.seekp(0);
        ser << file_tsin.rdbuf();
        file_tsin.close();
        ts.fill( ser );
    }

    std::cout << "\n\n============LETS GO==============\n\n" << std::endl;


// run without thread
//   synthFe->run();

   jack.unmute();
   uiServer.run();  // command processor
   jack.mute();
   synthFrontendThread.join();
   jack.shutdown();
//-----------------------------------------------------------
    return 0;
};


//-----------------------------------------------------------

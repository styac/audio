#pragma once

/*
 * Copyright (C) 2016 Istvan Simon
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
 * File:   FilterAllpass.h
 * Author: Istvan Simon
 *
 * Created on March 21, 2016, 6:52 PM
 */

// http://www.music.mcgill.ca/~ich/classes/FiltersChap2.pdf

// http://www.micromodeler.com/dsp/
// http://home.deib.polimi.it/bestagini/_Slides/lesson_3.pdf

#include    "FilterBase.h"

using namespace tables;

namespace filter {

typedef float v4sf __attribute__((mode(SF)))  __attribute__ ((vector_size(16),aligned(16)));

// **********************

class FilterAllpassBase : public FilterBase {
public:
    static constexpr    uint16_t oversamplingRate   = 1;
    static constexpr    float    fMin               = 25.0/48000.0;   // highest freq ~19500
    static constexpr    float    fMax               = 20000.0/48000.0;     // lowest freq ~23 lower might be unstable
    static constexpr    int      qRateExp           = 48;

protected:
    // NEW

};

// **********************

class FilterAllpass1 : public FilterAllpassBase {
public:
    inline int32_t controller2logF( const uint16_t index )  // controller - 0 .. 127 ; 0 = max freq
    {
        constexpr int32_t val0   = 0x1e500000 - (oversamplingRate-1) * 0x1000000;
        constexpr int32_t val127 = 0x15000000 - (oversamplingRate-1) * 0x1000000;
        constexpr int32_t rate   = ( val0 - val127 ) / 127;
        const int64_t logF =  val0 - int32_t(index) * rate;
//        param1.flog.setStep( logF );
    }

    inline int32_t controller2logQ( const uint16_t index )
    {
//        param1.qlog.setStep(  int64_t(index) << qRateExp );
    }


    static inline float evalFc( const float freq, const float samplingFrequency )
    {
        const float x = PI2 * freq / samplingFrequency;
        return  ( std::sin(x) - 1 ) / std::cos(x);
    };

    enum FilterType {
        BYPASS,
        ALLPASS,
        XPASS,
    };

    struct alignas(16) Channel {
        float   zx;
        float   zy;
    };

    struct Param {
        float           igain;
        float           fgain;
        float           qgain;

        float       ftarget;
        float       fcurrent;
        float       fdelta;
        int32_t     fcount;
        float       c;
        FilterType  type;
    };

    FilterAllpass1()
    {
        clear();
        param1.fcount = 1;
        sweep();
    };

    inline void setType( const FilterType t ) { param1.type=t; };

    void clear(void)
    {
        channelA    = {0};
        channelB    = {0};
        param1      = {0};
        param1.fcurrent = param1.ftarget = fMax;
    };

    float checkF( const float fv )
    {
        return fv <= fMin ? fMin : fv >= fMax ? fMax : fv;
    };
    inline void setF( float fv )
    {
        param1.ftarget  = checkF( fv ) ;
        param1.fdelta   = ( param1.ftarget - param1.fcurrent ) * (1.0f / paramIntCount);
        param1.fcount   = paramIntCount;
    };

    inline bool sweep( void )
    {
        bool ret = false;
        if( param1.fcount > 0 ) {
            --param1.fcount;
            param1.fcurrent += param1.fdelta;
            const float cosv = sintable.fastcos2PI( param1.fcurrent );   // f/fs
            const float sinv = sintable.fastsin2PI( param1.fcurrent );
            param1.c = ( sinv - 1.0f ) / cosv;
            ret = true;
        }
        return ret;
    };

    inline void getA( const float in, float& out )
    {
        doA(in);
        switch( param1.type ) {
        case BYPASS:
            out = in;
            return;
        case ALLPASS:
            out = channelA.zy;
            return;
        case XPASS:
            out = in - channelA.zy;
            return;
        }
    };

private:
    // Mitra-Hirano 1A
    inline void doA( const float in )
    {
        channelA.zy     = param1.c * ( in - channelA.zy ) + channelA.zx;
        channelA.zx     = in;
    };
    Channel     channelA;
    Channel     channelB;
    Param       param1; // obsolate
}; // end FilterAllpass1
// --------------------------------------------------------------------
//
//
// http://www.music.mcgill.ca/~ich/classes/FiltersChap2.pdf
//  page 12
//

class FilterAllpass2 : public FilterAllpassBase {
public:
    static constexpr    float    fMin               = 20.0/48000.0;
    static constexpr    float    fMax               = 19000.0/48000.0; // start changing phase

    // smaller q is bigger q !!! - smaller transient zone
    static constexpr    float    qMin               = 1.1f; // Fpi = Fbeg * q
    static constexpr    float    qMax               = 4.0f; // ??

    inline int32_t controller2logF( const uint16_t index )  // controller - 0 .. 127 ; 0 = max freq
    {
        constexpr int32_t val0   = 0x1e500000 - (oversamplingRate-1) * 0x1000000;
        constexpr int32_t val127 = 0x15000000 - (oversamplingRate-1) * 0x1000000;
        constexpr int32_t rate   = ( val0 - val127 ) / 127;
        const int64_t logF =  val0 - int32_t(index) * rate;
//        param1.flog.setStep( logF );
    }

    inline int32_t controller2logQ( const uint16_t index )
    {
//        param1.qlog.setStep(  int64_t(index) << qRateExp );
    }

    static inline float evalFc( const float freq, const float samplingFrequency )
    {
        const float x = PI2 * freq / samplingFrequency;
        return  ( std::sin(x) - 1 ) / std::cos(x);
    };
    static inline float evalFc48000( const float freq )
    {
        const float x = PI2 / 48000.0f * freq;
        return  ( std::sin(x) - 1 ) / std::cos(x);
    };
    static inline float evalFc44100( const float freq )
    {
        const float x = PI2 / 44100.0f * freq;
        return  ( std::sin(x) - 1 ) / std::cos(x);
    };

    // c = (  sin( 2 * pi * f/fs ) - 1 ) / cos ( 2 * PI * f/fs )
    // d = - cos (  2 * PI * f/fs )
    // c2 = d * (1 - c1)
    // c2 = sin( 2 * pi * f/fs )

    enum FilterType {
        BYPASS,                // get the input: the filter is runnning
        ALLPASS,
        XPASS,
        YPASS
    };

    struct alignas(16) Channel {
        float   zx1;
        float   zy1;
        float   zx2;
        float   zy2;
    };

    struct Param {
        float           igain;
        float           fgain;
        float           qgain;

        // OBSOLATE
        float       ftarget;
        float       fcurrent;
        float       fdelta;
        int32_t     fcount;
        float       qtarget;
        float       qcurrent;
        float       qdelta;
        int32_t     qcount;
        float       c1;
        float       c2;
        FilterType  type;
    };

    FilterAllpass2()
    {
        clear();
        param1.fcount = 1;
        sweep();
    };

    inline void setType( const FilterType t ) { param1.type=t; };

    void clear(void)
    {
        channelA    = {0};
        channelB    = {0};
        param1      = {0};
        param1.fcurrent = param1.ftarget = fMax;
        param1.qcurrent = param1.qtarget = qMin;
    };

    inline float checkF( const float fv )
    {
        return fv <= fMin ? fMin : fv >= fMax ? fMax : fv;
    };

    inline float checkQ( const float qv )
    {
        return qv <= qMin ? qMin : qv >= qMax ? qMax : qv;
    };

    inline void setQ( float qv )
    {
        param1.qtarget  = checkQ( qv ) ;
        param1.qdelta   = ( param1.qtarget - param1.qcurrent ) * (1.0f / paramIntCount);
        param1.qcount   = paramIntCount;
    };

    inline void setF( float fv )
    {
        param1.ftarget  = checkF( fv ) ;
        param1.fdelta   = ( param1.ftarget - param1.fcurrent ) * (1.0f / paramIntCount);
        param1.fcount   = paramIntCount;
    };


    // x = 2*pi*f/fs
    // c = (sin(x1) - 1)/cos(x1)
    // d = -cosv2
    // c1 = -c
    // c2 = d*(1-c)



    inline bool sweep( void )
    {
        bool ret = false;
        if( 0 < param1.qcount ) {
            --param1.qcount;
            param1.qcurrent += param1.qdelta;
            if( 0 >= param1.qcount ) {
                param1.qdelta = 0.0f;
            }
            if( 0 >= param1.fcount ) {
                evalFQ();
            }
            ret = true;
        }
        if( 0 < param1.fcount ) {
            --param1.fcount;
            param1.fcurrent += param1.fdelta;
            if( 0 >= param1.fcount ) {
                param1.fdelta = 0.0f;
            }
            evalFQ();
            ret = true;
        }
        return ret;
    };

    inline void getA( const float in, float& out )
    {
        doA(in);
        switch( param1.type ) {
        case BYPASS:
            out = in;
            return;
        case ALLPASS:
            out = channelA.zy2;
            return;
        case XPASS:
            out = in - channelA.zy2;
            return;
        case YPASS:
            out = in + channelA.zy2;
            return;
        }
    };

private:
    void evalFQ(void) {
        //const float c   = ( std::sin( PI2 * param1.fcurrent ) - 1.0f ) / std::cos(  PI2 * param1.fcurrent );
        // const float d   = - std::cos(  PI2 * fpi );
        const float c   = sintable.fastSin2PIx_1p_cos2PIx( param1.fcurrent );
        const float d   = -sintable.fastcos2PI( std::min( param1.fcurrent * param1.qcurrent, fMax ) );   // f/fs
        param1.c1 = -c;
        param1.c2 = d * ( c - 1.0f );
#if 0
        std::cout
            << "f "  << param1.fcurrent
            << " q "  << param1.qcurrent
            << " c "  << c
            << " d "  << d
            << " c1 "  << param1.c1
            << " c2 "  << param1.c2
            << std::endl;
#endif
    };

    // Mitra-Hirano 3D
    inline void doA( const float in )
    {
        const float t1  = param1.c1 * ( in - channelA.zy2 ) + param1.c2 * ( channelA.zx1 - channelA.zy1 ) + channelA.zx2 ;
        channelA.zx2    = channelA.zx1;
        channelA.zx1    = in;
        channelA.zy2    = channelA.zy1;
        channelA.zy1    = t1;
    };
    Channel     channelA;
    Channel     channelB;
    Param       param1;
}; // end FilterAllpass2

// --------------------------------------------------------------------

class FilterAllpass2X4 : public FilterAllpassBase {
public:
    static constexpr    float    fMin               = 20.0/48000.0;
    static constexpr    float    fMax               = 11000.0/48000.0; // start changing phase

    // smaller q is bigger q !!! - smaller transient zone
    static constexpr    float    qMin               = 1.1f; // Fpi = Fbeg * q
    static constexpr    float    qMax               = 4.0f; // ??
    inline int32_t controller2logF( const uint16_t index )  // controller - 0 .. 127 ; 0 = max freq
    {
        constexpr int32_t val0   = 0x1e500000 - (oversamplingRate-1) * 0x1000000;
        constexpr int32_t val127 = 0x15000000 - (oversamplingRate-1) * 0x1000000;
        constexpr int32_t rate   = ( val0 - val127 ) / 127;
        const int64_t logF =  val0 - int32_t(index) * rate;
  //      param1.flog.setStep( logF );
    }

    inline int32_t controller2logQ( const uint16_t index )
    {
//        param1.qlog.setStep(  int64_t(index) << qRateExp );
    }


    static inline float evalFc( const float freq, const float samplingFrequency )
    {
        const float x = PI2 * freq / samplingFrequency;
        return  ( std::sin(x) - 1 ) / std::cos(x);
    };
    static inline float evalFc48000( const float freq )
    {
        const float x = PI2 / 48000.0f * freq;
        return  ( std::sin(x) - 1 ) / std::cos(x);
    };
    static inline float evalFc44100( const float freq )
    {
        const float x = PI2 / 44100.0f * freq;
        return  ( std::sin(x) - 1 ) / std::cos(x);
    };

    // c = (  sin( 2 * pi * f/fs ) - 1 ) / cos ( 2 * PI * f/fs )
    // d = - cos (  2 * PI * f/fs )
    // c2 = d * (1 - c1)
    // c2 = sin( 2 * pi * f/fs )

    enum FilterType {
        BYPASS,                // get the input: the filter is runnning
        ALLPASS,
        XPASS,
        YPASS
    };

    struct alignas(16) Channel {
        float   zx1s0;
        float   zy1s0;
        float   zx2s0;
        float   zy2s0;
        float   zx1s1;
        float   zy1s1;
        float   zx2s1;
        float   zy2s1;
        float   zx1s2;
        float   zy1s2;
        float   zx2s2;
        float   zy2s2;
        float   zx1s3;
        float   zy1s3;
        float   zx2s3;
        float   zy2s3;
    };

    struct Param {
        float           igain;
        float           fgain;
        float           qgain;

        float       ftarget;
        float       fcurrent;
        float       fdelta;
        int32_t     fcount;
        float       qtarget;
        float       qcurrent;
        float       qdelta;
        int32_t     qcount;
        float       c1;
        float       c2;
        FilterType  type;
    };

    FilterAllpass2X4()
    {
        clear();
        param1.fcount = 1;
        sweep();
    };

    inline void setType( const FilterType t ) { param1.type=t; };

    void clear(void)
    {
        channelA    = {0};
        channelB    = {0};
        param1      = {0};
        param1.fcurrent = param1.ftarget = fMax;
        param1.qcurrent = param1.qtarget = qMin;
    };

    inline float checkF( const float fv )
    {
        return fv <= fMin ? fMin : fv >= fMax ? fMax : fv;
    };

    inline float checkQ( const float qv )
    {
        return qv <= qMin ? qMin : qv >= qMax ? qMax : qv;
    };

    inline void setQ( float qv )
    {
        param1.qtarget  = checkQ( qv ) ;
        param1.qdelta   = ( param1.qtarget - param1.qcurrent ) * (1.0f / paramIntCount);
        param1.qcount   = paramIntCount;
    };

    inline void setF( float fv )
    {
        param1.ftarget  = checkF( fv ) ;
        param1.fdelta   = ( param1.ftarget - param1.fcurrent ) * (1.0f / paramIntCount);
        param1.fcount   = paramIntCount;
    };


    // x = 2*pi*f/fs
    // c = (sin(x1) - 1)/cos(x1)
    // d = -cosv2
    // c1 = -c
    // c2 = d*(1-c)



    inline bool sweep( void )
    {
        bool ret = false;
        if( 0 < param1.qcount ) {
            --param1.qcount;
            param1.qcurrent += param1.qdelta;
            if( 0 >= param1.qcount ) {
                param1.qdelta = 0.0f;
            }
            if( 0 >= param1.fcount ) {
                evalFQ();
            }
            ret = true;
        }
        if( 0 < param1.fcount ) {
            --param1.fcount;
            param1.fcurrent += param1.fdelta;
            if( 0 >= param1.fcount ) {
                param1.fdelta = 0.0f;
            }
            evalFQ();
            ret = true;
        }
        return ret;
    };

    inline void getA( const float in, float& out )
    {
        doA(in);
        switch( param1.type ) {
        case BYPASS:
            out = in;
            return;
        case ALLPASS:
            out = channelA.zy2s3;
            return;
        case XPASS:
            out = in - channelA.zy2s3;
            return;
        case YPASS:
            out = in + channelA.zy2s3;
            return;
        }
    };

private:
    void evalFQ(void) {
        //const float c   = ( std::sin( PI2 * param1.fcurrent ) - 1.0f ) / std::cos(  PI2 * param1.fcurrent );
        // const float d   = - std::cos(  PI2 * fpi );
        const float c   = sintable.fastSin2PIx_1p_cos2PIx( param1.fcurrent );
        const float d   = -sintable.fastcos2PI( std::min( param1.fcurrent * param1.qcurrent, fMax ) );   // f/fs
        param1.c1 = -c;
        param1.c2 = d * ( c - 1.0f );
#if 0
        std::cout
            << "f "  << param1.fcurrent
            << " q "  << param1.qcurrent
            << " c "  << c
            << " d "  << d
            << " c1 "  << param1.c1
            << " c2 "  << param1.c2
            << std::endl;
#endif
    };

    // Mitra-Hirano 3D
    inline void doA( const float in )
    {
        const float t0    = param1.c1 * ( in - channelA.zy2s0 ) + param1.c2 * ( channelA.zx1s0 - channelA.zy1s0 ) + channelA.zx2s0 ;
        channelA.zx2s0    = channelA.zx1s0;
        channelA.zx1s0    = in;
        channelA.zy2s0    = channelA.zy1s0;
        channelA.zy1s0    = t0;

        const float t1    = param1.c1 * ( channelA.zy2s0 - channelA.zy2s1 ) + param1.c2 * ( channelA.zx1s1 - channelA.zy1s1 ) + channelA.zx2s1 ;
        channelA.zx2s1    = channelA.zx1s1;
        channelA.zx1s1    = channelA.zy2s0;
        channelA.zy2s1    = channelA.zy1s1;
        channelA.zy1s1    = t1;

        const float t2    = param1.c1 * ( channelA.zy2s1 - channelA.zy2s2 ) + param1.c2 * ( channelA.zx1s2 - channelA.zy1s2 ) + channelA.zx2s2 ;
        channelA.zx2s2    = channelA.zx1s2;
        channelA.zx1s2    = channelA.zy2s1;
        channelA.zy2s2    = channelA.zy1s2;
        channelA.zy1s2    = t2;

        const float t3    = param1.c1 * ( channelA.zy2s2 - channelA.zy2s3 ) + param1.c2 * ( channelA.zx1s3 - channelA.zy1s3 ) + channelA.zx2s3 ;
        channelA.zx2s3    = channelA.zx1s3;
        channelA.zx1s3    = channelA.zy2s2;
        channelA.zy2s3    = channelA.zy1s3;
        channelA.zy1s3    = t3;
    };
    Channel     channelA;
    Channel     channelB;
    Param       param1;
//    Param       param2;
//    Param       param3;
//    Param       param4;
//      Lfo     lfo
}; // end FilterAllpass2X4
// --------------------------------------------------------------------
template< std::size_t tableSizeX >
class FilterAllpassTableF {
public:
    static constexpr std::size_t tableSizeExp = tableSizeX;
    static constexpr std::size_t tableSize = 1<<tableSizeExp;
    static constexpr double PI  = 3.141592653589793238462643383279502884197;
    FilterAllpassTableF()
    :   FilterAllpassTableF( 25.0, 19900.0, 48000.0, 1 )
    {};
    FilterAllpassTableF( const double minFreq, const double maxFreq, const float samplingFrequencyP, const int16_t oversamplingRateP )
    :   oversamplingRate(oversamplingRateP)
    ,   samplingFrequency(samplingFrequencyP)
    {
        const double mult   = 1.0 / samplingFrequency / oversamplingRate;
        const double dfreq  = std::pow( 2.0f, std::log2( maxFreq / minFreq ) / tableSize ) ;
        double freq = minFreq;
        for( int i = 0; i < tableSize; ++i ) {
            f[ i ] =  freq * mult;
            freq *= dfreq;
        }
    };
    float get( const uint64_t index ) { return index < tableSize ? f[ index ] : f[ tableSize - 1 ]; };

private:
    float           f[ tableSize ];
    const int16_t   oversamplingRate;
    const float     samplingFrequency;
};

template< std::size_t tableSizeX >
class FilterAllpassTableQ {
public:
    static constexpr std::size_t tableSizeExp = tableSizeX;
    static constexpr std::size_t tableSize = 1<<tableSizeExp;
    static constexpr double PI  = 3.141592653589793238462643383279502884197;
    FilterAllpassTableQ()
    :   FilterAllpassTableQ( 0.2, 5.0 )
    {};
    FilterAllpassTableQ( const double minQ, const double maxQ )
    {
        const double dq  = std::pow( 2.0f, std::log2( maxQ / minQ ) / tableSize ) ;
        double qv = minQ;
        for( int i = 0; i < tableSize; ++i ) {
            q[ i ] =  qv;
            qv *= dq;
        }
    };
    float get( const uint64_t index ) { return index < tableSize ? q[ index ] : q[ tableSize - 1 ]; };

private:
    float           q[ tableSize ];
};

} // end namespace
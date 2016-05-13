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
 * File:   GaloisNoiser.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on March 12, 2016, 7:11 AM
 */
#include    "Limiters.h"

#include    <cstdint>
#include    <cmath>

//
// https://users.ece.cmu.edu/~koopman/lfsr/
//
// 1101 0001 1001 1000000000000000000000000000000000000000000000000001
//  0xd198000000000001
//
// (( p & 0x8000000000000000ULL ) ? ( p <<= 1, p ^=0xd198000000000001ULL ) : ( p<<=1 ))
//

using namespace limiter;

/*
 possible distant seeds
 ------------------------
count  5a00000 gNoiseShaped 86d1f04542518499 sec 5706 6c87
count  9a00000 gNoiseShaped dfc0f54e2bec6e01 sec 5706 71fd
count  fa00000 gNoiseShaped 5104a43013e444ba sec 5706 7a2e
 */
namespace noiser {
// --------------------------------------------------------------------
class GaloisShifter {
public:
    static constexpr uint64_t seed1     = 0x86d1f04542518499ULL;
    static constexpr uint64_t seed2     = 0xdfc0f54e2bec6e01ULL;
    static constexpr uint64_t seed3     = 0x5104a43013e444baULL;
    static constexpr uint64_t seed      = 0x868485654FE84945ULL;
    static constexpr int shdownWhite    = 24; // spectrum looks white noise in this slice

    GaloisShifter()
    :   lfsr(seed)
    {};
    inline void reset(void)
    {
        lfsr = seed;
    }
    inline void reset( uint64_t seedext )
    {
        lfsr = seedext;
    }
    inline uint64_t get(void)
    {
        return lfsr = (lfsr << 1) ^ (uint64_t(( int64_t(lfsr & 0x8000000000000000ULL)>>63) & 0xd198000000000001ULL));
    };
    inline int32_t getWhite(void)
    {
        return get() >> shdownWhite;
    };

    inline int16_t getWhite16(void)
    {
        return get() >> shdownWhite;
    };

protected:
    uint64_t    lfsr;
};
// --------------------------------------------------------------------
class GNoise16 : public GaloisShifter {
public:
    static constexpr int16_t bfilt  = 1023;
    static constexpr int16_t afilt  = 10;
    inline int16_t getWhite(void)
    {
        const int16_t x0A = GaloisShifter::getWhite16();
        const int16_t dcA = x0A - x1A;
        x1A = x0A;
        const int16_t avgA = ( dcA + x2A ) >> 1;
        x2A = dcA;
        return avgA;
    };

    inline int16_t getRed( void )
    {
        const int32_t n = getWhite();
        return y_1A = n + ((( y_1A - n ) * bfilt ) >> afilt );
    }
    
protected:
    int16_t x1A;
    int16_t x2A;
    int16_t y_1A;
};
// --------------------------------------------------------------------
class GNoiseShaped : public GaloisShifter {
public:
    static constexpr float range    = 1.0f/(1L<<32);
    inline void getWhitef( float& outA )
    {
        outA = getWhitef();
    }
    inline void getWhitef( float& outA, float& outB )
    {
        int32_t A,B;
        getWhite( A, B );
        outA = static_cast< float >  ( A ) * range;
        outB = static_cast< float >  ( B ) * range;
    }

    inline float getWhitef(void)
    {
        return static_cast< float > ( getWhite() ) * range;
    };

    inline int32_t getWhite(void)
    {
        const int32_t x0A = GaloisShifter::getWhite();
        const int32_t dcA = x0A - x1A;
        x1A = x0A;
        const int32_t avgA = ( dcA + x2A ) >> 1;
        x2A = dcA;
        return avgA;
    };
    inline void getWhite( int32_t& A, int32_t& B )
    {
        const int32_t x0A = GaloisShifter::getWhite();
        const int32_t x0B = GaloisShifter::getWhite();
        const int32_t dcA = x0A - x1A;
        const int32_t dcB = x0B - x1B;
        x1A = x0A;
        x1B = x0B;
        const int32_t avgA = ( dcA + x2A ) >> 1;
        const int32_t avgB = ( dcB + x2B ) >> 1;
        x2A = dcA;
        x2B = dcB;
        A = avgA;
        B = avgB;
    };

    template< std::size_t range >
    float   noiseX(void)
    {
        constexpr float rangef = 1.0f / (1L << ( 32 + range ));
        return static_cast< float > ( getWhite()) * rangef;
    }
    template< std::size_t range >
    float   noiseXU(void)
    {
        constexpr float rangef = 1.0f / (1L << ( 32 + range ));
        return static_cast< float > ( uint32_t(getWhite())) * rangef;
    }

private:
    int32_t x1A;
    int32_t x2A;
    int32_t x1B;
    int32_t x2B;
};
// --------------------------------------------------------------------
class GNoise : public GNoiseShaped {
public:
    static constexpr int16_t bfilt  = 1023;
    static constexpr int16_t afilt  = 10;
    static constexpr float range    = 1.0f/(1L<<32);

    inline float getRedf(void)
    {
        return static_cast< float > ( getRed() ) * range;
    }
    inline void getRedf( float& outA )
    {
        outA = static_cast< float > ( getRed() ) * range;
    }
    inline int32_t getRed( void )
    {
        const int64_t nsample = GNoiseShaped::getWhite();
        return y_1A = nsample + ((( y_1A - nsample ) * bfilt ) >> afilt );
//        return y_1A = (( GNoiseShaped::getWhite() >> afilt ) + bfilt * y_1A ) >> 10;
    }

protected:
    int32_t     y_1A;
};
// --------------------------------------------------------------------
class GNoiseStereo : public GNoiseShaped {
public:
    static constexpr int bfilt      = 1023;
    static constexpr int afilt      = 5;
    static constexpr float range    = 1.0f/(1L<<22);
/*
    void getWhitef( float& outA, float& outB )
    {
        outA = GNoiseShaped::getWhitef();
        outB = GNoiseShaped::getWhitef();
    }
 */
    void getRedf( float& outA, float& outB )
    {
        int32_t A, B;
        getRed( A, B );
        outA = static_cast< float > (A) * range;
        outB = static_cast< float > (B) * range;
    }
    void getRed( int32_t& outA, int32_t& outB )
    {
        outA = y_1A = (( GNoiseShaped::getWhite() >> afilt ) + bfilt * y_1A ) >> 10;
        outB = y_1B = (( GNoiseShaped::getWhite() >> afilt ) + bfilt * y_1B ) >> 10;
    }

protected:
    int32_t     y_1A;
    int32_t     y_1B;
};
// --------------------------------------------------------------------
//
// simple fast 4 pole filter
//
class GNoiseColorBase : public GNoiseShaped {
public:
    static constexpr int64_t    fMin               = 60000<<15;
    static constexpr int64_t    fMax               = 65000<<15;
    static constexpr int64_t    qMin               = 0;
    static constexpr int64_t    qMax               = 300;  // practically
    static constexpr int64_t    lim                = 30000;

    enum FilterType {
        LOWPASS,
        BANDPASS1,
        BANDPASS2,
        BANDPASS3,
    };
    struct alignas(16) Channel {
        int64_t     y0a;
        int64_t     y0b;
        int64_t     y0c;
        int64_t     y0d;
    };
    struct Param {
        int64_t     fcurrent;
        int64_t     qcurrent;
        int64_t     qgain;
        FilterType  type;
    };
    inline void setType( const FilterType t ) { param1.type=t; };

    inline void setFdeltaPhase( const uint32_t deltaPhase )
    {
        param1.fcurrent = std::lround( (1LL<<32) * 0.97 ) - (( 300000LL * deltaPhase ) >> 16 );
    };

    inline void setF( const uint32_t fv = 0x7F000000 )
    {
        param1.fcurrent     = fv;
    };

    inline void setQ( const uint32_t qv = 0 )
    {
        param1.qcurrent  = qv;
        param1.qgain = param1.qcurrent; // ???
    };

protected:
    Param       param1;

};
// --------------------------------------------------------------------
class GNoiseColor : public GNoiseColorBase {
public:
    GNoiseColor()
    {
        clear();
    };
    void clear(void)
    {
        channelA    = {0};
        param1      = {0};
        setF();
        setQ();
    };

    // int32_t in is the envelope curve
    inline void get( int32_t envelope, float& out )
    {
        doFeedback( envelope );
        switch( param1.type ) {
        case LOWPASS:
            out = float( channelA.y0d ) * (1.0f/(1L<<30));
            return;
        case BANDPASS1:
            out = float( channelA.y0d - channelA.y0a ) * (1.0f/(1L<<31));
            return;
        case BANDPASS2:
            out = float( channelA.y0d - channelA.y0b ) * (1.0f/(1L<<30));
            return;
        case BANDPASS3:
            out = float( channelA.y0d - channelA.y0c ) * (1.0f/(1L<<29));
            return;
        }
    };

    inline int64_t get( int32_t envelope )
    {
        doFeedback( envelope );
        switch( param1.type ) {
        case LOWPASS:
            return channelA.y0d;
        case BANDPASS1:
            return channelA.y0d - channelA.y0a;
        case BANDPASS2:
            return channelA.y0d - channelA.y0b;
        case BANDPASS3:
            return channelA.y0d - channelA.y0c;
        }
    };

private:
    inline void doFeedback( const int32_t in )
    {
        const int64_t x = (( getWhite() * int64_t(in) ) >> 31 ) - ( i64LimitClip<lim>( channelA.y0d ) * param1.qgain );
        channelA.y0a    = ((( channelA.y0a - x ) * param1.fcurrent ) >> 32 ) + x;
        channelA.y0b    = ((( channelA.y0b - channelA.y0a ) * param1.fcurrent ) >> 32 ) + channelA.y0a;
        channelA.y0c    = ((( channelA.y0c - channelA.y0b ) * param1.fcurrent ) >> 32 ) + channelA.y0b;
        channelA.y0d    = ((( channelA.y0d - channelA.y0c ) * param1.fcurrent ) >> 32 ) + channelA.y0c;
    };
    Channel     channelA;
}; // end GNoiseColor
// --------------------------------------------------------------------

class GNoiseColorStereo : public GNoiseColorBase {
public:
    GNoiseColorStereo()
    {
        clear();
    };
    void clear(void)
    {
        channelA    = {0};
        channelB    = {0};
        param1      = {0};
        setF();
        setQ();
    };

    // int32_t in is the envelope curve
    inline void get( int32_t envelope, float& outA, float& outB )
    {
        doFeedback( envelope );
        switch( param1.type ) {
        case LOWPASS:
            outA = float( channelA.y0d ) * (1.0f/(1L<<30));
            outB = float( channelB.y0d ) * (1.0f/(1L<<30));
            return;
        case BANDPASS1:
            outA = float( channelA.y0d - channelA.y0a ) * (1.0f/(1L<<31));
            outB = float( channelB.y0d - channelB.y0a ) * (1.0f/(1L<<31));
            return;
        case BANDPASS2:
            outA = float( channelA.y0d - channelA.y0b ) * (1.0f/(1L<<30));
            outB = float( channelB.y0d - channelB.y0b ) * (1.0f/(1L<<30));
            return;
        case BANDPASS3:
            outA = float( channelA.y0d - channelA.y0c ) * (1.0f/(1L<<29));
            outB = float( channelB.y0d - channelB.y0c ) * (1.0f/(1L<<29));
            return;
        }
    };

private:
    inline void doFeedback( const int32_t in )
    {
        const int64_t xA = (( getWhite() * int64_t(in) ) >> 31 ) - ( i64LimitClip<lim>( channelA.y0d ) * param1.qgain );
        const int64_t xB = (( getWhite() * int64_t(in) ) >> 31 ) - ( i64LimitClip<lim>( channelB.y0d ) * param1.qgain );
        channelA.y0a    = ((( channelA.y0a - xA ) * param1.fcurrent ) >> 32 ) + xA;
        channelB.y0a    = ((( channelB.y0a - xB ) * param1.fcurrent ) >> 32 ) + xB;
        channelA.y0b    = ((( channelA.y0b - channelA.y0a ) * param1.fcurrent ) >> 32 ) + channelA.y0a;
        channelB.y0b    = ((( channelB.y0b - channelB.y0a ) * param1.fcurrent ) >> 32 ) + channelB.y0a;
        channelA.y0c    = ((( channelA.y0c - channelA.y0b ) * param1.fcurrent ) >> 32 ) + channelA.y0b;
        channelB.y0c    = ((( channelB.y0c - channelB.y0b ) * param1.fcurrent ) >> 32 ) + channelB.y0b;
        channelA.y0d    = ((( channelA.y0d - channelA.y0c ) * param1.fcurrent ) >> 32 ) + channelA.y0c;
        channelB.y0d    = ((( channelB.y0d - channelB.y0c ) * param1.fcurrent ) >> 32 ) + channelB.y0c;
    };
    Channel     channelA;
    Channel     channelB;
}; // end GNoiseColorStereo
// --------------------------------------------------------------------
extern GaloisShifter   grandomOscillatorThread;

// --------------------------------------------------------------------
} // end namespace noiser

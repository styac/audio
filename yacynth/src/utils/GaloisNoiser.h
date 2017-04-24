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
#include    "../utils/FilterBase.h"

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


/*
 possible distant seeds
 ------------------------
count  5a00000 gNoiseShaped 86d1f04542518499 sec 5706 6c87
count  9a00000 gNoiseShaped dfc0f54e2bec6e01 sec 5706 71fd
count  fa00000 gNoiseShaped 5104a43013e444ba sec 5706 7a2e
 *
 * might be needed to count the instances and use different seeds for each instance !!!
 * to avoid correlation
 *
 */
using namespace limiter;
using namespace tables;
using namespace filter;

typedef int     v4si __attribute__((mode(SI)))  __attribute__ ((vector_size(16),aligned(16)));
typedef int     v4hi __attribute__((mode(HI)))  __attribute__ ((vector_size(8),aligned(16)));
typedef float   v4sf __attribute__((mode(SF)))  __attribute__ ((vector_size(16),aligned(16)));

namespace noiser {
// --------------------------------------------------------------------
//constexpr uint64_t seed0    = 0x868485654FE84945ULL;
//constexpr uint64_t seed1    = 0x86d1f04542518499ULL;

//constexpr uint64_t seed_32_0  = 0x868485654FE84945ULL; // seed0
// state 2^31
//constexpr uint64_t seed_32_1  = 0xf3e094e0ba8eb9fcULL;

/*
 i 0 vv dc910aca9fd0928b
i 1000000000 vv 36749a41c4b366e1
 *
i 2000000000 vv d85d5d197edb5d43
 *
i 3000000000 vv 2376b412d04e1453
 *
i 4000000000 vv 4d63d50ccdff3eb
 *
i 5000000000 vv f239f3cac7d02c4e


 */
// 0
constexpr uint64_t seedThreadEffect_noise       = 0x868485654FE84945ULL;
// i c000000000 vv 65cb8e6b9323242d
constexpr uint64_t seedThreadOscillator_noise   = 0x65cb8e6b9323242dULL;
// i 4000000000 vv e5eb475e9f173ed2 from seedThreadOscillator_noise
constexpr uint64_t seedThreadEffect_random      = 0xe5eb475e9f173ed2ULL;
// i 8000000000 vv from seedThreadOscillator_noise
constexpr uint64_t seedThreadOscillator_random  = 0x2da068af77c909abULL;


// i 8000000000 vv e4858c0e6cbc01b--

// --------------------------------------------------------------------
class GaloisShifterCascade {
public:
    //                                      7766554433221100
//  static constexpr uint64_t feedback  = 0xd19850abe0000001ULL;
    static constexpr uint64_t feedback  = 0xd198000000000001ULL;
    static constexpr int shdownWhite    = 24; // spectrum looks white noise in this slice

    GaloisShifterCascade( const uint64_t seed )
    { state2.lfsr = state1.lfsr = seed; };

    inline void reset( const uint64_t seed )
    {
        state1.lfsr = state2.lfsr = seed;
    }

    inline void inc(void)
    {
        const uint64_t s1 = int64_t(state1.lfsr)>>63;
        const uint64_t s2 = int64_t(state2.lfsr)>>63;
        state1.lfsr = (state1.lfsr + state1.lfsr) ^ ( s1 & feedback );
        if( (s1 & 0x070) == 0x070 ) {
            state2.lfsr = (state2.lfsr + state2.lfsr) ^ ( s2 & feedback );
        }

    };
    inline uint64_t getState(void)
    {
        return state2.lfsr;
    };

    inline uint64_t get(void)
    {
        inc();
        return state2.lfsr;
    };

    inline uint16_t get16(void)
    {
        inc();
        return state2.lfsr16[7];   // most random part
    };

    inline uint32_t getLow(void)
    {
        inc();
        return state2.lfsr32[0];
    };

    inline uint32_t getHigh(void)
    {
        inc();
        return state2.lfsr32[1];
    };

    // 24 bit significant
    inline int32_t getWhite24(void)
    {
        union {
            int32_t  res;
            int16_t  res16[2];
        };
        inc();
        // +3dB at 10kHz
        res16[1] = state2.lfsr8[6]^state2.lfsr8[7];
        res16[0] = state2.lfsr16[3];
        return res;
    };

    inline int32_t getWhiteRaw(void)
    {
        inc();
        return (state2.lfsr >> shdownWhite);
    };

    // 0..8 is BLUE
    inline int32_t getWhiteRaw( const uint8_t sh )
    {
        inc();
        return state2.lfsr >> sh;
    };

    struct State {
        union {
            uint64_t        lfsr;
            int32_t         lfsr32[2];
            int16_t         lfsr16[4];
            int8_t          lfsr8[8];
        };
    };

protected:
    State   state1;
    State   state2;
};

// --------------------------------------------------------------------
class GaloisShifter {
public:
    //                                      7766554433221100
//  static constexpr uint64_t feedback  = 0xd19850abe0000001ULL;
    static constexpr uint64_t feedback  = 0xd198000000000001ULL;
    static constexpr int shdownWhite    = 24; // spectrum looks white noise in this slice

    GaloisShifter( const uint64_t seed = seedThreadEffect_noise ) // default should be deleted
//    GaloisShifter( const uint64_t seed )
    :   lfsr(seed)
    {};

    inline void reset( const uint64_t seed )
    {
        lfsr = seed;
    }

    inline void inc(void)
    {
        lfsr = (lfsr + lfsr) ^ (uint64_t(( int64_t(lfsr)>>63) & feedback ));
    };
    inline uint64_t getState(void)
    {
        return lfsr;
    };

    inline uint64_t get(void)
    {
        inc();
        return lfsr;
    };

    inline uint16_t get16(void)
    {
        inc();
        return lfsr16[7];   // most random part
    };

    inline uint32_t getLow(void)
    {
        inc();
        return lfsr32[0];
    };

    inline uint32_t getHigh(void)
    {
        inc();
        return lfsr32[1];
    };

    // 24 bit significant
    inline int32_t getWhite24(void)
    {
        union {
            int32_t  res;
            int16_t  res16[2];
        };
        inc();
        // +3dB at 10kHz
        res16[1] = lfsr8[6]^lfsr8[7];
        res16[0] = lfsr16[3];
        return res;
    };

    inline int32_t getWhiteRaw(void)
    {
        inc();
        return (lfsr >> shdownWhite);
    };

    // 0..8 is BLUE
    inline int32_t getWhiteRaw( const uint8_t sh )
    {
        inc();
        return lfsr >> sh;
    };

protected:
    union alignas(sizeof(uint64_t)) {
        uint64_t        lfsr;
        int32_t         lfsr32[2];
        int16_t         lfsr16[4];
        int8_t          lfsr8[8];
    };
};


// --------------------------------------------------------------------
// a singleton for the effect stage noise vectors

template< uint64_t seed >
class GaloisShifterSingle : public GaloisShifter {
public:
    inline static GaloisShifterSingle& getInstance(void)
    {
        thread_local static GaloisShifterSingle instance;
        return instance;
    }

    inline static void reset(void)
    {
        static_cast<GaloisShifter>( GaloisShifterSingle<seed>::getInstance() ).reset(seed);
    }

private:
    GaloisShifterSingle()
    :   GaloisShifter(seed)
    {};
};

// --------------------------------------------------------------------

//
// cutoff frequencies: (Hz) fs = 48000 Hz
//  x = 1L<<k;
//  -std::log( (x-1)/x ) / 3.1415 / 2.0 * 48000.0;
//      ( 2^k - 1 ) / 2^k
//
//
/*
k 1 f 5295.41
k 2 f 2197.79
k 3 f 1020.13 -- 1323
k 4 f 493.053
k 5 f 242.549 -- 331
k 6 f 120.312
k 7 f 59.9192 -- 82
k 8 f 29.9009
k 9 f 14.9358  -- 20.6
k 10 f 7.46425
k 11 f 3.73122 -- 5.17
k 12 f 1.86538
k 13 f 0.932633
k 14 f 0.466302
k 15 f 0.233148
k 16 f 0.116573 -> random LFO
k 17 f 0.0582862
k 18 f 0.0291431
k 19 f 0.0145715
k 20 f 0.00728575

*/

// ----------------------------------------------------------
// TESTING
class OscillatorNoise {
public:
    OscillatorNoise() = delete;
    OscillatorNoise( GaloisShifter& gs )
    :   galoisShifter(gs)
    { clear(); };

    struct State {
        inline void clear(void)
        {
            sin = 0;
            vs[0] = vs[1] = vs[2] = vs[3] = v4si{0};
            g[0] = g[1] = g[2] = g[3] = 0;
        }
        union {
            v4si        vs[2];
            int32_t     s[8];
        };

        union {
            v4si    vf;
            int32_t f[4];
        };
        int32_t     sin;
        uint8_t     g[4];
    };

    inline void clear(void)
    {
        state.clear();
    }

    inline void setF( const uint8_t index, const uint16_t f )
    {
        state.f[index&3] = f;
    }
    inline void setG( const uint8_t index, const uint8_t g )
    {
        state.g[index&3] = g;
    }


    inline int32_t get1x4pole( void )
    {
        const int32_t x0 = state.sin;
        state.sin = galoisShifter.getWhiteRaw() >> 3;
        const int32_t vx =  state.sin + x0 - ( state.s[3] << 2 );
        state.s[0] = (( state.s[0] - vx ) >> 16 ) * state.f[0] + vx;
        state.s[1] = (( state.s[1] - state.s[0] ) >> 16 ) * state.f[0] + state.s[0];
        state.s[2] = (( state.s[2] - state.s[1] ) >> 16 ) * state.f[0] + state.s[1];
        const int32_t vv = (( state.s[3] - state.s[2] + 0x08000 ) >> 16 ) * state.f[0];  // rounding --> noise at DC !
        state.s[3] = vv + state.s[2];
        return vv;
    };

    inline int32_t get4x4pole( void )
    {
        const int32_t x0 = state.sin;
        state.sin = galoisShifter.getWhiteRaw() >> 3;
        const v4si vx =  state.sin + x0 - ( state.vs[3] << 2 );
        state.vs[0] = (( state.vs[0] - vx ) >> 16 ) * state.vf + vx;
        state.vs[1] = (( state.vs[1] - state.vs[0] ) >> 16 ) * state.vf + state.vs[0];
        state.vs[2] = (( state.vs[2] - state.vs[1] ) >> 16 ) * state.vf + state.vs[1];

        const union {
            v4si        vs3;
            int32_t     s3[4];
        } vv = { .vs3 = (( state.vs[3] - state.vs[2] + 0x08000 ) >> 16 ) * state.vf  };  // rounding --> noise at DC !
        state.vs[3] = vv.vs3 + state.vs[2];
        return ( vv.s3[0] >> state.g[0] ) + ( vv.s3[1] >> state.g[1] ) + ( vv.s3[2] >> state.g[2] ) + ( vv.s3[3] >> state.g[3] );
    };
/*
 s[0] - low
 s[1] - high
 s[2] - band

        channel[EbufferPar::chA].low1   = channel[EbufferPar::chA].low2         + channel[EbufferPar::chA].band2 * param1.fEff;
        channel[EbufferPar::chA].high1  = in - channel[EbufferPar::chA].low1    - channel[EbufferPar::chA].band2 * param1.qEff;
        channel[EbufferPar::chA].band1  = channel[EbufferPar::chA].band2        + channel[EbufferPar::chA].high1 * param1.fEff;

 */
    template< std::size_t count >
    inline void get1xsv( int32_t * out )
    {
        for( auto i = 0u; i < count; ++i ) {
            state.s[0] += ((state.s[2] + 0x08000 )>>16) * state.f[0];
            state.s[1] =  ( galoisShifter.getWhiteRaw() >> 6 ) - state.s[0] - ( state.s[2] >> 9 );
            const int32_t vv = state.s[2];
            state.s[2] += ((state.s[1] + 0x08000 )>>16) * state.f[0];
            *out++ = state.s[2] + vv;
        }
    };
    template< std::size_t count >
    inline void get2xsv( int32_t * out )
    {
        for( auto i = 0u; i < count; ++i ) {
            const int32_t nsampl = galoisShifter.getWhiteRaw() >> 6;
            const int32_t vv = state.s[2] + state.s[6];
            state.s[0] += ((state.s[2] + 0x08000 )>>16) * state.f[0];
            state.s[4] += ((state.s[6] + 0x08000 )>>16) * state.f[1];
            state.s[1] =  ( nsampl ) - state.s[0] - ( state.s[2] >> 6 );
            state.s[5] =  ( nsampl ) - state.s[4] - ( state.s[6] >> 6 );
            state.s[2] += ((state.s[1] + 0x08000 )>>16) * state.f[0];
            state.s[6] += ((state.s[5] + 0x08000 )>>16) * state.f[1];

            *out++ =  state.s[2] + state.s[6] + vv;
        }
    };

#if 0
    template< std::size_t count >
    inline void get1x4pole( int32_t * out )
    {
        for( auto i = 0u; i < count; ++i ) {
            const int32_t vx = ( galoisShifter.getWhiteRaw() >> 3 );// - ( state.s[3] << 2 );
            state.s[0] = (( state.s[0] - vx ) >> 16 ) * state.f[0] + vx;
            state.s[1] = (( state.s[1] - state.s[0] ) >> 16 ) * state.f[0] + state.s[0];
            state.s[2] = (( state.s[2] - state.s[1] ) >> 16 ) * state.f[0] + state.s[1];
            state.s[3] = (( state.s[3] - state.s[2] ) >> 16 ) * state.f[0] + state.s[2];
            state.s[4] = (( state.s[4] - state.s[3] ) >> 16 ) * state.f[0] + state.s[3];
            state.s[5] = (( state.s[5] - state.s[4] ) >> 16 ) * state.f[0] + state.s[4];
            const int32_t vv = state.sin;
            state.sin = (( state.s[8] - state.s[5] + 0x08000 ) >> 16 ) * state.f[0];  // rounding --> noise at DC !
            state.s[8] = state.sin + state.s[5];
            *out++ = state.s[8] -  state.s[3];
        }
    };
#endif
    //state.s[2] >> 3,4 looks good
    inline int32_t get1xsv( const int32_t sample  )
    {
        state.s[0] += ((state.s[2] + 0x08000 )>>16) * state.f[0];
        state.s[1] = sample - state.s[0] - ( state.s[2] >> 4 );
        state.s[2] += ((state.s[1] + 0x08000 )>>16) * state.f[0];

        return state.s[2];
    };

    inline int32_t get2xsv( const int32_t sample  )
    {
        state.s[3] = state.s[2];

        state.s[0] += ((state.s[2] + 0x08000 )>>16) * state.f[0];

        state.s[4] += ((state.s[6] + 0x08000 )>>16) * state.f[1];
        state.s[1] = sample - state.s[0] - ( state.s[2] >> 4 );

        state.s[5] = state.s[3] - state.s[4] - ( state.s[6] >> 4 );

        state.s[2] += ((state.s[1] + 0x08000 )>>16) * state.f[0];
        state.s[6] += ((state.s[5] + 0x08000 )>>16) * state.f[1];

        return state.s[6];
    };


    inline int32_t get1x4pole( const int32_t sample  )
    {
        const int32_t vx = ( ( sample - state.s[3] ) << 2 );
        state.s[0] = (( state.s[0] - vx ) >> 16 ) * state.f[0] + vx;
        state.s[1] = (( state.s[1] - state.s[0] ) >> 16 ) * state.f[0] + state.s[0];
        state.s[2] = (( state.s[2] - state.s[1] ) >> 16 ) * state.f[0] + state.s[1];
        const int32_t xx = state.s[7];
        state.s[7] = (( state.s[3] - state.s[2] + 0x08000 ) >> 16 ) * state.f[0];  // rounding --> noise at DC !
        state.s[3] = state.s[7] + state.s[2];
        return state.s[7] + xx;
    };


    template< std::size_t count >
    inline void get1x4pole( int32_t * out )
    {
        for( auto i = 0u; i < count; ++i ) {
//            const int32_t vx = ( galoisShifter.getWhiteRaw() >> 3 ) - i32LimitClip<0x08000000>( state.s[3] << 3 );
            const int32_t vx = ( galoisShifter.getWhiteRaw() >> 3 ) - ( state.s[3] << 2 );
            state.s[0] = (( state.s[0] - vx ) >> 16 ) * state.f[0] + vx;
            state.s[1] = (( state.s[1] - state.s[0] ) >> 16 ) * state.f[0] + state.s[0];
            state.s[2] = (( state.s[2] - state.s[1] ) >> 16 ) * state.f[0] + state.s[1];
            const int32_t vv = state.sin;
            state.sin = (( state.s[3] - state.s[2] + 0x08000 ) >> 16 ) * state.f[0];  // rounding --> noise at DC !
            state.s[3] = state.sin + state.s[2];
            *out++ = vv + state.sin;
        }
    };

    template< std::size_t count >
    inline void get4x4pole( int32_t * out )
    {
        for( auto i = 0u; i < count; ++i ) {
            const int32_t x0 = state.sin;
            state.sin = galoisShifter.getWhiteRaw() >> 3;
            const v4si vx =  state.sin + x0 - ( state.vs[3] << 2 );
            state.vs[0] = (( state.vs[0] -          vx ) >> 16 ) * state.vf + vx;
            state.vs[1] = (( state.vs[1] - state.vs[0] ) >> 16 ) * state.vf + state.vs[0];
            state.vs[2] = (( state.vs[2] - state.vs[1] ) >> 16 ) * state.vf + state.vs[1];
            const union {
                v4si        vs3;
                int32_t     s3[4];
            } vv = { .vs3 = (( state.vs[3] - state.vs[2] + 0x08000 ) >> 16 ) * state.vf };  // rounding --> noise at DC !

            state.vs[3] = vv.vs3 + state.vs[2];
            *out++ = ( vv.s3[0] >> state.g[0] ) + ( vv.s3[1] >> state.g[1] ) + ( vv.s3[2] >> state.g[2] ) + ( vv.s3[3] >> state.g[3] );
        }
    };

private:
    State           state;
    GaloisShifter&  galoisShifter;
};

class OscillatorNoiseFloat {
public:
    OscillatorNoiseFloat() = delete;
    OscillatorNoiseFloat( GaloisShifter& gs )
    :   galoisShifter(gs)
    { clear(); };

    struct State {
        inline void clear(void)
        {
            sin = 0;
            vs[0] = vs[1] = vs[2] = vs[3] = v4sf{0};
            g[0] = g[1] = g[2] = g[3] = 1.0f;
        }
        union {
            v4sf    vs[4];
            float   s[16];
        };
        union {
            v4sf    vf;
            float   f[4];
        };
        union {
            v4sf    vg;
            float   g[4];
        };
        int32_t     sin;
    };
    inline void clear(void)
    {
        state.clear();
    }

    inline void setF( const uint8_t index, const float f )
    {
        state.f[index&3] = f;
    }
    inline void setG( const uint8_t index, const float g )
    {
        state.g[index&3] = g;
    }

    template< std::size_t count >
    inline void get1x4pole( float * out )
    {
        for( auto i = 0u; i < count; ++i ) {
            const int32_t x0 = state.sin;
            state.sin = galoisShifter.getWhiteRaw() >> 3;
            const float vx = static_cast<float>( state.sin + x0 ) - ( state.s[3] * 3.9999f );
            state.s[0] = ( state.s[0] -          vx ) * state.f[0] + vx;
            state.s[1] = ( state.s[1] - state.s[0] ) * state.f[0] + state.s[0];
            state.s[2] = ( state.s[2] - state.s[1] ) * state.f[0] + state.s[1];
            const float vs = ( state.s[3] - state.s[2] ) * state.f[0];
            state.s[3] = vs + state.s[2];

            *out++ = vs;
        }
    };

    template< std::size_t count >
    inline void get4x4pole( float * out )
    {
        for( auto i = 0u; i < count; ++i ) {
            const int32_t x0 = state.sin;
            state.sin = galoisShifter.getWhiteRaw() >> 3;
            const v4sf vx = static_cast<float>( state.sin + x0 ) - ( state.vs[3] * 3.9999f );
            state.vs[0] = ( state.vs[0] -          vx ) * state.vf + vx;
            state.vs[1] = ( state.vs[1] - state.vs[0] ) * state.vf + state.vs[0];
            state.vs[2] = ( state.vs[2] - state.vs[1] ) * state.vf + state.vs[1];
            const v4sf vs = ( state.vs[3] - state.vs[2] ) * state.vf;
            state.vs[3] = vs + state.vs[2];

            const union {
                v4sf      vs3;
                float     s3[4];
            } vv = { .vs3 = vs * state.vg };

            *out++ = vv.s3[0] + vv.s3[1] + vv.s3[2] + vv.s3[3];
        }
    };


private:
    State           state;
    GaloisShifter&  galoisShifter;
};


// ----------------------------------------------------------
#if 0
class NoiseFrameFilter {
public:
    NoiseFilterMultiColor() = delete;
    NoiseFilterMultiColor( GaloisShifter& gs )
    :   galoisShifter(gs)
    {};

    static constexpr float range    = 1.0f/(1L<<30);

    struct State {
        union {
            v4si  v4[2];
            struct {
                int32_t s[8];
            };
        };
    };


    inline void getWhite( float& v )
    {
        v = static_cast< float > ( getWhite() ) * range;
    };

    inline void getWhite( int32_t& v )
    {
        v = getWhite();
    };

    inline void getBlue( float& v )
    {
        v = static_cast< float > ( getBlue() ) * range;
    };

    inline void getBlue( int32_t& v )
    {
        v = getBlue();
    };

    // average of 2 consecutive samples
    inline int32_t getWhiteBasic(void)
    {
        const int32_t x = state.s[0][0];
        state.s[0][0] = galoisShifter.getWhiteRaw() >> 2;
        return  x + state.s[0][0];
    };
    inline int32_t getWhiteBasic(const uint8_t sh)
    {
        const int32_t x = state.s[0][0];
        state.s[0][0] = galoisShifter.getWhiteRaw(sh) >> 2;
        return  x + state.s[0][0];
    };

    // dc-cut .. high cut white
    inline int32_t getWhite( void )
    {
        const int32_t x0 = state.s[0][0];
        const int32_t x1 = state.s[0][1];
        state.s[0][0] = galoisShifter.getWhiteRaw() >> 2;
        state.s[0][1] += x0 - state.s[0][0] - (state.s[0][1]>>11);
        return x1 + state.s[0][1];
    };

    inline void getWhite( int32_t& outA, int32_t& outB )
    {
        const int32_t x0A = state.s[0][0];
        const int32_t x1A = state.s[0][1];
        const int32_t x0B = state.s[0][2];
        const int32_t x1B = state.s[0][3];
        state.s[0][0] = galoisShifter.getWhiteRaw() >> 2;
        state.s[0][1] += x0A - state.s[0][0] - (state.s[0][1]>>11);
        state.s[0][2] = galoisShifter.getWhiteRaw() >> 2;
        state.s[0][3] += x0B - state.s[0][2] - (state.s[0][3]>>11);
        outA = x1A + state.s[0][1];
        outB = x1B + state.s[0][3];
    };

    inline int32_t getBlue(void)
    {
        const int32_t x0 = state.s[0][0];
        const int32_t x1 = state.s[0][1];
        state.s[0][0] = galoisShifter.getWhiteRaw() >> 2;
        state.s[0][1] = x0 - state.s[0][0];
        return state.s[0][1] + x1;
    };

    inline int32_t getRed(void)
    {
        const int32_t x0 = state.s[0][0];
        const int32_t x2 = state.s[0][2];
        state.s[0][0] = galoisShifter.getWhiteRaw() >> 2;
        state.s[0][1] += x0 - state.s[0][0] - (state.s[0][1]>>9);
        state.s[0][2] += ( state.s[0][1]>>5 ) - (state.s[0][2]>>9);
        return x2 + state.s[0][2];
    };

    inline int32_t getCrimson(void)
    {
        const int32_t x0 = state.s[0][0];
        const int32_t x3 = state.s[0][3];
        state.s[0][0] = galoisShifter.getWhiteRaw() >> 2;
        state.s[0][1] += ( x0 - state.s[0][0] ) - (state.s[0][1]>>9);
        state.s[0][2] += ( state.s[0][1]>>5 ) - (state.s[0][2]>>9);
        state.s[0][3] += ( state.s[0][2]>>8 ) - (state.s[0][3]>>9);
        return x3 + state.s[0][3];
    };

#if 0

    inline void getCrimson( float& v )
    {
        v = static_cast< float > ( getCrimson() ) * range;
    };

    inline void getCrimson( int32_t& v )
    {
        v = getCrimson();
    };
    inline int32_t getRed(void)
    {
        const int32_t x0 = state.s[0][0];
        state.s[0][0] += (galoisShifter.getWhiteRaw()>>8) - (state.s[0][0]>>10);
        return x0 + state.s[0][0];
    };

    inline int32_t getCrimson(void)
    {
        const int32_t x1 = state.s[0][1];
        state.s[0][0] += ( galoisShifter.getWhiteRaw()>>8) - (state.s[0][0]>>9);
        state.s[0][1] += ( state.s[0][0]>>7 ) - (state.s[0][1]>>9);
        return x1 + state.s[0][1];
    };
#endif

    inline int32_t getRed( const int32_t pv )
    {
        const int32_t poleExp  = ( pv & 0xF ) + 1;
        // gain compensation factors
        const int32_t cf1 = (poleExp>>1) + 1;
        const int32_t cf2 = cf1 + (poleExp&1);
        // save for low cut-high cut
        const int32_t x0 = state.s[0][0];
        const int32_t x2 = state.s[0][2];
        // range to avoid overflow
        state.s[0][0] = galoisShifter.getWhiteRaw() >> 2;
        // diff -- DC cut
        state.s[0][1] += x0 - state.s[0][0] - (state.s[0][1]>>poleExp);
        // 1 pole - 6 db/octave
        state.s[0][2] += ( state.s[0][1]>>cf1 ) + ( state.s[0][1]>>cf2 ) - ( state.s[0][2]>>poleExp );
        // high cut - fs/2
        return x2 + state.s[0][2];
    };

    // input : 0..15 -> poleExp = 1..16
    //
    inline int32_t getPurple( const int32_t pv )
    {
        const int32_t poleExp  = ( pv & 0xF ) + 1;
        // gain compensation factors
        const int32_t cf1 = (poleExp>>1) + 1;
        const int32_t cf2 = cf1 + (poleExp&1);
        const int32_t cf3 = poleExp;
        // save for low cut-high cut
        const int32_t x0 = state.s[0][0];
        const int32_t x3 = state.s[0][3];
        // range to avoid overflow
        state.s[0][0] = galoisShifter.getWhiteRaw() >> 2;
        // diff -- DC cut
        state.s[0][1] += x0 - state.s[0][0] - (state.s[0][1]>>poleExp);
        // 2 poles - 12 db/octave
        state.s[0][2] += ( state.s[0][1]>>cf1 ) + ( state.s[0][1]>>cf2 ) - ( state.s[0][2]>>poleExp );
        state.s[0][3] += ( state.s[0][2]>>cf3 ) - ( state.s[0][3]>>poleExp );
        // high cut - fs/2
        return x3 + state.s[0][3];
    };

    // shift: 9..16
    inline int32_t getRedLfo(void)
    {
        return state.s[0][0] += ( galoisShifter.getWhiteRaw()>>9 ) - (state.s[0][0]>>15);
    };
    inline int32_t getCrimsonLfo1(void)
    {
        const int32_t x1 = state.s[0][1];
        state.s[0][0] += ( galoisShifter.getWhiteRaw()>>9) - (state.s[0][0]>>14);
        state.s[0][1] += ( state.s[0][0]>>14 ) - (state.s[0][1]>>14);
        return state.s[0][1];
//        return x1 + channel.s[1];

    };

// The pinking filter consists of several one-pole low-pass filters,
// where each low-pass is spaced two octaves from its neighbors and
// filter gains are attenuated in 6dB steps.
// This configuration results in a nearly linear -3dB per octave overall
// frequency response when the low-pass filters are summed.
// http://www.firstpr.com.au/dsp/pink-noise/

    inline int32_t getPink(void)
    {
        constexpr int g = -1;
        constexpr int p = 9;
        const int32_t yn1 = state.s[0][0] + state.s[0][1] + state.s[0][2] + state.s[0][3] + state.s[1][0];
        // 14.9358 Hz
        state.s[0][0] += ( galoisShifter.getWhiteRaw() >> ( p - 0 + 0 + g ) ) - ( state.s[0][0]>>(p-0) );
        // 59.9192 Hz
        state.s[0][1] += ( galoisShifter.getWhiteRaw() >> ( p - 2 + 1 + g ) ) - ( state.s[0][1]>>(p-2) );
        // 242.549 Hz
        state.s[0][2] += ( galoisShifter.getWhiteRaw() >> ( p - 4 + 2 + g ) ) - ( state.s[0][2]>>(p-4) );
        // 1020.13 Hz
        state.s[0][3] += ( galoisShifter.getWhiteRaw() >> ( p - 6 + 3 + g ) ) - ( state.s[0][3]>>(p-6) );
        // 5295.41 Hz
        state.s[1][0] += ( galoisShifter.getWhiteRaw() >> ( p - 8 + 4 + g ) ) - ( state.s[1][0]>>(p-8) );
        const int32_t yn2 = state.s[0][0] + state.s[0][1] + state.s[0][2] + state.s[0][3] + state.s[1][0];
        const int32_t yn3 = state.s[1][1];
        // dc cut
        state.s[1][1] += ( yn1 - yn2 ) - ( state.s[1][1]>>(p+2) );
        // zero gain at fs/2
        return state.s[1][1] + yn3;
    };

    inline int32_t getPinkLow(void)
    {
        constexpr int g = -4;
        constexpr int p = 15;
        const int32_t yn1 = state.s[0][0] + state.s[0][1] + state.s[0][2] + state.s[0][3] + state.s[1][0];
        state.s[0][0] += ( galoisShifter.getWhiteRaw() >> ( p - 0 + 0 + g ) ) - ( state.s[0][0]>>(p-0) );
        state.s[0][1] += ( galoisShifter.getWhiteRaw() >> ( p - 2 + 1 + g ) ) - ( state.s[0][1]>>(p-2) );
        state.s[0][2] += ( galoisShifter.getWhiteRaw() >> ( p - 4 + 2 + g ) ) - ( state.s[0][2]>>(p-4) );
        state.s[0][3] += ( galoisShifter.getWhiteRaw() >> ( p - 6 + 3 + g ) ) - ( state.s[0][3]>>(p-6) );
        state.s[1][0] += ( galoisShifter.getWhiteRaw() >> ( p - 8 + 4 + g ) ) - ( state.s[1][0]>>(p-8) );
        const int32_t yn2 = state.s[0][0] + state.s[0][1] + state.s[0][2] + state.s[0][3] + state.s[1][0];
        // dc cut
        const int32_t yn3 = state.s[1][1];
        state.s[1][1] += ( yn1 - yn2 ) - ( state.s[1][1]>>(p+2) );
        // zero gain at fs/2
        return yn3 + state.s[1][1];
    };

    // p = 9..15
    // g = 0..-4
    inline int32_t getPink( const int p,  const int g )
    {
        const int32_t yn1 = state.s[0][0] + state.s[0][1] + state.s[0][2] + state.s[0][3] + state.s[1][0];
        state.s[0][0] += ( galoisShifter.getWhiteRaw() >> ( p - 0 + 0 + g ) ) - ( state.s[0][0]>>(p-0) );
        state.s[0][1] += ( galoisShifter.getWhiteRaw() >> ( p - 2 + 1 + g ) ) - ( state.s[0][1]>>(p-2) );
        state.s[0][2] += ( galoisShifter.getWhiteRaw() >> ( p - 4 + 2 + g ) ) - ( state.s[0][2]>>(p-4) );
        state.s[0][3] += ( galoisShifter.getWhiteRaw() >> ( p - 6 + 3 + g ) ) - ( state.s[0][3]>>(p-6) );
        state.s[1][0] += ( galoisShifter.getWhiteRaw() >> ( p - 8 + 4 + g ) ) - ( state.s[1][0]>>(p-8) );
        const int32_t yn2 = state.s[0][0] + state.s[0][1] + state.s[0][2] + state.s[0][3] + state.s[1][0];
        // dc cut
        const int32_t yn3 = state.s[1][1];
        state.s[1][1] += ( yn1 - yn2 ) - ( state.s[1][1]>>(p+2) );
        // zero gain at fs/2
        return yn3 + state.s[1][1];
    };



    inline int32_t getColor( const int64_t f )
    {
        const int32_t x0 = state.s[0][0];
        state.s[0][0] = galoisShifter.getWhiteRaw() >> 5;


        const int32_t xin = x0 + state.s[0][0] - ( state.s[1][0] << 2 );
        state.s[0][1] = ((( state.s[0][1] - xin ) * f ) >> 32 ) + xin;
        state.s[0][2] = ((( state.s[0][2] - state.s[0][1] ) * f ) >> 32 ) + state.s[0][1];
        state.s[0][3] = ((( state.s[0][3] - state.s[0][2] ) * f ) >> 32 ) + state.s[0][2];
        const int64_t sr = ( state.s[1][0] - state.s[0][3] ) * f;
        state.s[1][0] = ( sr >> 32 ) + state.s[0][3];
        return  sr >> 28;
    };

    // shifter forwarding
    inline void inc(void)
    {
        galoisShifter.inc();
    };
    inline uint64_t getState(void)
    {
        return galoisShifter.getState();
    };
    inline uint64_t get(void)
    {
        return galoisShifter.get();
    };

    inline int32_t getWhiteRaw(void)
    {
        return galoisShifter.getWhiteRaw();
    };

private:
    State           state;
    GaloisShifter&  galoisShifter;
};
#endif
// --------------------------------------------------------------------
} // end namespace noiser

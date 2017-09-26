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
 * File:   Ebuffer.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on March 4, 2016, 8:14 PM
 */
#include   "v4.h"

#include    <array>
#include    <cstring>
#include    <iostream>
// #include    <ieee754.h>

namespace yacynth {
// --------------------------------------------------------------------
// manipulate the floats
union floatexp {
    float       f;
    uint32_t    i;
    struct __attribute__((packed)) {
        uint32_t mantissa   : 23;
        uint32_t exponent   : 8;
        uint32_t sign       : 1;
    };
};

// to kill small numbers
inline float noisefloor( float val )
{
    return (floatexp{val}).i & 0x60000000 ? val : 0.0f; // ca. 1e-19 .... highest 2 bits of exponent zero
}

// --------------------------------------------------------------------
//
// secSize = internal buffer size (64)
// sectionCount generally 1 but for short and long delay lines can be anything
// MONO
//  channel[chA]
//
//
// Effect stage main parameters
//
// frame size: the best is if == to the oscillator oscillatorOutSampleCountExp
//  1<<EffectFrameSizeExp
//
// possible oversampling rate 1<<EffectOversamplingRateExp
//
// --------------------------------------------------------------------

struct EbufferPar {
    static constexpr std::size_t EffectFrameSizeExp          = 6;
    static constexpr std::size_t EffectOversamplingRateExp   = 0;
    static constexpr std::size_t sectionSizeExp     = EffectFrameSizeExp+EffectOversamplingRateExp;
    static constexpr std::size_t sectionSize        = 1<<sectionSizeExp;
    static constexpr std::size_t sectionSizeMask    = sectionSize-1;
    static constexpr std::size_t vsectionSizeExp    = sectionSizeExp-2;
    static constexpr std::size_t vsectionSize       = 1<<vsectionSizeExp;
    static constexpr std::size_t vsectionSizeMask   = vsectionSize-1;

    static constexpr std::size_t chA                = 0;
    static constexpr std::size_t chB                = 1;
};

// --------------------------------------------------------------------
// TODO > fadev4Add with ( float * inCH0 float * inCH1 .... )
// TODO > multAdd with ( float * inCH0 float * inCH1 .... )
// --------------------------------------------------------------------
// Effect stage IO buffer
struct FadeBufferTag { FadeBufferTag(int){} };

struct EIObuffer : public EbufferPar {
    static constexpr std::size_t channelCount       = 2;

    EIObuffer()
    { clear(); };

    // fast hack to make fade buffer
    EIObuffer( const FadeBufferTag )
    { makeFadeBuffer(); };

    void clear(void)
    {
        for( auto& ichn : channel ) memset( ichn, 0, sectionSize*sizeof(float) );
    }

    // chA fadeIn
    // chB fadeOut
    void makeFadeBuffer(void)
    {
        for( auto i=0u; i<sectionSize; ++i ) {
            const float x = float(i) / float(sectionSize);
            channel[chA][i] = x;
            channel[chB][sectionSize-1-i] = x;
        }
    }

    inline void copyMono2Stereo( int32_t * inp )
    {
        for( auto i=0u; i < sectionSize; ++i ) {
            channel[chB][i] = channel[chA][i] = static_cast<float>(*inp++);
        }
    }

    inline void copyMono2Stereo( int32_t * inp, float gain )
    {
        for( auto i=0u; i < sectionSize; ++i ) {
            channel[chB][i] = channel[chA][i] = static_cast<float>(*inp++) * gain;
        }
    }

    inline void copyMono2Stereo( int64_t * inp )
    {
        for( auto i=0u; i < sectionSize; ++i ) {
            channel[chB][i] = channel[chA][i] = static_cast<float>(*inp++);
        }
    }

    inline void copyMono2Stereo( int64_t * inp, float gain )
    {
        for( auto i=0u; i < sectionSize; ++i ) {
            channel[chB][i] = channel[chA][i] = static_cast<float>(*inp++) * gain;
        }
    }

    inline void copyMono2Stereo( const EIObuffer& inp )
    {
        for( auto i=0u; i < vsectionSize; ++i ) {
            vchannel[chB][i] = vchannel[chA][i] = inp.vchannel[chA][i];
        }
    }

    // do vectorizing - check
    inline void copy( const EIObuffer& inp )
    {
        for( auto i=0u; i < vsectionSize; ++i ) {
            vchannel[chA][i] = inp.vchannel[chA][i];
            vchannel[chB][i] = inp.vchannel[chB][i];
        }
    }

    template< std::size_t CH >
    inline void copyCH( const EIObuffer& inp )
    {
        for( auto i=0u; i < vsectionSize; ++i ) {
            vchannel[CH][i] = inp.vchannel[CH][i];
        }
    }

    inline void add( const EIObuffer& inp )
    {
        for( auto i=0u; i < vsectionSize; ++i ) {
            vchannel[chA][i] += inp.vchannel[chA][i];
            vchannel[chB][i] += inp.vchannel[chB][i];
        }
    }

    inline void add( const EIObuffer& inp1, const EIObuffer& inp2  )
    {
        for( auto i=0u; i < vsectionSize; ++i ) {
            vchannel[chA][i] = inp1.vchannel[chA][i] + inp2.vchannel[chA][i];
            vchannel[chB][i] = inp1.vchannel[chB][i] + inp2.vchannel[chB][i];
        }
    }

    inline void mult( const EIObuffer& inp, float gain )
    {
        for( auto i=0u; i < vsectionSize; ++i ) {
            vchannel[chA][i] = inp.vchannel[chA][i] * gain;
            vchannel[chB][i] = inp.vchannel[chB][i] * gain;
        }
    }

    inline void mult( const EIObuffer& inp, float gain0, float gain1 )
    {
        for( auto i=0u; i < vsectionSize; ++i ) {
            vchannel[chA][i] = inp.vchannel[chA][i] * gain0;
            vchannel[chB][i] = inp.vchannel[chB][i] * gain1;
        }
    }
    
    inline void multSet( const EIObuffer& inp, float gain0, float gain1 )
    {
        for( auto i=0u; i < vsectionSize; ++i ) {
            vchannel[ chA ][ i ] = inp.vchannel[ chA ][ i ] * gain0;
            vchannel[ chB ][ i ] = inp.vchannel[ chB ][ i ] * gain1;
        }
    }
    
    inline void mult( const EIObuffer& inp0, const EIObuffer& inp1,
                      float gain0, float gain1 )
    {
        for( auto i=0u; i < vsectionSize; ++i ) {
            vchannel[chA][i] = inp0.vchannel[chA][i] * gain0 + inp1.vchannel[chA][i] * gain1;
        }
        for( auto i=0u; i < vsectionSize; ++i ) {
            vchannel[chB][i] = inp0.vchannel[chB][i] * gain0 + inp1.vchannel[chB][i] * gain1;
        }
    }

    inline void mult( const EIObuffer& inp0, const EIObuffer& inp1, const EIObuffer& inp2,
                      float gain0, float gain1, float gain2 )
    {
        for( auto i=0u; i < vsectionSize; ++i ) {
            vchannel[chA][i] = inp0.vchannel[chA][i] * gain0
                    + inp1.vchannel[chA][i] * gain1
                    + inp2.vchannel[chA][i] * gain2;
        }
        for( auto i=0u; i < vsectionSize; ++i ) {
            vchannel[chB][i] = inp0.vchannel[chB][i] * gain0
                    + inp1.vchannel[chB][i] * gain1
                    + inp2.vchannel[chB][i] * gain2;
        }
    }

    inline void mult( const EIObuffer& inp0, const EIObuffer& inp1, const EIObuffer& inp2, const EIObuffer& inp3,
                      float gain0, float gain1, float gain2, float gain3 )
    {
        for( auto i=0u; i < vsectionSize; ++i ) {
            vchannel[chA][i] = inp0.vchannel[chA][i] * gain0
                    + inp1.vchannel[chA][i] * gain1
                    + inp2.vchannel[chA][i] * gain2;
                    + inp3.vchannel[chA][i] * gain3;
        }
        for( auto i=0u; i < vsectionSize; ++i ) {
            vchannel[chB][i] = inp0.vchannel[chB][i] * gain0
                    + inp1.vchannel[chB][i] * gain1
                    + inp2.vchannel[chB][i] * gain2;
                    + inp3.vchannel[chB][i] * gain3;
        }
    }

    inline void mult( float gain )
    {
        for( auto i=0u; i < vsectionSize; ++i ) {
            vchannel[chA][i] *= gain;
            vchannel[chB][i] *= gain;
        }
    }

    inline void multAdd( float gain, const EIObuffer& inp0 )
    {
        for( auto i=0u; i < vsectionSize; ++i ) {
            vchannel[chA][i] = vchannel[chA][i] * gain + inp0.vchannel[chA][i];
            vchannel[chB][i] = vchannel[chB][i] * gain + inp0.vchannel[chB][i];
        }
    }

    //
    // y = k * y + (1-k) * x = k * ( x - y ) + x
    //
    inline void wetDryBalance( const EIObuffer& inp0, float gain )
    {
        for( auto i=0u; i < vsectionSize; ++i ) {
            vchannel[chA][i] = ( vchannel[chA][i] - inp0.vchannel[chA][i] )  * gain + inp0.vchannel[chA][i];
            vchannel[chB][i] = ( vchannel[chB][i] - inp0.vchannel[chB][i] )  * gain + inp0.vchannel[chB][i];
        }
    }

    inline void fade( const EIObuffer& inp, float& gain, float dgain )
    {
        static constexpr float fadeGain     =  (1.0f/(1<<6));
        dgain *= fadeGain;
        float gaini = gain;
        for( auto i=0u; i < sectionSize; ++i ) {
            gain += dgain;
            channel[chA][i] = inp.channel[chA][i] * gain;
            channel[chB][i] = inp.channel[chB][i] * gain;
        }
        gain = gaini;
    }

    inline void fadeV4( const EIObuffer& inp, float& gain, float dgain )
    {
        constexpr float fadeGainV4   =  (1.0f/(1<<4));
        dgain *= fadeGainV4;
        float gaini = gain;
        for( auto i=0u; i < vsectionSize; ++i ) {
            gaini += dgain;
            vchannel[chA][i] = inp.vchannel[chA][i] * gain;
            vchannel[chB][i] = inp.vchannel[chB][i] * gain;
        }
        gain = gaini;
    }

    inline void fadeAdd( const EIObuffer& inp, float& gain0, float& gain1, float dgain0, float dgain1 )
    {
        constexpr float fadeGainV4   =  (1.0f/(1<<6));
        dgain0 *= fadeGainV4;
        dgain1 *= fadeGainV4;
        float gaintmp0 = gain0;
        for( auto i=0u; i < sectionSize; ++i ) {
            gaintmp0 += dgain0;
            channel[chA][i] = inp.channel[chA][i] * gaintmp0;
        }
        float gaintmp1 = gain1;
        for( auto i=0u; i < sectionSize; ++i ) {
            gaintmp1 += dgain1;
            channel[chB][i] = inp.channel[chB][i] * gaintmp1;
        }
        gain0 = gaintmp0;
        gain1 = gaintmp1;
    }
    
    inline void fadeAddV4( const EIObuffer& inp, float& gain0, float& gain1, float dgain0, float dgain1 )
    {
        constexpr float fadeGainV4   =  (1.0f/(1<<4));
        dgain0 *= fadeGainV4;
        dgain1 *= fadeGainV4;
        float gaintmp0 = gain0;
        for( auto i=0u; i < vsectionSize; ++i ) {
            gaintmp0 += dgain0;
            vchannel[chA][i] = inp.vchannel[chA][i] * gaintmp0;
        }
        float gaintmp1 = gain1;
        for( auto i=0u; i < vsectionSize; ++i ) {
            gaintmp1 += dgain1;
            vchannel[chB][i] = inp.vchannel[chB][i] * gaintmp1;
        }
        gain0 = gaintmp0;
        gain1 = gaintmp1;
    }
        
    inline void mult( const EIObuffer& inp1, const EIObuffer& inp2  )
    {
        for( auto i=0u; i < vsectionSize; ++i ) {
            vchannel[chA][i] = inp1.vchannel[chA][i] * inp2.vchannel[chA][i];
            vchannel[chB][i] = inp1.vchannel[chB][i] * inp2.vchannel[chB][i];
        }
    }

    inline void mult( const EIObuffer& inp ) {
        for( auto i=0u; i < vsectionSize; ++i ) {
            vchannel[chA][i] *= inp.vchannel[chA][i];
            vchannel[chB][i] *= inp.vchannel[chB][i];
        }
    }

    // AM == inp1 * inp2 * k1 + inp1 * k2


    inline void mult( const v4sf * inp ) {
        for( auto i=0u; i < vsectionSize; ++i ) {
            vchannel[chA][i] *= inp[i];
            vchannel[chB][i] *= inp[i];
        }
    }


    inline void mult( const v4sf * inpA, const v4sf * inpB ) {
        for( auto i=0u; i < vsectionSize; ++i ) {
            vchannel[chA][i] *= inpA[i];
            vchannel[chB][i] *= inpB[i];
        }
    }

    void dump( float * chnA, float * chnB ) const
    {
        const float * cA = channel[chA];
        for( auto i = 0u; i < sectionSize; ++i ) {
            *chnA++ = *cA++;
        }
        const float * cB = channel[chB];
        for( auto i = 0u; i < sectionSize; ++i ) {
            *chnB++ = *cB++;
        }
    }

    void dump( float * chnA, float * chnB, float kA, float kB ) const
    {
        const float * cA = channel[chA];
        for( auto i = 0u; i < sectionSize; ++i ) {
            *chnA++ = *cA++ * kA;
        }
        const float * cB = channel[chB];
        for( auto i = 0u; i < sectionSize; ++i ) {
            *chnB++ = *cB++ * kB;
        }
    }

    inline void fadeOut( const EIObuffer& inp );
    inline void fadeIn(  const EIObuffer& inp );
    inline void fadeOut( void );
    inline void fadeIn(  void );

    union alignas(16) {
        v4sf    vchannel[ channelCount ][ vsectionSize ];
        float   channel[ channelCount ][ sectionSize ];
    };
};
// --------------------------------------------------------------------
extern const EIObuffer nullEBuffer;
extern const EIObuffer fadeEBuffer;
// --------------------------------------------------------------------
inline void EIObuffer::fadeIn( const EIObuffer& inp )
{
    for( auto i=0u; i < vsectionSize; ++i ) {
        vchannel[chA][i] = inp.vchannel[chA][i] * fadeEBuffer.vchannel[chA][i]
                + vchannel[chA][i] * fadeEBuffer.vchannel[chB][i];
        vchannel[chB][i] = inp.vchannel[chB][i] * fadeEBuffer.vchannel[chA][i]
                + vchannel[chB][i] * fadeEBuffer.vchannel[chB][i];
    }
}

inline void EIObuffer::fadeIn(void)
{
    for( auto i=0u; i < vsectionSize; ++i ) {
        vchannel[chA][i] *= fadeEBuffer.vchannel[chA][i];
        vchannel[chB][i] *= fadeEBuffer.vchannel[chA][i];
    }
}

inline void EIObuffer::fadeOut( const EIObuffer& inp )
{
    for( auto i=0u; i < vsectionSize; ++i ) {
        vchannel[chA][i] = inp.vchannel[chA][i] * fadeEBuffer.vchannel[chB][i]
                + vchannel[chA][i] * fadeEBuffer.vchannel[chA][i];
        vchannel[chB][i] = inp.vchannel[chB][i] * fadeEBuffer.vchannel[chB][i]
                + vchannel[chB][i] * fadeEBuffer.vchannel[chA][i];
    }
}
inline void EIObuffer::fadeOut(void)
{
    for( auto i=0u; i < vsectionSize; ++i ) {
        vchannel[chA][i] *= fadeEBuffer.vchannel[chB][i];
        vchannel[chB][i] *= fadeEBuffer.vchannel[chB][i];
    }
}

// --------------------------------------------------------------------
// --- EDelayLine -----------------------------------------------------
// --------------------------------------------------------------------

struct EDelayLine : public EbufferPar {
    static constexpr std::size_t channelCount = 2;
    typedef std::array<float,channelCount> elementSetType;
    //template< std::size_t N > std::array<uint32_t, N> DelayIndex;

    EDelayLine( std::size_t sectionCountExp ) // section count MUST BE 1<<k
    :   index(0)
    ,   bufferSizeExp( sectionCountExp + sectionSizeExp )
    ,   bufferSize( 1L << ( sectionCountExp + sectionSizeExp) )
    ,   bufferSizeMask(bufferSize-1)
    {
        for( auto& ichn : channel ) ichn = (float *) new v4sf[ bufferSize >> 2 ]; // v4sf = 4 float
        for( int i=0; i<channelCount; ++i ) {
            if( (uint64_t) channel[i] & 0xF ) {
                std::cerr << "illegal alignment channel:" << channel[i] << std::endl;
            }
        }

        clear();
    };

    EDelayLine() = delete;

    ~EDelayLine()
    {
//        for( auto& ichn : channel ) delete[] ichn;
// channel[0]
        for( auto& ichn : channel ) delete  ichn;
    };

    void clear(void)
    {
        for( auto& ichn : channel ) memset( ichn, 0, bufferSize*sizeof(float) );
        index = 0;
    };
    // ----------------------------------------------------------------------

    inline void push( const float valA, const float valB )
    {
        channel[chA][ index ] = valA;
        channel[chB][ index ] = valB;
        index = ( ++index ) & bufferSizeMask;
    };
    inline void push( const float valA )
    {
        channel[chA][ index ] = valA;
        index = ( ++index ) & bufferSizeMask;
    };
    //       { return index & bufferSizeMask & ( ~sectionSizeMask ); };

    inline void pushSection( const float * valA, const float * valB )
    {
        auto * pA = &channel[chA][ ( index & ~sectionSizeMask ) & bufferSizeMask ];
        auto * pB = &channel[chB][ ( index & ~sectionSizeMask ) & bufferSizeMask ];
        for( uint32_t ind = 0u; ind < sectionSize; ++ind ) {
            *pA++ = *valA++;
            *pB++ = *valB++;
        }
        incIndexBySectionSize();
    };

    inline void pushSection( const EIObuffer& ebuf )
    {
        const v4sf * iA = ebuf.vchannel[ chA ];
        const v4sf * iB = ebuf.vchannel[ chB ];
        v4sf * pA = (v4sf *) &channel[chA][ ( index & ~sectionSizeMask ) & bufferSizeMask ];
        v4sf * pB = (v4sf *) &channel[chB][ ( index & ~sectionSizeMask ) & bufferSizeMask ];

        for( auto ind = 0u; ind < vsectionSize; ++ind ) {
            *pA++ = *iA++;
            *pB++ = *iB++;
        }
        incIndexBySectionSize();
    };

    inline void pushAndFeedbackSection( const EIObuffer& ebuf, uint32_t delay, float coeff )
    {
        const v4sf * iA = ebuf.vchannel[ chA ];
        const v4sf * iB = ebuf.vchannel[ chB ];
        v4sf * pA = (v4sf *) &channel[chA][ ( index & ~sectionSizeMask ) & bufferSizeMask ];
        v4sf * pB = (v4sf *) &channel[chB][ ( index & ~sectionSizeMask ) & bufferSizeMask ];

        for( auto ind = 0u; ind < vsectionSize; ++ind ) {
            *pA++ = *iA++;
            *pB++ = *iB++;
        }

        uint32_t cind = getIndex(delay);
        for( uint32_t ind = 0u; ind < sectionSize; ++ind ) {
            channel[ chA ][ index + ind ] += channel[ chA ][ cind ] * coeff;
            channel[ chB ][ index + ind ] += channel[ chB ][ cind ] * coeff;
            cind = ( ++cind ) & bufferSizeMask;
        }
        incIndexBySectionSize();
    };

    //
    // ----------------------------------------------------------------------
    // different utility functions
    // ----------------------------------------------------------------------

    // 2nd order Lagrange modified Farrow
    // bad http://users.spa.aalto.fi/vpv/publications/vesan_vaitos/ch3_pt2_lagrange.pdf page 101
    // ok http://seat.massey.ac.nz/personal/e.lai/Publications/c/ICASSP2011.pdf page 1630
    //
    // input: lindex -
    //  high 32 bit is the sample index
    //  low 32 bit is the interpolation value between 2 samples
    //
    template< std::size_t chN >
    inline float getInterpolated2Order( const uint64_t lindex ) const
    {
        static_assert( chN < channelCount, "channel count");
        constexpr float range = 1.0 / (1L<<32);
        const uint32_t  ind = ( lindex + 0x080000000LL )>>32;      // middle point -0.5..+0.5
        const float d = int32_t( lindex & 0x0FFFFFFFFLL ) * range;  // -0.5..+0.5
        const float sm1 = channel[ chN ][ ( index - ind - 1 ) & bufferSizeMask ] * 0.5f;
        const float s0  = channel[ chN ][ ( index - ind     ) & bufferSizeMask ];
        const float sp1 = channel[ chN ][ ( index - ind + 1 ) & bufferSizeMask ] * 0.5f;
        return (( sm1 + sp1 - s0 ) * d - sp1 + sm1 ) * d + s0;
    };

    // const float y0 = channel[chA][ indk0 ]
    // const float dy = y0 - channel[chA][ indk1 ]
    // return y0 + dy * ( lindex & 0x0FFFFFFFF ) * range;
    //
    // index: high 32 index, low 32 fraction - linear interpolation between the points i-1, i
    //
    template< std::size_t chN >
    inline float getInterpolated1Order( const uint64_t lindex ) const
    {
        static_assert( chN < channelCount, "channel count");
        constexpr float range   = 1.0 / (1L<<32);
        const uint32_t  ind     = ( index - ( lindex>>32 ) ) & bufferSizeMask;
        const float     d       = ( lindex & 0x0FFFFFFFFLL ) * range;
        return channel[chN][ ind ] * ( 1.0f - d ) + channel[chN][( ind-1 ) & bufferSizeMask ] * d;
    };

    // ----------------------------------------------------------------------
    // these are used by the Comb
    inline void multAdd( const uint32_t delay, const float mult, float& A, float& B ) const
    {
        const uint32_t cind = getIndex(delay);
        A += channel[chA][cind] * mult;
        B += channel[chB][cind] * mult;
    }

    template< std::size_t chN >
    inline float get( const uint32_t delay ) const
    {
        return channel[chN][getIndex(delay)];
    }

    inline void get( const uint32_t delay, float& A, float& B ) const
    {
        const uint32_t cind = getIndex(delay);
        A = channel[chA][cind];
        B = channel[chB][cind];
    }

    inline void feedbackSection(
        uint32_t delay,
        float  coeff
        )
    {
        uint32_t cind = getIndex(delay);
        for( uint32_t ind = 0u; ind < sectionSize; ++ind ) {
            channel[ chA ][ index + ind ] += channel[ chA ][ cind ] * coeff;
            channel[ chB ][ index + ind ] += channel[ chB ][ cind ] * coeff;
            cind = ( ++cind ) & bufferSizeMask;
        }
    }

    inline void feedbackSection(
        uint32_t delayA,
        uint32_t delayB,
        float  coeffA,
        float  coeffB
        )
    {
        uint32_t cindA = getIndex(delayA);
        uint32_t cindB = getIndex(delayB);
        for( uint32_t ind = 0u; ind < sectionSize; ++ind ) {
            channel[ chA ][ index + ind ] += channel[ chA ][ cindA ] * coeffA;
            channel[ chB ][ index + ind ] += channel[ chB ][ cindB ] * coeffB;
            cindA = ( ++cindA ) & bufferSizeMask;
            cindB = ( ++cindB ) & bufferSizeMask;
        }
    }
    // ----------------------------------------------------------------------
    // tapped delay line - TDL (FIR)
    //
    void fillTDLSection(
        uint32_t delayA,
        uint32_t delayB,
        float * chnA,
        float * chnB
        )
    {
        // might be better for cache
        uint32_t cindA = getIndex(delayA);
        for( uint32_t ind = 0u; ind < sectionSize; ++ind ) {
            *chnA++ = channel[chA][ cindA ];
            cindA = ( ++cindA ) & bufferSizeMask;
        }

        uint32_t cindB = getIndex(delayB);
        for( uint32_t ind = 0u; ind < sectionSize; ++ind ) {
            *chnB++ = channel[chB][ cindB ];
            cindB = ( ++cindB ) & bufferSizeMask;
        }
    }

    void fillTDLSection(
        uint32_t delayA,
        uint32_t delayB,
        float  coeffA,
        float  coeffB,
        float * chnA,
        float * chnB
        )
    {
        uint32_t cindA = getIndex(delayA);
        for( uint32_t ind = 0u; ind < sectionSize; ++ind ) {
            *chnA++ = channel[chA][ cindA ] * coeffA;
            cindA = ( ++cindA ) & bufferSizeMask;
        }

        uint32_t cindB = getIndex(delayB);
        for( uint32_t ind = 0u; ind < sectionSize; ++ind ) {
            *chnB++ = channel[chB][ cindB ] * coeffB;
            cindB = ( ++cindB ) & bufferSizeMask;
        }
    }

    void addTDLSection(
        uint32_t delayA,
        uint32_t delayB,
        float  coeffA,
        float  coeffB,
        float * chnA,
        float * chnB
        )
    {
        uint32_t cindA = getIndex(delayA);
        for( uint32_t ind = 0u; ind < sectionSize; ++ind ) {
            *chnA++ += channel[chA][ cindA ] * coeffA;
            cindA = ( ++cindA ) & bufferSizeMask;
        }

        uint32_t cindB = getIndex(delayB);
        for( uint32_t ind = 0u; ind < sectionSize; ++ind ) {
            *chnB++ += channel[chB][ cindB ] * coeffB;
            cindB = ( ++cindB ) & bufferSizeMask;
        }
    }

    // ----------------------------------------------------------------------

    inline uint32_t getIndex( void )  const
        { return index; };

    inline uint32_t getIndex( const uint32_t delay )  const
        { return ( index - delay ) & bufferSizeMask; };

    inline void incIndex(void)
        { index = (++index) & bufferSizeMask; };

    inline std::size_t getSectionIndex( const uint32_t sectionOffset ) const
        { return ( index - ( sectionOffset & (~sectionSizeMask) )) & bufferSizeMask; };

    inline std::size_t getSectionIndex() const
        { return index & bufferSizeMask & ( ~sectionSizeMask ); };

    inline void incIndexBySectionSize( void )
        {
            index = ( index + sectionSize ) & ( bufferSizeMask & ~sectionSizeMask );
        };

    // ----------------------------------------------------------------------

    float             * channel[channelCount];
    uint32_t            index;
    const uint32_t      bufferSizeExp;
    const uint32_t      bufferSize;
    const uint32_t      bufferSizeMask;
};

// --------------------------------------------------------------------
// --- EDelayLineArray ------------------------------------------------
// --------------------------------------------------------------------

template< std::size_t channelCount >
struct EDelayLineArray : public EbufferPar {
    static constexpr std::size_t V4channelCount = (channelCount+3)/4;

    typedef union alignas(16) {
        float   v[channelCount];
        v4sf    v4[V4channelCount];
    } PackedChannel;

    typedef std::array<uint32_t, channelCount> DelayIndex;

    EDelayLineArray( std::size_t sectionCountExp ) // section count MUST BE 1<<k
    :   bufferSizeExp( sectionCountExp + sectionSizeExp )
    ,   bufferSize( 1L << ( sectionCountExp + sectionSizeExp) )
    ,   bufferSizeMask(bufferSize-1)
    {
        // debug
        for( int i=0; i<channelCount; ++i ) {
            channel[ i ] = (float *) new v4sf[ bufferSize >> 2 ]; // v4sf = 4 float
            if( (uint64_t) channel[i] & 0xF ) {
                std::cerr << "illegal alignment channel:" << channel[i] << std::endl;
            }
        }
        clear();
    };

    EDelayLineArray() = delete;

    void clear(void)
    {
        for( int i=0; i<channelCount; ++i ) {
            memset( channel[ i ], 0, bufferSize*sizeof(float) );
        }
        index = 0;
    };

    ~EDelayLineArray()
    {
        for( int i=0; i<channelCount; ++i ) {
            v4sf *p = (v4sf *) channel[ i ];
            delete[] p;
        }
    };

    inline void push( const PackedChannel& p )
    {
        for( uint32_t i=0; i<channelCount; ++i )
            channel[ i ][ index ] = p.v[i];
        incIndex();
    };

    inline void get( const DelayIndex ind, PackedChannel& p ) const
    {
        for( uint32_t i=0; i<channelCount; ++i )
             p.v[i] = channel[ i ][ getIndex( ind[i] ) ];
    };

    // get: 0->1.... n-2->n-1 n-1->0
    inline void getRotate( const DelayIndex ind, PackedChannel& p ) const
    {
        static_assert( channelCount>2," channel count min 3" );
        for( uint32_t i=1; i<channelCount; ++i )
             p.v[i-1] = channel[ i ][ getIndex( ind[i] ) ];
        p.v[channelCount-1] = channel[ 0 ][ getIndex( ind[0] ) ];
    };

    // 2nd order Lagrange modified Farrow
    // bad http://users.spa.aalto.fi/vpv/publications/vesan_vaitos/ch3_pt2_lagrange.pdf page 101
    // ok http://seat.massey.ac.nz/personal/e.lai/Publications/c/ICASSP2011.pdf page 1630
    //
    // input: lindex -
    //  high 32 bit is the sample index
    //  low 32 bit is the interpolation value between 2 samples
    //
    template< std::size_t chN >
    inline float getInterpolated2Order( const uint64_t lindex ) const
    {
        static_assert( chN < channelCount, "channel count");
        constexpr float range = 1.0 / (1L<<32);
        const uint32_t  ind = ( lindex + 0x080000000LL )>>32;      // middle point -0.5..+0.5
        const float d = int32_t( lindex & 0x0FFFFFFFFLL ) * range;  // -0.5..+0.5
        const float sm1 = channel[ chN ][ ( index - ind - 1 ) & bufferSizeMask ] * 0.5f;
        const float s0  = channel[ chN ][ ( index - ind     ) & bufferSizeMask ];
        const float sp1 = channel[ chN ][ ( index - ind + 1 ) & bufferSizeMask ] * 0.5f;
        return (( sm1 + sp1 - s0 ) * d - sp1 + sm1 ) * d + s0;
    };

    // const float y0 = channel[chA][ indk0 ]
    // const float dy = y0 - channel[chA][ indk1 ]
    // return y0 + dy * ( lindex & 0x0FFFFFFFF ) * range;
    //
    // index: high 32 index, low 32 fraction - linear interpolation between the points i-1, i
    //
    template< std::size_t chN >
    inline float getInterpolated1Order( const uint64_t lindex ) const
    {
        static_assert( chN < channelCount, "channel count");
        constexpr float range   = 1.0 / (1L<<32);
        const uint32_t  ind     = ( index - ( lindex>>32 ) ) & bufferSizeMask;
        const float     d       = ( lindex & 0x0FFFFFFFFLL ) * range;
        return channel[chN][ ind ] * ( 1.0f - d ) + channel[chN][( ind-1 ) & bufferSizeMask ] * d;
    };

    inline uint32_t getIndex( void )  const
        { return index; };

    inline uint32_t getIndex( const uint32_t delay )  const
        { return ( index - delay ) & bufferSizeMask; };

    inline void incIndex(void)
        { index = (++index) & bufferSizeMask; };

    inline std::size_t getSectionIndex( const uint32_t sectionOffset ) const
        { return ( index - ( sectionOffset & (~sectionSizeMask) )) & bufferSizeMask; };

    inline void incIndexBySectionSize( void )
        { index = ( index + sectionSize ) & ( bufferSizeMask & ~sectionSizeMask ); };

    // ----------------------------------------------------------------------

    float             * channel[channelCount];
    uint32_t            index;
    const uint32_t      bufferSizeExp;
    const uint32_t      bufferSize;
    const uint32_t      bufferSizeMask;
};

// --------------------------------------------------------------------
} // end namespace yacynth


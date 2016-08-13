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

namespace yacynth {
// --------------------------------------------------------------------
// manipulate the floats
union   floatexp {
    float   inFl;
    struct {
        uint32_t mantisa    : 23;
        uint32_t exponent   : 8;
        uint32_t sign       : 1;
    };
};

template< int level >
inline float noisefloor( const float val )
{
    const floatexp  fexpv { .inFl = val };
    return fexpv.exponent < (127+level) ? 0.0f : val;    // 2^-27
}
// --------------------------------------------------------------------

//
// secSize = internal buffer size (64)
// sectionCount generally 1 but for short and long delay lines can be anything
// MONO
//  channelA
//
// STEREOLR
//  left    = channelA
//  right   = channelB
//
// STEREOMS
//  middle  = channelA
//  side    = channelB
//
// CH4
//  not planned seriously yet
//

//  #define CH4VERSION  1

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

#ifdef CH4VERSION
    static constexpr std::size_t channelCount       = 4;
    static constexpr std::size_t chC      = 2;
    static constexpr std::size_t chD      = 3;
#else
    static constexpr std::size_t channelCount       = 2;
#endif

};

// --------------------------------------------------------------------
// Effect stage IO buffer
struct FadeBufferTag {
    FadeBufferTag() = default;
};

struct EIObuffer : public EbufferPar {

    EIObuffer()
    { clear(); };

    // fast hack to make fade buffer
    EIObuffer( const FadeBufferTag * x)
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
    inline void mult( float gain )
    {
        for( auto i=0u; i < vsectionSize; ++i ) {
            vchannel[chA][i] *= gain;
            vchannel[chB][i] *= gain;
        }
    }

    inline void fade( const EIObuffer& inp, float& gain, float dgain )
    {
        for( auto i=0u; i < sectionSize; ++i ) {
            gain += dgain;
            channel[chA][i] = inp.channel[chA][i] * gain;
            channel[chB][i] = inp.channel[chB][i] * gain;
        }
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
    inline void mult( const EIObuffer& inp1, const EIObuffer& inp2, float k1, float k2 ) {
        for( auto i=0u; i < vsectionSize; ++i ) {
            vchannel[chA][i] =
                inp1.vchannel[chA][i] * inp2.vchannel[chA][i] * k1 + inp1.vchannel[chA][i] * k2;
            vchannel[chB][i] =
                inp1.vchannel[chB][i] * inp2.vchannel[chB][i] * k1 + inp1.vchannel[chB][i] * k2;
        }
    }

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
        for( auto i = 0u; i < sectionSize; ++i ) {
            *chnA++ = channel[chA][i];
            *chnB++ = channel[chB][i];
        }
    }

    void dump( float * chnA, float * chnB, float kA, float kB ) const
    {
        for( auto i = 0u; i < sectionSize; ++i ) {
            *chnA++ = channel[chA][i] * kA;
            *chnB++ = channel[chB][i] * kB;
        }
    }

    inline void fadeOut( const EIObuffer& inp );
    inline void fadeIn(  const EIObuffer& inp );
    inline void fadeOut( void );
    inline void fadeIn(  void );

    union alignas(16) {
        v4sf    vchannel[ channelCount ][ vsectionSize ];
        float   channel[ channelCount ][ sectionSize ];
        struct {
            float  channelA[ sectionSize ];
            float  channelB[ sectionSize ];
#ifdef CH4VERSION
            float  channelC[ sectionSize ];
            float  channelD[ sectionSize ];
#endif
        };
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
struct EDelayLine : public EbufferPar {
    EDelayLine( std::size_t sectionCountExp ) // section count MUST BE 1<<k
    :   index(0)
    ,   bufferSizeExp( sectionCountExp + sectionSizeExp )
    ,   bufferSize( 1L << ( sectionCountExp + sectionSizeExp) )
    ,   bufferSizeMask(bufferSize-1)
    {
        for( auto& ichn : channel ) ichn = new float [ bufferSize ];
        clear();
    };

    EDelayLine() = delete;

    ~EDelayLine()
    {
        for( auto& ichn : channel ) delete[] ichn;
    };

    void clear(void)
    {
        for( auto& ichn : channel ) memset( ichn, 0, bufferSize*sizeof(float) );
        index = 0;
    };
    // ----------------------------------------------------------------------

    inline void push( const float valA, const float valB )
    {
        channelA[ index ] = valA;
        channelB[ index ] = valB;
        index = ( ++index ) & bufferSizeMask;
    };
    inline void push( const float valA )
    {
        channelA[ index ] = valA;
        index = ( ++index ) & bufferSizeMask;
    };
    inline void pushSection( const float * valA, const float * valB )
    {
        auto * pA = &channelA[ ( index & ~sectionSizeMask ) & bufferSizeMask ];
        auto * pB = &channelB[ ( index & ~sectionSizeMask ) & bufferSizeMask ];
        for( uint32_t ind = 0u; ind < sectionSize; ++ind ) {
            *pA++ = *valA++;
            *pB++ = *valB++;
        }
        incIndexBySectionSize();
    };
    inline void pushBlock( const float * valA, float * valB )
    {
        for( uint32_t ind = 0u; ind < sectionSize; ++ind ) {
            channelA[ index ] = *valA++;
            channelB[ index ] = *valB++;
            index = ( ++index ) & bufferSizeMask;
        }
    };
    //
    // ----------------------------------------------------------------------
    // different utility functions - perhaps will be moved to an other class

    // ----------------------------------------------------------------------
    //
    // const float y0 = channelA[ indk0 ]
    // const float dy = y0 - channelA[ indk1 ]
    // return y0 + dy * ( lindex & 0x0FFFFFFFF ) * range;
    //
    // index: high 32 index, low 32 fraction - linear interpolation between the points i-1, i
    //
    inline float getInterpolatedA( const uint64_t lindexA ) const
    {
        constexpr float range       = 1.0 / (1L<<32);
        constexpr uint64_t Fmask    = 0x0FFFFFFFFLL;
        const uint32_t  indAk0      = ( index - ( lindexA>>32 ) ) & bufferSizeMask; // from the past
        const float     multAF0     = ( lindexA & Fmask ) * range;
        return channelA[ ( indAk0 - 1 ) & bufferSizeMask ] * multAF0 + channelA[ indAk0 ] * ( 1.0f - multAF0 );
    };

    inline float getInterpolatedB( const uint64_t lindexB ) const
    {
        constexpr float range       = 1.0 / (1L<<32);
        constexpr uint64_t Fmask    = 0x0FFFFFFFFLL;
        const uint32_t  indBk0      = ( index - ( lindexB>>32 ) ) & bufferSizeMask; // from the past
        const float     multBF0     = ( lindexB & Fmask ) * range;
        return channelB[ ( indBk0 - 1 ) & bufferSizeMask ] * multBF0 + channelB[ indBk0 ] * ( 1.0f - multBF0 );
    };

    // ---------------------------------------------------------------------

    // these are used by the Echo
    inline void multAddMixStereo( const uint32_t delay, const V4sfMatrix trm, float& A, float& B ) const
    {
        const uint32_t cind = getIndex(delay);
        A += channelA[cind] * trm.aa + channelB[cind] * trm.ab;
        B += channelA[cind] * trm.ba + channelB[cind] * trm.bb;
    }

    template< int Noisefloor >
    inline void multAddMixStereoNoisefloor( const uint32_t delay, const V4sfMatrix trm, float& A, float& B ) const
    {
        const uint32_t cind = getIndex(delay);
        const float inA = noisefloor<Noisefloor>(channelA[cind]);  // 2^-24
        const float inB = noisefloor<Noisefloor>(channelB[cind]);
        A += inA * trm.aa + inB * trm.ab;
        B += inA * trm.ba + inB * trm.bb;
    }
    // ----------------------------------------------------------------------
    // these are used by the Comb
    inline void multAdd( const uint32_t delay, const float mult, float& A, float& B ) const
    {
        const uint32_t cind = getIndex(delay);
        A += channelA[cind] * mult;
        B += channelB[cind] * mult;
    }
    inline void get( const uint32_t delay, float& A, float& B ) const
    {
        const uint32_t cind = getIndex(delay);
        A = channelA[cind];
        B = channelB[cind];
    }

    // lin interpolated delay line  used by the Comb
    inline void getInterpolatedMult( const uint64_t lindexA, const uint64_t lindexB, const float mult, float& A, float& B ) const
    {
        constexpr float range       = 1.0 / (1L<<32);
        constexpr uint64_t Fmask    = 0x0FFFFFFFFLL;
        const float     multR       = mult * range;
        const uint32_t  indAk0      = ( index - ( lindexA>>32 ) ) & bufferSizeMask;
        const float     multAF0     = ( lindexA & Fmask ) * multR;
        const uint32_t  indBk0      = ( index - ( lindexB>>32 ) ) & bufferSizeMask;
        const float     multBF0     = ( lindexB & Fmask ) * multR;
        A = channelA[ ( indAk0 - 1 ) & bufferSizeMask ] * multAF0 + channelA[ indAk0 ] * ( mult - multAF0 );
        B = channelB[ ( indBk0 - 1 ) & bufferSizeMask ] * multBF0 + channelB[ indBk0 ] * ( mult - multBF0 );
    };
    // lin interpolated delay line  used by the Comb
    inline void getInterpolated( const uint64_t lindexA, const uint64_t lindexB, float& A, float& B ) const
    {
        constexpr float range       = 1.0 / (1L<<32);
        constexpr uint64_t Fmask    = 0x0FFFFFFFFLL;
        const uint32_t  indAk0      = ( index - ( lindexA>>32 ) ) & bufferSizeMask;
        const float     multAF0     = ( lindexA & Fmask ) * range;
        const uint32_t  indBk0      = ( index - ( lindexB>>32 ) ) & bufferSizeMask;
        const float     multBF0     = ( lindexB & Fmask ) * range;
        A = channelA[ ( indAk0 - 1 ) & bufferSizeMask ] * multAF0 + channelA[ indAk0 ] * ( 1.0f - multAF0 );
        B = channelB[ ( indBk0 - 1 ) & bufferSizeMask ] * multBF0 + channelB[ indBk0 ] * ( 1.0f - multBF0 );
    };

    // ----------------------------------------------------------------------
    // mono stereo
    inline void combFeedbackA(
        const uint32_t  lindexA,
        const float     multA,
        const float     inA,
        float& outA  )
    {
        channelA[ index ] = outA = inA + channelA[ getIndex(lindexA) ] * multA;
        incIndex();
    };

    inline void combFeedbackStereo(
        const uint32_t  lindexA, const uint32_t  lindexB,
        const float multA, const float multB,
        const float inA, const float inB,
        float& outA, float& outB  )
    {
        channelA[ index ] = outA = inA + channelA[ getIndex(lindexA) ] * multA;
        channelB[ index ] = outB = inB + channelB[ getIndex(lindexB) ] * multB;
        incIndex();
    };
    inline void combFeedbackDelayA(
        const uint32_t  lindexA,
        const float     multA,
        const float     inA,
        float& outA  )
    {
        outA = channelA[ getIndex(lindexA) ];
        channelA[ index ] = inA + outA * multA;
        incIndex();
    };

    inline void combFeedbackDelayStereo(
        const uint32_t  lindexA, const uint32_t  lindexB,
        const float multA, const float multB,
        const float inA, const float inB,
        float& outA, float& outB  )
    {
        outA = channelA[ getIndex(lindexA) ];
        channelA[ index ] = inA + outA * multA;
        outB = channelA[ getIndex(lindexA) ];
        channelB[ index ] = inB + outB * multB;
        incIndex();
    };

    inline void combAllpassA(
        const uint32_t  lindexA,
        const float     multA,
        const float     inA,
        float& outA  )
    {
        const auto yA = channelA[ getIndex(lindexA) ];
        const auto xA = channelA[ index ] = inA - yA * multA;
        outA = yA + xA * multA;
        incIndex();
    };

    inline void combAllpassStereo(
        const uint32_t  lindexA, const uint32_t  lindexB,
        const float     multA, const float     multB,
        const float inA, const float inB,
        float& outA, float& outB  )
    {
        const auto yA = channelA[ getIndex(lindexA) ];
        const auto yB = channelB[ getIndex(lindexB) ];
        const auto xA = channelA[ index ] = inA - yA * multA;
        const auto xB = channelB[ index ] = inB - yB * multB;
        outA = yA + xA * multA;
        outB = yB + xB * multB;
        incIndex();
    };
    // ----------------------------------------------------------------------


    // ----------------------------------------------------------------------

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
    union {
        float * channel[ channelCount ];
        struct {
            // once channelA should be replaced by channel[Ebuffer::chA]
            // once channelB should be replaced by channel[Ebuffer::chB]
            float  * channelA;
            float  * channelB;
#ifdef CH4VERSION
            // this should not be used
            float        * channelC;
            float        * channelD;
#endif
        };
    };
    std::uint32_t       index;          // let suppose that it will be less than 4G float
    const std::size_t   bufferSizeExp;
    const std::size_t   bufferSize;
    const std::size_t   bufferSizeMask;
};

// --------------------------------------------------------------------
} // end namespace yacynth


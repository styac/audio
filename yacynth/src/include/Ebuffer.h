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

// remove template !
using EDataType  = float;
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
// this will be only float
template< typename Tstore >
struct EIObufferT : public EbufferPar {
    typedef Tstore value_type;
    EIObufferT()
    { clear(); };

    void clear(void)
    {
        for( auto& ichn : channel ) memset( ichn, 0, sectionSize*sizeof(Tstore) );
        channelGain = 1.0f;
    }

    inline void copyMono2Stereo( const EIObufferT& inp )
    {
        const auto * inpAp = inp.channel[EbufferPar::chA];
        auto * outAp = channel[EbufferPar::chA];
        auto * outBp = channel[EbufferPar::chB];
        for( auto i=0u; i < sectionSize; ++i ) {
            *outAp++ = *outBp++ = *inpAp++;
        }
    }

    inline void copy( const EIObufferT& inp )
    {
        // no aliasing -> __restrict__
        const auto * inpAp = inp.channel[EbufferPar::chA];
        const auto * inpBp = inp.channel[EbufferPar::chB];
        auto * outAp = channel[EbufferPar::chA];
        auto * outBp = channel[EbufferPar::chB];
        for( auto i=0u; i < sectionSize; ++i ) {
            *outAp++ = *inpAp++;
            *outBp++ = *inpBp++;
        }
    }

    inline void add( const EIObufferT& inp )
    {
        const auto * inpAp = inp.channel[EbufferPar::chA];
        const auto * inpBp = inp.channel[EbufferPar::chB];
        auto * outAp = channel[EbufferPar::chA];
        auto * outBp = channel[EbufferPar::chB];
        for( auto i=0u; i < sectionSize; ++i ) {
            *outAp++ += *inpAp++;
            *outBp++ += *inpBp++;
        }
    }

    inline void add( const EIObufferT& inp1, const EIObufferT& inp2  )
    {
        const auto * inp1Ap = inp1.channel[EbufferPar::chA];
        const auto * inp1Bp = inp1.channel[EbufferPar::chB];
        const auto * inp2Ap = inp2.channel[EbufferPar::chA];
        const auto * inp2Bp = inp2.channel[EbufferPar::chB];
        auto * outAp = channel[EbufferPar::chA];
        auto * outBp = channel[EbufferPar::chB];
        for( auto i=0u; i < sectionSize; ++i ) {
            *outAp++ += *inp1Ap++ + *inp2Ap++;
            *outBp++ += *inp1Bp++ + *inp2Bp++;
        }
    }

    inline void add( const EIObufferT& inp1, const EIObufferT& inp2, const EIObufferT& inp3  )
    {
        const auto * inp1Ap = inp1.channel[EbufferPar::chA];
        const auto * inp1Bp = inp1.channel[EbufferPar::chB];
        const auto * inp2Ap = inp2.channel[EbufferPar::chA];
        const auto * inp2Bp = inp2.channel[EbufferPar::chB];
        const auto * inp3Ap = inp3.channel[EbufferPar::chA];
        const auto * inp3Bp = inp3.channel[EbufferPar::chB];
        auto * outAp = channel[EbufferPar::chA];
        auto * outBp = channel[EbufferPar::chB];
        for( auto i=0u; i < sectionSize; ++i ) {
            *outAp++ += *inp1Ap++ + *inp2Ap++ + *inp3Ap++;
            *outBp++ += *inp1Bp++ + *inp2Bp++ + *inp3Bp++;
        }
    }

    inline void mult( const EIObufferT& inp );

    void dump( Tstore * chnA, Tstore * chnB ) const
    {
        for( auto i = 0u; i < sectionSize; ++i ) {
            *chnA++ = channel[EbufferPar::chA][i];
            *chnB++ = channel[EbufferPar::chB][i];
        }
    }

    inline void setGain( const float v )
    {
        channelGain = v;
    }

//    union {
    union alignas(16) {
        v4sf    vchannel[ channelCount ][ vsectionSize ];
        Tstore  channel[ channelCount ][ sectionSize ];
        struct {
            Tstore  channelA[ sectionSize ];
            Tstore  channelB[ sectionSize ];
#ifdef CH4VERSION
            Tstore  channelC[ sectionSize ];
            Tstore  channelD[ sectionSize ];
#endif
        };
    };
    float           channelGain;
};
// --------------------------------------------------------------------
template<> inline void EIObufferT<float>::mult( const EIObufferT& inp )
{
    for( auto i=0u; i < sectionSize; ++i ) {
        channel[EbufferPar::chA][i] *= inp.channel[EbufferPar::chA][i];
        channel[EbufferPar::chB][i] *= inp.channel[EbufferPar::chB][i];
    }
}
// --------------------------------------------------------------------
template<> inline void EIObufferT<int32_t>::mult( const EIObufferT& inp )
{
    for( auto i=0u; i < sectionSize; ++i ) {
        channel[EbufferPar::chA][i] = ( int64_t(inp.channel[EbufferPar::chA][i]) * channel[EbufferPar::chA][i] ) >> 32;
        channel[EbufferPar::chB][i] = ( int64_t(inp.channel[EbufferPar::chB][i]) * channel[EbufferPar::chB][i] ) >> 32;
    }
}
// --------------------------------------------------------------------
template< typename Tstore >
struct EDelayLineT : public EbufferPar {
    EDelayLineT( std::size_t sectionCountExp ) // section count MUST BE 1<<k
    :   index(0)
    ,   bufferSizeExp( sectionCountExp + sectionSizeExp )
    ,   bufferSize( 1L << ( sectionCountExp + sectionSizeExp) )
    ,   bufferSizeMask(bufferSize-1)
    {
        for( auto& ichn : channel ) ichn = new Tstore [ bufferSize ];
        clear();
    };

    EDelayLineT() = delete;

    ~EDelayLineT()
    {
        for( auto& ichn : channel ) delete[] ichn;
    };

    void clear(void)
    {
        for( auto& ichn : channel ) memset( ichn, 0, bufferSize*sizeof(Tstore) );
        index = 0;
    };
    // ----------------------------------------------------------------------

    inline void push( const Tstore valA, const Tstore valB )
    {
        channelA[ index ] = valA;
        channelB[ index ] = valB;
        index = ( ++index ) & bufferSizeMask;
    };
    inline void push( const Tstore valA )
    {
        channelA[ index ] = valA;
        index = ( ++index ) & bufferSizeMask;
    };
    inline void pushSection( const Tstore * valA, const Tstore * valB )
    {
        auto * pA = &channelA[ ( index & ~sectionSizeMask ) & bufferSizeMask ];
        auto * pB = &channelB[ ( index & ~sectionSizeMask ) & bufferSizeMask ];
        for( uint32_t ind = 0u; ind < sectionSize; ++ind ) {
            *pA++ = *valA++;
            *pB++ = *valB++;
        }
        incIndexBySectionSize();
    };
    inline void pushBlock( const Tstore * valA, Tstore * valB )
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
    inline void get( const uint32_t delay, Tstore& A, Tstore& B ) const
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
        Tstore * channel[ channelCount ];
        struct {
            // once channelA should be replaced by channel[Ebuffer::chA]
            // once channelB should be replaced by channel[Ebuffer::chB]
            Tstore  * channelA;
            Tstore  * channelB;
#ifdef CH4VERSION
            // this should not be used
            Tstore        * channelC;
            Tstore        * channelD;
#endif
        };
    };
    std::uint32_t       index;          // let suppose that it will be less than 4G float
    const std::size_t   bufferSizeExp;
    const std::size_t   bufferSize;
    const std::size_t   bufferSizeMask;
};

// --------------------------------------------------------------------
// --------------------------------------------------------------------
using EIObuffer         = EIObufferT<EDataType>;
//using EIObufferInt      = EIObufferT<int32_t>;
//using EIObufferFloat    = EIObufferT<float>;

using EDelayLine        = EDelayLineT<EDataType>;
// --------------------------------------------------------------------
extern const EIObuffer nullEBuffer;
// --------------------------------------------------------------------
} // end namespace yacynth


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
// Effect stage IO buffer
struct FadeBufferTag { FadeBufferTag(int){} };

struct EIObuffer : public EbufferPar {
    static constexpr std::size_t channelCount       = 2;
    typedef std::array<float,channelCount> elementSetType;
    
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
       
    inline void fadeV4( const EIObuffer& inp0, const EIObuffer& inp1, 
        float& gain0, float& gain1, 
        float dgain0, float dgain1 )
    {
        constexpr float fadeGainV4   =  (1.0f/(1<<4));
        dgain0 *= fadeGainV4;
        dgain1 *= fadeGainV4;
        float gaini0 = gain0;
        float gaini1 = gain1;
        dgain0 *= fadeGainV4;
        dgain1 *= fadeGainV4;
        for( auto i=0u; i < vsectionSize; ++i ) {
            gaini0 += dgain0;
            gaini1 += dgain1;
            vchannel[chA][i] = 
                    inp0.vchannel[chA][i] * gain0
                  + inp1.vchannel[chA][i] * gain1;
            vchannel[chB][i] = 
                    inp0.vchannel[chB][i] * gain0
                  + inp1.vchannel[chB][i] * gain1;
        }
        gain0 = gaini0;
        gain1 = gaini1;
    }
    
    inline void fadeV4( const EIObuffer& inp0, const EIObuffer& inp1, const EIObuffer& inp2, 
        float& gain0, float& gain1, float& gain2, 
        float dgain0, float dgain1, float dgain2 )
    {
        constexpr float fadeGainV4   =  (1.0f/(1<<4));
        dgain0 *= fadeGainV4;
        dgain1 *= fadeGainV4;
        dgain2 *= fadeGainV4;
        float gaini0 = gain0;
        float gaini1 = gain1;
        float gaini2 = gain2;
        for( auto i=0u; i < vsectionSize; ++i ) {
            gaini0 += dgain0;
            gaini1 += dgain1;
            gaini2 += dgain2;
            vchannel[chA][i] = 
                    inp0.vchannel[chA][i] * gain0
                  + inp1.vchannel[chA][i] * gain1
                  + inp2.vchannel[chA][i] * gain2;
            vchannel[chB][i] = 
                    inp0.vchannel[chB][i] * gain0
                  + inp1.vchannel[chB][i] * gain1
                  + inp2.vchannel[chB][i] * gain2;
        }
        gain0 = gaini0;
        gain1 = gaini1;
        gain2 = gaini2;
    }

    inline void fadeV4( const EIObuffer& inp0, const EIObuffer& inp1, const EIObuffer& inp2, const EIObuffer& inp3, 
        float& gain0, float& gain1, float& gain2, float& gain3, 
        float dgain0, float dgain1, float dgain2, float dgain3 )
    {
        constexpr float fadeGainV4   =  (1.0f/(1<<4));
        dgain0 *= fadeGainV4;
        dgain1 *= fadeGainV4;
        dgain2 *= fadeGainV4;
        dgain3 *= fadeGainV4;
        float gaini0 = gain0;
        float gaini1 = gain1;
        float gaini2 = gain2;
        float gaini3 = gain3;
        for( auto i=0u; i < vsectionSize; ++i ) {
            gaini0 += dgain0;
            gaini1 += dgain1;
            gaini2 += dgain2;
            vchannel[chA][i] = 
                    inp0.vchannel[chA][i] * gain0
                  + inp1.vchannel[chA][i] * gain1
                  + inp2.vchannel[chA][i] * gain2
                  + inp3.vchannel[chA][i] * gain3;
            vchannel[chB][i] = 
                    inp0.vchannel[chB][i] * gain0
                  + inp1.vchannel[chB][i] * gain1
                  + inp2.vchannel[chB][i] * gain2
                  + inp3.vchannel[chB][i] * gain3;
        }
        gain0 = gaini0;
        gain1 = gaini1;
        gain2 = gaini2;
        gain3 = gaini3;
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
// channel[0] = (float *) ::operator new  ( bufferSize * sizeof(float) * channelCount );
// channel[1] = channel[0] + bufferSize
        for( auto& ichn : channel ) ichn = (float *) ::operator new  ( bufferSize * sizeof(float) );
//        for( auto& ichn : channel ) ichn = new float [ bufferSize ];

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
    inline void pushBlock( const float * valA, float * valB )
    {
        for( uint32_t ind = 0u; ind < sectionSize; ++ind ) {
            channel[chA][ index ] = *valA++;
            channel[chB][ index ] = *valB++;
            index = ( ++index ) & bufferSizeMask;
        }
    };
    //
    // ----------------------------------------------------------------------
    // different utility functions - perhaps will be moved to an other class

    // ----------------------------------------------------------------------
    //
    // const float y0 = channel[chA][ indk0 ]
    // const float dy = y0 - channel[chA][ indk1 ]
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
        return channel[chA][ ( indAk0 - 1 ) & bufferSizeMask ] * multAF0 + channel[chA][ indAk0 ] * ( 1.0f - multAF0 );
    };

    inline float getInterpolatedB( const uint64_t lindexB ) const
    {
        constexpr float range       = 1.0 / (1L<<32);
        constexpr uint64_t Fmask    = 0x0FFFFFFFFLL;
        const uint32_t  indBk0      = ( index - ( lindexB>>32 ) ) & bufferSizeMask; // from the past
        const float     multBF0     = ( lindexB & Fmask ) * range;
        return channel[chB][ ( indBk0 - 1 ) & bufferSizeMask ] * multBF0 + channel[chB][ indBk0 ] * ( 1.0f - multBF0 );
    };

    // ---------------------------------------------------------------------

    // these are used by the Echo
    inline void multAddMixStereo( const uint32_t delay, const V4sfMatrix trm, float& A, float& B ) const
    {
        const uint32_t cind = getIndex(delay);
        A += channel[chA][cind] * trm.aa + channel[chB][cind] * trm.ab;
        B += channel[chA][cind] * trm.ba + channel[chB][cind] * trm.bb;
    }

    template< int Noisefloor >
    inline void multAddMixStereoNoisefloor( const uint32_t delay, const V4sfMatrix trm, float& A, float& B ) const
    {
        const uint32_t cind = getIndex(delay);
        const float inA = noisefloor<Noisefloor>(channel[chA][cind]);  // 2^-24
        const float inB = noisefloor<Noisefloor>(channel[chB][cind]);
        A += inA * trm.aa + inB * trm.ab;
        B += inA * trm.ba + inB * trm.bb;
    }
    // ----------------------------------------------------------------------
    // these are used by the Comb
    inline void multAdd( const uint32_t delay, const float mult, float& A, float& B ) const
    {
        const uint32_t cind = getIndex(delay);
        A += channel[chA][cind] * mult;
        B += channel[chB][cind] * mult;
    }
    inline void get( const uint32_t delay, float& A, float& B ) const
    {
        const uint32_t cind = getIndex(delay);
        A = channel[chA][cind];
        B = channel[chB][cind];
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
        A = channel[chA][ ( indAk0 - 1 ) & bufferSizeMask ] * multAF0 + channel[chA][ indAk0 ] * ( mult - multAF0 );
        B = channel[chB][ ( indBk0 - 1 ) & bufferSizeMask ] * multBF0 + channel[chB][ indBk0 ] * ( mult - multBF0 );
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
        A = channel[chA][ ( indAk0 - 1 ) & bufferSizeMask ] * multAF0 + channel[chA][ indAk0 ] * ( 1.0f - multAF0 );
        B = channel[chB][ ( indBk0 - 1 ) & bufferSizeMask ] * multBF0 + channel[chB][ indBk0 ] * ( 1.0f - multBF0 );
    };

    // ----------------------------------------------------------------------
    // mono stereo
    inline void combFeedbackA(
        const uint32_t  lindexA,
        const float     multA,
        const float     inA,
        float& outA  )
    {
        channel[chA][ index ] = outA = inA + channel[chA][ getIndex(lindexA) ] * multA;
        incIndex();
    };

    inline void combFeedbackStereo(
        const uint32_t  lindexA, const uint32_t  lindexB,
        const float multA, const float multB,
        const float inA, const float inB,
        float& outA, float& outB  )
    {
        channel[chA][ index ] = outA = inA + channel[chA][ getIndex(lindexA) ] * multA;
        channel[chB][ index ] = outB = inB + channel[chB][ getIndex(lindexB) ] * multB;
        incIndex();
    };
    inline void combFeedbackDelayA(
        const uint32_t  lindexA,
        const float     multA,
        const float     inA,
        float& outA  )
    {
        outA = channel[chA][ getIndex(lindexA) ];
        channel[chA][ index ] = inA + outA * multA;
        incIndex();
    };

    inline void combFeedbackDelayStereo(
        const uint32_t  lindexA, const uint32_t  lindexB,
        const float multA, const float multB,
        const float inA, const float inB,
        float& outA, float& outB  )
    {
        outA = channel[chA][ getIndex(lindexA) ];
        channel[chA][ index ] = inA + outA * multA;
        outB = channel[chA][ getIndex(lindexA) ];
        channel[chB][ index ] = inB + outB * multB;
        incIndex();
    };

    inline void combAllpassA(
        const uint32_t  lindexA,
        const float     multA,
        const float     inA,
        float& outA  )
    {
        const auto yA = channel[chA][ getIndex(lindexA) ];
        const auto xA = channel[chA][ index ] = inA - yA * multA;
        outA = yA + xA * multA;
        incIndex();
    };

    inline void combAllpassStereo(
        const uint32_t  lindexA, const uint32_t  lindexB,
        const float     multA, const float     multB,
        const float inA, const float inB,
        float& outA, float& outB  )
    {
        const auto yA = channel[chA][ getIndex(lindexA) ];
        const auto yB = channel[chB][ getIndex(lindexB) ];
        const auto xA = channel[chA][ index ] = inA - yA * multA;
        const auto xB = channel[chB][ index ] = inB - yB * multB;
        outA = yA + xA * multA;
        outB = yB + xB * multB;
        incIndex();
    };
    // ----------------------------------------------------------------------
    // tapped line delay (FIR) 
    // 
    void fillTLDSection( 
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
    
    void fillTLDSection( 
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

    void addTLDSection( 
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
// --- EDelayLineArray ------------------------------------------------
// --------------------------------------------------------------------

template< std::size_t channelCount >
struct EDelayLineArray : public EbufferPar {
    typedef std::array<float,channelCount> elementSetType;
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
//        for( auto& ichn : channel ) ichn = new float ( bufferSize );
        for( auto& ichn : channel ) ichn = (float *) ::operator new  ( bufferSize * sizeof(float) );
        clear();
    };

    EDelayLineArray() = delete;

    void clear(void)
    {
        for( auto& ichn : channel ) memset( ichn, 0, bufferSize*sizeof(float) );
        index = 0;
    };

    ~EDelayLineArray()
    {
        for( auto& ichn : channel ) delete ichn;
    };

    inline void push( const PackedChannel& p )
    {
        for( uint32_t i=0; i<channelCount; ++i )
            channel[i][ index ] = p.v[i];
        incIndex();
    };

    inline void get( const DelayIndex ind, PackedChannel& p ) const
    {
        for( uint32_t i=0; i<channelCount; ++i )
             p.v[i] = channel[i][ getIndex( ind[i] ) ];
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


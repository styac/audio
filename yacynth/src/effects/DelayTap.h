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
 * File:   DelayTap.h
 * Author: Istvan Simon
 *
 * Created on April 4, 2016, 8:00 PM
 */
#include    "Serialize.h"
#include    "Ebuffer.h"
#include    "v4.h"

#include    <cstdint>
#include    <cmath>
#include    <algorithm>

namespace yacynth {

// used in LateReverb
// delay with lowpass + highpass
template< std::size_t N >
struct alignas(16) MonoDelayBandpassTapArray {
    static constexpr std::size_t size = N;
    typedef std::array<uint32_t, N> DelayIndex;
    static constexpr std::size_t v4size = (N+3)/4;

    struct {
        union {
            float   v[N];             // feedback - output
            v4sf    v4[(N+3)/4];
        };
    } coeff;

    struct {
        union {
            float   v[N];             // feedback - output
            v4sf    v4[(N+3)/4];
        };
    } coeffLowPass;
    struct {
        union {
            float   v[N];             // feedback - output
            v4sf    v4[(N+3)/4];
        };
    } coeffHighPass;

    DelayIndex    delayIndex;             // delay of the tap
};

// used in LateReverb
// delay with lowpass
template< std::size_t N >
struct alignas(16) MonoDelayLowpassTapArray {
    static constexpr std::size_t size = N;
    typedef std::array<uint32_t, N> DelayIndex;
    static constexpr std::size_t v4size = (N+3)/4;

    struct {
        union {
            float   v[N];             // feedback - output
            v4sf    v4[(N+3)/4];
        };
    } coeff;

    struct {
        union {
            float   v[N];             // feedback - output
            v4sf    v4[(N+3)/4];
        };
    } coeffLowPass;

    DelayIndex    delayIndex;             // delay of the tap
};

// used in LateReverb
// simple delay
template< std::size_t N >
struct alignas(16) MonoDelayTapArray {
    static constexpr std::size_t size = N;
    typedef std::array<uint32_t, N> DelayIndex;
    static constexpr std::size_t v4size = (N+3)/4;
    struct {
        union {
            float   v[N];             // feedback - output
            v4sf    v4[(N+3)/4];
        };
    } coeff;

    DelayIndex    delayIndex;             // delay of the tap
};


// used in Echo
template< std::size_t N >
struct StereoDelayTapArray {
    typedef std::array<uint32_t, N> DelayIndex;
    static constexpr std::size_t size = N;
    V4vf        coeff[N];
    uint32_t    delayIndex[N][2];
};

// used in Echo
template< std::size_t N >
struct StereoDelayLowPassTapArray {
    typedef std::array<uint32_t, N> DelayIndex;
    static constexpr std::size_t size = N;
    V4vf        coeff[N];
    V4vf        coeffLowPass[N];
    uint32_t    delayIndex[N][2];
};

template< std::size_t N >
struct StereoFractionalDelayTapArray {
    typedef std::array<uint32_t, N> DelayIndex;
    static constexpr std::size_t size = N;
    V4vf        coeff[N];
    uint64_t    delayIndex[N][2];
};


#if 0
// ------------------------------------------------
// --------------------------------------------------------------------
// out      = delay * wet
// delay    = delay * decay
//
struct Gains {
    float   wet;
    float   decay;
};

// --------------------------------------------------------------------
// store/create
struct StereoDelayTapOld {
    void clear(void)
    {
        delaySrc = delayDst = 0;
        coeff.clear();
    }
    V4sfMatrix  coeff;
    //
    // interpreting the delay:
    // for non interpolated tap the delaySrcL,delayDstL is NOT USED
    // for interpolated the delaySrcL,delayDstL is the fraction
    // for "fine" tap the delaySrcH, delayDstH is the delay value
    // for "fast" the lower 6bits are not used of the delaySrcH, delayDstH
    // (if the section size is 64)
    // this is for LOW ENDIAN CPU
    //
    union {
        uint64_t        delaySrc;       // delay of the tap
        struct {
            uint32_t    delaySrcL;
            uint32_t    delaySrcH;
        };
    };
    //
    // delay for internal feedback
    // otherwise only padding for 16 bytes
    // relative to the delaySrc !!!
    //
    union {
        int64_t         delayDst;
        struct {
            int32_t     delayDstL;
            int32_t     delayDstH;
        };
    };
};
// --------------------------------------------------------------------

// --------------------------------------------------------------------
// working
//
//  dstA = srcA*aa + srcB *ab
//  dstB = srcA*ba + srcB *bb
//
//  v = v0 * gain   : gain = wet/dry/decay
//
struct alignas(16) StereoDelayTapVar /* : public StereoDelayTap */ {
    static constexpr uint64_t invalid = -1L;
    void clear(void)
    {
        delaySrc = invalid;
        delayDst = 0;
        coeff.clear();
        coeff0.clear();
    }
    void set( const StereoDelayTapOld& tap ) {
        coeff.v = coeff0.v = tap.coeff.v;
        delaySrc = tap.delaySrc;
        delayDst = tap.delayDst;
    }
    inline void mult( const float mv = 1.0f ) // copy
    {
        coeff.v = mv * coeff0.v; // ok
    }
    inline bool isValid(void) const
    {
        return delaySrcH != uint32_t( invalid );
    }
    inline void setInvalid(void)
    {
        delaySrc = invalid;
    }
    //------------------------
    V4sfMatrix  coeff;
    V4sfMatrix  coeff0;

    //
    // interpreting the delay:
    // for non interpolated tap the delaySrcL,delayDstL is NOT USED
    // for interpolated the delaySrcL,delayDstL is the fraction
    // for "fine" tap the delaySrcH, delayDstH is the delay value
    // for "fast" the lower 6bits are not used of the delaySrcH, delayDstH
    // (if the section size is 64)
    // this is for LOW ENDIAN CPU
    //
    union {
        uint64_t        delaySrc;       // delay of the tap
        struct {
            uint32_t    delaySrcL;
            uint32_t    delaySrcH;
        };
    };
    //
    // delay for internal feedback
    // otherwise only padding for 16 bytes
    // relative to the delaySrc !!!
    //
    union {
        int64_t         delayDst;
        struct {
            int32_t     delayDstL;
            int32_t     delayDstH;
        };
    };
    struct {
        int32_t         modulationFrequency;
        int32_t         modulationDepth;
        float           filterFc;
        int8_t          modulationType;    //  phase diff between A,B
        int8_t          rfu1;
        int8_t          rfu2;
        int8_t          rfu3;
    };
    struct {
        float           filterA;
        float           filterB;
    };
};

// --------------------------------------------------------------------
template<
    uint32_t sectionSizeExp,    // 2^k needed to check the delay validity
    uint32_t vectorSizeExp      // 2^k
    >
struct alignas(16) StereoDelayTapVector {
    static constexpr   uint16_t sectionSize = 1<<sectionSizeExp;
    static constexpr   uint16_t vectorSize  = 1<<vectorSizeExp;
    StereoDelayTapVector()
    :   usedTapCount(0)
    {};

    void setDelayLength( const uint32_t v )
    {
        delayLength = v;
    }
    void clear(void)
    {
        for( auto& v : dtvec ) v.clear();
        usedTapCount = 0;
    }

    inline void mult( const float mv )
    {
        for( auto& v : dtvec ) v.mult( mv );
    }
    inline void setInvalid(void)
    {
        for( auto& v : dtvec ) v.setInvalid();
    }
    bool checkDelaySrcLast(void)
    {
        if( 0 == usedTapCount ) // nothing
            return true;
        auto& v = dtvec[usedTapCount-1];
        if( ! v.isValid() )
            return false;
        if( v.delaySrcH >= delayLength ) {
            v.setInvalid();
            return false;
        }
        return true;
    }
    bool checkDelaySrc(void)
    {
        for( auto& v : dtvec ) {
            if( ! v.isValid() )
                continue;
            if( v.delaySrcH >= delayLength )
                v.setInvalid();
        }
    }
    bool checkDelayDst(void)
    {
        for( auto& v : dtvec ) {
            if( ! v.isValid() )
                continue;
            const int64_t  sums = v.delaySrcH + v.delayDstH;
            if(     v.delaySrcH >= delayLength
                ||  std::abs(v.delayDstH) < sectionSize
                ||  sums >= delayLength
                ||  sums < 0
            )
            v.setInvalid();
        }
    }
    bool dropLastInvalid( void )
    {
        if( ! checkDelaySrcLast() ) {
            --usedTapCount;
            return false;
        }
        return true;
    }
    bool add( const StereoDelayTapOld& tap ) {
        if( usedTapCount > vectorSize)
            return false;
        dtvec[usedTapCount++].set(tap);
        return dropLastInvalid();
    }
    uint16_t fill( std::stringstream& ser ) {
        StereoDelayTapOld    tmp;
        clear();
//        deserializeBegin(ser);
        for( auto i = 0u; i < vectorSize; ++i ) {
//            if( ! deserialize( ser, tmp ) ) {
//                return usedTapCount;
//            }
            add(tmp);
        }
        return usedTapCount;
    };

    void list(void)
    {
        for( auto i = 0u; i < vectorSize; ++i ) {
            auto& v = dtvec[i];
            std::cout
                << "i " << i
                << " aa  " << v.coeff0.aa
                << " ab  " << v.coeff0.ab
                << " ba  " << v.coeff0.ba
                << " bb  " << v.coeff0.bb
                << " delaySrc  " << v.delaySrc
                << "  delayDst " << v.delayDst
                << std::endl;
        }

    }
    StereoDelayTapVar  dtvec[ vectorSize ];
    uint32_t        delayLength;
    uint16_t        usedTapCount;
};
// --------------------------------------------------------------------
struct alignas(16) StereoEchoFilter {
    // minfc is max frequency
    // exp( -2*pi*f/fs )
    static constexpr float minfc = 0.06;
    static constexpr float maxfc = 0.999;
    static constexpr float mingn = 0.25;
    static constexpr float maxgn = 4.0;
    StereoEchoFilter()
    :   A(0)
    ,   B(0)
    { set( minfc ); };

    inline void set( const float fc, const float gain = 1.0f )
    {
        const float fci = fc < minfc ? minfc : fc > maxfc ? maxfc : fc;
        const float gni = gain < mingn ? mingn : gain > maxgn ? maxgn : gain;
        a1 = fci;
        b0 = gni - a1 * gni;
    }
    inline void getAB( const EIObuffer& inp, EIObuffer& out )
    {
        const float *inpA = inp.channel[EbufferPar::chA];
        float *outA = out.channel[EbufferPar::chA];
        const float *inpB = inp.channel[EbufferPar::chB];
        float *outB = out.channel[EbufferPar::chB];
        for( auto i = 0u; i < EIObuffer::sectionSize; ++i ) {
            *outA++ = A = *inpA++ * b0 + A * a1;
            *outB++ = B = *inpB++ * b0 + B * a1;
        }
    }
    inline void getAB( EIObuffer& out )
    {
        float *outA = out.channel[EbufferPar::chA];
        float *outB = out.channel[EbufferPar::chB];
        for( auto i = 0u; i < EIObuffer::sectionSize; ++i ) {
            A = *outA * b0 + A * a1;
            B = *outB * b0 + B * a1;
            *outA++ = A;
            *outB++ = B;
        }
    }
    union {
        v4sf    v;
        struct {
            float   A;
            float   B;
            float   a1;
            float   b0;
        };
    };
};
#endif
// --------------------------------------------------------------------
} // end namespace yacynth

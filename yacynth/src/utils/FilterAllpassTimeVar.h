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
 * File:   FilterAllpassTimeVar.h
 * Author: Istvan Simon
 *
 * Created on March 24, 2016, 5:06 PM
 */


#include    "FilterAllpass.h"
#include    "../oscillator/Lfo.h"

using namespace yacynth;

namespace filter {

// --------------------------------------------------------------------
class FilterAllpassTimeVarBase : public FilterAllpassBase {
protected:
    static constexpr uint16_t    stageCountMax = 16;
    Lfo       lfo;
};

//
// phaser flanger
//
class FilterAllpass2TimeVar : public FilterAllpassTimeVarBase {
public:
    static constexpr    float    fMin               = 0.001;    // 48 Hz
    static constexpr    float    fMax               = 0.24;     // ~ 11000 Hz

    // smaller q is bigger q !!! - smaller transient zone
    static constexpr    float    qMin               = 1.1f;     // ?? Fpi = Fbeg * q
    static constexpr    float    qMax               = 4.0f;     // ??
    inline int32_t controller2logF( const uint16_t index )  // controller - 0 .. 127 ; 0 = max freq
    {
        constexpr int32_t val0   = 0x1e500000 - (oversamplingRate-1) * 0x1000000;
        constexpr int32_t val127 = 0x15000000 - (oversamplingRate-1) * 0x1000000;
        constexpr int32_t rate   = ( val0 - val127 ) / 127;
        const int64_t logF =  val0 - int32_t(index) * rate;
        param1.flog.setStep( logF );
    }

    inline int32_t controller2logQ( const uint16_t index )
    {
        param1.qlog.setStep(  int64_t(index) << qRateExp );
    }


    struct alignas(16) Channel {
        float   zx1;
        float   zy1;
        float   zx2;
        float   zy2;
        float   c1;
        float   c2;
    };

    struct Param {
        ParamIterInt    flog;
        ParamIterInt    qlog;
        float           igain;
        float           fgain;
        float           qgain;


        float       ftarget[ stageCountMax ];   // fbase
        float       qtarget;
        float       feedback;
        float       modDepth;       // fmax/fmid ??? - f*k .. f/k ??? up 2x  down 1x - boost1 lfo ( 1 +- k )
        uint16_t    phaseDiffAB;    // 0,90,180,360
        uint16_t    stageCount; // 1..stageCountMax
    };

    FilterAllpass2TimeVar()
    {
        clear();
        sweep();
    };

    void clear(void)
    {
        channelA    = {0};
        channelB    = {0};
        param1       = {0};
        param1.qtarget = qMin;
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
//        param1.qtarget  = checkQ( qv ) ;
    };

    inline void setF( float fv )
    {
//        param1.ftarget  = checkF( fv ) ;
    };

    inline bool sweep( void )
    {
        return true;
    };

    inline void getA( const float in, float& out )
    {
        doA(in);
        out = channelA[ param1.stageCount - 1 ].zy2;
    };

private:
    void evalFQ(void) {
//        const float c   = sintable.fastSin2PIx_1_cos2PIx_071( param1.fcurrent );
//        const float d   = -sintable.fastcos2PI( std::min( param1.fcurrent * param1.qcurrent, fMax ) );   // f/fs
//        param1.c1 = -c;
//        param1.c2 = d * ( c - 1.0f );

    };

    // Mitra-Hirano 3D
    inline void doA( const float in )
    {
        const float inv = in + param1.feedback * channelA[ param1.stageCount - 1 ].zy2;
        const float t1  =  channelA[ 0 ].c1 * ( inv     - channelA[ 0 ].zy2 )
            + channelA[ 0 ].c2 * ( channelA[ 0 ].zx1    - channelA[ 0 ].zy1 ) + channelA[ 0 ].zx2;
        channelA[ 0 ].zx2    = channelA[ 0 ].zx1;
        channelA[ 0 ].zx1    = inv;
        channelA[ 0 ].zy2    = channelA[ 0 ].zy1;
        channelA[ 0 ].zy1    = t1;

        for( uint16_t st = 1u; st < param1.stageCount; ++st ) {
            const float t1  =  channelA[ st ].c1 * ( channelA[ st-1 ].zy2 - channelA[ st ].zy2 )
                + channelA[ st ].c2 * ( channelA[ st ].zx1  - channelA[ st ].zy1 ) + channelA[ st ].zx2;
            channelA[ st ].zx2    = channelA[ st ].zx1;
            channelA[ st ].zx1    = channelA[ st-1 ].zy2;
            channelA[ st ].zy2    = channelA[ st ].zy1;
            channelA[ st ].zy1    = t1;
        };
    };
    inline void doB( const float in )
    {
        const float inv = in + param1.feedback * channelB[ param1.stageCount - 1 ].zy2;
        const float t1  =  channelB[ 0 ].c1 * ( inv     - channelB[ 0 ].zy2 )
            + channelB[ 0 ].c2 * ( channelB[ 0 ].zx1    - channelB[ 0 ].zy1 ) + channelB[ 0 ].zx2;
        channelB[ 0 ].zx2    = channelB[ 0 ].zx1;
        channelB[ 0 ].zx1    = inv;
        channelB[ 0 ].zy2    = channelB[ 0 ].zy1;
        channelB[ 0 ].zy1    = t1;

        for( uint16_t st = 1u; st < param1.stageCount; ++st ) {
            const float t1  =  channelB[ st ].c1 * ( channelB[ st-1 ].zy2 - channelB[ st ].zy2 )
                + channelB[ st ].c2 * ( channelB[ st ].zx1  - channelB[ st ].zy1 ) + channelB[ st ].zx2;
            channelB[ st ].zx2    = channelB[ st ].zx1;
            channelB[ st ].zx1    = channelB[ st-1 ].zy2;
            channelB[ st ].zy2    = channelB[ st ].zy1;
            channelB[ st ].zy1    = t1;
        };
    };

    inline void doAB( const float inA, const float inB )
    {
        const float invA = inA + param1.feedback * channelA[ param1.stageCount - 1 ].zy2;
        const float invB = inB + param1.feedback * channelB[ param1.stageCount - 1 ].zy2;
        const float tA  =  channelA[ 0 ].c1 * ( invA    - channelA[ 0 ].zy2 )
            + channelA[ 0 ].c2 * ( channelA[ 0 ].zx1    - channelA[ 0 ].zy1 ) + channelA[ 0 ].zx2;
        const float tB  =  channelB[ 0 ].c1 * ( invB    - channelB[ 0 ].zy2 )
            + channelB[ 0 ].c2 * ( channelB[ 0 ].zx1    - channelB[ 0 ].zy1 ) + channelB[ 0 ].zx2;
        channelA[ 0 ].zx2    = channelA[ 0 ].zx1;
        channelB[ 0 ].zx2    = channelB[ 0 ].zx1;
        channelA[ 0 ].zx1    = invA;
        channelB[ 0 ].zx1    = invB;
        channelA[ 0 ].zy2    = channelA[ 0 ].zy1;
        channelB[ 0 ].zy2    = channelB[ 0 ].zy1;
        channelA[ 0 ].zy1    = tA;
        channelB[ 0 ].zy1    = tB;

        for( uint16_t st = 1u; st < param1.stageCount; ++st ) {
            const float tA  =  channelA[ st ].c1 * ( channelA[ st-1 ].zy2 - channelA[ st ].zy2 )
                + channelA[ st ].c2 * ( channelA[ st ].zx1  - channelA[ st ].zy1 ) + channelA[ st ].zx2;
            const float tB  =  channelB[ st ].c1 * ( channelB[ st-1 ].zy2 - channelB[ st ].zy2 )
                + channelB[ st ].c2 * ( channelB[ st ].zx1  - channelB[ st ].zy1 ) + channelB[ st ].zx2;
            channelA[ st ].zx2    = channelA[ st ].zx1;
            channelB[ st ].zx2    = channelB[ st ].zx1;
            channelA[ st ].zx1    = channelA[ st-1 ].zy2;
            channelB[ st ].zx1    = channelB[ st-1 ].zy2;
            channelA[ st ].zy2    = channelA[ st ].zy1;
            channelB[ st ].zy2    = channelB[ st ].zy1;
            channelA[ st ].zy1    = tA;
            channelB[ st ].zy1    = tB;
        };
    };
    Channel         channelA[   stageCountMax ];
    Channel         channelB[   stageCountMax ];
    Param           param1;
}; // end FilterAllpass2X4

} // end namespace

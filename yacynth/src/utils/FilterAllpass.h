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
// https://ccrma.stanford.edu/~jos/bbt/Bark_Frequency_Scale.html - Bark freqs
// http://www.music.mcgill.ca/~ich/classes/dafx_book.pdf
// http://www.music.mcgill.ca/~ich/classes/FiltersChap2.pdf

// http://www.micromodeler.com/dsp/
// http://home.deib.polimi.it/bestagini/_Slides/lesson_3.pdf

// http://cdn.intechweb.org/pdfs/15188.pdf 2nd order structures - fractional delay

#include    "FilterBase.h"

using namespace tables;

typedef float v4sf __attribute__((mode(SF)))  __attribute__ ((vector_size(16),aligned(16)));

namespace filter {

template< std::size_t filterCountExp, std::size_t channelCountExp, std::size_t stateCountExp  >
class FilterAllpass : public FilterBase {
public:
    static constexpr  std::size_t filterCount       = 1<<filterCountExp;
    static constexpr  std::size_t filterCountMask   = filterCount-1;
    static constexpr  std::size_t stateCount        = 1<<stateCountExp;
    static constexpr  std::size_t channelCount      = 1<<channelCountExp;
    static constexpr  std::size_t channelCountMask  = channelCount-1;
    static constexpr  std::size_t v4count           = (filterCount+stateCount+channelCount+3)/4;
    static constexpr  std::size_t klinCount         = 1<<(filterCountExp+channelCountExp);
    static constexpr  std::size_t klinCountMask     = klinCount-1;

    static constexpr  std::size_t ZD0   = 0;    // z delay
    static constexpr  std::size_t ZD1   = 1;    // z delay
    static constexpr  std::size_t CH0   = 0;    // channel
    static constexpr  std::size_t CH1   = 1;    // channel
    static constexpr  std::size_t FL0   = 0;    // filter
    static constexpr  std::size_t FL1   = 1;    // filter
    static constexpr  std::size_t FL2   = 2;    // filter
    static constexpr  std::size_t FL3   = 3;    // filter

    // parameter direct set
    template< std::size_t CH, std::size_t ZD >
    inline void setK( float v, std::size_t index=0 )
    {
        if( ZD0 == ZD ) {
            k[ZD][ index & filterCountMask ][CH] = v;
        } else if( ZD1 == ZD ) {
            k[ZD][ index & filterCountMask ][CH] = v;
        } else {
            static_assert(ZD==0 || ZD==1,"illegal parameter");
        }
    }

    // parameter set with frequency -- only for testing -- slow
    template< std::size_t CH, std::size_t ZD >
    inline void setFreq( float fc, std::size_t index=0 )
    {
        if( ZD0 == ZD ) {
            k[ZD][ index & filterCountMask ][CH] = FilterTable::fc_sinpercosPi2_F( fc );
        } else if( ZD1 == ZD ) {
            k[ZD][ index & filterCountMask ][CH] = FilterTable::fc_cosPi2_F( fc );
        } else {
            static_assert(ZD==0 || ZD==1,"illegal parameter");
        }
    }

    // parameter set with ycent
    template< std::size_t CH, std::size_t ZD >
    inline void setYcent( int32_t ycent, std::size_t index=0 )
    {
        if( ZD0 == ZD ) {
            k[ZD][ index & filterCountMask ][CH] = FilterTableSinCosPi2::getInstance().getFloat( ycent );
        } else if( ZD1 == ZD ) {
            k[ZD][ index & filterCountMask ][CH] = FilterTableCos2Pi::getInstance().getFloat( ycent );
        } else {
            static_assert(ZD==0 || ZD==1,"illegal parameter");
        }
    }

    // index 0..2^k-1 -- filter  2^k -- channel
    template< std::size_t ZD >
    inline void setYcent( int32_t ycent, std::size_t index )
    {
        static_assert(sizeof(klin) == sizeof(k), "different sizes");
        if( ZD0 == ZD ) {
            klin[ZD][ index & klinCountMask ] = FilterTableSinCosPi2::getInstance().getFloat( ycent );
        } else if( ZD1 == ZD ) {
            klin[ZD][ index & klinCountMask ] = FilterTableCos2Pi::getInstance().getFloat( ycent );
        } else {
            static_assert(ZD==0 || ZD==1,"illegal parameter");
        }
    }

    template< std::size_t ZD >
    inline void setYcentAll( int32_t ycent, int32_t ycentDelta )
    {
        static_assert(sizeof(klin) == sizeof(k), "different sizes");
        if( ZD0 == ZD ) {
            for( auto fi=0u; fi < klinCount; ++fi ) {
                klin[ZD][ fi ] = FilterTableSinCosPi2::getInstance().getFloat( ycent );
                ycent += ycentDelta;
            }
        } else if( ZD1 == ZD ) {
            for( auto fi=0u; fi < klinCount; ++fi ) {
                klin[ZD][ fi ] = FilterTableCos2Pi::getInstance().getFloat( ycent );
                ycent += ycentDelta;
            }
        } else {
            static_assert(ZD==0 || ZD==1,"illegal parameter");
        }
    }

    inline void setFeedback( float fb)
    {
        feedback = fb;
    }

//----------------------------------------------
// 1st order single

    template< std::size_t CH >
    inline void allpass1_transposed( const float x0, float& y0 )
    {
        y0 = x0 * k[ZD0][FL0][CH] + z[ZD0][FL0][CH];
        z[ZD0][FL0][CH] = x0 - y0 * k[ZD0][FL0][CH];
    }

    template< std::size_t CH >
    inline void allpass1_direct( const float x0, float& y0 )
    {
        const float t0 = x0 - z[ZD0][FL0][CH] * k[ZD0][FL0][CH];
        y0 = t0 * k[ZD0][FL0][CH] + z[ZD0][FL0][CH];
        z[ZD0][FL0][CH] = t0;
    }

    // -----------------
    // 1 multiplier is the best for 1st order -- default
    template< std::size_t CH >
    inline void allpass1( const float x0, float& y0 )
    {
        static_assert(channelCount>CH,"channel count low");
        const float t0 = ( x0 - z[ZD0][FL0][CH] ) * k[ZD0][FL0][CH];
        y0 = t0 + z[ZD0][FL0][CH];
        z[ZD0][FL0][CH] = t0 + x0;
    }

    // 2 channel - stereo special case
    inline void allpass1( const float x0, const float x1, float& y0, float& y1 )
    {
        static_assert(channelCount>1,"channel count low");
        const float t0 = ( x0 - z[ZD0][FL0][CH0] ) * k[ZD0][FL0][CH0];
        const float t1 = ( x1 - z[ZD0][FL0][CH1] ) * k[ZD0][FL0][CH1];
        y0 = t0 + z[ZD0][FL0][CH0];
        y1 = t1 + z[ZD0][FL0][CH1];
        z[ZD0][FL0][CH0] = t0 + x0;
        z[ZD0][FL0][CH1] = t1 + x1;
    }

    // name compatibility forwarders
    template< std::size_t CH >
    inline void allpass1_1mult( const float x0, float& y0 )
    {
        allpass1<CH>( x0, y0 );
    }

    inline void allpass1_1mult( const float x0, const float x1, float& y0, float& y1 )
    {
        allpass1( x0, x1, y0, y1 );
    }

//----------------------------------------------
// 2nd order single
    template< std::size_t CH >
    inline void allpass2_direct( const float x0, float& y0 )
    {
        static_assert(stateCount>1,"state count low");
        static_assert(channelCount>CH,"channel count low");
        // 1th stage
        const float t0 = x0 + z[ZD0][FL0][CH] * k[ZD0][FL0][CH];
        y0 = z[ZD0][FL0][CH] - t0 * k[ZD0][FL0][CH];
        // 2nd stage
        const float t1 = t0 + z[ZD1][FL0][CH] * k[ZD1][FL0][CH];
        z[ZD0][FL0][CH] = z[ZD1][FL0][CH] - t1 * k[ZD1][FL0][CH];
        z[ZD1][FL0][CH] = t1;
    }

    template< std::size_t CH >
    inline void allpass2_1mult( const float x0, float& y0 )
    {
        static_assert(stateCount>1,"state count low");
        static_assert(channelCount>CH,"channel count low");
        // 1th stage -- tan
        const float t00 = ( z[ZD0][FL0][CH] - x0 ) * k[ZD0][FL0][CH];
        y0 = t00 + z[ZD0][FL0][CH];
        // 2nd stage -- cos
        const float t001 = t00 + x0;
        const float t01 = ( z[ZD1][FL0][CH] - t001 ) * k[ZD1][FL0][CH];
        z[ZD0][FL0][CH] = t01 + z[ZD1][FL0][CH];
        z[ZD1][FL0][CH] = t01 + t001;
    }

    // 1 stage > transposed
    // 2 stage > 1mult
    template< std::size_t CH >
    inline void allpass2_1mult_transposed( const float x0, float& y0 )
    {
        static_assert(stateCount>1,"state count low");
        static_assert(channelCount>CH,"channel count low");
        // 1th stage -- tan
        y0 = z[ZD0][FL0][CH] - x0 * k[ZD0][FL0][CH]; // - ( tan-1)/(tan+1) !!
        const float t0 = x0 + y0 * k[ZD0][FL0][CH]; // - ( tan-1)/(tan+1) !!
        // 2nd stage
        const float t01 = ( z[ZD1][FL0][CH] - t0 ) * k[ZD1][FL0][CH];
        z[ZD0][FL0][CH] = t01 + z[ZD1][FL0][CH];
        z[ZD1][FL0][CH] = t01 + t0;
    }

    //transposed is the best for 2nd order
    template< std::size_t CH >
    inline void allpass2( const float x0, float& y0 )
    {
        static_assert(stateCount>1,"state count low");
        static_assert(channelCount>CH,"channel count low");
        // 1th stage
        y0 = z[ZD0][FL0][CH] - x0 * k[ZD0][FL0][CH]; // - ( tan-1)/(tan+1) !!
        // 2nd stage
        const float t0 = x0 + y0 * k[ZD0][FL0][CH]; // - ( tan-1)/(tan+1) !!
        z[ZD0][FL0][CH] = z[ZD1][FL0][CH] - t0 * k[ZD1][FL0][CH];  // - cos !!
        z[ZD1][FL0][CH] = t0 + z[ZD0][FL0][CH] * k[ZD1][FL0][CH];  // - cos !!
    }

    inline void allpass2( const float x0, const float x1, float& y0, float& y1 )
    {
        static_assert(stateCount>1,"state count low");
        static_assert(channelCount>1,"channel count low");
        // 1th stage
        y0 = z[ZD0][FL0][CH0] - x0 * k[ZD0][FL0][CH0];
        y1 = z[ZD0][FL0][CH1] - x1 * k[ZD0][FL0][CH1];
        // 2nd stage
        const float t0 = x0 + y0 * k[ZD0][FL0][CH0];
        const float t1 = x1 + y1 * k[ZD0][FL0][CH1];
        z[ZD0][FL0][CH0] = z[ZD1][FL0][CH0] - t0 * k[ZD1][FL0][CH0];
        z[ZD0][FL0][CH1] = z[ZD1][FL0][CH1] - t1 * k[ZD1][FL0][CH1];
        z[ZD1][FL0][CH0] = t0 + z[ZD0][FL0][CH0] * k[ZD1][FL0][CH0];
        z[ZD1][FL0][CH1] = t1 + z[ZD0][FL0][CH1] * k[ZD1][FL0][CH1];
    }

    template< std::size_t CH >
    inline void allpass2_transposed( const float x0, float& y0 )
    {
        allpass2<CH>(x0,y0);
    }

    inline void allpass2_transposed( const float x0, const float x1, float& y0, float& y1 )
    {
        allpass2(x0,x1,y0,y1);
    }
//----------------------------------------------
// 1st order multi

//----------------------------------------------
// 2nd order multi

    // allpass cascade> 2 x 4 filter
    inline void allpass2x8( const float x0, const float x1, float& y0, float& y1 )
    {
        static_assert(stateCount>1,"state count low");
        static_assert(channelCount>1,"channel count low");
        static_assert(filterCount>3,"filter count low");
        static_assert(sizeof(klin) == sizeof(k), "different sizes");

        union {
            v4sf    tv4[2];
            float   t[4][2];
        };

        // feedback
//        const float t0y0 = x0;
//        const float t0y1 = x1;
        const float t0y0 = x0 + b[CH0];
        const float t0y1 = x1 + b[CH1];
        // 1th stage
        const float t1y0 = z[ZD0][FL0][CH0] - t0y0 * k[ZD0][FL0][CH0];
        const float t1y1 = z[ZD0][FL0][CH1] - t0y1 * k[ZD0][FL0][CH1];
        t[FL0][CH0] = t0y0 + t1y0 * k[ZD0][FL0][CH0];
        t[FL0][CH1] = t0y1 + t1y1 * k[ZD0][FL0][CH1];

        const float t2y0 = z[ZD0][FL1][CH0] - t1y0 * k[ZD0][FL1][CH0];
        const float t2y1 = z[ZD0][FL1][CH1] - t1y1 * k[ZD0][FL1][CH1];
        t[FL1][CH0] = t1y0 + t2y0 * k[ZD0][FL1][CH0];
        t[FL1][CH1] = t1y1 + t2y1 * k[ZD0][FL1][CH1];

        const float t3y0 = z[ZD0][FL2][CH0] - t2y0 * k[ZD0][FL2][CH0];
        const float t3y1 = z[ZD0][FL2][CH1] - t2y1 * k[ZD0][FL2][CH1];
        t[FL2][CH0] = t2y0 + t3y0 * k[ZD0][FL2][CH0];
        t[FL2][CH1] = t2y1 + t3y1 * k[ZD0][FL2][CH1];

        y0 = z[ZD0][FL3][CH0] - t3y0 * k[ZD0][FL3][CH0];
        y1 = z[ZD0][FL3][CH1] - t3y1 * k[ZD0][FL3][CH1];
        t[FL3][CH0] = t3y0 + y0 * k[ZD0][FL3][CH0];
        t[FL3][CH1] = t3y1 + y1 * k[ZD0][FL3][CH1];
        b[CH0] = feedback * y0;
        b[CH1] = feedback * y1;
#if 1
        // 2nd stage
        z[ZD0][FL0][CH0] = z[ZD1][FL0][CH0] - t[FL0][CH0] * k[ZD1][FL0][CH0];
        z[ZD0][FL0][CH1] = z[ZD1][FL0][CH1] - t[FL0][CH1] * k[ZD1][FL0][CH1];
        z[ZD0][FL1][CH0] = z[ZD1][FL1][CH0] - t[FL1][CH0] * k[ZD1][FL1][CH0];
        z[ZD0][FL1][CH1] = z[ZD1][FL1][CH1] - t[FL1][CH1] * k[ZD1][FL1][CH1];

        z[ZD1][FL0][CH0] = t[FL0][CH0] + z[ZD0][FL0][CH0] * k[ZD1][FL0][CH0];
        z[ZD1][FL0][CH1] = t[FL0][CH1] + z[ZD0][FL0][CH1] * k[ZD1][FL0][CH1];
        z[ZD1][FL1][CH0] = t[FL1][CH0] + z[ZD0][FL1][CH0] * k[ZD1][FL1][CH0];
        z[ZD1][FL1][CH1] = t[FL1][CH1] + z[ZD0][FL1][CH1] * k[ZD1][FL1][CH1];

        z[ZD0][FL2][CH0] = z[ZD1][FL2][CH0] - t[FL2][CH0] * k[ZD1][FL2][CH0];
        z[ZD0][FL2][CH1] = z[ZD1][FL2][CH1] - t[FL2][CH1] * k[ZD1][FL2][CH1];
        z[ZD0][FL3][CH0] = z[ZD1][FL3][CH0] - t[FL3][CH0] * k[ZD1][FL3][CH0];
        z[ZD0][FL3][CH1] = z[ZD1][FL3][CH1] - t[FL3][CH1] * k[ZD1][FL3][CH1];

        z[ZD1][FL2][CH0] = t[FL2][CH0] + z[ZD0][FL2][CH0] * k[ZD1][FL2][CH0];
        z[ZD1][FL2][CH1] = t[FL2][CH1] + z[ZD0][FL2][CH1] * k[ZD1][FL2][CH1];
        z[ZD1][FL3][CH0] = t[FL3][CH0] + z[ZD0][FL3][CH0] * k[ZD1][FL3][CH0];
        z[ZD1][FL3][CH1] = t[FL3][CH1] + z[ZD0][FL3][CH1] * k[ZD1][FL3][CH1];
#else
        const v4sf& tf01ch01 = tv4[0];
        const v4sf& tf23ch01 = tv4[1];

        const v4sf& k0fl01ch01 = v4[0];
        const v4sf& k0fl23ch01 = v4[1];
        const v4sf& k1fl01ch01 = v4[2];
        const v4sf& k1fl23ch01 = v4[3];

        v4sf& z0fl01ch01 = v4[4];
        v4sf& z0fl23ch01 = v4[5];
        v4sf& z1fl01ch01 = v4[6];
        v4sf& z1fl23ch01 = v4[7];

        z0fl01ch01  = z1fl01ch01 - tf01ch01 * k1fl01ch01;
        z1fl01ch01  = tf01ch01 + z0fl01ch01 * k1fl01ch01;
        z0fl23ch01  = z1fl23ch01 - tf23ch01 * k1fl23ch01;
        z1fl23ch01  = tf23ch01 + z0fl23ch01 * z1fl23ch01;
#endif

        // 1. 1 Hz 1st order fix allpass --> changes phase -- forward signal minus ! - to supresss DC
    }

//----------------------------------------------
private:
    union alignas(cacheLineSize) {
        v4sf    v4[v4count];
        struct {
            union {
                float   klin[stateCount][klinCount]; // linear access for setting
                float   k[stateCount][filterCount][channelCount];
            };
            float   z[stateCount][filterCount][channelCount];
        };
    };

    // this should go from here to FxFilter param
    float b[channelCount];  // feedback
    float feedback;         // feedback coeff
};

} // end namespace
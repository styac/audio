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

// modelling
// http://www.micromodeler.com/dsp/

//  2nd order structures - fractional delay
// http://cdn.intechweb.org/pdfs/15188.pdf
// http://home.deib.polimi.it/bestagini/_Slides/lesson_3.pdf

#include    "FilterBase.h"

using namespace tables;

typedef float v4sf __attribute__((mode(SF)))  __attribute__ ((vector_size(16),aligned(16)));

namespace filter {

// TODO review
//
// 2nd order allpass
// kFf = - cos omega --- k1  -- back 2nd stage
// kBw =( 1 - tan fi ) / ( 1 + tan fi )  --- k2 -- front 1st stage
// omega = 2pi * fc
// 
//
//  filterCount     : filter / channel
//  channelCount    : mono=1, stereo=2
//  stateCount      : 1st order=1, 2nd order=2
// 
template< std::size_t filterCountExp, std::size_t channelCountExp, std::size_t stateCountExp  >
class FilterAllpass : public FilterBase {
public:
    static constexpr  std::size_t filterCount           = 1<<filterCountExp;
    static constexpr  std::size_t filterCountMask       = filterCount-1;
    static constexpr  std::size_t stateCount            = 1<<stateCountExp;
    static constexpr  std::size_t coeffCount            = stateCount;
    static constexpr  std::size_t coeffCountMask        = coeffCount-1;
    static constexpr  std::size_t channelCount          = 1<<channelCountExp;
    static constexpr  std::size_t channelCountMask      = channelCount-1;
    static constexpr  std::size_t v4kcount              = (filterCount*coeffCount*channelCount+3)/4;
    static constexpr  std::size_t v4zcount              = (filterCount*stateCount*channelCount+3)/4;    
    static constexpr  std::size_t allFilterCount        = 1<<(filterCountExp+channelCountExp);
    static constexpr  std::size_t allFilterCountMask    = allFilterCount-1;
    static constexpr  std::size_t allStateCount         = 1<<(filterCountExp+channelCountExp+stateCountExp);

    
    FilterAllpass()
    {
        clear();
    };
    
    void clear(void)
    {
        for( auto &v : zv ) v = 0;
    };

    // TODO refactor set_Q set_F
    
    // parameter direct set
    template< std::size_t CH, std::size_t CD >
    inline void setK( float v, std::size_t index=0 )
    {
        if( ZD0 == CD ) {
            // for AP2 direct set BW > -0.5 .. -0.9xxx
            km3[CD][ index & filterCountMask ][CH] = v;
        } else if( ZD1 == CD ) {
            km3[CD][ index & filterCountMask ][CH] = v;
        } else {
            static_assert(CD==0 || CD==1,"illegal parameter");
        }
    }
#if 0
    // like setYcentAll
    template< std::size_t CD >
    inline void setKAll( float v )
    {
        if( ZD0 == CD ) {
            // for AP2 direct set BW > -0.5 .. -0.9xxx
            km3[CD][ index & filterCountMask ][CH] = v;
        } else if( ZD1 == CD ) {
            km3[CD][ index & filterCountMask ][CH] = v;
        } else {
            static_assert(CD==0 || CD==1,"illegal parameter");
        }
    }
#endif
    // parameter set with frequency -- only for testing -- slow
    template< std::size_t CH, std::size_t CD >
    inline void setFreq( float fc, std::size_t index=0 )
    {
        if( ZD0 == CD ) {
            km3[CD][ index & filterCountMask ][CH] = FilterTable::fc_sinpercosPi2_F( fc );
        } else if( ZD1 == CD ) {
            km3[CD][ index & filterCountMask ][CH] = FilterTable::fc_cosPi2_F( fc );
        } else {
            static_assert(CD==0 || CD==1,"illegal parameter");
        }
    }

    // parameter set with ycent
    template< std::size_t CH, std::size_t CD >
    inline void setYcent( int32_t ycent, std::size_t index=0 )
    {
        if( ZD0 == CD ) {
            // TODO should be < -0.5 .. -0.99xx - bandwidth for AP2
            km3[CD][ index & filterCountMask ][CH] = FilterTableSinCosPi2::getInstance().getFloat( ycent );
        } else if( ZD1 == CD ) {
            // TODO only this make sense - f control AP2
            km3[CD][ index & filterCountMask ][CH] = FilterTableCos2Pi::getInstance().getFloat( ycent );
        } else {
            static_assert(CD==0 || CD==1,"illegal parameter");
        }
    }

    // index 0..2^k-1 -- filter  2^k -- channel
    template< std::size_t CD >
    inline void setYcent( int32_t ycent, std::size_t index )
    {
        static_assert(sizeof(km2) == sizeof(km3), "different sizes");
        if( ZD0 == CD ) {
            // TODO should be < -0.5 .. -0.99xx - bandwidth AP2
            km2[CD][ index & allFilterCountMask ] = FilterTableSinCosPi2::getInstance().getFloat( ycent );
        } else if( ZD1 == CD ) {
            // TODO only this make sense - f control Ap2
            km2[CD][ index & allFilterCountMask ] = FilterTableCos2Pi::getInstance().getFloat( ycent );
        } else {
            static_assert(CD==0 || CD==1,"illegal parameter");
        }
    }

    template< std::size_t CD >
    inline void setYcentAll( int32_t ycent, int32_t ycentDelta )
    {
        static_assert(sizeof(km2) == sizeof(km3), "different sizes");
        if( ZD0 == CD ) {
            for( auto fi=0u; fi < allFilterCount; ++fi ) {
                km2[CD][ fi ] = FilterTableSinCosPi2::getInstance().getFloat( ycent );
                ycent += ycentDelta;
            }
        } else if( ZD1 == CD ) {
            for( auto fi=0u; fi < allFilterCount; ++fi ) {
                km2[CD][ fi ] = FilterTableCos2Pi::getInstance().getFloat( ycent );
                ycent += ycentDelta;
            }
        } else {
            static_assert(CD==0 || CD==1,"illegal parameter");
        }
    }

    inline void setFeedback( float fb )
    {
        feedback = fb;
    }

//----------------------------------------------
// 1st order single

    template< std::size_t CH >
    inline void allpass1_transposed( const float x0, float& y0 )
    {
        y0 = x0 * km3[ZD0][FL0][CH] + zm3[ZD0][FL0][CH];
        zm3[ZD0][FL0][CH] = x0 - y0 * km3[ZD0][FL0][CH];
    }

    template< std::size_t CH >
    inline void allpass1_direct( const float x0, float& y0 )
    {
        const float t0 = x0 - zm3[ZD0][FL0][CH] * km3[ZD0][FL0][CH];
        y0 = t0 * km3[ZD0][FL0][CH] + zm3[ZD0][FL0][CH];
        zm3[ZD0][FL0][CH] = t0;
    }

    // -----------------
    // 1 multiplier is the best for 1st order -- default
    template< std::size_t CH >
    inline void allpass1( const float x0, float& y0 )
    {            
        static_assert(channelCount>CH,"channel count low");
        const float t0 = ( x0 - zm3[ZD0][FL0][CH] ) * km3[ZD0][FL0][CH];
        y0 = t0 + zm3[ZD0][FL0][CH];
        zm3[ZD0][FL0][CH] = t0 + x0;
    }

    // 2 channel - stereo special case
    inline void allpass1( const float x0, const float x1, float& y0, float& y1 )
    {          
        static_assert(channelCount>1,"channel count low");
        const float t0 = ( x0 - zm3[ZD0][FL0][CH0] ) * km3[ZD0][FL0][CH0];
        const float t1 = ( x1 - zm3[ZD0][FL0][CH1] ) * km3[ZD0][FL0][CH1];
        y0 = t0 + zm3[ZD0][FL0][CH0];
        y1 = t1 + zm3[ZD0][FL0][CH1];
        zm3[ZD0][FL0][CH0] = t0 + x0;
        zm3[ZD0][FL0][CH1] = t1 + x1;
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
        const float t0 = x0 + zm3[ZD0][FL0][CH] * km3[ZD0][FL0][CH];
        y0 = zm3[ZD0][FL0][CH] - t0 * km3[ZD0][FL0][CH];
        // 2nd stage
        const float t1 = t0 + zm3[ZD1][FL0][CH] * km3[ZD1][FL0][CH];
        zm3[ZD0][FL0][CH] = zm3[ZD1][FL0][CH] - t1 * km3[ZD1][FL0][CH];
        zm3[ZD1][FL0][CH] = t1;
    }

    template< std::size_t CH >
    inline void allpass2_1mult( const float x0, float& y0 )
    {
        static_assert(stateCount>1,"state count low");
        static_assert(channelCount>CH,"channel count low");
        // 1th stage -- tan
        const float t00 = ( zm3[ZD0][FL0][CH] - x0 ) * km3[ZD0][FL0][CH];
        y0 = t00 + zm3[ZD0][FL0][CH];
        // 2nd stage -- cos
        const float t001 = t00 + x0;
        const float t01 = ( zm3[ZD1][FL0][CH] - t001 ) * km3[ZD1][FL0][CH];
        zm3[ZD0][FL0][CH] = t01 + zm3[ZD1][FL0][CH];
        zm3[ZD1][FL0][CH] = t01 + t001;
    }

    // 1 stage > transposed
    // 2 stage > 1mult
    template< std::size_t CH >
    inline void allpass2_1mult_transposed( const float x0, float& y0 )
    {
        static_assert(stateCount>1,"state count low");
        static_assert(channelCount>CH,"channel count low");
        // 1th stage -- tan
        y0 = zm3[ZD0][FL0][CH] - x0 * km3[ZD0][FL0][CH]; // - ( tan-1)/(tan+1) !!
        const float t0 = x0 + y0 * km3[ZD0][FL0][CH]; // - ( tan-1)/(tan+1) !!
        // 2nd stage
        const float t01 = ( zm3[ZD1][FL0][CH] - t0 ) * km3[ZD1][FL0][CH];
        zm3[ZD0][FL0][CH] = t01 + zm3[ZD1][FL0][CH];
        zm3[ZD1][FL0][CH] = t01 + t0;
    }

    //transposed is the best for 2nd order
    template< std::size_t CH >
    inline void allpass2( const float x0, float& y0 )
    {
        static_assert(stateCount>1,"state count low");
        static_assert(channelCount>CH,"channel count low");
        // 1th stage
        y0 = zm3[ZD0][FL0][CH] - x0 * km3[ZD0][FL0][CH]; // - ( tan-1)/(tan+1) !!
        // 2nd stage
        const float t0 = x0 + y0 * km3[ZD0][FL0][CH]; // - ( tan-1)/(tan+1) !!
        zm3[ZD0][FL0][CH] = zm3[ZD1][FL0][CH] - t0 * km3[ZD1][FL0][CH];  // - cos !!
        zm3[ZD1][FL0][CH] = t0 + zm3[ZD0][FL0][CH] * km3[ZD1][FL0][CH];  // - cos !!
    }

    inline void allpass2( const float x0, const float x1, float& y0, float& y1 )
    {
        static_assert(stateCount>1,"state count low");
        static_assert(channelCount>1,"channel count low");
        // 1th stage
        y0 = zm3[ZD0][FL0][CH0] - x0 * km3[ZD0][FL0][CH0];
        y1 = zm3[ZD0][FL0][CH1] - x1 * km3[ZD0][FL0][CH1];
        // 2nd stage
        const float t0 = x0 + y0 * km3[ZD0][FL0][CH0];
        const float t1 = x1 + y1 * km3[ZD0][FL0][CH1];
        zm3[ZD0][FL0][CH0] = zm3[ZD1][FL0][CH0] - t0 * km3[ZD1][FL0][CH0];
        zm3[ZD0][FL0][CH1] = zm3[ZD1][FL0][CH1] - t1 * km3[ZD1][FL0][CH1];
        zm3[ZD1][FL0][CH0] = t0 + zm3[ZD0][FL0][CH0] * km3[ZD1][FL0][CH0];
        zm3[ZD1][FL0][CH1] = t1 + zm3[ZD0][FL0][CH1] * km3[ZD1][FL0][CH1];
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
    
    // TODO > check the sign of k coeffs
    inline void allpass2x8( const float x0, const float x1, float& y0, float& y1 )
    {
        static_assert(stateCount>1,"state count low");
        static_assert(channelCount>1,"channel count low");
        static_assert(filterCount>3,"filter count low");
        static_assert(sizeof(km2) == sizeof(km3), "different sizes");

        // temp storage for stage 1 -> 2
        union {
            v4sf    tv4[2];
            float   t[4][2];
        };
//        std::cout << "allpass2x8 k0 " << km3[0][0][0] << " k1 " << km3[1][0][0]  << std::endl;
     
        // 1th stage
        const float t0y0 = x0 + feedback * ylast[CH0];
        const float t0y1 = x1 + feedback * ylast[CH1];
        
        const float t1y0 = zm3[ZD0][FL0][CH0] - t0y0 * km3[ZD0][FL0][CH0];
        const float t1y1 = zm3[ZD0][FL0][CH1] - t0y1 * km3[ZD0][FL0][CH1];
        t[FL0][CH0] = t0y0 + t1y0 * km3[ZD0][FL0][CH0];
        t[FL0][CH1] = t0y1 + t1y1 * km3[ZD0][FL0][CH1];
        
        const float t2y0 = zm3[ZD0][FL1][CH0] - t1y0 * km3[ZD0][FL1][CH0];
        const float t2y1 = zm3[ZD0][FL1][CH1] - t1y1 * km3[ZD0][FL1][CH1];
        t[FL1][CH0] = t1y0 + t2y0 * km3[ZD0][FL1][CH0];
        t[FL1][CH1] = t1y1 + t2y1 * km3[ZD0][FL1][CH1];
        
        const float t3y0 = zm3[ZD0][FL2][CH0] - t2y0 * km3[ZD0][FL2][CH0];
        const float t3y1 = zm3[ZD0][FL2][CH1] - t2y1 * km3[ZD0][FL2][CH1];
        t[FL2][CH0] = t2y0 + t3y0 * km3[ZD0][FL2][CH0];
        t[FL2][CH1] = t2y1 + t3y1 * km3[ZD0][FL2][CH1];
        
        ylast[CH0] = y0  = zm3[ZD0][FL3][CH0] - t3y0 * km3[ZD0][FL3][CH0];
        ylast[CH1] = y1  = zm3[ZD0][FL3][CH1] - t3y1 * km3[ZD0][FL3][CH1];
        t[FL3][CH0] = t3y0 + y0   * km3[ZD0][FL3][CH0];
        t[FL3][CH1] = t3y1 + y1   * km3[ZD0][FL3][CH1];

        // 2nd stage - state 0
        // zv4[0]          = zv4[2]             - tv4[0]      * kv4[2]
        // zv4[0]  = zv4[2] - tv4[0] * kv4[2];
        zm3[ZD0][FL0][CH0] = zm3[ZD1][FL0][CH0] - t[FL0][CH0] * km3[ZD1][FL0][CH0];
        zm3[ZD0][FL0][CH1] = zm3[ZD1][FL0][CH1] - t[FL0][CH1] * km3[ZD1][FL0][CH1];
        zm3[ZD0][FL1][CH0] = zm3[ZD1][FL1][CH0] - t[FL1][CH0] * km3[ZD1][FL1][CH0];
        zm3[ZD0][FL1][CH1] = zm3[ZD1][FL1][CH1] - t[FL1][CH1] * km3[ZD1][FL1][CH1];
        // zv4[1]          = zv4[3]             - tv4[1]      * kv4[3]
        // zv4[1]  = zv4[3] - tv4[1] * kv4[3];
        zm3[ZD0][FL2][CH0] = zm3[ZD1][FL2][CH0] - t[FL2][CH0] * km3[ZD1][FL2][CH0];
        zm3[ZD0][FL2][CH1] = zm3[ZD1][FL2][CH1] - t[FL2][CH1] * km3[ZD1][FL2][CH1];
        zm3[ZD0][FL3][CH0] = zm3[ZD1][FL3][CH0] - t[FL3][CH0] * km3[ZD1][FL3][CH0];
        zm3[ZD0][FL3][CH1] = zm3[ZD1][FL3][CH1] - t[FL3][CH1] * km3[ZD1][FL3][CH1];

        // 2nd stage - state 1
        // zv4[2]          = tv4[0]      + zv4[0]             * kv4[2]
        // zv4[2]  = tv4[0] + zv4[0] * kv4[2];
        zm3[ZD1][FL0][CH0] = t[FL0][CH0] + zm3[ZD0][FL0][CH0] * km3[ZD1][FL0][CH0];
        zm3[ZD1][FL0][CH1] = t[FL0][CH1] + zm3[ZD0][FL0][CH1] * km3[ZD1][FL0][CH1];
        zm3[ZD1][FL1][CH0] = t[FL1][CH0] + zm3[ZD0][FL1][CH0] * km3[ZD1][FL1][CH0];
        zm3[ZD1][FL1][CH1] = t[FL1][CH1] + zm3[ZD0][FL1][CH1] * km3[ZD1][FL1][CH1];

        // zv4[3]          = tv4[1]      + zv4[1]             * kv4[3]
        // zv4[3]  = tv4[1] + zv4[1] * kv4[3]; 
        zm3[ZD1][FL2][CH0] = t[FL2][CH0] + zm3[ZD0][FL2][CH0] * km3[ZD1][FL2][CH0];
        zm3[ZD1][FL2][CH1] = t[FL2][CH1] + zm3[ZD0][FL2][CH1] * km3[ZD1][FL2][CH1];
        zm3[ZD1][FL3][CH0] = t[FL3][CH0] + zm3[ZD0][FL3][CH0] * km3[ZD1][FL3][CH0];
        zm3[ZD1][FL3][CH1] = t[FL3][CH1] + zm3[ZD0][FL3][CH1] * km3[ZD1][FL3][CH1];        
    }

//----------------------------------------------
//private:
    union alignas(cacheLineSize) {
        struct {
            union {
                v4sf    kv4[ v4kcount ];
                float   kv[  coeffCount * allFilterCount ];
                float   km2[ coeffCount ][ allFilterCount ];
                float   km3[ coeffCount ][ filterCount ][ channelCount ];
            };
            union {
                v4sf    zv4[ v4zcount ];
                float   zv[  stateCount * allFilterCount ];
                float   zm2[ stateCount][ allFilterCount ];
                float   zm3[ stateCount][ filterCount ][ channelCount ];                
            };
        };
    };

    float ylast[ channelCount ];  // feedback storage
    float feedback;  // feedback coeff
};



} // end namespace
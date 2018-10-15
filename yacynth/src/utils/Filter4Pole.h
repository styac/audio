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
 * File:   Filter4Pole.h
 * Author: Istvan Simon
 *
 * Created on March 21, 2016, 6:27 PM
 */

#include "FilterBase.h"
#include "Limiters.h"
#include "v4.h"

#include <cstdint>
#include <iostream>
#include <iomanip>

using namespace limiter;

namespace filter {

// --------------- NEW

template< std::size_t filterCountExp, std::size_t channelCountExp >
class Filter4Pole : public FilterBase {
public:
    static constexpr  std::size_t filterCount           = 1<<filterCountExp;
    static constexpr  std::size_t filterCountMask       = filterCount-1;
    static constexpr  std::size_t stateCountExp         = 4;
    static constexpr  std::size_t stateCount            = 1<<stateCountExp;
    static constexpr  std::size_t coeffCount            = 2;
    static constexpr  std::size_t coeffCountMask        = coeffCount-1;
    static constexpr  std::size_t channelCount          = 1<<channelCountExp;
    static constexpr  std::size_t channelCountMask      = channelCount-1;
    static constexpr  std::size_t v4kcount              = (filterCount*coeffCount*channelCount)/4;
    static constexpr  std::size_t v4zcount              = (filterCount*stateCount*channelCount)/4;
    static constexpr  std::size_t allFilterCount        = 1<<(filterCountExp+channelCountExp);
    static constexpr  std::size_t allFilterCountMask    = allFilterCount-1;
    static constexpr  std::size_t allStateCount         = 1<<(filterCountExp+channelCountExp+stateCountExp);
    
    // fcontrolMin, fcontrolMin supposed to be made by table 
    static constexpr float  fcontrolMin  = std::exp(-PI2*fcMin);
    static constexpr float  fcontrolMax  = std::exp(-PI2*fcMax);
    static constexpr float  qcontrolMin  = 0.0;
    static constexpr float  qcontrolMax  = 3.996; // checked - try to get the max limit !
    
    Filter4Pole()
    {
        clear();
    };

    void clear(void)
    {
        for( auto &v : zv ) v = 0;
    };

    // CD0 = freq
    // CD1 = Q
    
    inline void set_F_K( float fval, std::size_t index=0 )
    {
        km2[CD0][ index & allFilterCountMask ] = std::max( fcontrolMax, std::min( fcontrolMin, fval ));
    }

    // qval = 0..4.0
    inline void set_Q_K( float qval, std::size_t index=0 )
    {
        km2[CD1][ index & allFilterCountMask ] = std::max( qcontrolMin, std::min( qcontrolMax, qval ));
    }    

    template< std::size_t CH >
    inline void set_F( int32_t ycent, std::size_t index=0 )
    {
        const float f = FilterTableExp2Pi::getInstance().getFloat( ycent );
        km3[CD0][ index & filterCountMask ][CH] = f; // std::max( fcontrolMax, std::min( fcontrolMin, f ));
    }

    // qval = 0.0 .. 1.0
    template< std::size_t CH >
    inline void set_Q( float qval, std::size_t index=0 )
    {
        const float q = qval * qcontrolMax;
        km3[CD1][ index & filterCountMask ][CH] = q;
    }
    
    inline void set_F( int32_t ycent, std::size_t index=0 )
    {
        const float f = FilterTableExp2Pi::getInstance().getFloat( ycent );
        km2[CD0][ index & allFilterCountMask ] = f; //std::max( fcontrolMax, std::min( fcontrolMin, f ));
    }

    // qval = 0.0 .. 1.0
    inline void set_Q( float qval, std::size_t index=0 )
    {
        const float q = qval * qcontrolMax;
        km2[CD1][ index & allFilterCountMask ] = q;
    }

    inline void set_F_all( int32_t ycent, int32_t ycentDelta )
    {
        for( auto fi=0u; fi < allFilterCount; ++fi ) {
            const float f = FilterTableExp2Pi::getInstance().getFloat( ycent ) * 2.0f;
            km2[CD0][ fi ] = f;// std::max( fcontrolMax, std::min( fcontrolMin, f ));
            ycent += ycentDelta;
        }
    }

    // qval = 0.0 .. 1.0
    inline void set_Q_all( float qval )
    {
        const float q = qval * qcontrolMax;
        for( auto fi=0u; fi < allFilterCount; ++fi ) {
            km2[CD1][ fi ] = q;
        }        
    }

    // new version
    // get band pass
    template< std::size_t FL, std::size_t CD >
    inline void get2x1BP32( float& y0, float& y1, const float gain = 1.0f )
    {
        static_assert(CD >= 1 && CD <= 2, "illegal pole number" );
        y0 = ( zm3[CD][FL][CH0] - zm3[CD+1][FL][CH0] ) * gain;
        y1 = ( zm3[CD][FL][CH1] - zm3[CD+1][FL][CH1] ) * gain;        
    }

    template< std::size_t FL, std::size_t CD >
    inline void get2x1BP32ADD( float& y0, float& y1, const float gain = 1.0f )
    {
        static_assert(CD >= 1 && CD <= 2, "illegal pole number" );
        y0 += ( zm3[CD][FL][CH0] - zm3[CD][FL][CH0] ) * gain;
        y1 += ( zm3[CD][FL][CH1] - zm3[CD][FL][CH1] ) * gain;        
    }

    template< std::size_t FL, std::size_t CD >
    inline void get1x1BP32( float& y0, const float gain = 1.0f )
    {
        static_assert(CD >= 1 && CD <= 2, "illegal pole number" );
        y0 = ( zm3[CD][FL][CH0] - zm3[CD+1][FL][CH0] ) * gain;
    }

    template< std::size_t FL, std::size_t CD >
    inline void get1x1BP32ADD( float& y0, const float gain = 1.0f )
    {
        static_assert(CD >= 1 && CD <= 2, "illegal pole number" );
        y0 += ( zm3[CD][FL][CH0] - zm3[CD][FL][CH0] ) * gain;
    }

    
    // get low pass
    template< std::size_t FL >
    inline void get2x1LP( float& y0, float& y1 )
    {
        y0 = zm3[ZD3][FL][CH0];
        y1 = zm3[ZD3][FL][CH1];
    }

    template< std::size_t FL >
    inline void get2x1LPADD( float& y0, float& y1 )
    {
        y0 += zm3[ZD3][FL][CH0];
        y1 += zm3[ZD3][FL][CH1];
    }

    template< std::size_t FL >
    inline void get1x1LP( float& y0 )
    {
        y0 = zm2[ZD3][FL];
    }

    template< std::size_t FL >
    inline void get1x1LPADD( float& y0 )
    {
        y0 += zm2[ZD3][FL];
    }
        
    template< std::size_t FL >
    inline void set2x1( const float x0, const float x1 )
    {
        const float inx0 = x0 - zm3[ZD3][FL][CH0] * km3[CD1][FL][CH0];
        const float inx1 = x1 - zm3[ZD3][FL][CH1] * km3[CD1][FL][CH1];
        zm3[ZD0][FL][CH0] = ( zm3[ZD0][FL][CH0] - inx0 ) * km3[CD0][FL][CH0] + inx0;
        zm3[ZD0][FL][CH1] = ( zm3[ZD0][FL][CH1] - inx1 ) * km3[CD0][FL][CH1] + inx1;        
        zm3[ZD1][FL][CH0] = ( zm3[ZD1][FL][CH0] - zm3[ZD0][FL][CH0] ) * km3[CD0][FL][CH0] + zm3[ZD0][FL][CH0];
        zm3[ZD1][FL][CH1] = ( zm3[ZD1][FL][CH1] - zm3[ZD0][FL][CH1] ) * km3[CD0][FL][CH1] + zm3[ZD0][FL][CH1];        
        zm3[ZD2][FL][CH0] = ( zm3[ZD2][FL][CH0] - zm3[ZD1][FL][CH0] ) * km3[CD0][FL][CH0] + zm3[ZD1][FL][CH0];
        zm3[ZD2][FL][CH1] = ( zm3[ZD2][FL][CH1] - zm3[ZD1][FL][CH1] ) * km3[CD0][FL][CH1] + zm3[ZD1][FL][CH1];
        zm3[ZD3][FL][CH0] = ( zm3[ZD3][FL][CH0] - zm3[ZD2][FL][CH0] ) * km3[CD0][FL][CH0] + zm3[ZD2][FL][CH0];
        zm3[ZD3][FL][CH1] = ( zm3[ZD3][FL][CH1] - zm3[ZD2][FL][CH1] ) * km3[CD0][FL][CH1] + zm3[ZD2][FL][CH1];
    }

    // 1 channel lin address
    
    template< std::size_t FL, std::size_t CD >
    inline void get2x1BP32( float& y0, const float gain = 1.0f )
    {
        static_assert(CD >= 1 && CD <= 2, "illegal pole number" );
        y0 = ( zm2[CD][FL] - zm2[CD+1][FL] ) * gain;
    }

    template< std::size_t FL, std::size_t CD >
    inline void get2x1BP32ADD( float& y0, const float gain = 1.0f )
    {
        static_assert(CD >= 1 && CD <= 2, "illegal pole number" );
        y0 += ( zm2[CD][FL] - zm2[CD][FL] ) * gain;
    }

    // get low pass
    template< std::size_t FL >
    inline void get2x1LP( float& y0 )
    {
        y0 = zm2[ZD3][FL];
   }

    template< std::size_t FL >
    inline void get2x1LPADD( float& y0 )
    {
        y0 += zm2[ZD3][FL];
    }    
    template< std::size_t FL >
    inline void set1x1( const float x0 )
    {
        
        const float inx0 = x0 - zm2[ZD3][FL] * km2[CD1][FL];
        zm2[ZD0][FL] = ( zm2[ZD0][FL] - inx0 ) * km2[CD0][FL] + inx0;
        zm2[ZD1][FL] = ( zm2[ZD1][FL] - zm2[ZD0][FL] ) * km2[CD0][FL] + zm2[ZD0][FL];
        zm2[ZD2][FL] = ( zm2[ZD2][FL] - zm2[ZD1][FL] ) * km2[CD0][FL] + zm2[ZD1][FL];
        zm2[ZD3][FL] = ( zm2[ZD3][FL] - zm2[ZD2][FL] ) * km2[CD0][FL] + zm2[ZD2][FL];
    }
    
private:    
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
}; // end Filter4pole

// --------------------------------------------------------------------
} // end namespace filter
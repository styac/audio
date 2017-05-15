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
 * File:   FilterStateVariable.h
 * Author: Istvan Simon
 *
 * Created on March 15, 2016, 6:59 PM
 */

#include    "FilterBase.h"

#include    <cstdint>
#include    <iostream>
#include    <iomanip>

// http://courses.cs.washington.edu/courses/cse490s/11au/Readings/Digital_Sound_Generation_2.pdf
// Beat Frei: Digital Sound Generation – Part 2 chap 3.3 - page 13 - 14
//      0 < Fc <= 1
//      0.2 < Dc <= 2
//
// what         case PEEKPIN:
//            return ( channelA.low2 - channelA.high1 + in ) * 0.03f;

namespace filter {
  
//    
// with 2x oversampling    
// input ycent must be corrected by the caller !!!!! 
// the translation table is for normal
// ycent2xOVS = ycent - (1<<24) -- 1 octave lower
//    

template< std::size_t filterCountExp, std::size_t channelCountExp >
class FilterStateVariable2xOVS : public FilterBase {
public:
    static constexpr  std::size_t stateCount            = 3;
    static constexpr  std::size_t coeffCount            = 3; // F,D,Dc
    static constexpr  std::size_t filterCount           = 1<<filterCountExp;
    static constexpr  std::size_t filterCountMask       = filterCount-1;
    static constexpr  std::size_t channelCount          = 1<<channelCountExp;
    static constexpr  std::size_t channelCountMask      = channelCount-1;
    static constexpr  std::size_t v4kcount              = (filterCount*coeffCount*channelCount)/4;
    static constexpr  std::size_t v4zcount              = (filterCount*stateCount*channelCount)/4;
    static constexpr  std::size_t allFilterCount        = filterCount*channelCount;
    static constexpr  std::size_t allFilterCountMask    = allFilterCount-1;
    static constexpr  std::size_t allStateCount         = filterCount*channelCount*stateCount;
        
    static constexpr float  fcontrolMin  = 0.0000001;
    static constexpr float  fcontrolMax  = 0.9999999;
    static constexpr float  qcontrolMin  = 0.03;
    static constexpr float  qcontrolMax  = 2.0;
    
    // to refine
    static float D_svf( float Fc, float Dc ) 
    {
        return 2.0f * Dc * ( 1.0 - Fc )  + 0.03f;
    }
    
    FilterStateVariable2xOVS()
    {
        clear();
    };

    void clear(void)
    {
        for( auto &v : zv ) v = 0;
    };
    
    // for direct setting
    inline void set_F_K( float fval, std::size_t index=0 )
    {
        km2[CDF][ index & allFilterCountMask ] = std::max( fcontrolMax, std::min( fcontrolMin, fval ));
    }

    // for direct setting
    inline void set_Q_K( float qval, std::size_t index=0 )
    {
        km2[CDD][ index & allFilterCountMask ] = km2[CDDCIN][ index & allFilterCountMask ] = std::max( qcontrolMin, std::min( qcontrolMax, qval ));
    }    

    template< std::size_t CH >
    inline void set_F( int32_t ycent, std::size_t index=0 )
    {
        km3[ CDF ][ index & filterCountMask ][ CH ] = FilterTable2SinPi::getInstance().getFloat( ycent );
    }

    template< std::size_t CH >
    inline void set_Q( float q, std::size_t index=0 )
    {
        km3[ CDDCIN ][ index & filterCountMask ][ CH ] = q;
        km3[ CDD ][ index & filterCountMask ][ CH ] = D_svf( km3[ CDF ][ index & filterCountMask ][ CH ], q );
    }
    
    inline void set_F( int32_t ycent, std::size_t index=0 )
    {
        km2[ CDF ][ index & filterCountMask ] = FilterTable2SinPi::getInstance().getFloat( ycent );
        km2[ CDD ][ index & filterCountMask ] = D_svf( km2[ CDF ][ index & filterCountMask ], km2[ CDDCIN ][ index & filterCountMask ] );
        std::cout << " ***          set_F " << km2[ CDF ][ index & filterCountMask ] << std::endl;
        std::cout << " ***          set_Q " << km2[ CDD ][ index & filterCountMask ] << " QC " << km2[ CDDCIN ][ index & filterCountMask ] << std::endl;
    }
    
    inline void set_Q( float q, std::size_t index=0 )
    {
        km2[ CDDCIN ][ index & filterCountMask ] = q;        
        km2[ CDD ][ index & filterCountMask ] = D_svf( km2[ CDF ][ index & filterCountMask ], q );
        std::cout << " ***          set_F " << km2[ CDF ][ index & filterCountMask ] << std::endl;
        std::cout << " ***          set_Q " << km2[ CDD ][ index & filterCountMask ] << " QC " << km2[ CDDCIN ][ index & filterCountMask ] << std::endl;

    }

    inline void set_F_all( int32_t ycent, int32_t ycentDelta )
    {
        for( auto fi=0u; fi < allFilterCount; ++fi ) {
            km2[ CDF ][ fi ] = FilterTable2SinPi::getInstance().getFloat( ycent );
            ycent += ycentDelta;
        }
    }

    inline void set_Q_all( float q )
    {
        const float qval = std::max( qcontrolMin, std::min( qcontrolMax,  2.0f - 2.0f * q ));
        for( auto fi=0u; fi < allFilterCount; ++fi ) {
            km2[ CDDCIN ][ fi ] = qval;
            km2[ CDD ][ fi ] = D_svf( km2[ CDF ][ fi ], km2[ CDDCIN ][ fi ]);
        }        
    }
    
    template< std::size_t FL >
    inline float get1x1LP()
    {
        return zm2[ LOW ][ FL ];
    }

    template< std::size_t FL >
    inline float get1x1BP1()
    {
        return zm2[ BND ][ FL ] * 2.0f;
    }


    template< std::size_t FL >
    inline float get1x1HP()
    {
        return zm2[ HGH ][ FL ];
    }
    
    template< std::size_t FL >
    inline float get1x1PK()
    {
        return zm2[ BND ][ FL ] - zm2[ HGH ][ FL ];
    }

    template< std::size_t FL >
    inline float get1x1NT()
    {
        return zm2[ BND ][ FL ] + zm2[ HGH ][ FL ];
    }

    template< std::size_t FL >
    inline void get1x1LP( float& y0 )
    {
        y0 = zm2[ LOW ][ FL ];
    }

    template< std::size_t FL >
    inline void get1x1BP1( float& y0 )
    {
        y0 = zm2[ BND ][ FL ];
    }

    template< std::size_t FL >
    inline void get1x1HP( float& y0 )
    {
        y0 = zm2[ HGH ][ FL ];
    }
    
    template< std::size_t FL >
    inline void get1x1PK( float& y0 )
    {
        y0 = zm2[ BND ][ FL ] - zm2[ HGH ][ FL ];
    }

    template< std::size_t FL >
    inline void get1x1NT( float& y0 )
    {
        y0 = zm2[ BND ][ FL ] + zm2[ HGH ][ FL ];
    }

    template< std::size_t FL >
    inline void set1x1( const float x0 )    
    {
        for( auto i=0u; i<2; ++i ) {
            zm2[ LOW ][ FL ] +=     zm2[ BND ][ FL ] * km2[ CDF ][ FL ];
            zm2[ HGH ][ FL ] = x0 - zm2[ BND ][ FL ] * km2[ CDD ][ FL ] - zm2[ LOW ][ FL ];
            zm2[ BND ][ FL ] +=     zm2[ HGH ][ FL ] * km2[ CDF ][ FL ];            
        }
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
}; // end FilterStateVariable
      
// --------------------------------------------------------------------
} // end namespace filter
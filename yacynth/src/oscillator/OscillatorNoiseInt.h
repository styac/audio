#pragma once

/*
 * Copyright (C) 2016 Istvan Simon -- stevens37 at gmail dot com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed x the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/*
 * File:   OscillatorNoiseInt.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on May 27, 2016, 8:49 PM
 */
#include "utils/FilterBase.h"

using namespace tables;
using namespace filter;

typedef int     v4si __attribute__((mode(SI)))  __attribute__ ((vector_size(16),aligned(16)));

namespace yacynth {

class OscillatorNoise : public FilterBase {
public:
    static constexpr    uint8_t stateCountExp   = 3;
    static constexpr    uint8_t stateCount      = 1<<stateCountExp;

    OscillatorNoise()
    { clear(); };

    inline void clear(void)
    {
        for( auto& x : zv ) x = 0;
    }

    inline int32_t getFreqSv( const int32_t ycent )
    {
        return FilterTable2SinPi::getInstance().getInt( ycent );
    }
    
    inline int32_t getFreqAp2( const int32_t ycent )
    {
        return FilterTableCos2Pi::getInstance().getInt( ycent );
    }
    
    inline int32_t getFreq4p( const int32_t ycent ) 
    {
        return FilterTableExp2Pi::getInstance().getInt( ycent );
    }

    inline int32_t getFiltered4Px1( const int32_t x, int32_t f, int16_t b )
    {   
        constexpr uint8_t scaleExpF   = 31;
        constexpr int64_t scaleRoundF = 1<<(scaleExpF-1);          
        constexpr uint8_t scaleExpB   = 13; // 2 digit int
        constexpr int64_t scaleRoundB = 1LL<<(scaleExpB-1);
        const int32_t t0 = x - (( int64_t(zv[ 3 ]) * b + scaleRoundB ) >> scaleExpB);
        zv[ 0 ]          = ((int64_t( zv[ 0 ] - t0      ) * f + scaleRoundF ) >> scaleExpF ) + t0;
        zv[ 1 ]          = ((int64_t( zv[ 1 ] - zv[ 0 ] ) * f + scaleRoundF ) >> scaleExpF ) + zv[ 0 ];
        zv[ 2 ]          = ((int64_t( zv[ 2 ] - zv[ 1 ] ) * f + scaleRoundF ) >> scaleExpF ) + zv[ 1 ]; 
        const int32_t y  = ((int64_t( zv[ 3 ] - zv[ 2 ] ) * f + scaleRoundF ) >> scaleExpF );
        zv[ 3 ]          = y + zv[ 2 ];                    
        return y; 
    }; 
        
    inline int32_t getFiltered4Px2( const int32_t x, int32_t f, int16_t b )
    {   
        constexpr uint8_t scaleExpF   = 31;
        constexpr int64_t scaleRoundF = 1<<(scaleExpF-1);          
        constexpr uint8_t scaleExpB   = 13; // 2 digit int
        constexpr int64_t scaleRoundB = 1LL<<(scaleExpB-1);
        
        const int32_t t0 = x  - (( int64_t(zv[ 3 ]) * b + scaleRoundB ) >> scaleExpB);
        zv[ 0 ]          = ((int64_t( zv[ 0 ] - t0      ) * f + scaleRoundF ) >> scaleExpF ) + t0;
        zv[ 1 ]          = ((int64_t( zv[ 1 ] - zv[ 0 ] ) * f + scaleRoundF ) >> scaleExpF ) + zv[ 0 ];
        zv[ 2 ]          = ((int64_t( zv[ 2 ] - zv[ 1 ] ) * f + scaleRoundF ) >> scaleExpF ) + zv[ 1 ]; 
        const int32_t y0 = ((int64_t( zv[ 3 ] - zv[ 2 ] ) * f + scaleRoundF ) >> scaleExpF );
        zv[ 3 ]          = y0 + zv[ 2 ];                    

        const int32_t t1 = y0 - (( int64_t(zv[ 7 ]) * b + scaleRoundB ) >> scaleExpB);
        zv[ 4 ]          = ((int64_t( zv[ 4 ] - t1      ) * f + scaleRoundF ) >> scaleExpF ) + t1;
        zv[ 5 ]          = ((int64_t( zv[ 5 ] - zv[ 4 ] ) * f + scaleRoundF ) >> scaleExpF ) + zv[ 4 ];
        zv[ 6 ]          = ((int64_t( zv[ 6 ] - zv[ 5 ] ) * f + scaleRoundF ) >> scaleExpF ) + zv[ 5 ]; 
        const int32_t y1 = ((int64_t( zv[ 7 ] - zv[ 6 ] ) * f + scaleRoundF ) >> scaleExpF );
        zv[ 7 ]          = y1 + zv[ 6 ];                    
        return y1; 
    }; 
    
    // DOC :  Mitra Regalia thedigitalallpassfilter.pdf
    // 2nd order allpass -- 1 multiplier / order
    // f = cos 
    // b = tan/tan direct setting 0 < b < 1 -- reversed polarity
    //
    inline int32_t getFilteredAP2x1( int32_t x, int32_t f, int16_t b )
    {
        constexpr uint8_t scaleExpF   = 31;
        constexpr int64_t scaleRoundF = 1LL<<(scaleExpF-1);
        constexpr uint8_t scaleExpB   = 15;
        constexpr int64_t scaleRoundB = 1LL<<(scaleExpB-1);
        const int32_t t0 = (int64_t( x  - zv[ 0 ] ) * b + scaleRoundB ) >> scaleExpB; // reversed polarity> b >= 0
        const int32_t y  = x - t0 - zv[ 0 ];
        const int32_t t1 = t0 + x;
        const int32_t t2 = (int64_t( zv[ 1 ] - t1 ) * f + scaleRoundF ) >> scaleExpF;
        zv[ 0 ] = t2 + zv[ 1 ];
        zv[ 1 ] = t2 + t1;
        return y;
    }    

    inline int32_t getFilteredAP2x2( int32_t x, int32_t f, int16_t b )
    {
        constexpr uint8_t scaleExpF   = 31;
        constexpr int64_t scaleRoundF = 1LL<<(scaleExpF-1);
        constexpr uint8_t scaleExpB   = 15;
        constexpr int64_t scaleRoundB = 1LL<<(scaleExpB-1);
        
        const int32_t t00 = (int64_t( x   - zv[ 0 ] ) * b + scaleRoundB ) >> scaleExpB; // reversed polarity> b >= 0
        const int32_t y00 = x - t00 - zv[ 0 ];
        const int32_t t01 = t00 + x;
        const int32_t t02 = (int64_t( zv[ 1 ] - t01 ) * f + scaleRoundF ) >> scaleExpF;
        zv[ 0 ] = t02 + zv[ 1 ];
        zv[ 1 ] = t02 + t01;
        
        const int32_t t10 = (int64_t( y00 - zv[ 2 ] ) * b + scaleRoundB ) >> scaleExpB; // reversed polarity> b >= 0
        const int32_t y10 = y00 - t10 - zv[ 2 ];
        const int32_t t11 = t10 + y00;
        const int32_t t12 = (int64_t( zv[ 3 ] - t11 ) * f + scaleRoundF ) >> scaleExpF;
        zv[ 2 ] = t12 + zv[ 3 ];
        zv[ 3 ] = t12 + t11;
        
        return y10;
    }  
    
    inline int32_t getFilteredAP2x3( int32_t x, int32_t f, int16_t b )
    {
        constexpr uint8_t scaleExpF   = 31;
        constexpr int64_t scaleRoundF = 1LL<<(scaleExpF-1);
        constexpr uint8_t scaleExpB   = 15;
        constexpr int64_t scaleRoundB = 1LL<<(scaleExpB-1);
        
        const int32_t t00 = (int64_t( x   - zv[ 0 ] ) * b + scaleRoundB ) >> scaleExpB; // reversed polarity> b >= 0
        const int32_t y00 = x - t00 - zv[ 0 ];
        const int32_t t01 = t00 + x;
        const int32_t t02 = (int64_t( zv[ 1 ] - t01 ) * f + scaleRoundF ) >> scaleExpF;
        zv[ 0 ] = t02 + zv[ 1 ];
        zv[ 1 ] = t02 + t01;
        
        const int32_t t10 = (int64_t( y00 - zv[ 2 ] ) * b + scaleRoundB ) >> scaleExpB; // reversed polarity> b >= 0
        const int32_t y10 = y00 - t10 - zv[ 2 ];
        const int32_t t11 = t10 + y00;
        const int32_t t12 = (int64_t( zv[ 3 ] - t11 ) * f + scaleRoundF ) >> scaleExpF;
        zv[ 2 ] = t12 + zv[ 3 ];
        zv[ 3 ] = t12 + t11;
        
        const int32_t t20 = (int64_t( y10 - zv[ 4 ] ) * b + scaleRoundB ) >> scaleExpB; // reversed polarity> b >= 0
        const int32_t y20 = y10 - t20 - zv[ 4 ];
        const int32_t t21 = t20 + y10;
        const int32_t t22 = (int64_t( zv[ 5 ] - t21 ) * f + scaleRoundF ) >> scaleExpF;
        zv[ 4 ] = t22 + zv[ 5 ];
        zv[ 5 ] = t22 + t21;

        return y20;
    }  

    // TODO rearrange to make it v4 optimal -
    //  1st stage : zv[0,1,2,3] - 
    //  2nd stage : zv[4,5,6,7] - 
    inline int32_t getFilteredAP2x4( int32_t x, int32_t f, int16_t b )
    {
        constexpr uint8_t scaleExpF   = 31;
        constexpr int64_t scaleRoundF = 1LL<<(scaleExpF-1);
        constexpr uint8_t scaleExpB   = 15;
        constexpr int64_t scaleRoundB = 1LL<<(scaleExpB-1);
        
        const int32_t t00 = (int64_t( x   - zv[ 0 ] ) * b + scaleRoundB ) >> scaleExpB; // reversed polarity> b >= 0
        const int32_t y00 = x - t00 - zv[ 0 ];
        const int32_t t01 = t00 + x;
        const int32_t t02 = (int64_t( zv[ 1 ] - t01 ) * f + scaleRoundF ) >> scaleExpF;
        zv[ 0 ] = t02 + zv[ 1 ];
        zv[ 1 ] = t02 + t01;
        
        const int32_t t10 = (int64_t( y00 - zv[ 2 ] ) * b + scaleRoundB ) >> scaleExpB; // reversed polarity> b >= 0
        const int32_t y10 = y00 - t10 - zv[ 2 ];
        const int32_t t11 = t10 + y00;
        const int32_t t12 = (int64_t( zv[ 3 ] - t11 ) * f + scaleRoundF ) >> scaleExpF;
        zv[ 2 ] = t12 + zv[ 3 ];
        zv[ 3 ] = t12 + t11;
        
        const int32_t t20 = (int64_t( y10 - zv[ 4 ] ) * b + scaleRoundB ) >> scaleExpB; // reversed polarity> b >= 0
        const int32_t y20 = y10 - t20 - zv[ 4 ];
        const int32_t t21 = t20 + y10;
        const int32_t t22 = (int64_t( zv[ 5 ] - t21 ) * f + scaleRoundF ) >> scaleExpF;
        zv[ 4 ] = t22 + zv[ 5 ];
        zv[ 5 ] = t22 + t21;

        const int32_t t30 = (int64_t( y20 - zv[ 6 ] ) * b + scaleRoundB ) >> scaleExpB; // reversed polarity> b >= 0
        const int32_t y30 = y20 - t30 - zv[ 6 ];
        const int32_t t31 = t30 + y20;
        const int32_t t32 = (int64_t( zv[ 7 ] - t31 ) * f + scaleRoundF ) >> scaleExpF;
        zv[ 6 ] = t32 + zv[ 7 ];
        zv[ 7 ] = t32 + t31;

        return y30;
    }  

    // state variable filters
    
    template< uint8_t fb>
    inline int32_t getFilteredSVx1( int32_t x, int64_t f )
    {
        constexpr uint8_t feedback   = fb;
        constexpr uint8_t scaleExp   = 32;
//        constexpr int64_t scaleRound = 1LL<<(scaleExp-1); // TODO is this needed?
        zv[0] += ( zv[1] * f ) >> scaleExp;
        const int32_t t00 = x - zv[0] - ( zv[1] >> feedback );
        zv[1] += ( t00 * f ) >> scaleExp;
        const int32_t r0 = zv[1];

        zv[0] += ( zv[1] * f ) >> scaleExp;
        const int32_t t01 = x - zv[0] - ( zv[1] >> feedback );
        zv[1] += ( t01 * f ) >> scaleExp;
        return r0 + zv[1];
    };

    // state variable 2x
    template< uint8_t fb>
    inline int32_t getFilteredSVx2( int32_t x, int64_t f )
    {
        constexpr uint8_t feedback   = fb;
        constexpr uint8_t scaleExp   = 32;
//        constexpr int64_t scaleRound = 1LL<<(scaleExp-1); // TODO is this needed?

        zv[0] += ( zv[1] * f ) >> scaleExp;
        const int32_t t00 = x    - zv[0] - ( zv[1] >> feedback );
        zv[1] += ( t00 * f ) >> scaleExp;
        
        zv[2] += ( zv[3] * f ) >> scaleExp;
        const int32_t t10 = zv[1] - zv[2] - ( zv[3] >> feedback );
        zv[3] += ( t10 * f ) >> scaleExp;
        const int32_t r0 = zv[3];

        zv[0] += ( zv[1] * f ) >> scaleExp;
        const int32_t t01 = x    - zv[0] - ( zv[1] >> feedback );
        zv[1] += ( t01 * f ) >> scaleExp;
        
        zv[2] += ( zv[3] * f ) >> scaleExp;
        const int32_t t11 = zv[1] - zv[2] - ( zv[3] >> feedback );
        zv[3] += ( t11 * f ) >> scaleExp;
        return zv[3] + r0;
    };

    // state variable 3x
    template< uint8_t fb>
    inline int32_t getFilteredSVx3( int32_t x, int64_t f )
    {
        constexpr uint8_t feedback   = fb;
        constexpr uint8_t scaleExp   = 32;
//        constexpr int64_t scaleRound = 1LL<<(scaleExp-1); // TODO is this needed?

        zv[0] += ( zv[1] * f ) >> scaleExp;
        const int32_t t00 = x     - zv[0] - ( zv[1] >> feedback );
        zv[1] += ( t00 * f ) >> scaleExp;
        zv[2] += ( zv[3] * f ) >> scaleExp;
        const int32_t t10 = zv[1] - zv[2] - ( zv[3] >> feedback );
        zv[3] += ( t10 * f ) >> scaleExp;
        zv[4] += ( zv[5] * f ) >> scaleExp;
        const int32_t t20 = zv[3] - zv[4] - ( zv[5] >> feedback );
        zv[5] += ( t20 * f ) >> scaleExp;
        const int32_t r0 = zv[5];

        zv[0] += ( zv[1] * f ) >> scaleExp;
        const int32_t t01 = x     - zv[0] - ( zv[1] >> feedback );
        zv[1] += ( t01 * f ) >> scaleExp;
        zv[2] += ( zv[3] * f ) >> scaleExp;
        const int32_t t11 = zv[1] - zv[2] - ( zv[3] >> feedback );
        zv[3] += ( t11 * f ) >> scaleExp;
        zv[4] += ( zv[5] * f ) >> scaleExp;
        const int32_t t21 = zv[3] - zv[4] - ( zv[5] >> feedback );
        zv[5] += ( t21 * f ) >> scaleExp;
        return zv[5] + r0;
    };

    template< uint8_t fb>
    inline int32_t getFilteredSVx4( int32_t x, int64_t f )
    {
        constexpr uint8_t feedback   = fb;
        constexpr uint8_t scaleExp   = 32;
        constexpr int64_t scaleRound = 1LL<<(scaleExp-1); // TODO is this needed?

        zv[0] += ( zv[4] * f + scaleRound ) >> scaleExp;
        const int32_t t00 = x    - zv[0] - ( zv[4] >> feedback );
        zv[4] += ( t00 * f + scaleRound ) >> scaleExp;

        zv[1] += ( zv[5] * f + scaleRound ) >> scaleExp;
        const int32_t t10 = zv[4] - zv[1] - ( zv[5] >> feedback );
        zv[5] += ( t10 * f + scaleRound ) >> scaleExp;

        zv[2] += ( zv[6] * f + scaleRound ) >> scaleExp;
        const int32_t t20 = zv[5] - zv[2] - ( zv[6] >> feedback );
        zv[6] += ( t20 * f + scaleRound ) >> scaleExp;

        zv[3] += ( zv[7] * f + scaleRound ) >> scaleExp;
        const int32_t t30 = zv[6] - zv[3] - ( zv[7] >> feedback );
        zv[7] += ( t30 * f + scaleRound ) >> scaleExp;
        
        const int32_t r0 = zv[7];
        
        zv[0] += ( zv[4] * f + scaleRound ) >> scaleExp;
        const int32_t t01 = x    - zv[0] - ( zv[4] >> feedback );
        zv[4] += ( t01 * f + scaleRound ) >> scaleExp;

        zv[1] += ( zv[5] * f + scaleRound ) >> scaleExp;
        const int32_t t11 = zv[4] - zv[1] - ( zv[5] >> feedback );
        zv[5] += ( t11 * f + scaleRound ) >> scaleExp;

        zv[2] += ( zv[6] * f + scaleRound ) >> scaleExp;
        const int32_t t21 = zv[5] - zv[2] - ( zv[6] >> feedback );
        zv[6] += ( t21 * f + scaleRound ) >> scaleExp;

        zv[3] += ( zv[7] * f + scaleRound ) >> scaleExp;
        const int32_t t31 = zv[6] - zv[3] - ( zv[7] >> feedback );
        zv[7] += ( t31 * f + scaleRound ) >> scaleExp;
        
        return r0 + zv[7];
    };

private:
    union {
        int32_t zv[stateCount];
        v4si    zv4[stateCount/4];
    };
};

} // end namespace yacynth


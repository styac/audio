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
 * File:   BaseOscillator.h
 * Author: Istvan Simon
 *
 * Created on March 26, 2016, 10:37 AM
 */
#include    "../oscillator/Tables.h"
#include    "../utils/Primes.h"
#include    "../utils/Fastsincos.h"

#include    <cstdint>
#include    <cmath>


// TODO
//  - call with phase -> multiphase osc
//  - direct float sin -> 18 bit index

typedef int v4si __attribute__((mode(SI)))  __attribute__ ((vector_size(16),aligned(16)));

using namespace tables;

namespace yacynth {

class IntegratorOscillator {
public:
    static constexpr int cycle = 6;
    static constexpr int phaseCount = 16;

    IntegratorOscillator();

    int64_t     mod( const int64_t in );


protected:
    int64_t     amplDelta;
    int64_t     amplDecay;
    int64_t     retAmpl;

    int16_t     phase;
    uint16_t    decay;
    const Primes&   phaseDeltaRef;

    uint16_t    phaseDelta;
    uint8_t     quarter;
    uint8_t     depth;      // current amplitude >> depth
};

// NEW:
// other are obsolate
// -> singleton ?

// real size = 1 << (sizeExpT+2)
// BaseOscillatorArray<6> --> 256 oscillator !!

template< std::size_t sizeExpT >
class BaseOscillatorArray {
public:
    static constexpr uint16_t sizeExp   = (sizeExpT+2);
    static constexpr uint16_t size      = 1<<sizeExp;
    static constexpr uint16_t sizeMask  = size-1;

    static constexpr uint32_t phase1    = 1 << (14+16);
    static constexpr uint32_t phase2    = 1 << (15+16);
    static constexpr float    frange    = 1.0 / (1<<30);
    static constexpr float    franges   = 1.0 / (1<<15);
    static constexpr float    frangesu  = 1.0 / (1<<16);

    static constexpr float    frange1e2 = 1.13 / (1<<16);
    static constexpr float    frange1h2 = 3.0 / (1<<17);
    static constexpr float    frange1q2 = 3.6 / (1<<17);
    static constexpr uint32_t uOffs     = 0x1FFFFFFF;


    inline void inc(void)
    {
        for( auto i=0u; i < size; ++i  ) {
            phase[i] += phaseDelta[i];
        }
    }
    inline void reset(void)
    {
        for( auto i=0u; i < size; ++i  ) {
            phase[i] = phase0[i];
        }
    }
    inline void sync( const uint16_t index )
    {
        phase[index&sizeMask] = phase0[index&sizeMask];
    }
    inline void sync( const uint16_t index1, const uint16_t index2 )
    {
        for( auto i=(index1&sizeMask); i < (index2&sizeMask); ++i  ) {
            phase[i] = phase0[i];
        }
    }
    inline void set0( const uint16_t index, const uint32_t v )
    {
        phase[index&sizeMask] = phase0[index&sizeMask] = v;
    }
    inline void setDelta( const uint16_t index, const uint32_t v )
    {
        phaseDelta[index&sizeMask] = v;
    }
    // 16 bit out
    // TODO : for float use the float sin table !!!
    inline int32_t sin16(const uint16_t index)      const {return tables::waveSinTable[ uint16_t(  phase[index&sizeMask] >>16 ) ];};
    inline float   fsin(const uint16_t index)       const {return float(sin16()) * franges; };
    inline int32_t cos16(const uint16_t index)      const {return tables::waveSinTable[ uint16_t(( phase[index&sizeMask] + phase1 ) >>16 ) ];};
    inline float   fcos(const uint16_t index)       const {return float(cos16()) * franges;  };

    inline int32_t sin16u(const uint16_t index)     const {return 0x7FFF + tables::waveSinTable[ uint16_t(  phase[index&sizeMask] >>16 )];};
    inline float   fsinu(const uint16_t index)      const {return float(sin16u()) * frangesu; };
    inline int32_t cos16u(const uint16_t index)     const {return 0x7FFF+ tables::waveSinTable[ uint16_t(( phase[index&sizeMask] + phase1 ) >>16 )];};
    inline float   fcosu(const uint16_t index)      const {return float(cos16u()) * frangesu;};

    // normalized to 30 bit
    inline int32_t sin(const uint16_t index)        const {return tables::waveSinTable[ uint16_t(  phase[index&sizeMask] >>16 ) ]<<15;};
    inline int32_t cos(const uint16_t index)        const {return tables::waveSinTable[ uint16_t(( phase[index&sizeMask] + phase1 ) >>16 ) ]<<15;};
    inline int32_t sinu(const uint16_t index)       const {return uOffs + (tables::waveSinTable[ uint16_t(  phase[index&sizeMask] >>16 ) ]<<15);};
    inline int32_t cosu(const uint16_t index)       const {return uOffs + (tables::waveSinTable[ uint16_t(( phase[index&sizeMask] + phase1 ) >>16 ) ]<<15);};

    inline int32_t saw(const uint16_t index)        const {return int32_t(phase[index&sizeMask]) >> 1;};
    inline float   fsaw(const uint16_t index)       const {return float(saw()) * frange;};
    inline int32_t sawu(const uint16_t index)       const {return uOffs + (int32_t(phase[index&sizeMask]) >> 2);};
    inline float   fsawu(const uint16_t index)      const {return float(sawu()) * frange;};

    // shifted by PI/2 ==> cos
    inline int32_t sawc(const uint16_t index)       const {return int32_t(phase[index&sizeMask]+phase1) >> 1;};
    inline float   fsawc(const uint16_t index)      const {return float(sawc()) * frange;};
    inline int32_t sawcu(const uint16_t index)      const {return uOffs + (int32_t(phase[index&sizeMask]+phase1) >> 2);};
    inline float   fsawcu(const uint16_t index)     const {return float(sawcu()) * frange;};

    inline int32_t square(const uint16_t index)     const {return (( int32_t(phase[index&sizeMask]) >> 31 ) | 1 ) << 30;};
    inline float   fsquare(const uint16_t index)    const {return float(square()) * frange;};
    inline int32_t squareu(const uint16_t index)    const {return uOffs + ((( int32_t(phase[index&sizeMask]) >> 31 ) | 1 ) << 29);};
    inline float   fsquareu(const uint16_t index)   const {return float(squareu()) * frange;};

    // shifted by PI/2 ==> cos
    inline int32_t squarec(const uint16_t index)    const {return (( int32_t(phase[index&sizeMask]+phase1) >> 31 ) | 1 ) << 30;};
    inline float   fsquarec(const uint16_t index)   const {return float(squarec()) * frange;};
    inline int32_t squarecu(const uint16_t index)   const {return uOffs + ((( int32_t(phase[index&sizeMask]+phase1) >> 31 ) | 1 ) << 29);};
    inline float   fsquarecu(const uint16_t index)  const {return float(squarecu()) * frange;};

    // triangle -> sometimes called sawtooth
    inline int32_t angle3(const uint16_t index)     const {return (( int32_t(phase[index&sizeMask]+phase1) | 1 ) ^ ( int32_t(phase[index&sizeMask]+phase1) >> 31 )) - 0x3FFFFFFF;};
    inline float   ftangle3(const uint16_t index)   const {return float(angle3()) * frange;};
    inline int32_t angle3u(const uint16_t index)    const {return (( int32_t(phase[index&sizeMask]+phase1) | 1 ) ^ ( int32_t(phase[index&sizeMask]+phase1) >> 31 ))>>1;};
    inline float   fangle3u(const uint16_t index)   const {return float(angle3u()) * frange;};

    // shifted by PI/2 ==> cos
    inline int32_t angle3c(const uint16_t index)    const {return -((( int32_t(phase[index&sizeMask]) | 1 ) ^ ( int32_t(phase[index&sizeMask]) >> 31 )) - 0x3FFFFFFF);};
    inline float   fangle3c(const uint16_t index)   const {return float(angle3c()) * frange;};
    inline int32_t angle3cu(const uint16_t index)   const {return ((( int32_t(phase[index&sizeMask]) | 1 ) ^ ( int32_t(phase[index&sizeMask]) >> 31 ))>>1);};
    inline float   fangle3cu(const uint16_t index)  const {return float(angle3cu()) * frange;};

    // phase distorsion sin(sin) series
    //
    inline int32_t pd00ssin(const uint16_t index)   const {return tables::waveSinTable[ uint16_t((tables::waveSinTable[phase[index&sizeMask] >>16])    + (phase[index&sizeMask] >>16))]<<15;};
    inline float   fpd00ssin(const uint16_t index)  const {return float(pd00ssin()) * frange;};

    inline int32_t pd01ssin(const uint16_t index)   const {return tables::waveSinTable[ uint16_t((tables::waveSinTable[phase[index&sizeMask] >>16]>>1) + (phase[index&sizeMask] >>16))]<<15;};
    inline float   fpd01ssin(const uint16_t index)  const {return float(pd01ssin()) * frange;};
    inline int32_t pd02ssin(const uint16_t index)   const {return tables::waveSinTable[ uint16_t((tables::waveSinTable[phase[index&sizeMask] >>16]>>2) + (phase[index&sizeMask] >>16))]<<15;};
    inline float   fpd02ssin(const uint16_t index)  const {return float(pd02ssin()) * frange;};
    inline int32_t pd03ssin(const uint16_t index)   const {return tables::waveSinTable[ uint16_t((tables::waveSinTable[phase[index&sizeMask] >>16]>>3) + (phase[index&sizeMask] >>16))]<<15;};
    inline float   fpd03ssin(const uint16_t index)  const {return float(pd03ssin()) * frange;};

    inline int32_t pd10ssin(const uint16_t index)   const {return tables::waveSinTable[ uint16_t((tables::waveSinTable[phase[index&sizeMask] >>16]<<1) + (phase[index&sizeMask] >>16))]<<15;};
    inline float   fpd10ssin(const uint16_t index)  const {return float(pd10ssin()) * frange;};
    inline int32_t pd20ssin(const uint16_t index)   const {return tables::waveSinTable[ uint16_t((tables::waveSinTable[phase[index&sizeMask] >>16]<<2) + (phase[index&sizeMask] >>16))]<<15;};
    inline float   fpd20ssin(const uint16_t index)  const {return float(pd20ssin()) * frange;};
    inline int32_t pd30ssin(const uint16_t index)   const {return tables::waveSinTable[ uint16_t((tables::waveSinTable[phase[index&sizeMask] >>16]<<2) + (phase[index&sizeMask] >>16))]<<15;};
    inline float   fpd30ssin(const uint16_t index)  const {return float(pd30ssin()) * frange;};

    inline int32_t pd0qssin(const uint16_t index)   const {return tables::waveSinTable[ uint16_t( tables::waveSinTable[phase[index&sizeMask] >>16] )]<<15;};
    inline float   fpdq0ssin(const uint16_t index)  const {return float(pd0qssin()) * frange;};
    // pair sin 1+2
    //
    inline int32_t psin1e2(const uint16_t index)    const {return (tables::waveSinTable[uint16_t(phase[index&sizeMask]>>16)]+(tables::waveSinTable[uint16_t(phase[index&sizeMask]>>15)]));};
    inline float   fpsin1e2(const uint16_t index)   const {return float(psin1e2()) * frange1e2;};
    inline int32_t psin1h2(const uint16_t index)    const {return (tables::waveSinTable[uint16_t(phase[index&sizeMask]>>16)]+(tables::waveSinTable[uint16_t(phase[index&sizeMask]>>15)]>>1));};
    inline float   fpsin1h2(const uint16_t index)   const {return float(psin1h2()) * frange1h2;};
    inline int32_t psin1q2(const uint16_t index)    const {return (tables::waveSinTable[uint16_t(phase[index&sizeMask]>>16)]+(tables::waveSinTable[uint16_t(phase[index&sizeMask]>>15)]>>2));};
    inline float   fpsin1q2(const uint16_t index)   const {return float(psin1q2()) * frange1q2;};

    inline int32_t trigger(const uint16_t index)    const {return (phase[index&sizeMask]+phaseDelta[index&sizeMask]) > phase[index&sizeMask];};

protected:
    union {
        v4si        vp[size/4];
        uint32_t    phase[size];
    };
    union {
        v4si        vpd[size/4];
        uint32_t    phaseDelta[size];
    };
    union {
        v4si        vp0[size/4];
        uint32_t    phase0[size];
    };
};

// obsolate
class BaseOscillatorFunction {
public:
    static constexpr uint32_t phase1    = 1 << (14+16);
    static constexpr uint32_t phase2    = 1 << (15+16);
    static constexpr float    frange    = 1.0 / (1<<30);
    static constexpr float    franges   = 1.0 / (1<<15);
    static constexpr float    frangesu  = 1.0 / (1<<16);

    static constexpr float    frange1e2 = 1.13 / (1<<16);
    static constexpr float    frange1h2 = 3.0 / (1<<17);
    static constexpr float    frange1q2 = 3.6 / (1<<17);
    static constexpr uint32_t uOffs     = 0x1FFFFFFF;

    BaseOscillatorFunction()
    {};
    inline void reset(void) {
        phase = 0;
    };
    inline void phase0( const uint32_t phaseP ) {
        phase  = phaseP;
    };
    inline void inc( const uint32_t phaseDelta ) {
        phase  += phaseDelta;
    };
    inline uint32_t getPhase(void)  const {return phase;};

    // 16 bit out
    inline int32_t sin16(void)      const {return tables::waveSinTable[ uint16_t(  phase >>16 ) ];};
    inline float   fsin(void)       const {return float(sin16()) * franges; };
    inline int32_t cos16(void)      const {return tables::waveSinTable[ uint16_t(( phase + phase1 ) >>16 ) ];};
    inline float   fcos(void)       const {return float(cos16()) * franges;  };

    inline int32_t sin16u(void)     const {return 0x7FFF + tables::waveSinTable[ uint16_t(  phase >>16 )];};
    inline float   fsinu(void)      const {return float(sin16u()) * frangesu; };
    inline int32_t cos16u(void)     const {return 0x7FFF+ tables::waveSinTable[ uint16_t(( phase + phase1 ) >>16 )];};
    inline float   fcosu(void)      const {return float(cos16u()) * frangesu;};

    // normalized to 30 bit
    inline int32_t sin(void)        const {return tables::waveSinTable[ uint16_t(  phase >>16 ) ]<<15;};
    inline int32_t cos(void)        const {return tables::waveSinTable[ uint16_t(( phase + phase1 ) >>16 ) ]<<15;};
    inline int32_t sinu(void)       const {return uOffs + (tables::waveSinTable[ uint16_t(  phase >>16 ) ]<<15);};
    inline int32_t cosu(void)       const {return uOffs + (tables::waveSinTable[ uint16_t(( phase + phase1 ) >>16 ) ]<<15);};

    inline int32_t saw(void)        const {return int32_t(phase) >> 1;};
    inline float   fsaw(void)       const {return float(saw()) * frange;};
    inline int32_t sawu(void)       const {return uOffs + (int32_t(phase) >> 2);};
    inline float   fsawu(void)      const {return float(sawu()) * frange;};

    // shifted by PI/2 ==> cos
    inline int32_t sawc(void)       const {return int32_t(phase+phase1) >> 1;};
    inline float   fsawc(void)      const {return float(sawc()) * frange;};
    inline int32_t sawcu(void)      const {return uOffs + (int32_t(phase+phase1) >> 2);};
    inline float   fsawcu(void)     const {return float(sawcu()) * frange;};

    inline int32_t square(void)     const {return (( int32_t(phase) >> 31 ) | 1 ) << 30;};
    inline float   fsquare(void)    const {return float(square()) * frange;};
    inline int32_t squareu(void)    const {return uOffs + ((( int32_t(phase) >> 31 ) | 1 ) << 29);};
    inline float   fsquareu(void)   const {return float(squareu()) * frange;};

    // shifted by PI/2 ==> cos
    inline int32_t squarec(void)    const {return (( int32_t(phase+phase1) >> 31 ) | 1 ) << 30;};
    inline float   fsquarec(void)   const {return float(squarec()) * frange;};
    inline int32_t squarecu(void)   const {return uOffs + ((( int32_t(phase+phase1) >> 31 ) | 1 ) << 29);};
    inline float   fsquarecu(void)  const {return float(squarecu()) * frange;};

    // triangle -> sometimes called sawtooth
    inline int32_t angle3(void)     const {return (( int32_t(phase+phase1) | 1 ) ^ ( int32_t(phase+phase1) >> 31 )) - 0x3FFFFFFF;};
    inline float   ftangle3(void)   const {return float(angle3()) * frange;};
    inline int32_t angle3u(void)    const {return (( int32_t(phase+phase1) | 1 ) ^ ( int32_t(phase+phase1) >> 31 ))>>1;};
    inline float   fangle3u(void)   const {return float(angle3u()) * frange;};

    // shifted by PI/2 ==> cos
    inline int32_t angle3c(void)    const {return -((( int32_t(phase) | 1 ) ^ ( int32_t(phase) >> 31 )) - 0x3FFFFFFF);};
    inline float   fangle3c(void)   const {return float(angle3c()) * frange;};
    inline int32_t angle3cu(void)   const {return ((( int32_t(phase) | 1 ) ^ ( int32_t(phase) >> 31 ))>>1);};
    inline float   fangle3cu(void)  const {return float(angle3cu()) * frange;};

    // phase distorsion sin(sin) series
    //
    inline int32_t pd00ssin(void)   const {return tables::waveSinTable[ uint16_t((tables::waveSinTable[phase >>16])    + (phase >>16))]<<15;};
    inline float   fpd00ssin(void)  const {return float(pd00ssin()) * frange;};

    inline int32_t pd01ssin(void)   const {return tables::waveSinTable[ uint16_t((tables::waveSinTable[phase >>16]>>1) + (phase >>16))]<<15;};
    inline float   fpd01ssin(void)  const {return float(pd01ssin()) * frange;};
    inline int32_t pd02ssin(void)   const {return tables::waveSinTable[ uint16_t((tables::waveSinTable[phase >>16]>>2) + (phase >>16))]<<15;};
    inline float   fpd02ssin(void)  const {return float(pd02ssin()) * frange;};
    inline int32_t pd03ssin(void)   const {return tables::waveSinTable[ uint16_t((tables::waveSinTable[phase >>16]>>3) + (phase >>16))]<<15;};
    inline float   fpd03ssin(void)  const {return float(pd03ssin()) * frange;};

    inline int32_t pd10ssin(void)   const {return tables::waveSinTable[ uint16_t((tables::waveSinTable[phase >>16]<<1) + (phase >>16))]<<15;};
    inline float   fpd10ssin(void)  const {return float(pd10ssin()) * frange;};
    inline int32_t pd20ssin(void)   const {return tables::waveSinTable[ uint16_t((tables::waveSinTable[phase >>16]<<2) + (phase >>16))]<<15;};
    inline float   fpd20ssin(void)  const {return float(pd20ssin()) * frange;};
    inline int32_t pd30ssin(void)   const {return tables::waveSinTable[ uint16_t((tables::waveSinTable[phase >>16]<<2) + (phase >>16))]<<15;};
    inline float   fpd30ssin(void)  const {return float(pd30ssin()) * frange;};

    inline int32_t pd0qssin(void)   const {return tables::waveSinTable[ uint16_t( tables::waveSinTable[phase >>16] )]<<15;};
    inline float   fpdq0ssin(void)  const {return float(pd0qssin()) * frange;};
    // pair sin 1+2
    //
    inline int32_t psin1e2(void)    const {return (tables::waveSinTable[uint16_t(phase>>16)]+(tables::waveSinTable[uint16_t(phase>>15)]));};
    inline float   fpsin1e2(void)   const {return float(psin1e2()) * frange1e2;};
    inline int32_t psin1h2(void)    const {return (tables::waveSinTable[uint16_t(phase>>16)]+(tables::waveSinTable[uint16_t(phase>>15)]>>1));};
    inline float   fpsin1h2(void)   const {return float(psin1h2()) * frange1h2;};
    inline int32_t psin1q2(void)    const {return (tables::waveSinTable[uint16_t(phase>>16)]+(tables::waveSinTable[uint16_t(phase>>15)]>>2));};
    inline float   fpsin1q2(void)   const {return float(psin1q2()) * frange1q2;};

protected:
    uint32_t    phase;
}; // end class BaseOscillator


// obsolate
class BaseOscillator : public BaseOscillatorFunction {
public:
    BaseOscillator()
    :   phaseDelta(0)
    {};
    BaseOscillator( const uint32_t delta  )
    :   phaseDelta( delta )
    {};
    inline void set( const uint32_t delta ) {
        phaseDelta = delta;
    };
    inline void inc( const uint32_t phaseDeltaP ) {
        phaseDelta = phaseDeltaP;
        phase  += phaseDelta;
    };
    inline void inc(void) {
        phase  += phaseDelta;
    };
    // 0 or 1
    inline int32_t trigger(void)    const {return (phase-phaseDelta) > phase;};
    inline float   ftrigger(void)   const {return float(trigger());};

protected:
    uint32_t    phaseDelta; // freq
}; // end class BaseOscillatorInt


/*

         case SAW_SOFT: {
            const int64_t res = lastSaw;
            lastSaw = (( ( int64_t(dphase) << rangeExp ) + lastSaw*255 ) >> 8) ;
            return res -  ( 195LL<<(rangeExp+18) ); // DC offset - need more testing
            }



 */
} // end namespace yacynth

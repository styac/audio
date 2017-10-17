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
 * File:   FxOutOscillator.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on June 23, 2016, 7:46 PM
 */

#include    "FxOutOscillatorParam.h"
#include    "../utils/Fastsincos.h"
#include    "../effects/FxBase.h"

using namespace tables;

namespace yacynth {

class FxOutOscillator : public Fx<FxOutOscillatorParam>  {
public:
    using MyType = FxOutOscillator;
    FxOutOscillator()
    :   Fx<FxOutOscillatorParam>()
    {
        for( auto& si : slaves ) si.setMasterId(id());
    }

    virtual bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    virtual bool setSprocessNext( uint16_t mode ) override;
    // no input !
    virtual bool connect( const FxBase * v, uint16_t ind ) override;

    // bool connectSlaves( const FxBase * v, uint16_t ind );

private:
    // do slaves also
    static void sprocessClear2Nop( void * );
    static void sprocessFadeOut( void * );
    static void sprocessFadeIn( void * );
    static void sprocessCrossFade( void * );
    
    static void sprocess_01( void * thp );
    static void sprocess_02( void * thp );
    static void sprocess_03( void * thp );
    static void sprocess_04( void * thp );
    static void sprocess_05( void * thp );
    static void sprocess_06( void * thp );
    static void sprocess_07( void * thp );
    static void sprocess_08( void * thp );
    static void sprocess_09( void * thp );
    static void sprocess_10( void * thp );
    static void sprocess_11( void * thp );
    static void sprocess_12( void * thp );
    static void sprocess_13( void * thp );

    
    // for testing > dirac / 2^18
#if 0
    inline void processTestDirac(void)
    {
        for( auto si=0u; si < sectionSize; ++si ) {
            constexpr   uint32_t mask = (1<<17) - 1;
            const float pulse = (++dirac & mask) == mask;
            out().channel[0][si] = pulse;
            out().channel[1][si] = -pulse;
        }
    }
#endif

    // TODO : cleanup
    //  sinTable[(phase[0][0])>>16] -> sinTable[ uint16_t( phase[0][0] >> scalePhaseIndexExp )  ]
    //  freq2deltaPhase(20000.0);
    //
    inline void processTestSine20000(void)
    {
        constexpr uint32_t dphase = freq2deltaPhase(20000.0);
        for( auto si=0u; si < sectionSize; ++si ) {
            phase[0][0] += dphase;
            phase[0][1] += dphase;
            out().channel[0][si] = sinTable[(phase[0][0])>>16];
            out().channel[1][si] = sinTable[(phase[0][1])>>16];
        }
    }

    inline void processTestSine440m16(void)
    {
        constexpr uint32_t dphase = freq2deltaPhase(440.0*16.0);
        for( auto si=0u; si < sectionSize; ++si ) {
            phase[0][0] += dphase;
            phase[0][1] += dphase;
            out().channel[0][si] = sinTable[(phase[0][0])>>16];
            out().channel[1][si] = sinTable[(phase[0][1])>>16];
        }
    }

    inline void processTestSine440m4(void)
    {
        constexpr uint32_t dphase = freq2deltaPhase(440.0*4.0);
        for( auto si=0u; si < sectionSize; ++si ) {
            phase[0][0] += dphase;
            phase[0][1] += dphase;
            out().channel[0][si] = sinTable[(phase[0][0])>>16];
            out().channel[1][si] = sinTable[(phase[0][1])>>16];
        }
    }


    inline void processTestSine440(void)
    {
        constexpr uint32_t dphase = freq2deltaPhase(440.0);
        for( auto si=0u; si < sectionSize; ++si ) {
            phase[0][0] += dphase;
            phase[0][1] += dphase;
            out().channel[0][si] = sinTable[(phase[0][0])>>16];
            out().channel[1][si] = sinTable[(phase[0][1])>>16];
        }
    }

    inline void processTestSine440d4(void)
    {
        constexpr uint32_t dphase = freq2deltaPhase(440.0/4.0);
        for( auto si=0u; si < sectionSize; ++si ) {
            phase[0][0] += dphase;
            phase[0][1] += dphase;
            out().channel[0][si] = sinTable[(phase[0][0])>>16];
            out().channel[1][si] = sinTable[(phase[0][1])>>16];
        }
    }

    inline void processTestSine440d16(void)
    {
        constexpr uint32_t dphase = freq2deltaPhase(440.0/16.0);
        for( auto si=0u; si < sectionSize; ++si ) {
            phase[0][0] += dphase;
            phase[0][1] += dphase;
            out().channel[0][si] = sinTable[(phase[0][0])>>16];
            out().channel[1][si] = sinTable[(phase[0][1])>>16];
        }
    }

    inline void processTestSinImpulse(void)
    {
        constexpr   uint32_t mask = (1<<12) - 1;
        if( (++dirac & mask) == mask ) {
            uint16_t sphaseA = 0;
            uint16_t sphaseB = 0;
            constexpr uint16_t dphaseA = 1<<(16-sectionSizeExp);
            constexpr uint16_t dphaseB = 2 * dphaseA;
            for( auto si=0u; si < sectionSize; ++si ) {
                out().channel[ chA ][si] = sinTable[sphaseA];
                out().channel[ chB ][si] = sinTable[sphaseB];
                sphaseA += dphaseA;
                sphaseB += dphaseB;
            }

        } else {
            for( auto si=0u; si < sectionSize; ++si ) {
                out().channel[0][si] = 0;
                out().channel[1][si] = 0;
            }
        }
    }

    inline void processTestDirac(void)
    {
        constexpr   uint32_t mask = (1<<12) - 1;
        if( (++dirac & mask) == mask ) {
            out().channel[0][0] = -1.0f;
            out().channel[1][0] = -1.0f;
            out().channel[0][1] = +1.0f;
            out().channel[1][1] = +1.0f;
            for( auto si=2u; si < sectionSize; ++si ) {
                out().channel[0][si] = 0;
                out().channel[1][si] = 0;
            }

        } else {
            for( auto si=0u; si < sectionSize; ++si ) {
                out().channel[0][si] = 0;
                out().channel[1][si] = 0;
            }
        }
    }

// TODO : MONO version
// TODO : slaves

    inline void processSine(void)
    {
        for( auto si=0u; si < sectionSize; ++si ) {
            inc0();
            out().channel[0][si] = sinTable[(phase[0][0])>>16];
            out().channel[1][si] = sinTable[(phase[0][1])>>16];
        }
    }

    inline void processSinePd0(void)
    {
        for( auto si=0u; si < sectionSize; ++si ) {
            inc0();
            const uint16_t phase0 = (phase[0][0])>>16;
            const uint16_t phase1 = (phase[0][1])>>16;
            out().channel[0][si] = sinTable[ uint16_t( (waveSinTable[phase0]>>1) + phase0 ) ];
            out().channel[1][si] = sinTable[ uint16_t( (waveSinTable[phase1]>>1) + phase1 ) ];
        }
    }

    inline void processSinePd1(void)
    {
        for( auto si=0u; si < sectionSize; ++si ) {
            inc0();
            const uint16_t phase0 = (phase[0][0])>>16;
            const uint16_t phase1 = (phase[0][1])>>16;
            out().channel[0][si] = sinTable[ uint16_t( (waveSinTable[phase0]>>2) + phase0 ) ];
            out().channel[1][si] = sinTable[ uint16_t( (waveSinTable[phase1]>>2) + phase1 ) ];
        }
    }

    inline void processSinePd2(void)
    {
        for( auto si=0u; si < sectionSize; ++si ) {
            inc0();
            const uint16_t phase0 = (phase[0][0])>>16;
            const uint16_t phase1 = (phase[0][1])>>16;
            out().channel[0][si] = sinTable[ uint16_t( (waveSinTable[phase0]>>3) + phase0 ) ];
            out().channel[1][si] = sinTable[ uint16_t( (waveSinTable[phase1]>>3) + phase1 ) ];
        }
    }

    inline void processSinePd3(void)
    {
        for( auto si=0u; si < sectionSize; ++si ) {
            inc0();
            const uint16_t phase0 = (phase[0][0])>>16;
            const uint16_t phase1 = (phase[0][1])>>16;
            out().channel[0][si] = sinTable[ uint16_t( (waveSinTable[phase0])) ];
            out().channel[1][si] = sinTable[ uint16_t( (waveSinTable[phase1])) ];
        }
    }

    // index 0 - master 1..n slave
    // the channel 1 phase differs
    inline void updateParamPhaseDiff(void)
    {
        if( phasePhaseFreqDiffValue[0].update( param.mode01.indexPhaseFreqDiff[0] ) ) {
            phase[0][1] -= phaseDiff[0];
            phaseDiff[0] = param.mode01.indexPhaseFreqDiff[0].getPhaseValueU32();
            phase[0][1] += phaseDiff[0];
        }

        if( phaseDeltaValue[0].update( param.mode01.indexPhaseDelta ) ) {
            // get the ycent value
            const auto freqYcent = param.mode01.freqMapper.getOffseted(
                param.mode01.freqMapper.getScaled( phaseDeltaValue[0].getValueI32() ) );
            phaseDelta[0][1] = phaseDelta[0][0] = tables::ExpTable::getInstance().ycent2deltafi( freqYcent );
        }
    }

    // the channel 1 freq differs
    inline void updateParamFreqDiff(void)
    {
        if( phasePhaseFreqDiffValue[0].update( param.mode01.indexPhaseFreqDiff[0] ) ) {
            // smooth transition
            phaseDiff[0] = phasePhaseFreqDiffValue[0].getValueI32();
            const auto freqYcent1 = param.mode01.freqMapper.getOffseted(
                param.mode01.freqMapper.getScaled( phaseDeltaValue[0].getValueI32() + phaseDiff[0] ) );
            phaseDelta[0][1] = tables::ExpTable::getInstance().ycent2deltafi( freqYcent1 );
            std::cout << "  updateParamPhaseDiff phaseDiff " << phaseDiff[0] << std::endl;
        }

        if( phaseDeltaValue[0].update( param.mode01.indexPhaseDelta ) ) {
            // get the ycent value
            const auto freqYcent0 = param.mode01.freqMapper.getOffseted( param.mode01.freqMapper.getScaled( phaseDeltaValue[0].getValueI32() ) );
            const auto freqYcent1 = param.mode01.freqMapper.getOffseted( param.mode01.freqMapper.getScaled( phaseDeltaValue[0].getValueI32() + phaseDiff[0] ) );
            phaseDelta[0][0] = tables::ExpTable::getInstance().ycent2deltafi( freqYcent0 );
            phaseDelta[0][1] = tables::ExpTable::getInstance().ycent2deltafi( freqYcent1 );
        }
    }

    // TODO : MONO

    inline void inc0(void)
    {
        phase[0][0] += phaseDelta[0][0];
        phase[0][1] += phaseDelta[0][1];
    }

    // master + 1 slave
    // TODO v4
    inline void inc1(void)
    {
        phase[0][0] += phaseDelta[0][0];
        phase[0][1] += phaseDelta[0][1];
        phase[1][0] += phaseDelta[1][0];
        phase[1][1] += phaseDelta[1][1];
    }

    // master + 3 slave
    // TODO v4
    inline void inc2(void)
    {
        phase[0][0] += phaseDelta[0][0];
        phase[0][1] += phaseDelta[0][1];
        phase[1][0] += phaseDelta[1][0];
        phase[1][1] += phaseDelta[1][1];
        phase[2][0] += phaseDelta[2][0];
        phase[2][1] += phaseDelta[2][1];
        phase[3][0] += phaseDelta[3][0];
        phase[4][1] += phaseDelta[3][1];
    }

    virtual void clearState(void) override;

    uint32_t                        phaseDiff[  FxOutOscillatorParam::slaveCount + 1 ];     // needed to store the current value
    union {
        v4si                        v4PhaseDelta[ (FxOutOscillatorParam::slaveCount + 1)/4 ];
        uint32_t                    phaseDelta[ FxOutOscillatorParam::slaveCount + 1 ][2];
    };
    union {
        v4si                        v4Phase[ (  FxOutOscillatorParam::slaveCount + 1)/4 ];
        uint32_t                    phase[      FxOutOscillatorParam::slaveCount + 1 ][2]; // current phase of oscillators
    };

    // new controller
    ControllerCache phaseDeltaValue[            FxOutOscillatorParam::slaveCount + 1 ];
    ControllerCache phasePhaseFreqDiffValue[    FxOutOscillatorParam::slaveCount + 1 ];
    // slave identity
    FxSlave<FxOutOscillatorParam>   slaves[ FxOutOscillatorParam::slaveCount ];

    uint32_t                        dirac;
    uint32_t                        diracdt;
};


} // end namespace yacynth


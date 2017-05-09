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

#include    "FxBase.h"
#include    "../utils/Fastsincos.h"

using namespace tables;

namespace yacynth {
using namespace TagEffectTypeLevel_02;

class FxOutOscillatorParam {
public:
    // mandatory fields
    static constexpr char const * const name    = "Oscillator4x"; // 1 main + 3 slave
    static constexpr TagEffectType  type        = TagEffectType::FxOutOscillator;
    static constexpr std::size_t maxMode        = 13; // 0 is always exist> 0,1,2
    static constexpr std::size_t inputCount     = 0;
    static constexpr std::size_t slaveCount     = 3; // 0-base signal 1-modulation
    static constexpr char const * const slavename = " ^OscillatorSlave";

    bool parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );
    // master
    ControllerMapLinear<1>  freqMapper;         // controller value to freq (delta phase)
    ControllerIndex         indexPhaseDelta;    // controller index of freq of channel 0
    // master + n slaves
    ControllerIndex         indexPhaseFreqDiff[slaveCount+1]; // controller index of phase or freq diff of channel 1


    // need a direct steady control for multiphase source if there will be
};

class FxOutOscillator : public Fx<FxOutOscillatorParam>  {
public:
    using MyType = FxOutOscillator;
    FxOutOscillator()
    :   Fx<FxOutOscillatorParam>()
    {
        for( auto& si : slaves ) si.setMasterId(id());

        fillSprocessv<0>(sprocess_00);
        fillSprocessv<1>(sprocess_01);
        fillSprocessv<2>(sprocess_02);
        fillSprocessv<3>(sprocess_03);
        fillSprocessv<4>(sprocess_04);
        fillSprocessv<5>(sprocess_05);
        fillSprocessv<6>(sprocess_06);
        fillSprocessv<7>(sprocess_07);
        fillSprocessv<8>(sprocess_08);

        fillSprocessv<9>(sprocess_09);
        fillSprocessv<10>(sprocess_10);
        fillSprocessv<11>(sprocess_11);
        fillSprocessv<12>(sprocess_12);
        fillSprocessv<13>(sprocess_13);
    }

    virtual bool parameter( Yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );


    // go up to Fx ??
    // might change -> set sprocessTransient
    // FIRST TEST WITHOUT TRANSIENT
    // THEN  WITH TRANSIENT -> all types > out,
    // 00 is always clear for output or bypass for in-out == effect OFF
    bool setProcMode( uint16_t ind )  override
    {
        if( procMode == ind ) {
            return true; // no change
        }
        if( getMaxMode() < ind ) {
            return false; // illegal
        }
        if( 0 == procMode ) {
            fadePhase = FadePhase::FPH_fadeInSimple;
        } else if( 0 == ind ) {
            fadePhase = FadePhase::FPH_fadeOutSimple;
        } else {
            fadePhase = FadePhase::FPH_fadeOutCross;
        }

        procMode = ind;
        sprocessp = sprocesspSave = sprocessv[ind];
        // sprocesspSave = sprocessv[ind];
        // sprocessp = sprocessTransient;
        return true;
    }
#if 0
    // go up to Fx ?? virtual ?
    SpfT getProcMode( uint16_t ind ) const override
    {
        switch( ind ) {
        case 0:
            return sprocess_00;
        case 1:
            return sprocess_01;
        case 2:
            return sprocess_02;
        case 3:
            return sprocess_03;
        case 4:
            return sprocess_04;
        case 5:
            return sprocess_05;
        default:
            return sprocessp; // illegal index no change
        }
    }
#endif
    // no input !
    virtual bool connect( const FxBase * v, uint16_t ind ) override;

    // bool connectSlaves( const FxBase * v, uint16_t ind );


private:
    // go up to Fx ???
    // this should interpolate the old and new
    static void sprocessTransient( void * thp );

    static void sprocess_00( void * thp );  // bypass > inp<0> -> out
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
        if( phasePhaseFreqDiffValue[0].update( param.indexPhaseFreqDiff[0] ) ) {
            phase[0][1] -= phaseDiff[0];
            phaseDiff[0] = param.indexPhaseFreqDiff[0].getPhaseValueU32();
            phase[0][1] += phaseDiff[0];
        }

        if( phaseDeltaValue[0].update( param.indexPhaseDelta ) ) {
            // get the ycent value
            const auto freqYcent = param.freqMapper.getOffseted( param.freqMapper.getScaled( phaseDeltaValue[0].getValue() ) );
            phaseDelta[0][1] = phaseDelta[0][0] = tables::ExpTable::getInstance().ycent2deltafi( freqYcent );
        }
    }

    // the channel 1 freq differs
    inline void updateParamFreqDiff(void)
    {
        if( phasePhaseFreqDiffValue[0].update( param.indexPhaseFreqDiff[0] ) ) {
            // smooth transition
            phaseDiff[0] = phasePhaseFreqDiffValue[0].getValue();
            const auto freqYcent1 = param.freqMapper.getOffseted( param.freqMapper.getScaled( phaseDeltaValue[0].getValue() + phaseDiff[0] ) );
            phaseDelta[0][1] = tables::ExpTable::getInstance().ycent2deltafi( freqYcent1 );
            std::cout << "  updateParamPhaseDiff phaseDiff " << phaseDiff[0] << std::endl;
        }

        if( phaseDeltaValue[0].update( param.indexPhaseDelta ) ) {
            // get the ycent value
            const auto freqYcent0 = param.freqMapper.getOffseted( param.freqMapper.getScaled( phaseDeltaValue[0].getValue() ) );
            const auto freqYcent1 = param.freqMapper.getOffseted( param.freqMapper.getScaled( phaseDeltaValue[0].getValue() + phaseDiff[0] ) );
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

    virtual void clearTransient(void) override;

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


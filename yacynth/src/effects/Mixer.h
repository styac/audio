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
 * File:   Mixer.h
 * Author: Istvan Simon
 *
 * Created on March 15, 2016, 6:23 PM
 */

#include    "EffectBase.h"
#include    "Ebuffer.h"
#include    "yacynth_globals.h"

#include    <array>
#include    <iostream>

namespace yacynth {
// --------------------------------------------------------------------
struct InputChannel {    
    InputChannel()    
    { clear(); };
    
    inline void clear( void ) 
    {
        inp = &effectNil.get(); 
        setGain( 0.0f );
    };

    inline void put( float * channelA, float * channelB ) const 
    {
        const float gainAeff =  gainA * inp->channelGain; // this should run non RT
        const float gainBeff =  gainB * inp->channelGain; // this should run non RT
        for( auto i=0u; i < EbufferPar::sectionSize; ++i ) {
            *channelA++ = inp->channelA[i] * gainAeff;
            *channelB++ = inp->channelB[i] * gainBeff;
        }        
    }
    
    inline void add( float * channelA, float * channelB ) const
    {
        if( ! enabled )
            return;
        const float gainAeff =  gainA * inp->channelGain; // this should run non RT
        const float gainBeff =  gainB * inp->channelGain; // this should run non RT
        for( auto i=0u; i < EbufferPar::sectionSize; ++i ) {
            *channelA++ += inp->channelA[i] * gainAeff;
            *channelB++ += inp->channelB[i] * gainBeff;
        }        
    }
    
    void setGain( const float gain ) 
    {
        gainChannel = inp->channelGain;
        if( std::abs( gain ) < 1e-20 ) {
            gainA   = gainB = 0.0f;
            enabled = false;
            return;            
        }
        gainA = gainB = gain * gainChannel;
        enabled = true;        
    }
    
    const EIObuffer   * inp;
    float               gainA;
    float               gainB;
    float               gainChannel;
    bool                enabled;
};
// --------------------------------------------------------------------
// End mixer: output to audio callback buffer
//
template< std::size_t inputCountP >
class MixerUnit {
public:
    static constexpr std::size_t  inputCount  = inputCountP;
    static constexpr std::size_t  sectionSize = EbufferPar::sectionSize;

    MixerUnit()
    { clear(); };
    
    inline void clear( void ) 
    {
        for( auto ci=0u; ci < inputCount; ++ci  ) {
           inputs[ci].clear(); 
        }        
    };
    
    inline void process( float * channelA, float * channelB ) const 
    {        
        inputs[0].put( channelA, channelB ); 
        for( auto ci=1u; ci < inputCount; ++ci  ) {
           inputs[ci].add( channelA, channelB ); 
        }        
    };
    
    inline void process( EIObuffer& out ) const
    {     
        process( out.channelA, out.channelB );
        out.channelGain = 1.0f;
//        out.amplitudeSumm = 0;  // not valid anymore
    };
    
    inline void setGain( const float gain, uint8_t index ) 
    {
        if( index >= inputCount )
            return;
        inputs[index].setGain(gain);
    }
    inline void connect( const EIObuffer& v, const uint16_t index ) 
    {
        if( index >= inputCount )
            return;
        inputs[index].inp = &v;    
    };
    
    MixerUnit(MixerUnit const &)        = delete;
    void operator=(MixerUnit const &t)  = delete;
    MixerUnit(MixerUnit &&)             = delete;
    
protected:
    InputChannel inputs[ inputCount ];
};
// --------------------------------------------------------------------
// Effect mixer
template< std::size_t inputCountP >
class Mixer : public EffectBase {
public:
    static constexpr std::size_t  inputCount  = inputCountP;
    static constexpr std::size_t  sectionSize = EbufferPar::sectionSize;

    Mixer()
    :   EffectBase("Mixer")
    { clear(); };

    void clear( void ) override
    {
        mixerUnit.clear();
    };    
    virtual void process(void) override
    {     
        mixerUnit.process( out );
    };
    virtual void connect( const EIObuffer& v, const uint16_t index=0 ) override
    {  
        mixerUnit.connect( v, index );
    };
    inline void setGain( const float gain, uint8_t index ) 
    {
        mixerUnit.setGain( gain, index );
    }

    Mixer(Mixer const &)            = delete;
    void operator=(Mixer const &t)  = delete;
    Mixer(Mixer &&)                 = delete;

private:    
    MixerUnit<inputCountP>   mixerUnit;
};
// --------------------------------------------------------------------
// Effect mixer
template< std::size_t inputCountP >
class EndMixer : public EffectBase {
public:
    static constexpr std::size_t  inputCount  = inputCountP;
    static constexpr std::size_t  sectionSize = EbufferPar::sectionSize;

    EndMixer()
    :   EffectBase("EndMixer")
    { clear(); };

    void clear( void ) override
    {
        mixerUnit.clear();
    };    
    inline void setGain( const float gain, uint8_t index ) 
    {
        mixerUnit.setGain( gain, index );
    }
    inline void process( float * channelA, float * channelB ) const 
    {        
        mixerUnit.process( channelA, channelB );
    };
    
    virtual void connect( const EIObuffer& v, const uint16_t index=0 ) override
    {  
        mixerUnit.connect( v, index );
    };
    
    EndMixer(EndMixer const &)          = delete;
    void operator=(EndMixer const &t)   = delete;
    EndMixer(EndMixer &&)               = delete;

private:    
    MixerUnit<inputCountP>   mixerUnit;
};
// --------------------------------------------------------------------
} // end namespace yacynth


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
 * File:   FxBase.h
 * Author: Istvan Simon
 *
 * Created on April 6, 2016, 11:22 PM
 */

#include    "Ebuffer.h"
#include    "control/Controllers.h"
#include    "Tags.h"
#include    "protocol.h"

#include    <cstdint>
#include    <sstream>
#include    <ostream>
#include    <vector>

// #define EFFECT_DEBUG 1

// #define DO_FX_STATISTICS 1

namespace yacynth {
using namespace TagEffectTypeLevel_02;

class FxBase;
class FxRunner;
class FxNode;
class FxCollector;

// --------------------------------------------------------------------
// FxCollector --------------------------------------------------------
// --------------------------------------------------------------------
// effect collector class - singleton
// contains all effects
// effects inserted by their own ctor !
// index 0 -- nilFx


class FxCollector {
public:
    NON_COPYABLE_NOR_MOVABLE(FxCollector);

    bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );

    static FxCollector& getInstance(void)
    {
        static FxCollector instance;
        return instance;
    }

    inline void put( FxBase& node )
    {
        nodes.push_back( &node );
    }

    inline std::size_t count(void) const
    {
        return nodes.size();
    }

    // count must be called/checked first !!!
    inline FxBase * get( const std::size_t id ) const
    {
        return nodes.at(id); // may throw !!!
    }

private:
    // create a new instance of an effect
    bool factory( TagEffectType effectType );

    // destroys all dynamic effects and restore static instance counters
    bool cleanup();

    void getFullName( const FxBase &fxmaster, const FxBase &fxcurr, char * name, size_t nameLength );
    FxCollector()
    :   nodes()
    ,   firstDynamicInstance(0)
    {
        nodes.reserve(128);
    };
    std::vector< FxBase * >  nodes;
    uint16_t    firstDynamicInstance;
};

// --------------------------------------------------------------------
// FxBase -------------------------------------------------------------
// --------------------------------------------------------------------

// base of all effects

class FxBase : public EIObuffer {
public:
    NON_COPYABLE_NOR_MOVABLE(FxBase);
    FxBase() = delete;
    virtual ~FxBase();

    using MyType = FxBase;
    using SpfT = void (*)( void * );

    friend class FxNode;
    friend class FxCollector;


    FxBase( const char * name,
            uint16_t maxM = 0,
            uint16_t iC = 0,
            TagEffectType type = TagEffectType::FxNop )
    :   EIObuffer()
    ,   sprocessp(sprocessNop)
    ,   sprocesspCurr(sprocessNop)
    ,   sprocesspNext(sprocessNop)
    ,   nextSetPocModeCycle(-1)
    ,   myName(name)
    ,   myType(type)
    ,   myId(++count)
    ,   inCount(iC)
    ,   maxMode(maxM)
    ,   masterId(0)
    ,   myInstance(0)
    ,   procMode(0)
    ,   dynamic(0)
    {
        FxCollector::getInstance().put(*this);
    };

    inline const FxBase& get(void) const { return *this; };
    inline const std::string& name(void) const { return myName; };
    inline uint16_t id(void) const { return myId; };
    inline uint16_t myInstanceIndex(void) const { return myInstance; };
    inline uint16_t getInputCount(void) const { return inCount; };
    inline uint16_t getMasterId(void) const { return masterId; };
    inline uint16_t getMaxMode(void) const { return maxMode; };
    inline TagEffectType getType(void) const { return myType; };
    inline bool isSlave(void) const { return masterId != 0; };
    inline uint8_t isDynamic() { return dynamic; }
    inline EIObuffer& out(void) { return *static_cast<EIObuffer *>(this); }
    static inline uint16_t getMaxId(void) { return count; };

    bool setProcessingMode( uint16_t ind );                                     // control thread
    virtual bool connect( const FxBase * v, uint16_t ind );                     // control thread
    virtual bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex ); // control thread

    inline  void exec(void) { sprocessp(this); }                                // RT thread

protected:
    virtual void clearState();                                                  // RT thread

    virtual bool setSprocessNext( uint16_t mode );                              // control thread
    // called by FxCollector::factory to manage dynamic instances
    inline void setDynamic() { dynamic = 1; }
    inline void setCount( uint16_t val ) { count = val; }
    inline uint16_t getCount() { return count; }

    SpfT                sprocessp;      // called by exec
    SpfT                sprocesspCurr;  // copy of sprocessp for fadeOut,fadeIn
    SpfT                sprocesspNext;  // next processing mode after crossfade
    int64_t             nextSetPocModeCycle;
    const std::string   myName;
    const TagEffectType myType;
    const uint8_t       myId;
    const uint8_t       inCount;
    const uint8_t       maxMode;
    uint8_t             masterId;       // 0 - master , 0 < slave -- value is the id of master
    uint8_t             myInstance;
    uint8_t             procMode;
    uint8_t             dynamic;        // instance was created dynamically

#if DO_FX_STATISTICS==1
    // add some statistics counters here (conditional)
    uint64_t            cycleCount; // number of cycles from start
    uint64_t            spentTime;  // nanosec of spent time
    uint64_t            maxTime;    // max cycle time
    uint64_t            minTime;    // min cycle time
    uint64_t            entryTime;  // tmp to store current entry time
#endif

    static uint16_t     count;      // static counter to make unique id
    static void sprocessNop( void * );                                          // RT thread
    static void sprocessClear2Nop( void * );                                    // RT thread
    static void sprocessFadeOut( void * );                                      // RT thread
    static void sprocessFadeIn( void * );                                       // RT thread
    static void sprocessCrossFade( void * );                                    // RT thread
};

// --------------------------------------------------------------------
// --------------------------------------------------------------------
// nil instance - for init, do nothing
// TODO check> singleton solution
extern FxBase fxNil;

// --------------------------------------------------------------------
// FxSlave ------------------------------------------------------------
// --------------------------------------------------------------------
// FxSlave only provides additional output if needed
// all functionality is in master
//
template< typename Tparam  >
class FxSlave : public FxBase {
public:
    FxSlave()
    :   FxBase(Tparam::slavename, 0, 0, TagEffectType::FxSlave )
    {}
    virtual ~FxSlave() = default;

    inline void setMasterId( uint8_t val )
    {
        masterId = val;
    }

    inline void setInstanceIndex( uint8_t val )
    {
        myInstance = val;
    }

    virtual bool setSprocessNext( uint16_t mode ) override
    {
        return false;
    }
    // this could be here -- no parameter at all
    virtual bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex ) override
    {
        message.setStatus( yaxp::MessageT::noParameter );
        return false;
    };
    // this could be here -- clear out
    virtual void clearState() override
    {
        out().clear();
    };

protected:
};

// --------------------------------------------------------------------
// FxNode -------------------------------------------------------------
// --------------------------------------------------------------------
// to store the pointer of an effect and exec the current process

class FxNode {
public:
    FxNode(FxNode const &)          = delete;
    void operator=(FxNode const &t) = delete;
    FxNode(FxNode &&)               = delete;

    FxNode();
    inline void exec ( void ) { thp->sprocessp( thp ); };
    inline void set( FxBase& th ) { thp = &th; };

    friend class FxRunner;

private:
    FxBase * thp;
};

// --------------------------------------------------------------------
// FxRunner -----------------------------------------------------------
// --------------------------------------------------------------------

// effect runner
//
// EndMixer         == 0 - called statically at the end
// no output ** out 0 --> null
// OscillatorMixer  == 1 - called statically at the beginning
// no input  --> process called directly by osc output


class FxRunner {
public:
    NON_COPYABLE_NOR_MOVABLE(FxRunner);

    static constexpr std::size_t runFrom    = 1;
    static constexpr std::size_t nodeCount  = 64;   // need to query the config > max 64 effects - looks enough

    enum class RET {
        OK,
        RUNNERFULL,
        LOWID,
        ALREADYADDED,
        NOTEXIST,
        SLAVE,
    };

    bool parameter( yaxp::Message& message, uint8_t tagIndex, uint8_t paramIndex );


    // parameter format:
    // Fill:
    //  uint8_t element count: 0..255 -- 0 means 0
    //  uint8_t Fx id in FxCollectior
    //
    // SetConnections:
    //  uint8_t element count: 0..255 -- 0 means 0
    // count times this 3 elements
    //  uint8_t Fx id in FxCollectior -- output
    //  uint8_t Fx id in FxRunner -- input
    //  uint8_t Fx id in  input id
    //


    FxRunner( FxBase& endMixer )
    :   usedCount(runFrom)
    {
        nodes[0].set( endMixer );
    };

    void clear(void)
    {
        clearConnections();
        usedCount = runFrom;
    }

    FxRunner::RET add( uint8_t ind )
    {
#ifdef YAC_DEBUG
        std::cout
            << " *** add ind " << uint16_t(ind)
            << std::endl;
#endif
        if( usedCount >= nodeCount ) {
            return FxRunner::RET::RUNNERFULL;
        }
        if( 2 >= ind ) {  // never add nil, endMixer,  ???
            return FxRunner::RET::LOWID;
        }
        if( isAdded(ind) ) {
            return FxRunner::RET::ALREADYADDED;
        }
        auto& collector = FxCollector::getInstance();
        if( collector.count() <= ind ) {
            return FxRunner::RET::NOTEXIST;
        }
        if( collector.get(ind)->isSlave() ) {
            return FxRunner::RET::SLAVE;
        }
        nodes[usedCount++].set( *collector.get(ind) );
#ifdef YAC_DEBUG
        std::cout
            << "add usedCount " << usedCount
            << std::endl;
#endif
        return FxRunner::RET::OK;
    }


    // connect 2 nodes

    // change: from -> FxCollector to FxRunner ??? -> SLAVES are not in runners
    FxRunner::RET connect( const uint16_t fromOutFxId, const uint16_t toInRunnerIndex, uint16_t inputIndex = 0 )
    {
#if 0
        std::cout
            << "FxRunner::connect: toInRunnerIndex " << toInRunnerIndex
            << " fromOutFxId " << fromOutFxId
            << " inputIndex " << inputIndex
            << std::endl;
#endif

        if( toInRunnerIndex >= usedCount )
            return FxRunner::RET::NOTEXIST;

        if( 2 > fromOutFxId ) { // 0 = nil, 1 = endMixer
            nodes[ toInRunnerIndex ].thp->connect( &fxNil, inputIndex );
            return FxRunner::RET::OK;
        }

        if( fromOutFxId > fxNil.getMaxId() )
            return FxRunner::RET::NOTEXIST;

        // check validity of fromOutId
        // master - running , slave -> master running
#if 0
        std::cout
           << "FxRunner::connect: to " << nodes[ toInRunnerIndex ].thp->name()
            << " from " <<  FxCollector::getInstance().get(fromOutFxId)->name()
            << std::endl;
#endif

        nodes[ toInRunnerIndex ].thp->connect( FxCollector::getInstance().get(fromOutFxId), inputIndex );
        return FxRunner::RET::OK;
    };

    inline void run(void)
    {
        for( auto i=runFrom; i < usedCount; ++i ) {
            nodes[i].exec();
        }
    };

    inline void clearConnections(void)
    {
        for( auto i=0u; i < usedCount; ++i ) {
            nodes[i].thp->connect( &fxNil,0 );
        }
    };

    // test
    inline void list(void)
    {
        for( auto i=0u; i < usedCount; ++i ) {
            auto& node = nodes[i];
            std::cout
                << "FxRunner effect "
                << "i " << i
                << " id " << node.thp->id()
                << "  " << node.thp->name()
                << " input " << node.thp->getInputCount()
                << std::endl;
        }
    };

    inline bool isAdded( uint16_t id )
    {
        for( auto i=0u; i < usedCount; ++i ) {
            auto& node = nodes[i];
            if( node.thp->id() == id )
                return true;
        }
        return false;
    };

    inline uint16_t count()
    {
        return usedCount;
    }

private:
//    FxNode      nodes[ presetCount ][ nodeCount ]; // TODO more presets for the fast change
    FxNode      nodes[ nodeCount ];
    uint16_t    usedCount;
};


// --------------------------------------------------------------------
// Fx -----------------------------------------------------------------
// --------------------------------------------------------------------
// template to create an effect class
// add param block and a set of inputs

template< typename Tparam  >
class Fx : public FxBase {
public:
    Fx()
    :   FxBase( Tparam::name, Tparam::maxMode, Tparam::inputCount, Tparam::type )
    ,   param()
    {
        for( auto& ip : inpFx )  ip = &fxNil;
        myInstance = instanceCount++;
    }

    virtual ~Fx()
    {
        --instanceCount;
    }

private:
    Fx(Fx const &)              = delete;
    void operator=(Fx const &t) = delete;
    Fx(Fx &&)                   = delete;

protected:
    bool doConnect( const FxBase * v, uint16_t ind )
    {

#ifdef YAC_DEBUG
        if( v != &fxNil ) {
            std::cout
                << "** Fx::connect " << name()
                << " input " << ind
                << " from " << v->name()
                << std::endl;

        }
#endif
        if( ind >= Tparam::inputCount )
            return false;
        inpFx[ind] = v;
        return true;
    };

    // usage: inp<0>, inp<1> ...
    template< std::size_t n=0 >
    inline const EIObuffer& inp(void) const
    {
        static_assert( 0<Tparam::inputCount, "inp: non existent input" );
        static_assert( n<Tparam::inputCount, "inp: non existent input" );
        return *static_cast<const EIObuffer *>(inpFx[n]);
    }

    // n not checked
    inline const EIObuffer& inp( uint8_t n ) const
    {
        return *static_cast<const EIObuffer *>(inpFx[n]);
    }

    static void sprocessCopy0( void * thp )
    {
        static_cast< Fx * >(thp)->processCopy0();
    }

    inline void processCopy0(void)
    {
        static_assert( 0<Tparam::inputCount, "sprocessCopy0: non existent input" );
        out().copy( inp() );
    }

    const FxBase      * inpFx[Tparam::inputCount];
    Tparam              param;
    static uint16_t     instanceCount;      // static counter to make unique id
};

template< typename Tparam  > uint16_t Fx< Tparam  >::instanceCount = 0;

} // end namespace yacynth


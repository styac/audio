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
 * File:   EffectBase.h
 * Author: Istvan Simon
 *
 * Created on April 6, 2016, 11:22 PM
 */

#include    "Ebuffer.h"
#include    "../control/Controllers.h"

#include    <cstdint>
#include    <sstream>
#include    <ostream>
#include    <vector>

//#define EFFECT_DEBUG 1

namespace yacynth {
// --------------------------------------------------------------------
// effect collector class
// contains all effects (also statically used) -- seq is create sequence
class FxBase;
template< uint16_t nodeCount > class FxRunner;


// index 0 -- nilFx
// index 1 -- endMixer

class FxCollector {
public:
    void initialize(void);

    static FxCollector& getInstance(void)
    {
        static FxCollector instance;
        return instance;
    }

    void put( FxBase * node )
    {
        nodes.push_back( node );
    }

    std::size_t count(void) const
    {
        return nodes.size();
    }

    // count must be called/checked first !!!
    FxBase * get( const std::size_t id ) const
    {
        return nodes.at(id); // may throw !!!
    }

    void check(void);


private:
    FxCollector()
    :   nodes()
    {
        nodes.reserve(64);
    };
    std::vector< FxBase * >  nodes;
};
// --------------------------------------------------------------------
// base of all effects

class FxBase : public EIObuffer {
public:
    FxBase()                        = delete;
    FxBase(FxBase const &)          = delete;
    void operator=(FxBase const &t) = delete;
    FxBase(FxBase &&)               = delete;
//    typedef void ( * SpfT )( void * );
    using SpfT = void (*)( void * );
    enum class FadePhase : uint8_t {
        FPH_fadeNo, // no fading
        FPH_fadeOutClear,
        FPH_fadeOutSimple,
        FPH_fadeInSimple,
        FPH_fadeOutCross,
        FPH_fadeInCross,
    };

    FxBase( const char * name, uint16_t maxM = 0, uint16_t iC = 0 )
    :   EIObuffer()
    ,   sprocessp(sprocessNop)
    ,   sprocesspSave(sprocessNop)
    ,   myName(name)
    ,   myId(++count)
    ,   fadePhase(FadePhase::FPH_fadeNo)
    ,   procMode(0)
    ,   inCount(iC)
    ,   maxMode(maxM)
    ,   masterId(0)
    {
        FxCollector::getInstance().put(this);
    };

    virtual ~FxBase() {};

    inline const FxBase& get(void) const { return *this; };
    inline const std::string& name(void) const { return myName; };
    inline uint16_t id(void) const { return myId; };
    inline uint16_t getMaxId(void) const { return count; };
    inline uint16_t getInputCount(void) const { return inCount; };
    inline uint16_t getMasterId(void) const { return masterId; };
    inline uint16_t getMaxMode(void) const { return maxMode; };
    inline bool isSlave(void) const { return masterId != 0; };
    virtual bool connect( const FxBase * v, uint16_t ind );
    virtual bool setProcMode( uint16_t ind ); // might be non virtual
    inline  void exec(void)
    {
        sprocessp(this);
    }
//    virtual bool fill(  std::stringstream& ser );
//    virtual void query( std::stringstream& ser );
//    virtual void clear( void );
// pan> sin,cos
//    inline void setGain( const float gain, const float pan=0 ) { gain=v; };


    inline EIObuffer& out(void)
    {
        return *static_cast<EIObuffer *>(this);
    }

    friend class FxNode;

protected:
//    virtual SpfT getProcMode( uint16_t ind ) const { return sprocessp; };

    SpfT                sprocessp;
    SpfT                sprocesspSave;
//    SpfT                sprocesspNext;
    const std::string   myName;
    const uint8_t       myId;
    const uint8_t       inCount;
    const uint8_t       maxMode;
          uint8_t       masterId;   // 0 - master , 0 < slave -- value is the id of master
    uint8_t             procMode;// might go up the base
    FadePhase           fadePhase; // might go  up the base

    static uint16_t     count;
    static void sprocessNop( void * ) { return; };
};
// --------------------------------------------------------------------
template< typename Tparam  >
class FxSlave : public FxBase {
public:
    FxSlave()
    :   FxBase(Tparam::slavename)
    {}
    void setMasterId( uint8_t mid )
    {
        masterId = mid;
    }
};

// --------------------------------------------------------------------
// nil instance - for init, do nothing
extern FxBase fxNil;

// --------------------------------------------------------------------
// to store the pointer of an effect and exec the current process

class FxNode{
public:
    FxNode(FxNode const &)          = delete;
    void operator=(FxNode const &t) = delete;
    FxNode(FxNode &&)               = delete;

    FxNode();
    inline void exec ( void ) { thp->sprocessp( thp ); };
    inline void set( FxBase& th ) { thp = &th; };

    template< uint16_t nodeCount > friend class FxRunner;

private:
    FxBase * thp;
};
// --------------------------------------------------------------------
// effect runner
//
// EndMixer         == 0 - called statically at the end
// no output ** out 0 --> null
// OscillatorMixer  == 1 - called statically at the beginning
// no input  --> process called directly by osc output

template< uint16_t nodeCount >
class FxRunner {
public:
    static constexpr std::size_t runFrom = 1;

    enum class RET {
        OK,
        RUNNERFULL,
        LOWID,
        ALREADYADDED,
        NOTEXIST,
        SLAVE,
        
    };
    
    FxRunner( FxBase& endMixer )
    :   usedCount(runFrom)
    {
        fxNodes[0].set( endMixer );
    };

    void clear(void)
    {
        clearConnections();
        usedCount = runFrom;
    }

    FxRunner::RET add( uint16_t ind )
    {
        std::cout
            << " *** add ind " << ind
            << std::endl;
        
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
        fxNodes[usedCount++].set( *collector.get(ind) );
        std::cout
            << "add usedCount " << usedCount
            << std::endl;
        return FxRunner::RET::OK;
    }


    // connect 2 nodes

    // change: from -> FxCollector to FxRunner ??? -> SLAVES are not in runners
    FxRunner::RET connect( const uint16_t fromOutFxId, const uint16_t toInRunnerIndex, uint16_t inputIndex = 0 )
    {
        std::cout
            << "FxRunner::connect: toInRunnerIndex " << toInRunnerIndex
            << " fromOutFxId " << fromOutFxId
            << " inputIndex " << inputIndex
            << std::endl;


        if( toInRunnerIndex >= usedCount )
            return FxRunner::RET::NOTEXIST;
                
        if( 2 > fromOutFxId ) { // 0 = nil, 1 = endMixer
            fxNodes[ toInRunnerIndex ].thp->connect( &fxNil, inputIndex );
            return FxRunner::RET::OK;
        }

        if( fromOutFxId > fxNil.getMaxId() )
            return FxRunner::RET::NOTEXIST;

        // check validity of fromOutId
        // master - running , slave -> master running

        std::cout
           << "FxRunner::connect: to " << fxNodes[ toInRunnerIndex ].thp->name()
            << " from " <<  FxCollector::getInstance().get(fromOutFxId)->name()
            << std::endl;


        fxNodes[ toInRunnerIndex ].thp->connect( FxCollector::getInstance().get(fromOutFxId), inputIndex );
        return FxRunner::RET::OK;
    };

    inline void run(void)
    {
        for( auto i=runFrom; i < usedCount; ++i ) {
            fxNodes[i].exec();
        }
    };
    // need a member
    inline void clearConnections(void)
    {
        for( auto i=0u; i < usedCount; ++i ) {
            fxNodes[i].thp->connect( fxNil,0 );
        }
    };

    // test
    inline void list(void)
    {
        for( auto i=0u; i < usedCount; ++i ) {
            auto& node = fxNodes[i];
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
            auto& node = fxNodes[i];
            if( node.thp->id() == id )
                return true;
        }
        return false;
    };    
    
private:
    FxNode      fxNodes[ nodeCount ];
    uint16_t    usedCount;
};


// --------------------------------------------------------------------
// template to create an effect class
// add param block and a set of inputs

template< typename Tparam  >
class Fx : public FxBase {
// class Fx : public FxBase, public Tparam { // ????
public:

    Fx()
    :   FxBase( Tparam::name, Tparam::maxMode, Tparam::inputCount )
    ,   param()
    {
        for( auto& ip : inpFx )     ip = &fxNil;
        for( auto& ip : sprocessv ) ip = sprocessNop;
    }

    inline Tparam& getParam(void) // ????? -- serialize
    {
        return param;
    }


private:
    Fx(Fx const &)              = delete;
    void operator=(Fx const &t) = delete;
    Fx(Fx &&)                   = delete;

protected:
    bool doConnect( const FxBase * v, uint16_t ind )
    {
        std::cout
            << "Fx::connect " << name()
            << " ind " << ind
            << " from " << v->name()
            << std::endl;

        if( ind >= Tparam::inputCount )
            return false;
        inpFx[ind] = v;
        return true;
    };

    template<uint16_t ind>
    inline void fillSprocessv( SpfT p )
    {
        static_assert (ind <= Tparam::maxMode,"fillSprocessv: illegal index" );
        sprocessv[ind]=p;
    }
    // usage: inp<0>, inp<1> ...
    template< std::size_t n=0 >
    inline const EIObuffer& inp(void) const
    {
        static_assert( 0<Tparam::inputCount, "inp: non existent input" );
        static_assert( n<Tparam::inputCount, "inp: non existent input" );
        return *static_cast<const EIObuffer *>(inpFx[n]);
    }

    inline void processCopy0(void)
    {
        static_assert( 0<Tparam::inputCount, "sprocessCopy0: non existent input" );
        out().copy( inp() );
    }
//    bool setProcMode( uint16_t ind )  override;
//    static void sprocess0( void * thp );
//    static void sprocessTransient( void * thp );
    const FxBase      * inpFx[Tparam::inputCount];
    SpfT                sprocessv[Tparam::maxMode+1];
    Tparam              param;
};

// transient:
//  setProcMode ->
//      sprocessp = sprocessTransient
//      procModeNext = proceMode
// sprocessTransient
//      procModeNext == 0 -> fade out
//      procModeCurr == 0 -> fade in
//      else fade out,fade in
//
//
//
//      fade out/in - frame 1
//          run sprocesspSave
//          mult fade out
//          set sprocesspSave -> new mode
//      fade out/in - frame 2
//          run sprocesspSave
//          mult fade in
//          set sprocessp = sprocessp
//          set procModeCurr = procModeNext
//
//
//
//


// --------------------------------------------------------------------


// --------------------------------------------------------------------
} // end namespace yacynth


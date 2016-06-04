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

#include    "../utils/GaloisNoiser.h"
#include    "../control/Controllers.h"

#include    "yacynth_globals.h"
#include    "Ebuffer.h"

#include    <cstdint>
#include    <sstream>
#include    <ostream>
#include    <unordered_map>
#include    <vector>

//#define EFFECT_DEBUG 1
/*

 class X {
 X()
  : f(process)
  {}
 void (*f)( void*);
 static process( this );
 }



 */
//using namespace limiter;
using namespace noiser;

namespace yacynth {
// --------------------------------------------------------------------
class EffectBase;

class Effects {
public:
    static Effects& getInstance(void)
    {
        static Effects instance;
        return instance;
    };
    void put( EffectBase& node );
    EffectBase * get( const std::size_t id ) const
    {
        return nodes[id];
    }

    void check(void);

private:
    Effects() = default;
    std::vector< EffectBase * >  nodes;
};
// --------------------------------------------------------------------
extern Effects&  effects;
// --------------------------------------------------------------------

class EffectBase {
public:
//    static constexpr Sermagic sermagic = "EFBS:01";
    EffectBase( const char * const typeP  )
    :   gain(0)
    ,   id(  ++count )
    ,   efType(typeP)
    ,   name( typeP )
    {
        effects.put(*this);
#ifdef EFFECT_DEBUG
        diag();
#endif
    };

    static void sprocessNop( void * thp );
    virtual ~EffectBase() {};
    virtual bool fill(  std::stringstream& ser );
    virtual void query( std::stringstream& ser );
    virtual void clear( void );
    virtual bool setMode( const uint8_t mode );
    virtual void connect( const EIObuffer& v, const uint16_t index=0 );
    virtual void process(void); // not virtual anymore
//    virtual void control(void);

    inline const EIObuffer& get(void) const { return out; };
    inline void setGain( const float v ) { gain=v; };
    inline void setName( const std::string& np ) { name=np; };
    inline const char * const myType(void) const { return efType; };
    inline const std::string& myName(void) const { return name; };
    inline uint32_t myId(void) const { return id; };

    EffectBase(EffectBase const &)      = delete;
    void operator=(EffectBase const &t) = delete;
    EffectBase(EffectBase &&)           = delete;

#ifdef EFFECT_DEBUG
    void diag(void)
    {
        std::cout << "Effect: " << myType() << " " << myName() << " " <<  myId() << std::endl;
    }
#endif

protected:
    void                    (*pprocess)(void*);
    EIObuffer               out;
    float                   gain;
    std::string             name;
    const uint32_t          id;
    const char * const      efType;
//    static GNoiseShaped     gNoiseShaped;
    static uint32_t         count;
};

// --------------------------------------------------------------------

class EffectIOBase : public EffectBase {
public:
    EffectIOBase( const char * const typeP  )
    :   EffectBase( typeP )
    ,   inp( &nullEBuffer )
    {};

    virtual ~EffectIOBase() {};
    virtual bool fill( std::stringstream& ser )                         override;
    virtual void connect( const EIObuffer& v, const uint16_t index=0 )  override;
    virtual void process(void)                                          override;

    EffectIOBase(EffectIOBase const &)      = delete;
    void operator=(EffectIOBase const &t)   = delete;
    EffectIOBase(EffectIOBase &&)           = delete;

protected:
    const EIObuffer * inp;
//    static ControllerMatrix  * controllerMatrix;
};

extern EffectBase effectNil;
// --------------------------------------------------------------------

class EffectNoise : public EffectBase {
public:
//    static constexpr Sermagic sermagic = "EWNS:01";
    EffectNoise();
    ~EffectNoise();
    virtual bool fill( std::stringstream& ser )         override;
    virtual void process(void)                          override;

};

extern EffectNoise     effectNoise;
// --------------------------------------------------------------------

// http://www.codeproject.com/Articles/11015/The-Impossibly-Fast-C-Delegates
//
//
//
struct  EffectNode {
    EffectNode()
    :   pThis( &effectNil )
    {};
    EffectBase * pThis;
    //const Ebuffer64Float    *inp; // mixers???
    // this->process( inp ) // std::bind ???
    // call( process, this, inp )
    // mixer : call( process, this, inp0,....inpn )
};

// --------------------------------------------------------------------
// http://stackoverflow.com/questions/18619035/understanding-the-overhead-of-lambda-functions-in-c11
// lambda ??
// []( this, inp  ) { this->process(inp); };


template< uint16_t nodeCount >
class EffectNodeVector {
public:
    EffectNodeVector()
    :   usedCount(0)
    {};

//  void add()  // sequence is important !!!
//  bool fill()

    void clear(void)
    {
        clearConnections();
        usedCount = 0;
    }
    // connect 2 nodes
    inline bool connect( const uint16_t from, const uint16_t to )
    {
        if( from >= usedCount || to >= usedCount || from == to )
            return false;
        effectNodes[ to ].pThis->connect( effectNodes[ from ].pThis->get() );
        return true;
    };
    inline void run(void)
    {
        const uint16_t cnt = std::min(nodeCount,usedCount);
        for( auto i=0u; i < cnt; ++i ) {
            auto& node = effectNodes[i];
            node.pThis->process();
        }
    };
    inline void clearConnections(void)
    {
        const uint16_t cnt = std::min(nodeCount,usedCount);
        for( auto i=0u; i < cnt; ++i ) {
            auto& node = effectNodes[i];
            node.pThis->connect( nullEBuffer );
        }
    };

    // test
    inline void list(void)
    {
        const uint16_t cnt = std::min(nodeCount,usedCount);
        for( auto i=0u; i < cnt; ++i ) {
            auto& node = effectNodes[i];
            std::cout
                << "i " << i
                << " id " << node.pThis->myId()
                << std::endl;
        }
    };

private:
    EffectNode      effectNodes[ nodeCount ];
    uint16_t        usedCount;
};


// need a config format
// fabricated in init time

class EffectFactory {
public:
    EffectFactory()
    {};
private:
    std::vector<EffectIOBase> effectVector;
    //std::unordered_map<std::string, EffectBase *>  effectVector;

};
// --------------------------------------------------------------------
} // end namespace yacynth


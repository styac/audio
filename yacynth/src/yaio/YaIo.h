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
 * File:   YaIo.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on January 31, 2016, 9:17 AM
 */
#ifndef     YAIO
#define     YAIO

#include    "YaIoInQueue.h"
#include    "../message/yamsg.h"
#include    "yacynth_globals.h"

#include    <cstddef>
#include    <cstdint>
#include    <vector>
#include    <cstdio>
#include    <iostream>
#include    <cassert>
#include    <iterator>
#include    <string>

#define YAC_JACK_DEBUG  1

namespace yacynth {


class   YaIo {
public:

    YaIo()
    :   nameClient("yacsynth")
    ,   sampleRateMin(48000)
    ,   sampleRateMax(48000)
    ,   bufferSizeMin(64)
    ,   bufferSizeMax(256)
    ,   userData(nullptr)
    ,   midiInProcesing(nullptr)
    ,   audioOutProcesing(nullptr)
    ,   errorString("err: ")
    {};
    NON_COPYABLE_NOR_MOVABLE(YaIo)
    virtual ~YaIo() = default;
    virtual bool initialize(    void ) = 0;
    virtual bool run(           void ) = 0;
    virtual void shutdown(      void ) = 0;
    void setProcessCB(      void* userDataP,
        void (midiInCB)(    void *data, uint8_t *eventp, uint32_t eventSize, bool lastEvent ),
        void (audioOutCB)(  void *data, uint32_t nframes, float *outp1, float *outp2, int16_t bufferSizeMult ) )
    {
        userData            = userDataP;
        midiInProcesing     = midiInCB;
        audioOutProcesing   = audioOutCB;
    };
    void                setMyName( const std::string& name ) { nameClient = name; };
    const std::string   getMyName( void )       { return nameClient; };
    const std::string   getMyNameActual( void ) { return nameClientReal; };
    const std::string   getErrorString( void )  { return errorString; };
    std::int32_t        getErrorCode( void )    { return errorCode; };

protected:
    uint16_t        sampleRateMin;
    uint16_t        sampleRateMax;
    uint16_t        bufferSizeMin;
    uint16_t        bufferSizeMax;
    uint16_t        bufferSizeJack;
    uint16_t        bufferSizeMult;     // how many internal buffer(64samples) fill a jack buffer(256)
    std::string     nameClient;
    std::string     nameClientReal;
    int32_t         errorCode;
    std::string     errorString;
    void          * userData;
    void         (* midiInProcesing)(   void *data, uint8_t *eventp, uint32_t eventSize, bool lastEvent );
    void         (* audioOutProcesing)( void *data, uint32_t nframes, float *outp1, float *outp2, int16_t bufferSizeMult );
};

} // end namespace yacynth
#endif /* YAIO */

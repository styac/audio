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
 * File:   YaIo.cpp
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on January 31, 2016, 9:17 AM
 */

#include    "YaIoJack.h"
#include    "../control/global.h"
#include    <thread>



namespace yacynth {

// --------------------------------------------------------------------
YaIoJack::YaIoJack()
:   YaIo()
,   client(0)
,   jackOptions(JackNoStartServer)
,   midiInPort(     "midi_in_1",    JACK_DEFAULT_MIDI_TYPE,     JackPortIsInput|JackPortIsTerminal)
,   audioOutPort1(  "audio_out_1",  JACK_DEFAULT_AUDIO_TYPE,    JackPortIsOutput|JackPortIsTerminal)
,   audioOutPort2(  "audio_out_2",  JACK_DEFAULT_AUDIO_TYPE,    JackPortIsOutput|JackPortIsTerminal)
,   audioInPort1(   "audio_in_1",   JACK_DEFAULT_AUDIO_TYPE,    JackPortIsInput|JackPortIsTerminal)
,   audioInPort2(   "audio_in_2",   JACK_DEFAULT_AUDIO_TYPE,    JackPortIsInput|JackPortIsTerminal)
{}

YaIoJack::~YaIoJack()
{
    //shutdown();
}
// --------------------------------------------------------------------

void YaIoJack::shutdown( void )
{
    std::chrono::milliseconds const duration(100);
    // audioOutPort1.unreg( client );
    // audioOutPort2.unreg( client );
    // midiInPort.unreg(    client );
    if( client ) {
        jack_client_close(   client );
    }
    std::this_thread::sleep_for(duration);
    client = 0;
}

// --------------------------------------------------------------------

bool YaIoJack::initialize( void )
{
    if( nullptr == userData || nullptr == midiOutProcessing || nullptr == audioOutProcesing ) {
        errorString   += ":nullptr";
        return false;
    }
    client = jack_client_open ( nameClient.c_str(), jackOptions, &jackStatus );
    if( nullptr == client )
        return false;
    const int sampleRate = jack_get_sample_rate(client);
    if( sampleRate < sampleRateMin || sampleRate > sampleRateMax) {
        errorString   += ":illegal sample rate";

#ifdef     YAC_JACK_DEBUG
        std::cout
            << " illegal sample rate : " << sampleRate
            << std::endl;
#endif
        shutdown();
        return false;
    }
    
    bufferSizeJack = jack_get_buffer_size( client );
    if( bufferSizeJack < bufferSizeMin || bufferSizeJack > bufferSizeMax) {
        errorString   += ":illegal buffer size " + std::to_string(bufferSizeJack);

#ifdef     YAC_JACK_DEBUG
        std::cout
            << " illegal buffer size : " << bufferSizeJack
            << std::endl;
#endif
        shutdown();
        return false;
    }
    bufferSizeMult = bufferSizeJack / bufferSizeMin;
    if( jackStatus & JackNameNotUnique ) {
        nameClientReal = jack_get_client_name( client );
    }
    jack_set_process_callback( client, processCB, this );
    if( !audioOutPort1.reg( client ) ) {
        return false;
    }
    if( !audioOutPort2.reg( client ) ) {
        return false;
    }
    if( !midiInPort.reg( client ) ) {
        return false;
    }
    if( !audioInPort1.reg( client ) ) {
        return false;
    }
    if( !audioInPort2.reg( client ) ) {
        return false;
    }
    return true;
} // end YaIoJack::initialize

// --------------------------------------------------------------------

bool YaIoJack::run( void )
{
    if( 0 != jack_activate( client )) {
        errorString   += ":cannot activate client";
        std::cout << "cannot activate client " <<  std::endl;
        return false;
    }
    return true;
}

// --------------------------------------------------------------------
// jack process callback
// gets midi input -> process
// gets audio input -> FxInput
// puts audio data from somewhere
//
int YaIoJack::processCB( jack_nframes_t nframes, void *arg )
{
    YaIoJack& thp  = * static_cast<YaIoJack *> ( arg) ;
    thp.nframes = nframes;
    void * midiIn = thp.midiInPort.getBuffer( nframes );
    jack_default_audio_sample_t *audioOut1 = (jack_default_audio_sample_t *) thp.audioOutPort1.getBuffer( nframes );
    jack_default_audio_sample_t *audioOut2 = (jack_default_audio_sample_t *) thp.audioOutPort2.getBuffer( nframes );
    thp.processJackMidiIn();
    if( thp.mutedOutput ) {
        // obsolete  - use EndMixer setProcMode(0) after processJackMidiIn is there
        for( auto i=0; i < nframes; ++i ) {
            *audioOut1++ = 0.0f;
            *audioOut2++ = 0.0f;
        }
        return 0;
    }
    if( thp.mutedInput ) {
        thp.audioOutProcesing ( thp.userData, nframes, audioOut1, audioOut2 );                
    } else {
        jack_default_audio_sample_t *audioIn1  = (jack_default_audio_sample_t *) thp.audioInPort1.getBuffer( nframes );
        jack_default_audio_sample_t *audioIn2  = (jack_default_audio_sample_t *) thp.audioInPort2.getBuffer( nframes );
        thp.audioInOutProcesing ( thp.userData, nframes, audioOut1, audioOut2, audioIn1, audioIn2 );                        
    }
    return 0;
} // end YaIoJack::processCB

// --------------------------------------------------------------------

// --------------------------------------------------------------------

} // end namespace yacynth




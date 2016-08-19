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
 * File:   Mapping.h
 * Author: Istvan Simon -- stevens37 at gmail dot com
 *
 * Created on February 23, 2016, 12:11 AM
 */

//
//
// mapping the external controls to the internal resources
//
//  internal resources:
//  voice specific
//      oscillator
//      tuning
//      spectrum
//      envelope
//          amplitude
//          frequency
//          AM, FM
//
//  global
//      filter
//          envelope
//      spectrum mixer
//          envelope
//
//  these might be static
//      reverb
//      echo
//
//  --------------------------------------
//  voice specific
//      input:
//          note number
//          channel number
//          velocity
//      catch
//          - oscillator number - mode full poly, mono, nth mono
//          - octave
//          - note in octave - full microtonal - 12 x 16 
//          - spectrum - frequency dependent ( bands? 1/2 octave)
//          - ampltude envelope - frequency dependent ( bands? 1/2 octave)
//      
// 1st idea
//  input : note,channel,modifier ( n dimension )
//      select from n dim matrix a set of selector vector
//          
//  note        7 bit 
//      -> split input range (max 4) + transpose 
//      -> split octave + note
//      -> offset note by modifier
//      -> select range ~ deltaYcent high bits -> max 16 range
//      -> range select envelope
//
//      -> modifier select spectrum
//
//  modifier    8 bit
//          ------------
//              15 bit -> 32k elem
//  
//

namespace yacynth {

} // end namespace yacynth


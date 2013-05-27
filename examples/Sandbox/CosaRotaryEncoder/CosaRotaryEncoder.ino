/**
 * @file CosaRotaryEncoder.ino
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2013, Mikael Patel
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 *
 * @section Description
 * Cosa demonstration of Rotary Encoder.
 *
 * This file is part of the Arduino Che Cosa project.
 */

#include "Cosa/Rotary.hh"
#include "Cosa/Trace.hh"
#include "Cosa/IOStream/Driver/UART.hh"

void setup()
{
  // Use the UART as output stream
  uart.begin(9600);
  trace.begin(&uart, PSTR("CosaRotaryEncoder: started"));

  // Start the interrupt pin handler
  InterruptPin::begin();
}

// Rotary Dial is connected to D6/D1 and D7/D2 (as interrupt pins)
// Min: -100, Max: 10, Initial: -100
#if defined(__ARDUINO_TINY__)
Rotary::Dial dial(Board::PCI1, Board::PCI2, -100, 10, -100);
#else
Rotary::Dial dial(Board::PCI6, Board::PCI7, -100, 10, -100);
#endif

void loop()
{
  // Rotary Encoder/Dial will push event when a change occurs
  Event event;
  Event::queue.await(&event);

  // Dispatch the event so that the dial value is updated
  event.dispatch();

  // Print the new value
  TRACE(dial.get_value());
}


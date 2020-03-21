/**
 * @file UniversalLcd.hh
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2020, Jan Pavlis
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
 * This file is part of the Arduino Che Cosa project.
 */

#ifndef COSA_UNIVERSALLCD_HH
#define COSA_UNIVERSALLCD_HH

#include "Cosa/Types.h"
#include "Cosa/LCD.hh"
#include <Canvas.h>
#include "System5x7.hh"

class UniversalLcd : public LCD::Device {
public:
  UniversalLcd(LCD::IO* io, Font* font = &system5x7);

  /**
   * Get current text font.
   * @return font setting.
   */
  Font* get_text_font() const
  {
      return (m_font);
  }

  /**
   * Set text font. Returns previous setting.
   * @param[in] font.
   * @return previous font setting.
   */
  Font* set_text_font(Font* font) __attribute__((always_inline))
  {
      Font* previous = m_font;
      m_font = font;
      return (previous);
  }

  virtual uint16_t width_pixels() = 0;

  virtual uint16_t height_pixels() = 0;

  virtual uint8_t width_characters()
  {
      return width_pixels() / (m_font->WIDTH + m_font->SPACING);
  }

  virtual uint8_t height_characters()
  {
      return height_pixels() / (m_font->HEIGHT + m_font->LINE_SPACING);
  }

protected:
    LCD::IO* m_io;		//!< Display adapter.
    Font* m_font;		//!< Font.
};

#endif

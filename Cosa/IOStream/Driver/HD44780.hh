/**
 * @file Cosa/IOStream/Driver/HD44780.hh
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
 * This file is part of the Arduino Che Cosa project.
 */

#ifndef __COSA_IOSTREAM_DRIVER_HD44780_HH__
#define __COSA_IOSTREAM_DRIVER_HD44780_HH__

#include "Cosa/Pins.hh"
#include "Cosa/Board.hh"
#include "Cosa/IOStream.hh"

#if defined(__ARDUINO_TINYX5__)
#error "Cosa/IOStream/Driver/HD4480.hh: board not supported"
#endif

/**
 * HD44780 (LCD-II) Dot Matix Liquid Crystal Display Controller/Driver
 * for IOStream access. Binding to trace, etc. Supports simple text 
 * scroll, cursor, and handling of special characters such as form-feed, 
 * back-space and new-line. 
 *
 * @section See Also
 * For furter details see Product Specification, Hitachi, HD4478U,
 * ADE-207-272(Z), '99.9, Rev. 0.0.
 */
class HD44780 : public IOStream::Device {
protected:
  /**
   * Instructions (Table 6, pp. 24), RS(0), RW(0)
   */
  enum {
    CLEAR_DISPLAY = 0x01,    	// Clears entrire display and return home
    RETURN_HOME = 0x02,	     	// Sets DDRAM 0 in address counter
    ENTRY_MODE_SET = 0x04,	// Sets cursor move direction and display shift
    CONTROL_SET = 0x08,	 	// Set display, cursor and blinking controls
    SHIFT_SET = 0x10,		// Set cursor and shifts display 
    FUNCTION_SET = 0x20,	// Sets interface data length, line and font.
    SET_CGRAM_ADDR = 0x40,	// Sets CGRAM address
    SET_CGRAM_MASK = 0x3f,	// Mask CGRAM address (6-bit)
    SET_DDRAM_ADDR = 0x80,	// Sets DDRAM address
    SET_DDRAM_MASK = 0x7f	// Mask DDRAM address (7-bit)
  } __attribute__((packed));

  /**
   * ENTRY_MODE_SET attributes
   */
  enum { 
    DISPLAY_SHIFT = 0x01,	// Shift the entire display not cursor
    INCREMENT = 0x02,		// Increment (right) on write
    DECREMENT = 0x00		// Decrement (left) on write
  } __attribute__((packed));

  /**
   * CONTROL_SET attributes
   */
  enum {
    BLINK_ON = 0x01,		// The character indicated by cursor blinks
    CURSOR_ON = 0x02,		// The cursor is displayed
    DISPLAY_ON = 0x04,		// The display is on
  } __attribute__((packed));

  /**
   * SHIFT_SET attributes
   */
  enum {
    MOVE_LEFT = 0x00,		// Moves cursor and shifts display
    MOVE_RIGHT = 0x04,		// without changing DDRAM contents
    CURSOR_MODE = 0x00,
    DISPLAY_MOVE = 0x08
  } __attribute__((packed));

  /**
   * FUNCTION_SET attributes
   */
  enum {
    DATA_LENGTH_4BITS = 0x00,	// Sets the interface data length, 4-bit or
    DATA_LENGTH_8BITS = 0x10,	// - 8-bit
    NR_LINES_1 = 0x00,		// Sets the number of display lines, 1 or
    NR_LINES_2 = 0x08,		// - 2.
    FONT_5X8DOTS = 0x00,	// Sets the character font, 5X8 dots or
    FONT_5X10DOTS = 0x04	// - 5X10 dots
  } __attribute__((packed));

  // Display pins and state
  OutputPin m_rs;		// Register select (0/instruction, 1/data)
  OutputPin m_en;		// Starts data read/write
  uint8_t m_x;			// Cursor position x
  uint8_t m_y;			// Cursor position y
  uint8_t m_tab;		// Tab step
  uint8_t m_mode;		// Entry mode
  uint8_t m_cntl;		// Control
  uint8_t m_func;		// Function set

  /**
   * Generate enable pulse and transfer data or command (4-bits).
   */
  void pulse();

  /**
   * Write data or command to display
   * @param[in] data to write.
   */
  void write(uint8_t data);

  /**
   * Set/clear display attribute.
   * @param[in,out] cmd command variable.
   * @param[in] mask function.
   */
  void set(uint8_t& cmd, uint8_t mask) { write(cmd |= mask); }
  void clear(uint8_t& cmd, uint8_t mask) { write(cmd &= ~mask); }

public:
  // Max size of bitmap
  static const uint8_t BITMAP_MAX = 8;
  
  // Display width (characters per line) and height (lines)
  const uint8_t WIDTH;
  const uint16_t HEIGHT;

  /**
   * Construct LCD connected to given command and enable pin. Data
   * pins are implicit; D4..D7 for Arduino/Standard, Mighty and
   * ATtinyX4. D10..D13 for Arduino/Mega. Connect to LCD pins D4..D7. 
   * The display is initiated when calling begin().
   * @param[in] rs command/data select pin (Default D8).
   * @param[in] en enable pin (Default D9).
   * @param[in] width of display, characters per lin (Default 16).
   * @param[in] height of display, number of lines (Default 2).
   */
  HD44780(Board::DigitalPin rs = Board::D8, 
	  Board::DigitalPin en = Board::D9,
	  uint8_t width = 16,
	  uint8_t height = 2) :
    IOStream::Device(),
    m_rs(rs, 0),
    m_en(en, 0),
    m_x(0),
    m_y(0),
    m_tab(4),
    m_mode(ENTRY_MODE_SET | INCREMENT),
    m_cntl(CONTROL_SET | BLINK_ON | CURSOR_ON | DISPLAY_ON),
    m_func(FUNCTION_SET | DATA_LENGTH_4BITS | NR_LINES_2 | FONT_5X8DOTS),
    WIDTH(width),
    HEIGHT(height)
  {
  }
  
  /**
   * Clear display and move cursor to home.
   */
  void display_clear();

  /**
   * Turn display on/off. 
   */
  void display_on() { set(m_cntl, DISPLAY_ON); }
  void display_off() { clear(m_cntl, DISPLAY_ON); }

  /**
   * Set display scrolling left/right.
   */
  void display_scroll_left() { write(SHIFT_SET | DISPLAY_MOVE | MOVE_LEFT); }
  void display_scroll_right() { write(SHIFT_SET | DISPLAY_MOVE |  MOVE_RIGHT); }
  
  /**
   * Move cursor to home position.
   */
  void cursor_home();

  /**
   * Turn underline cursor on/off.
   */
  void cursor_underline_on() { set(m_cntl, CURSOR_ON);  }
  void cursor_underline_off() { clear(m_cntl, CURSOR_ON);  }

  /**
   * Turn cursor blink on/off.
   */
  void cursor_blink_on() { set(m_cntl, BLINK_ON); }
  void cursor_blink_off() { clear(m_cntl, BLINK_ON); }

  /**
   * Set text flow left-to-right or right-to-left.
   */
  void text_flow_left_to_right() { set(m_mode, INCREMENT); }
  void text_flow_right_to_left() { clear(m_mode, INCREMENT); }

  /**
   * Set text scroll right adjust or left adjust.
   */
  void text_scroll_left_adjust() { set(m_mode, DISPLAY_SHIFT); }
  void text_scroll_right_adjust() { clear(m_mode, DISPLAY_SHIFT); }

  /**
   * Get tab step.
   * @return tab step (1..WIDTH/2).
   */
  uint8_t get_tab_step()
  {
    return (m_tab);
  }

  /**
   * Set tab step to given value (1..WIDTH/2).
   * @param[in] tab step.
   */
  void set_tab_step(uint8_t step)
  {
    if (step == 0) step = 1;
    else if (step > (WIDTH/2)) step = WIDTH/2;
    m_tab = step;
  }

  /**
   * Get current cursor position.
   * @param[out] x.
   * @param[out] y.
   */
  void get_cursor(uint8_t& x, uint8_t& y)
  {
    x = m_x;
    y = m_y;
  }

  /**
   * Set cursor position to given position.
   * @param[in] x.
   * @param[in] y.
   */
  void set_cursor(uint8_t x, uint8_t y);

  /**
   * Set custom character bitmap to given id (0..7). 
   * @param[in] id character.
   * @param[in] bitmap pointer to bitmap.
   */
  void set_custom_char(uint8_t id, const uint8_t* bitmap);

  /**
   * Set custom character bitmap to given id (0..7). 
   * The bitmap should be stored in program memory.
   * @param[in] id character.
   * @param[in] bitmap pointer to program memory bitmap.
   */
  void set_custom_char_P(uint8_t id, const uint8_t* bitmap);

  /**
   * Start display for text output. Returns true if successful otherwise
   * false.
   * @return boolean.
   */
  bool begin();

  /**
   * Stop display and power down. Returns true if successful otherwise
   * false.
   */
  bool end();

  /**
   * @override
   * Write character to display. Handles carriage-return-line-feed, and
   * form-feed. Returns character or EOF on error.
   * @param[in] c character to write.
   * @return character written or EOF(-1).
   */
  virtual int putchar(char c);
};

#endif
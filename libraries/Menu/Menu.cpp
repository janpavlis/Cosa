/**
 * @file Menu.hh
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2014-2015, Mikael Patel
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

#include "Menu.hh"

void
Menu::Walker::print_menu()
{
	m_display.text_mode(LCD::Device::TextMode::NORMAL_TEXT_MODE);
	m_display.display_clear();

	// Access the current state of the menu walker
	Menu::item_list_P menu = m_stack[m_top];
	Menu::item_vec_P list = (Menu::item_vec_P) pgm_read_word(&menu->list);

	for (uint8_t i = m_scroll; i < m_scroll + m_display.height_characters() && ((Menu::item_P) pgm_read_word(&list[i])) != nullptr; i++)
	{
		Menu::item_P item = (Menu::item_P) pgm_read_word(&list[i]);
		Menu::type_t type = (Menu::type_t) pgm_read_byte(&item->type);

		//m_display.set_cursor(0, i - m_scroll);

		str_P value_pgm = NULL;
		char value[7] = "";

		// Print possible value of current menu item
		switch (type) {
		case Menu::CHECKBOX: {
			bool val = *((bool*)pgm_read_word(&((Menu::checkbox_P)item)->value));
			if (val)
				value_pgm = PSTR("[X]");
			else
				value_pgm = PSTR("[ ]");
			break;
		}
		case Menu::TEXT:
			break;
		case Menu::ONE_OF: {
			// get the one-of variable value string
			uint16_t ix = *((uint16_t*)pgm_read_word(&((Menu::one_of_P)item)->value));
			item_vec_P list = (item_vec_P)pgm_read_word(&((Menu::one_of_P)item)->list);
			value_pgm = (str_P)pgm_read_word(&((item_P)pgm_read_word(&list[ix]))->name);
			break;
		}
		case Menu::INT_RANGE: {
			// Get value of range variable
			int16_t* vp = (int16_t*)pgm_read_word(&((Menu::int_range_P)item)->value);
			int val = *vp;
			itoa(val, value, 10);
			break;
		}
		case Menu::ITEM_LIST:
			value_pgm = PSTR("...");
			break;
		default:
			;
		}

		Menu::print_item(m_display_stream, m_display, (str_P)pgm_read_word(&item->name), value_pgm, value, i == m_ix, m_is_edit_mode);
	}
}

void
Menu::print_item(IOStream& outs, UniversalLcd& lcd, str_P name, str_P value_pgm, const char* value, bool is_selected, bool is_edited)
{
	if (is_selected && !is_edited)
		lcd.text_mode(LCD::Device::TextMode::INVERTED_TEXT_MODE);
	else
		lcd.text_mode(LCD::Device::TextMode::NORMAL_TEXT_MODE);

	if(strlen_P(name) < lcd.width_characters())
		outs.print(name);
	else
	{
		lcd.write_P(name, lcd.width_characters());
		return;
	}

	int8_t spaces = lcd.width_characters() - strlen_P(name) - (value_pgm != NULL ? strlen_P(value_pgm) : 0) - (value != NULL ? strlen(value) : 0);

	if (spaces >= 0)
	{
		for (int i = 0; i < spaces; i++)
			outs.print(PSTR(" "));

		if (is_selected)
			lcd.text_mode(LCD::Device::TextMode::INVERTED_TEXT_MODE);
		else
			lcd.text_mode(LCD::Device::TextMode::NORMAL_TEXT_MODE);

		if (value != NULL)
			outs.print(value);
		if (value_pgm != NULL)
			outs.print(value_pgm);
	}
}

void
Menu::Walker::on_key_down(uint8_t nr)
{
	// Access the current menu item
	Menu::item_list_P menu = m_stack[m_top];
	Menu::item_vec_P list = (Menu::item_vec_P) pgm_read_word(&menu->list);
	Menu::item_P item = (Menu::item_P) pgm_read_word(&list[m_ix]);
	Menu::type_t type = (Menu::type_t) pgm_read_byte(&item->type);

	// React to key event
	switch (nr) {
	case NO_KEY:
		break;
	case SELECT_KEY:
	case RIGHT_KEY:
		switch (type) {
		case Menu::GO_TO_PARENT:
			if (m_top > 0) {
				m_top--;
				m_scroll = 0;
			}
			break;
		case Menu::TEXT:
			break;
		case Menu::ITEM_LIST: {
			// Walk into sub-menu
			m_stack[++m_top] = (Menu::item_list_P) item;
			m_scroll = 0;
			m_ix = 0;
		}
		break;
		case Menu::ACTION: {
			// Execute action and fall back to menu root
			Menu::action_P action = (Menu::action_P) item;
			Menu::Action* obj = (Menu::Action*) pgm_read_word(&action->obj);
			bool res = obj->run(item);
			m_top = 0;
			m_scroll = 0;
			m_ix = 0;
			if (!res)
				return;
			break;
		}
		case Menu::ONE_OF: {
			bool edit_mode_enabled = ((bool)pgm_read_byte(&((Menu::one_of_P)item)->edit_mode_enabled));
			
			if (edit_mode_enabled)
			{
				m_is_edit_mode = !m_is_edit_mode;
			}
			else
			{
				uint16_t* val_p = ((uint16_t*)pgm_read_word(&((Menu::one_of_P)item)->value));
				*val_p = (*val_p + 1);

				Menu::item_vec_P list = (Menu::item_vec_P) pgm_read_word(&((Menu::one_of_P)item)->list);
				if((Menu::item_P) pgm_read_word(&list[*val_p]) == NULL)
					*val_p = 0;;
			}
			break;
		}
		case Menu::CHECKBOX: {
			bool* val_p = ((bool*)pgm_read_word(&((Menu::checkbox_P)item)->value));
			*val_p = !(*val_p);
			break;
		}
		default:
			// Enter item modification mode
			m_is_edit_mode = !m_is_edit_mode;
			break;
		}
		break;
	case LEFT_KEY:
		// Exit item modification mode or walk back
		if (m_is_edit_mode) {
			m_is_edit_mode = false;
		}
		else if (m_top > 0) {
			m_top--;
			m_scroll = 0;
			m_ix = 0;
		}
		break;
	case DOWN_KEY:
		// Step to the next menu item or value in item modification mode
		if (!m_is_edit_mode) {
			if (m_ix > 0) {
				if (m_ix == m_scroll)
					m_scroll--;
				m_ix--;
			}
		}
		else {
			switch (type) {
			case Menu::ONE_OF: {
				// Step to the next enumeration value
				Menu::one_of_P evar = (Menu::one_of_P) item;
				uint16_t* vp = (uint16_t*)pgm_read_word(&evar->value);
				uint16_t value = *vp;
				if (value == 0) break;
				value -= 1;
				list = (Menu::item_vec_P) pgm_read_word(&evar->list);
				item = (Menu::item_P) pgm_read_word(&list[value]);
				*vp = value;
			}
			break;
			case Menu::INT_RANGE: {
				// Decrement the integer variable if within the range
				Menu::int_range_P range = (Menu::int_range_P) item;
				int16_t* vp = (int16_t*)pgm_read_word(&range->value);
				int value = *vp;
				int high = (int)pgm_read_word(&range->high);
				if (value == high) break;
				*vp = value + 1;
			}
			break;
			default:
				;
			}
		}
		break;
	case UP_KEY:
		// Step to the previous menu item or value in item modification mode
		if (!m_is_edit_mode) {
			if ((Menu::item_P) pgm_read_word(&list[m_ix + 1]) != NULL) {
				if (m_ix - m_scroll == m_display.height_characters() - 1)
					m_scroll++;
				m_ix++;
			}
		}
		else {
			switch (type) {
			case Menu::ONE_OF: {
				// Step to the previous enumeration value
				Menu::one_of_P evar = (Menu::one_of_P) item;
				uint16_t* vp = (uint16_t*)pgm_read_word(&evar->value);
				uint16_t value = *vp + 1;
				list = (Menu::item_vec_P) pgm_read_word(&evar->list);
				item = (Menu::item_P) pgm_read_word(&list[value]);
				if (item == NULL) break;
				*vp = value;
			}
			break;
			case Menu::INT_RANGE: {
				// Increment the integer variable in within range
				Menu::int_range_P range = (Menu::int_range_P) item;
				int16_t* vp = (int16_t*)pgm_read_word(&range->value);
				int value = *vp;
				int low = (int)pgm_read_word(&range->low);
				if (value == low) break;
				*vp = value - 1;
			}
			break;
			default:
				;
			}
		}
		break;
	}

	// Display the new walker state
	print_menu();
}

Menu::type_t
Menu::Walker::type()
{
	if (!m_is_edit_mode) return (ITEM_LIST);
	Menu::item_list_P menu = m_stack[m_top];
	Menu::item_vec_P list = (Menu::item_vec_P) pgm_read_word(&menu->list);
	Menu::item_P item = (Menu::item_P) pgm_read_word(&list[m_ix]);
	Menu::type_t type = (Menu::type_t) pgm_read_byte(&item->type);
	return (type);
}

void
Menu::RotaryController::on_event(uint8_t type, uint16_t direction)
{
	UNUSED(type);
	if (m_walker->type() == Menu::INT_RANGE)
		m_walker->on_key_down(direction == CW ?
			Menu::Walker::UP_KEY :
			Menu::Walker::DOWN_KEY);
	else
		m_walker->on_key_down(direction == CW ?
			Menu::Walker::DOWN_KEY :
			Menu::Walker::UP_KEY);
}

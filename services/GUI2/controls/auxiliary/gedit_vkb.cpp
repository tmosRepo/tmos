/*
 * gedit_vkb.cpp
 *
 *  Created on: Mar 22, 2023
 *      Author: bratkov
 */
#include <stdgui.h>
#include <gedit_vkb.h>


#define VKB_TEST		0	//set to 1 for easier testing

#define VKB_TXT_OFFSET	1	//num of empty pixel rows below TXT zone
#define VKB_CTRL_OFFSET	1	//num of empty pixel rows above CTRL zone

enum VKB_OBJECTS_ID : GId
{
	vkb_edit_window = 30,
	vkb_edit_text_id,
	vkb_keyboard_id,
	vkb_controls_id,
	vkb_control_ok_id,
	vkb_control_abc_id,
	vkb_control_x_id,
};

enum vk_button_ctrl_code_t
{
	vk_button_ok = 1000, // do not overlap key_mode
	vk_button_abc,
	vk_button_x
};

/**
 * @param shift_t: key_mode
 * @return: the string from g_keyboard_menu corresponding to shift_t,
 * or "ABC" by default
 */
static const char* get_shift_str(key_mode shift_t)
{
	for(int kbIndex = 0; g_keyboard_menu[kbIndex].item &&
			g_keyboard_menu[kbIndex].item_name != nullptr; ++kbIndex)
		if(g_keyboard_menu[kbIndex].item == shift_t)
		{
			// a menu item shortcut always starts with '&' and ends with '.'
			if(g_keyboard_menu[kbIndex].item_name[0] == '&')
			{
				const char* name = g_keyboard_menu[kbIndex].item_name +1;
				while(*name && *name != '.')
					name++;
				if(*(++name))
					return name;
			}else
				return g_keyboard_menu[kbIndex].item_name;
//			if(strlen(g_keyboard_menu[kbIndex].item_name) >= 6)
//				return (g_keyboard_menu[kbIndex].item_name + 3);
			break;
		}
	return "ERR";
}


//---------- GTextVKB ----------//

/**
 * adds a VKB to the screen Z-order and calls its initialize()
 * @return true
 */
bool GEditVKB::Create(GEdit* edit_)
{
	if(edit_)
	{
		GEditVKB* vkb = new GEditVKB(edit_);
		if(vkb)
		{
			//1. add to Z-order
			vkb->nextObj = Gdesktop->parent->focus->nextObj;
			Gdesktop->parent->focus->nextObj = vkb;
			//2. Set LCD and send WM_INIT to the new virtual keyboard
			vkb->parent = Gdesktop->parent;
			GQueue.push(GMessage(WM_INIT, 0, (long long)gui_driver.lcd, vkb));	//send WM_INIT to the new window
			return true;
		}
	}
	return false;
}

/**
 * set up container, create and add all children to it
 */
unsigned int GEditVKB::initialize (GMessage& msg)
{
	LCD_MODULE* lcd = ((LCD_MODULE **)msg.lparam)[0];
	id = vkb_edit_window;
	client_rect = rect = lcd->rect;  // LCD rectangle, get all display
	//displays = 1; // use main display, already set to 1 from GObject()
	flags = GO_FLG_ENABLED | GO_FLG_SHOW | GO_FLG_SELECTED;

#if VKB_TEST
	base_edit->max_len = 30;
#endif

	//text is always the first child
	//H is font H + 2 pixels on each side, -1 because it's inclusive
	addChild(new GVKB_edit(vkb_edit_text_id, RECT_T(0, 0, rect.x1, font->height + 3),
			base_edit, GO_FLG_DEFAULT|GO_FLG_BORDER));

	//Y: start 1 lower than TXT, end 1 higher than CTRL (offsets used only in draw_this)
	addChild(new GVKB_keyboard(vkb_keyboard_id, RECT_T(0, font->height + 4, rect.x1, rect.y1 - (font->height + 4)),
			GO_FLG_DEFAULT|GO_FLG_SELECTED));

	//use this empty container to draw a border around the CTRL texts, nothing else
	//H is font H + 2 pixels on each side, -1 because it's inclusive
	addChild(new GVKB_Controls(vkb_controls_id, RECT_T(0, rect.y1 - (font->height + 3), rect.x1, rect.y1),
			GO_FLG_DEFAULT|GO_FLG_BORDER));

	send_message(WM_DRAW, 0, 0L, this);
	return GContainer::initialize(msg);
}

unsigned int GEditVKB::process_key(GMessage& msg)
{
	if(msg.param & KEY_UP_CODE)
		return 0;

	if(focus && focus->message(msg))
		return 1;
	switch(msg.param)
	{
	case KEY_DOWN:
		return focus_on_next();
	case KEY_UP:
		return focus_on_previous();
	}
	return 0;
}

unsigned int GEditVKB::process_command(GMessage& msg)
{
	if(msg.param == vk_button_x)
	{
		exit_cancel();
		return 1;
	}
	if(msg.param == vk_button_ok)
	{
		exit_ok();
		return 1;
	}
	return 0;
}

void GEditVKB::exit_ok()
{
	//TODO add OK exit
	GVKB_edit* vk_edit = (GVKB_edit*)get_object(vkb_edit_text_id);
	if(vk_edit && base_edit->txt != vk_edit->txt)
	{
		base_edit->txt = vk_edit->txt;
		base_edit->shift = shift;
		// ATTENTION !  pos_change(pos) where pos is relative position
		base_edit->pos_change(vk_edit->pos - base_edit->pos, true);
		base_edit->text_change();
	}
	// regardless of whether the text has been changed or not, a confirmation message is sent
	if(base_edit->align & ES_USE_VIRTUAL_KEYBOARD)
	{
		send_message(WM_KEY, KEY_ENTER|KEY_ASCII_CODE, 0, base_edit);
	}
	exit_cancel();
}

void GEditVKB::exit_cancel()
{
	//TODO add cancel exit
	close();
	if(parent)
		parent->invalidate(parent, rect);
	delete this;
}


//---------- GVKB_edit ----------//

unsigned int GVKB_edit::process_key(GMessage& msg)
{
	if(msg.param & KEY_UP_CODE)
		return 0;

	if (edit_menu)
	{
		if (GEdit::process_key(msg))
		{
			GObject *obj;
			switch (msg.param)
			{
			case KEY_ENTER:
			case KEY_ESC:
			case KEY_CANCEL:
				obj = get_object(vkb_controls_id);
				if (obj)
				{
					obj->get_focus();
					obj = obj->get_object(vkb_control_abc_id);
					if (obj)
					{
						key_mode previous_shift = ((GEditVKB*) parent)->shift;
						if (shift != previous_shift)
						{
							bool change_cursor_pos = true;
							if((shift == KT_BG_CAPS && previous_shift == KT_BG)
									|| (shift == KT_BG && previous_shift == KT_BG_CAPS))
								change_cursor_pos = false;
							if((shift == KT_EN_CAPS && previous_shift == KT_EN)
									|| (shift == KT_EN && previous_shift == KT_EN_CAPS))
								change_cursor_pos = false;

							((GButton*) obj)->label = get_shift_str(shift);
							send_message(WM_DRAW, 0, 0LL, obj);

							((GEditVKB*) parent)->shift = shift;
							obj = get_object(vkb_keyboard_id);
							if (obj)
							{
								((GVKB_keyboard*) obj)->reinitialize(change_cursor_pos);
								obj->get_focus();
							}
						}
					}
				}
				break;
			}
			return 1;
		}
		return 0;
	}

	switch(msg.param)
	{
	case KEY_LEFT|KEY_REPEAT_CODE:
	case KEY_LEFT:
	case KEY_RIGHT|KEY_REPEAT_CODE:
	case KEY_RIGHT:
	case KEY_CANCEL:
		return GEdit::process_key(msg);

	case KEY_SHIFT:
		//changes the shift status
	{
		RECT_T bkp = rect;
		rect = parent->rect;
		GEdit::process_key(msg);
		rect = bkp;
	}
		return 1;

//	case KEY_UP:
//		return 1;
	case KEY_ESC:
		msg.param = KEY_CANCEL;
		return GEdit::process_key(msg);

	default:
		if(GEdit::process_key(msg))
		{
			send_message(WM_DRAW, 0, 0LL, this);
			return 1;
		}
		break;
	}

	return 0;
}


//---------- GVKB_keyboard ----------//

/**
 * set up variables and call fill_kb()
 */
unsigned int GVKB_keyboard::initialize (GMessage& msg)
{
	client_rect = rect;
	const RENDER_MODE* font = ((GEditVKB*)parent)->font;
	//calculate number of rows and cols on the keyboard
	//cols = (num pixels -1 starting padding) / character hspacing
	cols = rect.width() / font->hspacing;
	//rows = (num pixels - TXT offset - CTRL offset) / character vspacing
	rows = (rect.height() + 2 - VKB_TXT_OFFSET - VKB_CTRL_OFFSET) / font->vspacing;

	if(rows < 1 || cols < 1)
	{ //a VKB cannot be created, close it
		// TODO: ((GEditVKB*)parent)->exit_cancel();
		return 1;
	}

	fill_kb();
	return GObject::initialize(msg);
}

/**
 * usually called from GEditVKB::process_key(), as this doesn't have focus.
 * redraws the KB in case of page change
 */
unsigned int GVKB_keyboard::process_key(GMessage& msg)
{
	if(msg.param & KEY_UP_CODE)
		return 0;

	switch(msg.param)
	{

	case KEY_ENTER:
	{
		GObject* vkb_edit = get_object(vkb_edit_text_id);
		if(vkb_edit)
		{
			send_message(WM_KEY, getc()|KEY_ASCII_CODE, 0, vkb_edit);
			return 1;
		}
	}
	break;

	case KEY_UP:
	case KEY_UP | KEY_REPEAT_CODE:
		if(cursor_pos.y)
		{
			move_cursor(cursor_dir_up);
			return 1;
		}
		break;

	case KEY_DOWN:
	case KEY_DOWN | KEY_REPEAT_CODE:
		if(cursor_pos.y < max_pos.y)
		{
			if(cursor_pos.x <= max_pos.x)
				move_cursor(cursor_dir_down);
			else
				move_cursor(cursor_dir_eol);
			return 1;
		}
		break;

	case KEY_LEFT:
	case KEY_LEFT | KEY_REPEAT_CODE:
		if(cursor_pos.x)
		{
			move_cursor(cursor_dir_left);
		}else if(page)
		{
			//page change
			page--;
			cursor_pos = 0;
			fill_kb(); //draws the new KB page
		}
		return 1;

	case KEY_RIGHT:
	case KEY_RIGHT | KEY_REPEAT_CODE:
		if((cursor_pos.y != max_pos.y && cursor_pos.x + 1 < cols) ||
				(cursor_pos.y == max_pos.y && cursor_pos.x < max_pos.x))
		{
			move_cursor(cursor_dir_right);
		}else if(page < max_page)
		{
			//page change
			page++;
			cursor_pos = 0;
			fill_kb(); //draws the new KB page
		}
		return 1;
	}
	return 0;
}
bool GVKB_keyboard::move_cursor(cursor_direction_t dir)
{
	RECT_T redraw_rect;
	const RENDER_MODE* font = ((GEditVKB*)parent)->font;

	redraw_rect.x0 = cursor_pos.x*font->hspacing;
	redraw_rect.y0 = rect.y0 + cursor_pos.y*font->vspacing;
	switch(dir)
	{
	case cursor_dir_right:
		cursor_pos.x++;
		redraw_rect.x1 = (cursor_pos.x+1)*font->hspacing;
		redraw_rect.y1 = redraw_rect.y0 + font->vspacing +1;
		break;
	case cursor_dir_left:
		redraw_rect.x1 = redraw_rect.x0 + font->hspacing;
		cursor_pos.x--;
		redraw_rect.x0 = (cursor_pos.x-1)*font->hspacing;
		redraw_rect.y1 = redraw_rect.y0 + font->vspacing +1;
		break;
	case cursor_dir_down:
		redraw_rect.x1 = redraw_rect.x0 + font->hspacing;
		cursor_pos.y++;
		redraw_rect.y1 = redraw_rect.y0 + (cursor_pos.y+1)*font->vspacing +1;
		break;
	case cursor_dir_up:
		redraw_rect.x1 = redraw_rect.x0 + font->hspacing;
		redraw_rect.y1 = redraw_rect.y0 + font->vspacing +1;
		cursor_pos.y--;
		redraw_rect.y0 = rect.y0 + cursor_pos.y*font->vspacing;
		break;
	case cursor_dir_eol:
		redraw_rect.x1 = rect.x1;
		cursor_pos.y++;
		cursor_pos.x = max_pos.x;
		redraw_rect.x0 = cursor_pos.x*font->hspacing;
		redraw_rect.y1 = redraw_rect.y0 + (cursor_pos.y+1)*font->vspacing +1;
		break;
	}
	send_message(WM_DRAW, 0, (LPARAM)redraw_rect, this);
	return true;
}

/**
 * draws the symbols on the keyboard (from the current page).
 * if the keyboard has SELECTED flag, also draw the cursor
 */
void GVKB_keyboard::draw_this (LCD_MODULE* lcd)
{
	if(client_rect.height() > 0 && client_rect.width() > 0)
	{
		lcd->set_color(PIX_WHITE);

		lcd->set_font(((GEditVKB*)parent)->font);
		lcd->allign = (SS_LEFT|SS_TOP);
		lcd->pos_x = client_rect.x0;
		//rectangle takes more space but we want pixels between TXT and KB
		lcd->pos_y = client_rect.y0 + VKB_TXT_OFFSET; //so we use offset

		//draw alphabet on screen
		unsigned int alphaIndex = 0;
		int pos_x = 1;	//start from 2nd pixel
		KBPos tempPos;
		do
		{
			if(tempPos.x == cols)	//end of row reached
			{
				tempPos.x = 0;
				pos_x = 1;
				tempPos.y++;
				lcd->pos_y += lcd->font->vspacing;
			}

			lcd->draw_char(pos_x, alphabet[alphaIndex]);
			pos_x += lcd->font->hspacing;
		}
		while(tempPos != max_pos && ++alphaIndex < alphabet.length() && ++(tempPos.x) >= 0);

		//draw cursor if this is the current zone
		if(flags & GO_FLG_SELECTED)
		{
			pos_x = cursor_pos.x*lcd->font->hspacing;
			int y = rect.y0 + cursor_pos.y*lcd->font->vspacing;
			for(int i = 0; i <= lcd->font->vspacing; ++i, y++)
				lcd->invert_hline(pos_x, pos_x + lcd->font->hspacing, y);
		}
	}
}

/**
 * gets the whole alphabet from the current shift, places the part
 * that needs to be displayed in alphabet var, and draws it. calculates the
 * values of max_page and max_pos. variable "page" is controlled in process_key()
 */
void GVKB_keyboard::fill_kb()
{
	CSTRING full_alphabet(" ");	//alphabets start with a space
	//get alphabet
	key_mode shift = ((GEditVKB*)parent)->shift;
	switch(shift)
	{
	case KT_BG:
	case KT_BG_CAPS:
		full_alphabet += g_key_to_char_ref[8].bg_vals;
		full_alphabet += g_key_to_char_ref[9].bg_vals;
		for(unsigned char i = 4; i < 7; ++i)
			full_alphabet += g_key_to_char_ref[i].bg_vals;
		for(unsigned char i = 1; i < 4; ++i)
			full_alphabet += g_key_to_char_ref[i].bg_vals;
		break;
	case KT_EN:
	case KT_EN_CAPS:
		full_alphabet += g_key_to_char_ref[8].en_vals;
		full_alphabet += g_key_to_char_ref[9].en_vals;
		for(unsigned char i = 4; i < 7; ++i)
			full_alphabet += g_key_to_char_ref[i].en_vals;
		for(unsigned char i = 1; i < 4; ++i)
			full_alphabet += g_key_to_char_ref[i].en_vals;
		break;
	case KT_DIGIT:
		full_alphabet += "0123456789.";
		full_alphabet += g_key_to_char_ref[0].bg_vals;
		full_alphabet += g_key_to_char_ref[7].bg_vals;
		full_alphabet += g_key_to_char_ref[12].bg_vals;
		break;
	}

#if VKB_TEST
	full_alphabet += full_alphabet;
	full_alphabet += full_alphabet;
#endif

	if(full_alphabet.length() > 1)
	{
		if(shift == KT_BG_CAPS || shift == KT_EN_CAPS)	//upper letters if needed
			for(unsigned int i = 1; i < full_alphabet.length(); ++i)
				full_alphabet[i] = toupper(full_alphabet[i]);

		//get the desired part of the alphabet and display it
		unsigned int startPos = page * rows*cols;
		alphabet = full_alphabet.substr(startPos, min((unsigned) (rows*cols), full_alphabet.length() - startPos));
		//calculate max positions, div by 0 should be impossible here
		max_page = (full_alphabet.length() - 1) / (rows*cols);
		max_pos.x = (alphabet.length() - 1) % cols;
		max_pos.y = (alphabet.length() - 1) / cols;
	}else
	{
		//in case of error
		alphabet = "ERR";
		cursor_pos = 0;
		max_pos = KBPos(2, 0);
		page = max_page = 0;
	}
	send_message(WM_DRAW, 0, client_rect.as_int, this);
}

void GVKB_keyboard::reinitialize(bool reset_cursor)
{
	if(reset_cursor)
		cursor_pos = 0;
	fill_kb();
}

/**
 * @return the selected char (the one under the "cursor"), using cursor_pos
 */
inline char GVKB_keyboard::getc()
{
	return alphabet[cursor_pos.y*cols + cursor_pos.x];
}


//---------- GText_CTRL ----------//

void GVKB_button::draw_this(LCD_MODULE* lcd)
{
	if(client_rect.height() > 0 && client_rect.width() > 0)
	{
		lcd->set_color(PIX_WHITE);

		lcd->set_font(((GVKB_Controls*)parent)->font);
		set_xy_all(lcd, ((client_rect.y1 - client_rect.y0) >> 1) - (lcd->font->height >> 1), TA_CENTER);
		draw_text_line(lcd, label.c_str(), label.length());
		if((flags & GO_FLG_SELECTED) && (parent->flags & GO_FLG_SELECTED))
			for (int i = client_rect.y0; i <= client_rect.y1; i++)
				invert_hline (client_rect.x0, client_rect.x1, i);
	}
}

unsigned int GVKB_Controls::initialize (GMessage& msg)
{
	font = ((GEditVKB*)parent)->font;
	short int abcLen = font->hspacing * 3 + font->hdistance;

	//X: start from pixel 4 (text starts on 5), end on that + 2*(symbol spacing)
	//Y: start 1 lower than CTRL rectangle start
	addChild(new GVKB_button(vkb_control_ok_id, RECT_T(4, rect.y1 - (font->height + 2),
			4 + 2*font->hspacing, rect.y1 - 1), vk_button_ok, "OK", GO_FLG_DEFAULT));

	//X: center of the screen, from (len - abcLen)/2 to (len + abcLen)/2
	//Y: start 1 lower than CTRL rectangle start
	addChild(new GVKB_button(vkb_control_abc_id, RECT_T((rect.x1-abcLen)>>1, rect.y1 - (font->height + 2),
			((rect.x1+abcLen)>>1) - font->hdistance, rect.y1 - 1), vk_button_abc,
			get_shift_str(((GEditVKB*)parent)->shift), GO_FLG_DEFAULT|GO_FLG_SELECTED));

	//X: end 3 pixels from the end, start (symbol spacing) before that
	//Y: start 1 lower than CTRL rectangle start
	addChild(new GVKB_button(vkb_control_x_id, RECT_T(rect.x1 - 3 - font->hspacing,
			rect.y1 - (font->height + 2), rect.x1 - 3, rect.y1 - 1), vk_button_x,
			"X", GO_FLG_DEFAULT));

	GContainer::initialize(msg);
	return 1;
}

unsigned int GVKB_Controls::process_command(GMessage& msg)
{
	if(msg.param == vk_button_abc)
	{
		GVKB_edit* edit = (GVKB_edit*)get_object(vkb_edit_text_id);
		if(edit)
		{
			send_message(WM_KEY, KEY_SHIFT, 0LL, edit);
			return edit->get_focus();
		}
	}
	return GContainer::process_command(msg);
}

unsigned int GVKB_Controls::process_key(GMessage& msg)
{
	if(msg.param & KEY_UP_CODE)
		return 0;

	if(focus && focus->message(msg))
		return 1;

	switch(msg.param)
	{
	case KEY_LEFT:
		return focus_on_previous();

	case KEY_RIGHT:
		return focus_on_next();
	}
	return 0;
}

unsigned int GVKB_Controls::process_default(GMessage& msg)
{
	GObject* obj = children;
	switch(msg.code)
	{
	case WM_KILLFOCUS:
	case WM_SETFOCUS:
		while(obj)
		{
			if(obj->flags & GO_FLG_SELECTED)
				send_message(WM_DRAW, 0, 0LL, obj);
			obj = obj->nextObj;
		}
		return 1;

	default:
		break;
	}
	return 0;
}


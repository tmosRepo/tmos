/*
 * gtimeedit.cpp
 *
 *  Created on: May 3, 2023
 *      Author: bratkov
 */

#include <stdgui.h>

void time_pos_map::init_map(const char* format)
{
	uint32_t pos=0;
	max_index = index =0;
	while(format && format[0])
	{
		if(format[0] == '%')
		{
			format++;
			switch(format[0])
			{
			case 'M':
			case 'D':
			case 'y':
			case 'h':
			case 'm':
			case 's':
				map[max_index].pos = pos++;
				map[max_index++].type = format[0];
				break;
			case 'Y':
				map[max_index].pos = pos;
				map[max_index++].type = format[0];
				pos +=3;
				break;
			}
		}
		pos++;
		format++;
	}
}

char time_pos_map::get_type()
{
	if(is_index_valid())
	{
		return map[index].type;
	}
	return 0;
}
char time_pos_map::get_pos()
{
	if(is_index_valid())
	{
		return map[index].pos;
	}
	return 0;
}
bool time_pos_map::inc_index()
{
	if(is_index_valid())
	{
		if(index < max_index -1)
		{
			index++;
			return true;
		}
	}
	return false;
}
bool time_pos_map::dec_index()
{
	if(is_index_valid())
	{
		if(index )
		{
			index--;
			return true;
		}
	}
	return false;
}

unsigned int GDateTime::initialize (GMessage& msg)
{
	if((flags & GO_FLG_SELECTED) && is_available() && parent)
		get_focus();
	txt = time->sprintf(time_format);
	alloc_scrollbars();
	text_size = SetTextAlign(align);
	pos = 0;
	cursor = scroll_rect;
	cursor.y0 --;
	set_cursor_on_char();
	if(map)
		delete map;
	map = new time_pos_map;
	map->init_map(time_format);
	if(msg.param)
		send_message(WM_DRAW, 0, 0L, this);
	return 1;
}

void GDateTime::show_cursor()
{
	RECT_T datetime_rect = cursor;
	datetime_rect.x1 += text_font->hspacing;
	if(map->get_type() == 'Y')
		datetime_rect.x1 += 2*text_font->hspacing;
	invalidate (this, datetime_rect);
}

void weak_gui_message_beep(int code);

unsigned int GDateTime::update_time(char ch)
{
	char old_ch, old_ch2;
	if (ch >= '0' && ch <= '9')
	{
		old_ch = txt[pos];
		if (IS_DIGIT(old_ch))
		{
			time_t t = *time;

			txt[pos] = ch;
			t.sscanf(txt.c_str(), time_format);
			if(t.is_valid())
			{
				*time = t;
				increment_txt_pos();
			} else
			{
				//invalid date/time, try to fix by replacing next char
				old_ch2 = txt[pos+1];

				if (IS_DIGIT(old_ch2))
				{
					txt[pos+1] = (ch == '0')?'1':'0';
					t.sscanf(txt.c_str(), time_format);
					if(t.is_valid())
					{
						//fix success, set new time and change positions
						*time = t;
						increment_txt_pos();
						return 1;
					}
					else if(pos + 4 < txt.length() && IS_DIGIT(txt[pos+3]) && IS_DIGIT(txt[pos+4]))
					{
						//try to change the next field to 01 instead
						char old_ch3 = txt[pos+3];
						char old_ch4 = txt[pos+4];

						txt[pos+3] = '0';
						txt[pos+4] = '1';
						t.sscanf(txt.c_str(), time_format);
						if(t.is_valid())
						{
							*time = t;
							increment_txt_pos();
							return 1;
						}
						else
						{
							txt[pos+3] = old_ch3;
							txt[pos+4] = old_ch4;
						}
					}
					//could not fix date, revert to old value and don't change
					txt[pos+1] = old_ch2;
				}
				//revert char, set position to 0 and beep to signal invalid values
				txt[pos] = old_ch;
				pos_change(-pos, false);
				weak_gui_message_beep(2);
			}
		}
		return 1;
	}
	return 0;
}

struct min_max_val_t {
	uint16_t min;
	uint16_t max;
}__attribute__((packed));

static const min_max_val_t val_1_12 = {1, 12};
static const min_max_val_t val_0_23 = {0, 23};
static const min_max_val_t val_0_59 = {0, 59};
static const min_max_val_t val_0_99 = {0, 99};

static void change_value(uint8_t& val, const min_max_val_t& limit, bool inc)
{
	if(inc)
	{
		if(++val > limit.max)
			val = limit.min;
	}
	else
	{
		if(limit.min == val--)
			val = limit.max;
	}
}

static uint8_t get_max_days(const time_t& time)
{
	uint8_t res;
	res = time_t::month_lengths[time.mon];
	if(time.mon == 2 && time_t::is_leap_year(time.year))
		res++;
	return res;
}

unsigned int GDateTime::inc_dec_time(bool inc)
{
	uint8_t val;
	min_max_val_t limit;
	time_t t = *time;

	switch(map->get_type())
	{
	case 'm':
		change_value(t.minute, val_0_59, inc);
		val = t.minute;
		break;
	case 's':
		change_value(t.second, val_0_59, inc);
		val = t.second;
		break;
	case 'h':
		change_value(t.hour, val_0_23, inc);
		val = t.hour;
		break;

	case 'D':
		limit.min =1;
		limit.max = get_max_days(t);
		change_value(t.mday, limit, inc);
		val = t.mday;
		break;
	case 'M':
		change_value(t.mon, val_1_12, inc);
		val = get_max_days(t);
		if(t.mday > val)
			t.mday = val;
		val = t.mon;
		break;
	case 'Y':
	case 'y':
		val = t.year -2000;
		change_value(val, val_0_99, inc);
		t.year = 2000 + val;
		val = get_max_days(t);
		if(t.mday > val)
			t.mday = val;
		val = t.year -2000;
		break;
	default:
		break;
	}
	if(t.is_valid() && t != *time)
	{
		if(map->get_type() != 'D' && t.mday != time->mday)
		{
			txt.clear();
			txt = t.sprintf(time_format);
			send_message(WM_DRAW, 0, 0LL, this);
		}else
		{
			uint32_t offset =0;
			if(map->get_type() == 'Y')
				offset = 2;
			txt[pos + offset++] = val/10 + '0';
			txt[pos + offset] = val%10 + '0';
			show_cursor();
		}
		*time = t;
	}
	return 1;
}

unsigned int GDateTime::move_pos(datetime_move_e move)
{
	if(!(align & ES_USE_VIRTUAL_KEYBOARD))
	{
		switch(move)
		{
		case datetime_move_repeat_left:
		case datetime_move_left:
			if(!pos && (move < datetime_move_last))
				return 0;
			while(pos)
			{
				pos_change(-1, false);
				if(IS_DIGIT(txt[pos]))
					break;
			}
			return 1;

		case datetime_move_repeat_right:
		case datetime_move_right:
			if(!txt[pos] && (move < datetime_move_last))
				return 0;
			while (pos < txt.length())
			{
				pos_change(+1, false);
				if(IS_DIGIT(txt[pos]))
					break;
			}
			return 1;

		default:
			break;
		}
	}else
	{ // use the arrows to change the value
		if(!map || !map->is_index_valid())
			return 0;
		switch(move)
		{
		case datetime_move_repeat_left:
		case datetime_move_left:
			if(map->dec_index())
				pos_change(map->get_pos() - pos, false);
			else if(move == datetime_move_left)
					return 0;
			return 1;

		case datetime_move_repeat_right:
		case datetime_move_right:
			if(map->inc_index())
				pos_change(map->get_pos() - pos, false);
			else if(move == datetime_move_right)
					return 0;
			return 1;

		case datetime_move_up:
			return inc_dec_time(true);

		case datetime_move_down:
			return inc_dec_time(false);

		default:
			break;
		}
	}
	return 0;
}

unsigned int GDateTime::process_key (GMessage& msg)
{
	if(msg.param & KEY_UP_CODE)
		return 0;

	switch (msg.param)
	{
	case KEY_LEFT|KEY_REPEAT_CODE:
		return move_pos(datetime_move_repeat_left);
	case KEY_LEFT:
		return move_pos(datetime_move_left);

	case KEY_RIGHT|KEY_REPEAT_CODE:
		return move_pos(datetime_move_repeat_right);
	case KEY_RIGHT:
		return move_pos(datetime_move_right);

	case KEY_UP:
	case KEY_UP|KEY_REPEAT_CODE:
		return move_pos(datetime_move_up);

	case KEY_DOWN:
	case KEY_DOWN|KEY_REPEAT_CODE:
		return move_pos(datetime_move_down);

	case KEY_ENTER:
		if(align & ES_WANTRETURN)
			msg.param = KEY_DOWN;
		else
			send_message (WM_COMMAND, id, 0L, parent);
		return 0;

	default:
		return update_time(TranslateKey(msg.param));
	}
	return 0;
}

void GDateTime::increment_txt_pos()
{
	while (pos < txt.length())
	{
		pos_change(+1, true);
		if(IS_DIGIT(txt[pos]))
			break;
	}
}

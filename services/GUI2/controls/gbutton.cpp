/*
 * gbutton.cpp
 *
 *  Created on: 30.08.2013
 *      Author: edguest
 */
#include <stdgui.h>

//void GButton::allocate_border(void)
//{
//	client_rect = rect;
//	client_rect.Inflate(2,2,3,3);
//}

void GButton::draw_border(RECT_T& frame)
{
	draw_hline(frame.x0, frame.x1-1, frame.y0);
	draw_vline(frame.y0 + 1, frame.y1 - 1, frame.x0);
	if(flags & GO_FLG_SELECTED)
	{
		draw_hline(frame.x0, frame.x1, frame.y1);
		draw_vline(frame.y0, frame.y1, frame.x1);
	}
	else
	{
		draw_hline(frame.x0, frame.x1-1, frame.y1-1);
		draw_vline(frame.y0 + 1, frame.y1 - 2, frame.x1-1);

		draw_hline(frame.x0+1, frame.x1, frame.y1);
		draw_vline(frame.y0+1, frame.y1, frame.x1);
	}
}

void GButton::draw_this (LCD_MODULE* lcd)
{
	if(flags & GO_FLG_BORDER)
	{
		if(flags & GO_FLG_SELECTED)
			lcd->set_color(PIX_WHITE);
		else
			lcd->set_color(PIX_LIGHTGRAY);
		draw_border(rect);
	}
	lcd->set_font(&FNT5x7);
	set_xy_all(lcd, ((client_rect.y1 - client_rect.y0) >> 1) - (lcd->font->height >> 1), TA_CENTER);
	if(type)
	{
		lcd->allign = TA_LEFT;
		lcd->pos_x = client_rect.x0 + lcd->font->hdistance;
		if (flags & GO_FLG_CHECKED)
			lcd->set_color(PIX_GREEN);
		else
			lcd->set_color(PIX_WHITE);
		if (type == GB_TYPE_RADIO)												//radio button draw
		{
			if (flags & GO_FLG_CHECKED)
				lcd->draw_icon(GICON_CHECKED_CIRCLE);
			else
				lcd->draw_icon(GICON_CIRCLE);
		}
		else																	//check box draw
		{
			if (flags & GO_FLG_CHECKED)
				lcd->draw_icon(GICON_CHECKED_SQUARE);
			else
				lcd->draw_icon(GICON_SQUARE);
		}
		lcd->pos_x = /*(client_rect.y1 - client_rect.y0) +*/
				client_rect.x0 + ((lcd->font->hspacing * 4)/3);
	}
	lcd->set_color(get_fg_color());
	if(flags & GO_FLG_SELECTED)
	{
		lcd->set_background(get_fg_color());
		lcd->clear_rect(RECT_T(lcd->pos_x/*client_rect.x0*/, client_rect.y0+1, client_rect.x1, client_rect.y1-1));
		lcd->set_color(get_bg_color());
	}
	text_metrics_t size = get_text_metrics(label.c_str(), client_rect.width() -lcd->pos_x, &FNT5x7);
	if(size.height > client_rect.height())
		draw_text_line(lcd, label.c_str(), label.length());
	else
		draw_text(lcd, label.c_str());
}


unsigned int GButton::process_key (GMessage& msg)
{
	switch (msg.param)
	{
	case KEY_ENTER:
		if (type)
			send_message (WM_CHANGE, (unsigned int) this, 0L, parent);	//sends a change message if the button is radio or check box
		else
			send_message (WM_COMMAND, code, 0L, parent);				//sends a command based on the button code
		return 1;
	}
	return 0;
}

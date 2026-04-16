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
	// the frame in this case is equal to the rectangle of a button
	// '#' - rect, '-' - client_rect
	// x0,y0                                                x1,y0
	//   |################################################### |
	//   |# ----------------------------------------------- ##|
	//   |# -  Not selected                               - ##|
	//   |# -                                             - ##|
	//   |##-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-###|
	//   | ###################################################|
	// x0,y1                                                x1,y1

	// x0,y0                                                x1,y0
	//   |####################################################|
	//   |# ------------------------------------------------ #|
	//   |# - Selected                                     - #|
	//   |# -                                              - #|
	//   |# ------------------------------------------------ #|
	//   |####################################################|
	// x0,y1                                                x1,y1

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
	RECT_T r_frame, r_state;

	if(flags & GO_FLG_BORDER)
	{
		if (flags & GO_FLG_SELECTED) {
			lcd->set_color(PIX_WHITE);
		} else {
			lcd->set_color(PIX_LIGHTGRAY);
		}
		draw_border(rect);
	}
//	lcd->set_font(&FNT5x7); the font is set in the LCD constructor
	set_xy_all(lcd, client_rect.height()/2 - lcd->font->height/2, TA_CENTER);
	if(type)
	{
		lcd->allign = TA_LEFT;
		lcd->pos_x = client_rect.x0 + lcd->font->hdistance;
		if (flags & GO_FLG_CHECKED)
			lcd->set_color(PIX_GREEN);
		else
			lcd->set_color(PIX_WHITE);

		if (type == GB_TYPE_RADIO) {											//radio button draw
			if (flags & GO_FLG_CHECKED)
				lcd->draw_icon(GICON_CHECKED_CIRCLE);
			else
				lcd->draw_icon(GICON_CIRCLE);
		} else if (type == GB_TYPE_CHECK)										//check box draw
		{
			if (flags & GO_FLG_CHECKED)
				lcd->draw_icon(GICON_CHECKED_SQUARE);
			else
				lcd->draw_icon(GICON_SQUARE);
		}
		lcd->pos_x =  client_rect.x0 + ((lcd->font->hspacing * 4)/3);
		if (type == GB_TYPE_SWITCH)	{											// switch
			// calculates the frame of the switch icon
			r_frame = rect;
			if (!(flags & GO_FLG_SELECTED)) {
				r_frame.y1--;
			}
			if(r_frame.width() > lcd->font->width *2)
				r_frame.x1 = r_frame.x0 + lcd->font->width *2;

			// calculates the size of the switch icon
			r_state.x1 = (r_frame.width()*3/5); // 60%
			if ((r_frame.width() - r_state.width()) & 1) {
				r_state.x1 --;
			}
			r_state.y1 = r_frame.height()/5;	// 20%
			if (r_state.y1 < 3 ) {
				if (r_frame.width() > 3) {
					r_state.y1 = 3;
				} else {
					r_state.y1 = r_frame.y1;
				}
			}
			r_state.Offset(r_frame.x0 + (r_frame.width() - r_state.width())/2,0);

			if (flags & GO_FLG_CHECKED) {
				r_state.Offset(0, r_frame.y0 + 2);
			} else {
				r_state.Offset(0, r_frame.y1 - 2 - r_state.height());
			}
			if (flags & GO_FLG_BORDER) {
				lcd->draw_vline(r_frame.y0, r_frame.y1, r_frame.x1);
			} else {
				lcd->draw_rectangle(r_frame);
			}
			lcd->draw_rectangle(r_state, (flags & GO_FLG_CHECKED));
			lcd->pos_x =  r_frame.x1 + lcd->font->hspacing/3; // 1/3 of (character width + character spacing)
		}
	}
	lcd->set_color(get_fg_color());
	if(flags & GO_FLG_SELECTED)
	{
		lcd->set_background(get_fg_color());
		r_state = client_rect;
		r_state.x0 = lcd->pos_x;
		if(flags & GO_FLG_BORDER)
			r_state.Inflate(0, 1);
		//lcd->clear_rect(RECT_T(lcd->pos_x/*client_rect.x0*/, client_rect.y0+1, client_rect.x1, client_rect.y1-1));
		lcd->clear_rect(r_state);
		lcd->set_color(get_bg_color());
	}

	text_metrics_t size = get_text_metrics(label.c_str(), client_rect.width() -lcd->pos_x, lcd->font);
	if (size.height > client_rect.height()) {
		draw_text_line(lcd, label.c_str(), label.length());
	} else {
		draw_text(lcd, label.c_str());
	}
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

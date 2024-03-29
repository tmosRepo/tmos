/*
 * gedit.h
 *
 *  Created on: 17.09.2013
 *      Author: edguest
 */

#ifndef GEDIT_H_
#define GEDIT_H_

#include <stdgui.h>
#include <gtext.h>
#include <gmenu.h>

#define EDIT_INPUT_TIME		777
#define EDIT_BLINK_TIME		500

#define EDIT_TIMER_INPUT	1
#define EDIT_TIMER_BLINK	2

struct key_to_char_ref_t
{
	const char *bg_vals;
	const char *en_vals;
};
extern const key_to_char_ref_t g_key_to_char_ref[];
extern const MENUTEMPLATE g_keyboard_menu[];

enum key_mode:unsigned char {KT_BG_CAPS=1, KT_BG, KT_EN_CAPS, KT_EN, KT_DIGIT};

struct GEdit:GText
{
	friend struct GEditVKB;

	text_metrics_t text_size;
	RECT_T cursor;
	struct{
	bool	cursor_on;
	unsigned int pos;
	char last_key;
	unsigned char times_pressed;
	unsigned int max_len;
	key_mode shift;
	GMenu* edit_menu;
	}__attribute__((packed));
	GEdit (	GId id_t, const RECT_T& rect_t, CSTRING txt_t, const char* caption_t = nullptr,
			GFlags flags_t = GO_FLG_DEFAULT, unsigned short edit_flags_t = ES_DEFAULT,
			const RENDER_MODE* font_t = &FNT5x7)
		:GText (id_t, rect_t, txt_t, caption_t, flags_t, edit_flags_t, font_t)
		 /*text_size (0), no need to initialize (there is a default constructor)*/
		,cursor_on(false), pos (txt.length()), last_key (0)
		,times_pressed (0), max_len(-1u), shift (KT_BG_CAPS), edit_menu(nullptr)
	{
		// checking the number of available buttons
#if KEYBOARD_WITH_ARROWS
		align |= ES_USE_VIRTUAL_KEYBOARD;
#endif
	}

	GEdit ()
		:GText ()
		 /*text_size (0), no need to initialize (there is a default constructor)*/
		 ,cursor_on(false), pos (0), last_key (0)
		 ,times_pressed (0), max_len(-1u), shift (KT_BG_CAPS), edit_menu(nullptr)
	{
		// checking the number of available buttons
#if KEYBOARD_WITH_ARROWS
		align |= ES_USE_VIRTUAL_KEYBOARD;
#endif
	}

	virtual ~GEdit();

	GUI_GET_OBJECT_TYPE(OBJECT_EDIT);

	unsigned int initialize (GMessage& msg) override;
	unsigned int process_key (GMessage& msg) override;
	unsigned int process_default(GMessage& msg) override;
	unsigned int process_sysctrl(GMessage& msg) override;

	void draw_this (LCD_MODULE* lcd) override;
	void move(int x, int y) override;

#if	KEYBOARD_WITH_ARROWS
	inline virtual bool ascii_enter_is_used( void ) const __attribute__((optimize("Os")))
	{
		return 1;
	}
#endif
protected:
	void process_alpha_key(char pressed_key, const char* key_val);
	bool process_char(unsigned int ch);
	bool set_cursor_on_char (void);
	bool set_cursor_y_char (void);
	bool set_cursor_x_char (void);
	void text_change();
	void pos_change(int val, bool modified_text=true);
	virtual void show_cursor();
private:
	unsigned int process_edit_menu_keys(GMessage& msg);
};


#endif /* GEDIT_H_ */

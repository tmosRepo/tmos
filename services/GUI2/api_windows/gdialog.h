/*
 * gdialog.h
 *
 *  Created on: 13.09.2013
 *      Author: edguest
 */

#ifndef GDIALOG_H_
#define GDIALOG_H_

#include <stdgui.h>
#include <gwindow.h>
#include <gscroll.h>

struct GDialog : GWindow
{

	GScroll* 	vscroll;
	int			vpos;

	GDialog():GWindow(), vscroll(nullptr),vpos(0) {};
	GDialog(GId id_t, const RECT_T& rect_t,
			unsigned char displays_t, GFlags flags_t = GO_FLG_DEFAULT)
	:GWindow (id_t, rect_t, displays_t, flags_t), vscroll(nullptr), vpos(0)
	{
	};

	virtual ~GDialog()
	{
		if(vscroll)
		{
			delete vscroll;
			vscroll = nullptr;
		}
	}

	GUI_GET_OBJECT_TYPE(OBJECT_DIALOG);

	void move(int x, int y) override;

protected:
	unsigned int process_key (GMessage& msg) override;
	unsigned int process_default (GMessage& msg) override;
	unsigned int process_command(GMessage& msg) override;
	void draw_this (LCD_MODULE* lcd) override;
	unsigned int initialize (GMessage& msg) override;
private:
	int move_objects(int offset, int& selected_indx) const;
};


#endif /* GDIALOG_H_ */

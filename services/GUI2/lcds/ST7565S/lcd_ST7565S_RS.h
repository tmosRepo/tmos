/*
 * lcd_ST7565S_RS.h
 *
 *  Created on: 03.09.2013
 *      Author: stanly
 */

#ifndef LCD_ST7565S_RS_H_
#define LCD_ST7565S_RS_H_


#include <lcd_ST7565S.h>


struct ST7565S_RS: public ST7565S
{
	ST7565S_RS(	unsigned int x, unsigned int y,
				unsigned int dx, unsigned int dy,
				HANDLE hnd, const PIN_DESC* p, unsigned int m):
		ST7565S(x, y, dx, dy, hnd, p, m)
	{
	}
	;


	//virtual functions
	void lcd_command(unsigned int cmd) override FAST_FLASH;
	void draw_point( int x,  int y) override FAST_FLASH;
	void draw_bitmap( int x0, int y0, const char* src, int width, int rows) override FAST_FLASH;
	void draw_char(int x0, unsigned int ch) override FAST_FLASH;
	void draw_hline( int x0,  int x1,  int y) override FAST_FLASH;
	void draw_bline( int x0,  int x1,  int y) override FAST_FLASH;
	void draw_vline( int y0,  int y1,  int x) override FAST_FLASH;
	void invert_vline( int y0,  int y1,  int x) override FAST_FLASH;
	void invert_hline( int x0,  int x1,  int y) override FAST_FLASH;
	virtual void update_screen() override FAST_FLASH;
	static char inline revert_char (char ch) FAST_FLASH;
};


#endif /* LCD_ST7565S_RS_H_ */

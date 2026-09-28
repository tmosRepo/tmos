#ifndef	LCD_H
#define	LCD_H

#include <hardware_cpp.h>
#include <fonts.h>
#include <stdgui.h>

#define ALL_LEFT		0
#define ALL_RIGHT 		1
#define ALL_CENTER		2

#define SIG_BACKLIGHT_TASK	1

/**
 *  62K colors
 */
#define PIX_BLACK 			0x000000
#define PIX_BLUE			0x00001f
#define PIX_GREEN			0x003F00
#define PIX_CYAN			0x003F1F
#define PIX_RED				0x1F0000
#define PIX_MAGENTA			0x1F001F
#define PIX_BROWN			0x080401
#define PIX_LIGHTGRAY		0x040404
#define PIX_DARKGRAY		0x020202
#define PIX_LIGHTBLUE		0x0A0D0E
#define PIX_LIGHTGREEN		0x090E09
#define PIX_LIGHTCYAN		0x0E0F0F
#define PIX_LIGHTRED		0x0F0808
#define PIX_LIGHTMAGENTA	0x0F080E
#define PIX_YELLOW			0x1F3F00
#define PIX_WHITE			0x1F3F1F
#define PIX_ORANGE			0x1F1F00

#define rgb(r, g, b)		((((r)&0x1F)<<16)|(((g)&0x3F)<<8)|((b)&0x1F))

#define PIX_SEQ_BLACK 			0x00, 0x00, 0x00
#define PIX_SEQ_BLUE			0x00, 0x00, 0x1f
#define PIX_SEQ_GREEN			0x00, 0x3F, 0x00
#define PIX_SEQ_CYAN			0x00, 0x3F, 0x1F
#define PIX_SEQ_RED				0x1F, 0x00, 0x00
#define PIX_SEQ_MAGENTA			0x1F, 0x00, 0x1F
#define PIX_SEQ_BROWN			0x08, 0x04, 0x01
#define PIX_SEQ_LIGHTGRAY		0x04, 0x04, 0x04
#define PIX_SEQ_DARKGRAY		0x02, 0x02, 0x02
#define PIX_SEQ_LIGHTBLUE		0x0A, 0x0D, 0x0E
#define PIX_SEQ_LIGHTGREEN		0x09, 0x0E, 0x09
#define PIX_SEQ_LIGHTCYAN		0x0E, 0x0F, 0x0F
#define PIX_SEQ_LIGHTRED		0x0F, 0x08, 0x08
#define PIX_SEQ_LIGHTMAGENTA	0x0F, 0x08, 0x0E
#define PIX_SEQ_YELLOW			0x1F, 0x3F, 0x00
#define PIX_SEQ_WHITE			0x1F, 0x3F, 0x1F
#define PIX_SEQ_ORANGE			0x1F, 0x1F, 0x00


#define PIX_SPI3		0x01000100
//pixel format
// 00000001 GGGBBBBB 00000001 RRRRRGGG
#define PIX_SPI3_R(x)			(x << 3 )
#define PIX_SPI3_G(x)			( ((x&07)<<21 ) | (x>>3))
#define PIX_SPI3_B(x)			(x << 16)
#define PIX_SPI3_RGB(r,g,b)		(PIX_SPI3 | PIX_SPI3_R(r) | PIX_SPI3_G(g) | PIX_SPI3_B(b))
//expansion macros
#define PIX_SPI3_COLOR(x)		PIX_SPI3_RGB(x)

#define PIX_SPI3_BLACK 			PIX_SPI3_COLOR(PIX_SEQ_BLACK 		)
#define PIX_SPI3_BLUE			PIX_SPI3_COLOR(PIX_SEQ_BLUE			)
#define PIX_SPI3_GREEN			PIX_SPI3_COLOR(PIX_SEQ_GREEN		)
#define PIX_SPI3_CYAN			PIX_SPI3_COLOR(PIX_SEQ_CYAN			)
#define PIX_SPI3_RED			PIX_SPI3_COLOR(PIX_SEQ_RED			)
#define PIX_SPI3_MAGENTA		PIX_SPI3_COLOR(PIX_SEQ_MAGENTA		)
#define PIX_SPI3_BROWN			PIX_SPI3_COLOR(PIX_SEQ_BROWN		)
#define PIX_SPI3_LIGHTGRAY		PIX_SPI3_COLOR(PIX_SEQ_LIGHTGRAY	)
#define PIX_SPI3_DARKGRAY		PIX_SPI3_COLOR(PIX_SEQ_DARKGRAY		)
#define PIX_SPI3_LIGHTBLUE		PIX_SPI3_COLOR(PIX_SEQ_LIGHTBLUE	)
#define PIX_SPI3_LIGHTGREEN		PIX_SPI3_COLOR(PIX_SEQ_LIGHTGREEN	)
#define PIX_SPI3_LIGHTCYAN		PIX_SPI3_COLOR(PIX_SEQ_LIGHTCYAN	)
#define PIX_SPI3_LIGHTRED		PIX_SPI3_COLOR(PIX_SEQ_LIGHTRED		)
#define PIX_SPI3_LIGHTMAGENTA	PIX_SPI3_COLOR(PIX_SEQ_LIGHTMAGENTA	)
#define PIX_SPI3_YELLOW			PIX_SPI3_COLOR(PIX_SEQ_YELLOW		)
#define PIX_SPI3_WHITE			PIX_SPI3_COLOR(PIX_SEQ_WHITE		)
#define PIX_SPI3_ORANGE			PIX_SPI3_COLOR(PIX_SEQ_ORANGE		)

//values stored in the RGB565 format (65K color)
//RRRRRGGG GGGBBBBB
#define PIX_SPI4_RGB(r,g,b)	(((r)<<11) | ((g)<<5) | (b))
//expansion macros
#define PIX_SPI4_COLOR(x)		PIX_SPI4_RGB(x)

#define PIX_SPI4_BLACK 			PIX_SPI4_COLOR(PIX_SEQ_BLACK 		)
#define PIX_SPI4_BLUE			PIX_SPI4_COLOR(PIX_SEQ_BLUE			)
#define PIX_SPI4_GREEN			PIX_SPI4_COLOR(PIX_SEQ_GREEN		)
#define PIX_SPI4_CYAN			PIX_SPI4_COLOR(PIX_SEQ_CYAN			)
#define PIX_SPI4_RED			PIX_SPI4_COLOR(PIX_SEQ_RED			)
#define PIX_SPI4_MAGENTA		PIX_SPI4_COLOR(PIX_SEQ_MAGENTA		)
#define PIX_SPI4_BROWN			PIX_SPI4_COLOR(PIX_SEQ_BROWN		)
#define PIX_SPI4_LIGHTGRAY		PIX_SPI4_COLOR(PIX_SEQ_LIGHTGRAY	)
#define PIX_SPI4_DARKGRAY		PIX_SPI4_COLOR(PIX_SEQ_DARKGRAY		)
#define PIX_SPI4_LIGHTBLUE		PIX_SPI4_COLOR(PIX_SEQ_LIGHTBLUE	)
#define PIX_SPI4_LIGHTGREEN		PIX_SPI4_COLOR(PIX_SEQ_LIGHTGREEN	)
#define PIX_SPI4_LIGHTCYAN		PIX_SPI4_COLOR(PIX_SEQ_LIGHTCYAN	)
#define PIX_SPI4_LIGHTRED		PIX_SPI4_COLOR(PIX_SEQ_LIGHTRED		)
#define PIX_SPI4_LIGHTMAGENTA	PIX_SPI4_COLOR(PIX_SEQ_LIGHTMAGENTA	)
#define PIX_SPI4_YELLOW			PIX_SPI4_COLOR(PIX_SEQ_YELLOW		)
#define PIX_SPI4_WHITE			PIX_SPI4_COLOR(PIX_SEQ_WHITE		)
#define PIX_SPI4_ORANGE			PIX_SPI4_COLOR(PIX_SEQ_ORANGE		)

#define BKLT_PIN_INDX	0
#define RST_PIN_INDX	1

#define MSB2LSB(x)			(						  \
								(((x)&0x001)?0x100:0) | \
								(((x)&0x002)?0x080:0) | \
								(((x)&0x004)?0x040:0) | \
								(((x)&0x008)?0x020:0) | \
								(((x)&0x010)?0x010:0) | \
								(((x)&0x020)?0x008:0) | \
								(((x)&0x040)?0x004:0) | \
								(((x)&0x080)?0x002:0) | \
								(((x)&0x100)?0x001:0)   \
							)



struct LCD_MODULE
{
	unsigned short size_x;
	unsigned short size_y;
	HANDLE lcd_hnd;
	const PIN_DESC* pins;
	unsigned short pos_x; //!< current draw pos
	unsigned short pos_y; //!< current draw pos
	unsigned short frame_y0;
	unsigned short frame_y1;
	unsigned short chars_per_row;
	unsigned short allign;
	const RENDER_MODE* font;
#if GUI_DISPLAYS > 1
	unsigned char	display;
#endif
protected:
	unsigned int color;
public:
	LCD_MODULE(unsigned int x, unsigned int y, HANDLE hnd, const PIN_DESC* p) :
		size_x(x), size_y(y), lcd_hnd(hnd), pins(p)
	{
	}
	;
	virtual ~LCD_MODULE(){};

	virtual void lcd_init(GUI_CB splash);
	virtual void lcd_reset()=0;
	virtual void backlight_signal(void);

	virtual void draw_bitmap(unsigned int x0, unsigned int y0,
			const unsigned char* src, unsigned int width, unsigned int rows)=0;
	virtual void draw_hline(unsigned int x0, unsigned int x1, unsigned int y)=0;
	virtual void draw_bline(unsigned int x0, unsigned int x1, unsigned int y)=0;
	virtual void draw_vline(unsigned int y0, unsigned int y1, unsigned int x)=0;
	virtual void invert_vline(unsigned int y0, unsigned int y1, unsigned int x)=0;
	virtual void invert_hline(unsigned int x0, unsigned int x1, unsigned int y)=0;
	virtual void update_screen()=0;
	virtual void clear_screen()=0;
	virtual void redraw_screen(WINDOW desktop)=0;
	virtual void pixel_by_x(unsigned int x)=0;
	virtual void invert_pixel_by_x(unsigned int x)=0;
	virtual void set_color(unsigned int rgb)=0;
	virtual const RENDER_MODE* default_font()=0;

	void set_font(const RENDER_MODE* afont);
	void set_xy_all(unsigned int xy, unsigned int all);
	void clear_rect(unsigned int x0, unsigned int y0, unsigned int x1,
			unsigned int y1);
	const char* get_next_txt_row(const char *txt) const;
	const char* draw_text(const char *txt);
	const char* draw_text_no_space(const char *txt);
	const char* draw_row(const char *txt);
	void lcd_single_window(GUI_CB callback);

	void draw_line(int x0, int y0, int x1, int y1);
	void draw_circle(int x0, int y0, int radius, int sectors = 0xFF);
	void fill_circle(int x0, int y0, int radius);
	void draw_point(int x0, int y0);
protected:
	virtual size_t cmd_address_size() const = 0;
};

#endif

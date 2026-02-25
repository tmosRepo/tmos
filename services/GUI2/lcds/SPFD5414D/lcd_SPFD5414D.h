/*
 * lcd_SPFD5414D.h
 *
 *  Created on: 30.01.2011
 *      Author: Miroslav Kostadinov
 */

#ifndef LCD_SPFD5414D_H_
#define LCD_SPFD5414D_H_

#include <lcd.h>

// The indices 0(backlight) and 1(reset) are defined in lcd.h
// BKLT_PIN_INDX	0
// RST_PIN_INDX		1

// chip select
#ifndef CSX_PIN_INDX
#define CSX_PIN_INDX	2
#endif
// serial clock
#ifndef SCL_PIN_INDX
#define SCL_PIN_INDX	3
#endif
// serial data (can be bidirectional or output only)
#ifndef SDA_PIN_INDX
#define SDA_PIN_INDX	4
#endif
// Data/Command select
#ifndef DCX_PIN_INDX
#define DCX_PIN_INDX	5
#endif



#define SPFD5414D_NOP 				0x00
#define SPFD5414D_DISPLAYOFF 		0x28
#define SPFD5414D_DISPON 			0x29	//!< [0] Display Om
#define SPFD5414D_SLPOUT 			0x11	//!< [0] Sleep Out
#define SPFD5414D_NORON				0x13	//!< [0] Normal Display On

#define SPFD5414D_PWCTR1			0xC0	//!< [2] Power Control 1 (VRH, VCI)

#define SPFD5414D_SWRESET 			0x01
#define SPFD5414D_RDDPM 			0x0a
#define SPFD5414D_RDDMADCTR 		0x0b
#define SPFD5414D_RDDCOLMOD 		0x0c
#define SPFD5414D_RDDCIM 			0x0d
#define SPFD5414D_RDID4 			0xd3
#define SPFD5414D_CASET 			0x2a	//!< [4] Column Address Set (XS, XE)
#define SPFD5414D_RASET 			0x2b	//!< [4] Row Address Set (YS, YE)
#define SPFD5414D_RAMWR 			0x2c	//!< [N] Memory Write
#define SPFD5414D_RAMRD 			0x2e
#define SPFD5414D_RDDSDR 			0x0f
#define SPFD5414D_RDDID 			0x04
#define SPFD5414D_RDCOLMD 			0x0c
#define SPFD5414D_RDDST 			0x09
#define SPFD5414D_COLMOD 			0x3a	//!< [1] Interface Pixel Format (color mode)
#define SPFD5414D_MADCTR 			0x36	//!< [1] Memory Data Access Control
#define SPFD5414D_RGBSET 			0x2d

#define SPFD5414D_RDID1				0xDA
#define SPFD5414D_RDID2				0xDB
#define SPFD5414D_RDID3				0xDC

#define SPFD5414D_DATA(data)		(0x0100 |(data))
#define SPFD5414D_CMD(cmd)			(cmd)

#define SPFD5414D_COLMOD_12BPP		0x3
#define SPFD5414D_COLMOD_16BPP		0x5
#define SPFD5414D_COLMOD_18BPP		0x6

#define SPFD5414D_MADCTR_MY			0x80	//!< Row address order
#define SPFD5414D_MADCTR_MX			0x40	//!< Column address order
#define SPFD5414D_MADCTR_MV			0x20	//!< Row/Column exchange
#define SPFD5414D_MADCTR_ML			0x10	//!< Vertical refresh order
#define SPFD5414D_MADCTR_BGR		0x08	//!< RGB-BGR order (1=BGR)


class disp_buf_t {
private:
    size_t 	width;
    uint8_t *buf;
public:
    disp_buf_t() : width(0), buf(nullptr) {}

    disp_buf_t(size_t width_, size_t height_)
    	: width(width_)  {
    	buf = new TCM uint8_t[width_ * height_];
    }

    disp_buf_t(uint8_t* buf_, size_t width_)
        : width(width_), buf(buf_) {}

    inline uint8_t* operator[](size_t y)
    __attribute__((optimize("Os"), always_inline)) {

    	return (buf + y * width);
    }

    inline const uint8_t* operator[](size_t y) const
    __attribute__((optimize("Os"), always_inline)){
        return (buf + y * width);
    }
};

struct SPFD5414D: public LCD_MODULE
{
protected:
	unsigned int 	reset_timeout;
	disp_buf_t 		disp_buf;
	void*			tft_buf;

public:
	SPFD5414D(	unsigned int x, unsigned int y,
				unsigned int dx, unsigned int dy,
				HANDLE hnd, const PIN_DESC* p, const RENDER_MODE* _font = GUI_LCD_FONT)
		: LCD_MODULE(x, y, dx, dy, hnd, p, _font)
		, reset_timeout(0)
		, disp_buf(x/2, y)
	{
		if (p[DCX_PIN_INDX]) {
			tft_buf = new unsigned short int[x];
		} else {
			tft_buf = new unsigned int[x];
		}
	}

	//virtual functions
	void lcd_reset() override;
	void do_reset() override;
	void clear_screen() override;
	void draw_bitmap(int x0, int y0, const char* src,  int width,  int rows) override;
	void draw_char(int x0, unsigned int ch) override;
	void draw_point( int x,  int y) override;
	void draw_hline( int x0,  int x1,  int y) override;
	void draw_bline( int x0,  int x1,  int y) override;
	void clear_rect (const RECT_T& area) override;
	void draw_vline( int y0,  int y1,  int x) override;
	void invert_vline( int y0,  int y1,  int x) override;
	void invert_hline( int x0,  int x1,  int y) override;
	void update_screen() override
	{;}
	void redraw_screen(GObject* object, RECT_T area) override;
	void direct_write (GSplash draw_cb) override;
	void adjust_for_screen (GObject** object, RECT_T &area) override;

protected:
	virtual void tft_write_row(unsigned short address_cmd[]);
	virtual void tft_init_address_cmd(unsigned short address_cmd[]);
	virtual void tft_use_foreground_color(const int x_pos, const int y_pos);
	virtual void tft_use_background_color(const int x_pos, const int y_pos);
	virtual void tft_invert_color(const int x_pos, const int y_pos);
};

/**
 * TFT_CHECK class can be used to detect the LCD MODULE installed.
 * It is using GPIOs only so it can be called from AppInit()
 *
 */
struct TFT_CHECK
{
	const PIN_DESC* pins;
	unsigned int z_bits;
	unsigned int ID_24bits;
	unsigned int ID_3x8bits;
	bool dcx;
	TFT_CHECK(const PIN_DESC* p)
	: pins(p)
	, z_bits(0)
	, ID_24bits(0)
	, ID_3x8bits(0)
	{
		if(pins[DCX_PIN_INDX]) {
			dcx = true;
		} else {
			dcx = false;
		}
	}

	static void delay(unsigned int time = 0)
	{
		if (time) {
			tsk_sleep(time);
		}
	}
	void tft_write(unsigned int value, unsigned int bits=9);
	unsigned int tft_read(unsigned int bits=24);

	unsigned int read_id();
	bool id(unsigned int bits=9);
	void tft_reset();
};

#endif /* LCD_SPFD5414D_H_ */

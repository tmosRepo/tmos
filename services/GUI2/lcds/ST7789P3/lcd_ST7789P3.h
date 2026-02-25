/*
 * lcd_ST7789P3.h
 *
 *  Created on: 07.07.2025
 *      Author: Stanislav Bratkov
 */

#ifndef LCD_ST7789P3_H_
#define LCD_ST7789P3_H_

#include <lcd_SPFD5414D.h>

//#include <lcd.h>
//#include <tmos_stdlib.h>
//==============================================================================
//	MTF014QV05A-V1
//  240 x 240 dots
//#define MTF014QV05A_X			240
//#define MTF014QV05A_X_OFFSET	0
//#define MTF014QV05A_Y			240
//#define MTF014QV05A_Y_OFFSET	80
//==============================================================================

#define ST7789P3_X_SIZE			240
#define ST7789P3_Y_SIZE			320

#define ST7789P3_NOP 			0x00	// x
#define ST7789P3_DISPLAYOFF 	0x28
#define ST7789P3_DISPON 		0x29	//!< [0] Display Om
#define ST7789P3_SLPOUT 		0x11	//!< [0] Sleep Out
#define ST7789P3_NORON			0x13	//!< [0] Normal Display On

#define ST7789P3_PWCTR1			0xC0	//!< [2] Power Control 1 (VRH, VCI)

#define ST7789P3_SWRESET 		0x01	// x
#define ST7789P3_RDDPM 			0x0a
#define ST7789P3_RDDMADCTR 		0x0b
#define ST7789P3_RDDCOLMOD 		0x0c
#define ST7789P3_RDDCIM 		0x0d
#define ST7789P3_RDID4 			0xd3
#define ST7789P3_CASET 			0x2a	//!< [4] Column Address Set (XS, XE)
#define ST7789P3_RASET 			0x2b	//!< [4] Row Address Set (YS, YE)
#define ST7789P3_RAMWR 			0x2c	//!< [N] Memory Write
#define ST7789P3_RAMRD 			0x2e
#define ST7789P3_RDDSDR 		0x0f
#define ST7789P3_RDDID 			0x04
#define ST7789P3_RDCOLMD 		0x0c
#define ST7789P3_RDDST 			0x09
#define ST7789P3_COLMOD 		0x3a	//!< [1] Interface Pixel Format (color mode)
#define ST7789P3_MADCTR 		0x36	//!< [1] Memory Data Access Control
#define ST7789P3_RGBSET 		0x2d
#define ST7789P3_WRDISBV		0x51	//!< [1] Write Display Brightness 00 min / FF max

#define ST7789P3_DATA(data)		(0x0100 |(data))
#define ST7789P3_CMD(cmd)		(cmd)

#define ST7789P3_COLMOD_12BPP	0x3
#define ST7789P3_COLMOD_16BPP	0x55
#define ST7789P3_COLMOD_18BPP	0x6

#define ST7789P3_MADCTR_MY		0x80	//!< Row address order (Y-Mirror)
#define ST7789P3_MADCTR_MX		0x40	//!< Column address order (X-Mirror)
#define ST7789P3_MADCTR_MV		0x20	//!< Row/Column exchange
#define ST7789P3_MADCTR_ML		0x10	//!< Vertical refresh order
#define ST7789P3_MADCTR_BGR		0x08	//!< RGB-BGR order (1=BGR)


struct ST7789P3: public SPFD5414D
{
protected:
	const int x_offset;
	const int y_offset;
public:
	ST7789P3(	const unsigned int x, const unsigned int y,
				const unsigned int dx, const unsigned int dy,
				HANDLE hnd, const PIN_DESC* p,
				const RENDER_MODE* _font = GUI_LCD_FONT,
				const int x_offset_=0, const int y_offset_=80)
		:SPFD5414D(x, y, dx, dy, hnd, p, _font)
		, x_offset(x_offset_)
		, y_offset(y_offset_)
	{ ;	}

	//virtual functions
	void lcd_reset() override;
	void redraw_screen(GObject* object, RECT_T area) override;
	void direct_write (GSplash draw_cb) override;
	void adjust_for_screen (GObject** object, RECT_T &area) override;

protected:
    virtual void tft_sleep_out();
    virtual void tft_regs_init();
	virtual void tft_set_address(unsigned short address_cmd[]);
    void tft_write_row(unsigned short address_cmd[]) override;
	void tft_init_address_cmd(unsigned short address_cmd[]) override;
};

#endif /* LCD_ST7789P3_H_ */

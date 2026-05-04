/*
 * ST7789P3.cpp
 *
 *  Created on: 07.07.2025
 *      Author: Stanislav Bratkov
 */

#include <tmos.h>
#include <lcd_ST7789P3.h>
#include <fam_cpp.h>
#include <gwindow.h>


#define LCD_DEF_FONT		&FNT7x9
#define LCD_DEF_COL			PIX_WHITE


static const unsigned int lut_to_tft_color[16] =
{
	PIX_RGB_BLACK,
	PIX_RGB_BLUE,
	PIX_RGB_GREEN,
	PIX_RGB_RED,
	PIX_RGB_LIGHTMAGENTA,
	PIX_RGB_BROWN,
	PIX_RGB_LIGHTBLUE,
	PIX_RGB_LIGHTGRAY,
	PIX_RGB_DARKGRAY,
	PIX_RGB_LIGHTGREEN,
	PIX_RGB_LIGHTCYAN,
	PIX_RGB_LIGHTRED,
	PIX_RGB_CYAN,
	PIX_RGB_MAGENTA,
	PIX_RGB_YELLOW,
	PIX_RGB_WHITE
};

static const unsigned short tft_init[] =
{
	// ColorModeSet 16bpp
	ST7789P3_COLMOD, ST7789P3_DATA(ST7789P3_COLMOD_16BPP ),
	// MemoryAccess BGR
	ST7789P3_MADCTR, ST7789P3_DATA( 0
			//| ST7789P3_MADCTR_MY
			//| ST7789P3_MADCTR_MX
			//| ST7789P3_MADCTR_ML
			| ST7789P3_MADCTR_BGR
			),
	// Display on
	ST7789P3_NORON,
	ST7789P3_DISPON,
	// Write Display Brightness
	ST7789P3_WRDISBV, ST7789P3_DATA(0xFF),
	// ColumnAddressSet( x0, x0+dx-1 );
	ST7789P3_CASET, ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(127),
	// RowAddressSet( y0, y0+dy-1 );
	ST7789P3_RASET, ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(127),
};

static const unsigned short tft_init_address[] =
{
	ST7789P3_CASET, ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(127),
	ST7789P3_RASET, ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0),
	ST7789P3_RAMWR
};

FILE_OPTIMIZE("Os")

#if PIX_FORMAT == PIX_4BIT_FORMAT
/**
 * function is a placeholder for compatibility. no need for conversion if we're
 * using the 4bit PIX format
 */
static inline __attribute__((always_inline)) tft_color_lut pix_to_4bit(color_t color)
{
	return tft_color_lut(color);
}
#else
/**
 * converts a PIX format color (which may be RGB565 or 4byte PIX, depends on the
 * PIX_FORMAT flag) into 4bit color, for use in the disp_buf. converts only the
 * pre-determined 16 colors, if another color is given it returns the default
 * value of white
 * @param color_t: PIX color or RGB565 color
 * @return: the same 4bit color, or white if unsupported
 */
static tft_color_lut pix_to_4bit(unsigned int color_t)
{
	switch (color_t)
	{
	case PIX_BLACK:
		return lut_black;
# if !GUI_MONOCHROME
	case PIX_BLUE:
		return lut_blue;
	case PIX_GREEN:
		return lut_green;
	case PIX_CYAN:
		return lut_cyan;
	case PIX_RED:
		return lut_red;
	case PIX_MAGENTA:
		return lut_magenta;
	case PIX_BROWN:
		return lut_brown;
	case PIX_LIGHTGRAY:
		return lut_lightgray;
	case PIX_DARKGRAY:
		return lut_darkgray;
	case PIX_LIGHTBLUE:
		return lut_lightblue;
	case PIX_LIGHTGREEN:
		return lut_lightgreen;
	case PIX_LIGHTCYAN:
		return lut_lightcyan;
	case PIX_LIGHTRED:
		return lut_lightred;
	case PIX_LIGHTMAGENTA:
		return lut_lightmagenta;
	case PIX_YELLOW:
		return lut_yellow;
# endif
	default:
		return lut_white;
	}
}
#endif

FILE_RESET_OPTIONS

void ST7789P3::adjust_for_screen (GObject** object, RECT_T& area)				//change the area and object to desktop if the display draws only rows or columns
{
																				//          *** example for display that draws rows ***
																				//area.x0 = 0;
																				//area.x1 = size_x - 1;
																				//*object = children;
}

//------------------------------------------------------------------------------
//The TFT modules hardware interface methods
//------------------------------------------------------------------------------
void ST7789P3::tft_sleep_out()
{
	// sleep out
	const short	cmd = ST7789P3_SLPOUT;
	if (pins[DCX_PIN_INDX]) {
		PIO_Assert(pins[DCX_PIN_INDX]);		// Low Command
		lcd_hnd->tsk_write(&cmd, 1);
		PIO_Deassert(pins[DCX_PIN_INDX]); 	// High Data
	} else {
		lcd_hnd->tsk_write(&cmd, 1);
	}
}

void ST7789P3::tft_regs_init()
{
	if (pins[DCX_PIN_INDX]) {
		// 8 - BIT SPI
		const char* data = (const char*)tft_init;
		unsigned indx=0;
		for (; indx < sizeof(tft_init)/2; indx++, data+=2) {
			if(!data[1]) {
				PIO_Assert(pins[DCX_PIN_INDX]);		// Low Command
				lcd_hnd->tsk_write(&data, 1);
				PIO_Deassert(pins[DCX_PIN_INDX]); 	// High Data
			} else {
				lcd_hnd->tsk_write(&data, 1);
			}
		}
	} else {
		// 9 - BIT SPI
		lcd_hnd->tsk_write(tft_init, sizeof(tft_init)/2);
	}
}

void ST7789P3::lcd_reset()
{
	tft_sleep_out();
	tsk_sleep(120);
	tft_regs_init();
}


void ST7789P3::tft_init_address_cmd(unsigned short address_cmd[])
{
	memcpy(address_cmd, tft_init_address, sizeof(tft_init_address));
}

void ST7789P3::tft_set_address(unsigned short address_cmd[])
{
	if (pins[DCX_PIN_INDX]) {
		const char* data = (const char*)address_cmd;
		unsigned indx=0;
		for (; indx < sizeof(tft_init_address)/2; indx++, data+=2) {
			if(!data[1]) {
				PIO_Assert(pins[DCX_PIN_INDX]);
				lcd_hnd->tsk_write(&data, 1);
				PIO_Deassert(pins[DCX_PIN_INDX]);
			} else {
				lcd_hnd->tsk_write(&data, 1);
			}
		}
	} else {
		lcd_hnd->tsk_write(address_cmd, sizeof(tft_init_address)/2);
	}
}

void ST7789P3::tft_write_row(unsigned short address_cmd[])
{
/*		0					1					2				3					4
  	ST7789P3_CASET, ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(127),
		5					6					7				8					9
	ST7789P3_RASET, ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0),
	ST7789P3_RAMWR
 */
	address_cmd[2] = ST7789P3_DATA(frame.x0); address_cmd[4] = ST7789P3_DATA(frame.x1);
	address_cmd[6] = address_cmd[8] = ST7789P3_DATA(((frame.y0 + y_offset)>>8));
	address_cmd[7] = address_cmd[9] = ST7789P3_DATA((frame.y0 + y_offset) & 0xFF);
	//TODO: use_dcx_pin
	unsigned int * dst = (unsigned int *)tft_buf;
	for(int x= 0; x < ST7789P3_X_SIZE/2; x++)
	{
		*dst++ = lut_to_tft_color[(disp_buf[frame.y0][x] & 0xF0)>>4];
		*dst++ = lut_to_tft_color[disp_buf[frame.y0][x] & 0x0F];
	}
	tft_set_address(address_cmd);
	lcd_hnd->tsk_write(((unsigned int *)tft_buf)+frame.x0, frame.x1 - frame.x0 +1);
}

void ST7789P3::redraw_screen (GObject* object, RECT_T area)
{
	if(area.normalize(rect))
	{
		unsigned short cmd_address[sizeof(tft_init_address)/sizeof(unsigned short)];
		frame = area;
#if GUI_DEBUG
		uint32_t t = CURRENT_TIME;
		GUI_TRACELN("LCD%u update {%u,%u %u,%u}", displays, frame.x0, area.y0, frame.x1, area.y1);
#endif
		tft_init_address_cmd(cmd_address);
		for (frame.y0=area.y0; frame.y0 <= area.y1; frame.y0++)
		{
			tft_write_row(cmd_address);
		}
#if GUI_DEBUG
	    GUI_TRACE(" %u ms", ms_since(t));
#endif
	}
}


void ST7789P3::direct_write(GSplash draw_cb)
{
	unsigned short cmd_address[sizeof(tft_init_address)/sizeof(unsigned short)];

	clear_screen();
	frame.x0 = 0;
	frame.x1 = size_x-1;
	tft_init_address_cmd(cmd_address);
    for(frame.y0=0; frame.y0 < size_y; frame.y0++)
    {
    	frame.y1 = frame.y0+1;
    	draw_cb(this);
    	tft_write_row(cmd_address);
    }
}



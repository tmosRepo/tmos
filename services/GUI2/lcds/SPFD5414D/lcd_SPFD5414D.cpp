/*
 * SPFD5414D.cpp
 *
 *  Created on: 23.01.2011
 *      Author: Miroslav Kostadinow
 */

#include <tmos.h>
#include <lcd_SPFD5414D.h>
#include <fam_cpp.h>
#include <gwindow.h>


#define LCD_DEF_FONT		&FNT7x9
#define LCD_DEF_COL			PIX_WHITE

/*
enum tft_color_lut:unsigned char
{
	lut_black =0,
	lut_blue,
	lut_green,
	lut_cyan,
	lut_red,
	lut_magenta,
	lut_brown,
	lut_lightgray,
	lut_darkgray,
	lut_lightblue,
	lut_lightgreen,
	lut_lightcyan,
	lut_lightred,
	lut_lightmagenta,
	lut_yellow,
	lut_white
};
*/

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
	SPFD5414D_SLPOUT,
	// ColorModeSet 16bpp
	SPFD5414D_COLMOD, SPFD5414D_DATA(SPFD5414D_COLMOD_16BPP ),
	// MemoryAccess BGR
	SPFD5414D_MADCTR, SPFD5414D_DATA(SPFD5414D_MADCTR_MY | SPFD5414D_MADCTR_MX | SPFD5414D_MADCTR_ML | SPFD5414D_MADCTR_BGR),
	// Display on
	SPFD5414D_NORON,
	SPFD5414D_DISPON,
	// ColumnAddressSet( x0, x0+dx-1 );
	SPFD5414D_CASET, SPFD5414D_DATA(0), SPFD5414D_DATA(0), SPFD5414D_DATA(0), SPFD5414D_DATA(127),
	// RowAddressSet( y0, y0+dy-1 );
	SPFD5414D_RASET, SPFD5414D_DATA(0), SPFD5414D_DATA(0), SPFD5414D_DATA(0), SPFD5414D_DATA(127),
};

static const unsigned short tft_init_address[] =
{
	SPFD5414D_CASET, SPFD5414D_DATA(0), SPFD5414D_DATA(0), SPFD5414D_DATA(0), SPFD5414D_DATA(127),
	SPFD5414D_RASET, SPFD5414D_DATA(0), SPFD5414D_DATA(0), SPFD5414D_DATA(0), SPFD5414D_DATA(0),
	SPFD5414D_RAMWR
};

void SPFD5414D::lcd_reset()
{
	lcd_hnd->tsk_write(tft_init, sizeof(tft_init)/2);
}

void SPFD5414D::do_reset()
{
	if( ms_since(reset_timeout) > 500 )
	{
		reset_timeout = CURRENT_TIME;
		lcd_reset();
	}
}

void SPFD5414D::clear_screen()
{
//	memclr(disp_buf, sizeof(disp_buf));
	memclr(disp_buf[0], size_y * (size_x/2));

}

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

void SPFD5414D::tft_use_foreground_color(const int x_pos, const int y_pos)
{
	if (frame.x0 <= x_pos && x_pos <= frame.x1)
	{
		unsigned char pix = disp_buf[y_pos][x_pos >> 1];
		unsigned char pix_color;

		pix_color = pix_to_4bit(fg_color);

		if (x_pos & 1)
			pix = (pix & 0xF0) | pix_color;
		else
			pix = (pix & 0x0F) | (pix_color << 4);

		disp_buf[y_pos][x_pos >> 1] = pix;
	}
}

void SPFD5414D::tft_use_background_color(const int x_pos, const int y_pos)
{
	if (frame.x0 <= x_pos && x_pos <= frame.x1)
	{
		unsigned char pix = disp_buf[y_pos][x_pos >> 1];
		unsigned char pix_color;

		pix_color = pix_to_4bit(bg_color);

		if (x_pos & 1)
			pix = (pix & 0xF0) | pix_color;
		else
			pix = (pix & 0x0F) | (pix_color << 4);

		disp_buf[y_pos][x_pos >> 1] = pix;
	}
}

void SPFD5414D::tft_invert_color(const int x_pos, const int y_pos)
{
	if (frame.x0 <= x_pos && x_pos <= frame.x1)
	{
		unsigned char pix = disp_buf[y_pos][x_pos >> 1];
		unsigned char pix_color;

		if (x_pos & 1)
		{
			pix_color = lut_white - (pix & 0x0F);
			pix = (pix & 0xF0) | pix_color;
		}
		else
		{
			pix_color = (lut_white << 4) - (pix & 0xF0);
			pix = (pix & 0x0F) | pix_color;
		}

		disp_buf[y_pos][x_pos >> 1] = pix;
	}
}

void SPFD5414D::draw_bitmap( int x0, int y0, const char* src, int width, int rows)
{
	int offset=0;
	if(y0 < frame.y0)
	{
		offset = frame.y0 - y0;
		if(offset >= rows)
			return;
		y0 += offset;
		rows -= offset;
		offset *= width;
		src += offset /8;
		offset %= 8;
	}
	if(y0 < frame.y1)
	{
		rows += y0;
		if(rows > frame.y1)
			rows = frame.y1;
		rows -= y0;

		offset = 1<< offset;
		width += x0;
		while(rows--)
		{
			for( int i=x0; i<width; i++)
			{
				if(src[0] & offset)
				{
					tft_use_foreground_color(i, y0);
				}
				offset <<= 1;
				if(offset > 255)
				{
					offset = 1;
					src++;
				}
			}
			y0++;
		}

	}
}

void SPFD5414D::draw_char(int x0, unsigned int ch)
{
	if(ch > 0x1f && font)
	{
		ch -= 0x20;
		int y0 = pos_y;
		const char* src = font->font_data + ch * font->char_bytes;
		int width = font->width;
		int rows = font->height;
		int offset=0;
		if(y0 < frame.y0)
		{
			offset = frame.y0 - y0;
			if(offset >= rows)
				return;
			y0 += offset;
			rows -= offset;
			offset *= width;
			src += offset /8;
			offset %= 8;
		}
		if(y0 < frame.y1)
		{
			rows += y0;
			if(rows > frame.y1)
				rows = frame.y1;
			rows -= y0;

			offset = 1<< offset;
			width += x0;
			while(rows--)
			{
				for( int i=x0; i<width; i++)
				{
					if(src[0] & offset)
					{
						tft_use_foreground_color(i, y0);
					}
					offset <<= 1;
					if(offset > 255)
					{
						offset = 1;
						src++;
					}
				}
				y0++;
			}

		}
	}
}

void SPFD5414D::draw_point( int x, int y)
{
	if(frame.y0 <= y && y < frame.y1 && frame.x0 <= x && x <= frame.x1)
	{
		tft_use_foreground_color(x, y);
	}
}

void SPFD5414D::draw_hline( int x0, int x1, int y)
{
	if( (y>=frame.y0) && (y<frame.y1))
	{
		while(x0 <= x1)
		{
			tft_use_foreground_color(x0++, y);
		}
	}
}

void SPFD5414D::draw_bline( int x0, int x1, int y)
{
	if( (y>=frame.y0) && (y<frame.y1))
	{
		while(x0 <= x1)
		{
			tft_use_background_color(x0++, y);
		}
	}
}

void SPFD5414D::clear_rect (const RECT_T& area)
{
	RECT_T clr_area(area);
	if(clr_area.normalize(frame))
	{
		int y = clr_area.y0;
		while(y < clr_area.y1)
		{
			int x0 = clr_area.x0, x1 = clr_area.x1;
			if(x0 & 1)
			{
				tft_use_background_color(x0++, y);
			}

			if(x1 & 1)
				x1++;
			else
			{
				tft_use_background_color(x1, y);
			}
			if(x1 > x0 && (x1-x0) >= 2)
				memset(&disp_buf[y][x0 >> 1], pix_to_4bit(bg_color) | (pix_to_4bit(bg_color) << 4), (x1 -x0)>>1);
			y++;
		}
	}
}


void SPFD5414D::draw_vline( int y0, int y1, int x)
{
	if(frame.x0 <= x && x <= frame.x1)
	{
		if(y0 <= frame.y0)
			y0 = frame.y0;
		while(y0 <= y1 && y0 < frame.y1)
		{
			if (y0>=frame.y0)
			{
				tft_use_foreground_color(x, y0);
			}else
				break;
			y0++;
		}
	}
}

void SPFD5414D::invert_vline( int y0, int y1, int x)
{
	if(frame.x0 <= x && x <= frame.x1)
	{
		if(y0 <= frame.y0)
			y0 = frame.y0;
		while(y0 <= y1 && y0 < frame.y1)
		{
			if (y0>=frame.y0)
			{
				tft_invert_color(x, y0);
			}else
				break;
			y0++;
		}
	}
}

void SPFD5414D::invert_hline( int x0, int x1, int y)
{
	if( (y>=frame.y0) && (y<frame.y1))
	{
		while(x0 <= x1)
		{
			tft_invert_color(x0++, y);
		}
	}
}

FILE_RESET_OPTIONS

void SPFD5414D::adjust_for_screen (GObject** object, RECT_T& area)				//change the area and object to desktop if the display draws only rows or columns
{
																				//          *** example for display that draws rows ***
																				//area.x0 = 0;
																				//area.x1 = size_x - 1;
																				//*object = children;
}

//
//The TFT modules hardware interface methods

void SPFD5414D::tft_init_address_cmd(unsigned short address_cmd[])
{
	memcpy(address_cmd, tft_init_address, sizeof(tft_init_address));
}

void SPFD5414D::tft_write_row(unsigned short address_cmd[])
{
/*		0					1					2				3					4
  	SPFD5414D_CASET, SPFD5414D_DATA(0), SPFD5414D_DATA(0), SPFD5414D_DATA(0), SPFD5414D_DATA(127),
		5					6					7				8					9
	SPFD5414D_RASET, SPFD5414D_DATA(0), SPFD5414D_DATA(0), SPFD5414D_DATA(0), SPFD5414D_DATA(0),
	SPFD5414D_RAMWR
 */
	address_cmd[2] = SPFD5414D_DATA(frame.x0); address_cmd[4] = SPFD5414D_DATA(frame.x1);
	address_cmd[7] = SPFD5414D_DATA(frame.y0); address_cmd[9] = SPFD5414D_DATA(frame.y0);
	//TODO: use_dcx_pin
	unsigned int * dst = (unsigned int *)tft_buf;
	for(int x= 0; x < size_x/2; x++)
	{
		*dst++ = lut_to_tft_color[(disp_buf[frame.y0][x] & 0xF0)>>4];
		*dst++ = lut_to_tft_color[disp_buf[frame.y0][x] & 0x0F];
	}
	lcd_hnd->tsk_write(address_cmd, sizeof(tft_init_address)/2);
	lcd_hnd->tsk_write(((unsigned int *)tft_buf)+frame.x0, frame.x1 - frame.x0 +1);
}

void SPFD5414D::redraw_screen (GObject* object, RECT_T area)
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


void SPFD5414D::direct_write(GSplash draw_cb)
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

//------------------------------------------------------------------------------
#define SDA 	(pins[SDA_PIN_INDX])
#define SCL		(pins[SCL_PIN_INDX])
#define CSX		(pins[CSX_PIN_INDX])
#define DCX		(pins[DCX_PIN_INDX])
#define RST		(pins[RST_PIN_INDX])

#define INPUT_SETUP_TIME	2
/*
#define PINS_TRACE(str)	do { \
	TRACELN("%s  D%u   CL%u   CS%u", str, PIO_Read(SDA)?1:0, PIO_Read(SCL)?1:0, PIO_Read(CSX)?1:0); \
	tsk_sleep(2); }while(0)
*/
FILE_OPTIMIZE("Os")

unsigned int TFT_CHECK::tft_read( unsigned int bits )
{
	unsigned int res = 0;
	PIN_DESC SDI = SDA & ~(PD_PULL_DOWN|PD_PULL_UP);

	PIO_CfgInput(SDI|PD_PULL_UP);
	// data bits

	for (unsigned int i = 0; i < bits; i++)
	{
		PIO_ClrOutput(SCL);

		res <<= 1;
		z_bits <<= 1;
		if (PIO_Read(SDA)) {
			res ++;
			PIO_CfgInput(SDI|PD_PULL_DOWN);
			delay(INPUT_SETUP_TIME);
			if (!PIO_Read(SDI)) {
				z_bits++;
			}
			PIO_CfgInput(SDI|PD_PULL_UP);
			delay(INPUT_SETUP_TIME);
		}
		PIO_SetOutput(SCL);
	}

	PIO_ClrOutput(SCL);
	return (res);
}

void TFT_CHECK::tft_write(unsigned int value, unsigned int bits)
{
	int i = 0;
	//SET SDA to Port
	PIO_CfgOutput0(SDA);

	if (dcx && bits == 8) {
		i=1;
		value <<= 1;
		PIO_CfgOutput0(DCX);
	}

	//9 data bits
	for (; i < 9; i++)
	{
		PIO_ClrOutput(SCL);

		//    TFT_SDA = nByte & Mask8[i] ? 1 : 0;//51 msec
		if(value & 0x100)
			PIO_SetOutput(SDA);
		else
			PIO_ClrOutput(SDA);
		value <<= 1;
		PIO_SetOutput(SCL);
	}
	PIO_CfgInput(SDA);

	PIO_ClrOutput(SCL);
	if (dcx) {
		PIO_SetOutput(DCX);
	}
}

void TFT_CHECK::tft_reset()
{
    PIO_CfgOutput1(RST);

    PIO_ClrOutput(RST);
	delay(5);
	PIO_SetOutput(RST);
	delay(150);


    PIO_CfgOutput0(SCL);
    PIO_CfgOutput1(CSX);

#if 0
	PIO_ClrOutput(CSX);
	delay(5);
	tft_write(SPFD5414D_SWRESET);
	delay(5);
	PIO_SetOutput(CSX);
	delay(150);

	PIO_ClrOutput(CSX);
	tft_write(SPFD5414D_SLPOUT);
	PIO_SetOutput(CSX);
	delay(150);
#endif

}

unsigned int TFT_CHECK::read_id()
{
	tft_reset();
	if (id() ) {
		return  ID_24bits;
	} else if(dcx) {
		tft_reset();
		if (id(8)) {
			return  ID_24bits;
		}
	}
	return -1u;
}

bool TFT_CHECK::id(unsigned int bits)
{
	ID_24bits =0;
	ID_3x8bits =0;
	z_bits =0;

	PIO_ClrOutput(CSX);
	tft_write(SPFD5414D_RDDID, bits);

	PIO_SetOutput(SCL);		// dummy clock
	PIO_ClrOutput(SCL);

	ID_24bits = tft_read(24);
	PIO_SetOutput(CSX);

	delay(1);

	for (unsigned int i=0; i < 3; i++) {
		ID_3x8bits <<= 8;

		PIO_ClrOutput(CSX);
		tft_write(SPFD5414D_RDID1+i, bits);
		ID_3x8bits |= tft_read(8);
		PIO_SetOutput(CSX);
	}

	if((ID_24bits | ID_3x8bits) == 0xFFFFFF)
		return false;
	return ((ID_24bits == ID_3x8bits) );
}

FILE_RESET_OPTIONS

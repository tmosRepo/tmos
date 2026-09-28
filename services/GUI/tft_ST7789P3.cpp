/*
 * ST7789P3.cpp
 *
 *  Created on: 07.07.2025
 *      Author: Stanislav Bratkov
 */

#include <tmos.h>
#include <tft_ST7789P3.h>
#include <fam_cpp.h>


#define LCD_DEF_FONT		&FNT7x9
#define LCD_DEF_COL			PIX_WHITE

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
	ST7789P3_CASET, ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(240),
	// RowAddressSet( y0, y0+dy-1 );
	ST7789P3_RASET, ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0),
};

static const unsigned short tft_init_address[] =
{
	ST7789P3_CASET, ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(240),
	ST7789P3_RASET, ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0),
	ST7789P3_RAMWR
};

__attribute__((always_inline, optimize("Os")))
inline size_t ST7789P3::cmd_address_size() const
{
	return sizeof(tft_init_address)/sizeof(unsigned short);
}

__attribute__((always_inline, optimize("Os")))
inline void ST7789P3::pixel_by_x(unsigned int x)
{
	((unsigned short *)disp_buf)[x] = color;
}

__attribute__((always_inline, optimize("Os")))
inline void ST7789P3::set_color(unsigned int rgb)
{
	//values stored in the RGB565 format (65K color)
	//RRRRRGGG GGGBBBBB
	color = PIX_SPI4_RGB((rgb>>16),((rgb>>8)&0xFF),(rgb&0xFF));
}

__attribute__((always_inline, optimize("Os")))
inline void ST7789P3::invert_pixel_by_x(unsigned int x)
{
	((unsigned short *)disp_buf)[x] ^= PIX_SPI4_WHITE;
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

void ST7789P3::tft_reset()
{
	tft_sleep_out();
	tsk_sleep(120);
	tft_regs_init();
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

void ST7789P3::tft_write_row(unsigned short address_cmd[], unsigned short row)
{
/*          0              1 XS[15:8]        2 XS[7:0]         3 XE[15:8]        4 XE[7:0]
    ST7789P3_CASET, ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(127),
            5              6 YS[15:8]        7 YS[7:0]         8 YE[15:8]        9 YE[7:0]
	ST7789P3_RASET, ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0),
	ST7789P3_RAMWR
 */
	if (row == 0xFFFF) {
		// initialize row_address buffer
		memcpy(address_cmd, tft_init_address, sizeof(tft_init_address));
	} else {

//		address_cmd[2] = ST7789P3_DATA(0); address_cmd[4] = ST7789P3_DATA(240);
		address_cmd[6] = address_cmd[8] = ST7789P3_DATA(((row + y_offset)>>8));
		address_cmd[7] = address_cmd[9] = ST7789P3_DATA((row + y_offset) & 0xFF);
		//TODO: use_dcx_pin
/*
		unsigned int * dst = (unsigned int *)tft_buf;
		for(int x= 0; x < ST7789P3_X_SIZE/2; x++)
		{
			*dst++ = lut_to_tft_color[(disp_buf[frame.y0][x] & 0xF0)>>4];
			*dst++ = lut_to_tft_color[disp_buf[frame.y0][x] & 0x0F];
		}
*/
		tft_set_address(address_cmd);
		lcd_hnd->tsk_write(disp_buf, size_x);
		if(disp_buf == video_buf)
			disp_buf += 128;
		else
			disp_buf = video_buf;
	}
}




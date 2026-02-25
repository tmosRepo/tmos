/*
 * lcd_ST7789P3_UART.cpp
 *
 *  Created on: Mar 6, 2018
 *      Author: miro
 */

#include <tmos.h>
#include <lcd_ST7789P3_UART.h>
#include <fam_cpp.h>
#include <tmos_stdlib.h>


#ifndef USE_ST7789P3_DCX_PIN
#define USE_ST7789P3_DCX_PIN	1
#endif

#if USE_ST7789P3_DCX_PIN
#undef	MSB2LSB
// 8 bit SPI is usedp
#define MSB2LSB(x)			(							\
								(((x)&0x001)?0x080:0) | \
								(((x)&0x002)?0x040:0) | \
								(((x)&0x004)?0x020:0) | \
								(((x)&0x008)?0x010:0) | \
								(((x)&0x010)?0x008:0) | \
								(((x)&0x020)?0x004:0) | \
								(((x)&0x040)?0x002:0) | \
								(((x)&0x080)?0x001:0) | \
								(((x)&0x100)?0x100:0)   \
							)

//R4R3R2R1R0G5G4G3 G2G1G0B4B3B2B1B0
//B0B1B2B3B4G0G1G2 G3G4G5R0R1R2R3R4
#define PIX_565_MSB2LSB(x) ( (MSB2LSB((x)&0xFF) << 8) | (MSB2LSB((x)>>8)) )

static const unsigned short lut_to_tft_color_rotated[16] =
{
	PIX_565_MSB2LSB(PIX_565_BLACK),	            // 0
	PIX_565_MSB2LSB(PIX_565_BLUE),              // 1
	PIX_565_MSB2LSB(PIX_565_GREEN),             // 2
	PIX_565_MSB2LSB(PIX_565_RED),               // 3
	PIX_565_MSB2LSB(PIX_565_LIGHTMAGENTA),      // 4
	PIX_565_MSB2LSB(PIX_565_BROWN),             // 5
	PIX_565_MSB2LSB(PIX_565_LIGHTBLUE),         // 6
	PIX_565_MSB2LSB(PIX_565_LIGHTGRAY),         // 7
	PIX_565_MSB2LSB(PIX_565_DARKGRAY),          // 8
	PIX_565_MSB2LSB(PIX_565_LIGHTGREEN),        // 9
	PIX_565_MSB2LSB(PIX_565_LIGHTCYAN),         // A
	PIX_565_MSB2LSB(PIX_565_LIGHTRED),          // B
	PIX_565_MSB2LSB(PIX_565_CYAN),              // C
	PIX_565_MSB2LSB(PIX_565_MAGENTA),           // D
	PIX_565_MSB2LSB(PIX_565_YELLOW),            // E
	PIX_565_MSB2LSB(PIX_565_WHITE)              // F
};

#else
// 9 bit SPI is used
#define MSB2LSB(x)			(							\
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

static const unsigned int lut_to_tft_color_rotated[16] =
{
	PIX2LSB_RGB(PIX_RGB_BLACK),
	PIX2LSB_RGB(PIX_RGB_BLUE),
	PIX2LSB_RGB(PIX_RGB_GREEN),
	PIX2LSB_RGB(PIX_RGB_RED),
	PIX2LSB_RGB(PIX_RGB_LIGHTMAGENTA),
	PIX2LSB_RGB(PIX_RGB_BROWN),
	PIX2LSB_RGB(PIX_RGB_LIGHTBLUE),
	PIX2LSB_RGB(PIX_RGB_LIGHTGRAY),
	PIX2LSB_RGB(PIX_RGB_DARKGRAY),
	PIX2LSB_RGB(PIX_RGB_LIGHTGREEN),
	PIX2LSB_RGB(PIX_RGB_LIGHTCYAN),
	PIX2LSB_RGB(PIX_RGB_LIGHTRED),
	PIX2LSB_RGB(PIX_RGB_CYAN),
	PIX2LSB_RGB(PIX_RGB_MAGENTA),
	PIX2LSB_RGB(PIX_RGB_YELLOW),
	PIX2LSB_RGB(PIX_RGB_WHITE)
};

#endif


static const unsigned short ST7789P3_lsb_init[] =
{
	// ColorModeSet 16bpp
	MSB2LSB(ST7789P3_COLMOD),
		MSB2LSB(ST7789P3_DATA(ST7789P3_COLMOD_16BPP)),

	//_MemoryAccess BGR
	MSB2LSB(ST7789P3_MADCTR),
		MSB2LSB(ST7789P3_DATA( 0
				| ST7789P3_MADCTR_MY
				//| ST7789P3_MADCTR_MX
				//| ST7789P3_MADCTR_MV
				//| ST7789P3_MADCTR_ML
				| ST7789P3_MADCTR_BGR
				)),

	// Display on
	MSB2LSB(ST7789P3_NORON),
	MSB2LSB(ST7789P3_DISPON),

	// Write Display Brightness
	MSB2LSB(ST7789P3_WRDISBV),
		MSB2LSB(ST7789P3_DATA(0xFF)),

	// ST7789P3_ColumnAddressSet
	MSB2LSB(ST7789P3_CASET),
		MSB2LSB(ST7789P3_DATA(0)),MSB2LSB(ST7789P3_DATA(0)),
		MSB2LSB(ST7789P3_DATA(0)),MSB2LSB(ST7789P3_DATA(239)),

	// ST7789P3_RowAddressSet
	MSB2LSB(ST7789P3_RASET),
		MSB2LSB(ST7789P3_DATA(0)),MSB2LSB(ST7789P3_DATA(0)),
		MSB2LSB(ST7789P3_DATA(0)),MSB2LSB(ST7789P3_DATA(239))
};


static const unsigned short ST7789P3_lsb_row_address[] =
{
	MSB2LSB(ST7789P3_CASET),
		MSB2LSB(ST7789P3_DATA(0)),MSB2LSB(ST7789P3_DATA(0)),
		MSB2LSB(ST7789P3_DATA(0)),MSB2LSB(ST7789P3_DATA(0)),
	MSB2LSB(ST7789P3_RASET),
		MSB2LSB(ST7789P3_DATA(0)),MSB2LSB(ST7789P3_DATA(0)),
		MSB2LSB(ST7789P3_DATA(0)),MSB2LSB(ST7789P3_DATA(0)),
	MSB2LSB(ST7789P3_RAMWR)
};


static inline __attribute__ ((always_inline)) uint16_t rotate(uint16_t value)
{
	asm volatile (
    "       rbit            %0, %0			 \n\t"
#if USE_ST7789P3_DCX_PIN
    "       lsr           	%0, %0, #24		 \n\t"
    "       orr.w         	%0, %0, #256	 \n\t"
#else
    "       lsr           	%0, %0, #23		 \n\t"
#endif
		  : "+r" (value)
	);
	return value;
}

FILE_OPTIMIZE("Os")


void ST7789P3_UART::tft_init_address_cmd(unsigned short address_cmd[])
{
	memcpy(address_cmd, ST7789P3_lsb_row_address, sizeof(ST7789P3_lsb_row_address));
}

void ST7789P3_UART::tft_set_address(unsigned short address_cmd[])
{
	if (pins[DCX_PIN_INDX]) {
		const char* data = (const char*)address_cmd;
		unsigned indx=0;
		for (; indx < sizeof(ST7789P3_lsb_row_address)/2; indx++, data+=2) {
			if(!data[1]) {
				// Commands
				PIO_Assert(pins[DCX_PIN_INDX]);
				lcd_hnd->tsk_write_locked(data, 1);
				PIO_Deassert(pins[DCX_PIN_INDX]);
			} else {
				lcd_hnd->tsk_write_locked(data, 1);
			}
		}
	} else {
		lcd_hnd->tsk_write_locked(ST7789P3_lsb_row_address, sizeof(ST7789P3_lsb_row_address)/2);
	}
}

void ST7789P3_UART::tft_write_row(unsigned short address_cmd[])
{
/*
		0					1					2				3					4
	ST7789P3_CASET, ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0), 	//0 to 239
		5					6					7				8					9
	ST7789P3_RASET, ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0),		//0 to 319
	ST7789P3_RAMWR
 */
	address_cmd[2] = rotate(ST7789P3_DATA(frame.x0)); address_cmd[4] = rotate(ST7789P3_DATA(frame.x1));
	address_cmd[7] = address_cmd[9] = rotate(ST7789P3_DATA((frame.y0+y_offset) &0xFF));
	address_cmd[6] = address_cmd[8] = rotate(ST7789P3_DATA(((frame.y0+y_offset) >> 8) &0xFF));

	if (uart_lock) {
		uart_lock->lock();
	}
	PIO_Assert(pins[CSX_PIN_INDX]);

	tft_set_address(address_cmd);
#if USE_ST7789P3_DCX_PIN
	unsigned short int * dst = (unsigned short int *)tft_buf;
	unsigned char* src = &disp_buf[frame.y0][frame.x0 >> 1];
	for(int x= frame.x0; x <= frame.x1; x++)
	{
		if(x & 1)
		{
			*dst++ = lut_to_tft_color_rotated[*src & 0x0F];
			src++;
		}
		else
			*dst++ = lut_to_tft_color_rotated[(*src & 0xF0)>>4];
	}
	lcd_hnd->tsk_write_locked(((unsigned short int *)tft_buf), (frame.x1 - frame.x0 +1)*sizeof(short));
#else
	unsigned int * dst = (unsigned int *)tft_buf;
	for(int x= 0; x < ST7789P3_X_SIZE/2; x++)
	{
		*dst++ = lut_to_tft_color_rotated[(disp_buf[frame.y0][x] & 0xF0)>>4];
		*dst++ = lut_to_tft_color_rotated[disp_buf[frame.y0][x] & 0x0F];
	}
	lcd_hnd->tsk_write_locked(((unsigned int *)tft_buf)+frame.x0, (frame.x1 - frame.x0 +1));
#endif
	PIO_Deassert(pins[CSX_PIN_INDX]);
	if (uart_lock) {
		uart_lock->unlock();
	}

}

void ST7789P3_UART::redraw_screen(GObject* object, RECT_T area)
{
	if(area.normalize(rect))
	{
		unsigned short cmd_address[sizeof(ST7789P3_lsb_row_address)/sizeof(unsigned short)];
		frame = area;
#if GUI_DEBUG
		uint32_t t = CURRENT_TIME;
		GUI_TRACELN("LCD%u update {%u,%u %u,%u}", displays, frame.x0, area.y0, frame.x1, area.y1);
#endif
		tft_init_address_cmd(cmd_address);
		/*
				0					1					2				3					4
			ST7789P3_CASET, ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0), 	//0 to 239
				5					6					7				8					9
			ST7789P3_RASET, ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0),		//0 to 319
			ST7789P3_RAMWR
		 */
		cmd_address[2] = rotate(ST7789P3_DATA(frame.x0)); cmd_address[4] = rotate(ST7789P3_DATA(frame.x1));

		cmd_address[6] = rotate(ST7789P3_DATA(((frame.y0+y_offset) >> 8) &0xFF));
		cmd_address[7] = rotate(ST7789P3_DATA((frame.y0+y_offset) &0xFF));

		cmd_address[8] = rotate(ST7789P3_DATA(((frame.y1+y_offset) >> 8) &0xFF));
		cmd_address[9] = rotate(ST7789P3_DATA((frame.y1+y_offset) &0xFF));

		if (uart_lock) {
			uart_lock->lock();
		}
		PIO_Assert(pins[CSX_PIN_INDX]);

		tft_set_address(cmd_address);
		for (frame.y0=area.y0; frame.y0 <= area.y1; frame.y0++)
		{
#if USE_ST7789P3_DCX_PIN
			unsigned short int * dst = (unsigned short int *)tft_buf;
			unsigned char* src = &disp_buf[frame.y0][frame.x0 >> 1];
			for(int x= frame.x0; x <= frame.x1; x++)
			{
				if(x & 1)
				{
					*dst++ = lut_to_tft_color_rotated[*src & 0x0F];
					src++;
				}
				else
					*dst++ = lut_to_tft_color_rotated[(*src & 0xF0)>>4];
			}
			lcd_hnd->tsk_write_locked(((unsigned short int *)tft_buf), (frame.x1 - frame.x0 +1)*sizeof(short));
#else
			unsigned int * dst = (unsigned int *)tft_buf;
			for(int x= 0; x < ST7789P3_X_SIZE/2; x++)
			{
				*dst++ = lut_to_tft_color_rotated[(disp_buf[frame.y0][x] & 0xF0)>>4];
				*dst++ = lut_to_tft_color_rotated[disp_buf[frame.y0][x] & 0x0F];
			}
			lcd_hnd->tsk_write_locked(((unsigned int *)tft_buf)+frame.x0, (frame.x1 - frame.x0 +1));
#endif

		}
#if GUI_DEBUG
	    GUI_TRACE(" %u ms", ms_since(t));
#endif
		PIO_Deassert(pins[CSX_PIN_INDX]);
		if (uart_lock) {
			uart_lock->unlock();
		}
	}
}

void ST7789P3_UART::tft_sleep_out()
{
	// sleep out
	if (uart_lock) {
		uart_lock->lock();
	}
	PIO_Assert(pins[CSX_PIN_INDX]);
	const short	cmd = 	MSB2LSB(ST7789P3_SLPOUT);
	if (pins[DCX_PIN_INDX]) {
		PIO_Assert(pins[DCX_PIN_INDX]);		// Low Command
		lcd_hnd->tsk_write_locked(&cmd, 1);
		PIO_Deassert(pins[DCX_PIN_INDX]); 	// High Data
	} else {
		lcd_hnd->tsk_write_locked(&cmd, 1);
	}
	PIO_Deassert(pins[CSX_PIN_INDX]);
	if (uart_lock) {
		uart_lock->unlock();
	}
}

void ST7789P3_UART::tft_regs_init()
{
	if (uart_lock) {
		uart_lock->lock();
	}
	PIO_Assert(pins[CSX_PIN_INDX]);

	if (pins[DCX_PIN_INDX]) {
		// 8 - BIT synchronous UART
		const char* data = (const char*)ST7789P3_lsb_init;
		unsigned indx=0;
		for (indx=0, data =(const char*)ST7789P3_lsb_init; indx < sizeof(ST7789P3_lsb_init)/sizeof(short); indx++, data+=2) {
			if(!data[1]) {
				// Command
				PIO_Assert(pins[DCX_PIN_INDX]);	// Low Command
				lcd_hnd->tsk_write_locked(data, 1);
				PIO_Deassert(pins[DCX_PIN_INDX]); // High Data
			} else {
				// Data
				lcd_hnd->tsk_write_locked(data, 1);
			}
		}
	} else {
		// 9 - BIT synchronous UART
		lcd_hnd->tsk_write_locked(ST7789P3_lsb_init, sizeof(ST7789P3_lsb_init)/sizeof(short));
	}
	PIO_Deassert(pins[CSX_PIN_INDX]);
	if (uart_lock) {
		uart_lock->unlock();
	}

}

FILE_RESET_OPTIONS


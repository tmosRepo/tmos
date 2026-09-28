/*
 * tft_ST7789P3_UART.cpp
 *
 *  Created on: Jul 27, 2026
 *      Author: bratkov
 */

#include <tmos.h>
#include <tft_ST7789P3_UART.h>
#include <fam_cpp.h>

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

__attribute__((always_inline, optimize("Os")))
inline size_t ST7789P3_UART::cmd_address_size() const
{
	return sizeof(ST7789P3_lsb_row_address)/sizeof(unsigned short);
}

__attribute__((always_inline, optimize("Os")))
inline void ST7789P3_UART::pixel_by_x(unsigned int x)
{
	((unsigned short *)disp_buf)[x] = color;
}

__attribute__((always_inline, optimize("Os")))
inline void ST7789P3_UART::set_color(unsigned int rgb)
{
	//values stored in the RGB565 format (65K color)
	//RRRRRGGG GGGBBBBB
/*
	if(rgb)
		color = 0xFFFF;
	else
		color = 0;
*/
	color = PIX_565_MSB2LSB(PIX_SPI4_RGB((rgb>>16),((rgb>>8)&0xFF),(rgb&0xFF)));
}

__attribute__((always_inline, optimize("Os")))
inline void ST7789P3_UART::invert_pixel_by_x(unsigned int x)
{
	((unsigned short *)disp_buf)[x] ^= PIX_SPI4_WHITE;
}

//=============================================================================

//extern int test_dma;
//int dma_on=1;

void ST7789P3_UART::tft_sleep_out()
{
	// sleep out
	if (uart_lock) {
		uart_lock->lock();
	}
	// test_dma = 1;
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
	// test_dma = 0;
	if (uart_lock) {
		uart_lock->unlock();
	}
}

void ST7789P3_UART::tft_regs_init()
{
	if (uart_lock) {
		uart_lock->lock();
	}
	// // test_dma = 1;
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
	// test_dma = 0;
	if (uart_lock) {
		uart_lock->unlock();
	}
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
		lcd_hnd->tsk_write_locked(address_cmd, sizeof(ST7789P3_lsb_row_address)/2);
	}
}

void ST7789P3_UART::tft_write_row(unsigned short address_cmd[], unsigned short row)
{
	if (row == 0xFFFF) {
		// initialize row_address buffer
		memcpy(address_cmd, ST7789P3_lsb_row_address, sizeof(ST7789P3_lsb_row_address));
		address_cmd[2] = rotate(ST7789P3_DATA(0)); address_cmd[4] = rotate(ST7789P3_DATA(239));
	} else {

		/*
				0					1					2				3					4
			ST7789P3_CASET, ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0), 	//0 to 239
				5					6					7				8					9
			ST7789P3_RASET, ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0), ST7789P3_DATA(0),		//0 to 319
			ST7789P3_RAMWR
		 */
		address_cmd[7] = address_cmd[9] = rotate(ST7789P3_DATA((row+y_offset) &0xFF));
		address_cmd[6] = address_cmd[8] = rotate(ST7789P3_DATA(((row+y_offset) >> 8) &0xFF));

		if (uart_lock) {
			uart_lock->lock();
		}
		PIO_Assert(pins[CSX_PIN_INDX]);
		// test_dma = 1;
		tft_set_address(address_cmd);
		// test_dma = dma_on;
		lcd_hnd->tsk_write_locked(disp_buf, size_x*sizeof(uint16_t));
		// test_dma = 1;
		PIO_Deassert(pins[CSX_PIN_INDX]);
		if (uart_lock) {
			uart_lock->unlock();
		}

		if(disp_buf == video_buf)
			disp_buf += 128;
		else
			disp_buf = video_buf;
	}
}


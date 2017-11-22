/*
 * lcd_ST7735R_UART.cpp
 *
 *  Created on: 25.09.2017
 *      Author: Stanly
 */

#include <tmos.h>
#include <tft_ST7735R_UART.h>
#include <fam_cpp.h>




const unsigned short ST7735R_UART::ST7735R_lsb_init[] =
{
	MSB2LSB(ST7735R_SLPOUT),

	//------------------------------------ST7735R Frame Rate-----------------------------------------//
	MSB2LSB(ST7735R_FRMCTR1),
		MSB2LSB(ST7735R_DATA(0x01)),
		MSB2LSB(ST7735R_DATA(0x2C)),
		MSB2LSB(ST7735R_DATA(0x2D)),
	MSB2LSB(ST7735R_FRMCTR2),
		MSB2LSB(ST7735R_DATA(0x01)),
		MSB2LSB(ST7735R_DATA(0x2C)),
		MSB2LSB(ST7735R_DATA(0x2D)),
	MSB2LSB(ST7735R_FRMCTR3),
		MSB2LSB(ST7735R_DATA(0x01)),
		MSB2LSB(ST7735R_DATA(0x2C)),
		MSB2LSB(ST7735R_DATA(0x2D)),
		MSB2LSB(ST7735R_DATA(0x01)),
		MSB2LSB(ST7735R_DATA(0x2C)),
		MSB2LSB(ST7735R_DATA(0x2D)),
	//------------------------------------End ST7735R Frame Rate-----------------------------------------//
	MSB2LSB(ST7735R_INVCTR),
		MSB2LSB(ST7735R_DATA(0x04)), //Column inversion
	//------------------------------------ST7735R Power Sequence-----------------------------------------//
	MSB2LSB(ST7735R_DISSET5),
		MSB2LSB(ST7735R_DATA(0xB4)),
		MSB2LSB(ST7735R_DATA(0xF0)), //Extend gate non-overlap time
	MSB2LSB(ST7735R_PWCTR1),
		MSB2LSB(ST7735R_DATA(0xA2)),
		MSB2LSB(ST7735R_DATA(0x02)),
		MSB2LSB(ST7735R_DATA(0x84)),
	MSB2LSB(ST7735R_PWCTR2),
		MSB2LSB(ST7735R_DATA(0xC5)),
	MSB2LSB(ST7735R_PWCTR3),
		MSB2LSB(ST7735R_DATA(0x0A)),
		MSB2LSB(ST7735R_DATA(0x00)),
	MSB2LSB(ST7735R_PWCTR4),
		MSB2LSB(ST7735R_DATA(0x8A)),
		MSB2LSB(ST7735R_DATA(0x2A)),
	MSB2LSB(ST7735R_PWCTR5),
		MSB2LSB(ST7735R_DATA(0x8A)),
		MSB2LSB(ST7735R_DATA(0xEE)),
	//---------------------------------End ST7735R Power Sequence-------------------------------------//
	MSB2LSB(ST7735R_VMCTR1),
		MSB2LSB(ST7735R_DATA(0x0E)), //VCOM
	//_MemoryAccess BGR
	MSB2LSB(ST7735R_MADCTR),
		MSB2LSB(ST7735R_DATA(/*ST7735R_MADCTR_MY | ST7735R_MADCTR_MX | ST7735R_MADCTR_ML |*/ ST7735R_MADCTR_BGR)),
	//------------------------------------ST7735R Gamma Sequence-----------------------------------------//
	MSB2LSB(ST7735R_GAMCTRP1),
		MSB2LSB(ST7735R_DATA(0x0f)),
		MSB2LSB(ST7735R_DATA(0x1a)),
		MSB2LSB(ST7735R_DATA(0x0f)),
		MSB2LSB(ST7735R_DATA(0x18)),
		MSB2LSB(ST7735R_DATA(0x2f)),
		MSB2LSB(ST7735R_DATA(0x28)),
		MSB2LSB(ST7735R_DATA(0x20)),
		MSB2LSB(ST7735R_DATA(0x22)),
		MSB2LSB(ST7735R_DATA(0x1f)),
		MSB2LSB(ST7735R_DATA(0x1b)),
		MSB2LSB(ST7735R_DATA(0x23)),
		MSB2LSB(ST7735R_DATA(0x37)),
		MSB2LSB(ST7735R_DATA(0x00)),
		MSB2LSB(ST7735R_DATA(0x07)),
		MSB2LSB(ST7735R_DATA(0x02)),
		MSB2LSB(ST7735R_DATA(0x10)),
	MSB2LSB(ST7735R_GAMCTRN1),
		MSB2LSB(ST7735R_DATA(0x0f)),
		MSB2LSB(ST7735R_DATA(0x1b)),
		MSB2LSB(ST7735R_DATA(0x0f)),
		MSB2LSB(ST7735R_DATA(0x17)),
		MSB2LSB(ST7735R_DATA(0x33)),
		MSB2LSB(ST7735R_DATA(0x2c)),
		MSB2LSB(ST7735R_DATA(0x29)),
		MSB2LSB(ST7735R_DATA(0x2e)),
		MSB2LSB(ST7735R_DATA(0x30)),
		MSB2LSB(ST7735R_DATA(0x30)),
		MSB2LSB(ST7735R_DATA(0x39)),
		MSB2LSB(ST7735R_DATA(0x3f)),
		MSB2LSB(ST7735R_DATA(0x00)),
		MSB2LSB(ST7735R_DATA(0x07)),
		MSB2LSB(ST7735R_DATA(0x03)),
		MSB2LSB(ST7735R_DATA(0x10)),
	//------------------------------------End ST7735R Gamma Sequence-----------------------------------------//
	MSB2LSB(ST7735R_EXTCTRL), //Enable test command
		MSB2LSB(ST7735R_DATA(0x01)),
	MSB2LSB(0xF6), //Disable ram power save mode
		MSB2LSB(ST7735R_DATA(0x00)),

	// ColorModeSet 16bpp
	MSB2LSB(ST7735R_COLMOD),
		MSB2LSB(ST7735R_DATA(ST7735R_COLMOD_16BPP )),


	//  Display on
	MSB2LSB(ST7735R_NORON),
	MSB2LSB(ST7735R_DISPON),


//	  TFT_ColumnAddressSet( x0, x0+dx-1 );
	MSB2LSB(ST7735R_CASET),
		MSB2LSB(ST7735R_DATA(0)), MSB2LSB(ST7735R_DATA(0)),
		MSB2LSB(ST7735R_DATA(0)), MSB2LSB(ST7735R_DATA(127)),

//	  TFT_RowAddressSet( y0, y0+dy-1 );
	MSB2LSB(ST7735R_RASET),
		MSB2LSB(ST7735R_DATA(0)), MSB2LSB(ST7735R_DATA(0)),
		MSB2LSB(ST7735R_DATA(0)), MSB2LSB(ST7735R_DATA(127))
};

const unsigned short ST7735R_UART::ST7735R_lsb_init_size = sizeof(ST7735R_UART::ST7735R_lsb_init);

static const unsigned short ST7735R_lsb_row_address[] =
{
	MSB2LSB(ST7735R_RASET),
		MSB2LSB(ST7735R_DATA(0)),MSB2LSB(ST7735R_DATA(0)),
		MSB2LSB(ST7735R_DATA(0)),MSB2LSB(ST7735R_DATA(0)),
	MSB2LSB(ST7735R_RAMWR)
};

void ST7735R_UART::tft_reset()
{
	tsk_sleep(100);
	lcd_hnd->tsk_write(ST7735R_lsb_init, 1);
	tsk_sleep(120);
	lcd_hnd->tsk_write(ST7735R_lsb_init+1, sizeof(ST7735R_lsb_init)/2 -1);
}


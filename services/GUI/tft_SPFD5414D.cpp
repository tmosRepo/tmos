/*
 * SPFD5414D.cpp
 *
 *  Created on: 23.01.2011
 *      Author: Miroslav Kostadinow
 */

#include <tmos.h>
#include <tft_SPFD5414D.h>
#include <fam_cpp.h>

static const unsigned short spdf5414d_init[] =
{
	SPFD5414D_SLPOUT,

	// ColorModeSet 16bpp
	SPFD5414D_COLMOD,
		SPFD5414D_DATA(SPFD5414D_COLMOD_16BPP ),

	//_MemoryAccess BGR
	SPFD5414D_MADCTR,
		SPFD5414D_DATA(
				SPFD5414D_MADCTR_MY | SPFD5414D_MADCTR_MX |
				SPFD5414D_MADCTR_ML | SPFD5414D_MADCTR_BGR),

	//  Display on
	SPFD5414D_NORON,
	SPFD5414D_DISPON,


	SPFD5414D_CASET,
		SPFD5414D_DATA(0), SPFD5414D_DATA(0),
		SPFD5414D_DATA(0), SPFD5414D_DATA(127),

	SPFD5414D_RASET,
		SPFD5414D_DATA(0), SPFD5414D_DATA(0),
		SPFD5414D_DATA(0), SPFD5414D_DATA(127)
};


static const unsigned short spdf5414d_row_address[] =
{
	SPFD5414D_RASET,
		SPFD5414D_DATA(0), SPFD5414D_DATA(0),
		SPFD5414D_DATA(0), SPFD5414D_DATA(0),
	SPFD5414D_RAMWR
};

__attribute__((always_inline, optimize("Os")))
inline size_t SPFD5414D::cmd_address_size() const
{
	return sizeof(spdf5414d_row_address)/sizeof(unsigned short);
}

__attribute__((always_inline, optimize("Os")))
inline void SPFD5414D::pixel_by_x(unsigned int x)
{
	disp_buf[x] = color;
}

__attribute__((always_inline, optimize("Os")))
inline void SPFD5414D::set_color(unsigned int rgb)
{
	//pixel format
	// 00000001 GGGBBBBB 00000001 RRRRRGGG
	color = PIX_SPI3_RGB((rgb>>16),((rgb>>8)&0xFF),(rgb&0xFF));
}

__attribute__((always_inline, optimize("Os")))
inline void SPFD5414D::invert_pixel_by_x(unsigned int x)
{
	disp_buf[x] ^= (PIX_SPI3_WHITE ^ PIX_SPI3);
}

void SPFD5414D::draw_bitmap(unsigned int x0, unsigned int y0,
		const unsigned char* src, unsigned int width, unsigned int rows)
{
	unsigned int offset=0;
	if(y0 < frame_y0)
	{
		offset = frame_y0 - y0;
		if(offset >= rows)
			return;
		y0 += offset;
		offset *= width;
		src += offset /8;
		offset %= 8;
	}
	if(y0 < frame_y1)
	{
		offset = 1<< offset;
		while(width--)
		{
			if(src[0] & offset)
				pixel_by_x(x0);//disp_buf[x0] = color;
			offset <<= 1;
			if(offset > 255)
			{
				offset = 1;
				src++;
			}
			x0++;
		}
	}
}


void SPFD5414D::draw_hline(unsigned int x0, unsigned int x1, unsigned int y)
{
	if(y==frame_y0)
	{
		while(x0 <= x1)
		{
			pixel_by_x(x0++);//disp_buf[x0++] = color;
		}
	}
}

void SPFD5414D::draw_bline(unsigned int x0, unsigned int x1, unsigned int y)
{
	unsigned int bkp_color;
	if(y==frame_y0)
	{
		bkp_color = color;
		set_color(PIX_BLACK);
		while(x0 <= x1)
		{
			pixel_by_x(x0++); //disp_buf[x0++] = PIX_BLACK;
		}
		color = bkp_color;
	}
}

void SPFD5414D::draw_vline(unsigned int y0, unsigned int y1, unsigned int x)
{
	if((y0 <= frame_y0) && (y1 >= frame_y0))
	{
		pixel_by_x(x); //disp_buf[x] = color;
	}
}

void SPFD5414D::invert_vline(unsigned int y0, unsigned int y1, unsigned int x)
{
	if((y0 <= frame_y0) && (y1 >= frame_y0))
	{
		invert_pixel_by_x(x); //disp_buf[x] ^= (PIX_WHITE ^ PIX);
	}
}

void SPFD5414D::invert_hline(unsigned int x0, unsigned int x1, unsigned int y)
{
	if(y==frame_y0)
	{
		while(x0 <= x1)
		{
			invert_pixel_by_x(x0++); //disp_buf[x0++] ^= (PIX_WHITE ^ PIX);
		}
	}
}

void SPFD5414D::clear_screen()
{
	unsigned int bkp_color = color;
	set_color(PIX_BLACK);
	for(int i=0; i<size_y; i++)
		pixel_by_x(i); //disp_buf[i] = PIX_BLACK;
	color = bkp_color;
}

extern unsigned int cpu_usage;
#if USE_MEMORY_TRACKING
extern char end;
#endif

// InitLcd is called to initialize the hardware
void SPFD5414D::lcd_init(GUI_CB splash)
{
	LCD_MODULE::lcd_init(splash);

    //Splash screen..
    if(splash)
    {
    	lcd_single_window(splash);
    }
}

//
//The TFT modules hardware interface methods
//

void SPFD5414D::tft_write_row(unsigned short row_address_cmd[], unsigned short row)
{
	if(row == 0xFFFF) // initialize row_address buffer
	{
		memcpy(row_address_cmd, spdf5414d_row_address, sizeof(spdf5414d_row_address));
	}
	else
	{
		row_address_cmd[2] = row_address_cmd[4] = SPFD5414D_DATA(row);
		lcd_hnd->tsk_write(row_address_cmd, sizeof(spdf5414d_row_address)/2);
		lcd_hnd->tsk_write(disp_buf, 256);
		if(disp_buf == video_buf)
			disp_buf += 128;
		else
			disp_buf = video_buf;
	}
}

void SPFD5414D::tft_reset()
{
	lcd_hnd->tsk_write(spdf5414d_init, sizeof(spdf5414d_init)/2);
}

void SPFD5414D::lcd_reset()
{
	tft_reset();
}

void SPFD5414D::redraw_screen(WINDOW desktop)
{
    WINDOW win;
    WINDOW top;
	bool update=false;

	unsigned short row=0;
	unsigned short cmd_address [cmd_address_size()];//[sizeof(spdf5414d_row_address)/sizeof(unsigned short)];
	tft_write_row(cmd_address, -1);


	if( ms_since(reset_timeout) > 500 )
	{
		reset_timeout = CURRENT_TIME;
		lcd_reset();
	}

	disp_buf = video_buf;
    for(frame_y0=0; frame_y0<size_y-1; frame_y0++)
    {
    	frame_y1 = frame_y0+1;
    	clear_screen();
    	win = desktop;
    	while(win)
    	{
        	update = true;
    		top = (WINDOW)win->next;
			#if GUI_DISPLAYS > 1
			while(top && !(display & top->displays))
				top = (WINDOW) top->next;

			if(display & desktop->displays)
			#endif
			{
				if( (!top) || (win->rect.as_int != top->rect.as_int) )
				{
					//TODO: set_font(&FNT7x9);
					set_font(default_font());
					set_color(PIX_WHITE);
					if (win->callback((unsigned int) this, WM_DRAW))
					{
						reset_timeout = CURRENT_TIME;
						update = false;
					}
				}
			}

    		win = top;
    	}
    	if(update)
    	{
    		tft_write_row(cmd_address, row);
    	}
    	row++;
    }
	clear_screen();
#if GUI_DISPLAYS > 1
	if (display == 2)
#endif
	{
		set_color(PIX_LIGHTGRAY);
		draw_hline(0, (cpu_usage*(size_x-1))/100, size_x-1);
#if USE_MEMORY_TRACKING and 0
		unsigned int mem_use;
		mem_use = (BASE_SRAM + RAM_SIZE - (unsigned int)&_sram_end)>>2;
		mem_use = (PMAIN_TASK->aloc_mem * (size_x-1))/mem_use;
		invert_hline(size_x-1-mem_use, size_x-1, size_x-1);
#endif
	}
	tft_write_row(cmd_address, row);
}


/*
void TFT_CHECK::delay(unsigned int time)
{
	if(time)
	{
		tsk_sleep(time);
	}
}

void TFT_CHECK::tft_write( unsigned int value)
{
	//SET SDA to Port
	PIO_CfgOutput0(pins[SDA_PIN_INDX]);

	//9 data bits
	for (int i = 0; i < 9; i++)
	{
		PIO_ClrOutput(pins[SCL_PIN_INDX]);
		delay();

		//    TFT_SDA = nByte & Mask8[i] ? 1 : 0;//51 msec
		if(value & 0x100)
		{
			PIO_SetOutput(pins[SDA_PIN_INDX]);
		} else
		{
			PIO_ClrOutput(pins[SDA_PIN_INDX]);
		}
		value <<= 1;
		delay();

		PIO_SetOutput(pins[SCL_PIN_INDX]);
		delay();
	}
	PIO_ClrOutput(pins[SCL_PIN_INDX]);
	delay();
	PIO_SetOutput(pins[SCL_PIN_INDX]);
	delay();
	PIO_ClrOutput(pins[SCL_PIN_INDX]);
	delay();
}

unsigned int TFT_CHECK::tft_read( )
{
	unsigned int res = 0;

	PIO_CfgInput(pins[SDA_PIN_INDX]);

	//8 data bits
	for (int i = 0; i < 8; i++)
	{
		PIO_ClrOutput(pins[SCL_PIN_INDX]);
		delay();


		res <<= 1;
		if(PIO_Read(pins[SDA_PIN_INDX]))
			res ++;

		PIO_SetOutput(pins[SCL_PIN_INDX]);
		delay();
	}
	PIO_ClrOutput(pins[SCL_PIN_INDX]);
	delay();
	return (res);
}

unsigned int TFT_CHECK::read_id()
{
    PIO_CfgOutput1(pins[RST_PIN_INDX]);

    PIO_ClrOutput(pins[RST_PIN_INDX]);
	delay(5);
	PIO_SetOutput(pins[RST_PIN_INDX]);
	delay(150);


    PIO_CfgOutput0(pins[RST_PIN_INDX]);
	delay(5);
	PIO_SetOutput(pins[RST_PIN_INDX]);
	delay(150);

	PIO_CfgOutput0(pins[CSX_PIN_INDX]);
	delay(5);
	tft_write(SPFD5414D_SWRESET);
	delay(5);
	PIO_SetOutput(pins[CSX_PIN_INDX]);
	delay(150);

    PIO_CfgOutput0(pins[SCL_PIN_INDX]);

	PIO_CfgOutput0(pins[CSX_PIN_INDX]);
	tft_write(SPFD5414D_SLPOUT);
	PIO_SetOutput(pins[CSX_PIN_INDX]);
	delay(150);

	return id();
}

unsigned int TFT_CHECK::id()
{
	unsigned int res =0;

	PIO_CfgOutput0(pins[SCL_PIN_INDX]);
	PIO_CfgOutput0(pins[CSX_PIN_INDX]);
	tft_write(SPFD5414D_RDDID);
	res = tft_read();	//2A
	res <<= 8;
	res |= tft_read(); //2A40
	res <<= 8;
	res |= tft_read(); //2A4033
	PIO_SetOutput(pins[CSX_PIN_INDX]);
	return res;
}
*/

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
FILE_OPTIMIZE("O0")

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

unsigned int TFT_CHECK::tft_fast_read( unsigned int bits )
{
	unsigned int res = 0;
	PIN_DESC SDI = SDA & ~(PD_PULL_DOWN|PD_PULL_UP);

	PIO_CfgInput(SDI|PD_PULL_UP);
	// data bits

	for (unsigned int i = 0; i < bits; i++)
	{
		PIO_ClrOutput(SCL);

		res <<= 1;
		if (PIO_Read(SDA)) {
			res ++;
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
unsigned int TFT_CHECK::fast_read_id(unsigned int bits)
{
    PIO_CfgOutput0(SCL);
    PIO_CfgOutput1(CSX);
	PIO_ClrOutput(CSX);
	tft_write(SPFD5414D_RDDID, bits);
	PIO_SetOutput(SCL);		// dummy clock
	PIO_ClrOutput(SCL);
	ID_24bits = tft_fast_read(24);

	PIO_SetOutput(SCL);		// dummy clock
	PIO_SetOutput(CSX);
	return ID_24bits;
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



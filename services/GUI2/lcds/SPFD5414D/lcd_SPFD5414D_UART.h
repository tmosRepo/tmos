/*
 * lcd_SPFD5414D_UART.h
 *
 *  Created on: Mar 6, 2018
 *      Author: miro
 */

#ifndef SERVICES_GUI2_LCD_SPFD5414D_UART_H_
#define SERVICES_GUI2_LCD_SPFD5414D_UART_H_

#include <lcd_SPFD5414D.h>


struct SPFD5414D_UART: public SPFD5414D
{
protected:
	lock_t* uart_lock;
public:
	SPFD5414D_UART(unsigned int x, unsigned int y,
			unsigned int dx, unsigned int dy,
			HANDLE hnd, const PIN_DESC* p, const RENDER_MODE* _font = GUI_LCD_FONT,
			lock_t* _lock = nullptr) :
		SPFD5414D(x, y, dx, dy, hnd, p, _font)
		, uart_lock(_lock)
	{ ; }

	void lcd_reset() override;

protected:
	void tft_write_row(unsigned short address_cmd[]) override;
	void tft_init_address_cmd(unsigned short address_cmd[]) override;
};


#endif /* SERVICES_GUI2_LCD_SPFD5414D_UART_H_ */

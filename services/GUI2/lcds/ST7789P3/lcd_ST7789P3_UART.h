/*
 * lcd_ST7789P3_UART.h
 *
 *  Created on: Mar 6, 2018
 *      Author: miro
 */

#ifndef SERVICES_GUI2_LCD_ST7789P3_UART_H_
#define SERVICES_GUI2_LCD_ST7789P3_UART_H_

#include <lcd_ST7789P3.h>

struct ST7789P3_UART: public ST7789P3
{
protected:
	lock_t* uart_lock;
public:
	ST7789P3_UART(unsigned int x, unsigned int y,
			unsigned int dx, unsigned int dy,
			HANDLE hnd, const PIN_DESC* p,
			const RENDER_MODE* _font = GUI_LCD_FONT,
			lock_t* _lock = nullptr,
			const int x_offset_=0, const int y_offset_=80) :
				ST7789P3(x, y, dx, dy, hnd, p, _font, x_offset_, y_offset_)
				, uart_lock(_lock)
	{;}

	void redraw_screen(GObject* object, RECT_T area) override;

protected:
    void tft_sleep_out() override;
    void tft_regs_init() override;

	void tft_write_row(unsigned short address_cmd[]) override;
	void tft_init_address_cmd(unsigned short address_cmd[]) override;
	void tft_set_address(unsigned short address_cmd[]) override;
};


#endif /* SERVICES_GUI2_LCD_ST7789P3_UART_H_ */

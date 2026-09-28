/*
 * tft_ST7789P3_UART.h
 *
 *  Created on: Jul 27, 2026
 *      Author: bratkov
 */

#ifndef SERVICES_GUI_TFT_ST7789P3_UART_H_
#define SERVICES_GUI_TFT_ST7789P3_UART_H_

#include <tft_ST7789P3.h>

struct ST7789P3_UART: public ST7789P3
{
protected:
	lock_t* uart_lock;
public:
	ST7789P3_UART(	const unsigned int x, const unsigned int y,
				HANDLE hnd, const PIN_DESC* p, lock_t* _lock = nullptr,
				const int x_offset_=0, const int y_offset_=80)
		:ST7789P3(x, y, hnd, p, x_offset_, y_offset_)
		, uart_lock(_lock)
	{ ;	}

	//virtual functions
	void pixel_by_x(unsigned int x) override;
	void invert_pixel_by_x(unsigned int x) override;
	void set_color(unsigned int rgb) override;
protected:
	size_t cmd_address_size() const override;
    void tft_sleep_out() override;
    void tft_regs_init() override;
    void tft_set_address(unsigned short address_cmd[]) override;
    void tft_write_row(unsigned short address_cmd[], unsigned short row) override;
};



#endif /* SERVICES_GUI_TFT_ST7789P3_UART_H_ */

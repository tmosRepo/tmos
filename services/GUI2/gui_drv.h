#ifndef	GUI_DRV_H
#define	GUI_DRV_H


struct LCD_MODULE;

#define SIG_GUI_TASK_CANCEL		2


struct GUI_DRIVER_DATA
{
    HANDLE			waiting;
    HANDLE			helper;
};

struct GUI_DRIVER_INFO
{
	DRIVER_INFO_Type 	info;
	GUI_DRIVER_DATA * 	drv_data;
	/**
	 * Array of LCD modules.
	 * lcd[0] is the interface display (the one that receives keybord). So it
	 * must exists and installed last.
	 *
	 */
	LCD_MODULE*			lcd[GUI_DISPLAYS];
};

void GUI_DCR(GUI_DRIVER_INFO* drv_info, unsigned int reason, HANDLE param) FAST_FLASH;
void GUI_DSR(GUI_DRIVER_INFO* drv_info, HANDLE hnd) FAST_FLASH;

EXTERN_C int detect_displays(GUI_DRIVER_INFO* drv_info);

extern GUI_DRIVER_INFO gui_driver;

#endif

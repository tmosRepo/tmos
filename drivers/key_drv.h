#ifndef	KEY_DRV_H
#define	KEY_DRV_H

//#include <drivers.h>

#define DRV_KEYBUF_SIZE	0x1F

struct KEY_DRIVER_DATA
{
    unsigned char	key_in, key_out;
    unsigned short	state;
    HANDLE			waiting;

    unsigned short	key_buf[DRV_KEYBUF_SIZE+1];
};

struct KEY_DRIVER_INFO
{
	DRIVER_INFO_Type 	info;
    KEY_DRIVER_DATA* 	drv_data;
};


void KEY_DCR(KEY_DRIVER_INFO* drv_info, unsigned int reason, HANDLE param) FAST_FLASH;
void KEY_DSR(KEY_DRIVER_INFO* drv_info, HANDLE hnd) FAST_FLASH;

extern const unsigned char key_codes[];
extern const unsigned int KEY_RD_PINS[];
extern const unsigned int KEY_DRV_PINS[];



#endif

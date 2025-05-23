/*
 * tmos_cfg.h
 *
 *  Created on: 10.03.2015
 *      Author: Miroslav Kostadinov
 */


//* TMOS related configurations

#ifndef BRD_CFG_H_
#define BRD_CFG_H_

#define GD32F4XX_COMPATIBLE 1

#define USE_TMOS_TL_V2		1
#define USE_CSTRING			1


#define USE_TMOS_STRING		1
#define USE_TMOS_STDLIB		1
#define USE_TIME			1
#define USE_TMOS_STDIO		1
#define USE_TMOS_LLONG		1
#define USE_CRC16			1
#define USE_CRC32			1
#define USE_CACHE			1
#define USE_SHA_1			1
#define USE_BASE32			1
#define USE_GUI				0
#define USE_GUI2			0
#define USE_AES_TMOS		1
#define STATIC_AES_TABLES	1
#define	USE_GPIO_EXPANDER 	0

#define USE_EXCEPTION_RECORD 1
#define USE_MEMORY_TRACKING	1
#define USE_TASK_REGISTERS	1

#define URL_CUSTOM_ROUTING	1

#define USE_CPU_SLEEP_MODE	1

#define USE_ADC_DMA_DRIVER 1		//Enable ADC DMA

/// USB stack options
#define USE_USB				1
#define USB_ENABLE_DEVICE	1
#define USB_ENABLE_HOST 	0
#define USB_ENABLE_OTG 		0
#define USB_ENABLE_CDC		1
#define USB_ENABLE_PRN		0
#define USB_ENABLE_HID		0

/// Ethernet
#define USE_NET				1
#define USE_LWIP_1_4_1		1
#define ETH_PHY_TLK110		1
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//		USART DRIVER
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define USART_DRV_RX_BUF_SIZE	(1024)

#define USE_SECURITY		1
#define USE_SECURITY_TLS	1
#define USE_SECURITY_CRYPTO	1
#define USE_SECURITY_TLS_DRV 1
#define MPI_ASM_SUPPORT		1

#endif /* BRD_CFG_H_ */

/*
 * sdio_drv.h
 *
 *  Created on: Oct 3, 2013
 *      Author: miro
 */

#ifndef SDIO_DRV_H_
#define SDIO_DRV_H_

#include <tmos.h>
#include <hardware_cpp.h>

#ifndef USE_SDIO_DMA_DRIVER
#define USE_SDIO_DMA_DRIVER 1		//Enable DMA by defaulr
#endif

#ifndef USE_SDIO_MULTIPLE_SLOTS
#define USE_SDIO_MULTIPLE_SLOTS 0
#endif

#define SDIO_OP_IDLE		0	//!< no operations
#define SDIO_OP_R3			1	//!< Response 3 must ignore CRC FAIL
#define SDIO_OP_R1			2	//!< Check R1 response for errors
#define SDIO_OP_READ		4	//!< Read data
#define SDIO_OP_WRITE		8	//!< Write data
#define SDIO_OP_CMD			16	//!< just command state

/** SDIO Driver Mode structure **/
struct SDIO_DRIVER_MODE
{
	const PIN_DESC	   sdio_pwr_en;	 //!< card power
	const uint16_t	   sdio_clk_div; //!< SDIO_CK high speed
};

/** SDIO Driver Data structure **/
struct SDIO_DRIVER_DATA
{
	HANDLE locker;			//!< Handle locked the driver
	HANDLE pending;			//!< Handle in progress
	HANDLE waiting;			//!< waiting for operation
#if USE_SDIO_MULTIPLE_SLOTS
	HANDLE last_slot;		//!< Handle used for the last slot
#endif
struct{
	uint16_t cnt;			//!< number of open handles
	uint16_t is_resetting; 	//!< The SDIO driver is being reset
}__attribute__((packed));
	unsigned int sdio_op;	//!< SDIO operations
#if USE_SDIO_DMA_DRIVER
	CHandle		rx_dma_hnd;
	CHandle		tx_dma_hnd;
#endif
};

/** SDIO Driver Info structure **/
struct SDIO_DRIVER_INFO
{
	DRIVER_INFO_Type   info;		//!< Standard driver info
	SDIO_TypeDef* 	   hw_base;		//!< SDIO Hardware registers for control
	SDIO_DRIVER_DATA*  drv_data;	//!< driver data
	const PIN_DESC*	   sdio_pins;	//!< zero terminated PIN_DESC list
#if USE_SDIO_DMA_DRIVER
	DMA_DRIVER_MODE		rx_dma_mode;
	DMA_DRIVER_MODE		tx_dma_mode;
#endif
};

typedef const struct SDIO_DRIVER_INFO* SDIO_INFO;

void SDIO_DCR(SDIO_INFO drv_info, unsigned int reason, HANDLE hnd) FAST_FLASH;
void SDIO_DSR(SDIO_INFO drv_info, HANDLE hnd) FAST_FLASH;
void SDIO_ISR(SDIO_INFO drv_info) FAST_FLASH;

#endif /* SDIO_DRV_H_ */

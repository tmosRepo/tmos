/*
 * usart_drv.cpp
 *
 *  Created on: Nov 6, 2012
 *      Author: miro
 *
 *  Edited on: June 5, 2024
 *      Editor: bratkov
 *  1. All bits related to interrupt enable or DMA are cleared from the mode
 *  2. checks for a fake pending ISR on the USART and clears it if there is one
 *  3. UART_TRACEXXX macros

 *  Edited on: Occtober 5, 2024
 *      Editor: bratkov
 *  1. ability to redefine UART pins, e.g. when using full/half duplex
 *  2. Reworked Configure Usart(...) to be compatible with STM32F0X...X series
 *  3. Rework of the UART_TRACEXXX macros
 */

#include <tmos.h>
#include <usart_drv.h>
#if USE_UART_DMA_DRIVER
#include <dma_drv.h>
#endif

#ifndef TRACE_UART_LEVEL
#define TRACE_UART_LEVEL	TRACE_LEVEL_NONE
#endif

#define TRACE_UART(x)		(drv_info->info.drv_index == x)

#if (TRACE_UART_LEVEL >= TRACE_DEFAULT_LEVEL)
#define IS_TRACE_UART  TRACE_UART(USART3_IRQn) || TRACE_UART(USART1_IRQn)
#endif

#if TRACE_IS  && defined IS_TRACE_UART

FILE_OPTIMIZE("Os")
static char get_uart_index(USART_DRIVER_INFO * drv_info)
{
	uint32_t  res = 0;
	while(usart_index[res++] != drv_info->info.drv_index)
	{
		if(usart_index[res] ==  INALID_DRV_INDX)
		{
			res =0;
			break;
		}
	}
	return '0' + res;
}
FILE_RESET_OPTIONS

#define UART_TRACE_CHAR(ch) 		do{if(IS_TRACE_UART)TRACE_CHAR_LEVEL(TRACE_UART_LEVEL, ch);}while(0)
#define UART_TRACE(...) 			do{if(IS_TRACE_UART)TRACE_LEVEL(TRACE_UART_LEVEL, __VA_ARGS__);}while(0)
#define UART_TRACE1(str)			do{if(IS_TRACE_UART)TRACE1_LEVEL(TRACE_UART_LEVEL, str);}while(0)
#define UART_TRACELN(str, ...)		do{if(IS_TRACE_UART){	\
									TRACE_LEVEL(TRACE_UART_LEVEL, "\r\nUSART%c ",get_uart_index(drv_info));\
									TRACE_LEVEL(TRACE_UART_LEVEL, str, ##__VA_ARGS__);}\
									}while(0)
#define UART_TRACELN1(str)			do{if(IS_TRACE_UART){	\
									TRACELN_LEVEL(TRACE_UART_LEVEL, "\r\nUSART%c ",get_uart_index(drv_info));\
									TRACE1_LEVEL(TRACE_UART_LEVEL, str);}\
									}while(0)
#define UART_TRACE_ERROR(str, ...)	do{	\
									TRACELN("\r\n\e[91mUSART%c :",get_uart_index(drv_info));\
									TRACE(str "\e[m", ##__VA_ARGS__);\
									}while(0)
#else

#define UART_TRACE_CHAR(ch)
#define UART_TRACE(...)
#define UART_TRACE1(str)
#define UART_TRACELN(str, ...)
#define UART_TRACELN1(str)
#define UART_TRACE_ERROR(str, ...)

#endif // TRACE_IS && IS_TRACE_UART


/**
 *
 * @param drv_data	contains the current USART mode
 * @param mode		the handler mode to open
 * @return			true of modes match, otherwise false
 */
static bool SLOW_FLASH is_same_mode(USART_DRIVER_DATA * drv_data, USART_DRIVER_MODE * mode)
{
	USART_DRIVER_MODE& drv_mode = drv_data->mode;
	if(drv_mode.mode_cr1 != ((mode->mode_cr1 & ~USART_ISR_ENABLE_MASK) | USART_ENABLE))
		return false;
	if(drv_mode.mode_cr2 != mode->mode_cr2)
		return false;
	if(drv_mode.mode_cr3 != (mode->mode_cr3 & ~(USART_CR3_DMAR|USART_CR3_DMAT|USART_CR3_CTSIE)))
		return false;
	if(drv_mode.baudrate != mode->baudrate)
		return false;
	return true;
}
/**
 * configures the USART mode when opening a handler
 *
 * @param drv_info	pointer to	USART_DRIVER_INFO
 * @param mode		pointer to	USART_DRIVER_MODE (initial values of USART_CR1,2 and 3)
 * @return			true if configured successfully
 */
static bool SLOW_FLASH ConfigureUsart(USART_DRIVER_INFO * drv_info, USART_DRIVER_MODE * mode)
{
	USART_TypeDef* USARTx = drv_info->hw_base;
	USART_DRIVER_DATA * drv_data = drv_info->drv_data;
	USART_DRIVER_MODE& drv_mode = drv_data->mode;

	// before configuring the USART, it must be disabled
	drv_mode.mode_cr1 = (mode->mode_cr1 & ~(USART_ISR_ENABLE_MASK|USART_ENABLE));
	drv_mode.mode_cr2 = mode->mode_cr2;
	drv_mode.mode_cr3 = mode->mode_cr3 & ~(USART_CR3_DMAR|USART_CR3_DMAT|USART_CR3_CTSIE);
	drv_mode.baudrate = mode->baudrate;

	//Some bits in CR1,CR2 and CR3 can only be written when the USART is disabled
	USARTx->USART_CR1 = drv_mode.mode_cr1;
	USARTx->USART_CR2 = drv_mode.mode_cr2;
	USARTx->USART_CR3 = drv_mode.mode_cr3;

	/* Configure the USART Baud Rate -------------------------------------------*/
	set_usart_baudrate(USARTx, drv_info->info.peripheral_indx, mode->baudrate);

	// Now the USART can be enabled
	drv_mode.mode_cr1  |= USART_ENABLE;
	USARTx->USART_CR1 = drv_mode.mode_cr1;

	drv_enable_isr(&drv_info->info); // NVIC enable
	if(NVIC_GetPendingIRQ(drv_info->info.drv_index))
	{
		if(!(get_usart_sr(USARTx) & get_usart_imr(USARTx)))
		{
			UART_TRACE_ERROR("Fake pending ISR, removing !");
			NVIC_ClearPendingIRQ(drv_info->info.drv_index);
		}
	}

#if USE_UART_DMA_DRIVER
	if(drv_info->rx_dma_mode.dma_index < INALID_DRV_INDX)
	{
//		USARTx->USART_CR3 |= USART_CR3_DMAR;
		if((drv_data->rx_dma_hnd.res >= RES_CLOSED))
		{
			memcpy(&drv_data->rx_dma_mode, &drv_info->rx_dma_mode, sizeof(DMA_DRIVER_MODE));
			if(mode->mode_cr1 & USART_CR1_M)
			{
				drv_data->rx_dma_mode.dma_ch_cr = stm32_dma_msize(drv_data->rx_dma_mode.dma_ch_cr, 16);
				drv_data->rx_dma_mode.dma_ch_cr = stm32_dma_psize(drv_data->rx_dma_mode.dma_ch_cr, 16);
			}
			else
			{
				drv_data->rx_dma_mode.dma_ch_cr = stm32_dma_msize(drv_data->rx_dma_mode.dma_ch_cr, 8);
				drv_data->rx_dma_mode.dma_ch_cr = stm32_dma_psize(drv_data->rx_dma_mode.dma_ch_cr, 8);
			}
			if(!drv_data->rx_dma_hnd.drv_open(
				drv_info->rx_dma_mode.dma_index,
				&drv_data->rx_dma_mode))
				return false;
		}
	}
	if(drv_info->tx_dma_mode.dma_index < INALID_DRV_INDX)
	{
//		USARTx->USART_CR3 |= USART_CR3_DMAT;
		memcpy(&drv_data->tx_dma_mode, &drv_info->tx_dma_mode, sizeof(DMA_DRIVER_MODE));
		if(mode->mode_cr1 & USART_CR1_M)
		{
			drv_data->tx_dma_mode.dma_ch_cr = stm32_dma_msize(drv_data->tx_dma_mode.dma_ch_cr, 16);
			drv_data->tx_dma_mode.dma_ch_cr = stm32_dma_psize(drv_data->tx_dma_mode.dma_ch_cr, 16);
		}
		else
		{
			drv_data->tx_dma_mode.dma_ch_cr = stm32_dma_msize(drv_data->tx_dma_mode.dma_ch_cr, 8);
			drv_data->tx_dma_mode.dma_ch_cr = stm32_dma_psize(drv_data->tx_dma_mode.dma_ch_cr, 8);
		}
		if(!drv_data->tx_dma_hnd.drv_open(
			drv_info->tx_dma_mode.dma_index,
			&drv_data->tx_dma_mode))
			return false;
	}
#endif
	return true;
}

#if USE_UART_DMA_DRIVER
void UPDATE_RX_WRPTR(USART_DRIVER_INFO * drv_info, USART_DRIVER_DATA* drv_data)
{
	if(drv_info->rx_dma_mode.dma_index < INALID_DRV_INDX)
	{
		uint32_t remaining;

		remaining = dma_drv_get_ndtr(drv_info->rx_dma_mode.dma_index);
		if(remaining)
		{
			if(drv_data->mode.mode_cr1 & USART_CR1_M)
				remaining *= 2;
			if(remaining < USART_DRV_RX_BUF_SIZE)
				remaining = USART_DRV_RX_BUF_SIZE - remaining;
			else
				remaining = 0;
		}
		drv_data->rx_wrptr = drv_data->rx_buf + remaining;
	}
}
#else
#define UPDATE_RX_WRPTR(drv_info, drv_data)
#endif


static inline void START_RX_BUF(USART_TypeDef* uart, USART_DRIVER_DATA* drv_data)
{
	uint32_t ints = USART_STATUS_RXNEIE | USART_STATUS_IDLEIE;
	unsigned char* buf_reset = drv_data->rx_buf;

//	drv_data->rx_remaining = USART_DRV_RX_BUF_SIZE;
#if USE_UART_DMA_DRIVER
	if(drv_data->rx_dma_hnd.res < RES_CLOSED)
	{
		uint32_t remaining;
		DRIVER_INDEX drv_indx;

		drv_indx = ((DMA_DRIVER_MODE*)(drv_data->rx_dma_hnd.mode.as_voidptr))->dma_index;
		if(dma_drv_is_en(drv_indx))
		{
			remaining = dma_drv_get_ndtr(drv_indx);
			if(remaining)
			{
				if(drv_data->mode.mode_cr1 & USART_CR1_M)
					remaining *= 2;
				if(remaining < USART_DRV_RX_BUF_SIZE)
					remaining = USART_DRV_RX_BUF_SIZE - remaining;
				else
					remaining = 0;
			}

			buf_reset += remaining;
		} else
		{
			remaining = USART_DRV_RX_BUF_SIZE;
			if(drv_data->mode.mode_cr1 & USART_CR1_M)
				remaining /= 2;
			drv_data->rx_dma_hnd.drv_read_write(drv_data->rx_buf, (void*)&get_usart_rdr(uart), remaining);
			uart->USART_CR3 |= USART_CR3_DMAR;
		}
		ints = USART_STATUS_IDLEIE;
	}
#endif
	drv_data->rx_ptr = buf_reset;
	drv_data->rx_wrptr = buf_reset;
	enable_usart_drv_ints(uart, ints);
}

static inline void START_RX_HND(USART_TypeDef* uart, USART_DRIVER_DATA* drv_data, HANDLE hnd)
{
//	drv_data->rx_remaining = hnd->len;
	drv_data->rx_wrptr = hnd->dst.as_voidptr;
	if(drv_data->mode.mode_cr1 & USART_CR1_M)
		drv_data->rx_ptr = (uint16_t *)drv_data->rx_wrptr + hnd->len;
	else
		drv_data->rx_ptr = (uint8_t *)drv_data->rx_wrptr + hnd->len;
	enable_usart_drv_ints(uart, USART_STATUS_RXNEIE | USART_STATUS_IDLEIE);
}

static inline void STOP_TX(USART_TypeDef* uart)
{
	uart->USART_CR1 &= ~USART_CR1_TXEIE;
}

static inline void START_TX_HND(USART_DRIVER_INFO * drv_info, USART_TypeDef* uart, HANDLE hnd)
{
#if USE_UART_DMA_DRIVER
	if(drv_info->tx_dma_mode.dma_index < INALID_DRV_INDX)
	{
		drv_info->drv_data->tx_dma_hnd.drv_read_write((void*)&get_usart_tdr(uart),
				hnd->src.as_charptr, hnd->len);
		uart->USART_CR3 |= USART_CR3_DMAT; // enable a peripheral to send a DMA requests
		return;
	}
#endif
	if( get_usart_sr(uart) & USART_STATUS_TXE )
	{
//		TRACE_BUF(hnd->src.as_charptr, 1, TC_BG_MAGENTA);
		if(drv_info->drv_data->mode.mode_cr1 & USART_CR1_M)
			get_usart_tdr(uart) = *hnd->src.as_shortptr++;
		else
			get_usart_tdr(uart) = *hnd->src.as_charptr++;

		hnd->len--;
	}
	uart->USART_CR1 |= USART_CR1_TXEIE;
}

static inline void STOP_RX_HND(USART_TypeDef* uart, USART_DRIVER_DATA* drv_data, HANDLE hnd, RES_CODE res)
{
	uint32_t remaining;

	// STOP_RX(Uart);
	drv_data->hnd_rcv = hnd->next;
	remaining = (uint8_t *)drv_data->rx_ptr - (uint8_t *)drv_data->rx_wrptr;
	if(drv_data->mode.mode_cr1 & USART_CR1_M)
	{
		remaining >>= 1;
		if(remaining < hnd->len)
		{
			hnd->dst.as_shortptr += hnd->len - remaining;
			hnd->len = remaining;
		}
	}
	else
	{
		if(remaining < hnd->len)
		{
			hnd->dst.as_int += hnd->len - remaining;
			hnd->len = remaining;
		}
	}
	usr_HND_SET_STATUS(hnd, res);
  	if( (hnd=drv_data->hnd_rcv) )
      	START_RX_HND(uart, drv_data, hnd);
  	else
		START_RX_BUF(uart, drv_data);
}

static bool FLUSH_RX_BUF(USART_DRIVER_DATA *drv_data, HANDLE hnd)
{
	signed int size;
	if(drv_data->mode.mode_cr1 & USART_CR1_M)
	{
		uint16_t * ptr;
	  	ptr = (uint16_t *)drv_data->rx_wrptr;
	  	if( ptr != drv_data->rx_ptr )
	  	{
	  		if(ptr < drv_data->rx_ptr)
	  		{
	  			size =min(hnd->len, (((&drv_data->rx_buf[USART_DRV_RX_BUF_SIZE]) - (uint8_t *)drv_data->rx_ptr) >>1));
	  			memcpy(hnd->dst.as_byteptr, drv_data->rx_ptr, size * sizeof(uint16_t));
	  			hnd->len -= size;
	  			hnd->dst.as_shortptr += size;
	  			drv_data->rx_ptr = (uint16_t *)drv_data->rx_ptr + size;
	  			if(drv_data->rx_ptr >= &drv_data->rx_buf[USART_DRV_RX_BUF_SIZE])
	  				drv_data->rx_ptr = drv_data->rx_buf;
	  		}

	      	if( hnd->len && (ptr!= drv_data->rx_ptr) )
	  		{
	  			size =min(hnd->len, ptr - (uint16_t *)drv_data->rx_ptr);
	  			memcpy(hnd->dst.as_byteptr, drv_data->rx_ptr, size * sizeof(uint16_t));
	  			hnd->len -= size;
	  			hnd->dst.as_shortptr += size;
	  			drv_data->rx_ptr = (uint16_t *)drv_data->rx_ptr + size;
	  			if(drv_data->rx_ptr >= &drv_data->rx_buf[USART_DRV_RX_BUF_SIZE])
	  				drv_data->rx_ptr = drv_data->rx_buf;
	  		}
	  		return true;
	  	}
	}
	else
	{
		uint8_t * ptr;
	  	ptr = (uint8_t *)drv_data->rx_wrptr;
	  	if( ptr != drv_data->rx_ptr )
	  	{
	  		if(ptr < drv_data->rx_ptr)
	  		{
	  			size =min(hnd->len, (&drv_data->rx_buf[USART_DRV_RX_BUF_SIZE]) - (uint8_t *)drv_data->rx_ptr);
	  			memcpy(hnd->dst.as_byteptr, drv_data->rx_ptr, size);
	  			hnd->len -= size;
	  			hnd->dst.as_int += size;
	  			drv_data->rx_ptr = (uint8_t *)drv_data->rx_ptr + size;
	  			if(drv_data->rx_ptr >= &drv_data->rx_buf[USART_DRV_RX_BUF_SIZE])
	  				drv_data->rx_ptr = drv_data->rx_buf;
	  		}

	      	if( hnd->len && (ptr!= drv_data->rx_ptr) )
	  		{
	  			size =min(hnd->len, ptr - (uint8_t *)drv_data->rx_ptr);
	  			memcpy(hnd->dst.as_byteptr, drv_data->rx_ptr, size);
	  			hnd->len -= size;
	  			hnd->dst.as_int += size;
	  			drv_data->rx_ptr = (uint8_t *)drv_data->rx_ptr + size;
	  			if(drv_data->rx_ptr >= &drv_data->rx_buf[USART_DRV_RX_BUF_SIZE])
	  				drv_data->rx_ptr = drv_data->rx_buf;
	  		}
	  		return true;
	  	}
	}
  	return false;
}

static void dcr_cancel_handle(USART_DRIVER_INFO* drv_info,
		USART_DRIVER_DATA *drv_data, HANDLE hnd)
{
	USART_TypeDef * uart = drv_info->hw_base;

	if( hnd->res & FLG_BUSY )
	{
		if (hnd->cmd & FLAG_READ)
	    {
			if(hnd == drv_data->hnd_rcv)
			{
				uint32_t remaining;

				drv_data->hnd_rcv = hnd->next;

#if USE_UART_DMA_DRIVER
				if(drv_info->rx_dma_mode.dma_index < INALID_DRV_INDX)
				{
			      	//refresh buffer
			      	UPDATE_RX_WRPTR(drv_info, drv_data);

			      	if (FLUSH_RX_BUF(drv_data, hnd))
			      	{
			      		svc_HND_SET_STATUS(hnd, RES_SIG_OK);
			      	} else
			      	{
			      		svc_HND_SET_STATUS(hnd, RES_SIG_IDLE);
			      	}

				} else
#endif
				{
					// STOP_RX(Uart);
					if(drv_data->mode.mode_cr1 & USART_CR1_M)
					{
						remaining = (uint16_t *)drv_data->rx_ptr - (uint16_t *)drv_data->rx_wrptr;
						if(remaining < hnd->len)
						{
							hnd->dst.as_shortptr += hnd->len - remaining;
							hnd->len = remaining;
				      		svc_HND_SET_STATUS(hnd, RES_SIG_OK);
						} else
							svc_HND_SET_STATUS(hnd, RES_SIG_IDLE);
					}
					else
					{
						remaining = (uint8_t *)drv_data->rx_ptr - (uint8_t *)drv_data->rx_wrptr;
						if(remaining < hnd->len)
						{
							hnd->dst.as_int += hnd->len - remaining;
							hnd->len = remaining;
				      		svc_HND_SET_STATUS(hnd, RES_SIG_OK);
						} else
							svc_HND_SET_STATUS(hnd, RES_SIG_IDLE);
					}
					if( (hnd=drv_data->hnd_rcv) )
						START_RX_HND(uart, drv_data, hnd);
					else
						START_RX_BUF(uart, drv_data);
				}
			}
			else
			{
				// try cnacel
				hnd->svc_list_cancel(drv_data->hnd_rcv);
			}
		}
		else
		{
			if(hnd == drv_data->hnd_snd)
			{
				STOP_TX(uart);
				drv_data->hnd_snd = hnd->next;
	      		svc_HND_SET_STATUS(hnd, RES_SIG_IDLE);
				if( (hnd=drv_data->hnd_snd) )
					START_TX_HND(drv_info, uart, hnd);
			}
			else
			{
				hnd->svc_list_cancel(drv_data->hnd_snd);
			}

		}
	}

}

void USART_DCR(USART_DRIVER_INFO* drv_info, unsigned int reason, HANDLE hnd)
{
//	UART_Type * Uart = drv_info->hw_base;
	USART_DRIVER_DATA *drv_data = drv_info->drv_data;

	switch(reason)
	{
		case DCR_RESET:
			RCCPeripheralReset(drv_info->info.peripheral_indx);
			RCCPeripheralDisable(drv_info->info.peripheral_indx); // ??? turn off
#if USE_UART_DMA_DRIVER
        	drv_data->rx_dma_hnd.client.drv_index = drv_info->info.drv_index;
        	drv_data->tx_dma_hnd.client.drv_index = drv_info->info.drv_index;
#endif
			break;

		case DCR_OPEN:
		{
			hnd->res = RES_ERROR;
			USART_DRIVER_MODE *usart_mode = (USART_DRIVER_MODE *)(hnd->mode.as_voidptr);
			if(usart_mode)
			{
				//unsigned long mode, baudrate;
				if(drv_data->cnt)
				{
					if(!is_same_mode(drv_data, usart_mode))
						break; // return ERR (exit form case)
					UART_TRACELN(" OPEN[%u]", drv_data->cnt);
				}
				else
				{
					// Enable AND Reset the UART peripheral
					RCCPeripheralEnable(drv_info->info.peripheral_indx);
					RCCPeripheralReset(drv_info->info.peripheral_indx);
					// configures the USART pins
					if(usart_mode->use_pins == nullptr){
						// uses default pins and their modes
						PIO_Cfg_List(drv_info->uart_pins);
					}else{
						// user defined  pins and modes
						// NOTE! more than 4 pins can be configured
						PIO_Cfg_List(usart_mode->use_pins);
					}
					UART_TRACELN1(" OPEN");
					if(!ConfigureUsart(drv_info, usart_mode))
						break; // return ERR (exit form case)
/*
#if USE_UART_DMA_DRIVER
					if(drv_info->rx_dma_mode.dma_index < INALID_DRV_INDX)
						if((drv_data->rx_dma_hnd.res >= RES_CLOSED) && !drv_data->rx_dma_hnd.drv_open(
								drv_info->rx_dma_mode.dma_index,
								&drv_info->rx_dma_mode))
							break;
					if(drv_info->tx_dma_mode.dma_index < INALID_DRV_INDX)
						if(!drv_data->tx_dma_hnd.drv_open(
								drv_info->tx_dma_mode.dma_index,
								&drv_info->tx_dma_mode))
						{
							break;
						}
#endif
*/

					START_RX_BUF(drv_info->hw_base, drv_data);
				}
				drv_data->cnt++;
				hnd->res = RES_OK;
			}
			break;
		}

		case DCR_CLOSE:
			if(drv_data->cnt)
				drv_data->cnt--;
			if(!drv_data->cnt)
			{
				//Disable ?
#if USE_UART_DMA_DRIVER
				if(drv_info->rx_dma_mode.dma_index < INALID_DRV_INDX)
				{
					// can only cancel dma transfers
					if(drv_data->rx_dma_hnd.res & FLG_BUSY)
						drv_data->rx_dma_hnd.hcontrol(DCR_CANCEL);

					drv_data->rx_dma_hnd.close();
				}
				drv_data->tx_dma_hnd.close();
#endif
				UART_TRACELN1(" CLOSED");
				NVIC_DisableIRQ(drv_info->info.drv_index);
				//	Uart->UARTDisable();
				//	STOP_RX(Uart);
				STOP_TX(drv_info->hw_base);
				RCCPeripheralDisable(drv_info->info.peripheral_indx);

				PIO_Free_List(drv_info->uart_pins);
				memclr(&drv_data->mode, sizeof(USART_DRIVER_MODE));
			}
			break;

		case DCR_CANCEL:
			dcr_cancel_handle(drv_info, drv_data, hnd);
			break;

        case DCR_CLOCK:
        	break;

#if USE_UART_DMA_DRIVER
		case DCR_SIGNAL:
			//signal tx/dma complete
			if(hnd == &drv_data->tx_dma_hnd)
			{
				drv_info->hw_base->USART_CR3 &= ~USART_CR3_DMAT;
				hnd = drv_data->hnd_snd;
				if(hnd)
				{
					if(hnd->len > drv_data->tx_dma_hnd.len)
					{
						if(drv_data->mode.mode_cr1 & USART_CR1_M)
							hnd->src.as_shortptr += hnd->len - drv_data->tx_dma_hnd.len;
						else
							hnd->src.as_byteptr += hnd->len - drv_data->tx_dma_hnd.len;
						hnd->len = drv_data->tx_dma_hnd.len;
					}
				}
				drv_info->hw_base->USART_CR1 |= USART_CR1_TCIE;
			} else
			{
				if(hnd == &drv_data->rx_dma_hnd)
				{
					uint32_t remaining;

					if(drv_data->mode.mode_cr1 & USART_CR1_M)
						drv_data->rx_wrptr = drv_data->rx_buf + USART_DRV_RX_BUF_SIZE - drv_data->rx_dma_hnd.len * sizeof(uint16_t);
					else
						drv_data->rx_wrptr = drv_data->rx_buf + USART_DRV_RX_BUF_SIZE - drv_data->rx_dma_hnd.len;

					hnd = drv_data->hnd_rcv;
					if(hnd)
					{

						if(FLUSH_RX_BUF(drv_data, hnd))
						{
							drv_data->hnd_rcv = hnd->next;
							svc_HND_SET_STATUS(hnd, RES_SIG_OK);
						}

					}
					if(!drv_data->rx_dma_hnd.len)
					{
						drv_data->rx_wrptr = drv_data->rx_buf;
						remaining = USART_DRV_RX_BUF_SIZE;
						if(drv_data->mode.mode_cr1 & USART_CR1_M)
							remaining >>= 1;
					} else
						remaining = drv_data->rx_dma_hnd.len;
					if(drv_data->cnt)
						drv_data->rx_dma_hnd.drv_read_write(drv_data->rx_wrptr,
							(void*)&get_usart_rdr(drv_info->hw_base), remaining);
				}
			}
			break;
#endif

	}
}

void USART_DSR(USART_DRIVER_INFO* drv_info, HANDLE hnd)
{
	USART_DRIVER_DATA* drv_data = drv_info->drv_data;
	USART_TypeDef* uart = drv_info->hw_base;

	hnd->next = NULL;
	if (hnd->cmd & FLAG_READ)
    {
      	hnd->res = RES_BUSY; // no data transfered
      	if(drv_data->hnd_rcv)
      	{
			hnd->list_add(drv_data->hnd_rcv);
			return;
      	}

      	if(drv_data->usart_err)
      	{
      		hnd->error = drv_data->usart_err;
      		drv_data->usart_err = 0;
      		svc_HND_SET_STATUS(hnd, RES_SIG_ERROR);
      		return;
      	}

      	if (hnd->len == 0)
      	{
      		svc_HND_SET_STATUS(hnd, RES_SIG_OK);
      		return;
      	}

      	//refresh buffer
      	UPDATE_RX_WRPTR(drv_info, drv_data);

      	if (FLUSH_RX_BUF(drv_data, hnd))
      	{
      		svc_HND_SET_STATUS(hnd, RES_SIG_OK);
      	} else
      	{
          	//receive directly
          	drv_data->hnd_rcv = hnd;
#if USE_UART_DMA_DRIVER
    		if(drv_info->rx_dma_mode.dma_index >= INALID_DRV_INDX)
#endif
    		{
    			START_RX_HND(uart, drv_data, hnd);
    		}
      	}
		return;
    }

	if (hnd->cmd & FLAG_WRITE)
    {
		if(hnd->len)
		{
			if(drv_data->hnd_snd)
			{
				hnd->list_add(drv_data->hnd_snd);
				return;
			}
			hnd->res = RES_BUSY;
			drv_data->hnd_snd = hnd;
			START_TX_HND(drv_info, uart, hnd);
		}
		else
			svc_HND_SET_STATUS(hnd, RES_SIG_OK);
		return;
    }

	svc_HND_SET_STATUS(hnd, RES_SIG_ERROR);
}

void USART_ISR(USART_DRIVER_INFO* drv_info)
{
	USART_TypeDef* uart = drv_info->hw_base;
	USART_DRIVER_DATA* drv_data = drv_info->drv_data;
	HANDLE hnd;
	unsigned int status;



	status = get_usart_sr(uart);
	UART_TRACELN("isr %x", status);
	status &= get_usart_imr(uart);

	UART_TRACE("/%x", status);
	//check the receiver
	if (status & USART_STATUS_ORE)
	{
		TRACELN("over");
		clr_usart_over(uart); //clear overrun flag
    	if( (hnd=drv_data->hnd_rcv) )
    	{
#if USE_UART_DMA_DRIVER
    		if(drv_info->rx_dma_mode.dma_index < INALID_DRV_INDX)
    		{
    	      	UPDATE_RX_WRPTR(drv_info, drv_data);
				FLUSH_RX_BUF(drv_data, hnd);
				drv_data->hnd_rcv = hnd->next;
				usr_HND_SET_STATUS(hnd, RES_SIG_ERROR);
    		} else
#endif
    		{
    			STOP_RX_HND(uart, drv_data, hnd, RES_SIG_ERROR);
    		}
    	} else
    	{
    		//disable ovr interrupt
    		drv_data->usart_err = status;
    	}
	}
	if (status & USART_STATUS_RXNE)
    {
#if USE_UART_DMA_DRIVER
		if(drv_info->rx_dma_mode.dma_index >= INALID_DRV_INDX) // ignore fake interrupts
#endif
		{
			if(drv_data->mode.mode_cr1 & USART_CR1_M)
			{
				*(uint16_t *)drv_data->rx_wrptr = get_usart_rdr(uart);
				drv_data->rx_wrptr = (uint16_t *)drv_data->rx_wrptr + 1;
			}
			else
			{
				*(uint8_t *)drv_data->rx_wrptr = get_usart_rdr(uart);
				drv_data->rx_wrptr = (uint8_t *)drv_data->rx_wrptr + 1;
			}
			if(drv_data->rx_wrptr == drv_data->rx_ptr && (hnd = drv_data->hnd_rcv))
			{
				STOP_RX_HND(uart, drv_data, hnd, RES_SIG_OK);
			} else
			{
				if (drv_data->rx_wrptr == &drv_data->rx_buf[USART_DRV_RX_BUF_SIZE])
					drv_data->rx_wrptr = drv_data->rx_buf;
				if(drv_data->rx_wrptr == drv_data->rx_ptr)
				{
					if(drv_data->mode.mode_cr1 & USART_CR1_M)
						drv_data->rx_wrptr = (uint16_t *)drv_data->rx_wrptr + 1;
					else
						drv_data->rx_wrptr = (uint8_t *)drv_data->rx_wrptr + 1;
					if(drv_data->rx_ptr >= &drv_data->rx_buf[USART_DRV_RX_BUF_SIZE])
						drv_data->rx_ptr = drv_data->rx_buf;
				}
			}
		}
	} else
	{
		if (status & USART_STATUS_IDLE)
		{
			clr_usart_idle(uart); //clear idle flag
	    	if( (hnd=drv_data->hnd_rcv) )
	    	{
#if USE_UART_DMA_DRIVER
	    		if(drv_info->rx_dma_mode.dma_index < INALID_DRV_INDX)
	    		{
	    			do
	    			{
						UPDATE_RX_WRPTR(drv_info, drv_data);
						if (FLUSH_RX_BUF(drv_data, hnd))
						{
							drv_data->hnd_rcv = hnd->next;
							usr_HND_SET_STATUS(hnd, RES_SIG_OK);
						} else
							break;
	    			} while ((hnd=drv_data->hnd_rcv));
	    		} else
#endif
	    		{
	    			if(drv_data->mode.mode_cr1 & USART_CR1_M)
	    			{
						if((((uint16_t *)drv_data->rx_ptr - (uint16_t *)drv_data->rx_wrptr)>>1) != hnd->len)
						{
		  		  			TRACELN("idle");
							STOP_RX_HND(uart, drv_data, hnd, RES_SIG_OK);
						}
	    			}
	    			else
	    			{
						if(((uint8_t *)drv_data->rx_ptr - (uint8_t *)drv_data->rx_wrptr) != hnd->len)
						{
		  		  			TRACELN("idle");
							STOP_RX_HND(uart, drv_data, hnd, RES_SIG_OK);
						}
	    			}
	    		}
	    	}
		}
	}

	// check transmitter
	if( status & USART_STATUS_TC )
	{
		if((hnd=drv_data->hnd_snd))
		{
			drv_data->hnd_snd = hnd->next;
			usr_HND_SET_STATUS(hnd, RES_SIG_OK);
		}
#if USE_UART_DMA_DRIVER
		if(drv_info->tx_dma_mode.dma_index < INALID_DRV_INDX)
			uart->USART_CR1 &= ~(USART_CR1_TCIE | USART_CR1_TXEIE);
		else
#endif
		uart->USART_CR1 = (uart->USART_CR1 &~USART_CR1_TCIE) | USART_CR1_TXEIE;
	}
	if( status & USART_STATUS_TXE )
	{
		if((hnd=drv_data->hnd_snd))
		{
			if(hnd->len)
			{
				hnd->len--;
//				TRACE_BUF(hnd->src.as_charptr, 1, TC_TXT_MAGENTA);
    			if(drv_data->mode.mode_cr1 & USART_CR1_M)
    				get_usart_tdr(uart) = *hnd->src.as_shortptr++;
    			else
    				get_usart_tdr(uart) = *hnd->src.as_charptr++;
			} else
			{
				if(hnd->cmd & FLAG_LOCK)
				{
					uart->USART_CR1 = (uart->USART_CR1 &~USART_CR1_TXEIE) | USART_CR1_TCIE;
				} else
				{
					drv_data->hnd_snd = hnd->next;
					usr_HND_SET_STATUS(hnd, RES_SIG_OK);
				}
			}

		}
		if(!hnd)
		{
			STOP_TX(uart);
		}
	}

}



/*
 * dma_drv.cpp
 *
 *  Created on: Nov 21, 2012
 *      Author: miro
 */

#include <tmos.h>
#include <dma_drv.h>

#ifndef TRACE_DMA_LEVEL
#define TRACE_DMA_LEVEL	TRACE_LEVEL_NONE
#endif

#define TRACE_DMA_CHANNEL		(drv_info->ch_indx == 0 || drv_info->ch_indx == 5)

#define DMA_TRACE_CHAR(ch) 		do{if(TRACE_DMA_CHANNEL)TRACE_CHAR_LEVEL(TRACE_DMA_LEVEL, ch);}while(0)
#define DMA_TRACE(...) 			do{if(TRACE_DMA_CHANNEL)TRACE_LEVEL(TRACE_DMA_LEVEL, __VA_ARGS__);}while(0)
#define DMA_TRACE1(str)			do{if(TRACE_DMA_CHANNEL)TRACE1_LEVEL(TRACE_DMA_LEVEL, str);}while(0)
#define DMA_TRACELN(str, ...)	do{if(TRACE_DMA_CHANNEL)TRACELN_LEVEL(TRACE_DMA_LEVEL, str, ##__VA_ARGS__);}while(0)
#define DMA_TRACELN1(str)		do{if(TRACE_DMA_CHANNEL)TRACELN1_LEVEL(TRACE_DMA_LEVEL, str);}while(0)

extern 	 char* const DRV_TABLE[INALID_DRV_INDX+1];

uint32_t dma_drv_get_ndtr(DRIVER_INDEX drv_index)
{
	unsigned int ndtr = 0;

	if(drv_index < INALID_DRV_INDX)
	{
		DMA_DRIVER_INFO* drv_info;

		drv_info = (DMA_DRIVER_INFO*)(void*)(DRV_TABLE[drv_index]-1);
		ndtr = stm32_dma_ndtr(drv_info->hw_base, drv_info->ch_indx);
	}

	return ndtr;
}

uint32_t dma_drv_is_en(DRIVER_INDEX drv_index)
{
	unsigned int is_en = 0;

	if(drv_index < INALID_DRV_INDX)
	{
		DMA_DRIVER_INFO* drv_info;

		drv_info = (DMA_DRIVER_INFO*)(void*)(DRV_TABLE[drv_index]-1);
		is_en = stm32_dma_is_en(drv_info->hw_base, drv_info->ch_indx);
	}

	return is_en;
}

bool DMA_START_WAITING(DMA_DRIVER_INFO* drv_info)
{
	DMA_CHANNEL_DATA* ch_data;
	HANDLE hnd;

	ch_data = drv_info->ch_data;

	if( ch_data->pending == nullptr && ch_data->terminating == nullptr
			&& (hnd = ch_data->waiting))
	{
		DMA_DRIVER_MODE* mode;

		ch_data->waiting = hnd->next;

		mode = (DMA_DRIVER_MODE *)hnd->mode.as_voidptr;
		if(mode != ch_data->last_mode)
		{
			ch_data->last_mode = mode;
			// configure the channel
			stm32_dma_ch_cfg(drv_info->hw_base, drv_info->ch_indx, mode);
		}

		// start transfer...
		stm32_dma_start(drv_info->hw_base, drv_info->ch_indx, hnd);
		//TODO:
		{
			/*
			 * AN4031 - Using the STM32F2, STM32F4 and STM32F7 SeriesDMA controller
			 * If the user enables the used peripheral before the corresponding DMA stream, a “FEIF”
			 * (FIFO Error Interrupt Flag) may be set due to the fact the DMA is not ready to provide the
			 * first required data to the peripheral (in case of memory-to-peripheral transfer).
			 */
		}
		ch_data->pending = hnd;
		return true;
	}
	return false;
}
//*----------------------------------------------------------------------------
//*			DCR function
//*----------------------------------------------------------------------------
void DMA_DCR(DMA_DRIVER_INFO* drv_info, unsigned int reason, HANDLE hnd)
{
	DMA_DRIVER_DATA* drv_data = drv_info->drv_data;

	switch(reason)
    {
        case DCR_RESET:
        	if(drv_data->dma_state !=  DMA_ST_RESET)
        	{
        		// there can be one DMA Driver for each channel/stream
        		// but only the 1st channel will reset the entier DMA peripheral
        		drv_data->dma_state =  DMA_ST_RESET;
    			RCCPeripheralReset(drv_info->info.peripheral_indx);
            	RCCPeripheralDisable(drv_info->info.peripheral_indx);
        	}
        	break;

        case DCR_OPEN:

        	DMA_DRIVER_MODE * mode;
        	mode = (DMA_DRIVER_MODE *)hnd->mode.as_voidptr;
        	if(mode)
        	{
        		// first for this DMA?
        		if(!drv_data->cnt++)
        		{
        			// enable DMA controller
        			RCCPeripheralEnable(drv_info->info.peripheral_indx);
        		}

        		// first for this channel?
        		if(!drv_info->ch_data->cnt++)
        		{
        			// enable channel IRQ
    				drv_enable_isr(&drv_info->info);
        		}

				hnd->res = RES_OK;
        	}
        	break;

		case DCR_CANCEL:
			if(hnd->res == RES_BUSY)
			{
				DMA_CHANNEL_DATA* ch_data;

				ch_data = drv_info->ch_data;
				if(hnd == ch_data->pending)
				{
					ch_data->pending = NULL;
					ch_data->last_mode = NULL;
					DMA_TRACELN("D:%u cancel", drv_info->ch_indx);
					if(stm32_dma_is_en(drv_info->hw_base, drv_info->ch_indx))
					{
						stm32_dma_stop(drv_info->hw_base, drv_info->ch_indx);
						TRACELN_INFO("D:%u req cancel", drv_info->ch_indx);
						hnd->error =0;
						ch_data->terminating = hnd;
					}else
					{
						svc_HND_SET_STATUS(hnd, RES_SIG_IDLE);
					}
				} else
				{
					hnd->svc_list_cancel(ch_data->waiting);
				}
			}

			break;

		case DCR_CLOSE:
			if(!--drv_info->ch_data->cnt)
    			NVIC_DisableIRQ(drv_info->info.drv_index);

			if(!--drv_info->drv_data->cnt)
				RCCPeripheralDisable(drv_info->info.peripheral_indx);
        	break;
   }
}

//*----------------------------------------------------------------------------
//*			DSR function
//*----------------------------------------------------------------------------
void DMA_DSR(DMA_DRIVER_INFO* drv_info, HANDLE hnd)
{
	DMA_DRIVER_MODE* mode;
	DMA_CHANNEL_DATA* ch_data;

	if(hnd->len)
	{
		ch_data = drv_info->ch_data;
		if(ch_data->pending || ch_data->terminating)
		{
	    	//the DMA channel is busy with other client
			DMA_TRACELN1("D:BUSY");
	    	hnd->list_add(ch_data->waiting);
		} else
		{
			mode = (DMA_DRIVER_MODE *)hnd->mode.as_voidptr;

			if(stm32_dma_is_en(drv_info->hw_base, drv_info->ch_indx))
			{
				// do nothing if already enabled
				DMA_TRACELN("D:%u Running!", drv_info->ch_indx);
			} else
			{
				if(mode != ch_data->last_mode)
				{
					ch_data->last_mode = mode;
					// configure the channel
					stm32_dma_ch_cfg(drv_info->hw_base, drv_info->ch_indx, mode);
				}

				// start transfer...
				/*
				 * AN4031 - Using the STM32F2, STM32F4 and STM32F7 SeriesDMA controller
				 * If the user enables the used peripheral before the corresponding DMA stream, a “FEIF”
				 * (FIFO Error Interrupt Flag) may be set due to the fact the DMA is not ready to provide the
				 * first required data to the peripheral (in case of memory-to-peripheral transfer).
				 */
				DMA_TRACELN("D:%u Start %u", drv_info->ch_indx, hnd->len);
				stm32_dma_start(drv_info->hw_base, drv_info->ch_indx, hnd);
			}
			hnd->res  = RES_BUSY;
			ch_data->pending = hnd;
			stm32_en_ints(drv_info->hw_base, drv_info->ch_indx, mode);

		}
	} else
	{
		// zero length... done!
		svc_HND_SET_STATUS(hnd, RES_SIG_OK);
	}

}

//*----------------------------------------------------------------------------
//*			ISR function
//*----------------------------------------------------------------------------
//*
//* Note DMA driver must have higher priority than any other driver using DMA
//*

void DMA_ISR(DMA_DRIVER_INFO* drv_info)
{
	DMA_CHANNEL_DATA* ch_data;
	HANDLE hnd;
	uint32_t status;
	uint32_t enabled;

	status =	stm32_get_ints(drv_info->hw_base, drv_info->ch_indx);
	enabled =	stm32_get_en_ints(drv_info->hw_base, drv_info->ch_indx);
	ch_data =	drv_info->ch_data;
	hnd =		ch_data->pending;
	if(hnd)
	{
		ch_data->pending = NULL;
		if(status & STM32_DMA_ERRORS) // these interrupts are always enabled
		{
			// FIFO, direct mode or transfer error(s)
			hnd->error = status;
			if(stm32_dma_is_en(drv_info->hw_base, drv_info->ch_indx))
			{
				// disable DMA and wait to complete
				stm32_dma_stop(drv_info->hw_base, drv_info->ch_indx);
				ch_data->terminating = hnd;
				return;
			}
			usr_HND_SET_STATUS(hnd, RES_SIG_ERROR);
		} else
		{
			if(status & STM32_DMA_COMPLETE) // always enabled
			{
				if(stm32_dma_is_en(drv_info->hw_base, drv_info->ch_indx))
				{
					TRACELN_ERROR("D:%u ??? S:%X", drv_info->ch_indx, status);
				}
				DMA_TRACELN("D:%u End %u", drv_info->ch_indx, stm32_dma_ndtr(drv_info->hw_base, drv_info->ch_indx));
				hnd->len = 0;
				usr_HND_SET_STATUS(hnd, RES_SIG_OK);
			} else
			{
				if((status & STM32_DMA_HALF) && (STM32_DMA_HALF_ENABLE & enabled))
				{
					hnd->len = stm32_dma_ndtr(drv_info->hw_base, drv_info->ch_indx);
					DMA_TRACELN("D:%u HALF %u", drv_info->ch_indx, hnd->len);
					usr_HND_SET_STATUS(hnd, RES_SIG_OK);
					return;	// leave it working...
				}
#ifdef	STM32_DMA_FIFO_ERR
				if(status & STM32_DMA_FIFO_ERR)
				{
					DMA_TRACELN("D:%u FIFO Err:%X N:%u",
							drv_info->ch_indx, status, stm32_dma_ndtr(drv_info->hw_base, drv_info->ch_indx));
					if(STM32_DMA_FIFO_ERR_ENABLE & enabled)
					{
						hnd->error = status;
						if(stm32_dma_is_en(drv_info->hw_base, drv_info->ch_indx))
						{
							// disable DMA and wait to complete
							stm32_dma_stop(drv_info->hw_base, drv_info->ch_indx);
							ch_data->terminating = hnd;
							return;
						}
						usr_HND_SET_STATUS(hnd, RES_SIG_ERROR);
					}
				}
#endif
			}
		}

		// start waiting...
		DMA_START_WAITING(drv_info);
	} else
	{
		hnd = ch_data->terminating;
		if(hnd)
		{
			ch_data->terminating = nullptr;
			if(hnd->error)
				usr_HND_SET_STATUS(ch_data->terminating, RES_SIG_ERROR);
		}
		if(!DMA_START_WAITING(drv_info))
			stm32_dis_ints(drv_info->hw_base, drv_info->ch_indx);
	}
}


/*
 * dma_drv.cpp
 *
 *  Created on: Nov 21, 2012
 *      Author: miro
 */

#include <tmos.h>
#include <dma_drv.h>

#define TRACE_DMA_CH(x)		(drv_info->ch_indx == x)

#if (TRACE_DMA_LEVEL >= TRACE_DEFAULT_LEVEL)
#define TRACE_DMA_CHANNEL  (TRACE_DMA_CH(1)) // || TRACE_DMA_CH(7))
#else
#define TRACE_DMA_CHANNEL	0
#endif

#define DMA_TRACE_CHAR(ch) 		do{if(TRACE_DMA_CHANNEL)TRACE_CHAR_LEVEL(TRACE_DMA_LEVEL, ch);}while(0)
#define DMA_TRACE(...) 			do{if(TRACE_DMA_CHANNEL)TRACE_LEVEL(TRACE_DMA_LEVEL, __VA_ARGS__);}while(0)
#define DMA_TRACE1(str)			do{if(TRACE_DMA_CHANNEL)TRACE1_LEVEL(TRACE_DMA_LEVEL, str);}while(0)
#define DMA_TRACELN(str, ...)	do{if(TRACE_DMA_CHANNEL){	\
								TRACE_LEVEL(TRACE_DMA_LEVEL, "\r\nDMA%c/%u ",((drv_info->hw_base == DMA2)?'2':'1'), drv_info->ch_indx);\
								TRACE_LEVEL(TRACE_DMA_LEVEL, str, ##__VA_ARGS__);}\
								}while(0)
#define DMA_TRACELN1(str)		do{if(TRACE_DMA_CHANNEL)TRACELN1_LEVEL(TRACE_DMA_LEVEL, str);}while(0)

#define SET_ERROR(err)	(err | (1<<(8 + drv_info->ch_indx +((drv_info->hw_base == DMA2)?8:0))))

extern 	 char* const DRV_TABLE[INALID_DRV_INDX+1];

volatile uint32_t DMA_DRIVER_DATA::dma_active_streams;

extern "C" uint32_t ON_DMA_START(DMA_DRIVER_INFO* drv_info)
						__attribute__ ((weak, alias ("DRV_ON_DMA_START")));
extern "C" uint32_t DRV_ON_DMA_START(DMA_DRIVER_INFO* drv_info)
{
	uint32_t tmp;
	uint32_t flag;

	flag = (1<<(drv_info->ch_indx));
	if(drv_info->hw_base == DMA2)
		flag <<=8;
	tmp =  DMA_DRIVER_DATA::dma_active_streams & flag;
	locked_set_int(&DMA_DRIVER_DATA::dma_active_streams, flag);
	flag &= ~tmp;
	if(flag)
	{
//		TRACE("{%04.4X|", DMA_DRIVER_DATA::dma_active_streams);
//		TRACE("{%04.4X|", flag);
	}
	return flag;
}

extern "C" uint32_t ON_DMA_END(DMA_DRIVER_INFO* drv_info)
						__attribute__ ((weak, alias ("DRV_ON_DMA_END")));
extern "C" uint32_t DRV_ON_DMA_END(DMA_DRIVER_INFO* drv_info)
{
	uint32_t flag;
	uint32_t tmp;

	flag = (1<<(drv_info->ch_indx));
	if(drv_info->hw_base == DMA2)
		flag <<=8;
	tmp = locked_clr_int(&DMA_DRIVER_DATA::dma_active_streams, flag);
	flag &= tmp;
	if(flag)
	{
//		TRACE("|%04.4X}", DMA_DRIVER_DATA::dma_active_streams);
//		TRACE("|%04.4X}", flag);
	}
	return flag;
}

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

bool DMA_RESUME(DMA_DRIVER_INFO* drv_info)
{
	DMA_CHANNEL_DATA* ch_data;
	HANDLE hnd;

	ch_data = drv_info->ch_data;

	if( ch_data->pending == nullptr && ch_data->stops_pending == nullptr
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
		ON_DMA_START(drv_info);
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
	return (ch_data->pending != nullptr);
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
			DMA_TRACELN("DCR cancel");
			if(hnd->res == RES_BUSY)
			{
				DMA_CHANNEL_DATA* ch_data;

				ch_data = drv_info->ch_data;
				if(hnd == ch_data->pending || hnd == ch_data->stops_pending)
				{
					DMA_TRACELN("cancel");
					stm32_dis_ints(drv_info->hw_base, drv_info->ch_indx);
					ch_data->pending = nullptr;
					ch_data->last_mode = nullptr;
					ch_data->stops_pending = nullptr;
					// The DMA transfer should have already finished, but just in case
					stm32_dma_stop(drv_info->hw_base, drv_info->ch_indx);
					if(stm32_dma_is_en(drv_info->hw_base, drv_info->ch_indx))
					{
						// DMA is running, wait for it to stop. This shouldn't happen
						while(stm32_dma_is_en(drv_info->hw_base, drv_info->ch_indx))
							TRACE_CHAR_ERROR('*');
					}
					ON_DMA_END(drv_info);
					// clear the interrupt flags, they should be STM32_DMA_COMPLETE and STM32_DMA_HALF due to the DMA stop
					stm32_get_ints(drv_info->hw_base, drv_info->ch_indx);
					//the transfer is complete and DMA is stopped,
					//but hnd is not signaled. In this case the interrupt must be pending to be processed
					if(NVIC_GetPendingIRQ(drv_info->info.drv_index))
					{
						// clear pending interrupt
						NVIC_ClearPendingIRQ(drv_info->info.drv_index);
					}

					RES_CODE res;
					if(hnd->error)
					{
						res = RES_SIG_ERROR;
					}
					else
					{
						if(hnd->len != stm32_dma_ndtr(drv_info->hw_base, drv_info->ch_indx))
						{
							//all or part of the data has been transferred
							if(stm32_dma_is_peripheral_ctrl(drv_info->hw_base, drv_info->ch_indx))
								hnd->len -= stm32_dma_ndtr(drv_info->hw_base, drv_info->ch_indx);
							else
								hnd->len = stm32_dma_ndtr(drv_info->hw_base, drv_info->ch_indx);
							res = RES_SIG_OK;
						}else
						{
							res = RES_SIG_CANCEL;
						}
					}
					DMA_TRACELN("signals the hnd (0x%X)", res);
					svc_HND_SET_STATUS(hnd, res);
					// start waiting...
					if(DMA_RESUME(drv_info))
						stm32_en_ints(drv_info->hw_base, drv_info->ch_indx, (DMA_DRIVER_MODE *)ch_data->pending->mode.as_cvoidptr);
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

	hnd->res  = RES_BUSY; // mark it as ready for processing

	if(hnd->len)
	{
		ch_data = drv_info->ch_data;
		if(ch_data->pending || ch_data->stops_pending)
		{
	    	//the DMA channel is busy with other client
			DMA_TRACELN("BUSY");
	    	hnd->list_add(ch_data->waiting);
		} else
		{
			mode = (DMA_DRIVER_MODE *)hnd->mode.as_voidptr;

			if(stm32_dma_is_en(drv_info->hw_base, drv_info->ch_indx))
			{
				// do nothing if already enabled
				DMA_TRACELN("Running!");
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
				DMA_TRACELN("Start (%u)",hnd->len);
				ON_DMA_START(drv_info);
				stm32_dma_start(drv_info->hw_base, drv_info->ch_indx, hnd);
			}
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
		ch_data->pending = nullptr;
		if(status & STM32_DMA_ERRORS) // these interrupts are always enabled
		{
			// FIFO, direct mode or transfer error(s)
			hnd->error = SET_ERROR(status);
			if(stm32_dma_is_en(drv_info->hw_base, drv_info->ch_indx))
			{
				// disable DMA and wait to complete
				stm32_dma_stop(drv_info->hw_base, drv_info->ch_indx);
				ch_data->stops_pending = hnd; // mark to be signaled
				return; // wait to complete
			}
			// just in case, must be not happen, except for misconfiguration
			usr_HND_SET_STATUS(hnd, RES_SIG_ERROR);
		} else
		{
			if(status & STM32_DMA_COMPLETE) // always enabled
			{
				if(!stm32_dma_is_en(drv_info->hw_base, drv_info->ch_indx))
				{
					ON_DMA_END(drv_info);
				}
				hnd->len = 0;
				DMA_TRACELN("complete l:%u", stm32_dma_ndtr(drv_info->hw_base, drv_info->ch_indx));
				usr_HND_SET_STATUS(hnd, RES_SIG_OK);
			} else
			{
				if((status & STM32_DMA_HALF) && (STM32_DMA_HALF_ENABLE & enabled))
				{
					hnd->len = stm32_dma_ndtr(drv_info->hw_base, drv_info->ch_indx);
					DMA_TRACELN("half l:%u", hnd->len);
					usr_HND_SET_STATUS(hnd, RES_SIG_OK);
					return;	// leave it working...
				}
#if	STM32_DMA_FIFO_ERR && STM32_DMA_FIFO_ERR_ENABLE
				else
				{
					if((status & STM32_DMA_FIFO_ERR) && (STM32_DMA_FIFO_ERR_ENABLE & enabled))
					{
						hnd->error = SET_ERROR(status);
						stm32_dma_stop(drv_info->hw_base, drv_info->ch_indx);
						ch_data->stops_pending = hnd; // mark to be signaled
						return; //wait to complete
					}
				}
#endif
			}
			// start waiting...
			// f both handles (pending and stops_pending) are null,
			// get the waiting handler and start it if there is one
			DMA_RESUME(drv_info);
		}
	} else
	{
		if((hnd = ch_data->stops_pending))
		{
			DMA_TRACELN("hnd ");
			DMA_TRACE1(((hnd->error)?"is signaled with an error":"cancel complete"));
			ch_data->stops_pending = nullptr;
			ON_DMA_END(drv_info);
			usr_HND_SET_STATUS(hnd, ((hnd->error)?RES_SIG_ERROR:RES_SIG_IDLE));
		}else
		{
			DMA_TRACELN("unexpected ISR(%X)", status);
		}
		// start waiting...
		// f both handles (pending and stops_pending) are null,
		// get the waiting handler and start it if there is one
		if(!DMA_RESUME(drv_info))
		{
			// at this point missing pending handler
			stm32_dis_ints(drv_info->hw_base, drv_info->ch_indx);
		}
	}
}


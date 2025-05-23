/*
 * spi_drv.cpp
 *
 *  Created on: Nov 20, 2012
 *      Author: miro
 */

#include <tmos.h>
#include <spi_drv.h>

static inline __attribute__((always_inline, optimize("Os")))
void SPI_TX(SPI_TypeDef* pSPI, HANDLE hnd)
{
#if STM32_SPI_TYPE == 124 // xx32F1, xx32F2 and xx32F4
	if(hnd->cmd & FLAG_WRITE)
	{
		pSPI->SPI_DR = *hnd->src.as_byteptr++;
	} else
	{
		pSPI->SPI_DR = 0xFF;
	}
#endif	// xx32F1, xx32F2 and xx32F4

#if STM32_SPI_TYPE == 03 // xx32F0... and xx32F3....
	bool packet;

	packet = ((hnd->len > 1 ) || (GET_SPI_CR2_DS(pSPI->SPI_CR2) > SPI_CR2_DS_8bit));

	if(packet){
		// 0: RXNE event is generated if the FIFO level is greater than or equal to 1/2 (16-bit)
		pSPI->SPI_CR2 &= ~SPI_CR2_FRXTH;
	}else{
		// 1: RXNE event is generated if the FIFO level is greater than or equal to 1/4 (8-bit)
		pSPI->SPI_CR2 |= SPI_CR2_FRXTH;
	}
	if(hnd->cmd & FLAG_WRITE)
	{
		if(packet)
		{
#if USE_UNALIGNED_ACCESS
			pSPI->SPI_DR_16bit = *hnd->src.as_shortptr++;
#else
			pSPI->SPI_DR_16bit = hnd->src.as_byteptr[0] + (hnd->src.as_byteptr[1] << 8);
			hnd->src.as_shortptr++;
#endif
		}else{
			pSPI->SPI_DR_8bit = *hnd->src.as_byteptr++;
		}
	} else
	{
		if(packet){
			pSPI->SPI_DR_16bit = 0xFFFF;
		}else{
			pSPI->SPI_DR_8bit = 0xFF;
		}
	}
#endif // xx32F0... and xx32F3...
}

static inline __attribute__((always_inline, optimize("Os")))
bool SPI_RX(SPI_TypeDef* pSPI, HANDLE hnd)
{
	uint32_t rx_data;
#if STM32_SPI_TYPE == 124 // xx32F1, xx32F2 and xx32F4
	rx_data = pSPI->SPI_DR;
	if(hnd)
	{
		if(hnd->cmd & FLAG_READ)
		{
			*hnd->dst.as_charptr++ = rx_data;
		}

		if(--hnd->len){
			// send next byte..
			SPI_TX(pSPI, hnd);
		}else{
			return true;
		}
	}
	return false;
#endif	// xx32F1, xx32F2 and xx32F4

#if STM32_SPI_TYPE == 03 // xx32F0... and xx32F3....
	bool DR_16bit = !(pSPI->SPI_CR2 & SPI_CR2_FRXTH);
	bool mode_16bit = ((GET_SPI_CR2_DS(pSPI->SPI_CR2) > SPI_CR2_DS_8bit));

	if (DR_16bit)
		rx_data = pSPI->SPI_DR_16bit;
	else
		rx_data = pSPI->SPI_DR_8bit;

	if (hnd)
	{
		if (hnd->cmd & FLAG_READ)
		{
			if (DR_16bit)
			{
#if USE_UNALIGNED_ACCESS
				*hnd->dst.as_shortptr++ = rx_data;
#else
				*hnd->dst.as_byteptr++ = rx_data;
				*hnd->dst.as_byteptr++ = rx_data >> 8;
#endif
			} else
				*hnd->dst.as_byteptr++ = rx_data;
		}

		if(!--hnd->len)
			return true;
		if (!mode_16bit && DR_16bit && !--hnd->len)
			return true;
		SPI_TX(pSPI, hnd);
	}
	return false;
#endif // xx32F0... and xx32F3...
}

// Enable SPI
static void SPI_ENABLE(SPI_DRIVER_INFO* drv_info)
{
	SPI_TypeDef*  pSPI;

	RCCPeripheralEnable(drv_info->info.peripheral_indx);

	PIO_Cfg_List(drv_info->spi_pins);

	pSPI = drv_info->hw_base;
    pSPI->SPI_CR2 = 0; //Reset value (interrupts disabled)
	drv_enable_isr(&drv_info->info);
}

// Start a new transaction
static void SPI_START_TRANSACTION(SPI_DRIVER_INFO* drv_info, SPI_DRIVER_MODE* mode)
{
	SPI_TypeDef* pSPI = drv_info->hw_base;

	//	1. Select the BR[2:0] bits to define the serial clock baud rate (see
	//		SPI_CR1 register).
	//	2. Select the CPOL and CPHA bits to define one of the four relationships
	//		between the	data transfer and the serial clock (see Figure 249).
	//		This step is not required when the TI mode is selected.
	//	3. Set the DFF bit to define 8- or 16-bit data frame format
	//	4. Configure the LSBFIRST bit in the SPI_CR1 register to define the frame
	//		format. This step is not required when the TI mode is selected.
	//	5. If the NSS pin is required in input mode, in hardware mode, connect
	//		the NSS pin to a high-level signal during the complete byte transmit
	//		sequence. In NSS software mode, set the SSM and SSI bits in the
	//		SPI_CR1 register. If the NSS pin is required in output	mode, the
	//		SSOE bit only should be set. This step is not required when the TI
	//		mode is	selected.

	uint32_t spi_cr1;

#if STM32_SPI_TYPE == 124 // xx32F1, xx32F2 and xx32F4
	/* SPI2 and SPI3 are connected to APB1, all others are connected to APB2 */
	/* BR[2:0] are set for APB1/2 at frequency 30/60 MHz */
	uint32_t src_clk;
	uint32_t desired_clock;
	if (drv_info->info.peripheral_indx == ID_PERIPH_SPI2
			|| drv_info->info.peripheral_indx == ID_PERIPH_SPI3)
	{
		src_clk = APB1_clock_frequency;
		desired_clock = (30000000 >> (((mode->spi_cr1 & SPI_CR1_BR) >> 3)+1));
	}else
	{
		src_clk = APB2_clock_frequency;
		desired_clock = (60000000 >> (((mode->spi_cr1 & SPI_CR1_BR) >> 3)+1));
	}
	for(spi_cr1=0; spi_cr1 < 7; spi_cr1++)
	{
		src_clk >>=1;
		if(desired_clock >= src_clk)
			break;
	}
	spi_cr1 = (mode->spi_cr1 & ~(SPI_CR1_BR | SPI_CR1_SPE)) | (spi_cr1 << 3);
#endif // xx32F1, xx32F2 and xx32F4

#if STM32_SPI_TYPE == 03 // xx32F0... and xx32F3....
	spi_cr1 = mode->spi_cr1 & (~SPI_CR1_SPE);
#endif // xx32F0... and xx32F3...
	pSPI->SPI_CR1 = spi_cr1;

	//	6. Set the FRF bit in SPI_CR2 to select the TI protocol for serial communications.
	//	7. The MSTR and SPE bits must be set (they remain set only if the NSS pin is connected
	//	  to a high-level signal).

#if USE_SPI_DMA_DRIVER
	if(drv_info->rx_dma_mode.dma_index >= INALID_DRV_INDX) // DMA is not used to receive data
#endif
	{
		pSPI->SPI_CR2 |=  mode->spi_cr2 | SPI_CR2_RXNEIE;
	}
    //enable
	if(!(mode->spi_cr1 & SPI_CR1_SPE)) // if set SPI_CR1_SPE disable SPI
	{
		pSPI->SPI_CR1 = spi_cr1 | SPI_CR1_SPE;
	}

    // Assert CS
    PIO_Assert(mode->cs_pin);
}

static void SPI_END_TRANSACTION(SPI_DRIVER_MODE* mode)
{
	PIO_Deassert(mode->cs_pin);
}

static void START_HND(SPI_DRIVER_INFO* drv_info, SPI_DRIVER_DATA* drv_data, HANDLE hnd)
{
#if USE_SPI_DMA_DRIVER
	void *ptr;
	if(drv_info->rx_dma_mode.dma_index < INALID_DRV_INDX )
	{
		if(hnd->cmd & FLAG_READ)
		{
			ptr = hnd->dst.as_voidptr;
		} else
		{
			ptr = NULL;
		}
		drv_data->rx_dma_hnd.drv_read_write(ptr, (void *)&drv_info->hw_base->SPI_DR, hnd->len);
		drv_info->hw_base->SPI_CR2 |= SPI_CR2_RXDMAEN | SPI_CR2_ERRIE;
	}
	if(drv_info->tx_dma_mode.dma_index < INALID_DRV_INDX )
	{
		if(hnd->cmd & FLAG_WRITE)
		{
			ptr = hnd->src.as_voidptr;
		} else
		{
			ptr = (void*) 0xff; //fill with ff...
		}
		drv_data->tx_dma_hnd.drv_read_write((void *)&drv_info->hw_base->SPI_DR, ptr, hnd->len);
		drv_info->hw_base->SPI_CR2 |= SPI_CR2_TXDMAEN;
	} else
#endif
	{
		//no DMA
		SPI_TX(drv_info->hw_base, hnd);
	}
}

static void SPI_RESUME(SPI_DRIVER_INFO* drv_info, SPI_DRIVER_DATA* drv_data)
{
	HANDLE hnd;

	hnd = drv_data->pending;
	if( !hnd )
	{
		// no pending try with waiting...
		hnd = drv_data->waiting;

		if(drv_data->locker)
		{
			//search for the same client...
			while(hnd)
			{
				if(hnd->client.task == drv_data->locker)
					break;
				hnd = hnd->next;
			}
		}

		if( hnd )
		{
			// make it pending
			hnd->list_remove(drv_data->waiting);
			drv_data->pending = hnd;
			hnd->next = NULL;
		}
	}
	if(hnd)
	{
		if(!drv_data->locker)
		{
			SPI_START_TRANSACTION(drv_info, (SPI_DRIVER_MODE *)hnd->mode.as_voidptr);
			if( hnd->cmd & FLAG_LOCK)
				drv_data->locker = hnd->client.task;
		}
		START_HND(drv_info, drv_data, hnd);
	}
}
//*----------------------------------------------------------------------------
//*			DCR function
//*----------------------------------------------------------------------------
void SPI_DCR(SPI_DRIVER_INFO* drv_info, unsigned int reason, HANDLE hnd)
{
    SPI_DRIVER_DATA* drv_data = drv_info->drv_data;

	switch(reason)
    {
        case DCR_RESET:
			RCCPeripheralReset(drv_info->info.peripheral_indx);
        	RCCPeripheralDisable(drv_info->info.peripheral_indx);
#if USE_SPI_DMA_DRIVER
        	drv_data->rx_dma_hnd.client.drv_index = drv_info->info.drv_index;
        	drv_data->tx_dma_hnd.client.drv_index = drv_info->info.drv_index;
#endif
        	break;

        case DCR_OPEN:

        	SPI_DRIVER_MODE * mode;
        	mode = (SPI_DRIVER_MODE *)hnd->mode.as_voidptr;
        	if(mode)
        	{
        		if(!drv_data->cnt)
        		{
#if USE_SPI_DMA_DRIVER
        			if(drv_info->rx_dma_mode.dma_index < INALID_DRV_INDX &&
        					!drv_data->rx_dma_hnd.drv_open(
        					drv_info->rx_dma_mode.dma_index,
        					&drv_info->rx_dma_mode))
        				break;
        			if(drv_info->tx_dma_mode.dma_index < INALID_DRV_INDX &&
        					!drv_data->tx_dma_hnd.drv_open(
        					drv_info->tx_dma_mode.dma_index,
        					&drv_info->tx_dma_mode))
        				break;
#endif
        			SPI_ENABLE(drv_info);
        		}
    			drv_data->cnt++;
        		PIO_Cfg(mode->cs_pin);
				hnd->res = RES_OK;
        	}
        	break;

		case DCR_CANCEL:
			break;

		case DCR_CLOSE:
			if(!--drv_data->cnt)
			{
				RCCPeripheralDisable(drv_info->info.peripheral_indx);
#if USE_SPI_DMA_DRIVER
				drv_data->rx_dma_hnd.close();
				drv_data->tx_dma_hnd.close();
#endif
			}
        	break;

#if USE_SPI_DMA_DRIVER
		case DCR_SIGNAL:
			//signal rx/dma complete
			if(hnd == &drv_data->rx_dma_hnd)
			{
				//process rx dma signal only (data shifted out),
				//stopping DMA requests from the peripheral
				drv_info->hw_base->SPI_CR2 &= ~(SPI_CR2_RXDMAEN | SPI_CR2_TXDMAEN | SPI_CR2_ERRIE);

				hnd = drv_data->pending;
				if(hnd)
				{
					if(drv_data->rx_dma_hnd.res != RES_SIG_OK)
					{
						//but the transfer may not be complete if an error occurs.
						//So make sure tx dma completed
						if(drv_info->tx_dma_mode.dma_index < INALID_DRV_INDX)
						{
							if(drv_data->tx_dma_hnd.res == RES_BUSY)
								drv_data->tx_dma_hnd.hcontrol(DCR_CANCEL);
						}
					}
					//pending is done
					if(!drv_data->locker)
					{
						SPI_END_TRANSACTION((SPI_DRIVER_MODE *)hnd->mode.as_voidptr);
					}
					drv_data->pending = hnd->next;

					svc_HND_SET_STATUS(hnd, drv_data->rx_dma_hnd.res);

					//resume waiting
					SPI_RESUME(drv_info, drv_data);
				}
			}
			break;
#endif


        case DCR_PARAMS:
        	//send handle's client to release the lock
        	if(hnd == (HANDLE)drv_data->locker)
        	{
        		//unlock
        		drv_data->locker = NULL;

        		//start waiting
        		hnd = drv_data->waiting;
        		if(hnd)
        		{
        			drv_data->waiting = hnd->next;
        			SPI_DSR(drv_info, hnd);
        		}
        	}
        	break;

   }
}

//*----------------------------------------------------------------------------
//*			DSR function
//*----------------------------------------------------------------------------
void SPI_DSR(SPI_DRIVER_INFO* drv_info, HANDLE hnd)
{
	Task* locker;
    SPI_DRIVER_DATA* drv_data = drv_info->drv_data;

    if((locker = drv_data->locker))
	{
        //the SPI is locked
        if(locker != hnd->client.task)
        {
        	//the SPI is locked from other client
        	hnd->list_add(drv_data->waiting);
        	return;
        }

		if( !(hnd->cmd & FLAG_LOCK) )
			drv_data->locker = 0;

		if(	drv_data->pending)
		{
			hnd->list_add(drv_data->pending);
			return;
		}

		//start hnd
	} else
    {
		//the SPI is not locked
		if(	drv_data->pending)
		{
			//but is busy
			hnd->list_add(drv_data->waiting);
			return;
		}

		//the SPI is idle so start a new transaction
		SPI_START_TRANSACTION(drv_info, (SPI_DRIVER_MODE*)hnd->mode.as_voidptr);
		if( hnd->cmd & FLAG_LOCK)
			drv_data->locker = hnd->client.task;
    }



	if(hnd->len)
	{
		hnd->next = NULL;
		hnd->res  = RES_BUSY;
		drv_data->pending = hnd;

		START_HND(drv_info, drv_data, hnd);

	} else
	{
		if(!drv_data->locker)
		{
			SPI_END_TRANSACTION((SPI_DRIVER_MODE *)hnd->mode.as_voidptr);
			//resume waiting
			SPI_RESUME(drv_info, drv_data);
		}

		svc_HND_SET_STATUS(hnd, RES_SIG_OK);
	}
}

//*----------------------------------------------------------------------------
//*			ISR function
//*----------------------------------------------------------------------------
void SPI_ISR(SPI_DRIVER_INFO* drv_info)
{
	SPI_TypeDef* pSPI = drv_info->hw_base;
    SPI_DRIVER_DATA* drv_data ;
	unsigned int status, cr1;

	cr1 = pSPI->SPI_CR1;
	status = pSPI->SPI_SR;
	drv_data = drv_info->drv_data;

	if(status & SPI_SR_OVR)
	{
		//this should never happen but just in case...
		// Clearing the OVR bit is done by a read operation on the SPI_DR register
		// followed by a read access to the SPI_SR register
#if USE_SPI_DMA_DRIVER
		drv_info->hw_base->SPI_CR2 &= ~ SPI_CR2_ERRIE;
		if(drv_info->rx_dma_mode.dma_index < INALID_DRV_INDX)
		{
			if(drv_data->rx_dma_hnd.res == RES_BUSY)
			{
				drv_data->rx_dma_hnd.error = status;
				drv_data->rx_dma_hnd.hcontrol(DCR_CANCEL);
			}else
			{
				TRACE_ERROR("\r\nSPI:Err not processed");
			}
		}
#endif
		pSPI->SPI_DR;
		status |= pSPI->SPI_SR;
	}

	if(status & SPI_SR_MODF)
	{
//		Use the following software sequence to clear the MODF bit:
//		1. Make a read or write access to the SPI_SR register while the MODF bit is set.
//		2. Then write to the SPI_CR1 register.
		pSPI->SPI_CR1 = cr1;
	}


	if(status & SPI_SR_RXNE)
	{
		HANDLE hnd = drv_data->pending;
		if(SPI_RX(pSPI, hnd))
		{
			//done
			if(!drv_data->locker)
			{
				SPI_END_TRANSACTION((SPI_DRIVER_MODE *)hnd->mode.as_voidptr);
			}

			drv_data->pending = hnd->next;
			usr_HND_SET_STATUS(hnd, RES_SIG_OK);

			SPI_RESUME(drv_info, drv_data);
		}
/*
		status = pSPI->SPI_DR;
		hnd = drv_data->pending;
		if(hnd)
		{
			if(hnd->cmd & FLAG_READ)
			{
				*hnd->dst.as_charptr++ = status;
			}

			if(--hnd->len)
			{
				// send next byte..
				SPI_TX(pSPI, hnd);
			} else
			{
				//done
				if(!drv_data->locker)
				{
					SPI_END_TRANSACTION((SPI_DRIVER_MODE *)hnd->mode.as_voidptr);
				}

				drv_data->pending = hnd->next;
				usr_HND_SET_STATUS(hnd, RES_SIG_OK);

				SPI_RESUME(drv_info, drv_data);
			}
		}
*/
	}
}

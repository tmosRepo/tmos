/*
 * sdio_drv.cpp
 *
 *  Created on: Oct 3, 2013
 *      Author: miro
 */

#include <tmos.h>
#include <sdio_drv.h>

#ifndef DEBUG_SDIO_DRV
#define DEBUG_SDIO_DRV 0
#endif

#if USE_SDIO_MULTIPLE_SLOTS
WEAK void SDIO_SELECT_SLOT(SDIO_INFO drv_info, HANDLE hnd)
{
#if DEBUG_SDIO_DRV
	TRACELN1("SDIO SLOT SELECT not implemented!");
#endif
}
WEAK void SDIO_DESELECT_SLOT(SDIO_INFO drv_info, HANDLE hnd)
{
#if DEBUG_SDIO_DRV
	TRACELN1("SDIO SLOT DESELECT not implemented!");
#endif
}
#endif

WEAK void SDIO_POWERON_SLOT(SDIO_INFO drv_info, HANDLE hnd)
{
	const SDIO_DRIVER_MODE* mode = (const SDIO_DRIVER_MODE*)hnd->mode.as_cvoidptr;

	if(mode && mode->sdio_pwr_en)
	{
		PIO_Cfg(mode->sdio_pwr_en);
		PIO_Assert(mode->sdio_pwr_en);
	}
}

WEAK void SDIO_POWEROFF_SLOT(SDIO_INFO drv_info, HANDLE hnd)
{
	const SDIO_DRIVER_MODE* mode = (const SDIO_DRIVER_MODE*)hnd->mode.as_cvoidptr;

#if USE_SDIO_MULTIPLE_SLOTS
	SDIO_DESELECT_SLOT(drv_info, hnd);
#endif
	if(mode && mode->sdio_pwr_en)
	{
			PIO_Deassert(mode->sdio_pwr_en);
			const PIN_DESC* pins = drv_info->sdio_pins;
			while(*pins)
			{
				PIO_CfgOutput0(*pins);
				pins++;
			}
	}
}

static void ResetSDIO(SDIO_INFO drv_info)
{
#if USE_SDIO_DMA_DRIVER
	drv_info->drv_data->rx_dma_hnd.close();
	drv_info->drv_data->tx_dma_hnd.close();
#endif
	NVIC_DisableIRQ(drv_info->info.drv_index);
	RCCPeripheralReset(drv_info->info.peripheral_indx);
	RCCPeripheralDisable(drv_info->info.peripheral_indx);

}

static bool ConfigureSDIO(SDIO_INFO drv_info, HANDLE hnd)
{
	SDIO_TypeDef* hw_base = drv_info->hw_base;

	RCCPeripheralEnable(drv_info->info.peripheral_indx);
	RCCPeripheralReset(drv_info->info.peripheral_indx);

	PIO_Cfg_List(drv_info->sdio_pins);

	SDIO_POWERON_SLOT(drv_info, hnd);

	hw_base->SDIO_CLKCR  = SDIO_CLKCR_WIDBUS_1b | SDIO_CLKCR_PWRSAV | SDIO_CLKCR_CLKEN |
			SDIO_CLKCR_CLKDIV_Set(48000/400 -2);
	hw_base->SDIO_POWER = SDIO_POWER_PWRCTRL_on;

#if USE_SDIO_DMA_DRIVER
	unsigned int tx_flags, rx_flags;

	if(drv_info->rx_dma_mode.dma_index < INALID_DRV_INDX)
		rx_flags = 0;
	else
		rx_flags = SDIO_STA_RX_FLAGS;

	if(drv_info->tx_dma_mode.dma_index < INALID_DRV_INDX)
		tx_flags = 0;
	else
		tx_flags = SDIO_STA_TX_FLAGS;
	hw_base->SDIO_MASK = SDIO_STA_DONE_FLAGS | SDIO_STA_ERROR_FLAGS
			| SDIO_STA_CCRCFAIL | rx_flags | tx_flags;

	if(!drv_info->drv_data->rx_dma_hnd.drv_open(drv_info->rx_dma_mode.dma_index,
			&drv_info->rx_dma_mode))
		return false;
	if(!drv_info->drv_data->tx_dma_hnd.drv_open(drv_info->tx_dma_mode.dma_index,
			&drv_info->tx_dma_mode))
		return false;
#else
	hw_base->SDIO_MASK = SDIO_STA_DONE_FLAGS | SDIO_STA_ERROR_FLAGS
			| SDIO_STA_CCRCFAIL | SDIO_STA_RX_FLAGS | SDIO_STA_TX_FLAGS;
#endif
	hw_base->SDIO_DTIMER = 0xFFFFFFFF;

	drv_enable_isr(&drv_info->info);

	return true;
}

static void sdio_stop_transfer(SDIO_DRIVER_DATA *drv_data, SDIO_TypeDef* hw_base)
{
	// stop DMA
#if USE_SDIO_DMA_DRIVER
	if(drv_data->rx_dma_hnd.res & FLG_BUSY)
		drv_data->rx_dma_hnd.hcontrol(DCR_CANCEL);
	if(drv_data->tx_dma_hnd.res & FLG_BUSY)
		drv_data->tx_dma_hnd.hcontrol(DCR_CANCEL);
#endif

	// stop data state machine
	hw_base->SDIO_DLEN = 0;
	hw_base->SDIO_DCTRL &= ~(SDIO_DCTRL_DTEN | SDIO_DCTRL_DMAEN);
#if DEBUG_SDIO_DRV
	TRACELN("!SD stop %x", hw_base->SDIO_STA);
#endif
}

#define IS_SD_HIGH_CAPACITY(x) 1// should return 1 for SDHC and SDXC cards

static RES_CODE SDIO_START_HND(SDIO_INFO drv_info, HANDLE hnd, SDIO_DRIVER_DATA *drv_data)
{
	SDIO_TypeDef* hw_base = drv_info->hw_base;
	RES_CODE res = RES_IDLE;

#if USE_SDIO_DMA_DRIVER
	//stop RX DMA
	if(drv_data->rx_dma_hnd.res & FLG_BUSY)
		drv_data->rx_dma_hnd.hcontrol(DCR_CANCEL);
#endif

#if USE_SDIO_MULTIPLE_SLOTS
	if(hnd != drv_data->last_slot)
	{
		if(drv_data->last_slot)
		{
			SDIO_DESELECT_SLOT(drv_info, drv_data->last_slot);
		} else
		{
			ResetSDIO(drv_info);
			ConfigureSDIO(drv_info, hnd);
		}
		//change clock
		hw_base->SDIO_CLKCR  = (hw_base->SDIO_CLKCR & ~(SDIO_CLKCR_CLKDIV|SDIO_CLKCR_WIDBUS)) |
				(SDIO_CLKCR_CLKDIV_Set(hnd->mode0) |((hnd->mode1)<<8));

		SDIO_SELECT_SLOT(drv_info, hnd);
		drv_data->last_slot = hnd;
	}
#endif

	if( (hnd->cmd & FLAG_READ) && hnd->len)
	{
#if DEBUG_SDIO_DRV
		TRACE("[r %u]", hnd->len);
#endif
		//read block or multiple block command
		hw_base->SDIO_DLEN = hnd->len;
		hw_base->SDIO_DCTRL = SDIO_DCTRL_DBLOCKSIZE_512b | SDIO_DCTRL_DTDIR
				| SDIO_DCTRL_DTEN | SDIO_DCTRL_DMAEN;

		hw_base->SDIO_ARG = hnd->src.as_int;
		hw_base->SDIO_CMD = SD_CMD17_READ_SINGLE_BLOCK | SDIO_CMD_WAITRESP_short | SDIO_CMD_CPSMEN;
		//wait for cmdend (R1) or error
		drv_data->sdio_op = SDIO_OP_READ | SDIO_OP_R1;
	} else
	{
		if( (hnd->cmd & FLAG_WRITE) && hnd->len)
		{
#if DEBUG_SDIO_DRV
			TRACE("[w %u]", hnd->len);
#endif
			//write block or multiple block command
			hw_base->SDIO_ARG = hnd->dst.as_int;
			hw_base->SDIO_CMD = SD_CMD24_WRITE_BLOCK | SDIO_CMD_WAITRESP_short | SDIO_CMD_CPSMEN;

			//wait for cmdend (R1) or error
			drv_data->sdio_op = SDIO_OP_WRITE | SDIO_OP_R1;
		} else
		{
			if( hnd->cmd & FLAG_COMMAND)
			{
				unsigned int cmd, cmd_indx;

				cmd = hnd->src.as_intptr[0];
				cmd_indx = cmd & 0x3F;
#if DEBUG_SDIO_DRV
				TRACE("[c %u]", cmd_indx);
#endif
				if( cmd_indx == SD_CMD42_LOCK_UNLOCK && hnd->len)
				{
					hw_base->SDIO_ARG =  hnd->src.as_intptr[1];
					hw_base->SDIO_CMD = cmd;
					//wait for cmdend (R1) or error
					drv_data->sdio_op = SDIO_OP_CMD | SDIO_OP_R1;
				}
				else
				{
					if( (hnd->len > 4) && !(cmd & SDIO_CMD_WAITRESP_no1) )
					{
						// Commands like ACMD51 (read SCR), CMD6 (SWITCH func) ->R1 + read
						hw_base->SDIO_DLEN = hnd->len;
						switch(hnd->len)
						{
						case 8:
							hw_base->SDIO_DCTRL = SDIO_DCTRL_DBLOCKSIZE_8b | SDIO_DCTRL_DTDIR
									| SDIO_DCTRL_DTEN | SDIO_DCTRL_DMAEN;
							break;
						case 16:
							hw_base->SDIO_DCTRL = SDIO_DCTRL_DBLOCKSIZE_16b | SDIO_DCTRL_DTDIR
									| SDIO_DCTRL_DTEN | SDIO_DCTRL_DMAEN;
							break;
						case 32:
							hw_base->SDIO_DCTRL = SDIO_DCTRL_DBLOCKSIZE_32b | SDIO_DCTRL_DTDIR
									| SDIO_DCTRL_DTEN | SDIO_DCTRL_DMAEN;
							break;
						case 64:
							hw_base->SDIO_DCTRL = SDIO_DCTRL_DBLOCKSIZE_64b | SDIO_DCTRL_DTDIR
									| SDIO_DCTRL_DTEN | SDIO_DCTRL_DMAEN;
							break;
						case 128:
							hw_base->SDIO_DCTRL = SDIO_DCTRL_DBLOCKSIZE_128b | SDIO_DCTRL_DTDIR
									| SDIO_DCTRL_DTEN | SDIO_DCTRL_DMAEN;
							break;
						case 256:
							hw_base->SDIO_DCTRL = SDIO_DCTRL_DBLOCKSIZE_256b | SDIO_DCTRL_DTDIR
									| SDIO_DCTRL_DTEN | SDIO_DCTRL_DMAEN;
							break;
						default: //512
							hw_base->SDIO_DCTRL = SDIO_DCTRL_DBLOCKSIZE_512b | SDIO_DCTRL_DTDIR
									| SDIO_DCTRL_DTEN | SDIO_DCTRL_DMAEN;
							break;
						}
						drv_data->sdio_op = SDIO_OP_READ | SDIO_OP_R1;
					} else
					{
						// R3 response for ACMD41 and CMD58 (CMD51 and ACMD58 are reserved)
						if( (cmd_indx==41) || (cmd_indx==58))
							drv_data->sdio_op = SDIO_OP_CMD | SDIO_OP_R3;
						else
							drv_data->sdio_op = SDIO_OP_CMD;
					}
					hw_base->SDIO_ARG = hnd->src.as_intptr[1];
					hw_base->SDIO_CMD = cmd;
				}
				//wait for something..
			} else
			{
				res = RES_SIG_ERROR;
				drv_data->sdio_op = SDIO_OP_IDLE;
			}

		}
	}
#if USE_SDIO_DMA_DRIVER

	if(drv_info->rx_dma_mode.dma_index >= INALID_DRV_INDX)
		hw_base->SDIO_MASK |= SDIO_STA_RX_FLAGS;

	if(drv_info->tx_dma_mode.dma_index >= INALID_DRV_INDX)
		hw_base->SDIO_MASK |= SDIO_STA_TX_FLAGS;
#else
	hw_base->SDIO_MASK |= SDIO_STA_TR_FLAGS;
#endif

	return res;
}

static void SDIO_CMD_HND(SDIO_INFO drv_info, HANDLE hnd, SDIO_DRIVER_DATA *drv_data)
{
	SDIO_TypeDef* hw_base = drv_info->hw_base;

	if(drv_data->sdio_op & SDIO_OP_READ)
	{
#if USE_SDIO_DMA_DRIVER
		if(drv_info->rx_dma_mode.dma_index < INALID_DRV_INDX)
		{
			drv_data->rx_dma_hnd.drv_read_write(hnd->dst.as_voidptr, (void*)&hw_base->SDIO_FIFO, hnd->len/4);
		} else
#endif
		{
			//read without DMA
			while((hw_base->SDIO_STA & SDIO_STA_RXFIFOHF) && hnd->len > 31)
			{
				for(int i=0; i<8; i++)
					*hnd->dst.as_intptr++ = hw_base->SDIO_FIFO;
				hnd->len -= 32;
			}
			while(!(hw_base->SDIO_STA & SDIO_STA_RXFIFOE) && hnd->len > 3)
			{
				*hnd->dst.as_intptr++ = hw_base->SDIO_FIFO;
				hnd->len -= 4;
			}
//			if(hnd->len <= 3)
//			{
//				usr_HND_SET_STATUS(hnd, RES_SIG_OK);
//				drv_data->pending = NULL;
//			}
		}
	} else
	{
		if(drv_data->sdio_op & SDIO_OP_WRITE)
		{
#if USE_SDIO_DMA_DRIVER
			if(drv_info->tx_dma_mode.dma_index < INALID_DRV_INDX)
			{
				drv_data->tx_dma_hnd.drv_read_write((void *)&drv_info->hw_base->SDIO_FIFO, hnd->src.as_voidptr, hnd->len/4);
			} else
#endif
			{
				//write without DMA
				while((hw_base->SDIO_STA & SDIO_STA_TXFIFOHE) && hnd->len > 31)
				{
					for(int i=0; i<8; i++)
						hw_base->SDIO_FIFO = *hnd->src.as_intptr++ ;
					hnd->len -= 32;
				}
				while(!(hw_base->SDIO_STA & SDIO_STA_TXFIFOF) && hnd->len > 3)
				{
					hw_base->SDIO_FIFO = *hnd->src.as_intptr++;
					hnd->len -= 4;
				}
//				if(hnd->len <= 3)
//				{
//					usr_HND_SET_STATUS(hnd, RES_SIG_OK);
//					drv_data->pending = NULL;
//				}
			}
		} else
		{
			unsigned int len = hnd->len /4;

			if (len > 4)
				len = 4;
			while(len--)
			{
				hnd->dst.as_intptr[len] = hw_base->SDIO_RESPx[len];
			}
			usr_HND_SET_STATUS(hnd, RES_SIG_OK);
			drv_data->pending = NULL;
		}
	}
}
//*----------------------------------------------------------------------------
//*			DCR function
//*----------------------------------------------------------------------------
void SDIO_DCR(SDIO_INFO drv_info, unsigned int reason, HANDLE hnd)
{
	SDIO_DRIVER_DATA *drv_data;

	drv_data = drv_info->drv_data;
	switch(reason)
	{
		case DCR_RESET:
			if(hnd)	// software power reset
			{
				drv_data->is_resetting = true; //!< The caller must reset it !
				if(drv_data->cnt)
				{	drv_data->locker = nullptr;
					if(drv_data->pending)
						svc_HND_SET_STATUS(drv_data->pending, RES_FATAL|FLG_SIGNALED);
					while(drv_data->waiting)
					{
						svc_HND_SET_STATUS(drv_data->waiting, RES_FATAL|FLG_SIGNALED);
						drv_data->waiting = drv_data->waiting->next;
					}
					ResetSDIO(drv_info);
				}else
				{
					RCCPeripheralReset(drv_info->info.peripheral_indx);
					RCCPeripheralDisable(drv_info->info.peripheral_indx); // ??? turn off
				}
				const SDIO_DRIVER_MODE* mode = (const SDIO_DRIVER_MODE*)hnd->mode.as_cvoidptr;
				if(mode && mode->sdio_pwr_en)
					PIO_Cfg(mode->sdio_pwr_en); // It must be sure that pwr_en is configured (by default the power is turn off)
				SDIO_POWEROFF_SLOT(drv_info, hnd);
				break;
			}
			RCCPeripheralReset(drv_info->info.peripheral_indx);
			RCCPeripheralDisable(drv_info->info.peripheral_indx); // ??? turn off
			break;

		case DCR_OPEN:
		{
			if(drv_data->is_resetting)
			{
				hnd->res = RES_FATAL;
				break;
			}
#if USE_SDIO_MULTIPLE_SLOTS
			hnd->mode0 = 48000/400 -2; 	//default clock 400Khz
			hnd->mode1 = 0;				//default bus mode: SDIO_D0 used
#endif
			if(!drv_data->cnt)
			{
				if(!ConfigureSDIO(drv_info, hnd))
					break;
#if USE_SDIO_MULTIPLE_SLOTS
				if(drv_data->last_slot == nullptr)
				{
					SDIO_SELECT_SLOT(drv_info, hnd);
//					drv_data->last_slot = hnd;
				}
#endif
			}
			drv_data->cnt++;
			hnd->res = RES_OK;
			break;
		}

		case DCR_CLOSE:
		{
			if(drv_data->cnt)
				drv_data->cnt--;
			if(!drv_data->cnt)
			{
				ResetSDIO(drv_info);
			}
			SDIO_POWEROFF_SLOT(drv_info, hnd);
#if USE_SDIO_MULTIPLE_SLOTS
			drv_data->last_slot = nullptr;
#endif
			break;
		}

		case DCR_CLOCK:
		{
			//patch for low speed cards
			const SDIO_DRIVER_MODE* mode = (const SDIO_DRIVER_MODE*)hnd->mode.as_cvoidptr;
			if(mode)
			{
#if USE_SDIO_MULTIPLE_SLOTS
				if(drv_data->last_slot == hnd)
				{
					hnd->mode0 = mode->sdio_clk_div;
					drv_info->hw_base->SDIO_CLKCR  = (drv_info->hw_base->SDIO_CLKCR &
						~(SDIO_CLKCR_CLKDIV|SDIO_CLKCR_BYPASS))
						| SDIO_CLKCR_CLKDIV_Set(mode->sdio_clk_div)|(mode->sdio_clk_div & SDIO_CLKCR_BYPASS);
				}
#else
				drv_info->hw_base->SDIO_CLKCR  = (drv_info->hw_base->SDIO_CLKCR &
					~(SDIO_CLKCR_CLKDIV|SDIO_CLKCR_BYPASS))
					| SDIO_CLKCR_CLKDIV_Set(mode->sdio_clk_div)|(mode->sdio_clk_div & SDIO_CLKCR_BYPASS);
#endif
			}
			break;
		}

		case DCR_SIGNAL:
			//signal rx/dma complete
			// do nothing, use data end irq instead
			break;

		case DCR_CANCEL:
			if(hnd->res == RES_BUSY)
			{
				if(!hnd->svc_list_cancel(drv_data->waiting))
					if(hnd->svc_list_cancel(drv_data->pending))
					{
#if USE_SDIO_DMA_DRIVER
						if(drv_data->rx_dma_hnd.res & FLG_BUSY)
							drv_data->rx_dma_hnd.hcontrol(DCR_CANCEL);
						if(drv_data->tx_dma_hnd.res & FLG_BUSY)
							drv_data->tx_dma_hnd.hcontrol(DCR_CANCEL);
#endif
#if DEBUG_SDIO_DRV
						TRACELN1("SDIO cancel!");
#endif
					}
			}
			break;

	}
}

//*----------------------------------------------------------------------------
//*			DSR function
//*----------------------------------------------------------------------------
void SDIO_DSR(SDIO_INFO drv_info, HANDLE hnd)
{
	SDIO_DRIVER_DATA *drv_data = drv_info->drv_data;

	if(drv_data->is_resetting)
	{
		svc_HND_SET_STATUS(hnd, RES_FATAL|FLG_SIGNALED);
		return;
	}
	if(drv_data->pending || (drv_data->locker && hnd != drv_data->locker))
	{
    	//the driver is busy || locked from other client
    	hnd->list_add(drv_data->waiting);
	} else
	{
		if( hnd->cmd & FLAG_LOCK)
			drv_data->locker = hnd;
		else
			drv_data->locker = NULL;

		RES_CODE res;

		res = SDIO_START_HND(drv_info, hnd, drv_data);
		if(res & FLG_SIGNALED)
		{
			svc_HND_SET_STATUS(hnd, res);
		} else
		{
			drv_data->pending = hnd;
			hnd->next = NULL;
			hnd->res  = RES_BUSY;
		}
	}
}

//*----------------------------------------------------------------------------
//*			ISR function
//*----------------------------------------------------------------------------
void SDIO_ISR(SDIO_INFO drv_info)
{
	SDIO_DRIVER_DATA *drv_data = drv_info->drv_data;
	SDIO_TypeDef* hw_base = drv_info->hw_base;
	HANDLE hnd;
	unsigned int status;

	status = hw_base->SDIO_STA;
	status &= hw_base->SDIO_MASK;
	hw_base->SDIO_ICR = status;
#if DEBUG_SDIO_DRV
	TRACE("{i %x}", status);
#endif
	if((hnd = drv_data->pending))
	{
		if( (status & SDIO_STA_CCRCFAIL) && (drv_data->sdio_op & SDIO_OP_R3))
		{
			// Response R3 -> ignore CRC
			status &= ~SDIO_STA_CCRCFAIL;
			status |= SDIO_STA_CMDREND;
		}
		if(status & SDIO_STA_ERROR_FLAGS)
		{
			// error -> done!
			hnd->error = status;
			usr_HND_SET_STATUS(hnd, RES_SIG_ERROR);
			drv_data->pending = NULL;
			sdio_stop_transfer(drv_data, hw_base);
		} else
		{
			if(status & SDIO_STA_DONE_TR)
			{
				// done...
				if( (drv_data->sdio_op & SDIO_OP_READ) && (status & SDIO_STA_RX_FLAGS))
				{
					SDIO_CMD_HND(drv_info, hnd, drv_data);
				}
				usr_HND_SET_STATUS(hnd, RES_SIG_OK);
				drv_data->pending = NULL;
			} else
			{
				if(status & SDIO_STA_DONE_CMD)
				{
					//command end
					if((drv_data->sdio_op & SDIO_OP_R1) && (hw_base->SDIO_RESPx[0] & SDIO_RESP1_ERRORS))
					{
						// R1 response is bad...
						hnd->error = hw_base->SDIO_RESPx[0];
						usr_HND_SET_STATUS(hnd, FLG_SIGNALED | RES_INVALID_DATA);
						drv_data->pending = NULL;
					} else
					{
						if(drv_data->sdio_op & SDIO_OP_WRITE)
						{
							hw_base->SDIO_DLEN = hnd->len;
							hw_base->SDIO_DCTRL = SDIO_DCTRL_DBLOCKSIZE_512b
										| SDIO_DCTRL_DTEN | SDIO_DCTRL_DMAEN;
						}

						if(drv_data->sdio_op & SDIO_OP_CMD)
						{
							switch(hnd->src.as_intptr[0] & 0x3f)
							{
							case SD_CMD42_LOCK_UNLOCK:
								hnd->src.as_intptr += 2;
								hw_base->SDIO_DLEN = hnd->len;
								switch(hnd->len)
								{
								case 4:
									hw_base->SDIO_DCTRL = SDIO_DCTRL_DBLOCKSIZE_4b
											| SDIO_DCTRL_DTEN | SDIO_DCTRL_DMAEN;
									break;
								case 8:
									hw_base->SDIO_DCTRL = SDIO_DCTRL_DBLOCKSIZE_8b
											| SDIO_DCTRL_DTEN | SDIO_DCTRL_DMAEN;
									break;
								case 16:
									hw_base->SDIO_DCTRL = SDIO_DCTRL_DBLOCKSIZE_16b
											| SDIO_DCTRL_DTEN | SDIO_DCTRL_DMAEN;
									break;
								case 32:
									hw_base->SDIO_DCTRL = SDIO_DCTRL_DBLOCKSIZE_32b
											| SDIO_DCTRL_DTEN | SDIO_DCTRL_DMAEN;
									break;
								default:
									hw_base->SDIO_DCTRL = SDIO_DCTRL_DBLOCKSIZE_64b
											| SDIO_DCTRL_DTEN | SDIO_DCTRL_DMAEN;
									break;
								}
								drv_data->sdio_op = SDIO_OP_WRITE | SDIO_OP_R1;
								break;

							case SD_CMD00_GO_IDLE_STATE:
								//reset bus width
								break;

							case SD_ACMD06_SET_BUS_WIDTH: // CMD6 has different length
								if(!(hw_base->SDIO_RESPx[0] & SDIO_RESP1_ERRORS))
								{
									unsigned int reg;

									//change bus width
									reg = hw_base->SDIO_CLKCR & ~SDIO_CLKCR_WIDBUS;
									if(hnd->src.as_intptr[1] == 2)
									{
										reg |= SDIO_CLKCR_WIDBUS_4b;
#if USE_SDIO_MULTIPLE_SLOTS
										hnd->mode1 = (SDIO_CLKCR_WIDBUS_4b)>>8;
#endif
									}
									hw_base->SDIO_CLKCR = reg;
								}
								break;


							}
						}
						SDIO_CMD_HND(drv_info, hnd, drv_data);
					}

				} else
				{
					if( ((drv_data->sdio_op & SDIO_OP_READ) && (status & SDIO_STA_RX_FLAGS))
						|| ((drv_data->sdio_op & SDIO_OP_WRITE) && (status & SDIO_STA_TX_FLAGS)) )
					{
						SDIO_CMD_HND(drv_info, hnd, drv_data);
					}

				}
			}
		}
		if(!drv_data->pending)
		{
			// start waiting handle
			while(!drv_data->locker && (hnd=drv_data->waiting))
			{
				drv_data->waiting = hnd->next;
				if( hnd->cmd & FLAG_LOCK)
					drv_data->locker = hnd;

				RES_CODE res;

				res = SDIO_START_HND(drv_info, hnd, drv_data);
				if(res & FLG_SIGNALED)
				{
					usr_HND_SET_STATUS(hnd, res);
				} else
				{
					drv_data->pending = hnd;
					hnd->next = NULL;
					break;
				}
			}
		}
	} else
	{
		if(status & SDIO_STA_TR_FLAGS)
		{
			hw_base->SDIO_MASK &= ~SDIO_STA_TR_FLAGS;
		}
	}
}


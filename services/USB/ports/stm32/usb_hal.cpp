/*
 * usb_hal.cpp
 *
 *  Created on: Dec 3, 2012
 *      Author: miro
 */

#include <tmos.h>
#include <usb_hal.h>
#include <tmos_atomic.h>
#include <usb_api.h>

#if TRACE_USB_LEVEL >= TRACE_LEVEL_DEBUG
#pragma GCC optimize ("O0")
#endif
//-------------------  local static functions --------------------------------//

bool gd_is_used;


#if USE_CPU_SLEEP_MODE
static void FAST_FLASH usr_usb_HND_SET_STATUS(HANDLE hnd, RES_CODE result)
{
	if(HND_ERROR_GET_LB(hnd) == RES_BUSY)
		locked_dec_int(&cpu_sleep_counter);
	usr_HND_SET_STATUS(hnd, result);
}
static void FAST_FLASH svc_usb_HND_SET_STATUS(HANDLE hnd, RES_CODE result)
{
	if(HND_ERROR_GET_LB(hnd) == RES_BUSY)
		locked_dec_int(&cpu_sleep_counter);
	svc_HND_SET_STATUS(hnd, result);
}
#else
#define usr_usb_HND_SET_STATUS(hnd, res) usr_HND_SET_STATUS(hnd, res)
#define svc_usb_HND_SET_STATUS(hnd, res) svc_HND_SET_STATUS(hnd, res)
#endif

static void stm_uDelay (uint32_t usec)
{
	usec ++;
	usec *= 16;
	while(usec--)
		__NOP();
}

/**
* Soft reset of the core
*/
static void stm_otg_core_reset(USB_TypeDef* otg)
{
	uint32_t count = 0;
	uint32_t mask;

	/* Wait for AHB master IDLE state. */
	while (!(otg->core_regs.GRSTCTL & OTG_GRSTCTL_AHBIDL))
	{
		if (++count > 200000)
		{
			TRACE1_USB("otg busy!");
			break;
		}
		stm_uDelay(3);
	}

	mask = otg->core_regs.GINTMSK;
	otg->core_regs.GINTMSK = mask & ~OTG_GINTSTS_CIDSCHG;
	/* Core Soft Reset */
	otg->core_regs.GRSTCTL |= OTG_GRSTCTL_CSRST;
	count = 0;
	while (otg->core_regs.GRSTCTL & OTG_GRSTCTL_CSRST)
	{
		if (++count > 200000)
		{
			TRACE1_USB("otg rst!");
			break;
		}
	}

	/* Wait for 3 PHY Clocks*/
	stm_uDelay(3);
	otg->core_regs.GINTSTS = OTG_GINTSTS_CIDSCHG;
	otg->core_regs.GINTMSK = mask;
}

/**
 * First level initialization (common for host and device)
 * @param drv_info
 */
static void stm_otg_core_init1(USB_DRV_INFO drv_info)
{
	//-- Enable USB related pins
	PIO_Cfg_List(drv_info->cfg->usb_pins);

    //-- Enable Clocking to the USB controller.
	RCCPeripheralEnable(drv_info->info.peripheral_indx);
	// RCCPeripheralLPEnable(drv_info->info.peripheral_indx);
    if(drv_info->cfg->stm32_otg & CFG_STM32_OTG_ULPI)
	{
		RCCPeripheralEnable(ID_PERIPH_OTGHS_ULPI);
		// RCCPeripheralLPEnable(ID_PERIPH_OTGHS_ULPI);
	}
	RCCPeripheralReset(drv_info->info.peripheral_indx);

	//-- Enable common interrupts
	drv_info->hw_base->core_regs.GINTSTS = -1u; //debug
	drv_enable_isr(&drv_info->info);
}

/**
 * Second level initialization (common for host and device)
 * @param drv_info
 */
static void stm_otg_core_init2(USB_DRV_INFO drv_info)
{
	uint32_t old_reg, new_reg, cfg;
	USB_TypeDef* hw_base = drv_info->hw_base;

	// disable global ints
	hw_base->core_regs.GAHBCFG &= ~OTG_GAHBCFG_GINTMSK;

	cfg = drv_info->cfg->stm32_otg;

	//--- change phy
	old_reg = hw_base->core_regs.GUSBCFG;
    if(cfg & CFG_STM32_OTG_ULPI)
    {
    	// power down the phy
       	hw_base->core_regs.GCCFG &= ~(OTG_GCCFG_PWRDWN);
		new_reg = old_reg & ~(OTG_GUSBCFG_TSDPS | OTG_GUSBCFG_ULPIFSLS | OTG_GUSBCFG_PHYSEL);
		new_reg |= OTG_GUSBCFG_PHYSEL_ULPI;
    }
    else
    	new_reg = old_reg | OTG_GUSBCFG_PHYSEL_FSSER;
    if(old_reg != new_reg)
    {
    	TRACE1_USB(" phy change");

        hw_base->core_regs.GUSBCFG = new_reg;

        //--- Reset after a PHY select
    	stm_otg_core_reset(hw_base);

    }
	//--- general core configuration register (GCCFG)
	if (!(cfg & CFG_STM32_OTG_ULPI))
		hw_base->core_regs.GCCFG = OTG_GCCFG_PWRDWN;

	// CIDSCHG is set on powerup or after phy change, we do not need it now...
	hw_base->core_regs.GINTSTS = OTG_GINTSTS_CIDSCHG;

	hw_base->core_regs.GINTMSK = OTG_GINTMSK_WUIM | OTG_GINTMSK_CIDSCHGM |
			OTG_GINTMSK_RXFLVLM | OTG_GINTMSK_OTGINT | OTG_GINTMSK_SRQIM;

	// enable global ints
	hw_base->core_regs.GAHBCFG |= OTG_GAHBCFG_GINTMSK;


	//--- USB configuration register (GUSBCFG)
	// TRDT = 4 x AHB clock frequency+ 1 PHY clock frequency (it will be updated after enumeration?)
	new_reg = hw_base->core_regs.GUSBCFG;
	new_reg &= ~(OTG_GUSBCFG_TRDT_Msk | OTG_GUSBCFG_FDMOD | OTG_GUSBCFG_FHMOD |
			OTG_GUSBCFG_HNPCAP | OTG_GUSBCFG_SRPCAP);
	new_reg |= OTG_GUSBCFG_TRDT(11);

	if (cfg & CFG_STM32_OTG_FORCE_DEVICE)
		new_reg |= OTG_GUSBCFG_FDMOD;

	if (cfg & CFG_STM32_OTG_FORCE_HOST)
		new_reg |= OTG_GUSBCFG_FHMOD;

	if (cfg & CFG_STM32_OTG_INTERNAL_VBUS)
		new_reg &= ~(OTG_GUSBCFG_ULPIEVBUSD | OTG_GUSBCFG_ULPIEVBUSI); /* Use internal VBUS */
	else
		new_reg |= OTG_GUSBCFG_ULPIEVBUSD | OTG_GUSBCFG_ULPIEVBUSI; /* Use external VBUS */


    hw_base->core_regs.GUSBCFG = new_reg;

    if(gd_is_used)
	    hw_base->core_regs.GCCFG |= OTG_GCCFG_NOVBUSSENS;

	// Restart the Phy Clock
    hw_base->PCGCCTL = 0;


}

#if USB_ENABLE_DEVICE
static void stm_otg_core_init3(USB_DRV_INFO drv_info)
{
	uint32_t reg, cfg;

	cfg = drv_info->cfg->stm32_otg;

	// Enable pull-up (start device)
	if (cfg & CFG_STM32_OTG_VBUS_SENS)
		reg = OTG_GCCFG_PWRDWN | OTG_GCCFG_VBUSBSEN /*| OTG_GCCFG_VBUSASEN*/;
	else
		reg = OTG_GCCFG_PWRDWN | OTG_GCCFG_NOVBUSSENS;

	if (cfg & CFG_STM32_OTG_SOF_OUTPUT)
	{
		reg |= OTG_GCCFG_SOFOUTEN;
	}
	drv_info->hw_base->core_regs.GCCFG = reg;
}
#endif //USB_ENABLE_DEVICE

/**
 * Reads a packet from the Rx FIFO
 * @param drv_info
 * @param endpoint
 * @param len
 */
static bool FAST_FLASH stm_read_payload(USB_DRV_INFO drv_info,	uint32_t status)
{
    Endpoint* endpoint;
    uint32_t bcnt, top_rx_cnt;
    HANDLE hnd;
    __IO uint32_t* fifo = drv_info->hw_base->DFIFO[0].DFIFO;

#if USB_ENABLE_HOST
	//host mode?
    uint32_t ept_indx;
    bool is_host=false;

    ept_indx = status & OTG_GRXSTSP_EPNUM_Msk;
	if(drv_info->drv_data->otg_flags & USB_OTG_FLG_HOST_CON)
	{
		is_host = true;
		endpoint = &drv_info->drv_data->endpoints[ept_indx >> 1];
	} else
		endpoint = &drv_info->drv_data->endpoints[ept_indx];

	endpoint->epd_out.epd_flags ^= EPD_FLAG_DATA1;
#else
	endpoint = &drv_info->drv_data->endpoints[status & OTG_GRXSTSP_EPNUM_Msk];
#endif

	bcnt = OTG_GRXSTSP_BCNT_Get(status);
	top_rx_cnt = 0;
	TRACE_USB(" (%ub)",bcnt);

	if (endpoint->epd_out.epd_state == ENDPOINT_STATE_RECEIVING)
	{
		if (bcnt)
		{
			// loop pending handles
			hnd = endpoint->epd_out.epd_pending;
			while (hnd)
			{
//				TRACE_USB("/hnd(%u)", hnd->len);
				if (!top_rx_cnt)
				{
					// try whole words
					while (hnd->len > 3 && bcnt > 3)
					{
						*hnd->dst.as_intptr = *fifo;
#if USB_DATA_TRACE
						TRACE1_USB("\e[33m");			//TC_TXT_YELLOW
						for (int i = 0; i < 4; i++)
							TRACE_USB(" %02X", hnd->dst.as_byteptr[i]);
						TRACE1_USB("\e[m");
#endif
						hnd->dst.as_intptr++;
						hnd->len -= 4;
						bcnt -= 4;
					}

//					TRACE_USB(" bcnt:%u", bcnt);
					if (!bcnt)
					{
						// nothing left in fifo or top
#if USB_ENABLE_HOST
						if (is_host)
						{
							if (hnd->len)
							{
								// re-activate
								uint32_t reg;
								OTG_HC_REGS* ch_regs = &drv_info->hw_base->HC_REGS[ept_indx];

								reg = ch_regs->HCCHAR;
								reg = (reg | OTG_HCCHAR_CHENA) & ~OTG_HCCHAR_CHDIS;
								ch_regs->HCCHAR = reg;
								status = OTG_GRXSTSP_PKTSTS_SETUP_DATA; // leave
							} else
							{
								status = 0; // signal
							}
						}
#endif
						if((status & OTG_GRXSTSP_PKTSTS_Msk) != OTG_GRXSTSP_PKTSTS_SETUP_DATA)
						{
							endpoint->epd_out.epd_pending = hnd->next;
							usr_usb_HND_SET_STATUS(hnd, RES_SIG_OK);
							endpoint->epd_out.epd_state = ENDPOINT_STATE_IDLE;
						} else
						{
							hnd->res |= FLG_OK;
						}

						return true;
					}

					// bcnt is >0 and hnd->len is <4
					if (hnd->len)
					{
						// len !=0 -> we must pad
						endpoint->top_rx_word = *fifo;
						top_rx_cnt = (bcnt > 3) ? 4 : bcnt;
#if USB_DATA_TRACE
						TRACE1_USB("\e[33m");			//TC_TXT_YELLOW
						for (uint32_t i = 0; i < top_rx_cnt; i++)
							TRACE_USB(" %02X", ((char*)&endpoint->top_rx_word)[i]);
						TRACE1_USB("\e[m");
#endif
						bcnt -= top_rx_cnt;
					}
				}

				//process the top word
				while (hnd->len && top_rx_cnt)
				{
					*hnd->dst.as_byteptr++ = endpoint->top_rx_word;
					endpoint->top_rx_word >>= 8;
					top_rx_cnt--;
					hnd->len--;
				}

				if (hnd->len == 0)
				{
					if((status & OTG_GRXSTSP_PKTSTS_Msk) != OTG_GRXSTSP_PKTSTS_SETUP_DATA)
					{
						endpoint->epd_out.epd_pending = hnd->next;
						usr_usb_HND_SET_STATUS(hnd, RES_SIG_OK);
						hnd = endpoint->epd_out.epd_pending;
						endpoint->epd_out.epd_state = ENDPOINT_STATE_IDLE;
					} else
					{
						hnd->res |= FLG_OK;
						hnd = NULL;
					}
				}
			}

			if (bcnt + top_rx_cnt)
			{
				TRACE1_USB("^");
				endpoint->epd_out.epd_state = ENDPOINT_STATE_RECEIVING_OFF;
				endpoint->rxfifo_cnt = bcnt;
				endpoint->top_rx_cnt = top_rx_cnt;
				return false;
			}
		}
//		endpoint->epd_out.epd_state = ENDPOINT_STATE_IDLE;
	} else
	{
		if (endpoint->epd_out.epd_state == ENDPOINT_STATE_IDLE)
		{
			TRACE1_USB("^");
			endpoint->epd_out.epd_state = ENDPOINT_STATE_RECEIVING_OFF;
			endpoint->rxfifo_cnt = bcnt;
			endpoint->top_rx_cnt = top_rx_cnt;
			return false;
		} else
		{
			TRACE_USB("usb drop %X %u bytes ep state=%X", status, bcnt,	endpoint->epd_out.epd_state);
			bcnt = (bcnt + 3) / 4;
			while (bcnt--)
				status = *fifo;
		}
	}
	return true;
}

#if USB_HOST_KEEPS_CHANNELS_DOWN && USB_ENABLE_HOST

static void FAST_FLASH host_buffer_to_hnd(Endpoint* endpoint, HANDLE hnd)
{
	uint8_t *data_ptr = endpoint->rxfifo_buff;
	uint32_t len = hnd->len;
	uint32_t bcnt;

	bcnt = endpoint->rxfifo_cnt;
	if (len > bcnt)
		len = bcnt;
	memcpy(hnd->dst.as_byteptr, data_ptr, len);
	hnd->len -= len;
	hnd->dst.as_byteptr += len;
	bcnt -= len;
	if ( !(endpoint->rxfifo_cnt = bcnt))
	{
		hnd->mode1 = 0;
		endpoint->epd_out.epd_state = ENDPOINT_STATE_IDLE;
	}
}
/**
 * Reads the remainder of the packet from the Rx FIFO
 * @param drv_info
 * @param endpoint
 * @param len
 */
static void FAST_FLASH host_read_to_buffer(USB_DRV_INFO drv_info, Endpoint* endpoint, uint32_t bcnt, uint32_t top_rx_cnt)
{
    __IO uint32_t* fifo = drv_info->hw_base->DFIFO[0].DFIFO;
	union{
		uint8_t* 	byte_ptr;
		uint32_t*	int_ptr;
	}ptr;
	uint32_t len;

	if (!endpoint->rxfifo_buff)
	{
		// create a buffer if it does not exist
		endpoint->rxfifo_buff = (uint8_t*) usr_malloc(endpoint->epd_out.epd_fifo_sz);
	}

#if USB_DATA_TRACE
		TRACE1_USB("\e[1;93m");			//TC_TXT_YELLOW
#endif
	if((ptr.byte_ptr=endpoint->rxfifo_buff))
	{
		len = bcnt + top_rx_cnt; // remaining from the current packet

		endpoint->epd_out.epd_state = ENDPOINT_STATE_RECEIVING_OFF;
		endpoint->rxfifo_cnt = len;
		endpoint->top_rx_cnt =0;
		while(len)
		{
			if(!top_rx_cnt)
			{
				// try whole words
				while(len > 3 && bcnt > 3)
				{
					*ptr.int_ptr = *fifo;
#if USB_DATA_TRACE
					for(int i=0; i<4; i++)
						TRACE_USB(" %02X", ptr.byte_ptr[i]);
#endif
					len -= 4;
					bcnt -= 4;
					ptr.int_ptr++;
				}

				if(!bcnt)
				{	// nothing left in fifo or top
					break;
				}

				// bcnt is >0 and hnd->len is <4
				if(len)
				{
					// len !=0 -> we must pad
					endpoint->top_rx_word = *fifo;
					top_rx_cnt = (bcnt>3)?4:bcnt;
					bcnt -= top_rx_cnt;
				}
			}

			//process the top word
			while(len && top_rx_cnt)
			{
				*ptr.byte_ptr = endpoint->top_rx_word;
#if USB_DATA_TRACE
				TRACE_USB(" %02X", ptr.byte_ptr[0]);
#endif
				endpoint->top_rx_word >>= 8;
				ptr.byte_ptr++;
				top_rx_cnt--;
				len--;
			}
		}
	}else
	{// not enough memory to allocate buffer, shouldn't happen but just in case
	 // dummy data read and FIFO release
		endpoint->rxfifo_cnt = 0;
		endpoint->top_rx_cnt =0;
		ptr.int_ptr = &len;
		TRACELN1_USB("NOT ENOUGH MEMORY:");
		while(bcnt > 3)
		{
			*ptr.int_ptr = *fifo;
#if USB_DATA_TRACE
			for(int i=0; i<4; i++)
				TRACE_USB(" %02X", ptr.byte_ptr[i]);
#endif
			bcnt -= 4;
		}

	}
#if USB_DATA_TRACE
		TRACE1_USB("\e[m");
#endif
}

/**
 * Reads a packet from the Rx FIFO
 * @param drv_info
 * @param endpoint
 * @param len
 *
 * only handles the pending handler if there is one.
 * otherwise, the data is buffered
 *
 */
static bool FAST_FLASH host_halt_read_payload(USB_DRV_INFO drv_info, uint32_t status)
{
	__IO uint32_t* fifo  = drv_info->hw_base->DFIFO[0].DFIFO;
	Endpoint* endpoint = &drv_info->drv_data->endpoints[(status & OTG_GRXSTSP_EPNUM_Msk) >> 1];
	OTG_HC_REGS* ch_regs = &drv_info->hw_base->HC_REGS[status & OTG_GRXSTSP_EPNUM_Msk];

	uint32_t bcnt, top_rx_cnt;
	HANDLE hnd;

	endpoint->epd_out.epd_flags ^= EPD_FLAG_DATA1;

	bcnt = OTG_GRXSTSP_BCNT_Get(status);
	top_rx_cnt = 0;
	TRACE_USB("(%ub)", bcnt);


	if (endpoint->epd_out.epd_state == ENDPOINT_STATE_RECEIVING)
	{
		// processing the pending handler, if any
		hnd = endpoint->epd_out.epd_pending;
		if (bcnt)
		{
			if (hnd)
			{
				TRACE_USB(" hnd(%u)", hnd->len);
				// has a pending handle and received data ,
				// so it sets the result to Ok
				hnd->res |= FLG_OK;
				if (!top_rx_cnt)
				{
					// try whole words
					while (hnd->len > 3 && bcnt > 3)
					{
						*hnd->dst.as_intptr = *fifo;
#if USB_DATA_TRACE
						TRACE1_USB("\e[33m");			//TC_TXT_YELLOW
						for (int i = 0; i < 4; i++)
							TRACE_USB(" %02X", hnd->dst.as_byteptr[i]);
						TRACE1_USB("\e[m");
#endif
						hnd->dst.as_intptr++;
						hnd->len -= 4;
						bcnt -= 4;
					}

					TRACE_USB(" bcnt:%u", bcnt);
					if (!bcnt)
					{
						// nothing left in fifo or top
						if(ch_regs->HCTSIZ & OTG_HCTSIZ_PKTCNT_Msk)
						{
							/* re-activate the channel when more packets are expected */
							bcnt = ch_regs->HCCHAR;
							bcnt = (bcnt | OTG_HCCHAR_CHENA) & ~ OTG_HCCHAR_CHDIS;
							ch_regs->HCCHAR = bcnt;
						}
						return true;
					}

					// bcnt is >0 and hnd->len is <4
					if (hnd->len)
					{
						// len !=0 -> we must pad
						endpoint->top_rx_word = *fifo;
						top_rx_cnt = (bcnt > 3) ? 4 : bcnt;
#if USB_DATA_TRACE
						TRACE1_USB("\e[33m");			//TC_TXT_YELLOW
						for (uint32_t i = 0; i < top_rx_cnt; i++)
							TRACE_USB(" %02X",
									((char* )&endpoint->top_rx_word)[i]);
						TRACE1_USB("\e[m");
#endif
						bcnt -= top_rx_cnt;
					}
				}

				//process the top word
				while (hnd->len && top_rx_cnt)
				{
					*hnd->dst.as_byteptr++ = endpoint->top_rx_word;
					endpoint->top_rx_word >>= 8;
					top_rx_cnt--;
					hnd->len--;
				}
			}

			if (bcnt + top_rx_cnt)
			{
				TRACE1_USB("^");
				host_read_to_buffer(drv_info, endpoint, bcnt, top_rx_cnt);
			}
		}
		else
		{
			//transaction without data (bcnt == 0)
			if(hnd && !hnd->len)
			{
				hnd->res |= FLG_OK;
			}
		}
	}
	else
	{
		if (endpoint->epd_out.epd_state == ENDPOINT_STATE_IDLE)
		{
			TRACE1_USB("^");
			host_read_to_buffer(drv_info, endpoint, bcnt, top_rx_cnt);
		}
		else
		{
			TRACE_USB("usb drop %X %u bytes ep state=%X", status, bcnt,
					endpoint->epd_out.epd_state);
			bcnt = (bcnt + 3) / 4;
			while (bcnt--)
				status = *fifo;
		}
	}
	return true;
}
#endif //USB_HOST_KEEPS_CHANNELS_DOWN && USB_ENABLE_HOST

/** Flush a Tx FIFO
 *
 * @param otg
 * @param num - fifo
 */
static void stm_flush_tx_fifo(USB_TypeDef* otg, uint32_t num )
{
	uint32_t count = 0;

	otg->core_regs.GRSTCTL = OTG_GRSTCTL_TXFFLSH | OTG_GRSTCTL_TXFNUM(num);
	while(otg->core_regs.GRSTCTL & OTG_GRSTCTL_TXFFLSH)
	{
		if (++count > 200000)
			break;
	}
	/* Wait for 3 PHY Clocks*/
	stm_uDelay(3);
}

/**
 * Flush a Rx FIFO
 * @param otg
 */
static void stm_flush_rx_fifo( USB_TypeDef* otg )
{
	uint32_t count = 0;

	otg->core_regs.GRSTCTL = OTG_GRSTCTL_RXFFLSH;
	while(otg->core_regs.GRSTCTL & OTG_GRSTCTL_RXFFLSH)
	{
		if (++count > 200000)
			break;
	}

	/* Wait for 3 PHY Clocks*/
	stm_uDelay(3);
}

/**
 * active USB Core clock
 * @param otg
 * @param cfg
 */
static void stm_otg_ungate_clock(USB_TypeDef* otg, uint32_t cfg)
{
	if(cfg & CFG_STM32_OTG_LOW_POWER)
	{
		if(otg->device_regs.DSTS & OTG_DSTS_SUSPSTS)
		{
			/* un-gate USB Core clock */
			otg->PCGCCTL &= ~(OTG_PCGCCTL_GATEHCLK | OTG_PCGCCTL_STPPCLK);
		}
	}
}

//------------------------------------------------------------------------------
//						DEVICE related functions
//------------------------------------------------------------------------------
#if USB_ENABLE_DEVICE
/**
 * Try to load next packet in the FIFO
 * aka DCD_WriteEmptyTxFifo
 * @param drv_info
 * @param ept_indx
 */
static void FAST_FLASH stm_write_payload(USB_DRV_INFO drv_info, uint32_t ept_indx)
{
    ep_dir_state_t* epdir;
    HANDLE hnd;
	uint32_t len, len32b, txsize;
    __IO uint32_t *fifo;

	epdir = &(drv_info->drv_data->endpoints[ept_indx].epd_in);
	hnd = epdir->epd_pending;
	if(hnd)
	{
		fifo = drv_info->hw_base->DFIFO[ept_indx].DFIFO;
		txsize = drv_info->hw_base->in_ept_regs[ept_indx].DTXFSTS & DTXFSTS_INEPTFSAV_Msk;

		TRACE_USB(" Wr%d(%d) ", ept_indx , hnd->len);
		while( (len = hnd->len) )
		{
			if (len > epdir->epd_fifo_sz)
				len = epdir->epd_fifo_sz;

			TRACE1_USB("\e[32m");			//TC_TXT_GREEN
			for(uint32_t i=0; i<len; i++)
				TRACE_USB(" %02X", hnd->src.as_byteptr[i]);
			TRACE1_USB("\e[m");

			len32b = (len + 3) / 4;
			if (txsize <= len32b)
				break;
			txsize -= len32b;


			while(len32b--)
				fifo[0] = *hnd->src.as_intptr++;
			hnd->len -= len;
		}

		if(len)
		{
			if(epdir->epd_type != ENDPOINT_TYPE_ISOCHRONOUS)
			{
				drv_info->hw_base->device_regs.DIEPEMPMSK |= OTG_DIEPEMPMSK_INEPTXFEM(ept_indx);
			}
			return;
		}
	} else
	{
		TRACELN1_USB("usb tx emp !h");
	}
	drv_info->hw_base->device_regs.DIEPEMPMSK &= ~OTG_DIEPEMPMSK_INEPTXFEM(ept_indx);
}

static void FAST_FLASH stm_start_tx(USB_DRV_INFO drv_info, HANDLE hnd, uint32_t eptnum, ep_dir_state_t* epdir)
{
	uint32_t reg;
	USB_TypeDef* otg = drv_info->hw_base;

	// Do not send while in suspend state
	if(otg->device_regs.DSTS & OTG_DSTS_SUSPSTS)
	{
		TRACELN1_USB(" WR SUSPEND ");
		usb_drv_end_transfers(epdir, RES_SIG_ERROR);
		return;
	}

	// 1. Program the transfer size...
	if (hnd->len == 0)
	{
		reg = otg->in_ept_regs[eptnum].DIEPTSIZ;
		reg &= OTG_DIEPTSIZ_MCNT_Msk;
		reg |= OTG_DIEPTSIZ_PKTCNT(1);
	} else
	{
		/* Program the transfer size and packet count
		 * as follows: xfersize = N * maxpacket +
		 * short_packet pktcnt = N + (short_packet
		 * exist ? 1 : 0)
		 */
		reg = hnd->len;
		reg |= OTG_DIEPTSIZ_PKTCNT((hnd->len + epdir->epd_fifo_sz -1)/epdir->epd_fifo_sz);

		// set FLG_OK if we need to send ZLP packet
		if( (hnd->len % epdir->epd_fifo_sz) == 0)
			hnd->res |= FLG_OK;

		if (epdir->epd_type == ENDPOINT_TYPE_ISOCHRONOUS)
		{
			reg |= OTG_DIEPTSIZ_MCNT(1);
		}
	}
	otg->in_ept_regs[eptnum].DIEPTSIZ = reg;

	// 2. Enable endpoint, clear NAK etc..
	reg = otg->in_ept_regs[eptnum].DIEPCTL;
	if (epdir->epd_type == ENDPOINT_TYPE_ISOCHRONOUS)
	{
		if (otg->device_regs.DSTS & OTG_DSTS_FNSOF_ODD)
		{
			reg |= OTG_DIEPCTL_SD0PID;
		}
		else
		{
			reg |= OTG_DIEPCTL_SD1PID;
		}
	}
	otg->in_ept_regs[eptnum].DIEPCTL = reg | OTG_DIEPCTL_CNAK | OTG_DIEPCTL_EPENA;

	// 3. Write some data
	stm_write_payload(drv_info, eptnum);
}

static void FAST_FLASH stm_start_rx(USB_DRV_INFO drv_info, uint32_t ept_indx, Endpoint *endpoint)
{
	HANDLE hnd;

	hnd = endpoint->epd_out.epd_pending;
	if(hnd && (!ept_indx || drv_info->drv_data->usb_state >= USBST_DEVICE_ADDRESS))
	{
    	OTG_OUT_EPT_REGS* otg_regs;
    	uint32_t depctl, deptsiz, maxpacket;

    	//start xfer
    	otg_regs = &drv_info->hw_base->out_ept_regs[ept_indx];
    	depctl = otg_regs->DOEPCTL;
    	if( !( depctl & OTG_DOEPCTL_EPENA) )
    	{
    		deptsiz = otg_regs->DOEPTSIZ;
//    		TRACELN_USB("usb sr%u sz=%08x", ept_indx, deptsiz);
    		deptsiz &= OTG_DOEPTSIZ_STUPCNT_Msk;

    	    /* Program the transfer size and packet count as follows:
    	    * pktcnt = N
    	    * xfersize = N * maxpacket
    	    */
    		maxpacket = endpoint->epd_out.epd_fifo_sz;
    		if(!maxpacket)
    			maxpacket = 64;
    	    if (hnd->len == 0)
    	    {
    	    	deptsiz |= OTG_DOEPTSIZ_PKTCNT(1) | OTG_DOEPTSIZ_XFRSIZ(maxpacket);
    	    }
    	    else
    	    {
    	    	uint32_t pktcnt;
    	      	pktcnt = (hnd->len + (maxpacket - 1)) / maxpacket;
    	    	deptsiz |= OTG_DOEPTSIZ_PKTCNT(pktcnt) | OTG_DOEPTSIZ_XFRSIZ(pktcnt*maxpacket);
    	    }
    	    depctl |= OTG_DOEPCTL_EPENA | OTG_DOEPCTL_CNAK;
    		otg_regs->DOEPTSIZ = deptsiz;
	    	otg_regs->DOEPCTL = depctl;

    	} else
    	{
//    		TRACELN_USB("usb rr%u sz=%08x", ept_indx, otg_regs->DOEPTSIZ);
    	}
	}
}

static void stm_otg_core_init_dev(USB_TypeDef* otg, const usb_config_t* cfg)
{
	uint32_t reg, adr;
	uint32_t dev_endpoints;

	TRACELN1_USB("dev init");

	/* Device configuration register */
	reg = otg->device_regs.DCFG;
	reg &= ~(OTG_DCFG_PFIVL_Msk | OTG_DCFG_DAD_Msk);
	reg |= OTG_DCFG_PFIVL_80;
	otg->device_regs.DCFG = reg;


	// Device speed
	if(cfg->stm32_otg & CFG_STM32_OTG_HS_CORE)
	{
		// HS core

		/* Set High speed phy */
		if(cfg->stm32_otg & CFG_STM32_OTG_ULPI)
		{
			reg = OTG_DCFG_DSPD_HS;
		}
		else /* set High speed phy in Full speed mode */
		{
			reg =  OTG_DCFG_DSPD_FS11;
		}

	    dev_endpoints    = 6-1;
	} else
	{
		// FS core
		/* Set Full speed phy */
		reg =  OTG_DCFG_DSPD_FS11;

	    dev_endpoints    = 4-1;
	}
	reg |= otg->device_regs.DCFG & ~OTG_DCFG_DSPD_Msk;
	otg->device_regs.DCFG = reg;

	adr = cfg->rx_fifo_sz;
	/* set Rx FIFO size */
	otg->core_regs.GRXFSIZ = adr;

	/* EP0 TX*/
	otg->core_regs.DIEPTXF0_HNPTXFSIZ = adr + OTG_DIEPTXF0_TX0FD(cfg->tx_fifo_sz[0]);

	/* EP1 - EP5 TX*/
	for(uint32_t i=0; i<dev_endpoints; i++)
	{
		adr += cfg->tx_fifo_sz[i];
		otg->core_regs.DIEPTXF[i] = adr + OTG_DIEPTXF0_TX0FD(cfg->tx_fifo_sz[i+1]);
	}


	/* Flush the FIFOs */
	stm_flush_tx_fifo(otg, 0x10); /* all Tx FIFOs */
	stm_flush_rx_fifo(otg);
	/* Clear all pending Device Interrupts */
	otg->device_regs.DAINT		= 0xFFFFFFFF;
	otg->device_regs.DAINTMSK	= 0;

	for (uint32_t i = 0; i < dev_endpoints; i++)
	{
		reg = otg->in_ept_regs[i].DIEPCTL;
		if(reg & OTG_DIEPCTL_EPENA)
		{
			reg = OTG_DIEPCTL_EPDIS | OTG_DIEPCTL_SNAK;
		} else
		{
			reg = 0;
		}
		otg->in_ept_regs[i].DIEPCTL = reg;
		otg->in_ept_regs[i].DIEPINT = 0xFF;

		reg = otg->out_ept_regs[i].DOEPCTL;
		if(reg & OTG_DOEPCTL_EPENA)
		{
			reg = OTG_DOEPCTL_EPDIS | OTG_DOEPCTL_SNAK;
		} else
		{
			reg = 0;
		}
		otg->out_ept_regs[i].DOEPCTL = reg;
		otg->out_ept_regs[i].DOEPINT = 0xFF;
	}
	otg->device_regs.DIEPMSK = OTG_DIEPMSK_BIM | OTG_DIEPMSK_TXFURM |
			OTG_DIEPMSK_TOCM | OTG_DIEPMSK_EPDM |OTG_DIEPMSK_XFRCM;
#if USB_STM32_DEDICATED_EP1
	otg->device_regs.DINEP1MSK = OTG_DIEPMSK_BIM | OTG_DIEPMSK_TXFURM |
			OTG_DIEPMSK_TOCM | OTG_DIEPMSK_EPDM | OTG_DIEPMSK_XFRCM;
#endif
	otg->device_regs.DOEPMSK = OTG_DOEPMSK_BOIM | OTG_DOEPMSK_OPEM |
			OTG_DOEPMSK_B2BSTUP | OTG_DOEPMSK_EPDM;
#if USB_STM32_DEDICATED_EP1
	otg->device_regs.DOUTEP1MSK = OTG_DOEPMSK_BOIM | OTG_DOEPMSK_OPEM |
			OTG_DOEPMSK_B2BSTUP | OTG_DOEPMSK_EPDM;
#endif

	/* Enable interrupts matching to the Device mode ONLY */
	otg->core_regs.GINTSTS = OTG_GINTSTS_USBRST;
	otg->core_regs.GINTMSK |= OTG_GINTMSK_USBSUSPM | OTG_GINTMSK_USBRST	|
			OTG_GINTMSK_ENUMDNEM |	OTG_GINTMSK_IEPINT | OTG_GINTMSK_OEPINT |
			OTG_GINTMSK_SOFM | OTG_GINTMSK_IISOOXFRM | OTG_GINTMSK_IISOIXFRM
			| OTG_GINTMSK_RESETDET | OTG_GINTMSK_FETSUSP;

	TRACELN1_USB("dev init done");

}

static void stm_ept_config(USB_DRV_INFO drv_info, uint32_t ept_num,
		const USBGenericDescriptor* pDescriptor)
{
	ep_dir_state_t* epdir;
	uint32_t ept_indx = ept_num & 0xF;
	USB_TypeDef* otg = drv_info->hw_base;
	__IO uint32_t* depctl;
	uint32_t reg, ept_type, fifo_sz;

    if(ept_indx >= USB_NUMENDPOINTS)
    {
		TRACELN1_USB("Invalid endpoint descriptor!");

    } else
    {
        TRACE_USB(" CfgEp(%x)", ept_num);

        /* Abort the current transfer is the endpoint was configured and in
    	 Write or Read state */
    	usb_hal_ept_reset(drv_info, ept_num);

        // set fifo size
    	ept_type = ENDPOINT_TYPE_CONTROL;
    	switch(pDescriptor->bDescriptorType)
    	{
    	case DEVICE_DESCRIPTOR:
        	fifo_sz = ((USBDeviceDescriptor *)pDescriptor)->bMaxPacketSize0;
        	break;

    	case ENDPOINT_DESCRIPTOR:
    		fifo_sz = ((const USBEndpointDescriptor*)pDescriptor)->wMaxPacketSize;
    		ept_type = ((const USBEndpointDescriptor*)pDescriptor)->GetType();
    		break;

    	default:
    		fifo_sz = 0;
    		break;
        }

        if(ept_num & 0x80)
        {
            epdir = &drv_info->drv_data->endpoints[ept_indx].epd_in;
#if USB_STM32_DEDICATED_EP1
            if(ept_indx == EPT_1 && drv_info->cfg->stm32_otg & CFG_STM32_OTG_DEDICATED_EP1)
            	otg->device_regs.DEACHINTMSK |= OTG_DEACHINTMSK_IEP1INT;
            else
#endif
            otg->device_regs.DAINTMSK |= OTG_DAINTMSK_IEPM(ept_indx);

        	depctl = &otg->in_ept_regs[ept_indx].DIEPCTL;
        	reg = *depctl;
        	if(!(reg & OTG_DIEPCTL_USBAEP))
        	{
        		reg &= ~(OTG_DIEPCTL_TXFNUM_Msk | OTG_DIEPCTL_EPTYP_Msk |
        				OTG_DIEPCTL_MPSIZ_Msk);
        		reg |= OTG_DIEPCTL_SD0PID | OTG_DIEPCTL_TXFNUM(ept_indx) |
        				OTG_DIEPCTL_EPTYP(ept_type) | OTG_DIEPCTL_USBAEP |
        				OTG_DIEPCTL_MPSIZ(fifo_sz);
            	*depctl = reg;
        	}
        } else
        {
            epdir = &drv_info->drv_data->endpoints[ept_indx].epd_out;
#if USB_STM32_DEDICATED_EP1
            if(ept_indx == EPT_1 && drv_info->cfg->stm32_otg & CFG_STM32_OTG_DEDICATED_EP1)
            	otg->device_regs.DEACHINTMSK |= OTG_DEACHINTMSK_OEP1INTM;
            else
#endif
           	otg->device_regs.DAINTMSK |= OTG_DAINTMSK_OEPM(ept_indx);
        	depctl = &otg->out_ept_regs[ept_indx].DOEPCTL;
        	reg = *depctl;
        	if(!(reg & OTG_DOEPCTL_USBAEP))
        	{
        		reg &= ~( OTG_DOEPCTL_EPTYP_Msk | OTG_DOEPCTL_MPSIZ_Msk);
        		reg |= OTG_DOEPCTL_SD0PID | OTG_DOEPCTL_EPTYP(ept_type) |
        				OTG_DOEPCTL_USBAEP | OTG_DOEPCTL_MPSIZ(fifo_sz);
            	*depctl = reg;
        	}
        }

		epdir->epd_fifo_sz = fifo_sz;
    	epdir->epd_state = ENDPOINT_STATE_IDLE;

#if USB_STM32_DEDICATED_EP1
    	if(drv_info->cfg->stm32_otg & CFG_STM32_OTG_DEDICATED_EP1)
    	{
    		switch(ept_num)
    		{
    		case 0x01:
    			NVIC->NVIC_EnableIRQ(OTG_HS_EP1_OUT_IRQn);
    			break;
    		case 0x81:
    			NVIC->NVIC_EnableIRQ(OTG_HS_EP1_IN_IRQn);
    			break;
    		}

    	}
#endif
    }
}

/** Configure as device
 * called from the usbdrv thread
 * @param drv_info
 */
void usb_hal_device_start(USB_DRV_INFO drv_info)
{
	USB_CONTROLLER* otg = drv_info->hw_base;

	// init core
	stm_otg_core_init1(drv_info);
	stm_otg_core_init2(drv_info);

	// init device
	stm_otg_core_init_dev(otg, drv_info->cfg);
	stm_otg_core_init3(drv_info);
}

/**
 * Configures an endpoint according to its Endpoint Descriptor.
 * \param pDescriptor Pointer to an Endpoint descriptor.
 */
void usb_hal_ept_config(USB_DRV_INFO drv_info, const USBGenericDescriptor* pDescriptor)
{
	const USBEndpointDescriptor* descriptor;

	switch(pDescriptor->bDescriptorType)
	{
	case DEVICE_DESCRIPTOR:
		stm_ept_config(drv_info, 0, pDescriptor);
		stm_ept_config(drv_info, 0x80, pDescriptor);
        break;

	case ENDPOINT_DESCRIPTOR:
		descriptor = (const USBEndpointDescriptor*)pDescriptor;

		stm_ept_config(drv_info, descriptor->bEndpointAddress, pDescriptor);

		break;

	default:
		TRACELN1_USB("Invalid descriptor!");
		return;
	}

}

#endif // USB_ENABLE_DEVICE

#if USB_ENABLE_OTG
/**
 * USB starts with USB_CMD_DEVICE_CONFIG or USB_CMD_OTG_CONFIG and we get here
 *
 * Common low level initialization is called and then if the current mode does
 * not match the requested RES_IDLE is returned and the client will wait for
 * (CIDSCHGM, ...)
 *
 * Device:
 *  - on bus reset
 *  	reset endpoints
 *  	cfg endpoints
 *  	|= USB_OTG_FLG_DEV_CON | USB_OTG_FLG_DEV_OK
 *  	restart pending
 *
 * Host:
 *
 *
 * @param drv_info
 * @param mode
 * @return
 */
RES_CODE usb_hal_start(USB_DRV_INFO drv_info, uint32_t mode)
{
	RES_CODE res;
	USB_DRIVER_DATA* drv_data = drv_info->drv_data;
	USB_TypeDef* hw_base = drv_info->hw_base;

	if( drv_data->otg_flags & mode )
	{
		res = RES_SIG_OK;
	} else
	{
		// The mode we need is not active
		res = RES_IDLE;
		if ( !(drv_data->otg_flags & (USB_OTG_FLG_DEV | USB_OTG_FLG_HOST)) )
		{
			// otg peripheral is idle

			// low level reset now
			TRACE_USB_NAME(drv_info);
			TRACE1_USB(" Init");

			stm_otg_core_init1(drv_info);
			stm_otg_core_init2(drv_info);

			//wait resets and phy/clk changes to take effect and CMOD get set
			tsk_sleep(250);

			// check if the mode matches
#if USB_ENABLE_DEVICE
			if(mode == USB_OTG_FLG_DEV)
			{
				// check if we can continue as device
				if( !(hw_base->core_regs.GINTSTS & OTG_GINTSTS_CMOD))
				{
					stm_otg_core_init_dev(hw_base, drv_info->cfg);
					stm_otg_core_init3(drv_info);
					drv_data->otg_flags |= USB_OTG_FLG_DEV;
					res = RES_SIG_OK;
				}
			} else
#endif
			{
#if USB_ENABLE_HOST
				// check if we can continue as host
				if(hw_base->core_regs.GINTSTS & OTG_GINTSTS_CMOD)
				{
					drv_data->otg_flags |= USB_OTG_FLG_HOST;
					res = usb_hal_host_start(drv_info);
				}
#endif
			}
			TRACE_USB_NAME(drv_info);
			TRACE1_USB(" Init done");

		}
		else
		{
#if USB_ENABLE_HOST
		if(mode == USB_OTG_FLG_HOST && !(drv_data->otg_flags & USB_OTG_FLG_HOST))
			res = FLG_SIGNALED | RES_FATAL;
#endif
		}
	}

	return res;
}
#endif //USB_ENABLE_OTG


//------------------------------------------------------------------------------
//						HOST related functions
//------------------------------------------------------------------------------
#if USB_ENABLE_HOST
static void FAST_FLASH usb_host_start_halt_tout(USB_DRV_INFO drv_info, uint32_t ch_indx)
{
	USB_DRIVER_DATA* drv_data = drv_info->drv_data;
	OTG_HC_REGS* ch_regs = &drv_info->hw_base->HC_REGS[ch_indx];

	if(drv_data->ept_halt_flags & (1<<ch_indx))
    {
    	TRACELN_USB("\e[1;91msomething is wrong with ch:%u!\e[m", ch_indx);
    }else
    {
//    	TRACELN_USB("start SOF cnt(%u)", ch_indx);
    	if(!drv_data->ept_halt_flags)
    	{
    		if(!(drv_info->hw_base->core_regs.GINTMSK & OTG_GINTMSK_SOFM))
    		{
    			drv_info->hw_base->core_regs.GINTSTS = OTG_GINTSTS_SOF;
    			drv_info->hw_base->core_regs.GINTMSK |= OTG_GINTMSK_SOFM;
    		}
    	}
    	drv_data->ept_halt_flags |= (1<<ch_indx);
    	if(ch_regs->HCCHAR & OTG_HCCHAR_EPDIR)
    		drv_info->drv_data->endpoints[ch_indx >> 1].epd_out.epd_halt_tout = HALT_FRAMES_TOUT;
    	else
    		drv_info->drv_data->endpoints[ch_indx >> 1].epd_in.epd_halt_tout = HALT_FRAMES_TOUT;
    }

}

static void FAST_FLASH usb_host_stop_halt_tout(USB_DRV_INFO drv_info, uint32_t ch_indx)
{
	USB_DRIVER_DATA* drv_data = drv_info->drv_data;
	OTG_HC_REGS* ch_regs = &drv_info->hw_base->HC_REGS[ch_indx];

//	TRACELN_USB("stop SOF(%u)", ch_indx);

	drv_data->ept_halt_flags &= ~(1<<ch_indx);
	if(ch_regs->HCCHAR & OTG_HCCHAR_EPDIR)
		drv_info->drv_data->endpoints[ch_indx >> 1].epd_out.epd_halt_tout = HALT_FRAMES_TOUT;
	else
		drv_info->drv_data->endpoints[ch_indx >> 1].epd_in.epd_halt_tout = HALT_FRAMES_TOUT;
	if(!drv_data->ept_halt_flags)
	{
		if((drv_info->hw_base->core_regs.GINTMSK & OTG_GINTMSK_SOFM))
		{
			drv_info->hw_base->core_regs.GINTMSK &= ~OTG_GINTMSK_SOFM;
			drv_info->hw_base->core_regs.GINTSTS = OTG_GINTSTS_SOF;
		}
	}
}

static bool FAST_FLASH usb_host_process_halt_tout(USB_DRV_INFO drv_info)
{
	USB_DRIVER_DATA* drv_data = drv_info->drv_data;
	uint32_t ch_indx;
	bool is_in_channel;
	Endpoint *endpoint;
	ep_dir_state_t *epdir;

	if(drv_data->ept_halt_flags)
	{
		for(ch_indx=0; ch_indx < OTG_MAX_FIFOS; ch_indx++)
		{
			if(drv_data->ept_halt_flags & (1<<ch_indx))
			{

				endpoint = &drv_info->drv_data->endpoints[ch_indx >> 1];
				is_in_channel = (drv_info->hw_base->HC_REGS[ch_indx].HCCHAR & OTG_HCCHAR_EPDIR);

				if(is_in_channel){
		    		epdir = &endpoint->epd_out;
				}else{
		    		epdir = &endpoint->epd_in;
				}

				if(epdir->epd_halt_tout)
				{
					epdir->epd_halt_tout--;
				}
				else
				{
					if(is_in_channel)
					{
						if(epdir->epd_state == ENDPOINT_STATE_RECEIVING_OFF)
						{
							endpoint->rxfifo_cnt =0;
							endpoint->top_rx_cnt =0;
						}
					}
					TRACE1_USB(" res:FATAL");
				    HANDLE hnd;

					while( (hnd =  epdir->epd_pending) )
					{
						TRACE1_USB(" Tout ");
				#if USB_HOST_KEEPS_CHANNELS_DOWN
						HND_ERROR_SET_MB(hnd, epdir->epd_halt_reason);
				#endif
						usr_usb_HND_SET_STATUS(hnd, FLG_SIGNALED|RES_FATAL);
						epdir->epd_pending = hnd->next;
					}
					epdir->epd_state = ENDPOINT_STATE_IDLE;
#if USB_HOST_KEEPS_CHANNELS_DOWN
					epdir->epd_halt_reason = EPT_HALT_IDLE;
					reset_error_cnt();
#endif
					usb_host_stop_halt_tout(drv_info, ch_indx);
				}
			}
		}
	}
	return (drv_data->ept_halt_flags == 0);
}

void usb_hal_host_ept_cfg(USB_DRV_INFO drv_info, const USBEndpointDescriptor* pDescriptor)
{
	uint32_t ept_indx;
	uint32_t reg;
	USB_TypeDef* otg = drv_info->hw_base;
	bool is_in_dir = pDescriptor->GetDirection() == ENDPOINT_DIRECTION_IN;
	EndpointAttribute_TransferType ept_type = pDescriptor->GetType();
	ep_dir_state_t* epdir;

	ept_indx = (pDescriptor->bEndpointAddress & 0x7f) << 1;
	ept_indx += pDescriptor->bEndpointAddress >> 7;
    TRACE_USB(" CfgEp(%x=0x%02x)", ept_indx, pDescriptor->bEndpointAddress);

	// Clear any old pending interrupts for this host channel.
	otg->HC_REGS[ept_indx].HCINT = -1U;

	// Enable channel interrupts
	reg = OTG_HCINTMSK_XFRCM |	OTG_HCINTMSK_TRERRM;
	if(is_in_dir)	//in direction
		reg |= OTG_HCINT_BBERR;

	switch (ept_type)
	{
		case ENDPOINT_TYPE_CONTROL:
		case ENDPOINT_TYPE_BULK:
			reg |= OTG_HCINT_STALL | OTG_HCINT_NAK | OTG_HCINT_DTERR | OTG_HCINTMSK_CHHM;
			if (!is_in_dir)
				reg |= OTG_HCINT_NYET;
		break;

		case ENDPOINT_TYPE_INTERRUPT:
			reg |= OTG_HCINT_STALL | OTG_HCINT_NAK | OTG_HCINT_FRMOR | OTG_HCINT_DTERR | OTG_HCINTMSK_CHHM;
		break;

		case ENDPOINT_TYPE_ISOCHRONOUS:
			reg |= OTG_HCINT_ACK | OTG_HCINT_FRMOR | OTG_HCINTMSK_CHHM;
		break;
	}

	otg->HC_REGS[ept_indx].HCINTMSK = reg;

	// Enable the top level host channel interrupt.
	otg->host_regs.HAINTMSK |= 1 << ept_indx;


	// Program the HCCHAR register
	reg = OTG_HCCHAR_MPS(pDescriptor->wMaxPacketSize)
		| OTG_HCCHAR_EPNUM(pDescriptor->GetNumber())
		| OTG_HCCHAR_EPTYP(ept_type)
		| OTG_HCCHAR_DAD(pDescriptor->bEndpointAddress & 0x7F);

	// Special case settings for low speed devices
	if (((otg->HPRT & OTG_HPRT_PSPD_Msk) == OTG_HPRT_PSPD_LS))
		reg |= OTG_HCCHAR_LSDEV;

	// Special case settings for IN endpoints
	if (is_in_dir)
		reg |= OTG_HCCHAR_EPDIR;

	// Special case settings for INTR endpoints
	if (ept_type == ENDPOINT_TYPE_INTERRUPT)
		reg |= OTG_HCCHAR_ODDFRM;

	otg->HC_REGS[ept_indx].HCCHAR = reg;


	//configure endpoint
	if (is_in_dir)
		epdir = &drv_info->drv_data->endpoints[ept_indx>>1].epd_out;
	else
		epdir = &drv_info->drv_data->endpoints[ept_indx>>1].epd_in;
	usb_drv_end_transfers(epdir, USBD_STATUS_RESET);
	epdir->epd_state = ENDPOINT_STATE_IDLE;
	epdir->epd_fifo_sz = pDescriptor->wMaxPacketSize;
	epdir->epd_flags = 0;
	epdir->epd_type = ept_type;
#if USB_HOST_KEEPS_CHANNELS_DOWN
	epdir->epd_halt_reason = EPT_HALT_IDLE;
	reset_error_cnt();
#endif
}

static void stm_otg_core_init_host(USB_DRV_INFO drv_info)
{
	USB_TypeDef* otg = drv_info->hw_base;
	uint32_t port_config, port_status, new_phy_clk;

	// request session now
	TRACE_USB_NAME(drv_info);
	TRACE1_USB(" init host");

	// enable host specific interrupts
	otg->core_regs.GINTMSK |= OTG_GINTMSK_WUIM | OTG_GINTMSK_SRQIM /*| OTG_GINTMSK_DISCINT*/
		| OTG_GINTMSK_CIDSCHGM | OTG_GINTMSK_HCIM | OTG_GINTMSK_PRTIM |	OTG_GINTMSK_IPXFRM
		| OTG_GINTMSK_RXFLVLM | OTG_GINTMSK_SOFM | OTG_GINTMSK_OTGINT | OTG_GINTMSK_MMISM;

	/* Enable VBUS driving */
	usb_otg_set_flags(drv_info, USB_OTG_FLG_HOST_PWR);

	tsk_sleep(200);
	otg->core_regs.GINTSTS = OTG_GINTMSK_DISCINT;
	otg->core_regs.GINTMSK |= OTG_GINTMSK_DISCINT;

	// Enable VBUS sensing
	port_config = drv_info->cfg->stm32_otg;
	if ((port_config & CFG_STM32_OTG_VBUS_SENS) && !gd_is_used)
		port_status = OTG_GCCFG_PWRDWN | OTG_GCCFG_VBUSASEN;
	else
		port_status = OTG_GCCFG_PWRDWN | OTG_GCCFG_NOVBUSSENS;

	if (port_config & CFG_STM32_OTG_SOF_OUTPUT)
	{
		port_status |= OTG_GCCFG_SOFOUTEN;
	}
	drv_info->hw_base->core_regs.GCCFG = port_status;

	// reset devices
	for(int i=0; i<= MAX_HUB_PORTS; i++)
	{
		usb_hub_port_t* port;

		port = &drv_info->drv_data->usb_hub[i];
		port->dev_adress = 0;
		port->dev_interface = 0;
	}

#if USB_HOST_KEEPS_CHANNELS_DOWN
	Endpoint* eptr = drv_info->drv_data->endpoints;
	for(int i=0; i< USB_NUMENDPOINTS; i++)
	{
		if(eptr[i].rxfifo_buff)
			delete eptr[i].rxfifo_buff;
		eptr[i].rxfifo_buff = nullptr;
	}
#endif
	drv_info->drv_data->ept_halt_flags =0;
	// if port is enabled check if phy clock speed is OK
	port_status = otg->HPRT;
	port_config = otg->host_regs.HCFG;
	new_phy_clk = 0;
	if( (port_status & OTG_HPRT_PSPD_Msk) == OTG_HPRT_PSPD_LS)
	{
		// Set the Host Frame Interval Register for the 6KHz speed
		otg->host_regs.HFIR = 6000;

		// Are we switching from FS to LS?
		if((port_config & OTG_HCFG_FSLSPCS_Msk) != OTG_HCFG_FSLSPCS_6)
		{
			// configure for LS
			new_phy_clk = OTG_HCFG_FSLSPCS_6;
		}
		TRACE1_USB(" LOW_SPEED");
	} else
	{
		otg->host_regs.HFIR = 48000;

		// Are we switching from LS to FS?

		if((port_config & OTG_HCFG_FSLSPCS_Msk) != OTG_HCFG_FSLSPCS_48)
		{
			// configure for FS
			new_phy_clk = OTG_HCFG_FSLSPCS_48;
		}
		TRACE1_USB(" FULL_SPEED");
	}

	if(new_phy_clk)
	{
		TRACE_USB(" phy_clk=%x", new_phy_clk);

		//change phy clock
		otg->host_regs.HCFG = (port_config & ~OTG_HCFG_FSLSPCS_Msk) | new_phy_clk;
	}

	// Flush the FIFOs are flushed.
	stm_flush_tx_fifo(otg, 0x10);
	stm_flush_rx_fifo(otg);

	// Clear all HC Interrupts
	for (int i = 0; i < OTG_MAX_FIFOS; i++)
	{
		otg->HC_REGS[i].HCDMA = -1u;
		otg->HC_REGS[i].HCINTMSK = 0;
	}

	// set Rx FIFO size
	otg->core_regs.GRXFSIZ = 0x80;
	otg->core_regs.DIEPTXF0_HNPTXFSIZ = OTG_HNPTXFSIZ_NPTXFD(0x60) | 0x80;
	otg->core_regs.HPTXFSIZ			  = OTG_HPTXFSIZ_PTXFD(0x40)   | (0x80+0x60);

}

static void FAST_FLASH stm_host_write_payload(USB_DRV_INFO drv_info, uint32_t ept_indx)
{
    ep_dir_state_t* epdir;
    HANDLE hnd;
	uint32_t len, i;
    __IO uint32_t *fifo;

	epdir = &(drv_info->drv_data->endpoints[ept_indx].epd_in);
	hnd = epdir->epd_pending;
	if(hnd)
	{
		len = hnd->len;
		if(len)
		{
			fifo = drv_info->hw_base->DFIFO[ept_indx << 1].DFIFO;

			// For OUT transfers we will use single packets only on order to handle the NAKs properly
			if (len > epdir->epd_fifo_sz)
				len = epdir->epd_fifo_sz;

			// check if there is enough space in the FIFO
			i = drv_info->hw_base->core_regs.GNPTXSTS;
			if( (OTG_GNPTXSTS_NPTXFSAV_Msk & i) >= (len+3)/4 // TxFIFO space available
					&& (OTG_GNPTXSTS_NPTQXSAV_Msk & i) )	 // request queue space available
			{

				TRACE_USB(" Wr%d(%d) ", ept_indx*2 , len);
#if USB_DATA_TRACE
				TRACE1_USB("\e[32m");			//TC_TXT_GREEN
				for(i=0; i<len; i++)
					TRACE_USB(" %02X", hnd->src.as_byteptr[i]);
				TRACE1_USB("\e[m");
#endif
				len = (len +3) >> 2;
				for(i=0; i<len; i++)
					fifo[0] = hnd->src.as_intptr[i];
			}else
			{
				TRACE1_USB("\e[1;91m");			// RED
				TRACE_USB(" Wr%d(%d) ", ept_indx*2 , len);
				TRACE1_USB("\e[m");
				stm_flush_tx_fifo(drv_info->hw_base, ept_indx);
				do{
					epdir->epd_pending = hnd->next;
					usr_usb_HND_SET_STATUS(hnd, FLG_SIGNALED | RES_FATAL);
					hnd = epdir->epd_pending;
				}while(hnd);
				epdir->epd_state = ENDPOINT_STATE_IDLE;
#if USB_HOST_KEEPS_CHANNELS_DOWN
				epdir->epd_halt_reason = EPT_HALT_IDLE;
#endif
			}

		}

	} else
	{
		TRACELN1_USB("usb tx emp !h");
	}
}

static void FAST_FLASH stm_host_start_xfer(USB_DRV_INFO drv_info, HANDLE hnd, uint32_t eptnum, ep_dir_state_t* epdir)
{
	uint32_t reg;
	uint32_t dpid;
	OTG_HC_REGS* hc_regs = &drv_info->hw_base->HC_REGS[eptnum];
	uint32_t len;

#if TRACE_USB_LEVEL >= TRACE_LEVEL_DEBUG
	if(hc_regs->HCCHAR & OTG_HCCHAR_EPDIR)
		TRACELN1_USB("\e[93m");
	else
		TRACELN1_USB("\e[92m");

/*
	reg = (hc_regs->HCCHAR & OTG_HCCHAR_EPTYP_Msk) >> 18;
	switch(reg)
	{
	case 0:
		TRACE1_USB("ctr");
		break;
	case 1:
		TRACE1_USB("iso");
		break;
	case 2:
		TRACE1_USB("bulk");
		break;
	case 3:
		TRACE1_USB("intr");
		break;
	}
	TRACE_USB(":%u", eptnum);
*/
	TRACE_USB("%u", eptnum);
#endif
	// Program the transfer size and packet count
	len = hnd->len;
	if(epdir->epd_state == ENDPOINT_STATE_RECEIVING)
	{
		if (len > epdir->epd_fifo_sz)
		{
			reg = (len + epdir->epd_fifo_sz - 1) / epdir->epd_fifo_sz;
		} else
		{
			reg = 1; //PKTCNT = 1
		}
	} else
	{
		// For OUT transfers we will use single packets only on order to handle the NAKs properly
		reg = 1;
		if (len > epdir->epd_fifo_sz)
			len = epdir->epd_fifo_sz;
	}
	TRACE_USB("/%u", len);

	//Handle data toggle
	if(epdir->epd_type == ENDPOINT_TYPE_CONTROL)
	{
		TRACE1_USB(" pid");
		if(epdir->epd_state == ENDPOINT_STATE_RECEIVING)
		{
			// The control receive always starts with DATA1 then toggles
			dpid = OTG_HCTSIZ_DPID_DATA1;
			TRACE1_USB("1");
		} else
		{
			// Control sending starts with MDATA, this sends SETUP + DATA0
			// then DATA1 (the core will toggle the packets if we do a single write
			if(hnd->mode1)	// mode1 = start of setup (use MDATA PID)
			{
				dpid = OTG_HCTSIZ_DPID_MDATA;
				TRACE1_USB(" setup");
			}
			else
			{
				dpid = OTG_HCTSIZ_DPID_DATA1;
				TRACE1_USB("1");
			}
		}

	} else
	{
		TRACE1_USB(" d");
		// By default start with DATA0 then toggles
		if(epdir->epd_flags & EPD_FLAG_DATA1) // start
		{
			dpid = OTG_HCTSIZ_DPID_DATA1;
			TRACE1_USB("1");
		}
		else
		{
			dpid = OTG_HCTSIZ_DPID_DATA0;
			TRACE1_USB("0");
		}
	}
	TRACE1_USB("\e[m");
	hc_regs->HCTSIZ = OTG_HCTSIZ_XFRSIZ(len) |	OTG_HCTSIZ_PKTCNT(reg) | dpid;

	// Program the HCCHAR register
	reg = hc_regs->HCCHAR;
	reg &= ~(OTG_HCCHAR_ODDFRM | OTG_HCCHAR_CHDIS | OTG_HCCHAR_DAD_Msk);
	reg |= OTG_HCCHAR_DAD(drv_info->drv_data->usb_hub[hnd->mode0].dev_adress);

	if( !(drv_info->hw_base->host_regs.HFNUM & 1))
		reg |= OTG_HCCHAR_ODDFRM;

#if USB_HOST_KEEPS_CHANNELS_DOWN
	if(epdir->epd_halt_reason == EPT_HALT_IDLE) // if not in IDLE state, keep state and error counter
	{
		reset_error_cnt();
		epdir->epd_halt_reason = EPT_HALT_BUSY;
	}
#endif
	hc_regs->HCCHAR = reg | OTG_HCCHAR_CHENA;

#if USE_CPU_SLEEP_MODE
	locked_inc_int(&cpu_sleep_counter);
	HND_ERROR_SET_LB(hnd, RES_BUSY);
#endif
}

static void FAST_FLASH stm_host_start_tx(USB_DRV_INFO drv_info, HANDLE hnd, uint32_t eptnum, ep_dir_state_t* epdir)
{
	// The host uses one transmit FIFO for all non-periodic (control and bulk) OUT transactions
	// and one transmit FIFO for all periodic (isochronous and interrupt) OUT transactions
	// In order to handle the NAKs however we should not write more than a single packet for each channel
	// (unless we use DMA)
	// Therefore if the FIFO size is programmed to sustain the sum of maxpackets for all the endpoints,
	// there will be no need to check the FIFOs

	stm_host_start_xfer(drv_info, hnd, eptnum*2, epdir);

	// Write some data
	stm_host_write_payload(drv_info, eptnum);
}

static void FAST_FLASH stm_host_ch_halt(USB_DRV_INFO drv_info, OTG_HC_REGS* ch_regs)
{
	uint32_t avail, hcchar;

	// Check for space in the request queue to issue the halt.
	/*
	 * "Before disabling a channel, the application must ensure that there is at
	 *  least one free space available in the non-periodic request queue (when
	 *  disabling a non-periodic channel) or the periodic request queue (when
	 *  disabling a periodic channel). The application can simply flush the
	 *  posted requests when the Request queue is full (before disabling the
	 *  channel), by programming the OTG_FS_HCCHARx register with the CHDIS bit
	 *  set to 1, and the CHENA bit cleared to 0.
	 */

	switch(ch_regs->HCCHAR & OTG_HCCHAR_EPTYP_Msk)
	{
	case OTG_HCCHAR_EPTYP_CTL:
	case OTG_HCCHAR_EPTYP_BULK:
		avail = drv_info->hw_base->core_regs.GNPTXSTS & OTG_GNPTXSTS_NPTQXSAV_Msk;
		break;
	default:
		avail = drv_info->hw_base->host_regs.HPTXSTS & OTG_HPTXSTS_PTXQSAV_Msk;
		break;
	}

	hcchar = ch_regs->HCCHAR | OTG_HCCHAR_CHDIS | OTG_HCCHAR_CHENA;
	if(avail == 0)
	{
		// Request queue is full... we need to flush it with CHDIS=1, CHENA=0
		hcchar &= ~OTG_HCCHAR_CHENA;
	}
	ch_regs->HCCHAR = hcchar;
    //mark the channel as pending to halt
    usb_host_start_halt_tout(drv_info, ch_regs - drv_info->hw_base->HC_REGS);

//	ch_regs->HCINTMSK |= OTG_HCINTMSK_CHHM;
}

#if USB_HOST_KEEPS_CHANNELS_DOWN
static void FAST_FLASH usb_a_ch_halt(USB_DRV_INFO drv_info, uint32_t ch_indx, uint32_t interrupt, halt_reason state)
{
	OTG_HC_REGS* ch_regs = &drv_info->hw_base->HC_REGS[ch_indx];
	//1. enable channel halt interrupt
    ch_regs->HCINTMSK |= OTG_HCINTMSK_CHHM;
    //2. halt channel
    stm_host_ch_halt(drv_info, ch_regs);
    //3. clear channel interrupt
    ch_regs->HCINT = interrupt;
    //4. set reason to halt
	if(ch_regs->HCCHAR &  OTG_HCCHAR_EPDIR)
		drv_info->drv_data->endpoints[ch_indx >> 1].epd_out.epd_halt_reason = state;
	else
		drv_info->drv_data->endpoints[ch_indx >> 1].epd_in.epd_halt_reason = state;
}
#endif //USB_HOST_KEEPS_CHANNELS_DOWN

#endif // USB_ENABLE_HOST

//---------------  USB driver related functions ------------------------------//
#if USB_ENABLE_DEVICE
WEAK_C void usb_drv_event(USB_DRV_INFO drv_info, USB_EVENT event)
{
	USB_DRIVER_DATA* drv_data = drv_info->drv_data;

	switch(event)
	{
	case e_powered:
		drv_data->usb_state = USBST_DEVICE_POWERED;
		break;

	case e_susppend:
		if(drv_data->usb_state > USBST_DEVICE_SUSPENDED)
		{
			drv_data->usb_previous_state = drv_data->usb_state;
			drv_data->usb_state = USBST_DEVICE_SUSPENDED;
		}
		break;

	case e_resume:
		if(drv_data->usb_state < USBST_DEVICE_DEFAULT)
			drv_data->usb_state = drv_data->usb_previous_state;
		break;

	case e_reset:
		drv_data->usb_state = USBST_DEVICE_DEFAULT;
		break;

	case e_disconnect:
		drv_data->usb_state = USBST_ALL_DOWN;
		break;

	default: ;
	break;
	}
}
#endif //USB_ENABLE_DEVICE

/**
 * Called once after boot (on DCR_RESET) to reset the driver
 * @param drv_info
 */
void usb_drv_reset(USB_DRV_INFO drv_info)
{
	TRACE_USB_NAME(drv_info);
	TRACE1_USB(" Rst");
	gd_is_used = (MCU_JEP106_GD == mcu_jep106());
    //set priority
    NVIC->NVIC_IPR[drv_info->info.drv_index] = drv_info->info.isr_priority;
    NVIC->NVIC_ICER[drv_info->info.drv_index >> 5] = 1 << (drv_info->info.drv_index & 0x1F);
	if(drv_info->info.peripheral_indx != ID_NO_PERIPH)
	{
    	Task* task;

		task = usr_task_create_dynamic(drv_info->usb_name, (TASK_FUNCTION)usbdrv_thread,
				60,	500+TRACE_SIZE);
		if(task)
		{
			drv_info->drv_data->helper_task = task;
			svc_task_schedule(task);
           	task->sp->r0.as_cvoidptr = drv_info;
		}
	}
}

void usb_drv_cancel_hnd(USB_DRV_INFO drv_info, HANDLE hnd)
{
    ep_dir_state_t* epdir;

    if(hnd->res & FLG_BUSY)
    {
#if USB_ENABLE_HOST
		if(drv_info->drv_data->otg_flags & USB_OTG_FLG_HOST_CON)
		{
	    	if(hnd->cmd & FLAG_WRITE)
	    	{
	    		TRACELN_USB("\a\aCAN! w(%u)", hnd->mode.as_bytes[1]);
	    	} else
	    	{
	    		TRACELN_USB("\a\aCAN! r(%u)", hnd->mode.as_bytes[0]);
	    	}
			hnd->cmd &= ~(FLAG_READ | FLAG_WRITE);	//request cancel
			return;
		}
#endif
    	if(hnd->cmd & FLAG_WRITE)
    	{
    		epdir = &(drv_info->drv_data->endpoints[hnd->mode.as_bytes[1]].epd_in);
    	} else
    	{
    		epdir = &(drv_info->drv_data->endpoints[hnd->mode.as_bytes[0]].epd_out);

    	}
		if(hnd->list_remove(epdir->epd_pending))
		{
			svc_usb_HND_SET_STATUS(hnd, FLG_SIGNALED | (hnd->res & FLG_OK));
//			TRACE1_USB("Can!");
			if(!epdir->epd_pending && epdir->epd_state == ENDPOINT_STATE_SENDING)
				epdir->epd_state = ENDPOINT_STATE_IDLE;

		}
    }
}

/**
 * Start sending...
 *
 * STM32 logic:
 * 	1. Program the transfer size...
 * 	2. Enable endpoint
 * 	3. write some payload data
 * 	4. On tx fifo empty (TXFE) continue to write
 * 	5. On transf. complete (XFRC) signal the client
 *
 * @param drv_info
 * @param hnd
 */
void usb_drv_start_tx(USB_DRV_INFO drv_info, HANDLE hnd)
{
	ep_dir_state_t* epdir;
	uint32_t eptnum;
	HANDLE prev;

	eptnum = hnd->mode.as_bytes[1];
//    TRACE_USB(" Write%d(%d) ", eptnum , hnd->len);
	epdir = &(drv_info->drv_data->endpoints[eptnum].epd_in);
    if( (prev=epdir->epd_pending) )
    {
    	while(prev->next)
    		prev = prev->next;
    	prev->next = hnd;
    } else
    {
	    if (epdir->epd_state != ENDPOINT_STATE_IDLE)
	    {
			svc_HND_SET_STATUS(hnd, FLG_SIGNALED | RES_FATAL);
			TRACELN_USB(" Write%d fail (st=%02x) ", eptnum , epdir->epd_state);
	    } else
	    {
	    	epdir->epd_state = ENDPOINT_STATE_SENDING;
		    epdir->epd_pending = hnd;		//sending

#if USB_ENABLE_HOST
		    //host mode?
		    if(drv_info->drv_data->otg_flags & USB_OTG_FLG_HOST_CON)
		    {
		    	stm_host_start_tx(drv_info, hnd, eptnum, epdir);
		    } else
#endif
		    {
#if USB_ENABLE_DEVICE
		    	stm_start_tx(drv_info, hnd, eptnum, epdir);
#endif
		    }
	    }
    }
}

void usb_drv_start_rx(USB_DRV_INFO drv_info, HANDLE hnd)
{
	uint32_t eptnum;
	Endpoint *endpoint;

	eptnum = hnd->mode.as_bytes[0];
	endpoint = &drv_info->drv_data->endpoints[eptnum];

	if(endpoint->epd_out.epd_state == ENDPOINT_STATE_RECEIVING_OFF)
	{
#if USB_ENABLE_HOST && USB_HOST_KEEPS_CHANNELS_DOWN
		//host mode?
		if(drv_info->drv_data->otg_flags & USB_OTG_FLG_HOST_CON)
		{
			host_buffer_to_hnd(endpoint, hnd);
		}else
#endif	// USB_ENABLE_HOST	&& USB_HOST_KEEPS_CHANNELS_DOWN
		{
			uint32_t bcnt, top_rx_cnt;

			bcnt = endpoint->rxfifo_cnt;
			top_rx_cnt = endpoint->top_rx_cnt;
			__IO uint32_t *fifo = drv_info->hw_base->DFIFO[0].DFIFO;

			while (hnd->len)
			{
				if (!top_rx_cnt)
				{
					// try whole words
					while (hnd->len > 3 && bcnt > 3)
					{
						*hnd->dst.as_intptr++ = *fifo;
						hnd->len -= 4;
						bcnt -= 4;
					}

					if (!bcnt)
					{
						// nothing left in fifo or top
						break;
					}

					// bcnt is >0 and hnd->len is <4
					if (hnd->len)
					{
						// len !=0 -> we must pad
						endpoint->top_rx_word = *fifo;
						top_rx_cnt = (bcnt > 3) ? 4 : bcnt;
						bcnt -= top_rx_cnt;
					}
				}

				//process the top word
				while (hnd->len && top_rx_cnt)
				{
					*hnd->dst.as_byteptr++ = endpoint->top_rx_word;
					endpoint->top_rx_word >>= 8;
					top_rx_cnt--;
					hnd->len--;
				}
			}
			endpoint->rxfifo_cnt = bcnt;
			endpoint->top_rx_cnt = top_rx_cnt;
			if( !(bcnt + top_rx_cnt) )
			{
				endpoint->epd_out.epd_state = ENDPOINT_STATE_IDLE;
				drv_info->hw_base->core_regs.GINTMSK |= OTG_GINTMSK_RXFLVLM;
			}
		}
		svc_HND_SET_STATUS(hnd, RES_SIG_OK);
	} else
	{
#if USB_ENABLE_OTG
	    if (endpoint->epd_out.epd_state == ENDPOINT_STATE_DISABLED)
	    {
			svc_HND_SET_STATUS(hnd, FLG_SIGNALED | RES_FATAL);
	    } else
#endif
	    {
			HANDLE prev;

			if( (prev=endpoint->epd_out.epd_pending) )	//receiving
			{
				while(prev->next)
					prev = prev->next;
				prev->next = hnd;
			} else
			{
				endpoint->epd_out.epd_pending = hnd;	//receiving
#if USB_ENABLE_HOST
				//host mode?
				if(drv_info->drv_data->otg_flags & USB_OTG_FLG_HOST_CON)
				{
					hnd->mode1 = 0; //TODO
					if(endpoint->epd_out.epd_state == ENDPOINT_STATE_IDLE )
					{
						endpoint->epd_out.epd_state = ENDPOINT_STATE_RECEIVING;
						stm_host_start_xfer(drv_info, hnd, eptnum*2+1, &endpoint->epd_out);
					} else
					{
						svc_HND_SET_STATUS(hnd, FLG_SIGNALED | RES_FATAL);
						endpoint->epd_out.epd_pending = nullptr;
						TRACELN_USB(" Read%d fail (st=%02x) ", eptnum , endpoint->epd_out.epd_state);
					}
				} else
#endif
				{
#if USB_ENABLE_DEVICE
					if(drv_info->drv_data->usb_state > USBST_DEVICE_SUSPENDED)
					{
						if(hnd->len == 0)
						{
							svc_HND_SET_STATUS(hnd, FLG_SIGNALED | RES_OK);
							endpoint->epd_out.epd_pending = nullptr;
						} else
						{
							stm_start_rx(drv_info, eptnum, endpoint);
							endpoint->epd_out.epd_state = ENDPOINT_STATE_RECEIVING;
						}
					} else
					{
						svc_HND_SET_STATUS(hnd, FLG_SIGNALED | RES_FATAL);
						endpoint->epd_out.epd_pending = nullptr;
					}
#endif
				}
			}
	    }
	}

}

void usb_drv_end_transfers(ep_dir_state_t* epdir, unsigned int status)
{
    HANDLE hnd;

	while( (hnd =  epdir->epd_pending) )
	{
		TRACE1_USB(" EoT ");
		epdir->epd_pending = hnd->next;
		if (__get_IPSR() == 11){
			// SVCall
			svc_usb_HND_SET_STATUS(hnd, status);
		}else{
			usr_usb_HND_SET_STATUS(hnd, status);
		}
	}
}

//------------------  HAL related functions ----------------------------------//

bool usb_hal_get_ep_status(USB_DRV_INFO drv_info, uint8_t ept_num, uint16_t* data)
{
	uint32_t ept_indx = ept_num & 0xF;

	// Check if the endpoint exists
    if (ept_indx  > USB_NUMENDPOINTS)
    {
    	return false;
    }
    else
    {
        // Check if the endpoint if currently halted
    	if(ept_num & 0x80)
    	{
    		// IN direction
    		ept_indx = drv_info->drv_data->endpoints[ept_indx].epd_in.epd_state;
    	} else
    	{
    		ept_indx = drv_info->drv_data->endpoints[ept_indx].epd_out.epd_state;
    	}
        if (ept_indx  == ENDPOINT_STATE_HALTED)
        {
            *data = 1;
        } else
        {
        	*data = 0;
        }

    }
    return true;
}


void usb_hal_stall(USB_TypeDef* hw_base, unsigned int ept_num)
{
	uint32_t ept_indx = ept_num & 0xF;

	if (ept_num & 0x80)
	{
		uint32_t value;
		__IO uint32_t* reg;

		reg = &hw_base->in_ept_regs[ept_indx].DIEPCTL;

		value = *reg;
		if(value & OTG_DIEPCTL_EPENA)
			value |= OTG_DIEPCTL_EPDIS;
		value |= OTG_DIEPCTL_STALL;

		*reg = value;
	} else
	{
		hw_base->out_ept_regs[ept_indx].DOEPCTL |= OTG_DOEPCTL_STALL;
	}

}

void usb_hal_stall_clear(USB_TypeDef* hw_base, unsigned int ept_num)
{
	uint32_t value;
	__IO uint32_t* reg;
	uint32_t ept_indx = ept_num & 0xF;

	if (ept_num & 0x80)
	{
		reg = &hw_base->in_ept_regs[ept_indx].DIEPCTL;
	} else
	{
		reg = &hw_base->out_ept_regs[ept_indx].DOEPCTL;

	}
	value = *reg;
	value &= ~OTG_DIEPCTL_STALL;

	if(ept_num >0) // this should be for interrupt/bulk
		value |= OTG_DIEPCTL_SD0PID; // set endpoint data PID to DATA0

	*reg = value;
}


void usb_hal_ept_reset(USB_DRV_INFO drv_info, unsigned int ept_num)
{
	USB_TypeDef* otg = drv_info->hw_base;
	uint32_t ept_indx = ept_num & 0xF;
	ep_dir_state_t* epdir;

	// - clear status and flush
	if(ept_num & 0x80)
	{
        epdir = &drv_info->drv_data->endpoints[ept_indx].epd_in;
		otg->in_ept_regs[ept_indx].DIEPINT = 0xFF;
		stm_flush_tx_fifo(otg, ept_indx);

		// - disable ints...
		otg->device_regs.DAINT = OTG_DAINT_IEPINT(ept_indx);
		otg->device_regs.DAINTMSK &= ~OTG_DAINTMSK_IEPM(ept_indx);
	} else
	{
        epdir = &drv_info->drv_data->endpoints[ept_indx].epd_out;
		otg->out_ept_regs[ept_indx].DOEPINT = 0xFF;

		// - disable ints...
		otg->device_regs.DAINT = OTG_DAINT_OEPINT(ept_indx);
		otg->device_regs.DAINTMSK &= ~OTG_DAINTMSK_OEPM(ept_indx);
	}


	// - cancel transfers...
	usb_drv_end_transfers(epdir, USBD_STATUS_RESET);
    epdir->epd_state = ENDPOINT_STATE_DISABLED;


}

void usb_hal_config_fifo(USB_DRV_INFO drv_info)
{
	uint32_t adr, size;
	USB_TypeDef* otg = drv_info->hw_base;
	USB_DRIVER_DATA* drv_data = drv_info->drv_data;


	/* EP0 TX*/
	adr = drv_info->cfg->rx_fifo_sz;
	size = drv_data->endpoints[0].epd_in.epd_fifo_sz;
	TRACELN_USB("0:%x -> %x", otg->core_regs.DIEPTXF0_HNPTXFSIZ, adr + OTG_DIEPTXF0_TX0FD(size));
	otg->core_regs.DIEPTXF0_HNPTXFSIZ = adr + OTG_DIEPTXF0_TX0FD(size);

	/* EP1 - EP5 TX*/
	for(uint32_t i=1; i<USB_NUMENDPOINTS; i++)
	{
		adr += size;
		size = drv_data->endpoints[i].epd_in.epd_fifo_sz;
		TRACELN_USB("%u:%x -> %x", i, otg->core_regs.DIEPTXF[i-1], adr + OTG_DIEPTXF0_TX0FD(size));
		otg->core_regs.DIEPTXF[i-1] = adr + OTG_DIEPTXF0_TX0FD(size);
	}

}

#if USB_ENABLE_HOST
WEAK_C void usb_hal_host_power(USB_DRV_INFO drv_info, bool enable)
{
	unsigned int reg;

	reg = drv_info->hw_base->HPRT;
	reg &= ~(OTG_HPRT_rc_w1_bits);
	if(enable)
	{
		TRACE1_USB(" USB pwr en");
		reg |= OTG_HPRT_PPWR;
		// interrupts?
	} else
	{
		TRACE1_USB(" USB pwr dis");
	}
	drv_info->hw_base->HPRT = reg;
}

RES_CODE usb_hal_host_start(USB_DRV_INFO drv_info)
{
	USB_TypeDef* otg = drv_info->hw_base;


	// init core
	TRACE_USB_NAME(drv_info);
	TRACE1_USB(" host");

	// init host
	stm_otg_core_init_host(drv_info);

	if(!(otg->HPRT & OTG_HPRT_PENA))
	{
		tsk_sleep(200);
	}
	return RES_SIG_OK;
}

RES_CODE usb_hal_host_bus_reset(USB_DRV_INFO drv_info)
{
	uint32_t reg;
	USB_TypeDef* otg = drv_info->hw_base;
	RES_CODE res = RES_ERROR;

	TRACE_USB_NAME(drv_info);
	TRACE1_USB(" start reset");

	stm_otg_core_init_host(drv_info);

	reg = otg->HPRT & ~OTG_HPRT_rc_w1_bits;
	otg->HPRT = reg | OTG_HPRT_PRST;	// assert reset
	tsk_sleep(20);

	TRACE_USB_NAME(drv_info);
	TRACE1_USB(" end reset");
	reg = otg->HPRT & ~(OTG_HPRT_rc_w1_bits | OTG_HPRT_PRST);
	otg->HPRT = reg;	// deassert reset
	tsk_sleep(10);

	if(otg->HPRT & OTG_HPRT_PENA)
		res = RES_OK;

	return res;
}

void usb_hal_host_resume(USB_DRV_INFO drv_info)
{
	uint32_t reg;

	drv_info->hw_base->PCGCCTL = 0;	//Ungate clocks
	reg = drv_info->hw_base->HPRT & ~OTG_HPRT_rc_w1_bits;
	drv_info->hw_base->HPRT = reg | OTG_HPRT_PRES;	// drive resume signaling
	tsk_sleep(20);
	reg = drv_info->hw_base->HPRT & ~OTG_HPRT_rc_w1_bits;
	drv_info->hw_base->HPRT = reg & ~OTG_HPRT_PRES;	// stop driving

}
#endif // USB_ENABLE_HOST

#if USB_ENABLE_OTG
void usb_otg_clr_flags(USB_DRV_INFO drv_info, uint32_t flags)
{
    USB_DRIVER_DATA* drv_data = drv_info->drv_data;

	// disconnect
	if(flags & (USB_OTG_FLG_HOST | USB_OTG_FLG_DEV | USB_OTG_FLG_HOST_PWR))
	{
		// cancel all transfers
		drv_data->drv_state_cnt  |= USB_STATE_CNT_INVALID;
		for(int i=0; i<USB_NUMENDPOINTS; i++)
		{
			usb_drv_end_transfers(&drv_data->endpoints[i].epd_in, USBD_STATUS_RESET);
			usb_drv_end_transfers(&drv_data->endpoints[i].epd_out, USBD_STATUS_RESET);
		}
		if(drv_data->pending)
		{
			HANDLE hnd;
			while( (hnd=drv_data->pending) )
			{
				drv_data->pending = hnd->next;
				usr_usb_HND_SET_STATUS(hnd, FLG_SIGNALED | RES_FATAL);
			}
			atomic_clrex();
		}
	}

#if USB_ENABLE_HOST
	if(flags & USB_OTG_FLG_HOST)
		flags |= USB_OTG_FLG_HOST_CON | USB_OTG_FLG_HOST_PWR
				| USB_OTG_FLG_HOST_RST | USB_OTG_FLG_HOST_OK | USB_OTG_FLG_WKUP
				| USB_OTG_FLG_SUSPEND;
#endif // USB_ENABLE_HOST

	if(flags & USB_OTG_FLG_DEV)
		flags |= USB_OTG_FLG_DEV_CON | USB_OTG_FLG_DEV_OK | USB_OTG_FLG_WKUP
				| USB_OTG_FLG_SUSPEND;

	flags &= drv_data->otg_flags;
	drv_data->otg_flags &= ~flags;

#if USB_ENABLE_HOST
	// host power
	if(flags & USB_OTG_FLG_HOST_PWR)
	{
		drv_info->hw_base->core_regs.GINTMSK = OTG_GINTMSK_WUIM | OTG_GINTMSK_CIDSCHGM |
				OTG_GINTMSK_RXFLVLM | OTG_GINTMSK_OTGINT | OTG_GINTMSK_SRQIM;
		usb_hal_host_power(drv_info, false);

		//disable isrs
	    NVIC->NVIC_ICER[drv_info->info.drv_index >> 5] = 1 << (drv_info->info.drv_index & 0x1F);

	}
#endif // USB_ENABLE_HOST

	// device con
	if(flags & USB_OTG_FLG_DEV_CON)
	{
        // Abort all transfers
    	for(int i= 1; i<USB_NUMENDPOINTS; i++)
    	{
    		usb_hal_ept_reset(drv_info, i);
    	}
	}

}

void usb_otg_set_flags(USB_DRV_INFO drv_info, uint32_t flags)
{
    USB_DRIVER_DATA* drv_data = drv_info->drv_data;

	// do not set twice
	flags &= ~drv_data->otg_flags;
	if(flags)
	{
#if USB_ENABLE_HOST
		if(flags >= USB_OTG_FLG_HOST )
		{
			// host flags
			if(drv_data->otg_flags & USB_OTG_FLG_HOST)
			{
				usb_otg_clr_flags(drv_info, USB_OTG_FLG_DEV);

				switch(flags)
				{
				case USB_OTG_FLG_HOST_PWR:
					usb_hal_host_power(drv_info, true);
					flags |= USB_OTG_FLG_HOST_CON | USB_OTG_FLG_HOST_PWR;
					break;

				case USB_OTG_FLG_HOST_RST:
					TRACE1_USB(" HRST!");
					if(drv_data->helper_task)
					{
						int sig;
						do
						{
							sig = atomic_fetch((volatile int*)&drv_data->otg_h_sig);
							sig |= OTG_H_SIG_RST;
						} while(atomic_store((volatile int*)&drv_data->otg_h_sig, sig));
						usr_send_signal(drv_data->helper_task, USB_DRIVER_SIG);
					}
					break;

				case USB_OTG_FLG_HOST_OK:
					HANDLE hnd;
					__disable_irq();
					while( (hnd=drv_data->pending) )
					{
						drv_data->pending = hnd->next;
						__enable_irq();
						usr_usb_HND_SET_STATUS(hnd, FLG_SIGNALED | RES_FATAL);
						__disable_irq();
					}

					drv_data->drv_state_cnt += 2;
					drv_data->drv_state_cnt &= 0x7FFE;
					break;

				}

				drv_info->drv_data->otg_flags |= flags;
				__enable_irq();
			}
		} else
#endif // USB_ENABLE_HOST
		{
			//device flags
			if(drv_info->drv_data->otg_flags )
			{
				usb_otg_clr_flags(drv_info, USB_OTG_FLG_HOST);

				switch(flags)
				{
				case USB_OTG_FLG_DEV_OK:
					drv_data->drv_state_cnt += 2;
					drv_data->drv_state_cnt &= 0x7FFE;
					break;

				case USB_OTG_FLG_DEV_CON:
//					drv_info->hw_base->USBPOWER |= USB_USBPOWER_SOFTCONN;
					break;
				}

				if(drv_data->helper_task)
					usr_send_signal(drv_data->helper_task, USB_DRIVER_SIG);
				drv_info->drv_data->otg_flags |= flags;
			}
		}

	}

}

#endif // USB_ENABLE_OTG



//------------------------------------------------------------------------------
//						DEVICE related interrupts
//------------------------------------------------------------------------------
#if USB_ENABLE_DEVICE
/**
 * Handles out endpoint interrupts, except endpoint 1
 *
 * (DCD_HandleOutEP_ISR)
 * @param drv_info
 */
static void FAST_FLASH usb_b_ept_rx_handler(USB_DRV_INFO drv_info)
{
	USB_TypeDef* otg = drv_info->hw_base;
	uint32_t status, doepint;
	uint32_t ept_indx = 0;


	/* Read in the device interrupt bits */
	status = otg->device_regs.DAINT;
	status &= otg->device_regs.DAINTMSK;
	status >>= 16;

	while (status)
	{
		if (status & 0x1)
		{
			doepint = otg->out_ept_regs[ept_indx].DOEPINT;
			doepint &= otg->device_regs.DOEPMSK;
			/* Clear the bit in DOEPINTn for this interrupt */
			otg->out_ept_regs[ept_indx].DOEPINT = doepint;

	    	TRACE_USB(" [R%x=", ept_indx);
		    TRACE_USB("%02x]", doepint);

			/* Transfer complete */
			if (doepint & OTG_DOEPINT_XFRC)
			{

				/* RX COMPLETE */
				if(drv_info->drv_data->endpoints[ept_indx].epd_out.epd_pending)
				{
			    	TRACE1_USB(" rcv comp");
				} else
				{
			    	TRACE1_USB(" rcv comp!");

				}
			}
			/* Endpoint disable  */
			if (doepint & OTG_DOEPINT_EPDISD)
			{
			}
			/* Setup Phase Done (control EPs) */
			if (doepint & OTG_DOEPINT_STUP)
			{
				/* inform the upper layer that a setup packet is available */
				/* SETUP COMPLETE */
		    	TRACE1_USB(" stp done");
			}
		}
		ept_indx++;
		status >>= 1;
	}
}

#if USB_STM32_DEDICATED_EP1
/**
 * handles EP1_OUT Handler
 * (USBD_OTG_EP1OUT_ISR_Handler)
 * @param drv_info
 */
void USB_EP1_OUT_ISR(USB_DRV_INFO drv_info)
{
	USB_TypeDef* otg = drv_info->hw_base;
	uint32_t status;

	status = otg->out_ept_regs[1].DOEPINT;
	status &= otg->device_regs.DOUTEP1MSK;
	otg->out_ept_regs[1].DOEPINT = status;
	TRACELN_USB("USB1: [out %02X]", status);

	if(status & OTG_DOEPINT_XFRC)		// Transfer completed interrupt
	{
		// upper ...
		TRACELN1_USB("usb 1o xfrc");
	}

	if (status & OTG_DOEPINT_EPDISD) 	// Endpoint disabled interrupt
	{
		TRACELN1_USB("usb 1 dis");
	}
}
#endif

/**
 * Handles out endpoint interrupts, except endpoint 1
 * DCD_HandleInEP_ISR
 * @param drv_info
 */
static void FAST_FLASH usb_b_ept_tx_handler(USB_DRV_INFO drv_info)
{
	USB_TypeDef* otg = drv_info->hw_base;

	uint32_t ep_intr, diepint;
	uint32_t ept_indx = 0;

	ep_intr = otg->device_regs.DAINT & OTG_DAINT_IEPINT_Msk;
	ep_intr &= otg->device_regs.DAINTMSK;
	while (ep_intr)
	{
		if (ep_intr & 0x1) /* In ITR */
		{
			/* Get In ITR status */
			diepint = otg->device_regs.DIEPMSK;
			diepint |= ((otg->device_regs.DIEPEMPMSK >> ept_indx) & 1) << 7;
			diepint &= otg->in_ept_regs[ept_indx].DIEPINT;
			otg->in_ept_regs[ept_indx].DIEPINT = diepint;
	    	TRACE_USB(" [T%x=%02x]", ept_indx, diepint);

			if (diepint & OTG_DIEPINT_XFRC)		// Transfer completed
			{
				ep_dir_state_t* epdir;
			    HANDLE hnd;

				/* TX COMPLETE */
				epdir = &(drv_info->drv_data->endpoints[ept_indx].epd_in);
				hnd = epdir->epd_pending;
				if(hnd)
				{
					if(hnd->len == 0)
					{
				    	TRACE1_USB(" Wr!");
				    	if(hnd->res & FLG_OK)
				    	{
				    		hnd->res &= ~FLG_OK;
				    		// continue with ZLP packet
				    	} else
				    	{
							epdir->epd_pending = hnd->next;
							usr_HND_SET_STATUS(hnd, RES_SIG_OK);
							hnd = epdir->epd_pending;
				    	}
					}
				}
				if(hnd)
				{
					//start next transfer
			    	TRACE1_USB(" Wr");
					stm_start_tx(drv_info, hnd, ept_indx, epdir);
				} else
				{
					epdir->epd_state = ENDPOINT_STATE_IDLE;
				}

			}

			// Transmit FIFO empty
			if (diepint & OTG_DIEPINT_TXFE)
			{
				stm_write_payload(drv_info, ept_indx);
			}
		}
		ept_indx++;
		ep_intr >>= 1;
	}
}

#if USB_STM32_DEDICATED_EP1
/**
 *  USBD_OTG_EP1IN_ISR_Handler
 * @param drv_info
 */
void USB_EP1_IN_ISR(USB_DRV_INFO drv_info)
{
	USB_TypeDef* otg = drv_info->hw_base;
	uint32_t diepint;

	diepint = otg->device_regs.DINEP1MSK;
	diepint |= (otg->device_regs.DIEPEMPMSK & 2) << 6;
	diepint &= otg->in_ept_regs[1].DIEPINT;
	otg->in_ept_regs[1].DIEPINT = diepint;
	TRACELN_USB("USB1: [in %02X]", diepint);

	if(diepint & OTG_DIEPINT_XFRC)	//Transfer completed
	{
		ep_dir_state_t* epdir;
	    HANDLE hnd;

		/* TX COMPLETE */
		epdir = &(drv_info->drv_data->endpoints[1].epd_in);
		hnd = epdir->epd_pending;
		if(hnd)
		{
			if(hnd->len == 0)
			{
		    	TRACE1_USB(" Wr!");
				epdir->epd_pending = hnd->next;
				usr_HND_SET_STATUS(hnd, RES_SIG_OK);
				hnd = epdir->epd_pending;
			}
		}
		if(hnd)
		{
			//start next transfer
	    	TRACE1_USB(" Wr");
			stm_start_tx(drv_info, hnd, 1, epdir);
		} else
		{
			epdir->epd_state = ENDPOINT_STATE_IDLE;
		}

	}


	if(diepint & OTG_DIEPINT_TXFE)		// Transmit FIFO empty
	{
		stm_write_payload(drv_info, 1);
	}
}
#endif

/**
 *  Indicates that the USB_OTG controller has detected a resume or remote Wake-up sequence
 *  (DCD_HandleResume_ISR)
 * @param drv_info
 */
static void FAST_FLASH usb_b_gint_wkupint(USB_DRV_INFO drv_info)
{
	USB_TypeDef* otg = drv_info->hw_base;

	/* Clear interrupt */
	otg->core_regs.GINTSTS = OTG_GINTSTS_WKUPINT;

	if(drv_info->cfg->stm32_otg & CFG_STM32_OTG_LOW_POWER)
	{
		/* un-gate USB Core clock */
		otg->PCGCCTL &= ~(OTG_PCGCCTL_GATEHCLK | OTG_PCGCCTL_STPPCLK);
	}

	/* Clear the Remote Wake-up Signaling */
	otg->device_regs.DCTL &= ~OTG_DCTL_RWUSIG;

	/* Inform upper layer by the Resume Event */
	usb_drv_event(drv_info, e_resume);
	TRACELN1_USB("usb resume");

	otg->device_regs.DCTL |= OTG_DCTL_POPRGDNE;
}

/**
 * 	Indicates that SUSPEND state has been detected on the USB
 * 	(DCD_HandleUSBSuspend_ISR)
 * @param drv_info
 */
static void FAST_FLASH usb_b_gint_usbsusp(USB_DRV_INFO drv_info)
{
	uint32_t prev_state;;
	USB_TypeDef* otg = drv_info->hw_base;


	/* Clear interrupt */
	otg->core_regs.GINTSTS = OTG_GINTSTS_USBSUSP | OTG_GINTSTS_ESUSP;

	prev_state = drv_info->drv_data->usb_state;
	if(prev_state != USBST_DEVICE_SUSPENDED)
	{
		if( (drv_info->cfg->stm32_otg & CFG_STM32_OTG_LOW_POWER) &&
			(otg->device_regs.DSTS & OTG_DSTS_SUSPSTS)  &&
			(prev_state == USBST_DEVICE_CONFIGURED))
		{
			/*  switch-off the clocks */
			otg->PCGCCTL |= OTG_PCGCCTL_STPPCLK;
			otg->PCGCCTL |= OTG_PCGCCTL_GATEHCLK;
		}


//		/* Turn off forced host/peripheral mode  */
//		otg->core_regs.GUSBCFG &= ~(OTG_GUSBCFG_FDMOD | OTG_GUSBCFG_FHMOD);

		usb_drv_event(drv_info, e_susppend);
	}
}


/** Handle USB bus reset
 *
 * @param drv_info
 */
static void usb_b_gint_usbrst(USB_DRV_INFO drv_info)
{
	uint32_t dev_endpoints;
	const USBDDriverDescriptors * dev_descriptors;
	USB_TypeDef* otg = drv_info->hw_base;
	USB_DRIVER_DATA* drv_data = drv_info->drv_data;

	TRACE1_USB(" rstint");

	/* Clear the Remote Wake-up Signaling */
	otg->device_regs.DCTL &= ~OTG_DCTL_RWUSIG;

	/* Flush the Tx FIFO */
	stm_flush_tx_fifo(otg, 0);
	stm_flush_rx_fifo(otg);

	// reset endpoints
	if(drv_info->cfg->stm32_otg & CFG_STM32_OTG_HS_CORE)
	{
	    dev_endpoints    = 6-1;
	} else
	{
	    dev_endpoints    = 4-1;
	}
	for(uint32_t i= 0; i<dev_endpoints; i++)
	{
		usb_hal_ept_reset(drv_info, i | 0x80);
		usb_hal_ept_reset(drv_info, i);
	}

	dev_descriptors = drv_data->device.pDescriptors;

	/*Reset internal state machine */
	usb_drv_event(drv_info, e_reset);

	if(dev_descriptors)
	{
		usb_hal_ept_config(drv_info, &dev_descriptors->pFsDevice->as_generic);
	}

	/* Reset Device Address */
	otg->device_regs.DCFG &= ~ OTG_DCFG_DAD_Msk;


  	/* Clear interrupt */
	drv_info->hw_base->core_regs.GINTSTS = OTG_GINTSTS_USBRST;

#if USB_ENABLE_OTG

	usb_otg_set_flags(drv_info, USB_OTG_FLG_DEV_CON);

	//Step 3. Wakeup clients..
	usb_otg_set_flags(drv_info, USB_OTG_FLG_DEV_OK);
	if(drv_data->pending)
	{
		HANDLE hnd;
		while( (hnd=drv_data->pending) )
		{
			drv_data->pending = hnd->next;
			usr_HND_SET_STATUS(hnd, RES_SIG_IDLE);
		}
		atomic_clrex();
	}
#endif
}

/**
 * Enumeration done interrupt
 * Read the device status register and set the device speed
 * (DCD_HandleEnumDone_ISR)
 * @param drv_info
 */
static void FAST_FLASH usb_b_gint_enumdne(USB_DRV_INFO drv_info)
{
	USB_TypeDef* otg = drv_info->hw_base;
	uint32_t reg, speed;

	// enables EP0 OUT to receive SETUP packets and configures EP0
	speed = otg->device_regs.DSTS & OTG_DSTS_ENUMSPD_Msk;
	reg = otg->in_ept_regs[0].DIEPCTL & ~OTG_DIEPCTL_MPSIZ_Msk;
	if( speed == OTG_DSTS_ENUMSPD_LS_6)
		reg |= OTG_DIEPCTL_MPSIZ(8);
	else
		reg |= OTG_DIEPCTL_MPSIZ(64);
	otg->in_ept_regs[0].DIEPCTL = reg;

	otg->device_regs.DCTL |= OTG_DCTL_CGINAK;

	/* Set USB turn-around time based on device speed and PHY interface. */
	reg = otg->core_regs.GUSBCFG & ~OTG_GUSBCFG_TRDT_Msk;
	if(speed == OTG_DSTS_ENUMSPD_HS_30_60)
	{
		reg |= OTG_GUSBCFG_TRDT(9);
	} else
	{
		reg |= OTG_GUSBCFG_TRDT(5);
	}
	otg->core_regs.GUSBCFG = reg;

	/* Clear interrupt */
	drv_info->hw_base->core_regs.GINTSTS = OTG_GINTSTS_ENUMDNE;

}


/**
 * Handles the Rx Status Queue Level Interrupt
 * (DCD_HandleRxStatusQueueLevel_ISR)
 * @param drv_info
 */
static void FAST_FLASH usb_b_gint_rxflvl(USB_DRV_INFO drv_info)
{
	uint32_t status;
	USB_TypeDef* otg = drv_info->hw_base;


	/* Get the Status from the top of the FIFO */
	status = otg->core_regs.GRXSTSP;

	TRACE_USB("que%u: ", status & OTG_GRXSTSP_EPNUM_Msk);

	switch(status & OTG_GRXSTSP_PKTSTS_Msk)
	{
	case OTG_GRXSTSP_PKTSTS_NAK:
		TRACE1_USB("nak");
		break;

	case OTG_GRXSTSP_PKTSTS_SETUP_DATA:
	case OTG_GRXSTSP_PKTSTS_OUT_DATA:
		if(!stm_read_payload(drv_info, status))
		{
			/* Disable the Rx Status Queue Level interrupt */
			otg->core_regs.GINTMSK &= ~OTG_GINTMSK_RXFLVLM;
		}
		break;

	case OTG_GRXSTSP_PKTSTS_OUT_COMP:
		TRACE1_USB("in comp");
#if USB_ENABLE_HOST
		if(! (drv_info->drv_data->otg_flags & USB_OTG_FLG_HOST_CON) )
#endif
		{
		    uint32_t ept_indx = status & OTG_GRXSTSP_EPNUM_Msk;

		    // restart channel if there is a pending handle
			stm_start_rx(drv_info, ept_indx, &drv_info->drv_data->endpoints[ept_indx]);
		}

		break;

	case OTG_GRXSTSP_PKTSTS_SETUP_COMP:
		TRACE1_USB("stp comp");
		{
		    Endpoint* endpoint;
		    HANDLE hnd;
			endpoint = &drv_info->drv_data->endpoints[status & OTG_GRXSTSP_EPNUM_Msk];
			if(endpoint->epd_out.epd_state == ENDPOINT_STATE_RECEIVING)
			{
				hnd = endpoint->epd_out.epd_pending;
				if(hnd && (hnd->res & FLG_OK))
				{
					endpoint->epd_out.epd_state = ENDPOINT_STATE_IDLE;
					endpoint->epd_out.epd_pending = hnd->next;
					usr_HND_SET_STATUS(hnd, RES_SIG_OK);
					TRACE1_USB("$$");
				}
			}

		}
		break;

	default:
		TRACELN("unk st=%x", status);
		break;
	}

}

/**
 * Indicates that the USB_OTG controller has detected a connection
 * (DCD_SessionRequest_ISR)
 * @param drv_info
 */
static void FAST_FLASH usb_b_gint_srqint(USB_DRV_INFO drv_info)
{
	USB_TypeDef* otg = drv_info->hw_base;

	/* Clear interrupt */
	otg->core_regs.GINTSTS = OTG_GINTSTS_SRQINT;


	stm_otg_ungate_clock(otg, drv_info->cfg->stm32_otg);
	usb_drv_event(drv_info, e_powered);
}

/**
 * Indicates that the USB_OTG controller has detected an OTG event
 *   used to detect the end of session i.e. disconnection
 *
 * (DCD_OTG_ISR)
 * @param drv_info
 */
static void FAST_FLASH usb_b_gint_otgint(USB_DRV_INFO drv_info)
{

	uint32_t otg_int;

	otg_int = drv_info->hw_base->core_regs.GOTGINT;

	if (otg_int & OTG_GOTGINT_SEDET)
	{
		/* Inform upper layer by the Disconnect Event */
		TRACELN1_USB("ses end");
		usb_drv_event(drv_info, e_disconnect);

	}

	/* Clear OTG interrupt */
	drv_info->hw_base->core_regs.GOTGINT = otg_int;

	/* Clear interrupt */
	drv_info->hw_base->core_regs.GINTSTS = OTG_GINTSTS_OTGINT;
}
#endif // USB_ENABLE_DEVICE

#if USB_ENABLE_HOST


void usb_hal_host_nak_tout(USB_DRV_INFO drv_info)
{
	//tout (nak) processing
	for(int i=0; i<USB_NUMENDPOINTS; i++)
	{
		ep_dir_state_t* epdir = &drv_info->drv_data->endpoints[i].epd_in;
		for(int j=0; j<2; j++)
		{
			if(epdir->epd_state > ENDPOINT_STATE_IDLE)
				if(epdir->epd_state & ENDPOINT_STATE_IDLE)
				{
					OTG_HC_REGS* ch_regs = &drv_info->hw_base->HC_REGS[2*i + j];

					// this channel requires a restart?
					if(epdir->epd_pending && (epdir->epd_pending->cmd & (FLAG_READ | FLAG_WRITE))) // not cancelled
					{
						// Restart IN & OUT
						uint32_t reg;

						epdir->epd_state ^= ENDPOINT_STATE_IDLE;

						if(gd_is_used)
						{
							stm_host_start_xfer(drv_info, epdir->epd_pending, i * 2+j, epdir);
						}else
						{
							reg = ch_regs->HCCHAR;
#if USB_HOST_KEEPS_CHANNELS_DOWN
							reg |= OTG_HCCHAR_CHENA;
#endif
							ch_regs->HCCHAR = reg; // if CHDIS ->halt else re-enable
						}
						if(!j)
						{
							// Write some data OUT
							stm_host_write_payload(drv_info, i);
						}
					}
					else
					{
#if USB_HOST_KEEPS_CHANNELS_DOWN
						usb_a_ch_halt(drv_info, 2*i + j, 0, EPT_HALT_HALTED);
#else
						stm_host_ch_halt(drv_info, ch_regs);
#endif
					}

				}
			epdir++;
		}
	}
}

#if !USB_HOST_KEEPS_CHANNELS_DOWN

#define usb_a_clr_ch_int(x) \
		ch_ints ^= x; \
		ch_regs->HCINT = x;\
		i_cnt++

static void FAST_FLASH usb_a_ch_int(USB_DRV_INFO drv_info, uint32_t ch_indx)
{
	uint32_t ch_ints, is_in;
	OTG_HC_REGS* ch_regs = &drv_info->hw_base->HC_REGS[ch_indx];
    ep_dir_state_t* epdir;
	HANDLE hnd;
	unsigned i_cnt =0;

	//read channel interrupts
	ch_ints = ch_regs->HCINT;
	ch_ints &= ch_regs->HCINTMSK;
	TRACE_USB(" %x=", ch_indx);
    TRACE_USB("%02x", ch_ints);
	// get the state of endpoint
	is_in = ch_regs->HCCHAR &  OTG_HCCHAR_EPDIR;
	if(is_in)
		epdir = &drv_info->drv_data->endpoints[ch_indx >> 1].epd_out;
	else
		epdir = &drv_info->drv_data->endpoints[ch_indx >> 1].epd_in;


    if (ch_ints & OTG_HCINT_AHBERR )	// AHB error
	{
    	usb_a_clr_ch_int(OTG_HCINT_AHBERR);
		TRACE1_USB(" AHBERR!");
	}

	if (ch_ints & OTG_HCINT_ACK )		// ACK response received/transmitted
	{
		usb_a_clr_ch_int(OTG_HCINT_ACK);
		TRACE1_USB(" ACK");
	}

	if (ch_ints &  OTG_HCINT_NYET)		// NYET response
	{
		usb_a_clr_ch_int(OTG_HCINT_NYET);
		TRACE1_USB(" NYET");
		// this must be OUT(in) endpoint
		if(is_in)
		{
			TRACE1_USB(" wrong dir!");
		} else
		{
			epdir->epd_state |= ENDPOINT_STATE_NYET;
			stm_host_ch_halt(drv_info, ch_regs);
		}
	}

	if (ch_ints &  OTG_HCINT_FRMOR)		// Frame overrun
	{
		usb_a_clr_ch_int(OTG_HCINT_FRMOR);
		TRACE1_USB(" FRMOR");
		stm_host_ch_halt(drv_info, ch_regs);
	}

	if(ch_ints & OTG_HCINT_STALL)		// STALL response received
	{
		usb_a_clr_ch_int(OTG_HCINT_STALL);
		TRACELN1(" STALL");

		epdir->epd_state = ENDPOINT_STATE_STALL;
		stm_host_ch_halt(drv_info, ch_regs);
	}

	if(ch_ints & OTG_HCINT_DTERR)		// Data toggle error
	{
		usb_a_clr_ch_int(OTG_HCINT_DTERR);
		TRACE1_USB(" DTERR");

		epdir->epd_state |= ENDPOINT_STATE_ERR;
		stm_host_ch_halt(drv_info, ch_regs);
	}


	if(ch_ints & OTG_HCINT_XFRC)		// Transfer completed
	{
		usb_a_clr_ch_int(OTG_HCINT_XFRC);
		TRACE1_USB(" TC");

		if( (hnd=epdir->epd_pending) )
		{
			if (is_in)
			{
				// IN transfer competed
				epdir->epd_pending = hnd->next;
				if (!hnd->len || (hnd->res & FLG_OK))
					usr_usb_HND_SET_STATUS(hnd, RES_SIG_OK);
				else
					usr_usb_HND_SET_STATUS(hnd, RES_SIG_IDLE);

				epdir->epd_state = ENDPOINT_STATE_IDLE;
			} else
			{
				uint32_t len;
				// OUT transfer, update
				len = hnd->len;
				if (len)
				{
					//not yet, update transfer and continue
					if (len > epdir->epd_fifo_sz)
						len = epdir->epd_fifo_sz;
					hnd->src.as_byteptr += len;
					hnd->len -= len;
					len = hnd->len;
				}

				//toggle data
				if (epdir->epd_type == ENDPOINT_TYPE_BULK)
					epdir->epd_flags ^= EPD_FLAG_DATA1;

				// End of transfer ?
				if (!len)
				{
					TRACE1_USB(" Wr!");
					epdir->epd_pending = hnd->next;
					usr_usb_HND_SET_STATUS(hnd, RES_SIG_OK);
					hnd = epdir->epd_pending;
				}
				//check if we have more to write
				if (hnd)
				{
					// start it...
					stm_host_start_tx(drv_info, hnd, ch_indx >> 1, epdir);
				} else
				{
					epdir->epd_state = ENDPOINT_STATE_IDLE;
				}
			}
		}
	}

	if(ch_ints & OTG_HCINT_BBERR)		// Babble error
	{
		usb_a_clr_ch_int(OTG_HCINT_BBERR);
		TRACE1_USB(" BBERR");
		stm_host_ch_halt(drv_info, ch_regs);

	}

	if(ch_ints & OTG_HCINT_TRERR)		// Transaction Error
	{
		usb_a_clr_ch_int(OTG_HCINT_TRERR);
		TRACE1_USB(" TRERR");
		// ENDPOINT_STATE_ERR is used as a flag to re-enable the channel when it is halted
		epdir->epd_state |= ENDPOINT_STATE_ERR | ENDPOINT_STATE_IDLE;
		stm_host_ch_halt(drv_info, ch_regs);
#if USB_HOST_DRIVER_RETRY
		// The channel is assumed to have stopped before usb_hal_host_nak_tout() starts, relying on the delay before it starts
		tsk_send_signal(drv_info->drv_data->helper_task, USB_DRIVER_SIG_TOUT);
#endif
	}

	if(ch_ints & OTG_HCINT_NAK)			// NAK response received
	{
		usb_a_clr_ch_int(OTG_HCINT_NAK);
		TRACE1_USB(" NAK");

		// we need a timeout or halt?
		if(epdir->epd_type == ENDPOINT_TYPE_CONTROL || epdir->epd_type == ENDPOINT_TYPE_BULK)
		{
			if(epdir->epd_state > ENDPOINT_STATE_IDLE)
			{
#if USB_HOST_DRIVER_RETRY
				epdir->epd_state |= ENDPOINT_STATE_IDLE;
				if(!is_in)
				{
					TRACE1_USB(" OD ");
					// ENDPOINT_STATE_ERR is used as a flag to re-enable the channel when it is halted
					epdir->epd_state |= ENDPOINT_STATE_ERR;
					stm_host_ch_halt(drv_info, ch_regs);
					// The channel is assumed to have stopped before usb_hal_host_nak_tout() starts, relying on the delay before it starts
				}
				tsk_send_signal(drv_info->drv_data->helper_task, USB_DRIVER_SIG_TOUT);
#else
				epdir->epd_state |= ENDPOINT_STATE_ERR | ENDPOINT_STATE_IDLE;
				stm_host_ch_halt(drv_info, ch_regs);
#endif

			}
		} else
		{
			// ENDPOINT_TYPE_ISOCHRONOUS or ENDPOINT_TYPE_INTERRUPT
			stm_host_ch_halt(drv_info, ch_regs);
		}
	}

	if(ch_ints & OTG_HCINT_CHH)		// Channel halted
	{
		usb_a_clr_ch_int(OTG_HCINT_CHH);
		TRACE1_USB(" CH");
		usb_host_stop_halt_tout(drv_info, ch_indx);
		if(epdir->epd_state & ENDPOINT_STATE_ERR)
		{
			epdir->epd_state ^= ENDPOINT_STATE_ERR;
#if USB_HOST_DRIVER_RETRY
			if(!gd_is_used)
			{
				  /* re-activate the channel  */
				ch_regs->HCCHAR = (ch_regs->HCCHAR & ~OTG_HCCHAR_CHDIS) | OTG_HCCHAR_CHENA;
			}
#else
			if(epdir->epd_state > ENDPOINT_STATE_IDLE && (epdir->epd_state & ENDPOINT_STATE_IDLE))
			{
				// this channel requires a restart?
				if(epdir->epd_pending && (epdir->epd_pending->cmd & (FLAG_READ | FLAG_WRITE))) // not cancelled
				{
					// Restart IN & OUT
					epdir->epd_state ^= ENDPOINT_STATE_IDLE;
					stm_host_start_xfer(drv_info, epdir->epd_pending, ch_indx, epdir);
					if(!is_in)
					{// Write some data OUT
						stm_host_write_payload(drv_info, ch_indx >>1);
					}
				}
				else
				{
					// cancelled signal it
					goto Siganal_Endpoint_hanndlers;
				}
			}
#endif
		} else
		{
Siganal_Endpoint_hanndlers:
			while( (hnd =  epdir->epd_pending) )
			{
				TRACE1_USB(" EoT");
				epdir->epd_pending = hnd->next;
				if(epdir->epd_state == ENDPOINT_STATE_STALL)
				{
					usr_usb_HND_SET_STATUS(hnd, FLG_SIGNALED | FLG_DATA);
				} else
				{
					usr_usb_HND_SET_STATUS(hnd, FLG_SIGNALED | (hnd->res & FLG_OK));
				}
			}
			epdir->epd_state = ENDPOINT_STATE_IDLE;
		}
	}
    //clear channel interrupts
	if(ch_ints)
	{
		TRACELN_USB("\e[1;31m clear channel %02X\e[m", ch_ints);
		ch_regs->HCINT = ch_ints;
	}
	if(i_cnt > 1)
	{
		TRACELN_USB("\e[1;31m more than one interrupt %u\e[m", i_cnt);
	}
}

#else  //USB_HOST_KEEPS_CHANNELS_DOWN

/**
 * handles channel interrupts by taking only one of the flags.
 * The priority is determined by the order of the code,
 * the one at the beginning has the highest priority
 *
 * @param drv_info
 * @param ch_indx
 */

static void usb_a_halt_IN(USB_DRV_INFO drv_info, uint32_t ch_indx)
{
	HANDLE hnd;
	OTG_HC_REGS* ch_regs = &drv_info->hw_base->HC_REGS[ch_indx];
	Endpoint* endpoint = &drv_info->drv_data->endpoints[ch_indx >> 1];
    ep_dir_state_t* epdir = &endpoint->epd_out;;
	uint32_t ch_intr = ch_regs->HCINT & ch_regs->HCINTMSK;

	TRACE_USB(" ch%x=%02x", ch_indx, ch_intr);

	if (ch_intr & OTG_HCINT_STALL)
	{
		TRACE1_USB(" STALL");
		usb_a_ch_halt(drv_info, ch_indx, OTG_HCINT_STALL, EPT_HALT_STALL);
		ch_regs->HCINT = OTG_HCINT_NAK;

		/* note: When there is a 'STALL', reset also NAK,
		 else, the epd_halt_reason = HC_STALL
		 will be overwritten by 'NAK' in code below */
		ch_intr &= ~OTG_HCINT_NAK;
	}
	else if (ch_intr & OTG_HCINT_DTERR)
	{
		TRACE1_USB(" DTERR");
		usb_a_ch_halt(drv_info, ch_indx, OTG_HCINT_DTERR, EPT_HALT_DTGERR);
		ch_regs->HCINT = OTG_HCINT_NAK;
		ch_intr &= ~OTG_HCINT_NAK;
	}
	else if (ch_intr & OTG_HCINT_ACK)
	{
		TRACE1_USB(" ACK");
		ch_regs->HCINT = OTG_HCINT_ACK;
		ch_regs->HCINTMSK &= ~OTG_HCINTMSK_ACKM;
		reset_error_cnt();
	}

// ACK, SATLL and DTERR are processed simultaneously with the others
	if (ch_intr & OTG_HCINT_XFRC)
	{
		//Transfer completed
		TRACE1_USB(" XF");
		ch_regs->HCINT = (OTG_HCINT_XFRC | OTG_HCINT_ACK) ;
		ch_regs->HCINTMSK &= ~(OTG_HCINTMSK_ACKM);

		epdir->epd_halt_reason = EPT_HALT_XF;
		reset_error_cnt();


		switch ((EndpointAttribute_TransferType)epdir->epd_type)
		{
		case ENDPOINT_TYPE_CONTROL:
		case ENDPOINT_TYPE_BULK:
			usb_a_ch_halt(drv_info, ch_indx, OTG_HCINT_NAK, EPT_HALT_XF);
			break;

		case ENDPOINT_TYPE_INTERRUPT:
		case ENDPOINT_TYPE_ISOCHRONOUS:
			ch_regs->HCCHAR |= OTG_HCCHAR_ODDFRM;
			epdir->epd_halt_reason = EPT_HALT_IDLE;
			epdir->epd_state = ENDPOINT_STATE_IDLE;
			if( ( hnd = epdir->epd_pending) )
			{
				HND_ERROR_SET_MB(hnd, EPT_HALT_XF);
				usr_usb_HND_SET_STATUS(hnd, FLG_SIGNALED | (hnd->res & FLG_OK));
				epdir->epd_pending = hnd->next;
			}
			break;
		}
	}
	else if (ch_intr & OTG_HCINT_FRMOR)
	{
		TRACE1_USB(" FRMOR");
		usb_a_ch_halt(drv_info, ch_indx, OTG_HCINT_FRMOR, EPT_HALT_FRMOR);
	}
	else if (ch_intr & OTG_HCINT_TRERR)
	{
		TRACE1_USB(" TRERR");
		inc_error_cnt();
//		ch_regs->HCINT = OTG_HCINT_ACK;
		ch_regs->HCINTMSK |= OTG_HCINTMSK_ACKM;

		usb_a_ch_halt(drv_info, ch_indx, OTG_HCINT_TRERR, EPT_HALT_TRERR);
	}
	else if (ch_intr & OTG_HCINT_NAK)
	{
		TRACE1_USB(" NAK");
		epdir->epd_halt_reason = EPT_HALT_NAK;
		reset_error_cnt();
		switch (epdir->epd_type)
		{
		case ENDPOINT_TYPE_CONTROL:
		case ENDPOINT_TYPE_BULK:
			usb_a_ch_halt(drv_info, ch_indx, OTG_HCINT_NAK, EPT_HALT_NAK);
			break;

		case ENDPOINT_TYPE_INTERRUPT:
			ch_regs->HCINTMSK |= OTG_HCINTMSK_CHHM;
			epdir->epd_halt_reason = EPT_HALT_HALTED;
			stm_host_ch_halt(drv_info, ch_regs);
			break;

		default:
			break;
		}
		ch_regs->HCINT = OTG_HCINT_NAK;
	}
	else if (ch_intr & OTG_HCINT_CHH)
	{
		// disable channel HALT interrupt
		ch_regs->HCINTMSK &= ~OTG_HCINTMSK_CHHM;
		// clear channel HALt interrupt flag
		ch_regs->HCINT = OTG_HCINT_CHH;
		TRACE1_USB(" CH");
		usb_host_stop_halt_tout(drv_info, ch_indx);

		RES_CODE res = FLG_SIGNALED;
		switch (epdir->epd_halt_reason)
		{
		case EPT_HALT_XF:
			if( ( hnd = epdir->epd_pending) )
				res |= hnd->res & FLG_OK;
			break;

		case EPT_HALT_NAK:
#if USB_HOST_DRIVER_RETRY
			if(epdir->epd_state > ENDPOINT_STATE_IDLE)
			{
				epdir->epd_state |= ENDPOINT_STATE_IDLE;
				tsk_send_signal(drv_info->drv_data->helper_task, USB_DRIVER_SIG_TOUT);
				return;
			}
#else
			if( (hnd = epdir->epd_pending))
			{
				if(hnd->cmd & FLAG_READ)
				{
					epdir->epd_halt_reason = EPT_HALT_NAK;
					uint32_t reg;
					reg = ch_regs->HCCHAR;
					reg |= OTG_HCCHAR_CHENA;
					ch_regs->HCCHAR = reg;
					return;
				}
				epdir->epd_halt_reason = EPT_HALT_HALTED;
			}
#endif
			break;

		case EPT_HALT_STALL:
			res |= FLG_DATA;
			break;

		case EPT_HALT_TRERR:
		case EPT_HALT_DTGERR:
			res |= FLG_ERROR;
			break;

		case EPT_HALT_HALTED:
			TRACE1_USB(" CAN");
			[[fallthrough]];
		default:
			break;
		}

		if( ( hnd = epdir->epd_pending) )
		{
			TRACE_USB(" res:%02X", res);
			if(epdir->epd_state == ENDPOINT_STATE_RECEIVING_OFF)
				hnd->mode1 = 1;
			epdir->epd_pending = hnd->next;
			HND_ERROR_SET_MB(hnd, epdir->epd_halt_reason);
			usr_usb_HND_SET_STATUS(hnd, res);
			while( (hnd = epdir->epd_pending) )
			{
				if(epdir->epd_state == ENDPOINT_STATE_RECEIVING_OFF)
				{
					host_buffer_to_hnd(endpoint, hnd);
					usr_usb_HND_SET_STATUS(hnd, RES_SIG_OK);
				}
				else
				{
					if(epdir->epd_halt_reason == EPT_HALT_XF)
					{
						endpoint->epd_out.epd_state = ENDPOINT_STATE_RECEIVING;
						epdir->epd_halt_reason = EPT_HALT_IDLE;
						reset_error_cnt();
						stm_host_start_xfer(drv_info, hnd, ch_indx , epdir);
						break;
					}else
					{
						usr_usb_HND_SET_STATUS(hnd, res);
					}
				}
				epdir->epd_pending = hnd->next;
			}
		}
		if(epdir->epd_state != ENDPOINT_STATE_RECEIVING_OFF)
			epdir->epd_state = ENDPOINT_STATE_IDLE;
		epdir->epd_halt_reason = EPT_HALT_IDLE;
		reset_error_cnt();
	}

}

static void FAST_FLASH usb_a_signal_out_hnd(USB_DRV_INFO drv_info, uint32_t eptnum, ep_dir_state_t* epdir)
{
	HANDLE hnd;
	RES_CODE res = FLG_SIGNALED;

	if( (hnd = epdir->epd_pending) )
	{
		HND_ERROR_SET_MB(hnd, epdir->epd_halt_reason);
		switch(epdir->epd_halt_reason)
		{
		case EPT_HALT_XF:
			res |= FLG_OK;
			break;
		case EPT_HALT_STALL:
			res |= FLG_DATA;
			break;
		case EPT_HALT_BBERR:
		case EPT_HALT_DTGERR:
		case EPT_HALT_FRMOR:
		case EPT_HALT_TRERR:
			res |= FLG_ERROR;
			break;

		case EPT_HALT_BUSY:
		case EPT_HALT_HALTED:
		case EPT_HALT_NAK:
		case EPT_HALT_NYET:
		case EPT_HALT_IDLE:
			break;
		}

		epdir->epd_pending = hnd->next;
		usr_usb_HND_SET_STATUS(hnd, res);
		hnd = epdir->epd_pending;
		//check if we have more to write
		if (hnd)
		{
			// start it...
			epdir->epd_halt_reason = EPT_HALT_BUSY;
			epdir->epd_state = ENDPOINT_STATE_SENDING;
			stm_host_start_tx(drv_info, hnd, eptnum >> 1, epdir);
			return;
		}
	}
	epdir->epd_halt_reason = EPT_HALT_IDLE;
	epdir->epd_state = ENDPOINT_STATE_IDLE;
}

static void FAST_FLASH usb_a_halt_OUT(USB_DRV_INFO drv_info, uint32_t ch_indx)
{
	OTG_HC_REGS* ch_regs = &drv_info->hw_base->HC_REGS[ch_indx];
    ep_dir_state_t* epdir = &drv_info->drv_data->endpoints[ch_indx >> 1].epd_in;;
	HANDLE hnd = epdir->epd_pending;

	uint32_t ch_ints = ch_regs->HCINT & ch_regs->HCINTMSK;
	TRACE_USB(" ch%x=%02x", ch_indx, ch_ints);

	if(ch_ints & (OTG_HCINT_STALL | OTG_HCINT_DTERR | OTG_HCINT_FRMOR | OTG_HCINT_NAK
			| OTG_HCINT_TRERR | OTG_HCINT_NYET))
	{
		if (ch_ints & OTG_HCINT_STALL)
		{
			TRACE1_USB(" STALL");
			usb_a_ch_halt(drv_info, ch_indx, OTG_HCINT_STALL, EPT_HALT_STALL);

		}
		else if (ch_ints & OTG_HCINT_DTERR) //*
		{
			TRACE1_USB(" DTERR");
			usb_a_ch_halt(drv_info, ch_indx, OTG_HCINT_DTERR, EPT_HALT_DTGERR);
			ch_regs->HCINT = OTG_HCINT_NAK;

		}
		else if (ch_ints & OTG_HCINT_FRMOR) //*
		{
			TRACE1_USB(" FRMOR");
			usb_a_ch_halt(drv_info, ch_indx, OTG_HCINT_FRMOR, EPT_HALT_FRMOR);

		}
		else if (ch_ints & OTG_HCINT_NAK)
		{
			TRACE1_USB(" NAK");
			reset_error_cnt();
			usb_a_ch_halt(drv_info, ch_indx, OTG_HCINT_NAK, EPT_HALT_NAK);
		}
		else if (ch_ints & OTG_HCINT_TRERR)
		{
			TRACE1_USB(" TRERR");
			inc_error_cnt();
//			ch_regs->HCINTMSK |= OTG_HCINTMSK_ACKM;
			usb_a_ch_halt(drv_info, ch_indx, OTG_HCINT_TRERR, EPT_HALT_TRERR);
		}
		else if (ch_ints & OTG_HCINT_NYET) // USB HS
		{
			TRACE1_USB(" NYET");
			reset_error_cnt();
			if(epdir->epd_type == ENDPOINT_TYPE_CONTROL)
			{
				usb_a_ch_halt(drv_info, ch_indx, OTG_HCINT_NYET, EPT_HALT_XF);
			}
			else
			{
				usb_a_ch_halt(drv_info, ch_indx, OTG_HCINT_NYET, EPT_HALT_NYET);
			}
		}
	}
	else if (ch_ints & OTG_HCINT_CHH)
	{
		TRACE1_USB(" CH");
		// disable Channel halted interrupt
		ch_regs->HCINTMSK &= ~OTG_HCINTMSK_CHHM;
		// clear Channel halted interrupt
		ch_regs->HCINT = OTG_HCINT_CHH;
		usb_host_stop_halt_tout(drv_info, ch_indx);

		switch (epdir->epd_halt_reason)
		{
		case EPT_HALT_XF:
			usb_a_signal_out_hnd(drv_info, ch_indx, epdir);
			break;

		case EPT_HALT_NAK:
			TRACE1_USB("\e[1;96m retry\e[m");
#if USB_HOST_DRIVER_RETRY
			if(epdir->epd_state > ENDPOINT_STATE_IDLE)
			{
				epdir->epd_state |= ENDPOINT_STATE_IDLE;
				tsk_send_signal(drv_info->drv_data->helper_task, USB_DRIVER_SIG_TOUT);
				break;
			}
#else
			if( (hnd = epdir->epd_pending))
			{
				if(hnd->cmd & FLAG_WRITE)
				{
					stm_host_start_tx(drv_info, hnd, ch_indx >> 1, epdir);
					break;
				}
				epdir->epd_halt_reason = EPT_HALT_HALTED;
			}
#endif
			reset_error_cnt();;
			[[fallthrough]];

		case EPT_HALT_NYET:
			TRACE1_USB(" NAK/NYET res:00");
			usb_a_signal_out_hnd(drv_info, ch_indx, epdir);
			break;

		case EPT_HALT_STALL:
			TRACE1_USB(" res:08");
			usb_a_signal_out_hnd(drv_info, ch_indx, epdir);
			break;

		case EPT_HALT_TRERR:
#if USB_HOST_USE_TRERR_CNT
			if(epdir->epd_error_cnt < MAX_TRERR_CNT )
#endif
			{
#if USB_HOST_DRIVER_RETRY
				if(epdir->epd_state > ENDPOINT_STATE_IDLE)
				{
					epdir->epd_state |= ENDPOINT_STATE_IDLE;
					tsk_send_signal(drv_info->drv_data->helper_task, USB_DRIVER_SIG_TOUT);
					break;
				}
#else
				if( (hnd = epdir->epd_pending) && (hnd->cmd & FLAG_WRITE))
				{
					stm_host_start_tx(drv_info, hnd, ch_indx >> 1, epdir);
					break;
				}
#endif
			}
			reset_error_cnt();

			TRACE1_USB(" res:10");
			usb_a_signal_out_hnd(drv_info, ch_indx, epdir);
			break;

		case EPT_HALT_HALTED:
			TRACE1_USB(" CAN");
			[[fallthrough]];
		default:
			TRACE1_USB(" res:?");
			usb_a_signal_out_hnd(drv_info, ch_indx, epdir);
			break;
		}
	}
	else
	{
		if (ch_ints & OTG_HCINT_XFRC)
		{
			TRACE1_USB(" XF");
			ch_regs->HCINT = OTG_HCINT_XFRC;
			ch_regs->HCINTMSK |= OTG_HCINTMSK_ACKM;
		}

		if (ch_ints & OTG_HCINT_ACK)
		{

			// Transfer completed
			reset_error_cnt();
			TRACE1_USB(" ACK");
			ch_regs->HCINT = (OTG_HCINT_XFRC | OTG_HCINT_ACK);
	//		ch_regs->HCINTMSK &= ~(OTG_HCINTMSK_ACKM);
			reset_error_cnt();
			if (ENDPOINT_TYPE_BULK == epdir->epd_type){
				//toggle data
				epdir->epd_flags ^= EPD_FLAG_DATA1;
			}

			if( (hnd = epdir->epd_pending) )
			{
				uint32_t len;
				// OUT transfer, update
				len = hnd->len;
				if (len)
				{
					//not yet, update transfer and continue
					if (len > epdir->epd_fifo_sz)
						len = epdir->epd_fifo_sz;
					hnd->src.as_byteptr += len;
					hnd->len -= len;
				}
				// End of transfer ?
				if (!hnd->len)
				{
					TRACE1_USB(" Wr!");
					usb_a_ch_halt(drv_info, ch_indx, 0, EPT_HALT_XF);
				}else
				{
					// start it...
					epdir->epd_halt_reason = EPT_HALT_BUSY;
					epdir->epd_state = ENDPOINT_STATE_SENDING;
					stm_host_start_tx(drv_info, hnd, ch_indx >> 1, epdir);
				}

			}else
			{
				usb_a_ch_halt(drv_info, ch_indx, 0, EPT_HALT_XF);
			}
		}
	}

}

static void FAST_FLASH usb_a_halt_ch_int(USB_DRV_INFO drv_info, uint32_t ch_indx)
{
	if(drv_info->hw_base->HC_REGS[ch_indx].HCCHAR &  OTG_HCCHAR_EPDIR)
		usb_a_halt_IN(drv_info, ch_indx);
	else
		usb_a_halt_OUT(drv_info, ch_indx);
}

#endif //USB_HOST_KEEPS_CHANNELS_DOWN

/**
 * Host port interrupt
 * @param drv_info
 */
static void FAST_FLASH usb_a_hprt_int(USB_DRV_INFO drv_info)
{
	uint32_t port_status, port_config;
	USB_TypeDef* otg = drv_info->hw_base;

	// Read port status
	port_status = otg->HPRT;
	// Clear port interrupts
//	otg->HPRT = port_status & ~OTG_HPRT_PENA;

	if(port_status & OTG_HPRT_PCDET)	// Port Connect detected
	{
		TRACE1_USB(" PCDET");
		usb_otg_set_flags(drv_info, USB_OTG_FLG_HOST_RST);
	}

	if(port_status & OTG_HPRT_POCCHNG)	// Port overcurrent change
	{
		TRACE1_USB(" CURRENT!");
		if(port_status & OTG_HPRT_POCA)
			usb_otg_clr_flags(drv_info, USB_OTG_FLG_ALL);
	}


	if(port_status & OTG_HPRT_PENCHNG)	// Port enable/disable change
	{
		TRACE1_USB(" PENCHNG");
		if(port_status & OTG_HPRT_PENA)
		{
			TRACE1_USB(" PENA");
			//port enabled
			port_config = otg->host_regs.HCFG;
			if( (port_status & OTG_HPRT_PSPD_Msk) == OTG_HPRT_PSPD_LS)
			{
				// Set the Host Frame Interval Register for the 6KHz speed
				otg->host_regs.HFIR = 6000;

				// Are we switching from FS to LS?
				if((port_config & OTG_HCFG_FSLSPCS_Msk) != OTG_HCFG_FSLSPCS_6)
				{
					// configure for LS
					port_config &= ~OTG_HCFG_FSLSPCS_Msk;
					otg->host_regs.HCFG = port_config | OTG_HCFG_FSLSPCS_6;
					usb_otg_set_flags(drv_info, USB_OTG_FLG_HOST_RST);
				}

			} else
			{
				otg->host_regs.HFIR = 48000;

				// Are we switching from LS to FS?

				if((port_config & OTG_HCFG_FSLSPCS_Msk) != OTG_HCFG_FSLSPCS_48)
				{
					// configure for FS
					port_config &= ~OTG_HCFG_FSLSPCS_Msk;
					otg->host_regs.HCFG = port_config | OTG_HCFG_FSLSPCS_48;
					usb_otg_set_flags(drv_info, USB_OTG_FLG_HOST_RST);
				}

			}
		} else
		{
			if(drv_info->drv_data->otg_flags & USB_OTG_FLG_HOST_OK)
				usb_otg_clr_flags(drv_info, USB_OTG_FLG_HOST);
		}
	}
	otg->HPRT = port_status & ~OTG_HPRT_PENA;

}

static void FAST_FLASH usb_a_gint_rxflvl(USB_DRV_INFO drv_info)
{
	uint32_t status;
	USB_TypeDef* otg = drv_info->hw_base;

	/* disable the RX status queue level interrupt */
	otg->core_regs.GINTMSK &= ~OTG_GINTMSK_RXFLVLM;

	/* Get the Status from the top of the FIFO */
	status = otg->core_regs.GRXSTSP;
	TRACE_USB(" Q%u:", (status & OTG_GRXSTSP_EPNUM_Msk));
	switch(status & OTG_GRXSTSP_PKTSTS_Msk)
	{
	case OTG_GRXSTSP_PKTSTS_IN_DATA:	//  IN data packet received
#if USB_HOST_KEEPS_CHANNELS_DOWN
		host_halt_read_payload(drv_info, status);
#else
		if(!stm_read_payload(drv_info, status))
		{
			/* Disable the Rx Status Queue Level interrupt */
			otg->core_regs.GINTMSK &= ~OTG_GINTMSK_RXFLVLM;
			return;
		}
#endif
		break;

	case OTG_GRXSTSP_PKTSTS_IN_COMP:	//  IN transfer completed (triggers an interrupt)
		TRACE_USB("in comp");
		break;

	case OTG_GRXSTSP_PKTSTS_TOGGLE_ERR: //  Data toggle error (triggers an interrupt)
		TRACE1_USB("tog err");
		if(gd_is_used)
		{
			uint32_t count = OTG_GRXSTSP_BCNT_Get(status);
	        while (count > 0U) {
	        	status = otg->core_regs.GRXSTSP;
	            count--;
	        }
		}
		break;

	case OTG_GRXSTSP_PKTSTS_EP_HALT:	//  Channel halted (triggers an interrupt)
		TRACE1_USB("halt");
		break;

	default:
		TRACELN_USB("que: unk st=%x", status);
		break;

	}
	/* enable the RX status queue level interrupt */
	otg->core_regs.GINTMSK |= OTG_GINTMSK_RXFLVLM;
}

#endif // USB_ENABLE_HOST

void USB_OTG_ISR(USB_DRV_INFO drv_info)
{
	uint32_t status;
	USB_TypeDef* otg = drv_info->hw_base;
//#if USB_ENABLE_HOST
	USB_DRIVER_DATA* drv_data = drv_info->drv_data;
//#endif
#if USB_ENABLE_DEVICE
	uint32_t host_mode;
#endif

	status = otg->core_regs.GINTSTS;
//	TRACE_USB_NAME(drv_info);
//	TRACELN_USB("[%08X]", status);
#if USB_ENABLE_DEVICE
	host_mode = status & OTG_GINTSTS_CMOD;
#endif
	status &= otg->core_regs.GINTMSK;
	TRACELN_USB("[%08X]", status);
#if USB_ENABLE_HOST && USE_CPU_SLEEP_MODE
	while(drv_data->otg_sleep_flag)
	{
		locked_inc_int(&cpu_sleep_counter);			// disable sleep
		locked_dec_int(&drv_data->otg_sleep_flag);
	}
#endif //USB_ENABLE_HOST && USE_CPU_SLEEP_MODE

	if(status & OTG_GINTSTS_CIDSCHG)	// Connector ID status change
    {
		otg->core_regs.GINTSTS = OTG_GINTSTS_CIDSCHG;
		status ^= OTG_GINTSTS_CIDSCHG;
		TRACE1_USB(" CIDSCHG");

		if(otg->core_regs.GOTGCTL & OTG_GOTGCTL_CIDSTS)
		{
			//devive mode now
			TRACE1_USB(" ->dev");
#if USB_ENABLE_HOST
			usb_otg_clr_flags(drv_info, USB_OTG_FLG_HOST);
#endif //USB_ENABLE_HOST

		} else
		{
			//host mode now
			TRACE1_USB(" ->host");
#if USB_ENABLE_HOST
			usb_otg_clr_flags(drv_info, USB_OTG_FLG_DEV);
			if(drv_data->otg_flags & USB_OTG_FLG_HOST)
			{
				if(otg->HPRT & OTG_HPRT_PENA)
					usb_otg_set_flags(drv_info, USB_OTG_FLG_HOST_RST);
			}
#endif //USB_ENABLE_HOST
		}
    }

    /* Handle Host SOF Interrupts */
    if(status & OTG_GINTSTS_SOF)
    {
		TRACE1_USB(" SOF");
#if USB_ENABLE_HOST
		if(usb_host_process_halt_tout(drv_info))
			otg->core_regs.GINTMSK &= ~OTG_GINTMSK_SOFM;
#endif
		otg->core_regs.GINTSTS = OTG_GINTSTS_SOF;
		status ^= OTG_GINTSTS_SOF;
    }


#if USB_ENABLE_DEVICE
	/* ensure that we are in device mode */
	if( !host_mode )
	{
		if(status) /* avoid spurious interrupt */
		{
			if (status & OTG_GINTSTS_OEPINT) 	// OUT endpoints interrupt
			{
				usb_b_ept_rx_handler(drv_info);
			}

			if (status & OTG_GINTSTS_IEPINT)	// IN endpoints interrupt
			{
				usb_b_ept_tx_handler(drv_info);
			}

			if (status & OTG_GINTSTS_MMIS) 		// Mode Mismatch interrupt
			{
				TRACE1_USB(" mmis!");

				otg->core_regs.GINTSTS = OTG_GINTSTS_MMIS;
			}

			if (status & OTG_GINTSTS_WKUPINT)	// Resume/Remote wakeup detected interrupt
			{
				TRACE1_USB(" wkupint");
				TRACELN_USB("DCFG = %08x", otg->device_regs.DCFG);
				TRACELN_USB("DSTS = %08x", otg->device_regs.DSTS);
				usb_b_gint_wkupint(drv_info);
			}

			if (status & OTG_GINTSTS_USBSUSP)	// USB suspend
			{
				TRACE1_USB(" suspint");
				usb_b_gint_usbsusp(drv_info);
				for(int i=0; i<USB_NUMENDPOINTS; i++)
				{
					usb_drv_end_transfers(&drv_data->endpoints[i].epd_in, USBD_STATUS_RESET);
					usb_drv_end_transfers(&drv_data->endpoints[i].epd_out, USBD_STATUS_RESET);
				}
				if(drv_data->pending)
				{
					HANDLE hnd;
					while( (hnd=drv_data->pending) )
					{
						drv_data->pending = hnd->next;
						usr_usb_HND_SET_STATUS(hnd, FLG_SIGNALED | RES_ERROR);
					}
					atomic_clrex();
				}
			}

			if(status & OTG_GINTSTS_RESETDET)
			{
				TRACELN1_USB("-----ResetDet----------");
				TRACELN_USB("DCFG = %08x", otg->device_regs.DCFG);
				TRACELN_USB("DCTL = %08x", otg->device_regs.DCTL);
				TRACELN_USB("DSTS = %08x", otg->device_regs.DSTS);
				TRACELN_USB("PCGCCTL = %08x", otg->PCGCCTL);
				otg->core_regs.GINTSTS = OTG_GINTSTS_RESETDET;
				if(otg->PCGCCTL & OTG_PCGCCTL_STPPCLK)
				{
					/* un-gate USB Core clock */
					otg->PCGCCTL &= ~(OTG_PCGCCTL_GATEHCLK | OTG_PCGCCTL_STPPCLK);
				}
			}

			if(status & OTG_GINTSTS_FETSUSP)
			{
				TRACELN1_USB("-----FetSusp----------");
				TRACELN_USB("DCFG = %08x", otg->device_regs.DCFG);
				TRACELN_USB("DCTL = %08x", otg->device_regs.DCTL);
				TRACELN_USB("DSTS = %08x", otg->device_regs.DSTS);
				TRACELN_USB("PCGCCTL = %08x", otg->PCGCCTL);
				otg->core_regs.GINTSTS = OTG_GINTSTS_FETSUSP;
			}

			if (status & OTG_GINTSTS_RXFLVL)	// RxFIFO non-empty
			{
				usb_b_gint_rxflvl(drv_info);
			}

			if (status & OTG_GINTSTS_USBRST)	// USB reset
			{
				usb_b_gint_usbrst(drv_info);
			}

			if (status & OTG_GINTSTS_ENUMDNE)	// Enumeration done
			{
				TRACE1_USB(" enumint");
				usb_b_gint_enumdne(drv_info);
			}

			if (status & OTG_GINTSTS_IISOIXFR)	// Incomplete isochronous IN transfer
			{
				TRACE1_USB(" isoiint");

				otg->core_regs.GINTSTS = OTG_GINTSTS_IISOIXFR;
			}

			if (status & OTG_GINTSTS_IISOOXFR)	// Incomplete isochronous OUT transfer
			{
				TRACE1_USB(" isooint");

				otg->core_regs.GINTSTS = OTG_GINTSTS_IISOOXFR;
			}

			if (status & OTG_GINTSTS_SRQINT)
			{
				TRACE1_USB(" sesint");
				usb_b_gint_srqint(drv_info);
			}

			if (status & OTG_GINTSTS_OTGINT)
			{
				TRACE1_USB(" otgint");
				usb_b_gint_otgint(drv_info);
			}
		}
	}
#endif
#if USB_ENABLE_HOST
#if USB_ENABLE_DEVICE
	else
#endif
	{
//		TRACE_USB(" %04X", drv_info->hw_base->host_regs.HFNUM & 0xFFFF);
		if(status)
		{
			if (status & OTG_GINTSTS_WKUPINT)	// remote wakeup is detected on the USB
			{
				HANDLE hnd;
	    		TRACE1_USB(" WKUPINT");
	    		if(drv_data->otg_flags & USB_OTG_FLG_SUSPEND)
	    		{
	    			drv_data->otg_flags |= USB_OTG_FLG_WKUP;
	    			if(drv_info->drv_data->pending)
	    			{
	    				while( (hnd=drv_data->pending) )
	    				{
	    					drv_data->pending = hnd->next;
	    					usr_HND_SET_STATUS(hnd, FLG_SIGNALED);
	    				}
	    				atomic_clrex();
	    			}
	    		}
	    		otg->core_regs.GINTSTS = OTG_GINTSTS_WKUPINT;
	    		status ^= OTG_GINTSTS_WKUPINT;
			}

			if (status & OTG_GINTSTS_SRQINT)	// session request is detected from the device
			{
	    		TRACE1_USB(" SRQINT?");
	    		otg->core_regs.GINTSTS = OTG_GINTSTS_SRQINT;
	    		status ^= OTG_GINTSTS_SRQINT;
			}

	    	if(status & OTG_GINTSTS_DISCINT)	// disconnect
		    {
    			TRACE1_USB(" DISCON");
	    		otg->core_regs.GINTSTS = OTG_GINTSTS_DISCINT;
	    		status ^= OTG_GINTSTS_DISCINT;

    			if(status & OTG_GINTSTS_HPRTINT)
	    		{
	    			TRACE1_USB("(skip)");
	    		}
	    		else
	    		{
	    			drv_data->drv_state_cnt  |= USB_STATE_CNT_INVALID;
					usb_otg_clr_flags(drv_info, USB_OTG_FLG_HOST_OK);
		    	}
		    }

	        /* Handle Rx Queue Level Interrupts */
		    if(status & OTG_GINTSTS_RXFLVL)
		    {
				TRACE1_USB(" R");
				status ^= OTG_GINTSTS_RXFLVL;

				usb_a_gint_rxflvl(drv_info);
		    }

		    if(status & OTG_GINTSTS_HCINT)		// host channels
		    {
		    	uint32_t ch_status;
		    	uint32_t ch_indx = 0;


		    	TRACE1_USB(" C");
		    	// Read in the channel interrupt bits
		    	ch_status = otg->host_regs.HAINT;
		    	ch_status &= (uint16_t)otg->host_regs.HAINTMSK;

		    	while (ch_status)
		    	{
		    		if (ch_status & 0x1)
		    		{
#if USB_HOST_KEEPS_CHANNELS_DOWN
		    			usb_a_halt_ch_int(drv_info, ch_indx);
#else
	    				usb_a_ch_int(drv_info, ch_indx);
#endif
		    		}
		    		ch_indx++;
		    		ch_status >>= 1;
		    	}
		    	//This bit will be automatically cleared after the respective
		    	//channelís flags which cause channel interrupt are cleared.
		    	status ^= OTG_GINTSTS_HCINT;
		    }

		    if(status & OTG_GINTSTS_HPRTINT)	// Handle Host Port Interrupts
		    {
				TRACE1_USB(" HPRT");
		    	usb_a_hprt_int(drv_info);
			    otg->core_regs.GINTSTS = OTG_GINTMSK_SOFM;
				otg->core_regs.GINTMSK |= OTG_GINTMSK_SOFM;

				//This bit will be automatically cleared after the flags that causing a port interrupt are
				//cleared.
				status ^= OTG_GINTSTS_HPRTINT;
		    }

		    if (status & OTG_GINTSTS_IPXFR)		//incomplete periodic transactions
		    									// still pending, which are scheduled for the current frame
		    {

				TRACE1_USB(" IPXFR?");
		    	//disable one endoint:
				for(int i=0; i<USB_NUMENDPOINTS; i++)
				{
					ep_dir_state_t* ep_dir = &drv_data->endpoints[i].epd_out;
					if(ep_dir->epd_state == ENDPOINT_STATE_RECEIVING)
					{
						TRACE_USB(" disable %u", i);
						stm_host_ch_halt(drv_info, &otg->HC_REGS[i*2+1]);
						break;
					}
				}
				 otg->core_regs.GINTSTS = OTG_GINTSTS_IPXFR;
				 status ^= OTG_GINTSTS_IPXFR;
		    }

		    if(status & OTG_GINTSTS_NPTXFE)		// Non-periodic TxFIFO empty
		    {
				TRACE1_USB(" NPTXFE");
				// This should never happens
				otg->core_regs.GINTSTS = OTG_GINTSTS_NPTXFE;
				status ^= OTG_GINTSTS_NPTXFE;
		    }

		    if(status & OTG_GINTSTS_OTGINT)		// OTG interrupt
		    {
				TRACE1_USB(" OTGINT?");
				// (OTG_HS_GOTGINT)
				uint32_t otg_status = otg->core_regs.GOTGINT;
				TRACE_USB(" otg host int:%08X", otg_status);
				otg->core_regs.GOTGINT = otg_status;
				//This bit will be automatically cleared after the flags that causing a port interrupt are
				//cleared.
				status ^= OTG_GINTSTS_OTGINT;
		    }

		    if(status & OTG_GINTSTS_MMIS)		// Mode mismatch interrupt
		    {
				TRACE1_USB(" MMIS?");
				otg->core_regs.GINTSTS = OTG_GINTSTS_MMIS;
				status ^= OTG_GINTSTS_MMIS;
		    }

		    //clear status
		    // ignored - OTG_GINTSTS_MMIS, OTG_GINTSTS_IISOIXFR OTG_GINTSTS_IPXFR
		    // 			OTG_GINTSTS_PTXFE
		    	if(status){
		    		TRACE_USB(" Ignored{%08X}", status);
		    	}
 				otg->core_regs.GINTSTS = status;

		}
	}
#endif
}


/**
 * This function handles EXTI15_10_IRQ Handler.
 * @param drv_info
 */
void USB_HS_WKUP_ISR(USB_DRV_INFO drv_info)
{
	uint32_t cfg;

	cfg = drv_info->cfg->stm32_otg;
	if(cfg & CFG_STM32_OTG_LOW_POWER)
	{
		SCB->SCB_SCR &= ~(SCB_SCR_SLEEPDEEP | SCB_SCR_SLEEPONEXIT);

		stm_otg_ungate_clock(drv_info->hw_base, cfg);
	}
	EXTI->EXTI_PR = EXTI_PR_PR20;

	TRACELN1_USB("usb_wkup!");
}

#if TRACE_USB_LEVEL >= TRACE_LEVEL_DEBUG
#pragma GCC reset_options
#endif

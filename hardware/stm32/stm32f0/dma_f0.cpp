/**************************************************************************//**
 * @ingroup	 hardware_stm32f0_dma
 * @{
 * @file     hardware/stm32/stm32f0/dma_f0.cpp
 * @brief    STM32F2 DMA
 * @version  V3.00
 * @date     5. October 2024
 * @author	 Stanislav Bratkov
 * @}
 */
#include <tmos.h>
#include "fam_cpp.h"

/*
 * CMD		src  dst		P  M
 *  R		 x	  M         M  x		- 1memory to memory 	fill memory
 *  R        x    P			P  x		- periph to 1memory		drain periph
 *  W		 M	  x			x  M		- memory to 1memory     nonsense
 *  W        P    x			P  x		- 1memory to periph  	fill peroph
 * RW		 M	  P			P  M		- memory to periph
 * RW        P    M			P  M		- periph to memory
 * RW        M    M         dM sM		- memory to memory
 *
 *
 *	//mode= periph to memory
 *	hnd.src.as_voidptr = P;
 *
 *	tsk_read(dst, len); or tsk_read_write(dst, P, len);
 *
 *	tsk_read(NULL, len); or tsk_read_write(NULL, P, len);
 *
 *
 *
 */

/* ****************************************************************************
 * 		DMA usage
 *
 * When the handle is open one of the following modes must be selected
 *
 * 1) peripheral to memory
 *
 * 		tsk_write() - no sense
 *
 * 		hnd.src.as_voidptr = PA(peripheral adress); //set once
 * 		hnd.tsk_read(ptr,len);						//then read multiple times
 * 		or
 * 		hnd.tsk_read_write(ptr, PA, len);
 *
 * 		Note: ptr can be NULL (to drain peripherals)
 *
 *
 * 2) memory to peripheral
 *
 * 		tsk_read() - no sense
 *
 * 		hnd.dst.as_voidptr = PA;
 * 		hnd.tsk_write(ptr,len);
 * 		or
 *		hnd.tsk_read_write(PA, ptr, len);
 *
 * 		Note: ptr can be <=0xFF (to fill peripherals)
 *
 * 3) memory to memory
 *
 * 		hnd.src.as_voidptr = src;
 * 		hnd.tsk_read(dst,len);
 * 		or
 * 		hnd.dst.as_voidptr = dst;
 * 		hnd.tsk_write(src,len);
 * 		or
 *		hnd.tsk_read_write(dst, src, len);
 *
 * 		Note: src can be <=0xFF (to fill memory)
 *
 * ****************************************************************************/

void stm32_dma_start(DMA_TypeDef* dmac, uint32_t indx, HANDLE hnd)
{
	DMA_Channel_TypeDef* ch;

	ch = &dmac->DMA_Chx[indx];

	ch->DMA_CNDTRx = hnd->len;

	if( (hnd->src.as_int >> 28) == (BASE_PERIPHERAL >> 28))
	{
		//reading from "peripheral" (peripheral to memory)
		ch->DMA_CPARx = hnd->src.as_int;
		if(hnd->dst.as_int)
		{
			ch->DMA_CMARx = hnd->dst.as_int;
			ch->DMA_CCRx |= DMA_CCRx_MINC;
		}
		else
		{
			ch->DMA_CMARx = (uint32_t)&hnd->dst.as_int;
			ch->DMA_CCRx &= ~DMA_CCRx_MINC;
		}
	} else
	{
		//writing to "peripheral" (memory to peripheral or memory to memory)
		ch->DMA_CPARx = hnd->dst.as_int;
		if(hnd->src.as_int <= 0xFF)
		{
			ch->DMA_CMARx = (uint32_t)&hnd->src.as_int;
			ch->DMA_CCRx &= ~DMA_CCRx_MINC;
		} else
		{
			ch->DMA_CMARx = hnd->src.as_int;
			ch->DMA_CCRx |= DMA_CCRx_MINC;
		}
	}

	// Enable DMA
	ch->DMA_CCRx |= DMA_CCRx_EN ;

}

void stm32_dma_stop(DMA_TypeDef* dmac, uint32_t indx)
{
	dmac->DMA_Chx[indx].DMA_CCRx &= ~DMA_CCRx_EN;
}

void stm32_dma_ch_cfg(DMA_TypeDef* dmac, uint32_t indx, DMA_DRIVER_MODE* mode)
{
	DMA_Channel_TypeDef* ch;

	ch = &dmac->DMA_Chx[indx];

	ch->DMA_CCRx = (mode->dma_ch_cr & ~(DMA_CCRx_RESERVED_NOINTS|DMA_CCRx_RESERVED_CxS));
	if((mode->dma_ch_cr & DMA_CCRx_RESERVED_CxS)){
		uint32_t channel_select;
		indx *= 4;
		channel_select = dmac->DMA_CSELR;
		channel_select &= ~(DMA_CSELR_CxS << indx);
		channel_select |= ((mode->dma_ch_cr & DMA_CCRx_RESERVED_CxS)>>16) << indx;
		dmac->DMA_CSELR = channel_select;
	}
}

void stm32_en_ints(DMA_TypeDef* dmac, uint32_t indx, DMA_DRIVER_MODE* mode)
{
	DMA_Channel_TypeDef* ch;
	uint32_t reg;

	ch = &dmac->DMA_Chx[indx];
	reg = mode->dma_ch_cr;
	if( !(reg & DMA_CCRx_RESERVED_NOINTS))
	{
		ch->DMA_CCRx |= (reg & DMA_CCRx_HTIE) | DMA_CCRx_TEIE | DMA_CCRx_TCIE;
	}

}

void stm32_dis_ints(DMA_TypeDef* dmac, uint32_t indx)
{
	dmac->DMA_Chx[indx].DMA_CCRx &= ~(DMA_CCRx_TCIE | DMA_CCRx_TEIE | DMA_CCRx_EN);

}



/**************************************************************************//**
 * @ingroup	 hardware_stm32f2_usart
 * @{
 * @file     hardware/stm32/stm32f2/tim_f2.cpp
 * @brief    STM32F2 USART
 * @version  V3.00
 * @date     13. August 2024
 * @author	 Stanislav Bratkov
 * @}
 */

#include <tmos.h>
#include <fam_cpp.h>
/**
 *
 * @param hw_base			- timer registers
 * @param peripheral_indx	- peripheral index
 * @param fclk 				- timer frequency kHz
 */

void config_timer_clock(TIM_TypeDef* hw_base, const uint16_t peripheral_indx, const uint16_t fclk)
{
	uint32_t frequency;


	// If the APB prescaler is 1, the timer clock frequencies are set to the same frequency as
	// that of the APB domain to which the timers are connected. Otherwise, they are set to twice (x2)
	// the frequency of the APB domain to which the timers are connected.

	// NOTE: some F4 series have a register RCC_DCKCFGR which has TIMPRE bit.
	// If it set and the APB prescaler (PPRE1, PPRE2 in the RCC_CFGR register) is configured to a
	// division factor of 1, 2 or 4, TIMxCLK = HCLK. Otherwise, the timer clock frequencies are set
	// to four times to the frequency of the APB domain to which the timers are connected:TIMxCLK = 4xPCLKx.

	switch(peripheral_indx)
	{
	case ID_PERIPH_TIM1:
	case ID_PERIPH_TIM8:
	case ID_PERIPH_TIM9:
	case ID_PERIPH_TIM10:
	case ID_PERIPH_TIM11:
		// APB2 120(2x60)
		if((RCC_CFGR_PPRE2 & RCC->RCC_CFGR) == RCC_CFGR_PPRE2_DIV1){
			frequency = APB2_clock_frequency /1000;
		}else{
			frequency = APB2_clock_frequency /500;
		}
		hw_base->TIM_PSC = frequency/fclk;
		break;
	default:
		// APB1 60(2x30)
		if((RCC_CFGR_PPRE1 & RCC->RCC_CFGR) == RCC_CFGR_PPRE1_DIV1){
			frequency = APB1_clock_frequency /1000;
		}else{
			frequency = APB1_clock_frequency /500;
		}
		hw_base->TIM_PSC = frequency/fclk;
		break;
	}
}



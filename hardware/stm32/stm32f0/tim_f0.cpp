/**************************************************************************//**
 * @ingroup	 hardware_stm32f0_tim
 * @{
 * @file     hardware/stm32/stm32f0/tim_f0.cpp
 * @brief    STM32F0 TIM
 * @version  V3.00
 * @date     13. August 2024
 * @author	 Stanislav Bratkov
 */

#include <tmos.h>
#include <fam_cpp.h>

void config_timer_clock(TIM_TypeDef* hw_base, const uint16_t peripheral_indx, const uint16_t fclk)
{
	uint32_t frequency;

	// APB 48(2x24)
	if((RCC_CFGR_PPRE & RCC->RCC_CFGR) == RCC_CFGR_PPRE_DIV1){
		frequency = APB2_clock_frequency /1000;
	}else{
		frequency = APB2_clock_frequency /500;
	}
	hw_base->TIM_PSC = frequency/fclk;
}




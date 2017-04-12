/**************************************************************************//**
 * @ingroup	 hardware_stm32_exti
 * @{
 * @file     hardware/stm32/exti_stm32.cpp
 * @brief    STM32 External Interrupt/Event Controller (EXTI) functions
 * @version  V3.00
 * @date     18. February 2013
 * @author	 Miroslav Kostadinov
 */

#include <tmos.h>
#include <fam_cpp.h>

/**
 * Enable Interrupt for a given line(s)
 * @param exti
 * @param lines
 * @param type
 */
void exti_enable_irq(EXTI_TypeDef* exti, uint32_t lines, pio_def type)
{
    /* Clear Rising Falling edge configuration */
	exti->EXTI_RTSR &= ~lines;
	exti->EXTI_FTSR &= ~lines;

    /* Select the trigger for the selected external interrupts */
	if(type & PD_INT_FE) // falling edge
		exti->EXTI_FTSR |= lines;
	if(type & PD_INT_RE) // rising edge
		exti->EXTI_RTSR |= lines;

	exti->EXTI_IMR |= lines;

}

void exti_enable_event(EXTI_TypeDef* exti, uint32_t lines, pio_def type)
{
    /* Clear Rising Falling edge configuration */
	exti->EXTI_RTSR &= ~lines;
	exti->EXTI_FTSR &= ~lines;

    /* Select the trigger for the selected external interrupts */
	if(type & PD_INT_FE) // falling edge
		exti->EXTI_FTSR |= lines;
	if(type & PD_INT_RE) // rising edge
		exti->EXTI_RTSR |= lines;

	exti->EXTI_EMR |= lines;
}

void exti_disable_irq(EXTI_TypeDef* exti, uint32_t lines)
{
	exti->EXTI_IMR &= ~lines;
}

void exti_disable_event(EXTI_TypeDef* exti, uint32_t lines)
{
	exti->EXTI_EMR &= ~lines;
}

/** @} */

/**
 * @ingroup	 drivers
 * @defgroup DRV_SYSTICK SYSTICK Driver
 * Interface for SYSTICK driver.
 * @{
 *
 * This driver is used from the TMOS kernel for task schedule & timeout.
 *
 * @note The Systick driver is special driver. It must be declared
 * (in drivers.cpp) but the installation is done internally (not in DRV_TABLE).
 *
 * The driver cannot be used with handles, so it does not need DSR().
 *
 *
 * @note Systick ISR is written in assembler (services.S)
 *
 * @file     drivers/systick_drv.h
 * @ingroup	 DRV_SYSTICK
 * @brief    SYSTICK driver header
 *
 * @date     29. November 2010
 * @author	 Miroslav Kostadinov
 * @}
*/
#ifndef CLOCK_DRV_H_
#define CLOCK_DRV_H_

#include <tmos.h>


#ifdef __cplusplus
 extern "C" {
#endif

 /** SYSTICK Driver Info structure **/
struct SYSTICK_DRIVER_INFO
{
	DRIVER_INFO_Type info;				//!< standard driver info
	unsigned int OS_QUANTUM_PERIOD;		//!< quantum [ms] used from the kernel
};

/** SYSTICK Driver Info */
typedef const SYSTICK_DRIVER_INFO* SYSTICK_INFO;

void SYSTICK_DCR(SYSTICK_INFO drv_info, unsigned int reason, void* param) SLOW_FLASH;

/**
 * Systick ISR - written in assembler (services.S)
 * @param driver
 */
void isr_ClockDriver(SYSTICK_INFO driver);

#ifdef __cplusplus
}
#endif


#endif /* CLOCK_DRV_H_ */

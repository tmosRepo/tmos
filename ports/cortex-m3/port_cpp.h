/**************************************************************************//**
 * @ingroup	 ports
 * @defgroup ports_cortex Cortex M3 port
 * @{
 * @file     cortex-m3/port_cpp.h
 * @brief    Cortex M3 kernel CPP header
 * @version  V3.00
 * @date     08. November 2010
 * @author	 Miroslav Kostadinov
 *
 * @}
 *
 ******************************************************************************/

#ifndef PORT_CPP_H_
#define PORT_CPP_H_

#if GD32F4XX_COMPATIBLE
#define FAST_FLASH __attribute__((section(".FastFlash")))
#define SLOW_FLASH __attribute__((section(".SlowFlash")))
#else
#define FAST_FLASH
#define SLOW_FLASH
#endif

#if USE_TMOS_STDLIB
#include <tmos_stdlib.h>
#endif

#if USE_TMOS_STRING
#include <tmos_string.h>
#endif

#ifndef USE_TMOS_LLONG
#define USE_TMOS_LLONG 0
#endif
#if USE_TMOS_LLONG
#include <tmos_llong.h>
#endif


#include <tasks.h>
#include <swi_tab.h>
#include <tmos_swi.h>
#include <services.h>
#include <tmos_stdio.h>

#ifndef USE_MCU_DETECT
#define USE_MCU_DETECT	1
#endif

#if USE_MCU_DETECT
#include <mcu_id.h>
#endif

#ifdef __cplusplus
extern "C"
{
#if USE_CPU_SLEEP_MODE
	extern volatile int cpu_sleep_counter;
#endif
	extern volatile __no_init unsigned int system_clock_frequency;
	extern volatile __no_init unsigned int APB1_clock_frequency;
	extern volatile __no_init unsigned int APB2_clock_frequency;

	extern const char restart_on_exception;

	#if USE_EXCEPTION_RECORD
	typedef struct
	{
		unsigned int CFSR;	//!<  Configurable Fault Status Register
		unsigned int MMFAR; //!< Mem Manage Address Register
		unsigned int BFAR;  //!< Bus Fault Address Register
		unsigned int cur_task;
		unsigned int task_name;
		unsigned int reset_cause;
		unsigned int restart_cause;
		unsigned int record_crc;
	} EXCEPTION_RECORD_STRU;
	unsigned int exception_crc(const unsigned int* record);
	extern volatile EXCEPTION_RECORD_STRU exception_record;
	#endif

	/** Cortex M3 Fault Handler (implemented in exceptions.S **/
	void FaultHandler( void );
}
#else
extern volatile __no_init unsigned int system_clock_frequency;
/** Cortex M3 Fault Handler (implemented in exceptions.S **/
void FaultHandler( void );
#endif



#ifdef __cplusplus

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//          NOTE: Drivers are supported only in C++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include <handle.h>



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 		 DRIVER INFO
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/**
 * Pointer type to driver info structure
 */
typedef const struct DRIVER_INFO_Type * DRIVER_INFO;

/**
 * Driver DCR() function type
 *
 * DCR() is called from the kernel with a reason code
 *
 * @param drv_info	- pointer to driver info
 * @param reason	- reason code (DCR_XXXX type)
 * @param param		- optional parameter
 */
typedef void (*DRV_DCR)(DRIVER_INFO drv_info, unsigned int reason, void *param);

/**
 * Driver DSR() function type
 *
 * DSR() is called from the kernel with a handle which the driver must process
 *
 * @param drv_info	- pointer to driver info
 * @param hnd		- handle
 */
typedef void (*DRV_DSR)(DRIVER_INFO drv_info, HANDLE hnd);

/**
 * Driver ISR() function type
 *
 * @param drv_info
 */
typedef void (*DRV_ISR)(DRIVER_INFO drv_info);

struct DRIVER_INFO_Type {
	unsigned int op0; /**< ASM instruction */
	unsigned int op4; /**< ASM instruction */

	DRV_ISR isr; /**< pointer to the driver's ISR() */
	DRV_DCR dcr; /**< pointer to the driver's DCR() */
	DRV_DSR dsr; /**< pointer to the driver's DSR() */

	DRIVER_INDEX drv_index; /**< driver index in DRV_TABLE */
	unsigned char isr_priority; /**< driver ISR priority */
	unsigned short peripheral_indx; /**< peripheral index  */
};

/**
 *  Cortex M3 instructions ...?
 */
#define DRIVER_INFO_STUB  0x0004f2af, 0xf008f8d0

void FAST_FLASH drv_enable_isr(DRIVER_INFO drv_info);

/**
 *  Pure virtual function ...?
 */
extern "C" void SLOW_FLASH __cxa_pure_virtual(void);

#endif	/* end C++ */



void SLOW_FLASH usr_task_init_static(TASK_DESCRIPTION const * desc, int bStart);

/**
 * Creates a task using the dynamic memory
 * Examples:
 * @code
 * 		Task* task;
 *
 *      task = usr_task_create_dynamic("CANT", (TASK_FUNCTION) can_thread, 90,
 *				CAN_TASK_STACK_SIZE);
 *		if (task)
 *			svc_task_schedule(task);
 * @endcode
 * @return
 */
Task* usr_task_create_dynamic(const char* name, TASK_FUNCTION func,
		unsigned char priority, unsigned int stack_words);

/**
 * Macro to allocate a task signal
 */
#define ALLOCATE_SIGNAL(sig) CURRENT_TASK->aloc_sig |= sig

/**
 * Macro which test the task for signal
 */
#define tsk_test_signal(sig) (CURRENT_TASK->signals & sig)


#ifdef __cplusplus
extern "C"
{
#endif	/* end C++ */


unsigned int tsk_signal_allocate(void);
void tsk_signal_release(unsigned int signal);

void LowLevelReset( void);
void LowLevelReboot( void);
unsigned int LowLevelResetCause(void);

#ifdef __cplusplus
}
#endif	/* end C++ */

#endif /* PORT_CPP_H_ */

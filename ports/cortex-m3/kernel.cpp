//==========================================================================
// This file is part of TMOS Operating System.
// Copyright (C) 2007-2008 Informa Ltd.
//
// TMOS is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3 or (at your option) any later version.
//
// TMOS is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
//==========================================================================
//
// Author(s):     Miroslav Kostadinov
// Date:          2010-01-20
// Description:   Kernel
//
//========================================================================


#include <tmos.h>

#if USE_CPU_SLEEP_MODE
volatile int cpu_sleep_counter;
#endif

extern TASK_STRU main_task;
extern TASK_DESCRIPTION main_task_desc;
// main task declaration

/**
 * system_clock_frequency is gloabal variable and shows the current clock
 * frequency
 */
volatile __no_init unsigned int system_clock_frequency;
/**
 * APB1_clock_frequency is gloabal variable and shows the current APB1 clock
 * frequency
 */
volatile __no_init unsigned int APB1_clock_frequency;
/**
 * APB2_clock_frequency is gloabal variable and shows the current APB2 clock
 * frequency
 */
volatile __no_init unsigned int APB2_clock_frequency;

#if USE_EXCEPTION_RECORD
volatile __bkp_no_init EXCEPTION_RECORD_STRU exception_record;

extern "C" unsigned int exception_crc(const unsigned int* record)
{
	unsigned int crc=0;
    for(int i = sizeof(EXCEPTION_RECORD_STRU); i>4; i-=4)
        crc += *record++;

    return crc;
}
#endif

//*----------------------------------------------------------------------------
//*			Helper functions
//*----------------------------------------------------------------------------

static void process_exception()
{
    unsigned status = SCB->SCB_CFSR;
#if USE_EXCEPTION_RECORD
	TASK_STACKED_CTX_STRU* msp;
#endif

#if (TRACE_IS != TRACE_DISABLED) || USE_EXCEPTION_RECORD
    unsigned int mem_adr = SCB->SCB_MMFAR;
    unsigned int bus_adr = SCB->SCB_BFAR;

    TRACELN1("\r\nException ");
    if(status&0xFF)
    	TRACELN("MPU: %2.2x at %x ", status&0xFF,  mem_adr);
    if(status&0xFF00)
    	TRACELN("BUS: %2.2x at %x ", (status&0xFF00)>>8, bus_adr);
    if(status&0xFFFF0000)
    	TRACELN("USAGE: %4.4x",(status&0xFFFF0000)>>16);
#endif

    SCB->SCB_CFSR = status;

#if USE_EXCEPTION_RECORD
    exception_record.restart_cause = __get_IPSR();
    exception_record.CFSR = status;
	msp = (TASK_STACKED_CTX_STRU*)__get_PSP();
	if ((uint32_t)msp < BASE_SRAM || (uint32_t)msp >= (BASE_SRAM + RAM_SIZE) || ((uint32_t)msp & 3))
		msp = NULL;
	if( (status & SCB_CFSR_MMFSR_MMARVALID) || !msp)
		exception_record.MMFAR = mem_adr;
	else
		exception_record.MMFAR = msp->pc.as_int;

	if( (status & SCB_CFSR_BFSR_BFARVALID) || !msp)
	    exception_record.BFAR = bus_adr;
	else
		exception_record.BFAR = msp->lr.as_int;
    exception_record.cur_task = (unsigned int)CURRENT_TASK;
    if (((unsigned int) CURRENT_TASK > BASE_SRAM)
			&& ((unsigned int) CURRENT_TASK < (BASE_SRAM + RAM_SIZE)))
    {
    	exception_record.task_name = CURRENT_TASK->name[0]
				+ (CURRENT_TASK->name[1] << 8) + (CURRENT_TASK->name[2] << 16)
				+ (CURRENT_TASK->name[3] << 24);
	} else
    {
    	exception_record.task_name = 0;
    }
    exception_record.record_crc = exception_crc((const unsigned int*)&exception_record);
#endif

}

extern "C" void FaultHandler( void )
{
    volatile unsigned int i=1;

    process_exception();

    if(restart_on_exception)
    {
    	// stop systick
    	SYST->SYST_CSR = 0;
		LowLevelReboot();
    }

    while(i){}
}

/**
 *  DRV_TABLE - extern linker symbol
 */
extern char* const DRV_TABLE[];

WEAK_C void app_init(void)
{

}

//------------- initialize dynamic memory  ---------------//

extern "C" void SLOW_FLASH memory_heap_init( void )
						__attribute__ ((weak, alias ("default_memory_heap_init")));
extern "C" void SLOW_FLASH default_memory_heap_init( void )
{
#if USE_TMOS_STDLIB
#if USED_MEMORY_POOLS > 1 && USE_MCU_DETECT
	uint32_t init_pools = 0;
	if (Cortex_M4F == cpu_identify().type) {
		// initialize TCM pool
		svc_pool_init(&_tcm_end, (void*)(BASE_TCMSRAM +TCMSRAM_SIZE), pool_tcm);
		init_pools++;
		if (!strncmp(device_type, "GD32F4", strlen("GD32F4"))) {
			//256 KB to 768 KB of SRAM
			//1. Memory density information
			uint32_t ram_size = *(uint32_t *)0x1FFF7A20;
			// [15:0]The value indicates the on-chip SRAM density of the device in Kbytes.
			ram_size &= 0xFFFF; // total amount of available RAM, including that of TCM
			ram_size -= 64; 	// subtracts the size of the TCM (64Kbytes) and now we have the size (in Kbytes) of the available memory.
			ram_size <<= 10;    // size in bytes

			//2. Available SRAM for pool(s)
			uint32_t pool_start = (uint32_t)&__stack_svc_start;
			uint32_t pool_end = BASE_SRAM + ram_size;

			ram_size = pool_end - pool_start;
			if (ram_size > POOL_MAX_SIZE) {
				// The memory pool size cannot be more than 127 Kbytes + 1020bytes, so more pools are created
				pool_end = pool_start + POOL_MAX_SIZE;
			}
			uint32_t pool_indx = pool_default;
			while (ram_size) {
				svc_pool_init((void*)pool_start, (void*)pool_end, pool_indx);
				init_pools++;

				ram_size -= (pool_end - pool_start);
				pool_start = pool_end;
				pool_end = pool_start;
				if (ram_size > POOL_MAX_SIZE) {
					pool_end += POOL_MAX_SIZE;
				} else {
					pool_end += ram_size;
				}
				pool_indx += (1<<28);
				if (pool_indx == pool_tcm) {
					pool_indx = pool_addsram_1;
				}
			}

		} else {
			svc_pool_init(&__stack_svc_start, (void*)(BASE_SRAM + RAM_SIZE + SRAM2_SIZE), pool_default);
			init_pools++;
		}

	} else {
		svc_pool_init(&_sram_end, (void*)(BASE_SRAM + RAM_SIZE), 0);
		init_pools++;
	}

	TRACELN1("===== Dynamic memory =======");
	for (uint32_t indx=0; indx < USED_MEMORY_POOLS; indx++) {
		if (init_pools <= indx) {
			memory_heap[indx].start = memory_heap[0].start;
			memory_heap[indx].end = memory_heap[0].end;
			TRACELN("memory #%u not present", indx);
		} else {
			TRACELN("memory #%u size: %.6u bytes",
					indx, (uint32_t)memory_heap[indx].end - (uint32_t)memory_heap[indx].start);
		}

	}
#else
	svc_pool_init(&_sram_end, (void*)(BASE_SRAM + RAM_SIZE));
	TRACELN("===== Dynamic memory %u bytes =====", (char *)(BASE_SRAM + RAM_SIZE) - &_sram_end);
#endif // USED_MEMORY_POOLS
#endif // USE_TMOS_STDLIB
}


//*----------------------------------------------------------------------------
//*     sys_kernel_init
//*
//*	Performs low level initialization.
//*        this function is called before any hi-level C initialization
//*
//*	NOTE: JTAG Debugging may fail in this function if you step while CPU clock
//* 		is lower than JTAG clock
//*----------------------------------------------------------------------------

extern "C" void sys_call_static_ctors(void);

extern "C" void SLOW_FLASH sys_kernel_init( void )
{
    DRIVER_INFO drv_info;
    char *ptr;
    int i;

#if TRACE_IS > TRACE_DISABLED
/*
    //--------------- Start trace -----------------------------//
    CDBG->CDBG_DEMCR |= CDBG_DEMCR_TRCENA;

    //TPIU
    TPIU->TPIU_FFCR = 0x00;							//Formatter and Flush Control Register
    TPIU->TPIU_SPPR = TPIU_SPPR_ASYNC_MANCHESTER;	//Selected Pin Protocol Register
    TPIU->TPIU_FFCR = 0x00;							//Formatter and Flush Control Register
    TPIU->TPIU_CSPSR = TPIU_CSPSR_WORD;				//Current Sync Port Size Register
    TPIU->TPIU_ACPR = 0x04;							//Async Clock Prescaler Register

    //ITM

    //Lock Access Register
    ITM->ITM_LAR = ITM_LAR_KEY;
    //Trace Control Register = trace ID = 1, ITM and DTW enabled
    ITM->ITM_TCR = 0x10000 | ITM_TCR_DWTENA | ITM_TCR_ITMENA;
    ITM->ITM_TER = 0xFFFFFFFF ;						//Trace Enable = all stimulus ports
    ITM->ITM_TPR = 0x0;								//Trace Privilege = all
*/

	TRACELN1("===== " __DATE__ " === " __TIME__ " =====    "); //few more spaces (a clock change follows)
#endif

    // initialize main task
    usr_task_init_static(&main_task_desc, false);
    PMAIN_TASK->tlist = PMAIN_TASK;

    //extra initialization needed for main_task
    PMAIN_TASK->time = 0;								// current time is 000000000
    PMAIN_TASK->state = TSKSTATE_READY;				// leave it suspend ???
#if USE_MEMORY_TRACKING
    PMAIN_TASK->aloc_mem = 0;
    PMAIN_TASK->aloc_ptrs =0;
#endif
	//------------- initialize dynamic memory  ---------------//
#if USE_TMOS_STDLIB
	memory_heap_init();
#endif

    // Reset the drivers i
    for (i = SysTick_IRQn; i < INALID_DRV_INDX; i++)
    {
		ptr = DRV_TABLE[i];
		drv_info = (DRIVER_INFO) (void*) (ptr - 1);
		drv_info->dcr(drv_info, DCR_RESET, NULL);
    }

    // Application level init
    app_init();
#if USE_STATIC_CONSTRUCTORS
    sys_call_static_ctors();
#endif
}

//*----------------------------------------------------------------------------
//*     __cxa_pure_virtual()
//*
//*	Required from the linker. It is placed in vtable for classes with pure virtual
//*
//*----------------------------------------------------------------------------
extern "C" void __cxa_pure_virtual()
{
	TRACE_ERROR("\r\n\e[31mERROR: pure virtual function is called!\e[m");
}

/*-----------------------------------------------------------
 * TASK API
 *----------------------------------------------------------*/


void sys_task_return(void)
{
	while(1)
		usr_task_suspend(CURRENT_TASK);
}

/**
 * Task init
 * @param desc
 * @param bStart
 */
void usr_task_init_static(TASK_DESCRIPTION const * desc, int bStart)
{
	Task *task;
	TASK_STACKED_CTX ctx;

	ctx = (TASK_STACKED_CTX) desc->stack;
	ctx--;
	ctx->psr.as_int = 0x01000000; //thumb mode
	ctx->pc.as_voidptr = (void*) desc->func;
	ctx->lr.as_voidptr = (void*) sys_task_return;

	task = desc->tsk;
	task->sp = ctx;

	task->priority = desc->priority;
	task->time = main_task.time;
	task->prev = task->next = task->tprev = task->tnext = task;
	task->signals = 0;
	task->aloc_sig = 0;
	task->state = TSKSTATE_SUSPEND;
	for (int i = 0;; i++)
	{
		if (!(task->name[i] = desc->name[i]))
			break;
	}

	if (bStart)
	{
		if (__get_IPSR() == 11){
			// SVCall
			svc_task_schedule(task);
		}else{
			usr_task_schedule(task);
		}
	}
}

#ifdef malloc_swi
Task* usr_task_create_dynamic(const char* name, TASK_FUNCTION func,
		unsigned char priority, unsigned int stack_words)
{
	Task* task;

	//Allocate task control block and stack
	stack_words = (stack_words * 4) + sizeof(TASK_STRU);
	if (__get_IPSR() == 11) {
		// SVCall
		task = (Task*) ((unsigned int) svc_malloc(stack_words + 4) + 4);
	} else {
		task = (Task*) ((unsigned int) usr_malloc(stack_words + 4) + 4);
	}

	if(task != (void *)4)
	{
		TASK_STACKED_CTX ctx;
		ctx = (TASK_STACKED_CTX) (((unsigned int)task) + stack_words);
		ctx--;
		ctx->psr.as_int = 0x01000000; //thumb mode
		ctx->pc.as_voidptr = (void*) func;
		ctx->lr.as_voidptr = (void*) sys_task_return;

		task->sp = ctx;

		task->priority = priority;
		task->time = main_task.time;
		task->prev = task->next = task->tprev = task->tnext = task;
		task->signals = 0;
		task->aloc_sig = 0;
		task->state = TSKSTATE_SUSPEND;

		if (strlen(name) > sizeof(task->name)) {
			memcpy(task->name, name, sizeof(Task::name)-1);
			task->name[sizeof(task->name)-1] = 0;
		} else {
			strcpy(task->name, name);
		}
	}
	else
		task = NULL;

	return (task);
}
#endif /*malloc_swi*/

/**
 * Allocates one signal (from 8 to 16)
 * @return signal
 */
unsigned int tsk_signal_allocate(void)
{
	unsigned short signal;

    signal = CURRENT_TASK->aloc_sig;
    signal = (signal+256) & ~signal;
	CURRENT_TASK->aloc_sig |= signal;

	return (signal);
}

/** release signal
 *
 * @param signal
 */
void tsk_signal_release(unsigned int signal)
{
	CURRENT_TASK->aloc_sig &= ~signal;
}



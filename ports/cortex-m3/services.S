//==========================================================================
// This file is part of TMOS Operating System.
// Copyright (C) 2007-2010 Informa Ltd.
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
// Date:          2008-05-01
// Description:   TMOS Software Services
//
// NOTE: called from the SVC Exception Handler
//
//========================================================================
#include "port_asm.h"
#include "core_map.h"
#include "swi_tab.h"


//------------------------------------------------------------------------------
// Helper macros

.macro 		m_RERemove 		item, next, prev
				ldr			\next, [\item, #tsk_renext]
				ldr			\prev, [\item, #tsk_reprev]
				str			\next, [\prev, #tsk_renext]
				str			\prev, [\next, #tsk_reprev]
.endm

.macro 		m_TRemove 		item, next, prev
				ldr			\next, [\item, #tsk_tnext]
				ldr			\prev, [\item, #tsk_tprev]
				str			\next, [\prev, #tsk_tnext]
				str			\prev, [\next, #tsk_tprev]
.endm

.macro 		m_RECleanup 	item
				str			\item, [\item, #tsk_renext]
				str			\item, [\item, #tsk_reprev]
.endm

.macro		m_TCleanup		item
				str			\item, [\item, #tsk_tnext]
				str			\item, [\item, #tsk_tprev]
.endm

.macro		m_REInsert 		next, task, 	prev, val, pty
				ldrb		\pty, [\task, #tsk_pty]
1:
				mov			\prev, \next
				ldr			\next, [\next, #tsk_renext]
				ldrb		\val, [\next, #tsk_pty]
				cmp			\pty, \val
				bcc			1b
				str			\task, [\prev, #tsk_renext]
				str			\task, [\next, #tsk_reprev]
				str			\next, [\task, #tsk_renext]
				str			\prev, [\task, #tsk_reprev]
.endm

.macro		m_PENDSV		ra, rb
                ldr			\ra, = REG_SCB_ICSR
                movs		\rb, #SCB_ICSR_PENDSVSET
                str			\rb, [\ra]
#if USE_CPU_SLEEP_MODE
                ldr         \rb, [\ra, #REG_SCB_SCR - REG_SCB_ICSR]
                bic         \rb, #SCB_SCR_SLEEPONEXIT
                str         \rb, [\ra, #REG_SCB_SCR - REG_SCB_ICSR]
#endif
.endm




.macro		m_TInsert 		next, task, time,   prev, val
1:
				mov			\prev, \next
				ldr			\next, [\next, #tsk_tnext]
				ldr			\val, [\next, #tsk_time]
				cmp			\time, \val
				bcc			1b
				str			\task, [\prev, #tsk_tnext]
				str			\task, [\next, #tsk_tprev]
				str			\next, [\task, #tsk_tnext]
				str			\prev, [\task, #tsk_tprev]
.endm



//*----------------------------------------------------------------------------
//*			Task related services
//*----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//void SendSignal(Task * task, unsigned int signal);
FUNC(	svc_send_signal		)

				ldr				r2, [r0, #tsk_signal]
				ldrb			r3, [r0, #tsk_state]
                orrs			r1, r2
                str				r1, [r0, #tsk_signal]
				cmp				r3, #TSKSTATE_SIGNAL
                bne				_sys_return
				ldr				r2, [r0, #tsk_sp]
				ldr				r3, [r2, #tsk_sp_r1]
                ands			r3, r1
                beq				_sys_return
                str				r3, [r2, #tsk_sp_r0]
                bics			r1, r3
                str				r1, [r0, #tsk_signal]
#if !USE_TASK_REGISTERS
				ldr				r1, =main_task
#endif
                b				svc_task_sch

ENDF(	svc_send_signal		)


//------------------------------------------------------------------------------
//void task_schedule(Task *task)
FUNC(	svc_task_schedule	)
#if USE_MEMORY_TRACKING
				movs			r2, #0
				str				r2, [r0, #tsk_aloc_mem]
				str				r2, [r0, #tsk_aloc_ptrs]
				mov				r2, #RES_OK
				str				r2, [r0, #tsk_res]
#endif
#if USE_TASK_REGISTERS
                ldr             r2, [REG_MTASK, #tsk_tlist]
                str             r2, [r0, #tsk_tlist]
                str             r0, [REG_MTASK, #tsk_tlist]
#else
				ldr				r1, =main_task
                ldr             r2, [r1, #tsk_tlist]
                str             r2, [r0, #tsk_tlist]
                str             r0, [r1, #tsk_tlist]
#endif
svc_task_sch:
				m_RERemove		r0, r2, r3			//?
				m_TRemove		r0, r2, r3
				m_TCleanup		r0

#if USE_TASK_REGISTERS
				mov				r1, REG_MTASK
#endif
				m_REInsert		r1, r0,	  r2, r3, r12
                movs			r1, #TSKSTATE_READY
                strb			r1, [r0, #tsk_state]

                m_PENDSV		r1, r2
_sys_return:
				bx				lr

ENDF(	svc_task_schedule	)


//------------------------------------------------------------------------------
//void sleep_until( unsigned int time )
FUNC(	sys_sleep_until )

#if USE_TASK_REGISTERS
				mov				r1, REG_CTASK
				mov				r2, REG_MTASK
#else
				ldr				r2, =main_task
                ldr             r1, [r2, #tsk_int_task]
#endif
				ldr				r3, [r2, #tsk_time]			//cur time
				cmp				r0, r3
				bhi				TaskDelay
				bx				lr

ENDF(	sys_sleep_until )


//------------------------------------------------------------------------------
//void sleep_rel( unsigned int time )
FUNC(	sys_sleep_rel	)

#if USE_TASK_REGISTERS
				mov				r1, REG_CTASK
#else
				ldr				r2, =main_task
                ldr             r1, [r2, #tsk_int_task]
#endif
				ldr				r3, [r1, #tsk_time]					//task time
				add				r0, r0, r3
				str				r0, [r1, #tsk_time]

#if USE_TASK_REGISTERS
				ldr				r3, [REG_MTASK, #tsk_time]			//cur time
#else
				ldr				r3, [r2, #tsk_time]			//cur time
#endif

				subs			r0, r0, r3
				lsls			r0, r0, #1
                bmi				_sys_return
                lsrs			r0, r0, #1
                b				sys_sleep

ENDF(	sys_sleep_rel	)


//------------------------------------------------------------------------------
//void slep( unsigned int time )
FUNC(	sys_sleep		)

				cmp				r0, #0
				ble				_sys_return							//0 < time <0x80000000
#if USE_TASK_REGISTERS
				mov				r1, REG_CTASK
				mov				r2, REG_MTASK
#else
				ldr				r2, =main_task
                ldr             r1, [r2, #tsk_int_task]
#endif
				ldr				r3, [r2, #tsk_time]					//cur time
				add				r0, r0, r3
TaskDelay:
                movs			r3, #TSKSTATE_TIMED
                strb			r3, [r1, #tsk_state]
				str				r0, [r1, #tsk_time]
                mov				r12, r0
				m_RERemove		r1, r0, r3
				m_RECleanup		r1
				m_TInsert		r2, r1, r12, r0, r3
                m_PENDSV		r1, r2
				bx				lr

ENDF(	sys_sleep		)




//------------------------------------------------------------------------------
//void IncrementTicks(void);
FUNC(	isr_ClockDriver		)

#if USE_TASK_REGISTERS
				mov				r1, REG_MTASK
#else
				ldr				r1, =main_task
#endif
				ldr				r2, [r1, #tsk_time]					//cur time
                adds			r2, #1
				bmi				overlap_time
				str				r2, [r1, #tsk_time]					//cur time
overlap_done:
				ldr				r0, [r1, #tsk_tprev]
				ldr				r3, [r0, #tsk_time]					//tsk time
                cmp				r2, r3
				bhi				weakup_tasks_loop
                bx				lr

weakup_tasks_loop:
                movs			r2, #TSKSTATE_READY
                strb			r2, [r0, #tsk_state]
				m_REInsert		r1, r0,   r2, r3, r12
				m_TRemove		r0, r1, r3						//r1=MTASK, r3=prev
				m_TCleanup		r0

				ldr				r2, [r1, #tsk_time]					//cur time
				mov				r0, r3
				ldr				r3, [r0, #tsk_time]					//tsk time
                cmp				r2, r3
				bhi				weakup_tasks_loop
                m_PENDSV		r1, r2
                bx				lr


overlap_loop:
                lsrs			r2, r2, #1
				str				r2, [r1, #tsk_time]					//tsk time
				ldr				r1, [r1, #tsk_tprev]
				ldr				r2, [r1, #tsk_time]					//tsk time
overlap_time:
                lsls			r2, r2, #1
                bcs				overlap_loop
#if USE_TASK_REGISTERS
				mov				r1, REG_MTASK
#else
				ldr				r1, =main_task
#endif
				b				overlap_done


ENDF(	isr_ClockDriver		)






//------------------------------------------------------------------------------
//unsigned int test_signal(unsigned int signal);
FUNC(	sys_test_signal		)

#if USE_TASK_REGISTERS
				mov				r3, REG_CTASK
#else
				ldr				r2, =main_task
                ldr             r3, [r2, #tsk_int_task]
#endif
				ldr				r1, [r3, #tsk_signal]
				ands			r0, r1
				mrs 			r2, PSP					// Get Task SP
_return_signal:
                bics			r1, r0
				str				r1, [r3, #tsk_signal]
				str				r0, [r2, tsk_sp_r0]
				bx				lr

ENDF(	sys_test_signal		)

//------------------------------------------------------------------------------
//RES_CODE tsk_start_and_wait(HANDLE hnd);
FUNC(	sys_drv_service_wait  )

				push			{r0,lr}
				bl				svc_drv_service
				pop				{r0,lr}
				ldrb			r0, [r0, #HND_SIGNAL]
				b				sys_get_signal

ENDF(	sys_drv_service_wait  )

//------------------------------------------------------------------------------
//unsigned int get_signal(unsigned int signal);
FUNC(	sys_get_signal		)

#if USE_TASK_REGISTERS
				mov				r3, REG_CTASK
#else
				ldr				r2, =main_task
                ldr             r3, [r2, #tsk_int_task]
#endif
				mrs 			r2, PSP					// Get Task SP
                str				r0, [r2, #tsk_sp_r1]
				ldr				r1, [r3, #tsk_signal]
				ands			r0, r1
				bne				_return_signal
                str				r0, [r2, #tsk_sp_r0]

                movs			r0, #TSKSTATE_SIGNAL
s_susp_task:
 				str				r2, [r3]				//REG_CTASK->sp early update, because IRQ may get serviced before task switch
                strb			r0, [r3, #tsk_state]
				m_RERemove		r3, r0, r1
				m_RECleanup		r3
                m_PENDSV		r1, r2
				bx				lr

ENDF(	sys_get_signal		)


//------------------------------------------------------------------------------
// void task_suspend(Task *task)
FUNC(	svc_task_suspend	)

#if USE_TASK_REGISTERS
                mov             r1, REG_MTASK
#else
				ldr				r1, =main_task
				mov				r3, r1
#endif
s_susp_tlist:
                ldr             r2, [r1, #tsk_tlist]
                cmp             r0, r2
                beq             s_susp_lfound
                mov             r1, r2
#if USE_TASK_REGISTERS
                cmp             r1, REG_MTASK
#else
                cmp             r1, r3
#endif
                bne             s_susp_tlist
                beq             s_susp_nfound
s_susp_lfound:
                ldr             r2, [r2, #tsk_tlist]
                str             r2, [r1, #tsk_tlist]
s_susp_nfound:
				m_TRemove		r0, r2, r3
				m_TCleanup		r0
#ifdef free_swi
				push			{r0,lr}
				subs			r0, #4
				bl				svc_free
				pop				{r3,lr}
#else
				mov				r3, r0
#endif
                movs			r0, #TSKSTATE_SUSPEND
				b				s_susp_task

ENDF(	svc_task_suspend	)

//------------------------------------------------------------------------------
//unsigned int resume_wait_signal(unsigned int signal);
FUNC(	sys_resume_wait_signal	)

#if USE_TASK_REGISTERS
				ldr				r2, [REG_MTASK, #tsk_time]			// t2=cur time
				ldr				r1, [REG_CTASK, #tsk_time]			// t1 + Delay
#else
				ldr				r2, =main_task
                ldr             r3, [r2, #tsk_int_task]
				ldr				r2, [r2, #tsk_time]					// t2=cur time
				ldr				r1, [r3, #tsk_time]					// t1 + Delay
#endif
				subs			r1, r1, r2							// Delay + t1 - t2
				lsls			r1, r1, #1
                bmi				sys_test_signal
                lsrs			r1, r1, #1
                b				sys_wait_signal

ENDF(	sys_resume_wait_signal	)


//------------------------------------------------------------------------------
//unsigned int wait_signal(unsigned int signal, unsigned int time);
FUNC(	sys_wait_signal		)

				cmp				r1, #0
				ble				sys_test_signal						//0 < time <0x80000000

#if USE_TASK_REGISTERS
				ldr				r12, [REG_MTASK, #tsk_time]			//cur time
				mov				r3, REG_CTASK
//				cmp				r3, REG_MTASK
//              beq				sys_test_signal
#else
				ldr				r2, =main_task
                ldr             r3, [r2, #tsk_int_task]
				ldr				r12, [r2, #tsk_time]			//cur time
#endif
				adds			r12, r1
				str				r12, [r3, #tsk_time]

				ldr				r1, [r3, #tsk_signal]
				mrs 			r2, PSP					// Get Task SP
                str				r2, [r3, #tsk_sp]		//Early update before task switch
                str				r0, [r2, #tsk_sp_r1]
				ands			r0, r1
                bne				_return_signal
                str				r0, [r2, #tsk_sp_r0]
#if USE_TASK_REGISTERS
				mov				r2, REG_MTASK
#else
				ldr				r2, =main_task
#endif

                movs			r0, #TSKSTATE_SIGNAL
                strb			r0, [r3, #tsk_state]
				m_RERemove		r3, r0, r1
				m_RECleanup		r3
				m_TInsert		r2, r3, r12, 	r0, r1
                m_PENDSV		r1, r2
				bx				lr

ENDF(	sys_wait_signal		)







//*----------------------------------------------------------------------------
//*			Driver related services
//*----------------------------------------------------------------------------

//------------------------------------------------------------------------------
//void svc_HND_SET_STATUS(HANDLE hnd, RES_CODE result)
FUNC(	svc_HND_SET_STATUS	)
				.extern DRV_TABLE


				ldr				r3, [r0, #HND_CLIENT]
				strb			r1, [r0, #HND_RES]
				cmp				r3, #255
				bls				1f			//driver
				lsls			r2, r3, #30
				bne				2f			// thumb callback
				ldrb			r1, [r0, #HND_SIGNAL]
				mov				r0, r3
				b				svc_send_signal

1://driver
				mov				r2, r0
				ldr				r0, =DRV_TABLE
				ldr				r0, [r0, r3, lsl #2]
				subs			r0, #1
                ldr				r3, [r0, #DRV_INFO_DCR]
				ldrb			r1, [r2, #HND_SIGNAL]
2:
                bx				r3			//DCR(r0-info, r1-sig, r2-hnd)

ENDF(	svc_HND_SET_STATUS	)


//------------------------------------------------------------------------------
//RES_CODE drv_hcontrol(HANDLE hnd, unsigned int reason);
FUNC(	svc_drv_hcontrol	)

				mov				r2, r0
				ldrb			r0, [r2, #HND_DRV_INDX]
				b				svc_drv_icontrol

ENDF(	svc_drv_hcontrol	)


//------------------------------------------------------------------------------
//RES_CODE drv_icontrol(DRIVER_INDEX drv_index, unsigned int reason, void * param);
FUNC(	svc_drv_icontrol	)

				ldr				r3, =DRV_TABLE
				ldr				r0, [r3, r0, lsl #2]
				subs			r0, #1
                ldr				r3, [r0, #DRV_INFO_DCR]
                bx				r3							//DCR(r0-info, r1-reason, r2-param)

ENDF(	svc_drv_icontrol	)


//------------------------------------------------------------------------------
//RES_CODE drv_service(HANDLE hnd);
FUNC(	svc_drv_service		)

				ldrsb			r2, [r0, #HND_DRV_INDX]
				ldr				r3, =DRV_TABLE
				mov				r1, r0
				ldr				r0, [r3, r2, lsl #2]
				subs			r0, #1
                ldr				r3, [r0, #DRV_INFO_DSR]
                bx				r3							//DSR(r0-info, r1-handle)

ENDF(	svc_drv_service		)

//------------------------------------------------------------------------------
//void isr_control(DRIVER_INFO drv_info, unsigned int reason, void *param);
FUNC(	sys_isr_contol		)

                ldr				r3, [r0, #DRV_INFO_DCR]
                bx				r3			//DCR(r0-info, r1-reason, r2-param)

ENDF(	sys_isr_contol		)

#if USE_CPU_SLEEP_MODE
//void svc_wfi(void)
FUNC(   svc_wfi      )
                ldr             r1, =cpu_sleep_counter
                ldr             r0, [r1]
                cbnz            r0, 1f
                ldr             r1, =REG_SCB_SCR
                ldr             r2, [r1]
                orrs            r2, #SCB_SCR_SLEEPONEXIT
                str             r2, [r1]
1:
                bx              lr
ENDF(   svc_wfi      )
#endif

#define TRACE_PORT 0
//------------------------------------------------------------------------------
//void swd_trace_str(const char *buf);
FUNC(	swd_trace_str		)

.weak	usr_trace_str
.thumb_set	usr_trace_str, swd_trace_str

				movs			r1, #BASE_ITM
                ldr             r3, [r1, # REG_ITM_TER - BASE_ITM]
                cbz             r3, 4f
1:
				ldrb			r2, [r0], #1
				cbz				r2, 4f
2:
				ldrexb			r3, [r1]
				cbz				r3, 3f
				strexb			r3, r2, [r1]
				subs			r0, r3
				b				1b
3:
				b				2b
4:
				bx				lr

ENDF(	swd_trace_str		)

//------------------------------------------------------------------------------
//void swd_trace_char(char c);
FUNC(	swd_trace_char	)

.weak	usr_trace_char
.thumb_set	usr_trace_char, swd_trace_char

				movs			r1, #BASE_ITM + (TRACE_PORT * 4)
				cbz				r0, 2f
                ldr             r3, [r1, # REG_ITM_TER - BASE_ITM]
                cbz             r3, 2f
1:
				ldrexb			r3, [r1]
				cmp 			r3, #0
				ITT				NE
				strexbne		r3, r0, [r1]
				cmpne			r3, #1
				beq				1b
2:
				bx				lr

ENDF(	swd_trace_char	)

//------------------------------------------------------------------------------
//unsigned int  locked_inc_byte(volatile unsigned char* Addend);
//returns the resulting incremented value
FUNC(	locked_inc_byte		)

				movs			r1, #1
				b				locked_add_byte

ENDF(	locked_inc_byte		)

//------------------------------------------------------------------------------
//unsigned int  locked_add_byte(volatile unsigned char* Addend, unsigned int);
FUNC(	locked_add_byte		)

1:
				ldrexb			r3, [r0]
				adds			r3, r1
				strexb			r2, r3, [r0]
				cmp				r2, #0
				bne				1b
				mov				r0, r3
				bx				lr
ENDF(	locked_add_byte		)

//------------------------------------------------------------------------------
//unsigned int  locked_dec_byte(volatile unsigned char* Addend);
//returns the Decremented value
FUNC(	locked_dec_byte		)

1:
				ldrexb			r3, [r0]
				subs			r3, #1
				strexb			r2, r3, [r0]
				cmp				r2, #0
				bne				1b
				mov				r0, r3
				bx				lr
ENDF(	locked_dec_byte		)

//------------------------------------------------------------------------------
//unsigned int  locked_set_byte(volatile unsigned char& var, unsigned int flags);
FUNC(	locked_set_byte		)

1:
				ldrexb			r3, [r0]
				orrs			r3, r1
				strexb			r2, r3, [r0]
				cmp				r2, #0
				bne				1b
				mov				r0, r3
				bx				lr
ENDF(	locked_set_byte		)

//------------------------------------------------------------------------------
//unsigned int  locked_set_int(volatile unsigned int& var, unsigned int flags);
FUNC(	locked_set_int		)

1:
				ldrex			r3, [r0]
				orrs			r3, r1
				strex			r2, r3, [r0]
				cmp				r2, #0
				bne				1b
				mov				r0, r3
				bx				lr
ENDF(	locked_set_int		)

//------------------------------------------------------------------------------
//unsigned int  locked_clr_byte(volatile unsigned char& var, unsigned int flags);
FUNC(	locked_clr_byte		)
				mov				r12, r0
1:
				ldrexb			r3, [r12]
				and				r0, r1, r3
				bics			r3, r0
				strexb			r2, r3, [r12]
				cmp				r2, #0
				bne				1b
				bx				lr
ENDF(	locked_clr_byte		)

//------------------------------------------------------------------------------
//unsigned int  locked_clr_int(volatile unsigned int& var, unsigned int flags);
FUNC(	locked_clr_int		)
				mov				r12, r0
1:
				ldrex			r3, [r12]
				and				r0, r1, r3
				bics			r3, r0
				strex			r2, r3, [r12]
				cmp				r2, #0
				bne				1b
				bx				lr
ENDF(	locked_clr_int		)

//------------------------------------------------------------------------------
//unsigned int  locked_inc_short(volatile unsigned short* Addend);
//returns the resulting incremented value
FUNC(	locked_inc_short		)

				movs			r1, #1
				b				locked_add_short

ENDF(	locked_inc_short		)

//------------------------------------------------------------------------------
//unsigned int  locked_add_short(volatile unsigned short* Addend, unsigned int);
//returns the resulting incremented value
FUNC(	locked_add_short		)

1:
				ldrexh			r3, [r0]
				adds			r3, r1
				strexh			r2, r3, [r0]
				cmp				r2, #0
				bne				1b
				mov				r0, r3
				bx				lr
ENDF(	locked_add_short		)

//------------------------------------------------------------------------------
//unsigned int  locked_dec_short(volatile unsigned short* Addend);
//returns the Decremented value
FUNC(	locked_dec_short		)

1:
				ldrexh			r3, [r0]
				subs			r3, #1
				strexh			r2, r3, [r0]
				cmp				r2, #0
				bne				1b
				mov				r0, r3
				bx				lr
ENDF(	locked_dec_short		)

//------------------------------------------------------------------------------
//unsigned int  locked_inc_int(volatile unsigned int* Addend);
//returns the resulting incremented value
FUNC(	locked_inc_int		)

				movs			r1, #1
				b				locked_add_int

ENDF(	locked_inc_int		)

//------------------------------------------------------------------------------
//unsigned int  locked_add_int(volatile unsigned int* Addend, unsigned int);
//returns the resulting incremented value
FUNC(	locked_add_int		)

1:
				ldrex			r3, [r0]
				adds			r3, r1
				strex			r2, r3, [r0]
				cmp				r2, #0
				bne				1b
				mov				r0, r3
				bx				lr
ENDF(	locked_add_int		)

//------------------------------------------------------------------------------
//unsigned int  locked_dec_int(volatile unsigned int* Addend);
//returns the Decremented value
FUNC(	locked_dec_int		)

1:
				ldrex			r3, [r0]
				subs			r3, #1
				strex			r2, r3, [r0]
				cmp				r2, #0
				bne				1b
				mov				r0, r3
				bx				lr
ENDF(	locked_dec_int		)

//------------------------------------------------------------------------------
//void*  locked_set_if_null(void* pointer, void* value);
//set a pointer variable if it is NULL
//returns the initial pointer value (NULL for success)
FUNC(	locked_set_if_null		)

				mov				r2, r0
1:
				ldrex			r0, [r2]
				cbnz			r0, 2f
				strex			r0, r1, [r2]
				cmp				r0, #0
				bne				1b
				bx				lr
2:
				clrex
				bx				lr

ENDF(	locked_set_if_null		)

.end

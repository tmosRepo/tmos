/**************************************************************************//**
 * @ingroup	 ports_cortex
 * @file
 * @brief    LowLevel init for STM32
 *
 */


#include <tmos.h>
#include <fam_cpp.h>

#ifndef GD_USB_SDIO_USE_PLLSAI
#define GD_USB_SDIO_USE_PLLSAI		1
#endif

#ifndef GD_USB_SDIO_USE_IRC48M
#define GD_USB_SDIO_USE_IRC48M		0
#endif

#if GD_USB_SDIO_USE_PLLSAI && GD_USB_SDIO_USE_IRC48M
#error "cannot use PLLSAI and IRC48M at the same time"
#endif

#ifndef GDF42_F45_AT_200MHz
#define GDF42_F45_AT_200MHz 1
#endif

#if GDF42_F45_AT_200MHz && !(GD_USB_SDIO_USE_PLLSAI || GD_USB_SDIO_USE_IRC48M)
#warning "USB may work unstable !!!"
#endif

#ifndef HSE_STARTUP_TIMEOUT
#define HSE_STARTUP_TIMEOUT 0xFFFF
#endif

#ifdef HSE_VALUE
#if HSE_VALUE >= 1000000 && HSE_VALUE < 32000000
#define HSE_PLL_M   RCC_PLLCFGR_PLLM_Set(HSE_VALUE/1000000)
#else // HSE_VALUE
#error "Invalid XTAL value"
#endif // HSE_VALUE
#else // not defined HSE_VALUE
#error "The XTAL value must be defined!"
#endif

#ifndef HSI_VALUE
#define HSI_VALUE 16000000
#endif

#define HSI_PLL_M   RCC_PLLCFGR_PLLM_Set(HSI_VALUE/1000000)

static void SLOW_FLASH Reset_clocks_configuration()
{
	/* Reset the RCC clock configuration to the default reset state ------------*/

	RCC->RCC_CR |= RCC_CR_HSION; //Internal High Speed clock enable

	while(!(RCC->RCC_CR & RCC_CR_HSIRDY)){
		// wait until the Internal High Speed clock is ready
	}

	// decrease sys clock
	RCC->RCC_CFGR |= RCC_CFGR_HPRE_DIV2;
	for(volatile int i=0; i < 80; i++)
	{;}
	RCC->RCC_CFGR |= RCC_CFGR_HPRE_DIV4;
	for(volatile int i=0; i < 80; i++)
	{;}

	/* Select the Internal High Speed clock as system clock source */
	RCC->RCC_CFGR = (RCC->RCC_CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSI;
	while ((RCC->RCC_CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI){
		/* wait until Internal High Speed clock is selected as system clock */
	}

	// keep System clock switch
	RCC->RCC_CFGR &= RCC_CFGR_SW;
	/* Reset HSEON, CSSON and PLLON bits */
	RCC->RCC_CR &= ~(RCC_CR_PLLON | RCC_CR_HSEON | RCC_CR_CSSON);
	/* Reset HSEBYP bit */
	RCC->RCC_CR &= ~(RCC_CR_HSEBYP);
	/* Reset CFGR register */
	RCC->RCC_CFGR = 0x00000000;
	/* Reset PLLCFGR register */
	RCC->RCC_PLLCFGR =  0x24003010;
	/* Disable all interrupts */
	RCC->RCC_CIR = 0x00000000;
}

static void SLOW_FLASH Init_main_PLL(const uint32_t PLL_M, bool over_drive)
{
	if(over_drive){
		//These bits can be modified only when the PLL is OFF.
		//The new value programmed is active only when the PLL is ON.
		RCC->RCC_APB1ENR |= RCC_APB1ENR_PWREN;
		PWR->PWR_CR |= PWR_CR_VOS_1;	//Regulator voltage scaling output selection
	}

	/* HCLK = SYSCLK / 1*/
	RCC->RCC_CFGR |= RCC_CFGR_HPRE_DIV1;

	/* PCLK2 = HCLK / 2*/
	RCC->RCC_CFGR |= RCC_CFGR_PPRE2_DIV2;

	/* PCLK1 = HCLK / 4*/
	RCC->RCC_CFGR |= RCC_CFGR_PPRE1_DIV4;

	switch(system_clock_frequency)
	{
	case 240:
		/* Configure the main PLL, PSC = 25, PLL_N = 480, PLL_P = 2, PLL_Q = 10 */
		//CK_PLLVCOSRC = CK_PLLSRC / PLLPSC = 25 000 000 / 25 = 1 000 000
		//CK_PLLVCO = CK_PLLVCOSRC x PLLN = 1 000 000 x 480 = 480 000 000
		//CK_PLLP = CK_PLLVCO / PLLP = 480 000 000 / 2 = 240 000 000
		//CK_PLLQ = CK_PLLVCO / PLLQ = 480 000 000 / 10 =  48 000 000
		RCC->RCC_PLLCFGR = PLL_M
		| RCC_PLLCFGR_PLLN_Set(480)
		| RCC_PLLCFGR_PLLP_DIV2
		| RCC_PLLCFGR_PLLQ_Set(10)
		| RCC_PLLCFGR_PLLSRC_HSE ;
		break;
	case 200:
		/* Configure the main PLL, PSC = 25, PLL_N = 400, PLL_P = 2, PLL_Q = 9 */
		//CK_PLLVCOSRC = CK_PLLSRC / PLLPSC = 25 000 000 / 25 = 1 000 000
		//CK_PLLVCO = CK_PLLVCOSRC x PLLN = 1 000 000 x 400 = 400 000 000
		//CK_PLLP = CK_PLLVCO / PLLP = 400 000 000 / 2 = 200 000 000
		//CK_PLLQ = CK_PLLVCO / PLLQ = 400 000 000 / 9 =  44 444 444 != 48 000 000
		RCC->RCC_PLLCFGR = PLL_M
		| RCC_PLLCFGR_PLLN_Set(400)
		| RCC_PLLCFGR_PLLP_DIV2
		| RCC_PLLCFGR_PLLQ_Set(9)
		| RCC_PLLCFGR_PLLSRC_HSE ;
		break;
	case 192:
		/* Configure the main PLL, PSC = 25, PLL_N = 384, PLL_P = 2, PLL_Q = 8 */
		//CK_PLLVCOSRC = CK_PLLSRC / PLLPSC = 25 000 000 / 25 = 1 000 000
		//CK_PLLVCO = CK_PLLVCOSRC x PLLN = 1 000 000 x 384 = 384 000 000
		//CK_PLLP = CK_PLLVCO / PLLP = 384 000 000 / 2 = 192 000 000
		//CK_PLLQ = CK_PLLVCO / PLLQ = 384 000 000 / 8 =  48 000 000
		RCC->RCC_PLLCFGR = PLL_M
		| RCC_PLLCFGR_PLLN_Set(384)
		| RCC_PLLCFGR_PLLP_DIV2
		| RCC_PLLCFGR_PLLQ_Set(8)
		| RCC_PLLCFGR_PLLSRC_HSE ;
		break;
	case 168:
		/* Configure the main PLL, PSC = 25, PLL_N = 336, PLL_P = 2, PLL_Q = 7 */
		//CK_PLLVCOSRC = CK_PLLSRC / PLLPSC = 25 000 000 / 25 = 1 000 000
		//CK_PLLVCO = CK_PLLVCOSRC x PLLN = 1 000 000 x 336 = 336 000 000
		//CK_PLLP = CK_PLLVCO / PLLP = 336 000 000 / 2 = 168 000 000
		//CK_PLLQ = CK_PLLVCO / PLLQ = 336 000 000 / 7 =  48 000 000
		RCC->RCC_PLLCFGR = PLL_M
		| RCC_PLLCFGR_PLLN_Set(336)
		| RCC_PLLCFGR_PLLP_DIV2
		| RCC_PLLCFGR_PLLQ_Set(7)
		| RCC_PLLCFGR_PLLSRC_HSE ;
		break;
	default: // 120
		/* Configure the main PLL, PSC = 25, PLL_N = 240, PLL_P = 2, PLL_Q = 5 */
		//CK_PLLVCOSRC = CK_PLLSRC / PLLPSC = 25 000 000 / 25 = 1 000 000
		//CK_PLLVCO = CK_PLLVCOSRC x PLLN = 1 000 000 x 240 = 240 000 000
		//CK_PLLP = CK_PLLVCO / PLLP = 240 000 000 / 2 = 120 000 000
		//CK_PLLQ = CK_PLLVCO / PLLQ = 240 000 000 / 5 =  48 000 000
		RCC->RCC_PLLCFGR = PLL_M
		| RCC_PLLCFGR_PLLN_Set(240)
		| RCC_PLLCFGR_PLLP_DIV2
		| RCC_PLLCFGR_PLLQ_Set(5)
		| RCC_PLLCFGR_PLLSRC_HSE ;
		break;
	}

	/* Configure spread spectrum clock generation */
	RCC->RCC_SSCGR = RCC_SSCGR_SSCGEN |(126<<12) | (250);

	/* Enable the main PLL */
	RCC->RCC_CR |= RCC_CR_PLLON;

	while ((RCC->RCC_CR & RCC_CR_PLLRDY) == 0){
		/* Wait till the main PLL is ready */
	}

	if(over_drive){
		/* Enable the high-drive to extend the clock frequency to 200 Mhz */
		PWR->PWR_CR  |= PWR_CR_ODEN; // Over-drive enable
		while(0U == (PWR->PWR_CSR & PWR_CSR_ODRDY)){
			// Over-drive mode ready
		}

		/* select the high-drive mode */
		PWR->PWR_CR |= PWR_CR_ODSWEN; // Over-drive switching enabled.
		while(0U == (PWR->PWR_CSR & PWR_CSR_ODSWRDY)){
			// Over-drive mode switching ready
		}
	}

}

/**
  * @brief  Configures the System clock source, PLL Multiplier and Divider factors,
  *         AHB/APBx prescalers and Flash settings
  * @Note   This function should be called only once the RCC clock configuration
  *         is reset to the default reset state (done in SystemInit() function).
  * @param  None
  * @retval None
  */
extern "C" void SLOW_FLASH SystemInit( void )
						__attribute__ ((weak, alias ("DefaultSystemInit")));
extern "C" void SLOW_FLASH DefaultSystemInit( void )
{
	__IO uint32_t StartUpCounter = 0, HSEStatus;
	bool over_drive = false;
	/* Reset the RCC clock configuration to the default reset state ------------*/
	Reset_clocks_configuration();

	/* Enable HSE */
	RCC->RCC_CR |= RCC_CR_HSEON;
	/* Wait till HSE is ready and if Time out is reached exit */
	do{
		HSEStatus = RCC->RCC_CR & RCC_CR_HSERDY;
		StartUpCounter++;
	} while ((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

	get_mcu_name();
	if(0 == strncmp(mcu_name, "GD32F4", strlen("GD32F4"))){

		RCC->RCC_PLLSAICFGR = 0x24003000; // reset value
		over_drive = true;
		switch(mcu_name[strlen("GD32F4")])
		{
		case '7': // GD32F47X
			// 240 MHz
			system_clock_frequency =240;
			break;
		case '2': // GD32F42X
		case '5': // GD32F45X
#if GDF42_F45_AT_200MHz
			// 200 MHz
			system_clock_frequency =200;
#else
			// 192 MHz
			system_clock_frequency =192;
#endif
			break;
		default: // GD32F40X
			// 168 MHz
			system_clock_frequency =168;
			break;
		}
	}else if(0 == strncmp(mcu_name, "APM32F4", strlen("APM32F4"))){
		system_clock_frequency =168;
	}else{
		//ST32
		if(Cortex_M4F == get_core().type)
		{
			switch(mcu_name[strlen("STM32F4")])
			{
			case '0':
			case '1':
				system_clock_frequency =120;
				break;
			default:
				//SM32F4 2/3/7/6
				over_drive = true;
				system_clock_frequency =168;
			}

		}else{
			system_clock_frequency =120;
		}
	}

	if(HSEStatus){
		Init_main_PLL(HSE_PLL_M, over_drive);
	}else{
    	// If HSE fails to start-up, the application will have wrong clock
    	// configuration. User can add here some code to deal with this error
    	TRACELN1("Failed to start HSE");
		Init_main_PLL(HSI_PLL_M, over_drive);
	}

	// MCU specific settings
	if(mcu_name[0] == 'A' || mcu_name[0] == 'S' ){ // APM or STM

	/* Configure Flash prefetch, Instruction cache, Data cache and wait state */
	FLASH->FLASH_ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN
					| FLASH_ACR_LATENCY_3WS;

	}else if(mcu_name[0] == 'G'){ // GigaDevice

		uint32_t reg = RCC->RCU_ADDCTL;
#if GD_USB_SDIO_USE_PLLSAI || GD_USB_SDIO_USE_IRC48M
		if(system_clock_frequency == 200)
		{
#if GD_USB_SDIO_USE_PLLSAI
			RCC->RCC_CR &= ~(RCC_CR_PLLSAION);
			/* Configure the SAI PLL, PSC = 25, PLL_N = 192, PLL_P = 4, PLL_R = 4 */
			// CK_PLLSAIVCOSRC = CK_PLLSRC / PLLPSC = 25 000 000 / 25 = 1 000 000
			// CK_PLLSAIVCO = CK_ PLLSAIVCOSRC x PLLSAIN = 1 000 000 x 192 = 192 000 000
			// CK_PLLSAIP = CK_ PLLSAIVCO / PLLSAIP = 192 000 000 / 4 = 48 000 000
			// CK_PLLSAIR = CK_ PLLSAIVCO / PLLSAIR = 192 000 000 / 4 = 48 000 000 not used

			RCC->RCC_PLLSAICFGR =
					 RCC_PLLSAICFGR_PLLSAIN_Set(192)
					|RCC_PLLSAICFGR_PLLSAIP_DIV4
					|RCC_PLLSAICFGR_PLLSAIR_DIV4;

			/* enable PLLSAI */
			RCC->RCC_CR |= RCC_CR_PLLSAION;

			/* wait until PLLSAI is stable */
			while ((RCC->RCC_CR & RCC_CR_PLLSAIRDY) == 0)
			{
			}
		/* switch to USB,TRING and SDIO */
			reg |= RCU_PLL48MSEL; // Select CK_PLLSAIP clock
			reg &= ~(RCU_CK48MSEL); // disable IRC48M clock
#endif //GD_USB_SDIO_USE_PLLSAI
#if GD_USB_SDIO_USE_IRC48M
			// enable IRC48M
			RCC->RCU_ADDCTL |= RCU_IRC48MEN;

			// wait until IRC48M is stable
			while ((RCC->RCU_ADDCTL & RCU_IRC48MSTB) == 0){
			}
			reg = RCC->RCU_ADDCTL;
			reg |= RCU_CK48MSEL;
#endif // GD_USB_SDIO_USE_IRC48M
		}else
#endif // GD_USB_SDIO_USE_PLLSAI || GD_USB_SDIO_USE_IRC48M
		{
			// Does not use IRC48M clock, uses the main PLLQ as the USB/SDIO clock
			reg &= ~(RCU_PLL48MSEL|RCU_CK48MSEL);
		}
		RCC->RCU_ADDCTL = reg;
	}else{
		/* no operation */
	}

	/******************************************************************************/
	/*            PLL (clocked by HSE/HSI) used as System clock source            */
	/******************************************************************************/
	RCC->RCC_CFGR &= ~(RCC_CFGR_SW);
	RCC->RCC_CFGR |= RCC_CFGR_SW_PLL;

	while ((RCC->RCC_CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL){
		/* wait until PLL is selected as system clock */
	}
	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	// when Cortex_M4 is used, move service stack to TCM ram.
	if(Cortex_M4F == get_core().type){
		asm volatile (
		"		ldr			r2, =__stack_svc_end      \n\t"
		"		mov			r1, sp                    \n\t"
		"		subs		r2, r1                    \n\t"
		"		ldr			r0, =__tcm_stack_svc_end  \n\t"
		"		subs		r0, r2                    \n\t"
		"		bl			memcpy                    \n\t"
		"		msr			MSP, r0                   \n\t"
		"		msr			PSP, r0                   \n\t"
			:
			:
			:"r0", "r1", "r2", "cc" , "memory"
		);
	}

	// initializes the current system clock frequency
	RCC_ClocksTypeDef  clocks;
	RCC_GetClocksFreq (&clocks);
	system_clock_frequency = clocks.SYSCLK_Frequency;
	APB1_clock_frequency = clocks.PCLK1_Frequency;
	APB2_clock_frequency = clocks.PCLK2_Frequency;

}

/**
 *  Low Level init for STM32
 */
extern char _sbackup, _ebackup;

extern "C" void SLOW_FLASH LowLevelInit( void )
						__attribute__ ((weak, alias ("DefaultLowLevelInit")));
extern "C" void SLOW_FLASH DefaultLowLevelInit( void )
{
	SystemInit();

	DBGMCU->DBGMCU_CR = DBGMCU_CR_TRACE_IOEN;

	// Access to the backup SRAM
	//1. Enable the power interface clock by setting the PWREN bits in the RCC APB1ENR
	RCC->RCC_APB1ENR |= RCC_APB1ENR_PWREN;
    //2. Set the DBP bit in the PWR power control register (PWR_CR)
	//   to enable access to the backup domain
	PWR_BB->PWR_CR_DBP_BB = 1;
	//3. Enable the backup SRAM clock by setting BKPSRAMEN bit in the RCC AHB1
	//   peripheral clock register (RCC_AHB1ENR)
	RCC->RCC_AHB1ENR |= RCC_AHB1ENR_BKPSRAMEN;

	// Backup regulator (used to maintain backup SRAM content in Standby and VBAT
	// modes) is enabled.
	if(! PWR_BB->PWR_CSR_BRE_BB)
	{
		PWR_BB->PWR_CSR_BRE_BB = 1;
		while(!PWR_BB->PWR_CSR_BRR_BB )
		{
		}
		memclr(&_sbackup, &_ebackup - &_sbackup);
	}

	if(exception_record.record_crc != exception_crc((const unsigned int*)&exception_record))
	{
		memclr((void *)&exception_record,sizeof(exception_record));
	}

	if(!exception_record.reset_cause)
		exception_record.reset_cause = LowLevelResetCause();
	if(!(exception_record.reset_cause & RCC_CSR_PORRSTF))
	{
		exception_crc((const unsigned int*)&exception_record);
	}
	else
	{
		exception_record.reset_cause = 0;
	}

}

/**
 * Low level peripheral reset
 * Can be replaced by app
 */
WEAK_C void LowLevelReset( void)
{
}

/**
 * Reboots the device
 * Can be replaced by app
 */
WEAK_C void LowLevelReboot( void)
{
}

/**
 * Reset cause
 * @return device specific value for the cause
 */
WEAK_C unsigned int LowLevelResetCause(void)
{
	unsigned int res;

	res = RCC->RCC_CSR;
	RCC->RCC_CSR = RCC_CSR_RMVF;
	return (res);
}

WEAK_C uint32_t rand(void)
{
	return CURRENT_TIME;
}

/**************************************************************************//**
 * @ingroup	hardware_stm32f2_pwr
 * @file	hardware/stm32/stm32f2/pwr_f2.h
 * @brief  	STM32F2 PWR
 * @date    15. February 2013
 * @author	Miroslav Kostadinov
 *
 * @defgroup hardware_stm32f2_pwr   Power control (PWR)
 * @ingroup	 hardware_stm32f2
 * Source and definitions for STM32F2  Power control (PWR)
 * @{
 *
 ******************************************************************************/

#ifndef PWR_F2_H_
#define PWR_F2_H_

#include <mcu_inc.h>
#include <tmos_types.h>

/***************************************************************************//**
 *  Power control (PWR)
 ******************************************************************************/
typedef struct
{
  __IO uint32_t PWR_CR;   		   	//!< (pwr Offset: 0x00) PWR power control register
  __IO uint32_t PWR_CSR;      		//!< (pwr Offset: 0x04) PWR power control/status register
} PWR_TypeDef;

/** bit-banding version */
typedef struct
{
  __IO uint32_t PWR_CR_LPDS_BB;  	//!< Bit 0 LPDS: Low-power deep sleep
  __IO uint32_t PWR_CR_PDDS_BB;  	//!< Bit 1 PDDS: Power down deepsleep
  __IO uint32_t PWR_CR_CWUF_BB;  	//!< Bit 2 CWUF: Clear wakeup flag
  __IO uint32_t PWR_CR_CSBF_BB;  	//!< Bit 3 CSBF: Clear standby flag
  __IO uint32_t PWR_CR_PVDE_BB;  	//!< Bit 4 PVDE: Power voltage detector enable
  __IO uint32_t PWR_CR_PLS_0_BB;  	//!< Bits 7:5 PLS[2:0]: PVD level selection
  __IO uint32_t PWR_CR_PLS_1_BB;  	//!< Bits 7:5 PLS[2:0]: PVD level selection
  __IO uint32_t PWR_CR_PLS_2_BB;  	//!< Bits 7:5 PLS[2:0]: PVD level selection
  __IO uint32_t PWR_CR_DBP_BB;  	//!< Bit 8 DBP: Disable backup domain write protection
  __IO uint32_t PWR_CR_FPDS_BB;  	//!< Bit 9 FPDS: Flash power down in Stop mode
  __IO uint32_t PWR_CR_LPUDS_BB;  	//!< Bit 10 LPUDS: Low-power regulator in deepsleep under-drive mode
  __IO uint32_t PWR_CR_MRUDS_BB;	//!< Bit 11 MRUDS: Main regulator in deepsleep under-drive mode
  __IO uint32_t reserved1;
  __IO uint32_t PWR_CR_ADCDC1_BB; 	//!< Bit 13 ADCDC1: Refer to AN4073 for details on how to use this bit.
  __IO uint32_t PWR_CR_VOS_0_BB;	//!< Bits 15:14 VOS[1:0]: Regulator voltage scaling output selection
  __IO uint32_t PWR_CR_VOS_1_BB; 	//!< Bits 15:14 VOS[1:0]: Regulator voltage scaling output selection
  __IO uint32_t PWR_CR_ODEN_BB; 	//!< Bit 16 ODEN: Over-drive enable
  __IO uint32_t PWR_CR_ODSWEN_BB; 	//!< Bit 17 ODSWEN: Over-drive switching enabled.
  __IO uint32_t PWR_CR_UDEN_0_BB; 	//!< Bits 19:18 UDEN[1:0]: Under-drive enable in stop mode
  __IO uint32_t PWR_CR_UDEN_1_BB; 	//!< Bits 19:18 UDEN[1:0]: Under-drive enable in stop mode
  __IO uint32_t reserved2[12];  	//!< Bits 31:20 reserved

  __IO uint32_t PWR_CSR_WUF_BB;  	//!< Bit 0 WUF: Wakeup flag
  __IO uint32_t PWR_CSR_SBF_BB;  	//!< Bit 1 SBF: Standby flag
  __IO uint32_t PWR_CSR_PVDO_BB;  	//!< Bit 2 PVDO: PVD output
  __IO uint32_t PWR_CSR_BRR_BB;  	//!< Bit 3 BRR: Backup regulator ready
  __IO uint32_t reserved3[4]; 	 	//!< Bits 7:4 reserved
  __IO uint32_t PWR_CSR_EWUP_BB;  	//!< Bit 8 EWUP: Enable WKUP pin
  __IO uint32_t PWR_CSR_BRE_BB;  	//!< Bit 9 BRE: Backup regulator enable
  __IO uint32_t reserved4[4]; 	 	//!< Bits 13:10 reserved
  __IO uint32_t PWR_CSR_VOSRDY_BB; 	//!< Bit 14 VOSRDY: Regulator voltage scaling output selection ready bit
  __IO uint32_t reserved5;  		//!< Bit 15 reserved
  __IO uint32_t PWR_CSR_ODRDY_BB;  	//!< Bit 16 ODRDY: Over-drive mode ready
  __IO uint32_t PWR_CSR_ODSWRDY_BB;	//!< Bit 17 ODSWRDY: Over-drive mode switching ready
  __IO uint32_t PWR_CSR_UDRDY_0_BB; //!< Bits 19:18 UDRDY[1:0]: Under-drive ready flag
  __IO uint32_t PWR_CSR_UDRDY_1_BB; //!< Bits 19:18 UDRDY[1:0]: Under-drive ready flag
} PWR_BB_TypeDef;

/***************************************************************************//**
 * @relates PWR_BB_TypeDef
 * @{
 ******************************************************************************/

/** @name PWR_CR:      	(pwr Offset: 0x00) PWR power control register		  */
/** @{ */
#define PWR_CR_UDEN         0xC0000 //!< Bits 19:18 UDEN[1:0]: Under-drive enable in stop mode
#define PWR_CR_UDEN_ENABLE  0xC0000 //!<  11:Under-drive enable
#define PWR_CR_UDEN_DISABLE 0x00000 //!<  00: Under-drive disable
#define PWR_CR_ODSWEN       0x20000 //!< Bit 17 ODSWEN: Over-drive switching enabled.
#define PWR_CR_ODEN         0x10000 //!< Bit 16 ODEN: Over-drive enable
#define PWR_CR_VOS          0x0C000 //!< Bits 15:14 VOS[1:0]: Regulator voltage scaling output selection
#define PWR_CR_VOS_3_R      0x00000 //!<  00: Reserved (Scale 3 mode selected)
#define PWR_CR_VOS_3        0x04000 //!<  01: Scale 3 mode
#define PWR_CR_VOS_2        0x08000 //!<  10: Scale 2 mode
#define PWR_CR_VOS_1        0x0C000 //!<  11: Scale 1 mode (reset value)
#define PWR_CR_ADCDC1       0x02000 //!< Bit 13 ADCDC1: Refer to AN4073 for details on how to use this bit.
#define PWR_CR_MRUDS        0x00800 //!< Bit 11 MRUDS: Main regulator in deepsleep under-drive mode
#define PWR_CR_LPUDS        0x00400 //!< Bit 10 LPUDS: Low-power regulator in deepsleep under-drive mode
#define PWR_CR_FPDS         0x00200 //!< Bit 9 FPDS: Flash power down in Stop mode
#define PWR_CR_DBP          0x00100 //!< Bit 8 DBP: Disable backup domain write protection
#define PWR_CR_PLS          0x000E0 //!< Bits 7:5 PLS[2:0]: PVD level selection
#define PWR_CR_PLS_2_0V     0x00000 //!<  000: 2.0 V
#define PWR_CR_PLS_2_1V     0x00020 //!<  001: 2.1 V
#define PWR_CR_PLS_2_3V     0x00040 //!<  010: 2.3 V
#define PWR_CR_PLS_2_5V     0x00060 //!<  011: 2.5 V
#define PWR_CR_PLS_2_6V     0x00080 //!<  100: 2.6 V
#define PWR_CR_PLS_2_7V     0x000A0 //!<  101: 2.7 V
#define PWR_CR_PLS_2_8V     0x000C0 //!<  110: 2.8 V
#define PWR_CR_PLS_2_9V     0x000E0 //!<  111: 2.9 V
#define PWR_CR_PVDE         0x00010 //!< Bit 4 PVDE: Power voltage detector enable
#define PWR_CR_CSBF         0x00008 //!< Bit 3 CSBF: Clear standby flag
#define PWR_CR_CWUF         0x00004 //!< Bit 2 CWUF: Clear wakeup flag
#define PWR_CR_PDDS         0x00002 //!< Bit 1 PDDS: Power down deepsleep
#define PWR_CR_LPDS         0x00001 //!< Bit 0 LPDS: Low-power deep sleep
/** @} */

/** @name PWR_CSR:      (pwr Offset: 0x04) PWR power control/status register  */
/** @{ */
#define PWR_CSR_UDRDY       0xC0000 //!< Bits 19:18 UDRDY[1:0]: Under-drive ready flag
                                    //!<  00: Under-drive is disabled
                                    //!<  11: Under-drive mode is activated in Stop mode.
#define PWR_CSR_ODSWRDY     0x20000 //!< Bit 17 ODSWRDY: Over-drive mode switching ready
#define PWR_CSR_ODRDY       0x10000 //!< Bit 16 ODRDY: Over-drive mode ready
#define PWR_CSR_VOSRDY      0x04000 //!< Bit 14 VOSRDY: Regulator voltage scaling output selection ready bit
#define PWR_CSR_BRE         0x00200 //!< Bit 9 BRE: Backup regulator enable
#define PWR_CSR_EWUP        0x00100 //!< Bit 8 EWUP: Enable WKUP pin
#define PWR_CSR_BRR         0x00008 //!< Bit 3 BRR: Backup regulator ready
#define PWR_CSR_PVDO        0x00004 //!< Bit 2 PVDO: PVD output
#define PWR_CSR_SBF         0x00002 //!< Bit 1 SBF: Standby flag
#define PWR_CSR_WUF         0x00001 //!< Bit 0 WUF: Wakeup flag
/** @} */



/** @} */ // @relates PWR_BB_TypeDef



#endif /* PWR_F2_H_ */

/** @} */

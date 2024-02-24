/*
 * mcu_id.h
 *
 *  Created on: Jan 23, 2024
 *      Author: bratkov
 */

#ifndef PORTS_CORTEX_M3_MCU_ID_H_
#define PORTS_CORTEX_M3_MCU_ID_H_
#include <tmos_types.h>

#ifdef __cplusplus

enum cpu_arch_t:uint8_t
{
	Arch_Unknown,
	ArmV6,
	ArmV7
};

enum cpu_type_t:uint8_t
{
	CPU_Unknown,
	Cortex_M0,
	Cortex_M0P,
	Cortex_M1,
	Cortex_M3,
	Cortex_M4,
	Cortex_M4F,
	Cortex_M7
};

typedef struct
{
	union{
		struct{
		cpu_type_t 	type;
		cpu_arch_t	arch;
		uint8_t		variant;
		uint8_t		revision;
		} __attribute__((packed));
		uint32_t id;
	};
}cpu_t;

cpu_t cpu_identify(void);

#endif

#define MCU_JEP106_GD		0x751	// GigaDevice Semiconductor (Beijing)
#define MCU_JEP106_STM		0x020	// STMicroelectronics
#define MCU_JEP106_APM		0xB23	// Apex Microelectronics Co Ltd

extern __no_init char device_type[];

uint32_t mcu_jep106(void);
uint32_t mcu_part(void);
uint32_t mcu_revision(void);
const char* mcu_manufacturer(void);
const char* get_mcu_type(void);

#endif /* PORTS_CORTEX_M3_MCU_ID_H_ */

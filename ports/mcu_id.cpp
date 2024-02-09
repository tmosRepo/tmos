/*
 * mcu_id.cpp
 *
 *  Created on: Jan 23, 2024
 *      Author: bratkov
 */

#include <tmos.h>
#include <mcu_id.h>
#include <core_map.h>

#ifdef BASE_ROM_TABLE
uint32_t mcu_jep106(void)
{
	unsigned int jep=0;
	if(ROM_PID2 & 8)
	{
		jep |= (ROM_PID1 & 0xF0)>>4;
		jep |= (ROM_PID2 & 0x07)<<4;
		jep |= (ROM_PID4 & 0x0F)<<8;
	}
	return jep;
}

uint32_t mcu_part(void)
{
	uint32_t part;
	part = ROM_PID0; 				// ROM_PID0 			Part Number [7:0]
	part |= (ROM_PID1 & 0x0F) << 8; // ROM_PID1 Bits[3:0] 	Part Number [11:8]
	part |= (ROM_CID1) << 8; 		// ROM_CID1 Bits[7:4] 	Component Class
	return part;
}

uint32_t mcu_revision(void)
{
	uint32_t rev;
	rev = (ROM_PID2 & 0xF0) << 4;
	rev |= ROM_PID3;
	return rev;
}
#else
uint32_t mcu_jep106(void)
{
	return 0;
}

uint32_t mcu_part(void)
{
	return 0;
}

uint32_t mcu_revision(void)
{
	return 0;
}
#endif

cpu_t cpu_identify(void)
{
	cpu_t cpu;
    uint32_t CPUID= *((uint32_t*)0xE000ED00);
	switch ((CPUID>>16)&0xf) // ARCHITECTURE
	{
	case 0xC:
		cpu.arch = ArmV6;
		break;
	case 0xF:
		cpu.arch = ArmV7;
		break;
	default:
		cpu.arch = Arch_Unknown;
	}

	switch((CPUID>>4)&0xfff) // PARTNO
    {
    case 0xc20:
    	cpu.type = Cortex_M0;
    	break;
    case 0xc60:
    	cpu.type = Cortex_M0P;
    	break;
    case 0xc21:
    	cpu.type = Cortex_M1;
    	break;
    case 0xc23:
    	cpu.type = Cortex_M3;
    	break;
    case 0xc24:
    {
    	cpu.type = Cortex_M4;
        // cortex M4, do we have FPU ?
        uint32_t fp=*(uint32_t *)0xE000EF40; //Media and FP Feature Register 0, MVFR0
        if(((fp>>16)&0xff)==0x11) // FP divide & Square root
        {
            // Yes, activate FPU
        	cpu.type = Cortex_M4F;
        }

    }
    	break;
    case 0xc27:
    	cpu.type = Cortex_M7;
    	break;
	default:
		cpu.type = CPU_Unknown;
    }
	cpu.variant = (CPUID>>20)&0xf;
	cpu.revision = CPUID&0xf;
	return cpu;
}




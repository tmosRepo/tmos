/*
 * mcu_id.cpp
 *
 *  Created on: Jan 23, 2024
 *      Author: bratkov
 */
#include <tmos.h>
#include <hardware_cpp.h>
#include <mcu_id.h>

__no_init char device_type[16];
/* CoreSight infrastructure IDs for an ARMv7-M implementation */
/*
 * CIDx values are fully defined for the ROM table, and are CoreSight compliant.
 * PIDx values should be CoreSight compliant or RAZ.
 * See Appendix D1 ARMv7-M CoreSight Infrastructure IDs for more information.
 *
 * 0xFD0 IMP DEF 	PID4
 * 0xFD4 0			PID5
 * 0xFD8 0			PID6
 * 0xFDC 0			PID7
 * 0xFE0 IMP DEF	PID0
 * 0xFE4 IMP DEF	PID1
 * 0xFE8 IMP DEF	PID2
 * 0xFEC IMP DEF	PID3
 * 0xFF0 0x0000000D	CID0
 * 0xFF4 0x00000010	CID1
 * 0xFF8 0x00000005 CID2
 * 0xFFC 0x000000B1 CID3
 */
typedef struct
{
  __I uint32_t PID4;	//!< (Offset: 0xFD0) PID4
  __I uint32_t PID5;  	//!< (Offset: 0xFD4) PID5
  __I uint32_t PID6;   	//!< (Offset: 0xFD8) PID6
  __I uint32_t PID7;  	//!< (Offset: 0xFDC) PID7
  __I uint32_t PID0;  	//!< (Offset: 0xFE0) PID0
  __I uint32_t PID1;  	//!< (Offset: 0xFE4) PID1
  __I uint32_t PID2;  	//!< (Offset: 0xFE8) PID2
  __I uint32_t PID3;  	//!< (Offset: 0xFEC) PID3
  __I uint32_t CID0;  	//!< (Offset: 0xFF0) CID0
  __I uint32_t CID1;  	//!< (Offset: 0xFF4) CID1
  __I uint32_t CID2;  	//!< (Offset: 0xFF8) CID2
  __I uint32_t CID3;  	//!< (Offset: 0xFFC) CID3
} CoreSight_ID;

// CoreSight infrastructure IDs
#define Cortex_M134_ID  CAST(CoreSight_ID	, 0xE00FFFD0) //!< BASE_ROM_TABLE
#define Cortex_M7_ID  CAST(CoreSight_ID	, 0xE0041FD0) //!< ETM Base Address

static CoreSight_ID* Get_ID(void)
{
	switch((REG_SCB_CPUID>>4)&0xfff) // PARTNO
    {
    case 0xc21: // M1
    case 0xc23: // M3
    case 0xc24: // M4
    	return Cortex_M134_ID;
    	break;
     case 0xc27:
     	return Cortex_M7_ID;
    	break;
	default:
		return NULL;
    }
}


uint32_t mcu_jep106(void)
{

	CoreSight_ID* ID = Get_ID();
	unsigned int jep=0;
	if(ID)
	{
		if(ID->PID2 & 8)
		{
			jep |= (ID->PID1 & 0xF0)>>4;
			jep |= (ID->PID2 & 0x07)<<4;
			jep |= (ID->PID4 & 0x0F)<<8;
		}
	}
	return jep;
}

uint32_t mcu_part(void)
{
	uint32_t part =0;
	CoreSight_ID* ID = Get_ID();
	if(ID)
	{
		part = ID->PID0; 				// PID0 			Part Number [7:0]
		part |= (ID->PID1 & 0x0F) << 8; // PID1 Bits[3:0] 	Part Number [11:8]
		part |= (ID->CID1) << 8; 		// CID1 Bits[7:4] 	Component Class
	}
	return part;
}

uint32_t mcu_revision(void)
{
	uint32_t rev = 0;
	CoreSight_ID* ID = Get_ID();
	if(ID)
	{
		rev = (ID->PID2 & 0xF0) << 4;
		rev |= ID->PID3;
	}
	return rev;
}

const char* mcu_manufacturer(void)
{
	const char* ptr = "Unknown";
	switch(mcu_jep106())
	{
	case MCU_JEP106_APM:
		ptr = "Apex Microelectronics Co Ltd";
		break;
	case MCU_JEP106_GD:
		ptr = "GigaDevice Semiconductor (Beijing)";
		break;
 	case MCU_JEP106_STM:
 		ptr = "STMicroelectronics";
		break;
	}
	return ptr;
}

static bool is_crypto(void)
{
	bool res = false;
#ifdef	ID_PERIPH_CRYP
	RCCPeripheralEnable(ID_PERIPH_CRYP);
	RCCPeripheralReset(ID_PERIPH_CRYP);
#ifdef BASE_CRYP
	if(*((uint32_t*)(BASE_CRYP+4)) == 3) // CRYP status register (CRYP_SR)
	{
		res = true;
	}
#endif //BASE_CRYP
	RCCPeripheralDisable(ID_PERIPH_CRYP);
#endif //ID_PERIPH_CRYP
	return res;
}

static bool is_ethernet(void)
{
	bool res = false;
#ifdef ID_PERIPH_ETHMAC
	RCCPeripheralEnable(ID_PERIPH_ETHMAC);
	RCCPeripheralReset(ID_PERIPH_ETHMAC);
#ifdef ETH_MAC1
	if(ETH_MAC1->ETH_MACCR == 0x8000)
		res = true;
#endif //ETH_MAC1
	RCCPeripheralLPDisable(ID_PERIPH_ETHMAC);
#endif //ID_PERIPH_ETHMAC
	return res;
}

static bool is_lcd(void)
{
	bool res = false;
#ifdef ID_PERIPH_LCD
	RCCPeripheralEnable(ID_PERIPH_LCD);
	RCCPeripheralReset(ID_PERIPH_LCD);
#ifdef BASE_LCD
	if(*((uint32_t*)(BASE_LCD+0x18)) == 0x2220)
		res = true;
#endif //BASE_LCD
	RCCPeripheralLPDisable(ID_PERIPH_LCD);
#endif //ID_PERIPH_LCD
	return res;
}

static char* check_priphral_411_413(char* ptr)
{
	*ptr = (is_crypto()?'1':'0');
	ptr++;
	*ptr = (is_ethernet()?'7':'5');
	ptr++;
	*ptr = 0;
	return ptr;
}

/*
 * To be called only before the initialization of the drivers !
 */
const char* get_mcu_type(void)
{
	char *ptr = device_type;
	uint32_t jep = mcu_jep106();

	memset(device_type, 0, sizeof(device_type));
	switch(jep)
	{
	case MCU_JEP106_APM:
		strcpy(ptr,"APM32");
		break;
	case MCU_JEP106_GD:
		strcpy(ptr,"GD32");
		break;
 	case MCU_JEP106_STM:
		strcpy(ptr,"ST32");
		break;
 	default: // TODO: Other manufacturers should be added here
 		strcpy(ptr, "Unknown");
 		return device_type;
	}
	ptr += strlen(ptr);

	switch(DBGMCU->DBGMCU_IDCODE & 0xFFF)
	{
	case 0x411:
		strcpy(ptr, "F2");
		ptr = check_priphral_411_413(ptr);
		break;
	case 0x413:
		strcpy(ptr, "F4");
		ptr +=2;
		if(jep == MCU_JEP106_GD)
			strcpy(ptr, "27");
		else
			ptr = check_priphral_411_413(ptr);
		break;
	case 0x419:
		strcpy(ptr, "F4");
		ptr +=2;
		if(jep == MCU_JEP106_GD)
			strcpy(ptr, "70");
		else if(jep == MCU_JEP106_STM)
		{
			//(STM32F42xxx and STM32F43xxx)
			//STM32F437xx STM32F439xx
			//437= STM32F437xx, USB OTG FS/HS, camera interface, Ethernet, cryptographic acceleration
			//439= STM32F439xx, USB OTG FS/HS, camera interface, Ethernet, cryptographic acceleration, LCD-TFT
			//STM32F427xx STM32F429xx
			//427= STM32F427xx, USB OTG FS/HS, camera interface, Ethernet
			//429= STM32F429xx, USB OTG FS/HS, camera interface, Ethernet, LCD-TFT
			*ptr = (is_crypto()?'3':'2');
			ptr++;
			*ptr = (is_lcd()?'9':'7');
			ptr++;
			*ptr =0;
		}else
			strcpy(ptr, "XX");
		break;
	default:
		strcpy(ptr, "????"); // TODO: to add other ID codes
		return device_type;
	}
	ptr += strlen(ptr);
	if(jep == MCU_JEP106_GD)
	{
		switch(*((uint32_t*)0x40023D00)) // FMC_ID
		{
		case 0x4E4B5A30:
			strcpy(ptr, "ZK");
			break;
		case 0x4E475A30:
		case 0x4E475A37:
			strcpy(ptr, "ZG");
			break;
		}
	}
	return device_type;
}
cpu_t cpu_identify(void)
{
	cpu_t cpu;
	switch ((REG_SCB_CPUID>>16)&0xf) // ARCHITECTURE
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

	switch((REG_SCB_CPUID>>4)&0xfff) // PARTNO
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
	cpu.variant = (REG_SCB_CPUID>>20)&0xf;
	cpu.revision = REG_SCB_CPUID&0xf;
	return cpu;
}




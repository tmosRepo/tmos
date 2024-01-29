/*
 * mcu_id.cpp
 *
 *  Created on: Jan 23, 2024
 *      Author: bratkov
 */

#include <tmos_types.h>
#include <core_map.h>

uint32_t mcu_jep106()
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

uint32_t mcu_part()
{
	uint32_t part;
	part = ROM_PID0; 				// ROM_PID0 			Part Number [7:0]
	part |= (ROM_PID1 & 0x0F) << 8; // ROM_PID1 Bits[3:0] 	Part Number [11:8]
	part |= (ROM_CID1) << 8; 		// ROM_CID1 Bits[7:4] 	Component Class
	return part;
}

uint32_t mcu_revision()
{
	uint32_t rev;
	rev = (ROM_PID2 & 0xF0) << 4;
	rev |= ROM_PID3;
	return rev;
}

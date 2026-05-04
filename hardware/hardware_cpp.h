/**************************************************************************//**
 * @ingroup hardware
 * @{
 * @file     hardware_cpp.h
 * @brief    Hardware headers
 * @version  V3.00
 * @date     21. November 2010
 * @author	 Miroslav Kostadinov
 *
 *
 *
 ******************************************************************************/
#ifndef HARDWARE_CPP_H_
#define HARDWARE_CPP_H_

#include <fam_cpp.h>

#ifdef __CORTEX_M
# if       (__CORTEX_M >= 0x03)
#  ifndef USE_UNALIGNED_ACCESS
    // default  access for Cortex M3 and above
#   define USE_UNALIGNED_ACCESS 	1
#  endif
# else
#  ifndef USE_UNALIGNED_ACCESS
#   define USE_UNALIGNED_ACCESS 	0
#  else
#   error Cortex M0 do not support this
#  endif
# endif
#endif // __CORTEX_M

#endif /* HARDWARE_CPP_H_ */
/** @} ingroup hardware */

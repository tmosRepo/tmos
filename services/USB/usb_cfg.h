/*
 * usb_cfg.h
 *
 *  Created on: Oct 13, 2014
 *      Author: miro
 */

#ifndef USB_CFG_H_
#define USB_CFG_H_

#include <brd_cfg.h>

//----------------------------------------------------------------------------
//						USB debug trace
//----------------------------------------------------------------------------
#ifndef TRACE_USB_LEVEL
#define TRACE_USB_LEVEL			TRACE_LEVEL_NONE
#endif

#define TRACE_USB(...) 			TRACE_LEVEL(TRACE_USB_LEVEL, __VA_ARGS__)
#define TRACE1_USB(str)			TRACE1_LEVEL(TRACE_USB_LEVEL, str)

#if TRACE_USB_LEVEL >= TRACE_LEVEL_DEBUG
#	define TRACELN_USB(...)			TRACELN(__VA_ARGS__)
#	define TRACELN1_USB(...)		TRACELN1(__VA_ARGS__)
#else
#	define TRACELN_USB(str,...)
#	define TRACELN1_USB(str)
#endif

#if TRACE_USB_LEVEL >= TRACE_LEVEL_WARNING
#	define TRACE_USB_WARNING(...)		TRACE_WARNING(__VA_ARGS__)
#	define TRACE1_USB_WARNING(str)	TRACE1_WARNING(str)
#else
#	define TRACE_USB_WARNING(...)
#	define TRACE1_USB_WARNING(str)
#endif

#if TRACE_USB_LEVEL >= TRACE_LEVEL_ERROR
#	define TRACE_USB_ERROR(...)		TRACE_WARNING(__VA_ARGS__)
#	define TRACE1_USB_ERROR(str)	TRACE1_WARNING(str)
#else
#	define TRACE_USB_ERROR(...)
#	define TRACE1_USB_ERROR(str)
#endif


//------------------------------------------------------------------------------
//         				OTG settings
//------------------------------------------------------------------------------

/// System has a high speed USB OTG controller.
#ifndef USB_ENABLE_OTG
#define USB_ENABLE_OTG 0
#endif

//------------------------------------------------------------------------------
//         				HOST settings
//------------------------------------------------------------------------------
/// USB Host mode disabled by default
#ifndef USB_ENABLE_HOST
#define USB_ENABLE_HOST 0
#endif

#ifndef MAX_HUB_PORTS
#define MAX_HUB_PORTS	2
#endif

/// reads and stores the manufacturer, product, and serial number of a remote device as strings
#ifndef USB_REMOTE_DEV_STRINGS
#define USB_REMOTE_DEV_STRINGS	0
#endif
//------------------------------------------------------------------------------
//         				Device settings
//------------------------------------------------------------------------------
/// USB Device mode enabled by default
#ifndef USB_ENABLE_DEVICE
#define USB_ENABLE_DEVICE 1
#endif

/// Number of max interfaces used
#ifndef USB_MAX_INTERACES
#define	USB_MAX_INTERACES 2
#endif


//------------------------------------------------------------------------------
//         				USB Class settings
//------------------------------------------------------------------------------
#ifndef USB_ENABLE_CDC
#define USB_ENABLE_CDC 0
#endif

#ifndef USB_ENABLE_PRN
#define USB_ENABLE_PRN 0
#endif

#ifndef USB_ENABLE_HID
#define USB_ENABLE_HID 0
#endif

#ifndef USB_ENABLE_MSC
#define USB_ENABLE_MSC 0
#endif
//------------------------------------------------------------------------------
//         				Sanity checks
//------------------------------------------------------------------------------
#if !USB_ENABLE_DEVICE && !USB_ENABLE_HOST
#error "USB enable but no host or device!"
#endif

#endif /* USB_CFG_H_ */

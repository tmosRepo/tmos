/*
 * usbh_msc.h
 *
 *  Created on: Aug 8, 2016
 *      Author: miro
 */

#ifndef SERVICES_USB_HOST_USBH_MSC_H_
#define SERVICES_USB_HOST_USBH_MSC_H_

#include <tmos.h>
#include <usb_host.h>
#include <usb_msc_dtors.h>

#define USBMSC_WRITE_TOUT	5000
#define USBMSC_READ_TOUT	2500

/// when set, causes the data to be read packet by packet
#ifndef USBMSC_READING_BY_PACKETS
#define USBMSC_READING_BY_PACKETS 0
#endif

/// when set, causes the data to be write packet by packet
#ifndef USBMSC_WRITING_BY_PACKETS
#define USBMSC_WRITING_BY_PACKETS 0
#endif

struct usb_remote_msc_t: public usb_remote_dev_t
{
	HANDLE 	msc_hnd;
	USBInterfaceDescriptor* pid;

	uint32_t cb_tag;
	uint32_t lun;
	uint32_t blk_size;
	uint32_t capacity;
	uint8_t	msc_max_lun;

	usb_remote_msc_t(HANDLE h1, HANDLE h2): usb_remote_dev_t(h1),
			msc_hnd(h2), pid(NULL), cb_tag(1), lun(0), blk_size(0), capacity(0), msc_max_lun(0) {};


	RES_CODE get_max_lun();

	RES_CODE msc_command(usbmsc_cs_t* transaction, void* buf, uint32_t len);
	RES_CODE msc_command_with_retry(usbmsc_cs_t* transaction, void* buf, uint32_t len);
	RES_CODE cmd_test_unit_ready(usbmsc_cs_t* transaction);
	RES_CODE cmd_read_capcity(usbmsc_cs_t* transaction);
	RES_CODE cmd_inquiry(usbmsc_cs_t* transaction);
	RES_CODE init_msd();
	RES_CODE msc_reset();
	RES_CODE msc_request_sense();
	RES_CODE msc_read(void* buf, uint32_t sector, uint32_t count);
	RES_CODE msc_write(const void* buf, uint32_t sector, uint32_t count);
	RES_CODE scan_msc(uint32_t port_indx, USBSubClassCode subcls, USBProtocolCode proto);
};

//----------------------------------------------------------------------------
//						USB MSC debug trace
//----------------------------------------------------------------------------
#ifndef TRACE_MSC_LEVEL
#define TRACE_MSC_LEVEL			TRACE_LEVEL_NONE
#endif

#define TRACE_MSC(...) 			TRACE_LEVEL(TRACE_MSC_LEVEL, __VA_ARGS__)
#define TRACE1_MSC(str)			TRACE1_LEVEL(TRACE_MSC_LEVEL, str)

#if TRACE_MSC_LEVEL >= TRACE_LEVEL_DEBUG
#	define TRACELN_MSC(str, ...)	TRACELN("MSC:" str, ##__VA_ARGS__)
#	define TRACELN1_MSC(str)		TRACELN1("MSC:" str)
#else
#	define TRACELN_MSC(str,...)
#	define TRACELN1_MSC(str)
#endif

#if TRACE_MSC_LEVEL >= TRACE_LEVEL_WARNING
#	define TRACE_MSC_WARNING(str, ...)	TRACE_WARNING("MSC:" str, ##__VA_ARGS__)
#	define TRACE1_MSC_WARNING(str)	TRACE1_WARNING("MSC:" str)
#else
#	define TRACE_MSC_WARNING(...)
#	define TRACE1_MSC_WARNING(str)
#endif

#if TRACE_MSC_LEVEL >= TRACE_LEVEL_ERROR
#	define TRACE_MSC_ERROR(str, ...)		TRACELN_ERROR("MSC:" str, ##__VA_ARGS__)
#	define TRACE1_MSC_ERROR(str)			TRACE1_ERROR("MSC:" str)
#else
#	define TRACE_MSC_ERROR(...)
#	define TRACE1_MSC_ERROR(str)
#endif

#endif /* SERVICES_USB_HOST_USBH_MSC_H_ */

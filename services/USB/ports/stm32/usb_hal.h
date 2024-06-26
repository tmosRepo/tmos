/*
 * usb_hal.h
 *
 *  Created on: Dec 3, 2012
 *      Author: miro
 */

#ifndef USB_HAL_H_
#define USB_HAL_H_

#include <tmos.h>
#include <usb_opt.h>
#include <usb_drv.h>

#ifndef FAST_FLASH
#define FAST_FLASH
#endif

#define ENTPOINT_FIFO(hw_base, eptnum) &hw_base->DFIFO[eptnum].DFIFO[0]

/**
 * Indicates if the device is running in high or full-speed. Always returns 0
 * for now since high-speed mode requires external PHY...
 */
#define usb_hal_IsHighSpeed() 0

extern "C" void usb_drv_event(USB_DRV_INFO drv_info, USB_EVENT event);
void usb_drv_reset(USB_DRV_INFO drv_info) FAST_FLASH;
void usb_drv_cancel_hnd(USB_DRV_INFO drv_info, HANDLE hnd) FAST_FLASH;
void usb_drv_start_tx(USB_DRV_INFO drv_info, HANDLE hnd) FAST_FLASH;
void usb_drv_start_rx(USB_DRV_INFO drv_info, HANDLE hnd) FAST_FLASH;
void usb_drv_end_transfers(ep_dir_state_t* epdir, unsigned int status) FAST_FLASH;

bool usb_hal_get_ep_status(USB_DRV_INFO drv_info, uint8_t ept_num, uint16_t* data) FAST_FLASH;
void usb_hal_stall(USB_TypeDef* hw_base, unsigned int ept_num) FAST_FLASH;
void usb_hal_stall_clear(USB_TypeDef* hw_base, unsigned int ept_num) FAST_FLASH;
void usb_hal_device_start(USB_DRV_INFO drv_info) FAST_FLASH;
void usb_hal_ept_reset(USB_DRV_INFO drv_info, unsigned int ept_num) FAST_FLASH;
void usb_hal_ept_config(USB_DRV_INFO drv_info, const USBGenericDescriptor* pDescriptor) FAST_FLASH;
void usb_hal_config_fifo(USB_DRV_INFO drv_info) FAST_FLASH;
#if USB_ENABLE_HOST
EXTERN_C void usb_hal_host_power(USB_DRV_INFO drv_info, bool enable);
RES_CODE usb_hal_host_start(USB_DRV_INFO drv_info) FAST_FLASH;
RES_CODE usb_hal_host_bus_reset(USB_DRV_INFO drv_info) FAST_FLASH;
void usb_hal_host_resume(USB_DRV_INFO drv_info) FAST_FLASH;
void usb_hal_host_ept_cfg(USB_DRV_INFO drv_info, const USBEndpointDescriptor* pDescriptor) FAST_FLASH;
void usb_hal_host_nak_tout(USB_DRV_INFO drv_info) FAST_FLASH;
#endif
RES_CODE usb_hal_start(USB_DRV_INFO drv_info, uint32_t mode) FAST_FLASH;

void USB_HS_WKUP_ISR(USB_DRV_INFO drv_info) FAST_FLASH;
void USB_OTG_ISR(USB_DRV_INFO drv_info) FAST_FLASH;
#if USB_STM32_DEDICATED_EP1
void USB_EP1_IN_ISR(USB_DRV_INFO drv_info);
void USB_EP1_OUT_ISR(USB_DRV_INFO drv_info);
#endif

#if USB_ENABLE_OTG
void usb_otg_set_flags(USB_DRV_INFO drv_info, uint32_t flags) FAST_FLASH;
void usb_otg_clr_flags(USB_DRV_INFO drv_info, uint32_t flags) FAST_FLASH;
#endif


#endif /* USB_HAL_H_ */

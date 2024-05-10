/*
 * usb_opt.h
 *
 *  Created on: Dec 3, 2012
 *      Author: miro
 */

#ifndef USB_OPT_H_
#define USB_OPT_H_

#include <fam_cpp.h>
#include <otg_hs_stm32.h>
#include <usb_cfg.h>

#define CFG_STM32_OTG_ULPI			0x0001		//!< USE ULPI PHY
#define CFG_STM32_OTG_SOF_OUTPUT	0x0002		//!< SOF output enable
#define CFG_STM32_OTG_INTERNAL_VBUS 0x0004		//!< ULPI External VBUS Drive
#define CFG_STM32_OTG_VBUS_SENS		0x0008		//!< Enable the VBUS sensing
#define CFG_STM32_OTG_MODE			0x0010		//!< OTG Mode
#define CFG_STM32_OTG_HS_CORE		0x0020		//!< OTG HS peripheral
#define CFG_STM32_OTG_LOW_POWER		0x0040		//!< low power
#define CFG_STM32_OTG_FORCE_DEVICE	0x0080		//!< Forced device mode
#define CFG_STM32_OTG_FORCE_HOST	0x0100		//!< Forced host mode

/// Number of endpoints used
#ifndef USB_NUMENDPOINTS
#define	USB_NUMENDPOINTS CHIP_USB_NUMENDPOINTS
#endif

/// Dedicated EP1 interrupts (not recommended)
#ifndef USB_STM32_DEDICATED_EP1
#define USB_STM32_DEDICATED_EP1 0
#endif

#define EPD_FLAG_DATA1		1

#define HALT_FRAMES_TOUT	2500

///  rx/tx data trace
#ifndef USB_DATA_TRACE
#define USB_DATA_TRACE		0
#endif

/// after each transaction the channel is halted
#ifndef USB_HOST_KEEPS_CHANNELS_DOWN
#define USB_HOST_KEEPS_CHANNELS_DOWN	1
#endif

#ifndef USB_HOST_DRIVER_RETRY
#define USB_HOST_DRIVER_RETRY			0
#endif

/// delay (in ms) before processing tout (nak).
/// it doesn't matter if USB_HOST_DRIVER_RETRY is not set
/// 0 means immediate (no delay) tout (nak) processing.
#ifndef USB_HOST_DRIVER_RETRY_DELAY
#define USB_HOST_DRIVER_RETRY_DELAY		0
#endif

/// err_cnt is incremented every TRERR and if there are 3 errors in a row, stop sending data
/// if not used, no matter how many errors are accumulated until a timeout occurs
#ifndef USB_HOST_USE_TRERR_CNT
#define USB_HOST_USE_TRERR_CNT		 0
#endif

#if USB_HOST_USE_TRERR_CNT
	#ifdef MAX_TRERR_CNT
		#if MAX_TRERR_CNT >= 15 || MAX_TRERR_CNT == 0
		#undef MAX_TRERR_CNT
		#error Error: MAX_TRERR_CNT must be 1 to 14
		#endif
	#endif

	#ifndef MAX_TRERR_CNT
	#define MAX_TRERR_CNT			 10
	#endif
#define reset_error_cnt() (epdir->epd_error_cnt = 0)
#define inc_error_cnt()	(epdir->epd_error_cnt++)
#else
#define reset_error_cnt()
#define inc_error_cnt()
#endif // USB_HOST_USE_TRERR_CNT

#if USB_HOST_KEEPS_CHANNELS_DOWN
enum halt_reason : uint8_t
{
    EPT_HALT_IDLE,
	EPT_HALT_BUSY,
    EPT_HALT_XF,
    EPT_HALT_HALTED,
    EPT_HALT_NAK,
    EPT_HALT_NYET,
    EPT_HALT_STALL,
    EPT_HALT_TRERR,
    EPT_HALT_BBERR,
    EPT_HALT_FRMOR,
    EPT_HALT_DTGERR,
};
#endif // USB_HOST_KEEPS_CHANNELS_DOWN

struct usb_config_t
{
	uint32_t	stm32_otg;						//!< stm32 otg mode settings
	uint16_t	rx_fifo_sz;
	uint16_t	tx_fifo_sz[USB_NUMENDPOINTS];
	const PIN_DESC*	usb_pins;
};

struct ep_dir_state_t		// directional endpoint state
{
	uint8_t		epd_state;	  	//!< Endpoint state
	uint8_t		epd_type;		//!< Endpoint type
	uint8_t		epd_flags;		//!< PID DATA0/1 toggle
#if USB_HOST_KEEPS_CHANNELS_DOWN
#if USB_HOST_USE_TRERR_CNT
	struct{
		halt_reason	epd_halt_reason :4;	//!< reason for disabling the channel
		uint8_t		epd_error_cnt	:4;	//!< TRERR, NAK counter
	};
#else
	halt_reason	epd_halt_reason;
#endif // USB_HOST_USE_TRERR_CNT
#endif // USB_HOST_KEEPS_CHANNELS_DOWN
	uint16_t	epd_fifo_sz;	//!< Enpoint fifo size
	uint16_t	epd_halt_tout;	//!< Endpoint disable timeout (in frames)
//--------------------------
	HANDLE		epd_pending;	//!< Enpoint pending handles
};

struct Endpoint
{
	ep_dir_state_t epd_in;		//!< Endpoint IN direction state (tx)
	ep_dir_state_t epd_out;		//!< Endpoint OUT direction state (rx)
	unsigned short rxfifo_cnt;	//!< Bytes available in the rx Fifo
	unsigned short top_rx_cnt; 	//!< Bytes available in the top_rx_word
	unsigned int   top_rx_word;	//!< top FIFO word
#if USB_HOST_KEEPS_CHANNELS_DOWN
	uint8_t*	rxfifo_buff;	//!< holds the unexpected data
#endif // USB_HOST_KEEPS_CHANNELS_DOWN
};

#define CLEAR_EPT_IN_FIFOSZ(x)	x.epd_in.epd_fifo_sz = 0
#define CLEAR_EPT_OUT_FIFOSZ(x)	x.epd_out.epd_fifo_sz = 0

/**
 * USB Controller Registers Structure
 */
typedef USB_TypeDef USB_CONTROLLER;

#define USB_SETUP_READ_TOUT		500
#define USB_SETUP_WRITE_TOUT	200

#define HND_ERROR_GET_LB(hnd) 		(hnd->error & 0xFF)
#define HND_ERROR_SET_LB(hnd, val) 	(hnd->error = ((hnd->error & 0xFF00)|((val) & 0xFF)))
#define HND_ERROR_GET_MB(hnd) 		((hnd->error & 0xFF00)>>8)
#define HND_ERROR_SET_MB(hnd, val) 	(hnd->error = ((hnd->error & 0x00FF)|(((val)<<8) & 0xFF00)))

#endif /* USB_OPT_H_ */

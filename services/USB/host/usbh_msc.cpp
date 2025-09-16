/*
 * usbh_msc.cpp
 *
 *  Created on: Aug 8, 2016
 *      Author: miro
 */

#include <tmos.h>
#include <usbh_msc.h>
#include <usb_descriptors.h>
#include <usb_svc.h>
#include <scsi.h>
#include <usb_msc_reqs.h>

#if TRACE_MSC_LEVEL >= TRACE_LEVEL_DEBUG
FILE_OPTIMIZE("O0")
#endif

//------------------------------------------------------------------------------
//	Class Requests
//------------------------------------------------------------------------------
RES_CODE usb_remote_msc_t::get_max_lun()
{
	RES_CODE res;

	msc_max_lun = 0;
	req.bmRequestType = USB_REQ_IN_CLASS_INTERFACE;
	req.bRequest = MSCRequest_GET_MAX_LUN;
	req.wIndex = pid->bInterfaceNumber;				// Interface number
	req.wValue = 0;									//
	req.wLength = 1;
	res = std_request(&msc_max_lun);
	return res;
}


//------------------------------------------------------------------------------
//	Commands
//------------------------------------------------------------------------------

#if USBMSC_WRITING_BY_PACKETS || USBMSC_READING_BY_PACKETS
#ifndef USBMSC_DATA_SLEEP
#define USBMSC_DATA_SLEEP	(100)
#endif
#ifndef USBMSC_DATA_RETRY
#define USBMSC_DATA_RETRY	500
#endif
#endif

RES_CODE usb_remote_msc_t::msc_command(usbmsc_cs_t* transaction, void* buf, uint32_t len)
{
	RES_CODE res, res1;
	scsi_cmd_read_10_t* cdb = (scsi_cmd_read_10_t*)transaction->cbw.CBWCB;

	// send command
	transaction->cbw.dCBWSignature = USBMSC_CBW_SIGNATURE;
	transaction->cbw.dCBWTag = ++cb_tag;
	transaction->cbw.bCBWLUN = lun;
	transaction->cbw.dCBWDataTransferLength = len;

	TRACELN1_MSC("Cmd");
	if(cdb->opcode == SCSI_CMD_WRITE_10 || cdb->opcode == SCSI_CMD_READ_10)
	{
		TRACE_MSC(" Req %s sec:%u", ((cdb->opcode == SCSI_CMD_WRITE_10)?"Wr":"Rd"), __REV(cdb->lba));
	}else if(cdb->opcode == SCSI_CMD_REQUEST_SENSE)
	{
		TRACE_MSC(" Req Sense");
	}
	TRACE_MSC(" Tag:%X", cb_tag);

	res = msc_hnd->tsk_write(&transaction->cbw, USBMSC_CBW_SIZEOF, USBMSC_WRITE_TOUT);
	if(res != RES_OK)
	{
		TRACE_MSC(" res=%x(%04X)", res, msc_hnd->error);
	}
	if(res == RES_OK)
	{
		// data transfer
		if(buf && len)
		{
			if(transaction->cbw.bmCBWFlags & USBMSC_CBW_FLAGS_IN)
			{
				// IN transaction
				if(transaction->cbw.dCBWDataTransferLength)
				{
					TRACELN_MSC("Data read Tag:%X ", cb_tag);
#if USBMSC_READING_BY_PACKETS
					USBEndpointDescriptor* ped = usb_get_enpoint(config_descriptor,  msc_hnd->mode.as_byteptr[0], pid->bInterfaceNumber);
					uint32_t dwRead, retry, req_len = len;
					retry = USBMSC_DATA_RETRY;

					while(len && res == RES_OK)
					{
						dwRead = len;
						if(dwRead > ped->wMaxPacketSize)
							dwRead = ped->wMaxPacketSize;
						res = msc_hnd->tsk_read(buf, dwRead, USBMSC_READ_TOUT);
						if(res == RES_OK)
						{
							dwRead -= msc_hnd->len;
							buf = (char *)buf + dwRead;
							len -= dwRead;
							retry = USBMSC_DATA_RETRY;
						}else
						{
							TRACELN_MSC("Filed to read packet #%u !", (req_len -len)/ped->wMaxPacketSize);
							TRACE_MSC(" res=%x(%04X)", res, msc_hnd->error);
							TRACE_MSC("remaining %ub of %u ", len, req_len);
							if((--retry))
							{
								tsk_sleep(USBMSC_DATA_SLEEP);
								if(res == RES_FATAL)
									break;
								if(res == FLG_DATA)
									res = clr_endpoint_stall(msc_hnd->mode.as_bytes[0]);
								else
									res = RES_OK;
							}else{
								break;
							}
						}

					}
#else
					res = msc_hnd->tsk_read(buf, len, USBMSC_READ_TOUT);
#endif
					if(res != RES_OK)
					{
						TRACE_MSC("res=%x(%04X)", res, msc_hnd->error);
					}
				}
			} else
			{
				// OUT transaction
				TRACELN_MSC("Data write Tag:%X ", cb_tag);

#if USBMSC_WRITING_BY_PACKETS
					USBEndpointDescriptor* ped = usb_get_enpoint(config_descriptor, msc_hnd->mode.as_byteptr[1], pid->bInterfaceNumber);
					uint32_t dwRead, retry, req_len = len;
					retry = USBMSC_DATA_RETRY;
					while(len && res == RES_OK)
					{
						dwRead = len;
						if(dwRead > ped->wMaxPacketSize)
							dwRead = ped->wMaxPacketSize;
						res = msc_hnd->tsk_write(buf, dwRead, USBMSC_WRITE_TOUT);
						if(res == RES_OK)
						{
							dwRead -= msc_hnd->len;
							buf = (char *)buf + dwRead;
							len -= dwRead;
							retry = USBMSC_DATA_RETRY;
						}else
						{
							TRACELN_MSC("Filed to write packet #%u !", (req_len -len)/ped->wMaxPacketSize);
							TRACE_MSC(" res=%x(%04X)", res, msc_hnd->error);
							TRACE_MSC("remaining %ub of %u ", len, req_len);
							if((--retry))
							{
								tsk_sleep(USBMSC_DATA_SLEEP);
								if(res == RES_FATAL)
									break;
								if(res == FLG_DATA)
									res = clr_endpoint_stall(msc_hnd->mode.as_bytes[1]);
								else
									res = RES_OK;
							}else{
								break;
							}
						}
					}
#else
					res = msc_hnd->tsk_write(buf, len, USBMSC_WRITE_TOUT);
#endif
				if(res != RES_OK)
				{
					TRACE_MSC("res=%x(%04X)", res, msc_hnd->error);
				}else
				{
					TRACE1_MSC("Ok");
				}
			}
			// data transfer is complete

			if(res != RES_OK && (cdb->opcode == SCSI_CMD_WRITE_10 || cdb->opcode == SCSI_CMD_READ_10))
			{
				TRACE_MSC(" lba:%u",  __REV(cdb->lba));
			}
			if(res != RES_OK && res != RES_FATAL)
			{
				// Stall
				if(res == FLG_DATA )
				{
					if(transaction->cbw.bmCBWFlags & USBMSC_CBW_FLAGS_IN)
						clr_endpoint_stall(msc_hnd->mode.as_bytes[0] | 0x80);
					else
						clr_endpoint_stall(msc_hnd->mode.as_bytes[1]);
				}else if( res == RES_ERROR)
				{
					// transfer is complete with error
					// TODO:
					TRACE_MSC_ERROR("res=%x\e[1;97m msc reset\e[m", res);
					res1 =msc_reset();
					if(res1 == RES_OK)
					{
						TRACE1_MSC(" Ok");
					}
					else
						TRACE_MSC(" Fail %x(%X)", res1, ep0_hnd->error);
					return res;
				}
			}
		}

		// get status
		for (int retry = 0; retry < 3; retry++)
		{
			//
			// Attemp to the read Command Status Wrapper from bulk in endpoint
			//
			TRACELN1_MSC("Read CSW ");
			memclr(&transaction->csw, USBMSC_CSW_SIZEOF);
			res1 = msc_hnd->tsk_read(&transaction->csw, USBMSC_CSW_SIZEOF, USBMSC_READ_TOUT);
			//flush unexpected data
			if(msc_hnd->mode1 && (res1 == RES_OK) && !msc_hnd->len)	//mode1 = available data length
			{
				char buff[8];
				TRACELN1_MSC("Unexpected data!");
				while(msc_hnd->tsk_read(buff, sizeof(buff), USB_SETUP_READ_TOUT)== RES_OK)
				{
					if(!msc_hnd->mode1)
						break;
				}
			}
			if (res1 != RES_OK)
			{
				TRACE_MSC(" res=%x(%04X)", res1, msc_hnd->error);
				// Stall
				if (res1 == FLG_DATA)
				{
					clr_endpoint_stall(msc_hnd->mode.as_bytes[0] | 0x80);
				}
				if(res1 == RES_IDLE && res == RES_IDLE)
					break;
				continue;
			}else
			{
				TRACE1_MSC("Ok");
			}

			res1 = RES_ERROR;
			if (transaction->csw.dCSWSignature == USBMSC_CSW_SIGNATURE &&
					transaction->csw.dCSWTag == cb_tag)
			{
				if (transaction->csw.bCSWStatus != USBMSC_CSW_STATUS_PHASEERROR)
				{
					if(transaction->csw.bCSWStatus )
					{
						TRACELN1_MSC("Status: Command Failed");
					}
					res1 = RES_OK;
				}
				break;
			}else
			{
				if(transaction->csw.dCSWSignature != USBMSC_CSW_SIGNATURE)
				{
					TRACELN1_MSC("Invalis signature!");
				}else if(transaction->csw.dCSWTag != cb_tag)
				{
					TRACELN_MSC("Tags mismatch ret:%X expect:%X!", transaction->csw.dCSWTag, cb_tag);
				}else if(transaction->csw.bCSWStatus == USBMSC_CSW_STATUS_PHASEERROR)
				{
					TRACELN1_MSC("Status: Phase Error!");
				}else if(transaction->csw.bCSWStatus == USBMSC_CSW_STATUS_FAIL)
				{
					TRACELN1_MSC("Status: Command Failed!");
				}else if(transaction->csw.bCSWStatus == USBMSC_CSW_STATUS_PASS)
				{
					TRACELN1_MSC("Status: Command Passed");
				}

			}
		}
		if(res == RES_OK && res1 != RES_OK)
			res = res1;

	}
	if(res != RES_OK)
	{
		TRACE_MSC_ERROR("res=%x\e[1;97m msc reset\e[m", res);
		res1 =msc_reset();
		if(res1 == RES_OK)
			TRACE1_MSC(" Ok");
		else
			TRACE_MSC(" Fail %x(%X)", res1, ep0_hnd->error);
	}
	return res;
}

RES_CODE usb_remote_msc_t::msc_command_with_retry(usbmsc_cs_t* transaction, void* buf, uint32_t len)
{
	RES_CODE res;
	uint32_t time, retry;

	time = CURRENT_TIME;
	retry = 0;
	do
	{
		res = msc_command(transaction, buf, len);
		if(res == RES_FATAL)
			break;
		if(res == RES_OK && transaction->csw.bCSWStatus == USBMSC_CSW_STATUS_PASS)
			break;
		TRACELN_MSC("Retry remaining %u s", 60 - seconds_since(time));
		res = msc_request_sense();
		if( res == RES_FATAL)
			break;

	    // If the sense data shows the drive is not ready, we need execute the cmd again.
	    // We limit the upper boundary to 60 seconds.
		if(res != RES_OK)
		{
		    // If the status is other error, then just retry 5 times.
			if(++retry > 5)
				break;
		}
		res = RES_ERROR;
	} while( seconds_since(time) < 60);

	return res;
}

RES_CODE usb_remote_msc_t::cmd_test_unit_ready(usbmsc_cs_t* transaction)
{
	scsi_cmd_test_unit_ready_t* cdb = (scsi_cmd_test_unit_ready_t*)transaction->cbw.CBWCB;

	transaction->cbw.bmCBWFlags = USBMSC_CBW_FLAGS_OUT;

	transaction->cbw.bCBWCBLength = sizeof(*cdb);
	memclr(transaction->cbw.CBWCB, USBMSC_MAX_CDBLEN);
	cdb->opcode = SCSI_CMD_TEST_UNIT_READY;

	return msc_command_with_retry(transaction, nullptr, SCSI_TRLEN_TEST_UNIT_READY);
}

RES_CODE usb_remote_msc_t::cmd_read_capcity(usbmsc_cs_t* transaction)
{
	scsi_cmd_read_capacity_t* cdb = (scsi_cmd_read_capacity_t*)transaction->cbw.CBWCB;
	scsi_data_read_capacity_t data;
	RES_CODE res;

	transaction->cbw.bmCBWFlags = USBMSC_CBW_FLAGS_IN;

	transaction->cbw.bCBWCBLength = sizeof(*cdb);
	memclr(transaction->cbw.CBWCB, USBMSC_MAX_CDBLEN);
	cdb->opcode = SCSI_CMD_READ_CAPACITY;

	data.block_len = 0;
	data.lba = 0;
	res = msc_command_with_retry(transaction, &data, SCSI_TRLEN_READ_CAPACITY);
	blk_size = __REV(data.block_len);
	capacity = __REV(data.lba) + 1;
	return res;
}

RES_CODE usb_remote_msc_t::cmd_inquiry(usbmsc_cs_t* transaction)
{
	scsi_cmd_inquiry_t* cdb = (scsi_cmd_inquiry_t*)(transaction->cbw.CBWCB);
	scsi_data_inquiry_t data;
	RES_CODE res;

	transaction->cbw.bmCBWFlags = USBMSC_CBW_FLAGS_IN;

	transaction->cbw.bCBWCBLength = sizeof(*cdb);
	memclr(transaction->cbw.CBWCB, USBMSC_MAX_CDBLEN);
	cdb->opcode = SCSI_CMD_INQUIRY;
	cdb->allocation_length = __REV16(SCSI_TRLEN_INQUIRY);

	memclr(&data, sizeof(data));
	res = msc_command_with_retry(transaction, &data, SCSI_TRLEN_INQUIRY);
	if(res == RES_OK)
	{
		TRACELN("MSC Found:\r\ntype=%x", data.qualifier_type);
		TRACELN("rmb=%x", data.rmb);
		TRACELN("version=%x", data.version);
		TRACELN("len=%x\r\nvid=", data.len);
		TRACE_BUF(data.vendorid, 8);
		TRACELN1("pid=");
		TRACE_BUF(data.productid, 16);
		TRACELN1("nrev=");
		TRACE_BUF(data.revision, 4);
		TRACELN1("");

	}
	return res;
}

//------------------------------------------------------------------------------
//	functions
//------------------------------------------------------------------------------

RES_CODE usb_remote_msc_t::init_msd()
{
	usbmsc_cs_t transaction;
	RES_CODE res;

	get_max_lun();

	// test init
	for(int i=0; i <100; i++)
	{
		tsk_sleep(50);

		res = cmd_test_unit_ready( &transaction);
		if(res == RES_OK && transaction.csw.bCSWStatus == USBMSC_CSW_STATUS_PASS)
		{
				break;
		}
		if(res == RES_FATAL && i > 3)
			break;
	}

	// get capacity of volume
	if(res != RES_OK)
		res = RES_BUSY_WAITING;
	else
	{
		res = cmd_read_capcity(&transaction);
	}

	// Inquiry - information regarding the logical unit
	if(res == RES_OK)
	{
		res = cmd_inquiry(&transaction);
	}

	return res;
}


// Bulk-Only Mass Storage Reset (class-specific request)
RES_CODE usb_remote_msc_t::msc_reset()
{
	RES_CODE res;

	TRACELN1_MSC("\e[1;97mRESET");

	for(int i=0; i < 3; i++)
	{
		tsk_sleep(50);
		req.bmRequestType = USB_REQ_OUT_CLASS_INTERFACE;
		req.bRequest = MSCRequest_BOMSR;
		req.wValue = 0;
		req.wIndex = pid->bInterfaceNumber;						// interface
		req.wLength = 0;

		res = std_request(NULL);

		if (res == RES_OK)
		{
			TRACE1_MSC(":Ok IN");
			// The device shall NAK the host's request until the reset is
			// complete. We can use this to sync the device and host. For
			// now just stall 100ms to wait for the device.

			// Clear the Bulk-In and Bulk-Out stall condition.
			res = clr_endpoint_stall(msc_hnd->mode.as_bytes[0] | 0x80);
			if(res == RES_OK)
			{
				TRACE_MSC(":Ok OUT");
				res = clr_endpoint_stall(msc_hnd->mode.as_bytes[1]);
				if(res == RES_OK)
				{
					TRACE1_MSC(":Ok");
					break;
				}
			}
			TRACELN_MSC("FAIL(%u)\r\nRESET", i+1);
		}

	}
	if(RES_OK != res)
	{
		TRACE_MSC(":Fail res=%X(%X)", res, ep0_hnd->error);
	}
	TRACE1_MSC("\e[m");
	return res;
}

RES_CODE usb_remote_msc_t::msc_request_sense()
{
	usbmsc_cs_t transaction;
	scsi_data_request_sense_t data;
	scsi_cmd_request_sense_t* cdb =
			(scsi_cmd_request_sense_t*) (transaction.cbw.CBWCB);
	RES_CODE res;

	transaction.cbw.bmCBWFlags = USBMSC_CBW_FLAGS_IN;
	transaction.cbw.bCBWCBLength = sizeof(*cdb);
	memclr(transaction.cbw.CBWCB, USBMSC_MAX_CDBLEN);
	cdb->opcode = SCSI_CMD_REQUEST_SENSE;
	cdb->lun = USB_BOOT_LUN(lun);
	cdb->alloc_len = sizeof(scsi_data_request_sense_t);
	memclr(&data, sizeof(data));

	res = msc_command(&transaction, &data, SCSI_TRLEN_REQUEST_SENSE);

	if (res == RES_OK)
	{
		if (transaction.csw.bCSWStatus != USBMSC_CSW_STATUS_PASS)
			res = RES_INVALID_DATA;
		else
		{
			TRACELN_MSC("sense key %X", data.sense_key & 0xf);
			switch (data.sense_key & 0xf)
			{
			case USB_BOOT_SENSE_NO_SENSE:
				// It is not an error if a device does not have additional sense information
				if (data.asc != USB_BOOT_ASC_NO_ADDITIONAL_SENSE_INFORMATION)
				{
					// no response
					res = RES_ERROR;
				}
				break;

			case USB_BOOT_SENSE_RECOVERED:
				// Suppose hardware can handle this case, and recover later by itself
				res = RES_IDLE;
				break;

			case USB_BOOT_SENSE_NOT_READY:
				res = RES_ERROR;
				break;

			case USB_BOOT_SENSE_ILLEGAL_REQUEST:
				res = RES_ERROR; // INVALID_PARAMETER;
				break;

			case USB_BOOT_SENSE_UNIT_ATTENTION:
				res = RES_ERROR;
				break;

			case USB_BOOT_SENSE_DATA_PROTECT:
				res = RES_ERROR;
				break;

			default:
				res = RES_ERROR;
				break;
			}
		}
	}

	return res;
}

RES_CODE usb_remote_msc_t::msc_read(void* buf, uint32_t sector, uint32_t count)
{
	usbmsc_cs_t transaction;
	scsi_cmd_read_10_t* cdb = (scsi_cmd_read_10_t*)transaction.cbw.CBWCB;
	RES_CODE res;

	if( (sector +count) > capacity)
		res = RES_ERROR;
	else
	{
		transaction.cbw.bmCBWFlags = USBMSC_CBW_FLAGS_IN;

		transaction.cbw.bCBWCBLength = sizeof(*cdb);
		memclr(transaction.cbw.CBWCB, USBMSC_MAX_CDBLEN);
		cdb->opcode = SCSI_CMD_READ_10;
		cdb->blocks = __REV16(count);
		cdb->lba = __REV(sector);

		res = msc_command_with_retry(&transaction, buf, blk_size * count);
		if(res != RES_OK)
			msc_reset();
	}

	return res;
}

RES_CODE usb_remote_msc_t::msc_write(const void* buf, uint32_t sector, uint32_t count)
{
	usbmsc_cs_t transaction;
	scsi_cmd_write_10_t* cdb = (scsi_cmd_write_10_t*)transaction.cbw.CBWCB;
	RES_CODE res;

	if( (sector +count) > capacity)
		res = RES_ERROR;
	else
	{

		transaction.cbw.bmCBWFlags = USBMSC_CBW_FLAGS_OUT;

		transaction.cbw.bCBWCBLength = sizeof(*cdb);
		memclr(transaction.cbw.CBWCB, USBMSC_MAX_CDBLEN);
		cdb->opcode = SCSI_CMD_WRITE_10;
		cdb->blocks = __REV16(count);
		cdb->lba = __REV(sector);

		res = msc_command_with_retry(&transaction, (void*)buf, blk_size * count);
		if(res != RES_OK)
			msc_reset();
	}
	return res;

}

RES_CODE usb_remote_msc_t::scan_msc(uint32_t port_indx, USBSubClassCode subcls, USBProtocolCode proto)
{
	RES_CODE res;

	// select EPT_0 and device address
	ep0_hnd->mode.as_bytes[0] = EPT_0; // RX pipe
	ep0_hnd->mode.as_bytes[1] = EPT_0; // TX pipe
	res = hdc_init(port_indx);
	if(res == RES_OK)
	{
		msc_hnd->mode0 = ep0_hnd->mode0;  // device hub port
		// TRACE Device descriptor
		TRACELN("MSC: dev found %x:%x", dev_descriptor.idVendor, dev_descriptor.idProduct);
		trace_usb_descriptor(&dev_descriptor.as_generic);
		if(dev_descriptor.bDeviceClass == INTERFACE_DEFINED_CLASS)
		{
			//loop the configurations
			for(uint32_t cfg_indx=0; cfg_indx<dev_descriptor.bNumConfigurations; cfg_indx++)
			{
				res = get_config_descriptor(cfg_indx);
				if(res != RES_OK)
					break;

				// TRACE Configuration descriptor(s)
				trace_usb_descriptor(&config_descriptor->as_generic);

				//loop the interfaces
				for(uint32_t iface_indx=0; iface_indx<config_descriptor->bNumInterfaces; iface_indx++)
				{
					pid = usb_get_interface(config_descriptor, iface_indx);
					if(pid && pid->bDeviceClass == MASSSTORAGE_DEVICE_CLASS &&
							pid->bDeviceSubClass == subcls && pid->bDeviceProtocol == proto)
					{
						// set the configuration
//						set_configuration(0);
						res = set_configuration(config_descriptor->bConfigurationValue);

						if(res == RES_OK)
						{
							msc_hnd->mode.as_bytes[0] = EPT_0;
							msc_hnd->mode.as_bytes[1] = EPT_0;
							for(int i=0; i<pid->bNumEndpoints && i<2; i++)
							{
								USBEndpointDescriptor* ped;

								ped = usb_get_enpoint(config_descriptor, i, pid->bInterfaceNumber);
								if(ped && ped->bmAttributes == ENDPOINT_TYPE_BULK)
								{
									if( ped->bEndpointAddress & 0x80 )
									{
										if(msc_hnd->mode.as_bytes[0] == EPT_0)
										{
											msc_hnd->mode.as_bytes[0] = ped->bEndpointAddress & 0x7F;
											msc_hnd->mode.as_ushort[1] = ep0_hnd->mode.as_ushort[1]; //drv_state_cnt
											usb_svc_configendpoint(msc_hnd, &ped->as_generic);
										}
									} else
									{
										if(msc_hnd->mode.as_bytes[1] == EPT_0)
										{
											msc_hnd->mode.as_bytes[1] = ped->bEndpointAddress ;
											msc_hnd->mode.as_ushort[1] = ep0_hnd->mode.as_ushort[1]; //drv_state_cnt
											usb_svc_configendpoint(msc_hnd, &ped->as_generic);
										}
									}
								}
							}
							return init_msd();
						}
					}
				}
			}
		}
	}

	if(res == RES_OK)
		res = RES_ERROR;
	return res;
}

#if TRACE_MSC_LEVEL >= TRACE_LEVEL_DEBUG
FILE_RESET_OPTIONS
#endif

/*
 * esp8266.cpp
 *
 *  Created on: 05.05.2015
 *      Author: Stanly
 */

#include <tmos.h>
#include <hardware_cpp.h>
#include <wifi_drv.h>
#include <esp8266.h>

#define SOCKET_WAIT_ACCEPT	(0x10 + SOCKET_LISTEN)

#if WIFI_FLOW_CONTROL
#define INC_ALLOC_SIZE(size) mem_alloc_size += (size)
#define DEC_ALLOC_SIZE(size) mem_alloc_size -= (size)
#else
#define INC_ALLOC_SIZE(size)
#define DEC_ALLOC_SIZE(size)
#endif

WEAK_C void wifi_on_pwron_config(wifi_module_type* mod)
{
	//TODO: application specific initialization
}

WEAK_C void wifi_notify_power(wifi_module_type* mod, WIFI_DRIVER_DATA * drv_data)
{
	//TODO: Trace various info
}

WEAK bool wifi_name_pass(CSTRING& name)
{
	name += "\"ESP8266 NET\"";
//	name_pass += ",\"1234asdf\""; // Set password
	return false; // No psaasword, true if password required
}

bool wifi_get_param(const char*row, CSTRING& param, unsigned int num)
{
	const char* start=row;
	const char* end;

	if(row && IS_NOT_NULL(&param))
	{
		param.clear();
		if(*start == '+')
		{
			start++;
			while(*start && !IS_SPACE(*start) && *start != ':')
				start++;
			while(*start && (IS_SPACE(*start)||*start == ':'||*start=='('))
				start++;
 		}
		while(num > 1)
		{
			start=strchr(start,',');
			if(!start)
				return false;
			start++;
			num--;
		}
		if(*start)
		{
			if(/**start == ',' || */ IS_SPACE(*start))
			{
				start++;
				while(*start && IS_SPACE(*start))
					start++;
			}
			if(*start == '"')
			{
				end = strchr(++start, '"');
				if(!end)
					return false;
				param.assign(start, end-start);
				return true;
			}
			end = strchr(start, ',');
			if(end)
				param.assign(start, end-start);
			else
				param=start;
			return true;
		}

	}
	return false;
}

bool wifi_get_param(const char*row, unsigned int& param, unsigned int num)
{
	CSTRING str;
	if(wifi_get_param(row, str, num))
	{
		if(tmos_sscanf(str.c_str(), "%u", &param) == 1)
			return true;
	}
	return false;
}

bool wifi_get_param(const char*row, int& param, unsigned int num)
{
	CSTRING str;
	if(wifi_get_param(row, str, num))
	{
		if(tmos_sscanf(str.c_str(), "%d", &param) == 1)
			return true;
	}
	return false;
}

uint16_t wifi_rssi_to_level(const int32_t rssi )
{
	uint16_t level;
	// rssi in dBm
	if(rssi < -57)	// 4 < -57 dBm
		level = 4;
	else
		level = 5;	// 5 >= -57 dBm
	if(rssi < -69)	// 3 < -69 dBm
		level = 3;
	if(rssi < -81)	// 2 < -81 dBm
		level = 2;
	if(rssi < -93)	// 1 < -93 dBm
		level = 1;
	if(rssi < -105) // 0 < -105 dBm or unknown
		level = 0;
	return level;
}

RES_CODE esp8266_module::wifi_echo_off(bool lowlevel, uint32_t indx)
{
	RES_CODE res;
	DRIVER_INDEX	iface_driver_index;

	res = RES_ERROR;
	for(int i=0; i < 3; i++)
	{
		rcv_hnd.close();
		snd_hnd.close();
		if(lowlevel)
			return RES_OK;
		tsk_sleep(1000);
		iface_driver_index = drv_info->drv_data->iface_driver_index;
		if(!iface_driver_index)
			return RES_FATAL;
		if(rcv_hnd.tsk_open(iface_driver_index, drv_info->iface_mode_stru[indx]) &&
			snd_hnd.tsk_open(iface_driver_index, drv_info->iface_mode_stru[indx])	)
		{
			//wait until rx signals get stable
			wifi_sleep(1000);
			//turn off echo
			if(wifi_send_cmd("E0", 1) & WIFI_CMD_STATE_OK)
			{
				res = RES_OK;
				break;
			}
		}
	}
	return res;
}

RES_CODE esp8266_module::wifi_drv_pwron(bool lowlevel)
{
	RES_CODE res;
	WIFI_DRIVER_DATA * drv_data = drv_info->drv_data;
	bool changed = false;

	if(drv_data->wifi_flags_bad & WIFI_FLAG_SHUTDOWN)
		return wifi_error(NET_ERR_WIFI_SHUTDOWN);
	if( drv_data->wifi_flags_ok & WIFI_FLAG_ON)
		return RES_OK;


	TRACE1_WIFI_DEBUG("\r\nWIFI power");
	drv_data->wifi_flags_ok &= (WIFI_FLAG_DETECTED|WIFI_FLAG_PRESENT);
	if(wifi_pin_pwr)
		PIO_Cfg(wifi_pin_pwr);
#if WIFI_FLOW_CONTROL
	if(wifi_pin_rts)
		PIO_Cfg(wifi_pin_rts);
	PIO_Deassert(wifi_pin_rts);
#endif
	PIO_Deassert(wifi_pin_pwr);
	tsk_sleep(1000);
	PIO_Assert(wifi_pin_pwr);
	tsk_sleep(50);

	do
	{
		res = wifi_echo_off(lowlevel, 0);
		if(lowlevel)
			return res;
		if(res != RES_OK)
		{
			res = wifi_echo_off(lowlevel, 1);
			if(res == RES_OK && !changed)
			{
				//try to switch baudrate
				CSTRING cmd;

				cmd.format(WIFI_BAUD"=%u", *(const uint32_t*)drv_info->iface_mode_stru[0]);
#if !USE_DEPRECATED_AT_CMD
				cmd += ",8,1,0,0";
#endif
				if(wifi_send_cmd(cmd.c_str(), 2) & WIFI_CMD_STATE_OK)
					changed = true;
			}
		}
		else
			break;
	} while(changed);


	if(res == RES_OK)
	{
		CSTRING cmd;
#if WIFI_FLOW_CONTROL
		// Enable hardware flow control
		cmd.format(WIFI_BAUD"=%u", *(const uint32_t*)drv_info->iface_mode_stru[0]);
#if !USE_DEPRECATED_AT_CMD
		cmd += ",8,1,0,3";
#endif
		wifi_send_cmd(cmd.c_str(), 2);
#endif
		// info ESP version
		wifi_send_cmd("+GMR", 5);
		// configure WiFi module
		wifi_send_cmd(WIFI_QUIT_AP, 3);
		// 1. Enable the module to act as �Station�
		// 2. AP mode
		// 3  AP + Station mode
		res = wifi_send_cmd(WIFI_MODE"=3", 5);
		if(WIFI_CMD_STATE_OK == res)
		{

			wifi_on_pwron_config(this);
#if USE_WIFI_ESP8266 >= 3
			res = wifi_send_cmd("+CIPRECVMODE=1", 2);
			if(WIFI_CMD_STATE_OK == res)
#endif
		// 2.Enable multiple connections or not (1-multiple/ 0- Single
		//	Note: 	This mode can only be changed after all connections are
		//	 		disconnected. If server is started, reboot is required.
			res = wifi_send_cmd(WFI_MUX"=1", 50);
			if(WIFI_CMD_STATE_OK == res)
			{
				// configure AP
				CSTRING ssid(WIFI_CFG_AP"=");
				if(wifi_name_pass(ssid))
					ssid += ",6,3";
				else
					ssid += "\"\",6,0";
				res = wifi_send_cmd(ssid.c_str(), 5);
				if(WIFI_CMD_STATE_OK == res)
				{
					wifi_send_cmd(WIFI_CFG_AP"?", 5);
					drv_data->wifi_flags_ok |= WIFI_FLAG_ON;
					drv_data->wifi_flags_bad &= ~WIFI_FLAG_ON;
					wifi_notify_power(this, drv_data);
					return NET_OK;
				}
			}
		}
	}
	drv_data->wifi_error = NET_ERR_WIFI_ON;
	drv_data->wifi_flags_bad |= WIFI_FLAG_ON;
    return wifi_error(NET_ERR_WIFI);
}

RES_CODE esp8266_module::wifi_drv_off()
{
	NET_CODE res;
	WIFI_DRIVER_DATA * drv_data = drv_info->drv_data;


	res = wifi_on_deregister(this);
	if(res != NET_OK)
		return res;

	if(!(drv_data->wifi_flags_ok & WIFI_FLAG_ON))
		return RES_OK;

	drv_data->wifi_flags_ok &= ~(WIFI_FLAG_ON | WIFI_FLAG_REGISTERED );

#if USE_WIFI_LISTEN
	if(listen_socket)
		wifi_close_listen(listen_socket, NET_ERR_SOCK_ABORT);
#endif
	for(unsigned int sid=0; sid < WIFI_ESP8266_MAX_SOCKETS; sid++)
	{	wifi_esp8266_socket_close(sid);
		wifi_driver_socket_close(sid, NET_ERR_SOCK_ABORT);
	}

	//TODO: try to stop
	//check power state
	wifi_notify_power(this, drv_data);
	drv_data->signal_level = 0;
	PIO_Deassert(wifi_pin_pwr);
    TRACE1_WIFI_DEBUG("\r\nWIFI off");
	return RES_OK;
}


NET_CODE esp8266_module::wifi_reset(bool force, wifi_module_type** drv_module)
{
	WIFI_DRIVER_DATA *drv_data = drv_info->drv_data;
	if( (drv_data->wifi_flags_ok & WIFI_FLAG_ON ) &&
			!(drv_data->wifi_flags_bad & WIFI_FLAG_SHUTDOWN))
    {
		if(force || /*used_sockets ||*/ sleep_tout++ > MODULE_SLEEP_TOUT)
        {
       		sleep_tout = 0;
        	WIFI_CMD_STATE rc = wifi_send_cmd("",2);
            if(  rc & WIFI_CMD_STATE_OK )
            {
            	if(rc != WIFI_CMD_STATE_OK)
            		rc = wifi_send_cmd("E0",2);
            	else
            	{
    				rc = wifi_drv_level();
    				if(rc == RES_OK && wifi_watchdog_cnt)
    				{
    					rc = WIFI_CMD_STATE_OK;
    					wifi_watchdog_cnt = WIFI_WDT_PERIOD;
    				}
    				else
    					rc = WIFI_CMD_STATE_CMES;
            	}
            }

            if(rc != WIFI_CMD_STATE_OK || force )
            {
        		sleep_tout = MODULE_SLEEP_TOUT; //do not sleep
            	if(!wifi_watchdog_cnt--)
            	{
            		wifi_watchdog_cnt = WIFI_WDT_PERIOD;
            		drv_data->wifi_flags_bad |= WIFI_FLAG_SHUTDOWN;
            		usr_drv_icontrol(WIFI_DRV_INDX, DCR_GET_WAITING, (void *)RES_SIG_IDLE);
            		wifi_drv_off();
            		// force WiFi detection again
            		if(drv_module)
            			*drv_module = nullptr;
            		drv_data->wifi_flags_bad = WIFI_STATE_OFF;
            		drv_data->wifi_flags_ok = WIFI_STATE_OFF;
            	}
            }
        } else
        {
    		wifi_on_blink_transfer(this, WIFI_STATE_INDICATOR);
        }
    }
    return NET_OK;
}

NET_CODE esp8266_module::wifi_drv_level()
{
	WIFI_DRIVER_DATA * drv_data = drv_info->drv_data;
	unsigned short& level =drv_data->signal_level;
	RES_CODE res = NET_ERR_WIFI;
	int32_t rssi;

	if( (drv_data->wifi_flags_ok & (WIFI_FLAG_ON|WIFI_FLAG_REGISTERED))
			== (WIFI_FLAG_ON|WIFI_FLAG_REGISTERED))
	{
		if(wifi_send_cmd(WIFI_JOIN_TO_AP"?",10) == WIFI_CMD_STATE_ROK)
		{
			//+CWJAP_CUR:<ssid>, <bssid>, <channel>, <rssi>
			if(wifi_get_param(buf, rssi, 4))
			{
				level =wifi_rssi_to_level(rssi);
				TRACELN("rssi:%d(%u)", rssi, level);
			}
			res = RES_OK;
		}
	}else
	{
		level = 0;
		res = RES_OK;
	}
	return res;
}

int esp8266_module::wifi_notification(const char* row)
{
	if(IS_DIGIT(row[0]) && row[1] == ',')
	{
		unsigned int id = row[0] - '0';
		if(id < WIFI_ESP8266_MAX_SOCKETS)
		{
			if(cmd_submatch("CLOSED", row+2) || cmd_submatch("CONNECT FAIL", row+2))
			{
				if(alloc_sockets[id])
				{
					closed_sockets[id] = true;
					if(!received_size[id]
#if USE_WIFI_ESP8266 < 3 // version 3.0
					   || !received_data[id]
#endif
					)
						wifi_driver_socket_close(id, NET_ERR_SOCK_DISCONNECT);
					accept_id[id] = WIFI_ESP8266_MAX_SOCKETS;
				}
				return 1;
			}
			if(cmd_submatch("CONNECT", row+2))
			{
				closed_sockets[id] = false;
				if(pending_connection && pending_connection->sock_id == id)
				{
					TRACELN_WIFI_DEBUG("STATION conn:%u", id);
					pending_connection->sock_state = SOCKET_CONECTED;
					return 1;
				}
				//1: ESP8266 runs as server
				TRACELN_WIFI_DEBUG("SERVER conn:%u", id);
				if(listen_socket)
				{
					if( accept_id[id] == id)
					{
						// already accepted
						if(!alloc_sockets[id]) // but not allocated
							module_accepted_socket(id);
					}
					else
					{
						if(accept_id[id] == WIFI_ESP8266_MAX_SOCKETS)
						{
							accept_id[id] = id;
							module_accepted_socket(id);
						}
						else
						{
							// mismatch between id and accepted
							if(alloc_sockets[id])
							{
								wifi_driver_socket_close(id, NET_ERR_SOCK_DISCONNECT);
							}
							accept_id[id] = id;
							module_accepted_socket(id);
						}
					}
				}
				else
				{
					TRACELN_WIFI_DEBUG("SERVER conn:%u REJECT", id);
					notify_state = wifi_notify_t(WIFI_NOTIFY_REJECT_CONNECTION_FIRST+id);
					tsk_send_signal(CURRENT_TASK, WIFI_NOTIFY_SIGNAL);

				}
				return 1;
			}
		}
	}
	if(cmd_match("+IPD", row))
	{
		wifi_data_received(row);
		return 1;
	}
	if(cmd_match("busy p...", row))
	{
		// !?
		return 1;
	}
	if(cmd_match("WIFI DISCONNECT", row))
	{
		drv_info->drv_data->wifi_flags_ok &= ~WIFI_FLAG_REGISTERED;
		return 1;
	}
	if(cmd_match("WIFI CONNECTED", row))
	{
		//TODO:
		return 1;
	}
	if(cmd_match("WIFI GOT IP", row))
	{
		drv_info->drv_data->wifi_flags_ok |= WIFI_FLAG_REGISTERED;
		return 1;
	}
/*
	if(cmd_submatch("Linked", row))
	{
		notify_state = WIFI_NOTIFY_LINKED;
		tsk_send_signal(CURRENT_TASK, WIFI_NOTIFY_SIGNAL);
		return 1;
	}
	if(cmd_submatch("Link", row))
	{
		notify_state = WIFI_NOTIFY_LINK;
		tsk_send_signal(CURRENT_TASK, WIFI_NOTIFY_SIGNAL);
		return 1;
	}
	if(cmd_submatch("Unlink", row))
	{
		notify_state = WIFI_NOTIFY_UNLINK;
		tsk_send_signal(CURRENT_TASK, WIFI_NOTIFY_SIGNAL);
		return 1;
	}
*/
	return 0;
}

void esp8266_module::wifi_notificatoin_response()
{
	switch(notify_state)
	{
	case WIFI_NOTIFY_IDLE:
		break;
	case WIFI_NOTIFY_LINK:
#if USE_WIFI_LISTEN
		if(listen_socket)
		{
			get_socket_state(10);
		}
#endif
		break;
	case WIFI_NOTIFY_LINKED:
		break;
	case WIFI_NOTIFY_UNLINK:
#if USE_WIFI_LISTEN
		if(listen_socket && (listen_socket->res & RES_BUSY_WAITING) == RES_BUSY_WAITING)
		{
			get_socket_state(10);
		}
#endif
		break;
	case WIFI_NOTIFY_REJECT_CONNECTION_FIRST ... WIFI_NOTIFY_REJECT_CONNECTION_LAST:
		{
			CSTRING cmd;
			uint32_t id = notify_state - WIFI_NOTIFY_REJECT_CONNECTION_FIRST;
			cmd.format("+CIPCLOSEMODE=%u,1", id);
			if(wifi_send_cmd(cmd.c_str(), 45) & WIFI_CMD_STATE_OK)
			{
				cmd.format(WIFI_DISCONNECT"=%u", id);
				if(wifi_send_cmd(cmd.c_str(), 45) & WIFI_CMD_STATE_OK)
				{
					get_socket_state(id);
					TRACELN_WIFI_DEBUG("REJECT %u", id);

				}
			}
			notify_state = WIFI_NOTIFY_IDLE;
		}
		break;
	}
}

void esp8266_module::wifi_process_tout(void)
{
	CSocket* sock;
	HANDLE list;
	RES_CODE res;

	if(wifi_tout)
	{
		if(!--wifi_tout)
		{
			if(!used_sockets)
			{
				//--- deactivate the context?
				if(drv_info->drv_data->wifi_flags_ok & WIFI_FLAG_ON)
				{
					for(int i=0; i < 3; i++ )
					{
						if(wifi_send_cmd(WIFI_QUIT_AP, 150) & WIFI_CMD_STATE_OK)
						{
							wifi_send_cmd(WIFI_JOIN_TO_AP"?", 50);
							break;
						}
					}
				}
				wifi_on_disconnect(this);

				TRACELN1_WIFI_DEBUG("WIFI OFF");

				list = waiting_open;
				waiting_open = NULL;
				while(list)
				{
					sock = (CSocket*)list;
					list = sock->next;
					sock->res = RES_BUSY;
					res = wifi_sock_open(sock);
					if(res & FLG_SIGNALED)
						usr_HND_SET_STATUS(sock, res);
				}
				if(!used_sockets)
					wifi_drv_off();
			}
		}
	}
}

void esp8266_module::wifi_cancelation(bool all_station, bool all_softAP)
{
	CSocket* sock;

	//cancel open waiting
	sock = (CSocket*)waiting_open;
	while(sock)
	{
		if(sock->mode1 || all_station)
		{
			sock->list_remove(waiting_open);
			usr_HND_SET_STATUS(sock, RES_SIG_IDLE);
			sock = (CSocket*)waiting_open;
		} else
		{
			sock = (CSocket*)sock->next;
		}
	}

#if WIFI_FLOW_CONTROL
	//cancel pending
	sock = (CSocket*)pending_read_data;
	while(sock)
	{
		if(sock->mode1 || all_station)
		{
			sock->list_remove(pending_read_data);
			usr_HND_SET_STATUS(sock, RES_SIG_IDLE);
			sock = (CSocket*)pending_read_data;
		} else
		{
			sock = (CSocket*)sock->next;
		}
	}
#endif

	//cancel reading sockets
	for(int sid=0; sid < WIFI_ESP8266_MAX_SOCKETS; sid++)
	{
		sock = alloc_sockets[sid];
		if (  sock && ((sock->res & RES_BUSY_WAITING) == RES_BUSY_WAITING))
		{
			if(sock->mode1)
				usr_HND_SET_STATUS(sock, (sock->res & FLG_OK)| FLG_SIGNALED);
			else
			{
				if(all_station && accept_id[sid] == WIFI_ESP8266_MAX_SOCKETS)
					usr_HND_SET_STATUS(sock, (sock->res & FLG_OK)| FLG_SIGNALED);
				if(all_softAP && accept_id[sid] == sid)
					usr_HND_SET_STATUS(sock, (sock->res & FLG_OK)| FLG_SIGNALED);
			}
		}

	}
#if USE_WIFI_LISTEN
	if(listen_socket && (all_softAP || listen_socket->mode1) && ((listen_socket->res & RES_BUSY_WAITING) == RES_BUSY_WAITING))
	{
		usr_HND_SET_STATUS(listen_socket, (listen_socket->res & FLG_OK)| FLG_SIGNALED);
	}
#endif
}

//static const char* message = {"POST /RemoteMeasuringSystem/new_measurement.php HTTP/1.1\nUser-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\nHost: www.tmos-arm.com\nAccept-Language: en-us\nAccept-Encoding: gzip, deflate\nContent-Type: application/x-www-form-urlencoded\nContent-Length:65\nConnection: Keep-Alive\n\r\nid=7&pass=123456&type=temp&value=66.6&delayed=0&meas_interval=10"};
//const char* message = "7";

NET_CODE esp8266_module::wifi_esp8266_init_net(CSocket * sock)
{
	NET_CODE res;
	CSTRING cmd;
	wifi_access_point_t net;
	uint32_t sig = 0;
	bool found;

	res = wifi_send_cmd(WIFI_GET_LOCAL_IP, 50); // for debug info only

	res = wifi_on_init_station(this, sock, &net);
	if(res != NET_IDLE)
		return res;

	for(int i =0; i < 5; i++)
	{
		found = false;
		cmd_state |= WIFI_CMD_STATE_ROW_STOP;
		res = wifi_send_cmd(WIFI_LIST_AVAILABLE_AP, 15);
		cmd_state &= ~WIFI_CMD_STATE_ROW_STOP;

		do
		{
			if(sig)
			{
				process_input(sig, WIFI_LIST_AVAILABLE_AP);
				res = cmd_state;
			}

			if (res >= WIFI_CMD_STATE_OK)
				break;

			if(res & WIFI_CMD_STATE_RETURNED)
			{
				cmd_state = res & ~WIFI_CMD_STATE_RETURNED;
				if(!found)
				{
					res = wifi_on_get_station_net(this, sock, &net);
					if(res == RES_OK)
						found = true;
				}
				row_start = 0;
				row_end = 0;
			}

			sig = tsk_resume_wait_signal(rcv_hnd.signal);
		} while(sig);

		if(cmd_state & WIFI_CMD_STATE_OK)
		{
			if(sock && sock->mode.as_voidptr &&
					WIFI_AP_LIST_INTERFACE == static_cast<sock_mode_t*>(sock->mode.as_voidptr)->interface )
				return RES_OK;

			if(!found)
				res = NET_ERR_WIFI_NET_NAME;
			else
				res = NET_OK;
			break;
		}
	}

	if (res != NET_OK)
		return wifi_error(res);

	// If the wifi is already connected to a network different than the one
	// requested - return NET_IDLE - unavailable
	if (NET_OK == wifi_get_current_net_ssid(cmd))
	{
		if (0 != strcmp(cmd.c_str(), net.ssid.c_str()))
		{
			if(sock && sock->mode.as_voidptr &&
					WIFI_CHANGE_AP_INTERFACE != static_cast<sock_mode_t*>(sock->mode.as_voidptr)->interface )
			return NET_IDLE;
		}
	}

	cmd.format(WIFI_JOIN_TO_AP"=\"%s\",\"%s\"", net.ssid.c_str(), net.pwd.c_str());
	for(int i=0; i < 3; i++)
	{
		res = wifi_send_cmd(cmd.c_str(), 50);
		if (WIFI_CMD_STATE_OK & res)
		{
			res = wifi_send_cmd(WIFI_GET_LOCAL_IP, 50);
			if (WIFI_CMD_STATE_ROK == res)
			{
//				drv_info->drv_data->wifi_flags_ok |= WIFI_FLAG_REGISTERED;
				connected_network_name = net.ssid;
				return NET_OK;
			}
			break;
		}
	}
	return NET_ERR_WIFI_REGISTER;
}

NET_CODE esp8266_module::wifi_get_current_net_ssid(CSTRING& ssid)
{
#if !USE_DEPRECATED_AT_CMD
	if(wifi_send_cmd(WIFI_JOIN_TO_AP"?",10) & WIFI_CMD_STATE_OK)
	{
		//+CWJAP_CUR:<ssid>, <bssid>, <channel>, <rssi>
		if(wifi_get_param(buf, ssid, 1))
		{
			if(ssid != "No AP")
				return NET_OK;
			ssid.clear();
		}
	}
#else
	if (true != connected_network_name.empty())
	{
		ssid = connected_network_name;
		return NET_OK;
	}
#endif
	return NET_IDLE;
}

void esp8266_module::wifi_esp8266_socket_close(unsigned int sid)
{
	CSTRING cmd;

	if(get_socket_state(sid))
	{
		cmd.format(WIFI_DISCONNECT"=%u", sid);
		if(wifi_send_cmd(cmd.c_str(), 45) & WIFI_CMD_STATE_OK)
		{
			if( (sid < WIFI_ESP8266_MAX_SOCKETS) && alloc_sockets[sid])
			{
				for(int i=0; i< 500; i++)
				{
					wifi_sleep(10);
					if(closed_sockets[sid])
					{
						TRACE_WIFI_DEBUG("\r\nDisconnect %d", sid);
						break;
					}
				}
			}
		}
	}
}

NET_CODE esp8266_module::wifi_esp8266_socket_open(CSocket* sock)
{
	unsigned int sid;
	sock_mode_t* mode;

	RES_CODE res;
	const char* cmd = WIFI_CONNECTION_STAT;
	unsigned int sig=0;

	cmd_state |= WIFI_CMD_STATE_ROW_STOP;
	res = wifi_send_cmd(cmd, 15);
	cmd_state &= ~WIFI_CMD_STATE_ROW_STOP;

	do
	{
		if(sig)
		{
			process_input(sig, cmd);
			res = cmd_state;
		}

		if (res >= WIFI_CMD_STATE_OK)
			break;

		if(res & WIFI_CMD_STATE_RETURNED)
		{
			cmd_state = res & ~WIFI_CMD_STATE_RETURNED;
			row_start = 0;
			row_end = 0;
		}

		sig = tsk_resume_wait_signal(rcv_hnd.signal);
	} while(sig);


	mode = (sock_mode_t*)sock->mode.as_voidptr;
	if(mode )
	{
		if (  (mode->sock_type == IP_SOCKET_UDP && mode->port)
			||(mode->sock_type == IP_SOCKET_TCP) )
		{
#if USE_WIFI_LISTEN
			if (mode->interface == WIFI_SERVER_INTERFACE)
			{
				if(listen_socket)
				{
					return wifi_net_error(NET_ERR_SOCK_CREATE);
				}
				else
				{
					listen_socket = sock;
					listen_socket->sock_id = WIFI_ESP8266_MAX_SOCKETS;
					listen_socket->sock_state = SOCKET_OPEN;
					used_sockets++;
					return NET_OK;
				}
			}
#endif
			for(sid=0; sid < WIFI_ESP8266_MAX_SOCKETS; sid++)
			{
				if(NULL == alloc_sockets[sid])
				{
					sock->sock_id = sid;
					sock->sock_state = SOCKET_OPEN;
					alloc_sockets[sid] = sock;
					closed_sockets[sid] = false;
					// discard socket data
					if(received_data[sid])
					{
						delete received_data[sid];
						received_data[sid] = nullptr;
					}
					DEC_ALLOC_SIZE(received_size[sid]); //
					received_size[sid] = 0;

					used_sockets++;
					return NET_OK;
				}
			}
		}
	}
	TRACE_WIFI_ERROR("\r\nWIFI:%s create socket ERROR", sock->client.task->name);
	return wifi_net_error(NET_ERR_SOCK_CREATE);

}

RES_CODE esp8266_module::wifi_sock_open(CSocket* sock) // NET_ERROR OK
{
	if(sock->sock_state == SOCKET_CLOSED)
	{
		//---  check the registration
		sock_mode_t *mode = (sock_mode_t*)sock->mode.as_voidptr;
		if( mode && wifi_drv_on() == NET_OK )
		{
			NET_CODE res = NET_ERROR;
			if(mode->interface == WIFI_SERVER_INTERFACE)
				res = NET_OK;
			else
				res = wifi_esp8266_init_net(sock);
			switch(res)
			{
			case NET_IDLE:
				sock->next = waiting_open;
				waiting_open = sock;
				sock->res = RES_BUSY_WAITING;
				return RES_IDLE;

			case NET_OK:
				//--- open new socket
				if( NET_OK == wifi_esp8266_socket_open(sock))
					return RES_SIG_OK;
				break;
			}

		}
	} else
		wifi_net_error(NET_ERR_SOCK_NOT_CLOSED);

	return RES_SIG_ERROR;
}

#if USE_WIFI_LISTEN
RES_CODE esp8266_module::wifi_close_listen(CSocket* sock, unsigned int reason)
{
	if(listen_socket)
	{
		if(sock == listen_socket)
		{
			listen_port = 0;
			listen_socket = nullptr;
//			wifi_send_cmd(WIFI_SERVER"=0", 5);

			sock->sock_state = SOCKET_CLOSED;
			// do we have to signal it?
			if((sock->res & RES_BUSY_WAITING) == RES_BUSY_WAITING)
			{
				//How to signal it?
				sock->error = reason;
				TRACE_WIFI_DEBUG("\r\ndisconnect listen");
				tsk_HND_SET_STATUS(sock, (sock->res & FLG_OK)| FLG_SIGNALED);
			}
			if(!--used_sockets)
			{
				if(waiting_open)
					wifi_tout = 1;
				else
				 if(drv_info->drv_data->wifi_clients > 2)
						wifi_tout = 5;
				 else
						wifi_tout = 1;
			}
			return RES_OK;
		}
	}
	return RES_IDLE;
}
#endif

RES_CODE esp8266_module::wifi_sock_close(CSocket* sock) // NET_ERROR OK
{
	unsigned int sid;
	RES_CODE res = RES_SIG_ERROR;

	sid = sock->sock_id;
#if USE_WIFI_LISTEN
	if( RES_OK == wifi_close_listen(sock, NET_OK))
		return RES_SIG_OK;
#endif
	if(sid < WIFI_ESP8266_MAX_SOCKETS)
	{
		if( (sock->sock_state & SOCKET_CONECTED) )
			wifi_esp8266_socket_close(sid);

		wifi_driver_socket_close(sid, NET_OK);
		return RES_SIG_OK;
	}
	wifi_net_error(NET_ERR_SOCK_CLOSED);
	return res;
}

void esp8266_module::module_accepted_socket(unsigned int sock_id)
{
	if(listen_socket && (listen_socket->res & RES_BUSY_WAITING) == RES_BUSY_WAITING )
	{
		RES_CODE res = RES_SIG_ERROR;
		CSocket *new_socket = (CSocket *)(listen_socket->dst.as_voidptr);
		if(new_socket)
		{
			if(alloc_sockets[sock_id])
				wifi_driver_socket_close(sock_id, NET_ERR_SOCK_DISCONNECT);
			new_socket->sock_id = sock_id;
			new_socket->sock_state = SOCKET_CONECTED;
			alloc_sockets[sock_id] = new_socket;
			closed_sockets[sock_id] = false;
			used_sockets++;
			res = RES_SIG_OK;
		}
		usr_HND_SET_STATUS(listen_socket, res);
	}
}

unsigned int esp8266_module::get_socket_state(unsigned int sock_id)
{
	RES_CODE res;
	const char* cmd = WIFI_CONNECTION_STAT;;
	unsigned int sig=0;
	uint32_t id;
	bool found = false;
	unsigned int status;
	unsigned int scaned=0;

	cmd_state |= WIFI_CMD_STATE_ROW_STOP;
	res = wifi_send_cmd(cmd, 15);
	if(res <=WIFI_CMD_STATE_RETURNED)
	{
		do
		{
			if(sig)
			{
				process_input(sig, cmd);
				res = cmd_state;
			}

			if (res >= WIFI_CMD_STATE_OK)
				break;

			if(res & WIFI_CMD_STATE_RETURNED)
			{
				cmd_state = res & ~WIFI_CMD_STATE_RETURNED;
				if(tmos_sscanf(buf, "STATUS:%u", &status))
				{
					if(status == 5) // Connected
					{
						TRACELN1_WIFI_DEBUG("The Station does NOT connect to an AP");
					}
				}

				if(tmos_sscanf(buf, WIFI_CONNECTION_STAT":%u", &id) && id < WIFI_ESP8266_MAX_SOCKETS)
				{
					// +CIPSTATUS:<link ID>,<type>,<remote IP>,<remote port>,<local port>,<tetype>
					if(wifi_get_param(buf, status, WIFI_CONNECTION_TYPE_INDX))
					{
						//<tetype>:
						// 0: ESP8266 runs as a client.
						// 1: ESP8266 runs as a server.
						scaned |= (1<<id);
						if(status == 1)
						{
							//1: ESP8266 runs as server
							if( accept_id[id] == id)
							{
								// already accepted
								if(!alloc_sockets[id]) // but not allocated
									module_accepted_socket(id);
							}
							else
							{
								if(accept_id[id] == WIFI_ESP8266_MAX_SOCKETS)
								{
									accept_id[id] = id;
									module_accepted_socket(id);
								}
								else
								{
									// mismatch between id and accepted
									if(alloc_sockets[id])
									{
										wifi_driver_socket_close(id, NET_ERR_SOCK_DISCONNECT);
									}
									accept_id[id] = id;
									module_accepted_socket(id);
								}
							}
						}
					}
					if(id == sock_id)
						found = true;

				}
				row_start = 0;
				row_end = 0;
			}

			sig = tsk_resume_wait_signal(rcv_hnd.signal);
		} while(sig);

	}
	cmd_state &= ~WIFI_CMD_STATE_ROW_STOP;
	if((cmd_state & WIFI_CMD_STATE_OK))
	{
		for(unsigned int i=0; i < WIFI_ESP8266_MAX_SOCKETS; i++)
		{
			if(scaned & (1<<i))
				continue;
			// missing socket
			if(alloc_sockets[i] &&
				(!received_size[i]
#if USE_WIFI_ESP8266 < 3 // version 3.0
				 || !received_data[i]
#endif
				)
			)
				wifi_driver_socket_close(i, NET_ERR_SOCK_DISCONNECT);
			accept_id[i] = WIFI_ESP8266_MAX_SOCKETS;
		}
		if(found)
			return 1;
	}
	return 0;
}

void esp8266_module::wifi_driver_socket_close(unsigned int sid, unsigned int reason)
{
	CSTRING cmd;
	CSocket* sock;

	sock = alloc_sockets[sid];
	if(sock)
	{
		if(received_data[sid])
		{
			delete received_data[sid];
			received_data[sid] = NULL;
			DEC_ALLOC_SIZE(received_size[sid]);
		}
		received_size[sid] = 0;

		// do we have to signal it?
		if((sock->res & RES_BUSY_WAITING) == RES_BUSY_WAITING)
		{
			//How to signal it?
			sock->error = reason;
			TRACE_WIFI_DEBUG("\r\ndisconnect %d", sid);
			tsk_HND_SET_STATUS(sock, (sock->res & FLG_OK)| FLG_SIGNALED);
		}
		else
		{
			TRACE_WIFI_DEBUG("\r\ndisconnect %d, err %d", sid, reason);
			wifi_net_error(reason);
		}
		sock->sock_id = SOCKET_ID_INVALID;
		sock->sock_state = SOCKET_CLOSED;
		alloc_sockets[sid] = NULL;
#if USE_WIFI_LISTEN
		accept_id[sid] = WIFI_ESP8266_MAX_SOCKETS;
#endif

		if(!--used_sockets)
		{
			if(waiting_open)
				wifi_tout = 1;
			else
			 if(drv_info->drv_data->wifi_clients > 2)
					wifi_tout = 5;
			 else
					wifi_tout = 1;
		}
	}
}

#if USE_DEPRECATED_AT_CMD
RES_CODE esp8266_module::wifi_receive_check(char sym)
{
	char ch;

	if( rcv_hnd.tsk_read_pkt(&ch, 1, WIFI_READ_TOT) & RES_OK)
	{
		if(ch >= 0x20)
			TRACE_CHAR(ch);
		else
			TRACE("(%02u)", ch);
		if(ch == sym)
			return RES_OK;
		else
			return RES_EOF;
	}
	return rcv_hnd.res;
}
#endif

bool esp8266_module::wifi_data_received(const char* row)
#if USE_WIFI_ESP8266 < 3 // version 3.0
{
	uint32_t id, size, len;
	if(2 == tmos_sscanf(row,"+IPD,%u,%u", &id, &size))
	{
		if(id < WIFI_ESP8266_MAX_SOCKETS)
		{
			CSocket* sock = alloc_sockets[id];
			char* mem;

			if(sock && sock->sock_state == SOCKET_CONECTED)
			{
				if (sock && ((sock->res & RES_BUSY_WAITING) == RES_BUSY_WAITING))
				{
					len = min(size, sock->len);
					if( rcv_hnd.tsk_read_pkt(sock->dst.as_charptr, len, WIFI_READ_TOT) == RES_OK)
					{
						//TRACE_BUF(sock->dst.as_charptr, len, TC_TXT_BLUE);
						len -= rcv_hnd.len;
						sock->len -= len;
						sock->dst.as_ccharptr += len;
						tsk_HND_SET_STATUS(sock, RES_SIG_OK);
					}
					else
					{
						len = size;
						tsk_HND_SET_STATUS(sock, RES_SIG_ERROR);
					}
					size -= len;
				}
				if(size)
				{
					mem = (char*)tsk_realloc(received_data[id], received_size[id]+size);
					if(mem)
					{
						received_data[id] = mem;
						INC_ALLOC_SIZE(size);
						if( rcv_hnd.tsk_read_pkt(mem+received_size[id], size, WIFI_READ_TOT) == RES_OK)
						{
							TRACE1_WIFI_DEBUG("read Ok");
//							TRACE_BUF(received_data[id]+received_size[id], size, TC_TXT_CYAN);
							received_size[id] += size;
#if WIFI_FLOW_CONTROL
							if( mem_alloc_size > WIFI_FLOW_HOLD_SIZE )
							{
								PIO_Assert(wifi_pin_rts);
								stop_rcv = true;
							}
#endif
						}
					} else
					{
						//ops out of memory..
						// drain the data and close socket?
						while(size)
						{
							if( rcv_hnd.tsk_read_pkt(buf, min(size, sizeof(buf)), WIFI_READ_TOT) != RES_OK)
								break;

							size -= min(size, sizeof(buf));
						}
					}
				}

			}
			else
			{
				if(listen_socket)
				{
					if(size)
					{
						mem = (char*)tsk_realloc(received_data[id], received_size[id]+size);
						if(mem)
						{
							received_data[id] = mem;
							if( rcv_hnd.tsk_read_pkt(mem+received_size[id], size, WIFI_READ_TOT) == RES_OK)
							{
								INC_ALLOC_SIZE(size);
								received_size[id] += size;
							}

						} else
						{
							//ops out of memory..
							// drain the data and close socket?
							while(size)
							{
								if( rcv_hnd.tsk_read_pkt(buf, min(size, sizeof(buf)), WIFI_READ_TOT) != RES_OK)
									break;

								size -= min(size, sizeof(buf));
							}
						}
					}
				}
			}

#if USE_DEPRECATED_AT_CMD
			if(wifi_receive_check('\r') == RES_OK)
				if(wifi_receive_check('\n') == RES_OK)
				{
					while(1)
					{
						switch(wifi_receive_check('\r'))
						{
						case RES_OK:
							wifi_receive_check('\n');
							return;
						case RES_EOF:
							break;		// received "OK or something else"
						default:
							return; 	//error
						}
					}
				}
			//flush:
			// if read \r
			// if read \n
			// while !read \r\n
			//
#endif
		}
	}
#if WIFI_FLOW_CONTROL
	return stop_rcv;
#else
	return false;
#endif
}
#else
{
	uint32_t id, size;
	if(2 == tmos_sscanf(row,"+IPD,%u,%u", &id, &size))
	{
		CSocket* sock;
		if(id < WIFI_ESP8266_MAX_SOCKETS && (sock = alloc_sockets[id]))
		{
			sock_mode_t* mode = (sock_mode_t*)sock->mode.as_voidptr;
			if(mode)
			{
				if(mode->sock_type == IP_SOCKET_UDP)
				{
					if(size)
					{
						char *mem = (char*)tsk_realloc(received_data[id], received_size[id]+size);
						if(mem)
						{
							received_data[id] = mem;
							INC_ALLOC_SIZE(size);
							if( rcv_hnd.tsk_read_pkt(mem+received_size[id], size, WIFI_READ_TOT) == RES_OK)
							{
								TRACE1("read Ok");
	//							TRACE_BUF(received_data[id]+received_size[id], size, TC_TXT_CYAN);
								received_size[id] += size;
#if WIFI_FLOW_CONTROL
								if( mem_alloc_size > WIFI_FLOW_HOLD_SIZE )
								{
									PIO_Assert(wifi_pin_rts);
									stop_rcv = true;
								}
#endif
								if( sock->sock_state == SOCKET_CONECTED)
								{
									if ((sock->res & RES_BUSY_WAITING) == RES_BUSY_WAITING)
										sock->tsk_start_handle();
								}
							}
						} else
						{
							//ops out of memory..
							// drain the data and close socket?
							while(size)
							{
								if( rcv_hnd.tsk_read_pkt(buf, min(size, sizeof(buf)), WIFI_READ_TOT) != RES_OK)
									break;

								size -= min(size, sizeof(buf));
							}
						}
					}
				}
				else
				{
					received_size[id] = size;
					if( sock->sock_state == SOCKET_CONECTED)
					{
						if ((sock->res & RES_BUSY_WAITING) == RES_BUSY_WAITING)
							sock->tsk_start_handle();
					}
				}
			}
		}
	}
	return false;
}
#endif

#if USE_WIFI_ESP8266 < 3 // version 3.0
bool esp8266_module::is_data_received(unsigned char sock_state)
{
#if WIFI_FLOW_CONTROL
	if(sock_state == SOCKET_CONECTED)
	{
		if(stop_rcv && mem_alloc_size < WIFI_FLOW_RELEASE_SIZE)
		{
			process_input(0, nullptr, 0);
		    if (tsk_wait_signal(rcv_hnd.signal, 50))
		    {
				do
				{
					process_input(rcv_hnd.signal, NULL); // UNSOLICITED
					if(stop_rcv)
					{
						return true;
					}
				} while (tsk_resume_wait_signal(rcv_hnd.signal));
		    }
		}
	}
#endif
	return false;
}
#endif

RES_CODE esp8266_module::process_read(CSocket* sock)
#if USE_WIFI_ESP8266 < 3 // version 3.0
{
	unsigned size, id;
	CSTRING cmd;

	TRACELN_WIFI_DEBUG("WIFI: read %d?", sock->len);
	while( sock->sock_state == SOCKET_CONECTED
			|| (sock->sock_id < WIFI_ESP8266_MAX_SOCKETS && received_size[sock->sock_id])
			|| (sock->res & FLG_OK))
	{
		if(!sock->len)
		{
			TRACE1_WIFI_DEBUG(" done!");
			is_data_received(sock->sock_state);
			return RES_SIG_OK;
		}

		if(!received_size[sock->sock_id])
		{
			if(!(sock->res & FLG_OK))
			{
				// ��� ���� ����������� �� ����� � ������� � ������
				TRACE1_WIFI_DEBUG(" wait!");
				sock->res = RES_BUSY_WAITING;
				return RES_IDLE;
			}
			is_data_received(sock->sock_state);
			return  RES_SIG_OK;
		}

		if(!received_data[sock->sock_id])
			break;
		id = sock->sock_id;
		size = sock->len;
		if(size > received_size[id])
			size = received_size[id];
		memcpy(sock->dst.as_voidptr, received_data[id], size);
		sock->res |= FLG_OK;
		sock->dst.as_byteptr += size;
		sock->len -= size;
		if(received_size[id] > size)
		{
			memmove(received_data[id], received_data[id] + size, received_size[id] - size);
			DEC_ALLOC_SIZE(size);
			received_size[id] -= size;
		}
		else
		{
			DEC_ALLOC_SIZE(received_size[id]);
			received_size[id] = 0;
			delete received_data[id];
			received_data[id] = NULL;
			is_data_received(sock->sock_state);
			TRACE1_WIFI_DEBUG(" EOF done!");
			return RES_SIG_OK;
		}
	}
	wifi_sleep(120);
	wifi_net_error(NET_ERR_SOCK_READ);
	TRACE_ERROR("\r\nWIFI:%s read INVALID", sock->client.task->name);
	return RES_FATAL|FLG_SIGNALED;
}
#else
{
	unsigned size, available_len, id;
	const char *rcv_pending;
	CSTRING cmd;

    TRACELN_WIFI_DEBUG("WIFI: read %d?", sock->len);
	while(sock->sock_id < WIFI_ESP8266_MAX_SOCKETS && sock->sock_state == SOCKET_CONECTED)
	{
		if(!received_size[sock->sock_id] && closed_sockets[sock->sock_id])
		{
			wifi_driver_socket_close(sock->sock_id, NET_ERR_SOCK_DISCONNECT);
			if(!(sock->res & FLG_OK))
				return FLG_EOF | FLG_SIGNALED;
			return RES_SIG_OK;
		}

		if(!sock->len)
		{
			TRACE1_WIFI_DEBUG(" done!");
			return (sock->res & FLG_OK)| FLG_SIGNALED;
		}

		if(!received_size[sock->sock_id])
		{
			if(!(sock->res & FLG_OK))
			{
				// ��� ���� ����������� �� ����� � ������� � ������
				TRACE1_WIFI_DEBUG(" wait!");
				sock->res |= RES_BUSY_WAITING;
				return RES_IDLE;
			}
			return  RES_SIG_OK;
		}
		if(sock->mode.as_voidptr && ((sock_mode_t *)sock->mode.as_voidptr)->sock_type == IP_SOCKET_UDP)
		{
			id = sock->sock_id;
			if(!received_data[id])
				break;

			size = sock->len;
			if(size > received_size[id])
				size = received_size[id];
			memcpy(sock->dst.as_voidptr, received_data[id], size);
			sock->dst.as_byteptr += size;
			sock->len -= size;
			if(received_size[id] > size)
			{
				memmove(received_data[id], received_data[id] + size, received_size[id] - size);
				DEC_ALLOC_SIZE(size);
				received_size[id] -= size;
			}
			else
			{
				DEC_ALLOC_SIZE(received_size[id]);
				received_size[id] = 0;
				delete received_data[id];
				received_data[id] = NULL;
				TRACE1(" EOF done!");
			}
			return RES_SIG_OK;
		}
		size = sock->len;
		if(size > 1024)
			size = 1024;

		cmd.format("+CIPRECVDATA=%u,%u", sock->sock_id, size);
	    TRACELN_WIFI_DEBUG("WIFI: read %d from %d", size, received_size[sock->sock_id]);

	    rcv_pending = ":";
	    // ������� ��������� �� ������
	    cmd_state |= WIFI_CMD_STATE_HND;
	    if(wifi_send_cmd(cmd.c_str(), 15) == WIFI_CMD_STATE_RETURNED)
	    {
	    	do
	    	{
				process_input(rcv_hnd.signal, cmd.c_str(), rcv_pending); // ���� �� �� ����� ��,ERROR ��� ������ �� �������
    			if ( cmd_state >= WIFI_CMD_STATE_HND )
    			{
        			if ( cmd_state & WIFI_CMD_STATE_HND )
        			{
        				cmd_state &= ~WIFI_CMD_STATE_HND;
        				rcv_pending = 0;
						// ����� �� �������������� ������� ����� �� �� �����
						if(tmos_sscanf(buf,"+CIPRECVDATA,%u", &available_len) == 1)
						{
							if(size > available_len)
								size = available_len; // ������� �� ��������� � �������������� ������ �� ������

							if(size)
							{
								unsigned int mytime;

								mytime = CURRENT_TASK->time;
								if( rcv_hnd.tsk_read_pkt(sock->dst.as_voidptr, size, WIFI_READ_TOT) == RES_OK)
								{
									size -= rcv_hnd.len;
									TRACE_WIFI_DEBUG("(%d)\r\n", size);
									sock->dst.as_byteptr += size;
									sock->len -= size;
									if(received_size[sock->sock_id] >= size)
										received_size[sock->sock_id] -= size;
									else
										received_size[sock->sock_id] = 0;
									sock->res |= FLG_OK;

								}
								CURRENT_TASK->time = mytime;
							}
							else
								received_size[sock->sock_id] = 0;

							rcv_hnd.tsk_start_read(&received_ch, 1);

						}
						else
						{
							TRACE1_WIFI_ERROR("\r\nWIFI:Invalid data params STOP!");
							break;
						}

        			} else
        			{
    					if ( cmd_state >= WIFI_CMD_STATE_OK )
    						break; // ��������� � ��������� � OK, ERROR ..
        			}

    			}
				// ���������� �� ���� �������
	    	} while(tsk_resume_wait_signal(rcv_hnd.signal));
	    }



		if (!(cmd_state & WIFI_CMD_STATE_OK))
		{
			break;
		}
	}// ��� ��� �� ������ � ��� ���� ������ ����� �� �� ����� �� ����
	wifi_sleep(120);
	wifi_net_error(NET_ERR_SOCK_READ);
	TRACE_WIFI_ERROR("\r\nWIFI:%s read INVALID", sock->client.task->name);
	return RES_FATAL|FLG_SIGNALED;
}
#endif

RES_CODE esp8266_module::process_cmd(HANDLE client)
{
	RES_CODE res = RES_SIG_ERROR;


	if( !(drv_info->drv_data->wifi_flags_bad & WIFI_FLAG_ON))
	{
		switch (client->cmd)
		{
		case SOCK_CMD_CLOSE:
			res = wifi_sock_close((CSocket*) client);
			break;

		default:
#if WIFI_FLOW_CONTROL
			if(!stop_rcv)
				res = wifi_module_type::process_cmd(client);
			else
			{

				client->res = RES_BUSY_WAITING;
				client->next = pending_read_data;
				pending_read_data = client;
				TRACELN("Start pending %s", pending_read_data->client.task->name);
				res = RES_IDLE;
			}
#else
			res = wifi_module_type::process_cmd(client);
#endif
			break;
		}
	}
	else
		drv_info->drv_data->wifi_error = NET_ERR_PHY_NOT_READY;
//	if(RES_IDLE != res)
//		hnd_error(client);
	return res;

}

RES_CODE esp8266_module::process_write(CSocket* sock)
{
	unsigned size;//, newsize, id;
//	unsigned int start_size, write_size, sock_state;
	CSTRING cmd;
	const char* snd_pending=nullptr;

#if WIFI_FLOW_CONTROL
	if(stop_rcv)
	{
		sock->res = RES_BUSY_WAITING;
		sock->next = pending_read_data;
		pending_read_data = sock;
		TRACELN("Start write pending %s", pending_read_data->client.task->name);
		return RES_IDLE;
	}
#endif
	while(1)
	{
		size = sock->len;
		if(!size)
		{
		    TRACELN1_WIFI_DEBUG("WIFI: write OK");
			return RES_SIG_OK;
		}
		if(size > 2048)
			size = 2048;
		if(sock->sock_state != SOCKET_CONECTED)
			break;
		if(closed_sockets[sock->sock_id])
		{
			wifi_driver_socket_close(sock->sock_id, NET_ERR_SOCK_DISCONNECT);
			break;
		}

		// Send command
		cmd.format(WIFI_SEND_DATA"=%u,%u", sock->sock_id, size);
	    TRACELN_WIFI_DEBUG("WIFI: WRITE %d?", size);
#if USE_DEPRECATED_AT_CMD
	    cmd_state |= WIFI_CMD_STATE_HND;
	    if(wifi_send_cmd(cmd.c_str(), 20) != WIFI_CMD_STATE_RETURNED)
#else
	    if(wifi_send_cmd(cmd.c_str(), 20) != WIFI_CMD_STATE_OK)
#endif
	    {
	    	wifi_net_error(NET_ERR_SOCK_WRITE);
	    	return RES_SIG_ERROR;
	    }

	    // Send data
		snd_pending = "> ";
#if !USE_DEPRECATED_AT_CMD
		cmd_state = 0;
#endif
		bool timeout = true;
		do
		{
			process_input(rcv_hnd.signal, cmd.c_str(), snd_pending);
			if ( cmd_state >= WIFI_CMD_STATE_HND )
			{
				if ( cmd_state & WIFI_CMD_STATE_HND )
				{
					unsigned int mytime;

					cmd_state &= ~WIFI_CMD_STATE_HND;
					snd_pending = nullptr;

					rcv_hnd.tsk_start_read(&received_ch, 1);
					mytime = CURRENT_TASK->time;
//??????????					tsk_sleep(55); // data sheet recomendation
					if( snd_hnd.tsk_write(sock->src.as_voidptr, size, WIFI_WRITE_TOT) != RES_OK)
						break;
					CURRENT_TASK->time = mytime;

				} else
				{
					if ( cmd_state >= WIFI_CMD_STATE_OK )
					{
						timeout = false;
						break; // command completed with OK, ERROR ..
					}
				}
			}

		} while(tsk_resume_wait_signal(rcv_hnd.signal));

		if(timeout)
		{
			TRACELN1_WIFI_DEBUG("WIFI: send timeout!");
		}
//	    wifi_on_blink_transfer(this, GPRS_TRANSFER_INDICATOR);
		TRACE_WIFI_DEBUG("\r\nWIFI:state %X", cmd_state);
	    //Check the result
	    if(cmd_state & WIFI_CMD_STATE_OK)
	    {
			TRACE1_WIFI_DEBUG(" done!");
			sock->src.as_byteptr += size;
			sock->len -= size;
			continue;
		}

	    if (cmd_state & WIFI_CMD_STATE_CMES)
	    {
	    	TRACE_ERROR("\r\nWIFI:%s write ERROR", sock->client.task->name);
	    }

    	break;
	}
	wifi_sleep(120);
	wifi_net_error(NET_ERR_SOCK_WRITE);
	return RES_SIG_ERROR;
}

RES_CODE esp8266_module::wifi_sock_connect_adr(CSocket* sock)
{
	return wifi_sock_connect_url(sock);
}

RES_CODE esp8266_module::wifi_sock_connect_url(CSocket* sock)
{
	CSTRING cmd;

	for(int try_cnt=0; sock && try_cnt <3; try_cnt++ )
	{
		if(sock->sock_state & SOCKET_OPEN)
		{
			unsigned int sid = sock->sock_id;
			cmd.format(WIFI_CONNECT"=%u,\"%s\",\"%s\",%u", sid,
					(((sock_mode_t*)sock->mode.as_voidptr)->sock_type == IP_SOCKET_TCP)?"TCP":"UDP",
					sock->src.as_charptr,
					sock->dst.as_int);
			pending_connection = sock;
			if (wifi_send_cmd(cmd.c_str(), 40) & WIFI_CMD_STATE_OK)
			{
				for(int i=0; i<500; i++)
				{
					wifi_sleep(10);
					if(sock->sock_state == SOCKET_CONECTED)
						break;
				}
				pending_connection = nullptr;
/*
				if(received_data[sid])
				{
					delete received_data[sid];
					received_data[sid] = nullptr;
				}
				DEC_ALLOC_SIZE(received_size[sid]);
				received_size[sid] = 0;
*/
				if(sock->sock_state == SOCKET_CONECTED)
				{
					TRACE_WIFI_DEBUG("\r\nLink %d", sid);
					return RES_SIG_OK;
				}
				wifi_esp8266_socket_close(sid);
			}
			pending_connection = nullptr;

			//Debug
			get_socket_state(sid);
		} else
		{
			wifi_net_error(NET_ERR_SOCK_CLOSED);
			return RES_SIG_ERROR;
		}
	}
	wifi_net_error(NET_ERR_SOCK_CONNECT);
	return RES_SIG_ERROR;
}

RES_CODE esp8266_module::wifi_sock_disconect(CSocket* sock)
{
	uint32_t sid;
	if(sock && sock->sock_id < WIFI_ESP8266_MAX_SOCKETS)
	{
		sid =sock->sock_id;
		if(alloc_sockets[sid] == sock && sock->sock_state == SOCKET_CONECTED)
		{
			wifi_esp8266_socket_close(sid);
			if(!get_socket_state(sid))
				return RES_SIG_OK;
		}
	}
	return RES_SIG_ERROR;
}

RES_CODE esp8266_module::wifi_gethostbyname(CSocket* sock)
{
	return RES_OK;
}



RES_CODE esp8266_module::wifi_sock_bind_adr(CSocket* sock)
{
	if(listen_socket && sock == listen_socket)
	{
		listen_port = sock->dst.as_int;
		return RES_SIG_OK;
	}
	return RES_SIG_ERROR;
}

RES_CODE esp8266_module::wifi_sock_bind_url(CSocket* sock)
{
	return RES_SIG_ERROR;
}

//esptool.py -p /dev/ttyUSB0 --no-stub write_flash --flash_size 2MB-c1 0x0 img/boot_v1.7.bin 0x01000 img/user1.2048.new.5.bin 0x1fb000 img/blank.bin 0x1fc000 img/esp_init_data_default_v08.bin 0xfe000 img/blank.bin 0x1fe000 img/blank.bin
RES_CODE esp8266_module::wifi_sock_listen(CSocket* sock)
{
	CSTRING cmd;
	if(listen_socket && sock == listen_socket)
	{

//		cmd.format("+CIPSERVERMAXCONN=%u", sock->src.as_int);
//		if(wifi_send_cmd(cmd.c_str(), 5) == WIFI_CMD_STATE_OK)
//		{
			cmd.format(WIFI_SERVER"=1,%u", listen_port);
			if(wifi_send_cmd(cmd.c_str(), 5) == WIFI_CMD_STATE_OK)
			{
				wifi_send_cmd(WIFI_SRVER_TIMEOUT"=1800", 5); // 30 min timeout
				sock->sock_state = SOCKET_LISTEN;
				return RES_SIG_OK;
			}
//		}
	}
	else
		wifi_net_error(NET_ERR_SOCK_CONNECT);
	return RES_SIG_ERROR;
}

RES_CODE esp8266_module::wifi_sock_accept(CSocket* sock)
{
	if(listen_socket)
	{
		listen_socket->res = RES_BUSY_WAITING;
		get_socket_state(10);
		if(listen_socket->res & FLG_SIGNALED)
		{	// module_accepted_socket(id) has already accepted a connection
			// and signaled the handler
			return (listen_socket->res ^ FLG_SIGNALED);
		}
		listen_socket->res = RES_BUSY_WAITING;
		return RES_IDLE;
	}
	return RES_SIG_ERROR;
}

RES_CODE esp8266_module::wifi_sock_addr(CSocket* sock)
{
	return RES_SIG_ERROR;
}

RES_CODE esp8266_module::module_upgrade(HANDLE hnd)
{
	char gsm_ack[32];
	unsigned int sig, old_sig;//, total, tm, last;
	HND_CLIENTS old_client;
	RES_CODE res;
//	CSTRING log;
	CHandle hnd_snd;
	uint32_t buf_in, buf_out, rx_size=0, tx_size=0, gsm_ack_off;
	DRIVER_INDEX	iface_driver_index;

    //change hnd client
    old_sig = hnd->signal;
    old_client = hnd->client;
    sig = CURRENT_TASK->aloc_sig;
    sig = (sig+1) & ~sig;
    sig &= 255;
    if( sig )
    {
    	CURRENT_TASK->aloc_sig |= sig;
    	hnd->signal = sig;
	    hnd->client.task = CURRENT_TASK;
		wifi_drv_off();

	    hnd->tsk_write("\r\n----- READY -----\r\n");
	    if(wifi_pin_boot)
	    {
	    	PIO_Assert(wifi_pin_boot);
	    	tsk_sleep(100);
	    }
		wifi_drv_pwron(true);

		res = hnd->tsk_read(buf, 1, 2*25000);
		wifi_drv_pwron(true);

		iface_driver_index = drv_info->drv_data->iface_driver_index;
		if(res == RES_OK && !iface_driver_index)
			res = RES_FATAL;
		if(res == RES_OK)
		{
			// upgrade started


			if(rcv_hnd.tsk_open(iface_driver_index, drv_info->iface_mode_stru[1]) &&
				snd_hnd.tsk_open(iface_driver_index, drv_info->iface_mode_stru[1])	)
			{
		    	unsigned int pc_rcv, gsm_rcv;

			    TRACE1_WIFI("\r\n------ WIFI UPGRADE --------");

			    res = hnd_snd.tsk_open(hnd->drv_index, hnd->mode.as_voidptr);
			    //flush PC
			    buf_in = 20;
			    gsm_ack_off = 0;
			    while(--buf_in)
			    {
				    hnd->tsk_read(buf, WIFI_BUF_SIZE, 2000);
			    	pc_rcv = WIFI_BUF_SIZE - hnd->len;
			    	if(pc_rcv)
			    	{
			    		TRACE_BUF(buf, pc_rcv, TC_TXT_YELLOW);
			    		buf_in = pc_rcv;
			    		break;
			    	}
			    }


			    buf_out = 0;
			    while(1)
			    {

			    	//start reading from PC
			    	if(hnd->res <  FLG_SIGNALED)
			    	{
			    		if(buf_in >= buf_out)
			    		{
			    			rx_size = WIFI_BUF_SIZE - buf_in;
				    		if(rx_size > 1 || buf_out == 0)
				    			rx_size /= 2;

			    		}
			    		else
			    		{
			    			rx_size = buf_out - buf_in;
			    			rx_size /= 2;
			    		}
			    		if(rx_size)
			    		{
							if (rx_size > 16)
								rx_size = 16;
							if (!hnd->tsk_start_read(buf + buf_in, rx_size))
								break;
							TRACELN("rx st %u %u", buf_in, rx_size);
			    		}
			    	}

			    	// start sending to gsm
			    	if(buf_in != buf_out && (snd_hnd.res < FLG_SIGNALED))
			    	{
			    		if(buf_out > buf_in)
			    			tx_size = WIFI_BUF_SIZE - buf_out;
			    		else
			    			tx_size = buf_in - buf_out;
			    		if (snd_hnd.tsk_start_write(buf+buf_out, tx_size) != RES_OK)
			    		{
			    			TRACELN1("TIMEOUT\r\n");
			    		}
						TRACELN("tx st %u %u", buf_out, tx_size);
			    	}

			    	//start reading from GSM
			    	if(rcv_hnd.res < FLG_SIGNALED)
						if(!rcv_hnd.tsk_start_read(gsm_ack+gsm_ack_off, sizeof(gsm_ack)/2))
								break;

			    	sig = tsk_wait_signal(hnd->signal | rcv_hnd.signal | snd_hnd.signal, 2*25000);

			    	// process send
			    	if(sig & snd_hnd.signal)
			    	{
			    		snd_hnd.res &= ~FLG_SIGNALED;
				    	pc_rcv = tx_size - snd_hnd.len;
						TRACELN("tx %u %u", buf_out, pc_rcv);
		    			buf_out += pc_rcv;
		    			if(buf_out >= WIFI_BUF_SIZE)
		    				buf_out = 0;
			    	}

			    	//process PC
			    	if(sig & hnd->signal)
			    	{
			    		hnd->res &= ~FLG_SIGNALED;
				    	pc_rcv = rx_size - hnd->len;
						TRACE_BUF(buf+buf_in, pc_rcv, TC_TXT_RED);

		    			buf_in += pc_rcv;
		    			if(buf_in >= WIFI_BUF_SIZE)
		    				buf_in = 0;
			    	}

			    	//process GSM
			    	if(sig & rcv_hnd.signal)
			    	{
			    		rcv_hnd.res &= ~FLG_SIGNALED;
				    	gsm_rcv = sizeof(gsm_ack)/2 - rcv_hnd.len;
				    	if(gsm_rcv)
				    	{
				    		hnd_snd.tsk_start_write(gsm_ack+gsm_ack_off, gsm_rcv);
				    		TRACE_BUF(gsm_ack+gsm_ack_off, gsm_rcv, TC_TXT_CYAN);
				    		if(gsm_ack_off)
				    			gsm_ack_off =0;
				    		else
				    			gsm_ack_off = sizeof(gsm_ack)/2;
				    	}
			    	}
			    	if(!sig)
			    	{
			    		hnd->tsk_cancel();
			    		rcv_hnd.tsk_cancel();
			    		break;
			    	}
			    }

			    TRACE1_WIFI("\r\n------ UPGRADE END --------");
			    if(wifi_pin_boot)
			    {
				    wifi_drv_off();
			    	PIO_Deassert(wifi_pin_boot);
			    	tsk_sleep(100);
				    wifi_drv_pwron();
			    }
			}
		} else
			TRACE1_WIFI("\r\nTIMEOUT\r\n");

	    //restore hnd owner
    	CURRENT_TASK->aloc_sig &= ~hnd->signal;
    	hnd->signal = old_sig;
	    hnd->client = old_client;
	} else
	{
		res = RES_ERROR;
	}

	return res;
}

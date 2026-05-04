/*
 * wifi_module.cpp
 *
 *  Created on: Feb 1, 2012
 *      Author: miro
 */

#include <tmos.h>
#include <hardware_cpp.h>
#include <wifi_drv.h>
#include <wifi_core.h>
#include <csocket.h>

//uint32_t wifi_access_point_t::ap_cnt = 0;
const char* mdns_desc_t::protocol_type[2] = {"_tcp", "_udp"};
mdns_text_t mdns_desc_t::nil;

WEAK_C NET_CODE wifi_on_init_station(wifi_module_type* mod
									,CSocket* sock
									,wifi_access_point_t* network)
{
	return NET_OK;
}

WEAK_C NET_CODE wifi_on_get_AP(	wifi_module_type* mod
								,CSocket* sock
								,wifi_access_point_t* network)
{
	return NET_ERR_WIFI_NET_NAME_PWD;
}

WEAK_C void wifi_on_disconnect(wifi_module_type* mod)
{

}

WEAK_C void wifi_on_blink_transfer(wifi_module_type* mod, int reason)
{

}

WEAK_C NET_CODE wifi_on_register(wifi_module_type* mod)
{
	return NET_OK;
}

WEAK_C NET_CODE wifi_on_deregister(wifi_module_type* mod)
{
	return NET_OK;
}

WEAK_C mdns_desc_t* wifi_get_mdns_desc(wifi_module_type* mod)
{

	mdns_desc_t *mdns_desc = new user_mdns_desc_t<3>;


	if (mdns_desc) {
		mdns_desc->hostname = "shell";
		mdns_desc->instance = "Shell_Terminal";
		mdns_desc->service = "_telnet";//"_shell";
		mdns_desc->port =23;
		mdns_desc->set_text(0, "vendor", "espresiif");
		mdns_desc->set_text(1, "type", "ESP8266");
		mdns_desc->set_text(2, "sn", "123456");
	}
	return mdns_desc;
}

CSTRING wifi_ip_to_str(const ip_addr_t* adr, bool quotes)
{
	CSTRING ip;
	const char *p;
	if (adr) {
		p = (const char*)adr;
		if (quotes)
			ip = '"';
		ip.appendf("%u.%u.%u.%u", p[0], p[1], p[2], p[3]);
		if (quotes)
			ip += '"';
	}
	return ip;
}

const char *skip_cmd_echo(const char* src)
{
	if (src && *src == '+') {
		src++;
		while (*src && !IS_SPACE(*src) && *src != ':')
			src++;
		while ( *src && ( IS_SPACE(*src) || *src == ':' || *src =='(' ) )
			src++;
	}
	return src;
}

bool wifi_get_param(const char*row, CSTRING& param, unsigned int num)
{
	const char* end;

	if(row && IS_NOT_NULL(&param))
	{
		param.clear();
		row = skip_cmd_echo(row);
		while(num > 1)
		{
			if (*row == CHAR_STX) {
				row = strchr(row, CHAR_ETX);
				if (!row)
					return false;
				row++;
			}
			row=strchr(row,',');
			if(!row)
				return false;
			row++;
			num--;
		}
		if (*row)
		{
			while(*row && IS_SPACE(*row))
				row++;
			if (*row == '"' || *row == CHAR_STX) {
				if ( *row == '"') {
					end = strchr(++row, '"');
				} else {
					end = strchr(++row, CHAR_ETX);
				}
				if(!end)
					return false;
				if (end > row) {
					param.assign(row, end-row);
				} else {
					param = "";
				}
				return true;
			}
			end = strchr(row, ',');
			if (end) {
				if (end > row) {
					param.assign(row, end-row);
				} else {
					param = "";
				}
			} else {
				param=row;
			}
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

bool wifi_get_param(const char*row, int8_t& param, unsigned int num)
{
	CSTRING str;
	if(wifi_get_param(row, str, num))
	{
		int val;
		if (tmos_sscanf(str.c_str(), "%d", &val) == 1) {
			if ( val < -128) {
				val = -128;
			} else if (val > 127) {
				val = 127;
			}
			param = val;
			return true;
		}
	}
	return false;
}

bool wifi_get_param(const char*row, wifi_encryption_type_t& param, unsigned int num)
{
	int8_t enc;
	if(wifi_get_param(row, enc, num) && enc >= enc_unknown && enc < enc_last_supported)
	{
		param = (wifi_encryption_type_t)enc;
		return true;
	}
	return false;
}

uint8_t wifi_rssi_to_level(const int8_t rssi )
{
	// rssi in dBm
    if (rssi < -105) return 0;
    else if (rssi < -93) return 1;
    else if (rssi < -81) return 2;
    else if (rssi < -69) return 3;
    else if (rssi < -57) return 4;
    else return 5;
}

// CMEE*
// CMEE
bool wifi_module_type::cmd_match(const char* cmd, const char* row)
{
	bool res = false;
	while(*cmd && (*cmd == *row))
	{
		res = true;
		cmd++;
		row++;
	}
	return res && !IS_ALPHANUM(*cmd);
}

/**
 * Return true if row is starting with the cmd
 * @param cmd
 * @param row
 * @return
 */
bool wifi_module_type::cmd_submatch(const char* cmd, const char* row)
{
	while(*row && (*cmd == *row))
    {
        cmd++;
        row++;
    }
    return (!*cmd);
}


WIFI_CMD_STATE wifi_module_type::wifi_process_row(const char *cmd)
{
	const char * row;

	row = buf + row_start;

	//------- command matching --------//
	if(cmd)
	{
		// OK
		if(find_in_list(row, SZ(OK) SZ(no change) SZ(SEND OK), NULL ))
			return WIFI_CMD_STATE_OK;

		// ERROR
		if(find_in_list(row, SZ(ERROR) SZ(+CME ERROR) SZ(+CMS ERROR) SZ(FAIL) SZ(ready), NULL ))
			return WIFI_CMD_STATE_CMES;

		if( cmd[0] && cmd_match(cmd, row))
		{
			//the row is response to the cmd
			row_start = row_end;
			return WIFI_CMD_STATE_RETURNED;
		}

		// check for echo
		if(row[0]=='A' && row[1]=='T' && !strcmp(cmd, row+2))
		{
			TRACE1_WIFI_DEBUG("\e[4;1;33m ");
			TRACE1_WIFI_DEBUG(row);
			TRACE1_WIFI_DEBUG(" \e[m");
		    return WIFI_CMD_STATE_UNK;
		}
	}

	//------------ WIFI notification -------------//
	if(wifi_notification(row))
		return 0;

	//------- command related --------//
	if(cmd)
	{
		if( (buf[row_start] != '+') || (cmd_state & WIFI_CMD_STATE_ROW_STOP))
		{
			//the row is response to the cmd
			row_start = row_end;
			return WIFI_CMD_STATE_RETURNED;
		}

	}
//	TRACE1_WIFI_DEBUG("^UNK[");
//	TRACE1_WIFI_DEBUG(row);
//	TRACE1_WIFI_DEBUG("]UNK^");
    return WIFI_CMD_STATE_UNK;
}

WIFI_CMD_STATE wifi_module_type::wifi_send_cmd(const char *cmd, unsigned int time)
{
	unsigned int sig;

	// make sure the handle is working if it is open
	while (rcv_hnd.res < FLG_BUSY) {
#if WIFI_FLOW_CONTROL
		if (stop_rcv) {
			cmd_state = WIFI_CMD_STATE_FATAL;
			return cmd_state;
		}
#endif
		process_input(0, NULL);
	}

    // make sure no URC is coming and the buf is empty
	if (cmd_state & WIFI_CMD_STATE_STARTED) {
		if (tsk_wait_signal(rcv_hnd.signal, 8192)) {
			do {
				process_input(rcv_hnd.signal, NULL);

			} while ((cmd_state & WIFI_CMD_STATE_STARTED) && tsk_resume_wait_signal(rcv_hnd.signal));
		}
	}

    //trace
    TRACE1_WIFI_DEBUG("\r\n\e[32mAT");
    TRACE1_WIFI_DEBUG(cmd);
    TRACE1_WIFI_DEBUG("\e[m ");

    //start clean
    cmd_state &= (WIFI_CMD_STATE_ROW_STOP | WIFI_CMD_STATE_HND);
   	row_start = row_end = 0;

    if(snd_hnd.tsk_write("AT", 2, WIFI_WRITE_TOT) == RES_OK)
    {
        if(snd_hnd.tsk_write(cmd, strlen(cmd), WIFI_WRITE_TOT) == RES_OK)
        {
            if(snd_hnd.tsk_write("\r\n", 2, WIFI_WRITE_TOT) == RES_OK)
            {
        		sig = tsk_wait_signal( rcv_hnd.signal, time << 10);
        		if(sig && (cmd_state & WIFI_CMD_STATE_HND))
        		{
        			cmd_state = 0;
        			return WIFI_CMD_STATE_RETURNED;
        		}

            	while(sig)
            	{
            		process_input(sig, cmd); // NORMAL AT COMMAND
        			if ( cmd_state > WIFI_CMD_STATE_RETURN )
        			{
                    	row_start = row_end = 0;
        				return cmd_state & WIFI_CMD_SEND_STATUS;
        			}
        			sig = tsk_resume_wait_signal(rcv_hnd.signal);
            	}
            	row_start = row_end = 0;
            	if(wifi_watchdog_cnt)
            		wifi_watchdog_cnt--;
            }
        }
    }
    cmd_state = WIFI_CMD_STATE_FATAL;
	return cmd_state;
}

void wifi_module_type::wifi_sleep(unsigned int time)
{
    while(rcv_hnd.res < FLG_BUSY)
    {
		process_input(0, NULL);
#if WIFI_FLOW_CONTROL
		if(stop_rcv)
			return;
#endif
    }
    if (tsk_wait_signal(rcv_hnd.signal, time))
    {
		do
		{
			process_input(rcv_hnd.signal, NULL); // UNSOLICITED

		} while (tsk_resume_wait_signal(rcv_hnd.signal));
    }
}

char* wifi_module_type::get_str_cmd(const char *cmd, unsigned int time)
{

	if(wifi_send_cmd(cmd, time) == WIFI_CMD_STATE_ROK)
	{
		char* ptr = strchr(buf, '"');
		if(ptr)
		{
			char *end = strchr(++ptr, '"');
			if(end)
			{
				*end = 0;
				return ptr;
			}
		}
	}
	return NULL;
}

char* wifi_module_type::get_str_prm(char *row, unsigned int param)
{
	char* end;

	while (param >1)
	{
		row = strchr(row, '"');
		if(!row)
			return NULL;
		row = strchr(row +1, '"');
		if(!row)
			return NULL;
		row++;
		param--;
	}

	row = strchr(row, '"');
	if(!row)
		return NULL;
	end = strchr(++row, '"');
	if(!end)
		return NULL;
	*end =0;
	return row;
}

RES_CODE wifi_module_type::wifi_error(int err)
{
	drv_info->drv_data->wifi_error = err;
	return RES_ERROR;
}

NET_CODE wifi_module_type::wifi_net_error(int err)
{
	drv_info->drv_data->wifi_error = err;
	return err;
}

void wifi_module_type::hnd_error(HANDLE hnd) const
{
	hnd->error = drv_info->drv_data->wifi_error;
}


NET_CODE wifi_module_type::wifi_get_current_net_ssid(wifi_access_point_t& access_point, bool check_only)
{
	return wifi_net_error(NET_ERR_WIFI_NET_NAME_PWD);
}


NET_CODE wifi_module_type::wifi_drv_on()
{
	NET_CODE res;
	WIFI_DRIVER_DATA * drv_data = drv_info->drv_data;
    drv_data->turnoff_time = 0;

/*
	if(drv_data->wifi_flags_ok & WIFI_FLAG_REGISTERED )
		return NET_OK;
*/

	if( !(drv_data->wifi_flags_ok & WIFI_FLAG_ON))
	{
		if(RES_OK != wifi_drv_pwron())
			return wifi_net_error(NET_ERR_PHY_NOT_READY);

	}

#if WIFI_NETWORK_NAME
	drv_data->network_name = "registering";
#endif


	// ---- 2. Check Network registration
	res = wifi_check_reg();
	if(res != NET_OK)
	{
//		drv_data->wifi_flags_bad |= WIFI_FLAG_REGISTERED;
		return res;
	}


#if WIFI_NETWORK_NAME
	res = wifi_get_network_name(drv_data->network_name);
	if(res != NET_OK)
		return res;
#endif


	// ---- 4. Notify the application and let it make additional checks
	res = wifi_on_register(this);
	if(res != NET_OK)
		return res;

	// ---- 5. WIFI module is ready now. Do some cosmetic


	TRACE1_WIFI_DEBUG("\r\nWIFI on");
//	drv_data->wifi_flags_ok |= WIFI_FLAG_REGISTERED;
//	drv_data->wifi_flags_bad &= ~WIFI_FLAG_REGISTERED;
	return NET_OK;
}



NET_CODE wifi_module_type::wifi_check_reg() // NET_ERROR: OK
{
	NET_CODE res;

#if WIFI_NETWORK_NAME
	drv_data->network_name = "searching";
#endif

	for(unsigned int i=0;i<30;i++)
	{
		res = wifi_drv_level();
		if( res == NET_OK)
			return NET_OK;
		if( res == NET_ERR_WIFI && i > 3 )
			break;
		if(cmd_state & WIFI_CMD_STATE_CMES)
		{
			break;
		}
	}
    return wifi_net_error(res);
}


RES_CODE wifi_module_type::process_cmd(HANDLE client)
{
	RES_CODE res = RES_SIG_ERROR;

	if (!(drv_info->drv_data->wifi_flags_bad & WIFI_FLAG_ON) || (client->cmd == WIFI_DRV_UPGRADE)) {
		switch (client->cmd) {
		case CMD_COMMAND:
			// WIFI Commands
			res = ((WIFI_CBF) client->src.as_voidptr)(this, client);
			break;
		case WIFI_CMD_CONFIG:
			res = wifi_cmd_config((CSocket*) client);
			break;
		case SOCK_CMD_OPEN:
			res = wifi_sock_cmd_open((CSocket*) client);
			break;
#if USE_WIFI_LISTEN
		case SOCK_CMD_BIND_ADR:
			res = wifi_sock_cmd_bind_adr((CSocket*) client);
			break;

		case SOCK_CMD_BIND_URL:
			res = wifi_sock_cmd_bind_url((CSocket*) client);
			break;

		case SOCK_CMD_LISTEN:
			res = wifi_sock_cmd_listen((CSocket*) client);
			break;

		case SOCK_CMD_ACCEPT:
			res = wifi_sock_cmd_accept((CSocket*) client);
			break;

		case SOCK_CMD_GET_ADDR:
			res = wifi_sock_cmd_addr((CSocket*) client);
			break;
#endif
		case SOCK_CMD_CONNECT_ADR:
			res = wifi_sock_cmd_connect_adr((CSocket*) client);
			break;

		case SOCK_CMD_CONNECT_URL:
			res = wifi_sock_cmd_connect_url((CSocket*) client);
			break;

		case SOCK_CMD_DISCONNECT:
			res = wifi_sock_cmd_disconect((CSocket*) client);
			break;

		case SOCK_CMD_CLOSE:
			res = wifi_sock_cmd_close((CSocket*) client);
			break;

		case WIFI_DRV_UPGRADE:
			module_upgrade((HANDLE) client->dst.as_voidptr);
			res = RES_SIG_OK;
			break;

		case WIFI_DRV_ON_CMD:
			res = wifi_drv_on() | FLG_SIGNALED;
			break;

		case WIFI_DRV_OFF_CMD:
			res = wifi_drv_off() | FLG_SIGNALED;
			break;

		default:
			// socket commands
			break;
		}
	} else
		drv_info->drv_data->wifi_error = NET_ERR_PHY_NOT_READY;
	return res;
}

void wifi_module_type::wifi_process_tout()
{
}

int wifi_module_type::wifi_notification(const char* row)
{
	return 0; // we do not recognize this row
}

void wifi_module_type::wifi_cancelation(bool all_station, bool all_softAP)
{
}



//*----------------------------------------------------------------------------
//*			WIFI_DRV_OFF_CMD / WIFI_DRV_ON_CMD
//*----------------------------------------------------------------------------

//RES_CODE wifi_drv_off(wifi_module_type *module, HANDLE hnd)
//{
//	return (module->wifi_drv_off()|FLG_SIGNALED);
//}
//
//RES_CODE wifi_drv_on(wifi_module_type *module, HANDLE hnd)
//{
//	return (module->wifi_drv_on()|FLG_SIGNALED);
//}

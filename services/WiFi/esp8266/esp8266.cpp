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
#include <memory.h>
/*
 *
 * AT version:1.7.5.0(Oct  9 2021 09:26:04)
 * SDK version:3.0.5(b29dcd3)
 * compile time:Oct 15 2021 18:05:38
 * Bin version(Wroom 02):1.7.5
 *
 * AT version:2.3.0.0(s-2522d50 - ESP8266 - Aug 13 2025 08:41:23)
 * SDK version:v3.4-112-gc965e03d
 * compile time(795c42d8):Apr 18 2026 20:11:52
 * Bin version:2.3.0.1(WROOM-02-N)
 *
 */

#define SOCKET_WAIT_ACCEPT	(0x10 + SOCKET_LISTEN)

#if WIFI_FLOW_CONTROL
#define INC_ALLOC_SIZE(size) mem_alloc_size += (size)
#define DEC_ALLOC_SIZE(size) mem_alloc_size -= (size)
#else
#define INC_ALLOC_SIZE(size)
#define DEC_ALLOC_SIZE(size)
#endif


/*
 * C ++17
 *
template<typename... Args>
constexpr unsigned _supported_encryption(Args... args) {
    return ((1u << args) | ...);
}
*/

#define ENC_ON(x) (1<<(x))

const unsigned esp8266_supported_encryption =
		ENC_ON(enc_open) |
		ENC_ON(enc_wep_enc) |
		ENC_ON(enc_wpa_psk) |
		ENC_ON(enc_wpa2_psk) |
		ENC_ON(enc_wpa_wpa2_psk) |
		ENC_ON(enc_wpa2_wpa3_psk);

bool is_esp8266_supported_encryption(wifi_encryption_type_t enc)
{
	if (enc > enc_unknown && enc < enc_last_supported) {
		if (esp8266_supported_encryption & ENC_ON(enc) )
			return true;
	}
	return false;
}

WEAK_C void wifi_on_pwron_config(wifi_module_type* mod)
{
	//TODO: application specific initialization
}

WEAK_C void wifi_notify_power(wifi_module_type* mod)
{
	//TODO: Trace various info
}

WEAK_C bool wifi_name_pass(CSTRING& name)
{
	name += "\"ESP8266 NET\"";
//	name_pass += ",\"1234asdf\""; // Set password
	return false; // No psaasword, true if password required
}

/*
 * this function is called by a notification, so do not execute AT commands
 */
WEAK_C void wifi_ap_state_changed(bool connected, wifi_module_type* mod)
{
	// TODO:
}
/*
 * This function is called when the AP is set from the web interface
 */
WEAK_C void wifi_ap_set_from_web(esp8266_ap_t ap_web, wifi_module_type* mod)
{

}
//=============================================================================
//	ESP8266 Access Point
//=============================================================================
bool esp8266_ap_t::from_str(const CSTRING& str_src) {
	unsigned param;
	if (!str_src.empty()) {
		clear();
		const char* src = str_src.c_str();
		if (wifi_get_param(src, ssid, 1)) {
			if (wifi_get_param(src, pwd, 2)) {
				if (wifi_get_param(src, mac, 3)) {
					if (!wifi_get_param(src, ap_enc, 4)) {
						ap_enc = enc_unknown;
					}
					if (!wifi_get_param(src, ap_rssi, 5)) {
						ap_rssi = -128;
					}
					if (wifi_get_param(src, param, 6)) {
						ap_flags = (param & ~WIFI_AP_FLAGS_INFO);
					}
					if (ap_flags & WIFI_AP_FLAG_STATIC_IP) {
						for(size_t i=0; i < sizeof(ip_arr)/sizeof(ip_addr_t); i++) {
							// TODO: IP V6
							if (wifi_get_param(src, param, 7+i))
								ip4_addr_set_u32(ip_arr +i, param);
						}
					}
					return true;
				}
			}
		}
	}
	clear();
	return false;
}

bool esp8266_ap_t::from_module(char* src) {
	uint32_t i;
	char* end;
	char *start;
	if (src == nullptr) {
		return false;
	}
	clear();
	if (!strncmp(src, "+CWLAP:", 7)) {
		// +CWLAP:(<ecn>, <"ssid">, <rssi>, <"mac">, [<ch>, <freq offset>, <freq calibration> are not interesting])
		// +CWLAP:(.....)
		// ......
		// +CWLAP:(0,"Note "\a", is not "beep"",-70,"da:44:89:19:b0:69")
		if ( (src=(char*)skip_cmd_echo(src)) && (i=strlen(src))) {
			end = src + i;
			i=0;
			while (IS_DIGIT(*src)) {
				i *= 10;
				i += *src - '0';
				src++;
			}
			if (i<enc_last_supported)
				ap_enc = (wifi_encryption_type_t)i;
			else
				ap_enc = enc_unknown;
			if (src[0] == ',' && src[1] == '"' && src[2]) {
				src += 2;
				i=3; // 3 quotes
				while (end > src && i) {
					if (*--end == '"') {
						i--;
					}
				}
				if (!i && end >= src) {
					if(end == src){
						ssid = "";
					} else {
						ssid.assign(src, end - src);
					}
					if (end[1] == ',') {
						end += 2;
						if (wifi_get_param(end, ap_rssi, 1) && wifi_get_param(end, mac, 2)) {
							pwd.clear();
							return true;
						}
					}
				}
			}
		}
	} else if (!strncmp(src, "+CWJAP", 6)) {
		//+CWJAP_CUR:<ssid>, <bssid-MAC>, <channel>, <rssi>, <?>
		end = strrchr(src, '"');
		start = strchr(src, '"');
		//start->"ssid","mac"<- end
		if ( start && end) {
			*start = CHAR_STX;
			while (*--end != '"' && end > start) {
				;
			}
			if ((end-start) > 2 && *--end == ',' && *--end == '"') {
				*end = CHAR_ETX;
				if (wifi_get_param(src,ssid, 1)) {
					if (wifi_get_param(src, mac, 2)) {
						if (wifi_get_param(src, ap_rssi, 4)) {
							pwd.clear();
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

CSTRING& esp8266_ap_t::to_str(CSTRING& dst) const {
	dst += CHAR_STX;
	dst += ssid;							// 1
	dst += STR_ETX "," STR_STX;
	dst += pwd;								// 2
	dst += STR_ETX ",";
	dst += mac;								// 3
	dst.appendf(",%d,%d,%u", ap_enc, ap_rssi, ap_flags);	// 4, 5, 6
	// TODO: IP V6
	dst.appendf(",%u,%u,%u\r\n", ip, mask, gw); // 7, 8, 9
	return dst;
}

//=============================================================================
//	ESP8266 status related methods
//=============================================================================

NET_CODE esp8266_module::esp8266_xxx_ip(wifi_access_point_t& access_point, bool set, const char* esp_ip_cmd)
{
	NET_CODE res=NET_ERROR;
	uint32_t sig = 0, index=0;
	CSTRING cmd(esp_ip_cmd);

	if (set) {
		cmd += '=';
		if (ip_addr_isany(&access_point.ip)) {
			// IP cannot be 'any'
			return wifi_net_error(NET_ERR_WIFI_INVALID_IP);
		}
		// IP
		cmd += wifi_ip_to_str(&access_point.ip, true);
		// GATEWAY
		if (!ip_addr_isany(&access_point.gw)) {
			cmd += ',';
			cmd += wifi_ip_to_str(&access_point.gw, true);
		}
		// MASK
		if (!ip_addr_isany(&access_point.mask)) {
			if (ip_addr_isany(&access_point.gw))
				cmd += ',';
			cmd += ',';
			cmd += wifi_ip_to_str(&access_point.mask, true);
		}
		if (WIFI_CMD_STATE_OK == wifi_send_cmd(cmd.c_str(), 5))
			res = NET_OK;
	} else {
		cmd += '?';

		cmd_state |= WIFI_CMD_STATE_ROW_STOP;
		res = wifi_send_cmd(cmd.c_str(), 15);
		cmd_state &= ~WIFI_CMD_STATE_ROW_STOP;
		do {
			if (sig) {
				process_input(sig, esp_ip_cmd);
				res = cmd_state;
			}

			if (res >= WIFI_CMD_STATE_OK)
				break;

			if (res & WIFI_CMD_STATE_RETURNED) {
				cmd_state = res & ~WIFI_CMD_STATE_RETURNED;

				if (!strncmp(buf, esp_ip_cmd, strlen(esp_ip_cmd))) {
					char *ptr = strchr(buf, ':');
					if (ptr++) {
						row_start = ptr - buf;
						ptr = strchr(ptr, ':');
						if (ptr && ptr[1] == '"' && IS_DIGIT(ptr[2])) {
							ptr += 2;
							row_end = ptr - buf;
							ptr = strchr(ptr, '"');
							if (ptr) {
								*ptr = 0;
								switch (buf[row_start]) {
								case 'i': // ip
									if (ipaddr_aton(buf + row_end, &access_point.ip))
										index++;
									break;
								case 'g': // gateway
									if (ipaddr_aton(buf + row_end, &access_point.gw))
										index++;
									break;
								case 'n': // netmask
									if (ipaddr_aton(buf + row_end, &access_point.mask))
										index++;
									break;
								}
							}
						}
					}
				}
				row_start = row_end = 0;
			}
			sig = tsk_resume_wait_signal(rcv_hnd.signal);
		} while (sig);

		res = ((cmd_state & WIFI_CMD_STATE_OK) && index == 3)? NET_OK : NET_ERROR;
	}
	return res;
}

FILE_OPTIMIZE("Os")

NET_CODE esp8266_module::esp8266_soft_ap_ip(wifi_access_point_t& access_point, bool set)
{
	// Currently, ESP8266 only supports class C IP addresses.
	return esp8266_xxx_ip(access_point, set, WIFI_SOFT_AP_IP);
}

NET_CODE esp8266_module::esp8266_station_ip(wifi_access_point_t& access_point, bool set)
{
	//Only when the ESP8266 Station is connected to an AP can its IP address be queried.
	return esp8266_xxx_ip(access_point, set, WIFI_STATION_IP);
}

FILE_RESET_OPTIONS
/**
 * Gets the Connection Status
 *
 * @param sock_id - Socket ID whose connection status will be returned.
 * 					When the ID is invalid, all connection states will be returned.
 * @return
 *  31 30 29 28 27 26 25 24   23 22 21 20 19 18 17 16
 * |P |F |x |c4|c3|c2|c1|c0| |x |x |x |l4|l3|l2|l1|l0|
 * |connected                |the connection
 *                           |has a socket
 *
 *  15 14 13 12 11 10  9  8    7  6  5  4  3  2  1  0
 * |x |x |x |s4|s3|s2|s1|s0| |S2|S1|S0|a4|a3|a2|a1|a0|
 * |connection               |accepted
 * |from the server          |connections
 *
 * F - If the socket ID is valid, it means it is connected.
 * P - successfully executed command
 */

unsigned int esp8266_module::esp8266_connections_status(unsigned int sock_id, ip_addr_t* ip_ptr)
{
	RES_CODE res;
	unsigned int sig=0;
	unsigned int link_id;
	unsigned int ret_flags = 0;
	unsigned int link_type;
	CSTRING ip_str;

	if (ip_ptr) {
		if(sock_id == ESP8266_GET_SATUS) {
			for(link_id=0; link_id < WIFI_ESP8266_MAX_SOCKETS; link_id++)
				ip_ptr[link_id] = ip_addr_any;
		} else {
			ip_ptr[0] = ip_addr_any;
		}
	}
	cmd_state |= WIFI_CMD_STATE_ROW_STOP;
	res = wifi_send_cmd(WIFI_CONNECTION_STAT, 15);
	if (res <= WIFI_CMD_STATE_RETURNED) {
		do {
			if (sig) {
				process_input(sig, WIFI_CONNECTION_STAT);
				res = cmd_state;
			}

			if (res >= WIFI_CMD_STATE_OK)
				break;

			if (res & WIFI_CMD_STATE_RETURNED) {
				cmd_state = res & ~WIFI_CMD_STATE_RETURNED;
				if (tmos_sscanf(buf, WIFI_CONNECTION_STAT":%u", &link_id) && link_id < WIFI_ESP8266_MAX_SOCKETS) {
					// +CIPSTATUS:<link ID>,<type>,<remote IP>,<remote port>,<local port>,<tetype>
					if (wifi_get_param(buf, link_type,	WIFI_CONNECTION_TYPE_INDX)) {

						ret_flags |= 1 << (ESP8266_STAT_OFFSET_CONNECTED + link_id);
						//<tetype>:
						// 0: ESP8266 runs as a client.
						// 1: ESP8266 runs as a server.
						if (link_type == 1) {
							// connection from the server
							ret_flags |= 1 << (ESP8266_STAT_OFFSET_FROM_SERVER + link_id);
							if (link_to_socket[link_id] < WIFI_ESP8266_MAX_SOCKETS) {
								// in use
								ret_flags |= 1 << (ESP8266_STAT_OFFSET_ACCEPTED + link_id);
							}
						} else {
							// connection from a client-side
							if (link_to_socket[link_id] < WIFI_ESP8266_MAX_SOCKETS) {
								// in use
								ret_flags |= 1 << (ESP8266_STAT_OFFSET_TO_SOCKET + link_id);
							}
						}
						if (ip_ptr && sock_id == ESP8266_GET_SATUS && wifi_get_param(buf, ip_str, 3)) {
							ipaddr_aton(ip_str.c_str(), &ip_ptr[link_id]);
						}
						// all connections
						if (link_to_socket[link_id] == sock_id) {
							ret_flags |= ESP8266_STATUS_SOCK_CONECTED;
							if (ip_ptr && wifi_get_param(buf, ip_str, 3))
								ipaddr_aton(ip_str.c_str(), ip_ptr);
						}
					}

				} else if (tmos_sscanf(buf, "STATUS:%u", &link_type)) {
					// status of the ESP8266 Station interface.
					// 2: The ESP8266 Station is connected to an AP and its IP is obtained.
					// 3: The ESP8266 Station has created a TCP or UDP transmission.
					// 4: The TCP or UDP transmission of ESP8266 Station is disconnected.
					// 5: The ESP8266 Station does NOT connect to an AP.
					if (link_type == 5) {
						TRACELN1_WIFI_DEBUG("The Station does NOT connect to an AP");
					}
					ret_flags |= (link_type << ESP8266_STAT_OFFSET_AP_STATUS);
				} else {
					// something is wrong
				}
				row_start = 0;
				row_end = 0;
			}

			sig = tsk_resume_wait_signal(rcv_hnd.signal);
		} while (sig);

	}
	cmd_state &= ~WIFI_CMD_STATE_ROW_STOP;
	if ((cmd_state & WIFI_CMD_STATE_OK)) {
		ret_flags |= ESP8266_GET_STATUS_OK;
	}
	return ret_flags;
}

NET_CODE esp8266_module::wifi_get_current_net_ssid(wifi_access_point_t& access_point, bool check_only)
{
#if !USE_DEPRECATED_AT_CMD
	access_point.clear();
	if(wifi_send_cmd(WIFI_JOIN_TO_AP"?",10) & WIFI_CMD_STATE_OK)
	{
		if (!(cmd_state & WIFI_CMD_STATE_RETURNED) && at_version >= RTOS_AT_VERSION )
			strcpy(buf, "No AP");
		//+CWJAP_CUR:<ssid>, <bssid-MAC>, [<channel>, <rssi>, <?> these are not interesting ]
		if(wifi_get_param(buf, access_point.ssid, 1))
		{
			if (access_point.ssid != "No AP") {
				char *end = strrchr(buf, '"');
				char *start = strchr(buf, '"');
				//start->"ssid","mac"<- end
				if ( start && end) {
					*start = CHAR_STX;
					while (*--end != '"' && end > start) {
						//
					}
					if ((end-start) > 2 && *--end == ',' && *--end == '"') {
						*end = CHAR_ETX;
						if (wifi_get_param(buf, access_point.ssid, 1)) {
							if (wifi_get_param(buf, access_point.mac, 2)) {
								if (check_only)
									return NET_OK;
								if(drv_info->drv_data->wifi_flags_ok & WIFI_FLAG_MDNS)
									access_point.ap_flags |= WIFI_AP_FLAG_MDNS;
								if (WIFI_CMD_STATE_ROK == wifi_send_cmd(WIFI_DHCP "?", 5)) {
									// Bit1:                         Bit0:
									// 0: Station DHCP is disabled.  0: SoftAP DHCP is disabled.
									// 1: Station DHCP is enabled.   1: SoftAP DHCP is enabled.
									// RTOS version
									// 0: SoftAP DHCP is disabled.   0: Station DHCP is disabled.
									// 1: SoftAP DHCP is enabled.    1: Station DHCP is enabled.
									// +CWDHCP_CUR:3 (length of '+CWDHCP_CUR:' = 12
									// +CWDHCP:3 (length of '+CWDHC:' = 8

									if (IS_DIGIT(buf[strlen(WIFI_DHCP "?")])) {
										if (at_version >= RTOS_AT_VERSION) {
											if (!(buf[strlen(WIFI_DHCP "?")]&1))
												access_point.ap_flags |= WIFI_AP_FLAG_STATIC_IP;
										} else {
											if (!(buf[strlen(WIFI_DHCP "?")]&2))
												access_point.ap_flags |= WIFI_AP_FLAG_STATIC_IP;
										}
									}
								}
								return esp8266_station_ip(access_point); //NET_OK;
							}
						}
					}
				}
			} else {
				access_point.clear();
				return NET_IDLE;
			}
		}
	}
	return RES_ERROR;
#else
	if (true != connected_network_name.empty())
	{
		access_point.ssid = connected_network_name;
		return NET_OK;
	}
	return NET_IDLE;
#endif
}

//=============================================================================
//	ESP8266 Helpers
//=============================================================================
FILE_OPTIMIZE("Os")

bool esp8266_module::esp8266_socket_to_connection(const unsigned sid, unsigned& link_id ) const
{
	if (sid >= WIFI_ESP8266_MAX_SOCKETS || !alloc_sockets[sid])
		return false;
	if(alloc_sockets[sid]->sock_state == SOCKET_CONECTED) {
		for(link_id =0; link_id < WIFI_ESP8266_MAX_SOCKETS; link_id++) {
			if (link_to_socket[link_id] == sid)
				return true;
		}
	}
	return false;
}

bool esp8266_module::esp8266_esc_syntax(const CSTRING& src, CSTRING& dst)
{
	const char* ptr = src.c_str();
	char ch;
	if (nullptr != ptr ) {
		while ((ch = *ptr++)) {
			if( ch == '\\' || ch == ',' || ch == '"') {
				dst += '\\';
			}
			dst += ch;
		}
		return true;
	}
	return false;
}

NET_CODE esp8266_module::esp8266_mdns(bool mdns_on)
{
	auto_ptr<mdns_desc_t> mdns;
	CSTRING cmd;
	NET_CODE res = RES_ERROR;

	cmd = WIFI_MDNS;
	mdns = nullptr;

	if (mdns_on ) {
		mdns=wifi_get_mdns_desc(this);
		if (!mdns.get()) {
			return res;
		}
		if (at_version >= RTOS_AT_VERSION_V2300) {
			cmd += "EX";
		}
		cmd += "=1,\"";
		cmd += mdns->hostname; cmd += "\",\"";
		cmd += mdns->service;
		cmd.appendf("\",%u", mdns->port);

		if (!mdns->instance.empty() && at_version >= RTOS_AT_VERSION_V2300) {
			cmd += ",\""; cmd += mdns->instance; cmd += '"';
			cmd += ",\""; cmd += mdns->protocol_type[mdns->protocol]; cmd += '"';
			if (mdns->get_txt_num()) {
				cmd.appendf(",%u",mdns->get_txt_num());
				for (unsigned i=0; i < mdns->get_txt_num(); i++) {
					if(!mdns->get_text(i).key.empty() && !mdns->get_text(i).value.empty()) {
						cmd += ",\""; cmd += mdns->get_text(i).key; cmd += '"';
						cmd += ",\""; cmd += mdns->get_text(i).value; cmd += '"';
					}
				}
			}
		}
		if (WIFI_CMD_STATE_ROK == wifi_send_cmd(cmd.c_str(), 10)) {
			drv_info->drv_data->wifi_flags_ok |= WIFI_FLAG_MDNS;
			res = RES_OK;
		}
	} else {
		cmd += "=0";
		if (WIFI_CMD_STATE_OK == wifi_send_cmd(cmd.c_str(), 10)) {
			drv_info->drv_data->wifi_flags_ok &= ~WIFI_FLAG_MDNS;
			res = RES_OK;
		}
	}
	return res;
}

bool esp8266_module::esp8266_get_version()
{
	NET_CODE res;
	unsigned sig =0;
	at_version=0;

	cmd_state |= WIFI_CMD_STATE_ROW_STOP;
	res = wifi_send_cmd("+GMR", 5);
	cmd_state &= ~WIFI_CMD_STATE_ROW_STOP;
	do {
		if (sig) {
			process_input(sig, "+GMR");
			res = cmd_state;
		}

		if (res >= WIFI_CMD_STATE_OK)
			break;

		if (res & WIFI_CMD_STATE_RETURNED) {
			cmd_state = res & ~WIFI_CMD_STATE_RETURNED;
			if (cmd_match("AT version:", buf)) {
				row_start = strlen("AT version:");
				while (IS_DIGIT(buf[row_start])) {
					at_version <<= 4;
					at_version |= buf[row_start++] - '0';
					if (buf[row_start++] != '.') {
						break;
					}

				}
			}
			row_start = row_end = 0;
		}

		sig = tsk_resume_wait_signal(rcv_hnd.signal);
	} while (sig);

	if (cmd_state & WIFI_CMD_STATE_OK) {
		return true;
	}
	return false;

}

FILE_RESET_OPTIONS
//=============================================================================
//	Methods for powering on/off/resetting the ESP8266 and related to its state.
//=============================================================================

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
	CSTRING cmd;


	if(drv_data->wifi_flags_bad & WIFI_FLAG_SHUTDOWN)
		return wifi_error(NET_ERR_WIFI_SHUTDOWN);
	if( drv_data->wifi_flags_ok & WIFI_FLAG_ON)
		return RES_OK;


	TRACE1_WIFI_DEBUG("\r\nWIFI power");
	drv_data->wifi_flags_ok &= (WIFI_FLAG_DETECTED|WIFI_FLAG_PRESENT|WIFI_FLAG_SERVER);
	if(wifi_pin_pwr)
		PIO_Cfg(wifi_pin_pwr);
#if WIFI_FLOW_CONTROL
	if(wifi_pin_rts)
		PIO_Cfg(wifi_pin_rts);
	PIO_Deassert(wifi_pin_rts);
#endif
	PIO_Deassert(wifi_pin_pwr);
	tsk_sleep(1000);
#if ESP8266_TRACE_BOOT
//######################################################################################################################
	DRIVER_INDEX	iface_driver_index;
	iface_driver_index = drv_info->drv_data->iface_driver_index;
	if (iface_driver_index && rcv_hnd.tsk_open(iface_driver_index, drv_info->iface_mode_stru[1])) {
		PIO_Assert(wifi_pin_pwr);
		while (RES_OK == rcv_hnd.tsk_read(buf, WIFI_BUF_SIZE, 100)) {
			TRACE_WIFI_BOOT_BUFF(buf, WIFI_BUF_SIZE - rcv_hnd.len);
		}
	} else {
		PIO_Assert(wifi_pin_pwr);
	}
//######################################################################################################################
#else
	PIO_Assert(wifi_pin_pwr);
#endif
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
#if WIFI_FLOW_CONTROL
		// Enable hardware flow control
		cmd.format(WIFI_BAUD"=%u", *(const uint32_t*)drv_info->iface_mode_stru[0]);
#if !USE_DEPRECATED_AT_CMD
		cmd += ",8,1,0,3";
#endif
		wifi_send_cmd(cmd.c_str(), 2);
#endif
		// info ESP version
		esp8266_get_version();
		if (at_version >= RTOS_AT_VERSION && wifi_send_cmd("+SYSSTORE?", 2) == WIFI_CMD_STATE_ROK) {
			if (wifi_get_param(buf,res, 1) && res == 1) {
				wifi_send_cmd("+SYSSTORE=0", 5);
/*
				wifi_send_cmd("+RESTORE", 30);
				wifi_send_cmd("+RST", 30);
*/
			}
			if (drv_info->iface_mode_stru[2]) {
				// Trying to increase speed to 460800
				changed = false;
				for (res=2; res; --res) {
					cmd.format(WIFI_BAUD"=%u", *(const uint32_t*)drv_info->iface_mode_stru[res]);
	#if !USE_DEPRECATED_AT_CMD
					cmd += ",8,1,0,0";
	#endif
					if (wifi_send_cmd(cmd.c_str(), 2) & WIFI_CMD_STATE_OK) {
						if(RES_OK == wifi_echo_off(false, res)) {
							changed = true;
							break;
						}
					}
				}
				if(!changed) {
					drv_data->wifi_error = NET_ERR_WIFI_ON;
					drv_data->wifi_flags_bad |= WIFI_FLAG_ON;
					return wifi_error(NET_ERR_WIFI);
				}
			}
		}
	}

	// configure WiFi module
	wifi_send_cmd(WIFI_QUIT_AP, 3);
	// 1. Enable the module to act as “Station”
	// 2. AP mode
	// 3  AP + Station mode
#if	ESP8266_SOFT_AP
	res = wifi_send_cmd(WIFI_MODE"=3", 5); // AP + Station mode
#else
	res = wifi_send_cmd(WIFI_MODE"=1", 5); // Station mode
#endif
	if(WIFI_CMD_STATE_OK == res)
	{
		// enable use of router DNS server
		if (at_version >= RTOS_AT_VERSION) {
			/*	When <enable> is set to 1, the DNS server may change according to the configuration of the router which the
				ESP device is connected to. */
			wifi_send_cmd("+CIPDNS=1",2);

		} else {
			/*	For command: AT+CIPDNS_CUR=0 (disable to use user-defined DNS servers), "208.67.222.222" will
				be used as DNS server by default. And the DNS server may change according to the configuration
				of the router which the chip connected to. */
			wifi_send_cmd("+CIPDNS_CUR=0",2);
		}
#if USE_WIFI_ESP8266 >= 3
		// 1: passive mode - ESP8266 will keep the received TCP data in an internal buffer (default is 2920
		// bytes), and wait for host MCU to read the data. If the buffer is full, the TCP transmission will be
		// blocked.
		res = wifi_send_cmd("+CIPRECVMODE=1", 2);
		if(WIFI_CMD_STATE_OK == res)
#endif
		// 2.Enable multiple connections or not (1-multiple/ 0- Single
		//	Note: 	This mode can only be changed after all connections are
		//	 		disconnected. If server is started, reboot is required.
		res = wifi_send_cmd(WFI_MUX"=1", 50);
		if(WIFI_CMD_STATE_OK == res)
		{
			// configuring the format of the list of available access points
			// CWLAP:<ecn>, <ssid>, <rssi>, <mac>
			res =wifi_send_cmd(WIFI_SET_LIST_OPTION"=1,15", 50);
			if(WIFI_CMD_STATE_OK == res)
			{
#if ESP8266_SOFT_AP
				// configure soft AP
				// channel - 6
				CSTRING ssid(WIFI_SOFT_AP_CFG"=");
				if(wifi_name_pass(ssid))
					ssid += ",6,3";	// WPA2_PSK
				else
					ssid += ",\"\",6,0"; // OPEN
				if (at_version >= RTOS_AT_VERSION ) {
					// max connections :8, hidden: false
					ssid += ",2,0";
				}
				res = wifi_send_cmd(ssid.c_str(), 30);
#endif
				if(WIFI_CMD_STATE_OK == res)
				{
					wifi_send_cmd(WIFI_SOFT_AP_CFG"?", 5); // debug only
					wifi_on_pwron_config(this);
					drv_data->wifi_flags_ok |= WIFI_FLAG_ON;
					drv_data->wifi_flags_bad &= ~WIFI_FLAG_ON;
					wifi_notify_power(this);
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
	for (unsigned int sid=0; sid < WIFI_ESP8266_MAX_SOCKETS; sid++) {
		esp8266_socket_close(sid);
		wifi_driver_socket_close(sid, NET_ERR_SOCK_ABORT); // wifi_drv_off
	}
	//TODO: try to stop
	//check power state
	wifi_notify_power(this);
	drv_data->signal_level = 0;
	PIO_Deassert(wifi_pin_pwr);
    TRACE1_WIFI_DEBUG("\r\nWIFI off");
    rcv_hnd.close();
    snd_hnd.close();
	return RES_OK;
}

NET_CODE esp8266_module::wifi_reset(bool force, wifi_module_type** drv_module)
{
	WIFI_DRIVER_DATA *drv_data = drv_info->drv_data;
	if( (drv_data->wifi_flags_ok & WIFI_FLAG_ON ) && !(drv_data->wifi_flags_bad & WIFI_FLAG_SHUTDOWN))
    {
		if(force || /*used_sockets ||*/ seconds_since(last_timeout) > MODULE_SLEEP_TOUT)
        {
       		last_timeout = CURRENT_TIME;
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
        		last_timeout = CURRENT_TIME + (MODULE_SLEEP_TOUT*1024); //do not sleep
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
	esp8266_ap_t ap;

	if( (drv_data->wifi_flags_ok & (WIFI_FLAG_ON|WIFI_FLAG_REGISTERED))
			== (WIFI_FLAG_ON|WIFI_FLAG_REGISTERED))
	{
		if(wifi_send_cmd(WIFI_JOIN_TO_AP"?",10) == WIFI_CMD_STATE_ROK)
		{
			//+CWJAP_CUR:<ssid>, <bssid>, <channel>, <rssi>, <?>
			if (ap.from_module(buf)) {
				level =	wifi_rssi_to_level(ap.ap_rssi);
			}
			res = RES_OK;
		}
	} else {
		level = 0;
		res = RES_OK;
	}
	return res;
}

//=============================================================================
//	CONFIG commands
//=============================================================================

NET_CODE esp8266_module::esp8266_get_available_ap(CSTRING& list)
{
	NET_CODE res;
	unsigned sig =0;
	esp8266_ap_t ap;

	cmd_state |= WIFI_CMD_STATE_ROW_STOP;
	res = wifi_send_cmd(WIFI_LIST_AVAILABLE_AP, 60); // Wait 60 seconds to ensure the command completes.
	cmd_state &= ~WIFI_CMD_STATE_ROW_STOP;
	do {
		if (sig) {
			process_input(sig, WIFI_LIST_AVAILABLE_AP);
			res = cmd_state;
		}

		if (res >= WIFI_CMD_STATE_OK)
			break;

		if (res & WIFI_CMD_STATE_RETURNED) {
			cmd_state = res & ~WIFI_CMD_STATE_RETURNED;
			if (ap.from_module(buf)) {
				ap.to_str(list);
			} else {
				// It’s not great, but let’s move on
			}
			row_start = row_end = 0;
		}

		sig = tsk_resume_wait_signal(rcv_hnd.signal);
	} while (sig);

	if (cmd_state & WIFI_CMD_STATE_OK) {
		res = RES_SIG_OK;
	} else {
		// This isn’t good, but it shouldn’t happen.
		res = RES_SIG_ERROR;
	}
	return res;
}

NET_CODE esp8266_module::esp8266_change_ap(esp8266_ap_t& current_ap, esp8266_ap_t& target_ap)
{
	unsigned ap_flag_dif;
	bool discard = false;
	if (current_ap.mac == target_ap.mac) {
		//1. modify
		ap_flag_dif = (current_ap.ap_flags ^ target_ap.ap_flags) & ~WIFI_AP_FLAGS_INFO;
		if (ap_flag_dif & WIFI_AP_FLAG_STATIC_IP) {
			// change STATIC IP <-> DHCP
			discard = true;
		} else {
			// same IP/DHCP flag
			if (current_ap.ap_flags & WIFI_AP_FLAG_STATIC_IP) {
				// static IP used
				for (int i=0; i < 3; i++) {
					if(!ip_addr_cmp(&current_ap.ip_arr[i], &target_ap.ip_arr[i])) {
						discard = true;
						break;
					}
				}
			}
		}
	} else {
		discard = true;
	}
	if (discard) {
		esp8266_discarding_connected_sockets();
		return esp8266_connect_to_ap(target_ap)|FLG_SIGNALED;
	} else if (ap_flag_dif & WIFI_AP_FLAG_MDNS) {
		// only MDNS changes
		return esp8266_mdns(target_ap.ap_flags & WIFI_AP_FLAG_MDNS)|FLG_SIGNALED;
	}
	return RES_SIG_OK;
}

RES_CODE esp8266_module::wifi_cmd_config(CSocket* sock)
{
	if (!sock)
		return RES_SIG_ERROR;
	if (!(drv_info->drv_data->wifi_flags_ok & WIFI_FLAG_ON))
		return wifi_error(NET_ERR_PHY_NOT_READY)|FLG_SIGNALED;

	esp8266_ap_t current_ap;
	NET_CODE res;

	switch (sock->src.as_int) {

	case WIFI_COFIG_PARAM_LIST:
		if (sock->dst.as_cvoidptr) {
			CSTRING* p_str = (CSTRING*)sock->dst.as_cvoidptr;
			return esp8266_get_available_ap(*p_str);
		}
		break;

	case WIFI_COFIG_PARAM_CONNECT_TO_AP:
		if (sock->dst.as_cvoidptr) {
			esp8266_ap_t* p_ap = (esp8266_ap_t*)sock->dst.as_cvoidptr;

			res = wifi_get_current_net_ssid(current_ap);
			if (res==NET_IDLE) {
				return esp8266_connect_to_ap(*p_ap)|FLG_SIGNALED;
			} else if (res==NET_OK){
				return esp8266_change_ap(current_ap, *p_ap);
			} else {
				// This isn’t ideal, but it shouldn’t occur.
			}
		}
		break;

	case WIFI_COFIG_PARAM_GET_CURRENT_AP:
		if (sock->dst.as_cvoidptr) {
			esp8266_ap_t* p_ap = (esp8266_ap_t*)sock->dst.as_cvoidptr;
			res = wifi_get_current_net_ssid(current_ap);
			if (RES_OK==res) {
				*p_ap = current_ap;
			}
			return res | FLG_SIGNALED;
		}
		break;
	case WIFI_COFIG_PARAM_DISCONNECT_AP:
		if (sock->dst.as_cvoidptr) {
			esp8266_ap_t* p_ap = (esp8266_ap_t*)sock->dst.as_cvoidptr;
			res = wifi_get_current_net_ssid(current_ap);
			if (RES_OK==res && p_ap->mac == current_ap.mac) {
				esp8266_discarding_connected_sockets();
				wifi_send_cmd(WIFI_QUIT_AP, 10);
			} else if (RES_IDLE == res) {
				res = RES_OK;
			}
			return res | FLG_SIGNALED;
		}
		break;

		break;

	case WIFI_COFIG_PARAM_SET_SERVER:
		res = RES_IDLE;
		if (sock->dst.as_int) {
			if (!(drv_info->drv_data->wifi_flags_ok & WIFI_FLAG_SERVER)) {
				drv_info->drv_data->wifi_flags_ok |= WIFI_FLAG_SERVER;
				res = RES_OK;
			}
		} else {
			if (drv_info->drv_data->wifi_flags_ok & WIFI_FLAG_SERVER) {
				drv_info->drv_data->wifi_flags_ok &= ~WIFI_FLAG_SERVER;
				res = RES_OK;
			}
		}
		if ((drv_info->drv_data->wifi_flags_ok & WIFI_FLAG_ON) && RES_OK == res) {
			wifi_notify_power(this);
			if (!(drv_info->drv_data->wifi_flags_ok & WIFI_FLAG_SERVER)) {
				wifi_send_cmd(WIFI_SERVER "=0", 5);
				for (int link_id = 0; link_id < WIFI_ESP8266_MAX_SOCKETS; link_id++) {
					uint32_t state = esp8266_connections_status(ESP8266_GET_SATUS);
					state = ESP8266_STATUS_FROM_SERVER(state);
					if (!state) {
						break;
					} else if (!(state & (1 << link_id))) {
						continue;
					} else {
						if (link_to_socket[link_id] < WIFI_ESP8266_MAX_SOCKETS)
							closing_sockets[link_to_socket[link_id]] = true;
						esp8266_close_connection(link_id);
					}
				}
			}
		}
		return RES_SIG_OK;
	}
	return RES_SIG_ERROR;
}

//=============================================================================
// NOTIFICATION / TIMEOUT / CANCEL
//=============================================================================

int esp8266_module::wifi_notification(const char* row)
{
	if(IS_DIGIT(row[0]) && row[1] == ',')
	{
		unsigned int link_id = row[0] - '0';
		if(link_id < WIFI_ESP8266_MAX_SOCKETS)
		{
			unsigned int sock_id = link_to_socket[link_id];
			if(cmd_submatch("CLOSED", row+2) || cmd_submatch("CONNECT FAIL", row+2))
			{
				if (wait_for_connection
					&& link_to_socket[link_id] == wait_for_connection->sock_id) {
					// Notifications are locked for this connection,
					// they are processed when the connection is established.

				} else if (sock_id < WIFI_ESP8266_MAX_SOCKETS && alloc_sockets[sock_id]) {
					if(!received_size[sock_id] || closing_sockets[sock_id]
#if USE_WIFI_ESP8266 < 3 // version 3.0
					   || !received_data[id]
#endif
					) {
						// discard socket link
						link_to_socket[link_id] = WIFI_ESP8266_MAX_SOCKETS;
						//2. no data to read or socket marked as closing, close it
						wifi_driver_socket_close(sock_id, NET_ERR_SOCK_DISCONNECT); // wifi_notification
					} else {
						// 1. the socket has data, mark it as closing and keep it active
						//    even though the connection is closed,
						//    we leave it so the received data can be read
						closing_sockets[sock_id] = true;
					}
				}
				return 1;
			}
			if(cmd_submatch("CONNECT", row+2))
			{
				// a new connection has arrived
				if (wait_for_connection) {
					//1. the client is waiting for connection confirmation
					if (link_to_socket[link_id] == wait_for_connection->sock_id) {
						TRACELN_WIFI_DEBUG("WIFI: conn:%u to AP", link_id);
						wait_for_connection->sock_state = SOCKET_CONECTED;
						//2. marks mark as alive
						closing_sockets[wait_for_connection->sock_id] = false;
						return 1;
					} else {
						// this link is not waiting for confirmation
					}
				}
#if USE_WIFI_LISTEN
				// new connection without client, assumed to be a request from the server
				if (listen_socket) {
					if ((listen_socket->res & RES_BUSY_WAITING) == RES_BUSY_WAITING) {
						// the connection is expected
						NET_CODE res = RES_SIG_ERROR;
						TRACELN_WIFI_DEBUG("WIFI: conn:%u from SERVER", link_id);
						CSocket* sock =  (CSocket *)(listen_socket->dst.as_voidptr);
						// 1. allocate socket, if any
						if (sock && NET_OK == esp8266_allocate_socket(sock)) {
							if (link_to_socket[link_id] == WIFI_ESP8266_MAX_SOCKETS) {
								link_to_socket[link_id] = sock->sock_id;
								sock->sock_state = SOCKET_CONECTED;
								//2. marks mark as alive
								closing_sockets[sock->sock_id] = false;
								res = RES_SIG_OK;
							} else {
								// something is wrong, this connection is already in use
								TRACELN_WIFI_DEBUG("WIFI: Err conn:%u to sock:%u", link_id, link_to_socket[link_id]);
								alloc_sockets[sock->sock_id] = nullptr;
								used_sockets--;
							}
						}
						usr_HND_SET_STATUS(listen_socket, res);
						return 1;
					} else {
						// We have a listening socket, but it is not ready to accept a connection.
					}
				}
				TRACELN_WIFI_DEBUG("WIFI: SERVER conn:%u REJECT", link_id);
				notify_state.push(wifi_notify_t{WIFI_NOTIFY_REJECT_CONNECTION, (uint8_t)link_id});
				if (!tsk_test_signal(WIFI_NOTIFY_SIGNAL)) {
					tsk_send_signal(CURRENT_TASK, WIFI_NOTIFY_SIGNAL);
				}
				return 1;
#endif
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
		// Busy processing. The system is in process of handling the previous command,
		// cannot accept the newly input.
		return 1;
	}
	if(cmd_match("WIFI DISCONNECT", row))
	{
		drv_info->drv_data->wifi_flags_ok &= ~(WIFI_FLAG_REGISTERED|WIFI_FLAG_CONNECTED);
		connected_ssid.free();
		wifi_ap_state_changed(false, this);
		return 1;
	}
	if(cmd_match("WIFI CONNECTED", row))
	{
		drv_info->drv_data->wifi_flags_ok |= WIFI_FLAG_CONNECTED;
		return 1;
	}
	if(cmd_match("WIFI GOT IP", row))
	{
		drv_info->drv_data->wifi_flags_ok |= WIFI_FLAG_REGISTERED;
		wifi_ap_state_changed(true, this);
		return 1;
	}
#if ESP8266_WEB_SERVER
	if(cmd_match("+WEBSERVERRSP:2,", row))
	{
		web_ap.clear();
		row = row + strlen("+WEBSERVERRSP:2,");
		unsigned len;
		if (1==tmos_sscanf(row,"%u", &len)) {
			row = strpbrk(row, "\"");
			if(row && row[1]) {
				web_ap.ssid.assign(++row, len);
				row += len;
				if (1== tmos_sscanf(row,"\",%u,\"", &len)) {
					row = strpbrk(++row, "\"");
					if(row && row[1]) {
						web_ap.pwd.assign(++row, len);
						row += len;
						if (row[0]=='"' && !row[1]) {
							notify_state.push(wifi_notify_t{WIFI_NOTIFY_WEB_AP_SET, 0});
							if (!tsk_test_signal(WIFI_NOTIFY_SIGNAL))
								tsk_send_signal(CURRENT_TASK, WIFI_NOTIFY_SIGNAL);

						}
					}
				}
			}
		}

	}
#endif
	return 0;
}

void esp8266_module::wifi_notificatoin_response()
{
	wifi_notify_t notify;
	while (notify_state.pop(notify)) {
		switch (notify.notification) {
		case WIFI_NOTIFY_IDLE:
			break;
		case WIFI_NOTIFY_REJECT_CONNECTION:
			if (at_version < RTOS_AT_VERSION) {
				CSTRING cmd;
				// abort to disconnect
				cmd.format("+CIPCLOSEMODE=%u,1", notify.link_id);
				wifi_send_cmd(cmd.c_str(), 45);
			}
			esp8266_close_connection(notify.link_id);
			{
#if TRACE_WIFI_LEVEL >= TRACE_LEVEL_DEBUG
				unsigned res = esp8266_connections_status(ESP8266_GET_SATUS);
				TRACELN("REJECT %u, stat: %08X", notify.link_id, res);
#endif
			}
			break;
#if ESP8266_WEB_SERVER
		case WIFI_NOTIFY_WEB_AP_SET:
			if (!web_ap.ssid.empty() && !web_ap.pwd.empty()) {
				esp8266_ap_t current_ap;
				if( RES_OK == wifi_get_current_net_ssid(current_ap)) {
					if (web_ap.ssid == current_ap.ssid) {
						web_ap.mac = current_ap.mac;
						wifi_ap_set_from_web(web_ap, this);
					}
				}
				web_ap.clear();
			}
			break;
#endif
		default:
			TRACELN1_WIFI_DEBUG("Unprocessed notification response !!!");
			break;
		}
	}
}

void esp8266_module::wifi_process_tout(void)
{
	if ((drv_info->drv_data->wifi_flags_ok & WIFI_FLAG_ON)) {
		if (drv_info->drv_data->wifi_flags_ok & WIFI_FLAG_REGISTERED) {
			//TODO: Do something ...
		}
	}

	if (wifi_tout) {
		if (!--wifi_tout) {
			if (!used_sockets) {
				//--- deactivate the context?
				if (drv_info->drv_data->wifi_flags_ok & WIFI_FLAG_ON) {
					for (int i = 0; i < 3; i++) {
						if (wifi_send_cmd(WIFI_QUIT_AP, 150) & WIFI_CMD_STATE_OK) {
							if (!(drv_info->drv_data->wifi_flags_ok & WIFI_FLAG_REGISTERED)) {
									break;
							}
						}
					}
				}
				wifi_on_disconnect(this);

				TRACELN1_WIFI_DEBUG("WIFI OFF");
			}
		}
	}
}

void esp8266_module::wifi_cancelation(bool all_station, bool all_softAP)
{
	CSocket* sock;

#if WIFI_FLOW_CONTROL
	//cancel pending
	sock = (CSocket*) pending_read_data;
	while (sock) {
		if (sock->mode1 == WIFI_USER_CANCEL_REQUEST || all_station) {
			sock->list_remove(pending_read_data);
			usr_HND_SET_STATUS(sock, RES_SIG_IDLE);
			sock = (CSocket*) pending_read_data;
		} else {
			sock = (CSocket*) sock->next;
		}
	}
#endif

	//cancel reading sockets
	for (int sid = 0; sid < WIFI_ESP8266_MAX_SOCKETS; sid++) {
		sock = alloc_sockets[sid];
		if (sock && ((sock->res & RES_BUSY_WAITING) == RES_BUSY_WAITING)) {
			if (sock->mode1 == WIFI_USER_CANCEL_REQUEST)
				usr_HND_SET_STATUS(sock, (sock->res & FLG_OK) | FLG_SIGNALED);
		}

	}
#if USE_WIFI_LISTEN
	if (listen_socket && (all_softAP || listen_socket->mode1 == WIFI_USER_CANCEL_REQUEST)) {
		if ((listen_socket->res & RES_BUSY_WAITING) == RES_BUSY_WAITING)
			usr_HND_SET_STATUS(listen_socket, (listen_socket->res & FLG_OK) | FLG_SIGNALED);
	}
#endif
}

void esp8266_module::process_input(unsigned int signals, const char* cmd,	const char* hnd_start)
{
	unsigned char ch;

#if WIFI_FLOW_CONTROL
	if (stop_rcv) {
		if (mem_alloc_size < WIFI_FLOW_RELEASE_SIZE) {
			while (pending_read_data && (pending_read_data->res & RES_BUSY_WAITING) == RES_BUSY_WAITING) {
				TRACELN("Resum pending %s", pending_read_data->client.task->name);
				pending_read_data->tsk_start_handle();
				pending_read_data = pending_read_data->next;
			}
			TRACELN1("RTS Start rcv!");
			stop_rcv = false;
			PIO_Deassert(wifi_pin_rts);
		} else
			return;
	}
#endif
	if (rcv_hnd.res & FLG_CLOSED)
		return;
	//if receiver is idle -> start
	if (rcv_hnd.res < FLG_SIGNALED) {
		rcv_hnd.tsk_start_read(&received_ch, 1);
	}

	// if there is no signal -> try to get it
	if (!(signals & rcv_hnd.signal)) {
		signals = tsk_test_signal(rcv_hnd.signal);
		if (signals)
			tsk_get_signal(signals);
	}

	if (signals & rcv_hnd.signal) {
		rcv_hnd.res &= ~FLG_SIGNALED;

		ch = received_ch; //accept this symbol

		if (ch != '\n') {
			if (IS_ALPHANUM(ch) || IS_PUNC(ch) || ch == '\r' || ch == ' ') {

				if (ch < ' ') { // \r \n ...
					if (cmd_state & WIFI_CMD_STATE_STARTED) {
						// има прочетени символи, край на реда
						cmd_state ^= WIFI_CMD_STATE_STARTED;
						buf[row_end++] = 0;
						TRACE1_WIFI_DEBUG(" | ");
						TRACE_WIFI_DEBUG_SLEEP(5);
						cmd_state |= wifi_process_row(cmd);

					}
					// process_row() changes row_start if the row must stay
					// otherwise the row will be dumped
					row_end = row_start;
				} else {
					buf[row_end++] = ch;
					TRACE1_WIFI_DEBUG("\e[33m");
					TRACE_CHAR_WIFI_DEBUG(ch);
					TRACE1_WIFI_DEBUG("\e[m");
					cmd_state |= WIFI_CMD_STATE_STARTED;

					if (hnd_start) {
						if (at_version > RTOS_AT_VERSION && *hnd_start == ',') {
							//Fix for read command in RTOS versions
							if(!cmd_submatch("+CIPR", &buf[row_start])) {
								ch = 0;
							}
						}
						if (strchr(hnd_start, ch)) {
							cmd_state ^= WIFI_CMD_STATE_STARTED;
							buf[row_end] = 0;
							row_end--; // remove it // +SORD 4,123 "........." OK
							cmd_state |= WIFI_CMD_STATE_HND;
							TRACE1_WIFI_DEBUG("^hnd^");
							return;
						}
					}

					if (at_version < RTOS_AT_VERSION && cmd_submatch("+IPD,", &buf[row_start]) && ch == ':') {
						cmd_state ^= WIFI_CMD_STATE_STARTED;
						buf[--row_end] = 0;
						TRACELN1_WIFI_DEBUG("WIFI:Receive");
						ch = wifi_data_received(&buf[row_start]);
						row_end = row_start = 0;
						if (ch) {
							return;
						}
					}
				}
				if (row_end >= WIFI_BUF_SIZE - 1) {
					TRACE1_WIFI_ERROR("Dumping: ");
					TRACE1_WIFI_ERROR(buf);

					row_end = row_start = 0;
					cmd_state = WIFI_CMD_STATE_FATAL;
				}
			} else {
				TRACE_WIFI_ERROR("!!!(%02.2x)\r\n", ch);
			}
		}

		rcv_hnd.tsk_start_read(&received_ch, 1);
	}
}

//=============================================================================
// connecting to AP
//=============================================================================

NET_CODE esp8266_module::esp8266_connect_to_network()
{

	NET_CODE res;
	esp8266_ap_t target_ap, available_ap;
	CSTRING already_passed;
	uint32_t sig = 0;
	bool found=false;

	res = wifi_get_current_net_ssid(target_ap, true /* check only*/);
	if (NET_OK==res) {
		//1. there's nothing to do, everything's fine
	} else if (NET_IDLE==res) {
		try_again:
		//2. ESP8266 is not connected, we let the APP choose one of the available networks
		cmd_state |= WIFI_CMD_STATE_ROW_STOP;
		res = wifi_send_cmd(WIFI_LIST_AVAILABLE_AP, 60); // Wait 60 seconds to ensure the command completes.
		cmd_state &= ~WIFI_CMD_STATE_ROW_STOP;
		do {
			if (sig) {
				process_input(sig, WIFI_LIST_AVAILABLE_AP);
				res = cmd_state;
			}

			if (res >= WIFI_CMD_STATE_OK)
				break;

			if (res & WIFI_CMD_STATE_RETURNED) {
				cmd_state = res & ~WIFI_CMD_STATE_RETURNED;
				if (!found) {
					if (available_ap.from_module(buf)) {
						if (NET_OK == is_ap_cnown(available_ap, target_ap, already_passed))
							found = true;
					} else {
						// It’s not great, but let’s move on
					}
				}
				row_start = row_end = 0;
			}

			sig = tsk_resume_wait_signal(rcv_hnd.signal);
		} while (sig);

		if (cmd_state & WIFI_CMD_STATE_OK) {
			res = (found)?NET_OK:NET_EOF;
			if (NET_OK==res) {
				res =esp8266_connect_to_ap(target_ap);
				if (NET_OK != res) {
					already_passed += '"';
					already_passed += target_ap.mac;
					already_passed += "\",";
					found = false;
					goto try_again;
				}
			}
		} else {
			// This isn’t good, but it shouldn’t happen.
			res = NET_ERR_WIFI;
		}

	} else {
		//3. This isn’t good, but it shouldn’t happen.
		res = NET_ERR_WIFI;
	}
	return res;
}

NET_CODE esp8266_module::esp8266_connect_to_ap(wifi_access_point_t& target_ap)
{
	CSTRING cmd;
	uint32_t res = NET_ERR_PHY_NOT_READY;
	esp8266_ap_t current_ap;
	const char* dhcp_cmd;
	// 1. switching between DHCP or STATIC IP
	if (target_ap.ap_flags & WIFI_AP_FLAG_STATIC_IP) {
		// 1.1 static IP
		if (at_version >= RTOS_AT_VERSION)
			dhcp_cmd = WIFI_DHCP "=0,1";
		else
			dhcp_cmd = WIFI_DHCP "=1,0";
		if (WIFI_CMD_STATE_OK == wifi_send_cmd(dhcp_cmd, 5)) {
			res = esp8266_station_ip(target_ap, true /* set */);
		}
	} else {
		// 1.2 uses DHCP
		if (WIFI_CMD_STATE_OK == wifi_send_cmd(WIFI_DHCP "=1,1", 5)) {
			res = NET_OK;
		}
	}
	if (res != RES_OK)
		return wifi_net_error(res);

	// 2. set the AP to which the ESP8266 should be connected
	cmd = WIFI_JOIN_TO_AP "=\"";
	if (!esp8266_esc_syntax(target_ap.ssid, cmd)) {
		return wifi_net_error(NET_ERR_WIFI_NET_NAME_PWD);
	}
	cmd += "\",\"";
	if (target_ap.ap_enc != enc_open && !esp8266_esc_syntax(target_ap.pwd, cmd)) {
		return wifi_net_error(NET_ERR_WIFI_NET_NAME_PWD);
	}
	cmd += '"';
	if (!target_ap.mac.empty()) {
		cmd += ",\"";
		cmd += target_ap.mac;
		cmd += '"';
	}

	//3. connecting to the access point
	uint32_t tout = 30; // 30s, 45s, 1min
	for(int i=0; i < 3; i++)
	{
		//3.1 trying to connect
		res = wifi_send_cmd(cmd.c_str(), tout);
		if (WIFI_CMD_STATE_OK & res) {
			// Already connected, reading assigned IP address, GATEWAY and MASK
			res = wifi_get_current_net_ssid(current_ap);
			if (RES_OK == res) {

				cmd = "+PING=";
				if (target_ap.ap_flags & WIFI_AP_FLAG_STATIC_IP) {
					if (ip_addr_isany(&target_ap.gw)) {
						if (ip_addr_cmp(&current_ap.gw, &target_ap.ip)) {
							// Gateway and IP address can't be the same
							return wifi_net_error(NET_ERR_WIFI_INVALID_GATEWAY);
						} else {
							cmd += wifi_ip_to_str(&current_ap.gw, true);
						}
					} else {
						cmd += wifi_ip_to_str(&target_ap.gw, true);
					}
					res = wifi_send_cmd(cmd.c_str(), 5);
					if( res & WIFI_CMD_STATE_CMES ) {
						return wifi_net_error(NET_ERR_WIFI_INVALID_GATEWAY);
					}
					if (ip_addr_isany(&target_ap.mask) &&
						!ip_addr_netcmp(&current_ap.ip, &current_ap.gw, &current_ap.mask)) {
						wifi_net_error(NET_ERR_WIFI_INVALID_MASK);
					}
				}
				// update IP, GATEWAY, NETMASK
				if (ip_addr_isany(&target_ap.ip)) {
					ip_addr_copy(target_ap.ip, current_ap.ip);
				}
				if (ip_addr_isany(&target_ap.gw)) {
					ip_addr_copy(target_ap.gw, current_ap.gw);
				}
				if (ip_addr_isany(&target_ap.mask)) {
					ip_addr_copy(target_ap.mask, current_ap.mask);
				}
				// checks Internet connection
				res = wifi_send_cmd("+PING=\"google.com\"", 10);
				if (res & WIFI_CMD_STATE_CMES) {
					target_ap.ap_flags |= WIFI_AP_FLAG_NO_INTERNET;
				} else if (res & WIFI_CMD_STATE_OK) {
					target_ap.ap_flags &= ~WIFI_AP_FLAG_NO_INTERNET;
				}
				esp8266_mdns(false);
				esp8266_mdns(target_ap.ap_flags & WIFI_AP_FLAG_MDNS);

				if (target_ap.mac.empty())
					target_ap.mac = current_ap.mac;
				// current_ap.ssid can be empty in case it is hidden
				connected_ssid = target_ap.ssid;

				return NET_OK;
			} else {
				// this shouldn't happen, but just in case
				// reconnect ...
				continue;
			}
		} else if (WIFI_CMD_STATE_CMES & res ) {
			int err;
			if ((WIFI_CMD_STATE_RETURNED & res ) && wifi_get_param(buf, err, 1)) {
				switch(err) {
				case 3: // can not found target AP
					if (i >= 2)
						return wifi_net_error(NET_ERR_WIFI_SSID_NOT_FOUND);
					break;
				case 1: // connecting timeout
					if (i >= 2)
						return wifi_net_error(NET_ERR_WIFI_CONNECTING_TIMEOUT);
					break;
				case 2: // wrong password
					return wifi_net_error(NET_ERR_WIFI_WRONG_PASSWORD);
				case 4: // connect fail
					return wifi_net_error(NET_ERR_WIFI_CONNECT_FAIL);
				default:
					return wifi_net_error(NET_ERR_WIFI);
					break;
				}
			}
			if (i < 2) {
				tout +=15;
				continue;
			}
		}
		break; // loop
	}
	return wifi_net_error(NET_ERR_WIFI_REGISTER);

}

//=============================================================================
// methods related to OPENING a socket
//=============================================================================

NET_CODE esp8266_module::esp8266_allocate_socket(CSocket* sock)
{
	for (unsigned sid=0; sid < WIFI_ESP8266_MAX_SOCKETS; sid++) {
		if (nullptr == alloc_sockets[sid]) {
			sock->sock_id = sid;
			sock->sock_state = SOCKET_OPEN;
			alloc_sockets[sid] = sock;
			// discard socket data
			if (received_data[sid]) {
				delete received_data[sid];
				received_data[sid] = nullptr;
			}
			DEC_ALLOC_SIZE(received_size[sid]); //
			received_size[sid] = 0;
			used_sockets++;
			return NET_OK;
		}
	}
	return NET_EOF;
}

NET_CODE esp8266_module::esp8266_socket_open(CSocket* sock)
{
	sock_mode_t* mode;

	mode = (sock_mode_t*)sock->mode.as_voidptr;
	if(mode )
	{
		if (  (mode->sock_type == IP_SOCKET_UDP && mode->port)
			||(mode->sock_type == IP_SOCKET_TCP) )
		{
#if USE_WIFI_LISTEN
			if (mode->interface == WIFI_SERVER_INTERFACE) {
				// opening the listening socket
				if (listen_socket) {
					// only one listening socket can be opened
					return wifi_net_error(NET_ERR_SOCK_CREATE);
				} else {
					// sets the socket id after all possible connections (invalid connection)
					listen_socket = sock;
					listen_socket->sock_id = WIFI_ESP8266_MAX_SOCKETS;
					listen_socket->sock_state = SOCKET_OPEN;
					used_sockets++;
					return NET_OK;
				}
			}
#endif
			if ( NET_OK == esp8266_allocate_socket(sock))
				return NET_OK;
			// no free sockets
		}
	}
	TRACE_WIFI_ERROR("\r\nWIFI:%s create socket ERROR", sock->client.task->name);
	return wifi_net_error(NET_ERR_SOCK_CREATE);

}

RES_CODE esp8266_module::wifi_sock_cmd_open(CSocket* sock) // NET_ERROR OK
{
	if (sock->sock_state == SOCKET_CLOSED) {
		NET_CODE res;
		//---  check the registration
		sock_mode_t *mode = (sock_mode_t*) sock->mode.as_voidptr;
		if (mode) {
			res = wifi_drv_on();
			if (NET_OK == res) {
				res = esp8266_connect_to_network();
				if (mode->interface == WIFI_SERVER_INTERFACE)
					res = NET_OK;
			}
			if (NET_OK == res) {
				res = esp8266_socket_open(sock);
			}
			return res | FLG_SIGNALED;
		}
	} else
		wifi_net_error(NET_ERR_SOCK_NOT_CLOSED);

	return RES_SIG_ERROR;
}

//=============================================================================
// methods related to CLOSING a socket
//=============================================================================

void esp8266_module::esp8266_discarding_connected_sockets()
{
	unsigned state;
	ip_addr_t links_ip[WIFI_ESP8266_MAX_SOCKETS];
	esp8266_ap_t soft_ap;
#if ESP8266_SOFT_AP
	bool soft_ap_on = false;
	if (WIFI_CMD_STATE_ROK == wifi_send_cmd(WIFI_MODE "?", 5)) {
		if (wifi_get_param(buf, state, 1) && (state & 2)) {
			// 2: SoftAP mode
			if (RES_OK == esp8266_soft_ap_ip(soft_ap))
				soft_ap_on = true;
		}
	}
#endif
	for (int link_id = 0; link_id < WIFI_ESP8266_MAX_SOCKETS; link_id++) {
		state = esp8266_connections_status(ESP8266_GET_SATUS, links_ip);
		if (!ESP8266_STATUS_CONNECTED(state)) {
			// no connections
			return;
		}
		if (ESP8266_STATUS_CONNECTED(state) & (1 << link_id)) {
			// the connection exists
			if (!(ESP8266_STATUS_TO_SOCKET(state) & (1 << link_id))) {
				// the connection was not created from a socket
				if (ESP8266_STATUS_FROM_SERVER(state) & (1 << link_id)) {
					// the connection was established by a server
#if ESP8266_SOFT_AP
					if (soft_ap_on && !ip_addr_isany(links_ip +link_id)) {
						if (ip_addr_netcmp(links_ip+link_id, &soft_ap.ip, &soft_ap.mask))
							continue;
					}
#endif
				} else {
					// connection, but where is it from
				}
			}
			if (link_to_socket[link_id] < WIFI_ESP8266_MAX_SOCKETS)
				closing_sockets[link_to_socket[link_id]] = true;
			esp8266_close_connection(link_id);
		}
	}
}

void esp8266_module::wifi_driver_socket_close(unsigned int sid, unsigned int reason)
{
	CSocket *sock;

	sock = alloc_sockets[sid];
	if (sock) {
		if (received_data[sid]) {
			delete received_data[sid];
			received_data[sid] = NULL;
			DEC_ALLOC_SIZE(received_size[sid]);
		}
		received_size[sid] = 0;
		sock->sock_id = SOCKET_ID_INVALID;
		sock->sock_state = SOCKET_CLOSED;
		alloc_sockets[sid] = nullptr;

		// do we have to signal it?
		if ((sock->res & RES_BUSY_WAITING) == RES_BUSY_WAITING) {
			//How to signal it?
			sock->error = reason;
			TRACELN_WIFI_DEBUG("Socket disconnect %d", sid);
			tsk_HND_SET_STATUS(sock, RES_EOF | FLG_SIGNALED);
		} else {
			TRACELN_WIFI_DEBUG("disconnect %d, reason: %d", sid, reason);
			wifi_net_error(reason);
		}

		if (!--used_sockets) {
			if (drv_info->drv_data->wifi_clients > 2)
				wifi_tout = 5;
			else
				wifi_tout = 1;
		}
	}
}

NET_CODE esp8266_module::esp8266_close_connection(unsigned link_id)
{
	CSTRING cmd;
	cmd.format(WIFI_DISCONNECT"=%u", link_id);
	if(wifi_send_cmd(cmd.c_str(), 45) & WIFI_CMD_STATE_OK) {
		for (int i=0; i< 500; i++){
			if(link_to_socket[link_id] == WIFI_ESP8266_MAX_SOCKETS)
				return NET_OK;
			wifi_sleep(10);
		}
	}
	return RES_FATAL;
}

NET_CODE esp8266_module::esp8266_socket_close(unsigned int sid)
{
	unsigned link_id=0, status;
	NET_CODE res = NET_OK;
	if (sid >= WIFI_ESP8266_MAX_SOCKETS || alloc_sockets[sid] == nullptr) {
		return NET_EOF;
	}
	if (alloc_sockets[sid]->sock_state != SOCKET_CONECTED) {
		return NET_OK;
	}
	status = esp8266_connections_status(sid);
	if (!(status & ESP8266_GET_STATUS_OK))
		res = RES_FATAL; // not response

	if(res == RES_OK && (status & ESP8266_STATUS_SOCK_CONECTED)) {
		res = RES_EOF;
		// forced closure, regardless of whether there is data received
		closing_sockets[sid] = true;
		for (;link_id < WIFI_ESP8266_MAX_SOCKETS; link_id++) {
			if (link_to_socket[link_id] == sid ) {
				res = esp8266_close_connection(link_id);
				break;
			}
		}
	}
	return res;
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

RES_CODE esp8266_module::wifi_sock_cmd_close(CSocket* sock) // NET_ERROR OK
{
	unsigned int sid;

	sid = sock->sock_id;
#if USE_WIFI_LISTEN
	if( RES_OK == wifi_close_listen(sock, NET_OK))
		return RES_SIG_OK;
#endif
	if(sid < WIFI_ESP8266_MAX_SOCKETS)
	{
		if( (sock->sock_state & SOCKET_CONECTED) )
			esp8266_socket_close(sid);

		wifi_driver_socket_close(sid, NET_OK); // wifi_sock_close
		return RES_SIG_OK;
	}
	wifi_net_error(NET_ERR_SOCK_CLOSED);
	return RES_SIG_ERROR;
}

//=============================================================================
// methods related to CONNECTING a socket
//=============================================================================

RES_CODE esp8266_module::wifi_sock_cmd_connect_adr(CSocket* sock)
{
	return wifi_sock_cmd_connect_url(sock);
}

RES_CODE esp8266_module::wifi_sock_cmd_connect_url(CSocket* sock)
{
	CSTRING cmd;
	int link_id;
	unsigned links;

	// 1.) the socket must be open
	if (!(sock->sock_state & SOCKET_OPEN)) {
		wifi_net_error(NET_ERR_SOCK_CLOSED);
		return RES_SIG_ERROR;
	}
	// 2.) allocate a connection ID for the socket
	//     The server allocate IDs from first available, so we allocate them in reverse order.
	for(link_id=WIFI_ESP8266_MAX_SOCKETS-1; link_id >= 0; link_id--) {
		if(link_to_socket[link_id] == WIFI_ESP8266_MAX_SOCKETS) {
			link_to_socket[link_id] = sock->sock_id;
			break;
		}
	}
	// 3.) if all IDs are in use, there is nothing we can do
	if (link_id < 0) {
		wifi_net_error(NET_ERR_NO_LINK);
		return RES_SIG_ERROR;
	}
	// 4.) tries 3 times to connect
	for (int try_cnt=0; sock && try_cnt <3; try_cnt++ )	{
			cmd.format(WIFI_CONNECT"=%u,\"%s\",\"%s\",%u", link_id,
				(((sock_mode_t*)sock->mode.as_voidptr)->sock_type == IP_SOCKET_TCP)?"TCP":"UDP",
				sock->src.as_charptr,
				sock->dst.as_int);
			// 4.1) marks the socket as waiting for a connection
			wait_for_connection = sock;
			// URCs "CLOSED", "CONNECT FAIL" and "CONNECT" are locked for this socket and connection ID.
			wifi_send_cmd(cmd.c_str(), 40);
			if (cmd_state & WIFI_CMD_STATE_OK) {
				// 4.2) OK
				for (int i=0; i<500; i++) {
					if (sock->sock_state == SOCKET_CONECTED) {
						wait_for_connection = nullptr;
						TRACELN_WIFI_DEBUG("Socket[%u] Link:%u", sock->sock_id, link_id);
						return RES_SIG_OK;
					}
					wifi_sleep(10);
				}
			} else if (cmd_state & WIFI_CMD_STATE_CMES) {
				bool already_connected = false;
				// 4.3) ERROR
				if (cmd_state & WIFI_CMD_STATE_RETURNED) {
					if (!strcmp(buf, "ALREADY CONNECTED")) {
						// 4.3.1) ALREADY CONNECTED | ERROR
						already_connected = true;
					}
				}
				links = esp8266_connections_status(ESP8266_GET_SATUS);
				if (links & ESP8266_GET_STATUS_OK) {
					if (already_connected) {
						if(ESP8266_STATUS_FROM_SERVER(links) & (1<<link_id)) {
							// The server was connected before the command was executed.
							// rejecting the connection and restoring the socket state
							esp8266_close_connection(link_id);

							sock->sock_state = SOCKET_OPEN;
							received_size[sock->sock_id] =0;
							if (received_data[sock->sock_id]) {
								delete received_data[sock->sock_id];
								received_data[sock->sock_id] = nullptr;
							}
						}
					} else {
						switch (ESP8266_STATUS_AP_STAUS(links)) {
						case 2: // The ESP8266 Station is connected to an AP and its IP is obtained.
							break;
						case 3: // The ESP8266 Station has created a TCP or UDP transmission.
							break;
						case 4: // The TCP or UDP transmission of ESP8266 Station is disconnected.
							break;
						case 5: // The ESP8266 Station does NOT connect to an AP.
							break;
						default:
							break;
						}
					}
				} else {
					// get_connection_state failed
				}
			} else {
				// WIFI_CONNECT failed
			}
	}
	// Unlock ----------------------------------------------------------
	wait_for_connection = nullptr;
	link_to_socket[link_id] = WIFI_ESP8266_MAX_SOCKETS;
	wifi_net_error(NET_ERR_SOCK_CONNECT);
	return RES_SIG_ERROR;
}

//=============================================================================
// methods related to LISTEN socket
//=============================================================================
#if USE_WIFI_LISTEN

RES_CODE esp8266_module::wifi_sock_cmd_bind_adr(CSocket* sock)
{
	if(listen_socket && sock == listen_socket)
	{
		listen_port = sock->dst.as_int;
		return RES_SIG_OK;
	}
	return RES_SIG_ERROR;
}

RES_CODE esp8266_module::wifi_sock_cmd_bind_url(CSocket* sock)
{
	return RES_SIG_ERROR;
}

RES_CODE esp8266_module::wifi_sock_cmd_listen(CSocket* sock)
{
	CSTRING cmd;
	if(listen_socket && sock == listen_socket)
	{
		cmd.format(WIFI_SERVER"=1,%u", listen_port);
		if(wifi_send_cmd(cmd.c_str(), 5) == WIFI_CMD_STATE_OK)
		{
			wifi_send_cmd(WIFI_SRVER_TIMEOUT"=1800", 5); // 30 min timeout
			sock->sock_state = SOCKET_LISTEN;
			return RES_SIG_OK;
		}
	}
	else
		wifi_net_error(NET_ERR_SOCK_CONNECT);
	return RES_SIG_ERROR;
}

RES_CODE esp8266_module::wifi_sock_cmd_accept(CSocket* sock)
{
	NET_CODE res = RES_SIG_ERROR;
	unsigned int link;
	unsigned int accepted, from_server;
	if(listen_socket == sock)
	{
		link = esp8266_connections_status(ESP8266_GET_SATUS);
		if (link & ESP8266_GET_STATUS_OK) {
			from_server = ESP8266_STATUS_FROM_SERVER(link);
			accepted = ESP8266_STATUS_ACCEPTED(link);
			if (from_server && from_server != accepted) {
				for (unsigned id=0, mask=0; id < WIFI_ESP8266_MAX_SOCKETS; id++, mask <<=1){
					if ((from_server & mask) && !(accepted & mask)) {
						CSocket* new_sock =  (CSocket *)(listen_socket->dst.as_voidptr);
						// 1. allocate socket, if any
						if (new_sock && NET_OK == esp8266_allocate_socket(new_sock)) {
							if (link_to_socket[id] == WIFI_ESP8266_MAX_SOCKETS) {
								link_to_socket[id] = new_sock->sock_id;
								sock->sock_state = SOCKET_CONECTED;
								//2. marks mark as alive
								closing_sockets[sock->sock_id] = false;
								res = RES_SIG_OK;
							} else {
								// something is wrong, this connection is already in use
								TRACELN_WIFI_DEBUG("WIFI: Err conn:%u to sock:%u", id, link_to_socket[id]);
								alloc_sockets[new_sock->sock_id] = nullptr;
								used_sockets--;
							}
						}
						return res;
					}
				}
			}
			listen_socket->res = RES_BUSY_WAITING;
			return RES_IDLE;
		}
	}
	return RES_SIG_ERROR;
}

#endif

RES_CODE esp8266_module::wifi_sock_cmd_disconect(CSocket* sock)
{
	uint32_t sid;
	if(sock && sock->sock_id < WIFI_ESP8266_MAX_SOCKETS)
	{
		sid =sock->sock_id;
		if(alloc_sockets[sid] == sock && sock->sock_state == SOCKET_CONECTED)
		{
			esp8266_socket_close(sid);
			sid = esp8266_connections_status(sid);
			if((sid & ESP8266_GET_STATUS_OK) && !(sid & ESP8266_STATUS_SOCK_CONECTED)) // wifi_sock_disconect
				return RES_SIG_OK;
		}
	}
	return RES_SIG_ERROR;
}

//=============================================================================
//	unsupported socket commands
//=============================================================================

RES_CODE esp8266_module::wifi_gethostbyname(CSocket* sock)
{
	return RES_OK;
}
#if USE_WIFI_LISTEN
RES_CODE esp8266_module::wifi_sock_cmd_addr(CSocket* sock)
{
	return RES_SIG_ERROR;
}
#endif
//=============================================================================
//	Receiving/reading data
//=============================================================================

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
	uint32_t link_id=WIFI_ESP8266_MAX_SOCKETS, sock_id=WIFI_ESP8266_MAX_SOCKETS, size;
	if(2 == tmos_sscanf(row,"+IPD,%u,%u", &link_id, &size))
	{
		if (link_id < WIFI_ESP8266_MAX_SOCKETS) {
			sock_id = link_to_socket[link_id];
		}
		CSocket* sock;
		if(sock_id < WIFI_ESP8266_MAX_SOCKETS && (sock = alloc_sockets[sock_id]))
		{
			sock_mode_t* mode = (sock_mode_t*)sock->mode.as_voidptr;
			if(mode)
			{
				if(mode->sock_type == IP_SOCKET_UDP && at_version < RTOS_AT_VERSION)
				{
					if(size)
					{
						char *mem = (char*)tsk_realloc(received_data[sock_id], received_size[sock_id]+size);
						if(mem)
						{
							received_data[sock_id] = mem;
							INC_ALLOC_SIZE(size);
							if( rcv_hnd.tsk_read_pkt(mem+received_size[sock_id], size, WIFI_READ_TOT) == RES_OK)
							{
								TRACE1("read Ok");
	//							TRACE_BUF(received_data[id]+received_size[id], size, TC_TXT_CYAN);
								received_size[sock_id] += size;
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
					received_size[sock_id] = size;
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
				// ако няма нотификация го слага в списъка с чакащи
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
	unsigned size, available_len;
	unsigned link_id, sock_id;
	const char *rcv_pending;
	CSTRING cmd;

	sock_id = sock->sock_id;
    TRACELN_WIFI_DEBUG("WIFI: read %d?", sock->len);
	while(sock_id < WIFI_ESP8266_MAX_SOCKETS && sock->sock_state == SOCKET_CONECTED)
	{
		if(!received_size[sock_id] && closing_sockets[sock_id])
		{
			if (esp8266_socket_to_connection(sock_id, link_id)) {
				link_to_socket[link_id] = WIFI_ESP8266_MAX_SOCKETS;
			}
			wifi_driver_socket_close(sock_id, NET_ERR_SOCK_DISCONNECT); // process_read
			if(!(sock->res & FLG_OK))
				return FLG_EOF | FLG_SIGNALED;
			return RES_SIG_OK;
		}

		if(!sock->len)
		{
			TRACE1_WIFI_DEBUG(" done!");
			return (sock->res & FLG_OK)| FLG_SIGNALED;
		}

		if(!received_size[sock_id])
		{
			if(!(sock->res & FLG_OK))
			{
				// ако няма нотификация го слага в списъка с чакащи
				TRACE1_WIFI_DEBUG("wait!");
				sock->res |= RES_BUSY_WAITING;
				return RES_IDLE;
			}
			return  RES_SIG_OK;
		}
		if (at_version < RTOS_AT_VERSION &&
				sock->mode.as_voidptr && ((sock_mode_t *)sock->mode.as_voidptr)->sock_type == IP_SOCKET_UDP)
		{
			// UDP receive
			if(!received_data[sock_id])
				break;

			size = sock->len;
			if(size > received_size[sock_id])
				size = received_size[sock_id];
			memcpy(sock->dst.as_voidptr, received_data[sock_id], size);
			TRACE_WIFI_RX_BUFF(sock->dst.as_voidptr, size);
			sock->dst.as_byteptr += size;
			sock->len -= size;
			if(received_size[sock_id] > size)
			{
				memmove(received_data[sock_id], received_data[sock_id] + size, received_size[sock_id] - size);
				DEC_ALLOC_SIZE(size);
				received_size[sock_id] -= size;
			}
			else
			{
				DEC_ALLOC_SIZE(received_size[sock_id]);
				received_size[sock_id] = 0;
				delete received_data[sock_id];
				received_data[sock_id] = nullptr;
				TRACE1_WIFI_DEBUG(" EOF done!");
			}
			return RES_SIG_OK;
		}
		// TCP receive
		size = sock->len;
		if(size > 2920) // non_os_sdk default is 2920 bytes, FreeRTOS 5760 bytes by default
			size = 2920;

		if (!esp8266_socket_to_connection(sock_id, link_id)) {
			break;
		}
		cmd.format("+CIPRECVDATA=%u,%u", link_id, size);
	    TRACELN_WIFI_DEBUG("WIFI: read %d from %d", size, received_size[sock_id]);
	    if (at_version >= RTOS_AT_VERSION)
		    rcv_pending = ",";
		else
		    rcv_pending = ":";
	    // изпраща командата за четене
	    cmd_state |= WIFI_CMD_STATE_HND;
	    if(wifi_send_cmd(cmd.c_str(), 15) == WIFI_CMD_STATE_RETURNED)
	    {
	    	do
	    	{
				process_input(rcv_hnd.signal, cmd.c_str(), rcv_pending); // чака да се върне ОК,ERROR или начало на данните
    			if ( cmd_state >= WIFI_CMD_STATE_HND )
    			{
        			if ( cmd_state & WIFI_CMD_STATE_HND )
        			{
        				cmd_state &= ~WIFI_CMD_STATE_HND;
        				rcv_pending = 0;
						// колко са действителните байтове които ще се четат +CIPRECVDATA:/+CIPRECVDATA,
						if(cmd_match("+CIPRECVDATA", buf) &&
								tmos_sscanf(&buf[strlen("+CIPRECVDATAx")],"%u", &available_len) == 1)
						{
							if(size > available_len)
								size = available_len; // промяна на дължината с деиствителният размер на четене

							if(size)
							{
								unsigned int mytime;

								mytime = CURRENT_TASK->time;
								if( rcv_hnd.tsk_read_pkt(sock->dst.as_voidptr, size, WIFI_READ_TOT) == RES_OK)
								{
									TRACE_WIFI_RX_BUFF(sock->dst.as_voidptr, size);
									size -= rcv_hnd.len;
									TRACE_WIFI_DEBUG(" (%d)\r\n", size);
									sock->dst.as_byteptr += size;
									sock->len -= size;
									if(received_size[sock_id] >= size)
										received_size[sock_id] -= size;
									else
										received_size[sock_id] = 0;
									sock->res |= FLG_OK;

								}
								else
								{
									TRACE_WIFI_ERROR("\e[1;91mERROR %X , len:%u\e[m", rcv_hnd.res, rcv_hnd.len);
								}
								CURRENT_TASK->time = mytime;
							}
							else
								received_size[sock_id] = 0;

							rcv_hnd.tsk_start_read(&received_ch, 1);

						}
						else
						{
							TRACE1_WIFI_ERROR("\r\n\e[1;91mWIFI:Invalid data params STOP!\e[m");
							break;
						}

        			} else
        			{
    					if ( cmd_state >= WIFI_CMD_STATE_OK )
    						break; // командата е завършила с OK, ERROR ..
        			}

    			}
				// продължава да чака отговор
	    	} while(tsk_resume_wait_signal(rcv_hnd.signal));
	    }



		if (!(cmd_state & WIFI_CMD_STATE_OK))
		{
			break;
		}
	}// има още за четене и ако няма приети данни ще го прати да чака
	wifi_sleep(120);
	wifi_net_error(NET_ERR_SOCK_READ);
	TRACE_WIFI_ERROR("\r\nWIFI:%s read INVALID", sock->client.task->name);
	return RES_FATAL|FLG_SIGNALED;
}
#endif

//=============================================================================
//	Sending data
//=============================================================================

RES_CODE esp8266_module::process_write(CSocket* sock)
{
	unsigned size;//, newsize, id;
//	unsigned int start_size, write_size, sock_state;
	CSTRING cmd;
	const char* snd_pending=nullptr;
	unsigned link_id;

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
		if(closing_sockets[sock->sock_id])
		{
			break;
		}
		if(!esp8266_socket_to_connection(sock->sock_id, link_id))
			break;
		// Send command
		cmd.format(WIFI_SEND_DATA"=%u,%u", link_id, size);
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
					if( snd_hnd.tsk_write(sock->src.as_voidptr, size, WIFI_WRITE_TOT) != RES_OK)
						break;
					TRACE_WIFI_TX_BUFF(sock->src.as_voidptr, size);
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


RES_CODE esp8266_module::process_cmd(HANDLE client)
{
	RES_CODE res = RES_SIG_ERROR;


	if( !(drv_info->drv_data->wifi_flags_bad & WIFI_FLAG_ON))
	{
		switch (client->cmd)
		{
		case SOCK_CMD_CLOSE:
			res = wifi_sock_cmd_close((CSocket*) client);
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
	return res;

}


//esptool -p /dev/ttyUSB0 -b 115200  --no-stub write-flash --flash-mode dio --flash-size 2MB --flash-freq 40m 0x0 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x9000 build/ota_data_initial.bin 0x10000 build/esp-at.bin 0xF0000 build/at_customize.bin 0xF1000 build/customized_partitions/factory_param.bin
//esptool.py -p /dev/ttyUSB0 --no-stub write_flash --flash_size 2MB-c1 0x0 img/boot_v1.7.bin 0x01000 img/user1.2048.new.5.bin 0x1fb000 img/blank.bin 0x1fc000 img/esp_init_data_default_v08.bin 0xfe000 img/blank.bin 0x1fe000 img/blank.bin
RES_CODE esp8266_module::module_upgrade(HANDLE hnd)
{
	char wifi_ack[32];
	unsigned int sig, old_sig;
	HND_CLIENTS old_client;
	RES_CODE res;
	CHandle hnd_snd;
	uint32_t buf_in, buf_out, rx_size=0, tx_size=0, wifi_ack_off;
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
	    	PIO_Cfg(wifi_pin_boot);
	    	tsk_sleep(20);
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
		    	unsigned int pc_rcv, wifi_rcv;

			    TRACE1_WIFI("\r\n------ WIFI UPGRADE --------");

			    res = hnd_snd.tsk_open(hnd->drv_index, hnd->mode.as_voidptr);
			    //flush PC
			    buf_in = 20;
			    wifi_ack_off = 0;
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

			    	// start sending to ESP8266
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

			    	//start reading from ESP8266
			    	if(rcv_hnd.res < FLG_SIGNALED)
						if(!rcv_hnd.tsk_start_read(wifi_ack+wifi_ack_off, sizeof(wifi_ack)/2))
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

			    	//process ESP8266
			    	if(sig & rcv_hnd.signal)
			    	{
			    		rcv_hnd.res &= ~FLG_SIGNALED;
				    	wifi_rcv = sizeof(wifi_ack)/2 - rcv_hnd.len;
				    	if(wifi_rcv)
				    	{
				    		hnd_snd.tsk_start_write(wifi_ack+wifi_ack_off, wifi_rcv);
				    		TRACE_BUF(wifi_ack+wifi_ack_off, wifi_rcv, TC_TXT_CYAN);
				    		if(wifi_ack_off)
				    			wifi_ack_off =0;
				    		else
				    			wifi_ack_off = sizeof(wifi_ack)/2;
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
			    	rcv_hnd.close();
			    	snd_hnd.close();
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

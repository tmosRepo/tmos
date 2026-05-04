/*
 * esp8266.h
 *
 *  Created on: 05.05.2015
 *      Author: Stanly
 */

#ifndef ESP8266_H_
#define ESP8266_H_

#include <tmos.h>
#include <wifi_core.h>
#include <csocket.h>
#include <mqueue.h>

WEAK_C wifi_module_type* wifi_detect(WIFI_DRIVER_INFO* drv_info);

bool is_esp8266_supported_encryption(wifi_encryption_type_t enc);

#define USE_CUR_SUF			1

#define RTOS_AT_VERSION			0x2220
#define RTOS_AT_VERSION_V2300	0x2300

/*
struct wifi_access_point_mode_t
{
	sock_mode_t				sock_mode;
	wifi_access_point_t* 	access_point;
};
*/

#ifndef ESP8266_TRACE_BOOT
#define ESP8266_TRACE_BOOT	1
#endif

#ifndef ESP8266_SOFT_AP
#define ESP8266_SOFT_AP 	0
#endif


#ifndef ESP8266_WEB_SERVER
#if 	ESP8266_SOFT_AP
#define ESP8266_WEB_SERVER 	1
#else
#define ESP8266_WEB_SERVER  0
#endif
#else	// ESP8266_WEB_SERVER defined
#if 	ESP8266_WEB_SERVER && !ESP8266_SOFT_AP
#undef 	ESP8266_WEB_SERVER	// SOFT_AP disabled, can't use WEB
#define ESP8266_WEB_SERVER	0
#warning "WEB_SERVER cannot be used if SOFT_AP is disabled"
#endif
#endif

#ifndef WIFI_ESP8266_MAX_SOCKETS
#define WIFI_ESP8266_MAX_SOCKETS	 5	//!< esp8266 maximum number of sockets
#endif

#define ESP8266_GET_SATUS			(WIFI_ESP8266_MAX_SOCKETS +1)

#define ESP8266_STAUS_MASK			((1<<WIFI_ESP8266_MAX_SOCKETS)-1)

#define ESP8266_STAT_OFFSET_CONNECTED		24
#define ESP8266_STAT_OFFSET_TO_SOCKET		16
#define ESP8266_STAT_OFFSET_FROM_SERVER	 	 8
#define ESP8266_STAT_OFFSET_AP_STATUS		 5
#define ESP8266_STAT_OFFSET_ACCEPTED		 0

#define ESP8266_STATUS_CONNECTED(x) 		((x>>ESP8266_STAT_OFFSET_CONNECTED) & ESP8266_STAUS_MASK)
#define ESP8266_STATUS_TO_SOCKET(x) 		((x>>ESP8266_STAT_OFFSET_TO_SOCKET) & ESP8266_STAUS_MASK)
#define ESP8266_STATUS_FROM_SERVER(x) 		((x>>ESP8266_STAT_OFFSET_FROM_SERVER) & ESP8266_STAUS_MASK)
#define ESP8266_STATUS_AP_STAUS(x)			((x>>ESP8266_STAT_OFFSET_AP_STATUS) & 7)
#define ESP8266_STATUS_ACCEPTED(x) 			((x>>ESP8266_STAT_OFFSET_ACCEPTED) & ESP8266_STAUS_MASK)

#define ESP8266_GET_STATUS_OK				0x80000000
#define ESP8266_STATUS_SOCK_CONECTED  		0x40000000

#define MODULE_SLEEP_TOUT 10 //0

#define WIFI_QUIT_AP			"+CWQAP"
#define WFI_MUX					"+CIPMUX"
#define WIFI_GET_LOCAL_IP		"+CIFSR"
#define WIFI_LIST_AVAILABLE_AP	"+CWLAP"
#define WIFI_DISCONNECT			"+CIPCLOSE"
#define WIFI_CONNECT			"+CIPSTART"
#define WIFI_SEND_DATA			"+CIPSEND"
#define WIFI_SERVER				"+CIPSERVER"
#define WIFI_SRVER_TIMEOUT		"+CIPSTO"
#define WIFI_CONNECTION_STAT	"+CIPSTATUS"

#if USE_DEPRECATED_AT_CMD
#define WIFI_BAUD				"+CIOBAUD"
#define WIFI_SOFT_AP_IP			"+CIPAP"
#define WIFI_MODE				"+CWMODE"
#define WIFI_SOFT_AP_CFG		"+CWSAP"
#define WIFI_JOIN_TO_AP			"+CWJAP"
	#define WIFI_CONNECTION_TYPE_INDX		5
#else

#if USE_CUR_SUF
#define WIFI_CUR_SUF
#else
#define WIFI_CUR_SUF			"_CUR"
#endif

#define WIFI_BAUD				"+UART_CUR"
#define WIFI_MODE				"+CWMODE" WIFI_CUR_SUF	// +CWMODE_CUR—Sets the Current Wi-Fi mode
#define WIFI_SOFT_AP_IP			"+CIPAP" WIFI_CUR_SUF	// +CIPAP_CUR—Sets/Gets the IP Address of the ESP8266 SoftAP
#define WIFI_SOFT_AP_CFG		"+CWSAP" WIFI_CUR_SUF	// +CWSAP_CUR—Configures the ESP8266 SoftAP
#define WIFI_JOIN_TO_AP			"+CWJAP" WIFI_CUR_SUF
	#define WIFI_CONNECTION_TYPE_INDX		6
#define WIFI_SET_LIST_OPTION	"+CWLAPOPT"
#define WIFI_STATION_IP			"+CIPSTA" WIFI_CUR_SUF	// Sets/Gets the current IP address of the station
#define WIFI_MDNS				"+MDNS"
#define WIFI_DHCP				"+CWDHCP" WIFI_CUR_SUF
#endif

enum wifi_notify_tyep_t:uint8_t
{
		WIFI_NOTIFY_IDLE=0,
		WIFI_NOTIFY_LINK,
		WIFI_NOTIFY_LINKED,
		WIFI_NOTIFY_UNLINK,
		WIFI_NOTIFY_REJECT_CONNECTION,
		WIFI_NOTIFY_WEB_AP_SET,
};

struct wifi_notify_t {
	uint8_t notification : 4;
	uint8_t link_id: 4;
}__attribute__((packed));

//=============================================================================
//	ESP8266 Access Point
//=============================================================================
struct esp8266_ap_t : public wifi_access_point_t
{

	esp8266_ap_t()
		:wifi_access_point_t()
	{ ; }

	~esp8266_ap_t()
	{ ; }

	void clear() override {
		wifi_access_point_t::clear();
	}

	bool from_str(const CSTRING& src) override;
	bool from_module(char* src) override;
	CSTRING& to_str(CSTRING& dst) const override;
};
//=============================================================================

struct esp8266_module: public wifi_module_type
{
    unsigned int 	used_sockets;
    unsigned int 	wifi_tout;
    CSTRING		 	connected_ssid;
    CSocket*	 	wait_for_connection;					 	//!< socket waiting for connection
    CSocket*	 	alloc_sockets[WIFI_ESP8266_MAX_SOCKETS];

    unsigned int 	received_size[WIFI_ESP8266_MAX_SOCKETS];
    char* 		 	received_data[WIFI_ESP8266_MAX_SOCKETS];
    bool 			closing_sockets[WIFI_ESP8266_MAX_SOCKETS];	//!< array of flags for sockets that will be closed
    unsigned short 	link_to_socket[WIFI_ESP8266_MAX_SOCKETS]; 	//!< array of relationships between connection ID and socket ID
#if USE_WIFI_LISTEN
    CSocket*	 	listen_socket;
    unsigned short 	listen_port;
#endif
#if ESP8266_WEB_SERVER
    esp8266_ap_t 	web_ap;
#endif
    mqueue<wifi_notify_t, 8> notify_state;
    unsigned int  	wifi_pin_pwr;
    unsigned int  	wifi_pin_boot;
    unsigned int 	at_version;

    esp8266_module(const WIFI_DRIVER_INFO* pinfo, unsigned pwr_pin =0, unsigned rst_pin=0, unsigned boot_pin =0)
    	:wifi_module_type(pinfo, rst_pin)
		{
				used_sockets = 0;
				wifi_tout = 0;
				wait_for_connection = nullptr;
#if USE_WIFI_LISTEN
				listen_socket = nullptr;
				listen_port = 0;
#endif
				for(int i=0; i<WIFI_ESP8266_MAX_SOCKETS; i++)
				{
					alloc_sockets[i]   = nullptr;
					received_data[i]   = nullptr;
					received_size[i]   = 0;
					link_to_socket[i]  = WIFI_ESP8266_MAX_SOCKETS; // invalid socket id
					closing_sockets[i] = false;
				}
				wifi_pin_pwr = pwr_pin;
				wifi_pin_boot = boot_pin;
				at_version =0;
		};
    friend wifi_module_type* wifi_detect(WIFI_DRIVER_INFO* drv_info);
protected:
    //=============================================================================
    //	ESP8266 status related methods
    //=============================================================================
    NET_CODE esp8266_xxx_ip(wifi_access_point_t& access_point, bool set, const char* esp_ip_cmd);
    NET_CODE esp8266_station_ip(wifi_access_point_t& access_point, bool set = false);
    NET_CODE esp8266_soft_ap_ip(wifi_access_point_t& access_point, bool set = false);
    unsigned int esp8266_connections_status(unsigned int sock_id, ip_addr_t* ip_ptr=nullptr);
    NET_CODE wifi_get_current_net_ssid(wifi_access_point_t& access_point, bool check_only=false) override;
    //=============================================================================
    //	ESP8266 Helpers
    //=============================================================================
	bool esp8266_socket_to_connection(const unsigned sid, unsigned& link_id ) const;
	static bool esp8266_esc_syntax(const CSTRING& src, CSTRING& dst);
    NET_CODE esp8266_mdns(bool mdn_on = false);
    bool esp8266_get_version();
    //=============================================================================
    //	Methods for powering on/off/resetting the ESP8266 and related to its state.
    //=============================================================================
	RES_CODE wifi_echo_off(bool lowlevel, uint32_t indx);
    RES_CODE wifi_drv_pwron(bool lowlevel = false) override;
    RES_CODE wifi_drv_off() override;
    NET_CODE wifi_reset(bool force, wifi_module_type** drv_module) override;
    NET_CODE wifi_drv_level() override;
    //=============================================================================
    //	CONFIG commands
    //=============================================================================
    NET_CODE esp8266_get_available_ap(CSTRING& list);
    NET_CODE esp8266_change_ap(esp8266_ap_t& current_ap, esp8266_ap_t& target_ap);
    RES_CODE wifi_cmd_config(CSocket* sock) override;
    //=============================================================================
    // NOTIFICATION / TIMEOUT / CANCEL
    //=============================================================================
    int  wifi_notification(const char* row) override;
    void wifi_notificatoin_response() override;
    void wifi_process_tout() override;
    void wifi_cancelation(bool all_station, bool all_softAP) override;
    void process_input(unsigned int signals, const char* cmd, const char* hnd_start=nullptr) override;
    //=============================================================================
    // connecting to AP
    //=============================================================================
    NET_CODE esp8266_connect_to_network();
    NET_CODE esp8266_connect_to_ap(wifi_access_point_t& target_ap);
    //=============================================================================
    // methods related to OPENING a socket
    //=============================================================================
	NET_CODE esp8266_allocate_socket(CSocket* sock);
    NET_CODE esp8266_socket_open(CSocket* sock);
    //=============================================================================
    // methods related to CLOSING a socket
    //=============================================================================
    void esp8266_discarding_connected_sockets();
    void wifi_driver_socket_close(unsigned int sid, unsigned int reason);
    NET_CODE esp8266_close_connection(unsigned link_id);
    NET_CODE esp8266_socket_close(unsigned int sid);
#if USE_WIFI_LISTEN
    RES_CODE wifi_close_listen(CSocket* sock, unsigned int reason);
#endif
    //=============================================================================
    //	Receiving/reading data
    //=============================================================================
#if USE_DEPRECATED_AT_CMD
    RES_CODE wifi_receive_check(char sym);
#endif
    bool wifi_data_received(const char* row) override;
#if USE_WIFI_ESP8266 < 3 // version 3.0
    bool is_data_received(unsigned char sock_state);
#endif

public:
    RES_CODE process_cmd(HANDLE client) override;
//------------------- socket commands -----------------------------------------
    RES_CODE process_read(CSocket* sock) override;
    RES_CODE process_write(CSocket* sock) override;
    RES_CODE wifi_sock_cmd_open(CSocket* sock) override;
    RES_CODE wifi_sock_cmd_connect_adr(CSocket* sock) override;
    RES_CODE wifi_sock_cmd_connect_url(CSocket* sock) override;
    RES_CODE wifi_sock_cmd_disconect(CSocket* sock) override;
    RES_CODE wifi_sock_cmd_close(CSocket* sock) override;
    RES_CODE wifi_gethostbyname(CSocket* sock) override;
#if USE_WIFI_LISTEN
    RES_CODE wifi_sock_cmd_bind_adr(CSocket* sock) override;
    RES_CODE wifi_sock_cmd_bind_url(CSocket* sock) override;
    RES_CODE wifi_sock_cmd_listen(CSocket* sock) override;
    RES_CODE wifi_sock_cmd_accept(CSocket* sock) override;
    RES_CODE wifi_sock_cmd_addr(CSocket* sock) override;
#endif
//-----------------------------------------------------------------------------
    RES_CODE module_upgrade(HANDLE hnd) override;
};




#endif /* ESP8266_H_ */

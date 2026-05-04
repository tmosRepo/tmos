/*
 * core_wifi.h
 *
 */

#ifndef CORE_WIFI_H_
#define CORE_WIFI_H_

#include <tmos.h>
#include <wifi_opt.h>
#include <csocket.h>

#if USE_LWIP_1_4_1 || USE_LWIP_2_2_0
#include <lwip/ip_addr.h>
#else
#error "LwIP must be enabled!"
#endif

#ifndef CHAR_STX
#define CHAR_STX 2
#endif
#ifndef CHAR_ETX
#define CHAR_ETX 3
#endif
#ifndef STR_STX
#define STR_STX "\x02"
#endif
#ifndef STR_ETX
#define STR_ETX "\x03"
#endif

enum wifi_encryption_type_t : int8_t
{
	enc_unknown = -1,			// -1: invalid
	enc_open =0,				//  0: OPEN
	enc_wep_enc,				//  1: WEP
	enc_wpa_psk,				//  2: WPA_PSK
	enc_wpa2_psk,				//  3: WPA2_PSK
	enc_wpa_wpa2_psk,			//  4: WPA_WPA2_PSK
	enc_wpa2_enterprise,		//  5: WPA2_Enterprise (AT can NOT connect to WPA2_Enterprise AP)
	enc_wpa3_psk,				//  6: WPA3_PSK
	enc_wpa2_wpa3_psk,			//  7: WPA2_WPA3_PSK
	enc_wapi_psk,				//  8: WAPI_PSK
	enc_owe,					//  9: OWE
	enc_wpa3_ent_192,			// 10: WPA3_ENT_192
	enc_wpa3_ext_psk,			// 11: WPA3_EXT_PSK
	enc_wpa3_ext_psk_mixed_mode,// 12: WPA3_EXT_PSK_MIXED_MODE
	enc_dpp,					// 13: DPP
	enc_wpa3_enterprise,		// 14: WPA3_ENTERPRISE
	enc_wpa2_wpa3_enterprise,	// 15: WPA2_WPA3_ENTERPRISE
	enc_last_supported
};

#define WIFI_AP_FLAG_STATIC_IP		0x0001
#define WIFI_AP_FLAG_MDNS			0x0002

#define WIFI_AP_FLAGS_INFO			0xF000
#define WIFI_AP_FLAG_NO_INTERNET	0x8000

struct WIFI_DRIVER_INFO;

const char *skip_cmd_echo(const char* src);
bool wifi_get_param(const char*row, CSTRING& param, unsigned int num);
bool wifi_get_param(const char*row, unsigned int& param, unsigned int num);
bool wifi_get_param(const char*row, int& param, unsigned int num);
bool wifi_get_param(const char*row, int8_t& param, unsigned int num);
bool wifi_get_param(const char*row, wifi_encryption_type_t& param, unsigned int num);

uint8_t wifi_rssi_to_level(const int8_t rssi );

CSTRING wifi_ip_to_str(const ip_addr_t* adr, bool quotes = false);

struct wifi_access_point_t
{
//private:
//	static uint32_t	ap_cnt;
public:
	CSTRING ssid;		//!< When the access point has a hidden SSID, the field is blank and must be set before connecting.
						///  In case the access point's SSID is not hidden, the module provides this field.
	CSTRING pwd;		//!< If the access point has encryption, this field must be set before connecting, otherwise it is blank.
	CSTRING mac;		//!< The MAC address of the access point, the module provides this field
	struct {
		wifi_encryption_type_t 	ap_enc;		//!< Encryption type supported by access point, this field is set by the module
		int8_t					ap_rssi;	//!< Signal strength in dBm, this field is set by the module
		uint16_t 				ap_flags;
	}__attribute__((packed));
	struct {
		union {
		struct {
			ip_addr_t ip;
			ip_addr_t mask;
			ip_addr_t gw;
		};
		ip_addr_t ip_arr[3];
		};
	}__attribute__((packed, aligned(4)));

	wifi_access_point_t()
		: ap_enc(enc_unknown)
		, ap_rssi(-128)
		, ap_flags(0)
		, ip(ip_addr_any), mask(ip_addr_any), gw(ip_addr_any)
	{;}

	virtual ~wifi_access_point_t()
	{;}

	virtual void clear() {
		ssid.clear(); pwd.clear(); mac.clear();
		ap_enc = enc_unknown;
		ap_rssi = -128;
		ap_flags =0;
		ip = ip_addr_any; mask = ip_addr_any; gw = ip_addr_any;
	}

	virtual bool from_str(const CSTRING& src) =0;
	virtual bool from_module(char* src) =0;
	virtual CSTRING& to_str(CSTRING& dst)  const =0;

	bool operator== (const wifi_access_point_t& ap) const
	{
		if (this == &ap) {
			return true;
		} else if( mac != ap.mac) {
			return false;
		} else if( ssid != ap.ssid) {
			return false;
		} else if( ap_enc != ap.ap_enc) {
			return false;
		} else if( pwd != ap.pwd) {
			return false;
		} else if ( (ap_flags ^ ap.ap_flags) & ~WIFI_AP_FLAGS_INFO ) {
			return false;
		}
		if (ap_flags & WIFI_AP_FLAG_STATIC_IP) {
			for(size_t i=0; i < sizeof(ip_arr)/sizeof(ip_addr_t); i++) {
				if (!ip_addr_cmp(ip_arr+i, ap.ip_arr+i))
					return false;
			}
		}
		return true;
	}

};
// ---------- MDNS descriptor ------------------------------------------------//
struct mdns_text_t
{
	CSTRING key;
	CSTRING value;
	mdns_text_t()
	{;}
};

enum mdns_protocol_t : uint8_t
{
	mdns_tcp,
	mdns_udp
};


//AT+MDNS=<mode>[,<"hostname">,<"service_type">,<port>][,<"instance">][,<"proto">][,<txt_number>][,<"key">,<"value">][...]
struct mdns_desc_t {
	static const char* protocol_type[2];
	static mdns_text_t nil;
	CSTRING			hostname;
	CSTRING 		service;
	CSTRING			instance;
	uint16_t		port;
	mdns_protocol_t protocol;

	virtual uint32_t get_txt_num() {return 0;}
	virtual mdns_text_t& get_text(uint32_t id) { return nil;}
	virtual void set_text(size_t num, const char* _key, const char* _val) {;}
	mdns_desc_t()
	: port(0), protocol(mdns_tcp)
	{;}

	mdns_desc_t(const char* _host, const char* _service, uint16_t _port = 0, mdns_protocol_t _protocol=mdns_tcp)
	: hostname(_host) , service(_service), port(_port), protocol(_protocol)
	{;}
	virtual ~mdns_desc_t()
	{;}
};

template <size_t size>
struct user_mdns_desc_t : mdns_desc_t
{
	size_t		text_num;
	mdns_text_t text[size];
	user_mdns_desc_t() : mdns_desc_t()
	, text_num(size)
	{;}
	virtual ~user_mdns_desc_t()
	{;}
	mdns_text_t& get_text(uint32_t id) override
	{
		if (id < size) {
			return text[id];
		} else {
			return nil;
		}
	}
	uint32_t get_txt_num() override
	{
		return size;
	}
	void set_text(size_t num, const char* _key, const char* _val) override
	{
		if(num < size) {
			text[num].key = _key;
			text[num].value = _val;
		}
	}
};

// ----------        AT Commands API	--------------------------------------//
typedef unsigned int WIFI_CMD_STATE;

#define WIFI_CMD_STATE_IDLE		0x00
#define WIFI_CMD_STATE_STARTED	0x01	//!< something has been received
#define WIFI_CMD_STATE_UNK		0x02	//!< Unknown row received
#define WIFI_CMD_STATE_HND		0x04	//!< exit wifi_send_cmd() on special character
#define WIFI_CMD_STATE_ROW_STOP	0x08	//!< exit wifi_send_cmd() on first answer row
#define WIFI_CMD_STATE_RETURNED	0x10	//!< one or more answer rows recognized
#define WIFI_CMD_STATE_OK		0x20	//!< OK answer received
#define WIFI_CMD_STATE_CMES		0x40	//!< CMEE or CMES answer received
#define WIFI_CMD_STATE_FATAL	0x80	//!< failed to send command
#define WIFI_CMD_STATE_CRLF		0x100	//!< CRLF received first
#define WIFI_CMD_STATE_CRLFOK	0x200	//!< CRLF OK CRLF received


#define WIFI_CMD_STATE_RETURN	(WIFI_CMD_STATE_RETURNED | WIFI_CMD_STATE_HND)
#define WIFI_CMD_STATE_ROK		(WIFI_CMD_STATE_RETURNED | WIFI_CMD_STATE_OK)
#define WIFI_CMD_SEND_STATUS	(WIFI_CMD_STATE_ROK | WIFI_CMD_STATE_CMES | WIFI_CMD_STATE_FATAL)

#define WIFI_ON_REGISTER_CMDS		1


struct wifi_module_type
{
	const WIFI_DRIVER_INFO* drv_info;

    unsigned short 	row_start;
    unsigned short 	row_end;
    unsigned short 	cmd_state;  // WIFI_CMD_STATE_XXX
    unsigned char 	received_ch;
    unsigned char 	wifi_watchdog_cnt;
    bool			in_notification;
    unsigned		last_timeout;

    CHandle rcv_hnd;
    CHandle snd_hnd;
#if WIFI_FLOW_CONTROL
    bool 		  stop_rcv;
    unsigned int  mem_alloc_size;
    HANDLE	 	 pending_read_data;
#endif
    unsigned int  wifi_pin_rts;
    char buf[WIFI_BUF_SIZE];

//----------------- constructor/destructor -------------------------------------
    wifi_module_type
    	(
    		const WIFI_DRIVER_INFO* pinfo
    		,unsigned int PIN_RTS
    	)
    	:drv_info(pinfo)
    	, row_start(0)
    	, row_end(0)
    	, cmd_state(0)
    	, received_ch(0)
    	, wifi_watchdog_cnt(0)
    	, in_notification(false)
    	, last_timeout(CURRENT_TIME)
#if WIFI_FLOW_CONTROL
    	, stop_rcv(false)
    	, mem_alloc_size(0),
    	pending_read_data(nullptr)
#endif
    	, wifi_pin_rts(PIN_RTS)
    {
    }

    virtual ~wifi_module_type()
    {
    	//module_off();
    }

//---------------- start virtual methods (members if any) ----------------------
    virtual RES_CODE wifi_drv_pwron(bool lowlevel = false)=0;
    virtual RES_CODE wifi_drv_off()=0;
    virtual NET_CODE wifi_reset(bool force, wifi_module_type** driver_module)=0;
    virtual NET_CODE wifi_drv_level()=0;

    virtual void process_input(unsigned int signals, const char* cmd, const char* hnd_start=nullptr) =0;
    virtual int  wifi_notification(const char* row);
    virtual void wifi_notificatoin_response()
    {;}
    virtual void wifi_process_tout();
    virtual void wifi_cancelation(bool all_station, bool all_softAP);
    virtual bool wifi_data_received(const char* row)
    {
    	return false;
    }

    virtual WIFI_CMD_STATE wifi_process_row(const char *cmd);
    virtual WIFI_CMD_STATE wifi_send_cmd(const char *cmd, unsigned int time);
    virtual RES_CODE process_cmd(HANDLE client);
//---------------- socket commands --------------------------------------------
    virtual RES_CODE process_read(CSocket* sock) =0;
    virtual RES_CODE process_write(CSocket* sock) =0;
    virtual RES_CODE wifi_sock_cmd_open(CSocket* sock)=0;
    virtual RES_CODE wifi_sock_cmd_connect_adr(CSocket* sock)=0;
    virtual RES_CODE wifi_sock_cmd_connect_url(CSocket* sock)=0;
    virtual RES_CODE wifi_sock_cmd_disconect(CSocket* sock)=0;
    virtual RES_CODE wifi_sock_cmd_close(CSocket* sock)=0;
    virtual RES_CODE wifi_gethostbyname(CSocket* sock)=0;
#if USE_WIFI_LISTEN
    virtual RES_CODE wifi_sock_cmd_bind_adr(CSocket* sock)=0;
    virtual RES_CODE wifi_sock_cmd_bind_url(CSocket* sock)=0;
    virtual RES_CODE wifi_sock_cmd_listen(CSocket* sock)=0;
    virtual RES_CODE wifi_sock_cmd_accept(CSocket* sock)=0;
    virtual RES_CODE wifi_sock_cmd_addr(CSocket* sock)=0;
#endif
    virtual RES_CODE wifi_cmd_config(CSocket* sock)
    	{return NET_ERR_PHY_NOT_READY;}
    virtual NET_CODE wifi_get_current_net_ssid(wifi_access_point_t& access_point, bool check_only=false);
    virtual RES_CODE module_upgrade(HANDLE hnd)
    	{return NET_ERR_PHY_NOT_READY;}
//---------------- end virtual methods (members if any) ------------------------

    char* get_str_cmd(const char *cmd, unsigned int time);
    char* get_str_prm(char *row, unsigned int param);
    NET_CODE wifi_drv_on();

//    friend RES_CODE wifi_drv_off(wifi_module_type *module, HANDLE hnd);
//    friend void wifi_thread(WIFI_DRIVER_INFO* drv_info);
//protected:
    static bool cmd_match(const char* cmd, const char* row);
    static bool cmd_submatch(const char* cmd, const char* row);

    NET_CODE wifi_check_reg();
    void wifi_sleep(unsigned int time);
    RES_CODE wifi_error(int err);
    NET_CODE wifi_net_error(int err);
    void hnd_error(HANDLE hnd) const;
};

// type definition for the commands
typedef RES_CODE (*WIFI_CBF)(wifi_module_type *module, HANDLE hnd);

#define WIFI_CMD_CONFIG			(( 12 <<4)+CMD_COMMAND)
#define WIFI_DRV_ON_CMD 		(( 13 <<4)+CMD_COMMAND)
#define WIFI_DRV_UPGRADE 		(( 14 <<4)+CMD_COMMAND)
#define WIFI_DRV_OFF_CMD 		(( 15 <<4)+CMD_COMMAND)

// WIFI_CMD_CONFIG params
#define WIFI_COFIG_PARAM_LIST			1	// list of available access points
#define WIFI_COFIG_PARAM_CONNECT_TO_AP	2	// connect to access points
#define WIFI_COFIG_PARAM_GET_CURRENT_AP 3   // get the currently used access point, if any.
#define WIFI_COFIG_PARAM_DISCONNECT_AP  4   // disconnect from the access point
#define WIFI_COFIG_PARAM_SET_SERVER		5	// start/stop server

/** auxiliary function for customizing initialization
 * called after common initialization is completed
 */
extern "C" void wifi_on_pwron_config(wifi_module_type* mod);
/** auxiliary function for customizing settings when turning the module on/off
 *	called after the module is turned on
 *	called before the module is shut down
 */
struct WIFI_DRIVER_DATA;
extern "C" void wifi_notify_power(wifi_module_type* mod);
/**  auxiliary function to customize or cancel the shutdown of the module.
 *   called before starting the shutdown.
 *   If the return code is other than NET_OK, the shutdown will be canceled.
 */
extern "C" NET_CODE wifi_on_deregister(wifi_module_type* mod);
/**
 *
 * @param mod
 * @param sock
 * @param network
 * @return
 */
extern "C" NET_CODE wifi_on_init_station(wifi_module_type* mod, CSocket* sock, wifi_access_point_t* network);
extern "C" NET_CODE wifi_on_get_station_net(wifi_module_type* mod, CSocket* sock,  wifi_access_point_t* network);
extern "C" void wifi_on_disconnect(wifi_module_type* mod);
extern "C" void wifi_on_blink_transfer(wifi_module_type* mod, int reason);
extern "C" mdns_desc_t* wifi_get_mdns_desc(wifi_module_type* mod);

extern "C" NET_CODE is_ap_cnown(const wifi_access_point_t& available_ap, wifi_access_point_t& known_ap, const CSTRING& already_passed);

#endif /* CORE_WIFI_H_ */

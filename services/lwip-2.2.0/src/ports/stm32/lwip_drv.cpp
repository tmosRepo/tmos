/*
 * lwip_drv.cpp
 *
 *  Created on: 22.03.2011
 *      Author: Miroslav Kostadinov
 */

#include <tmos.h>
#include "lwip/opt.h"
#include "lwip/sys.h"
#include <lwip_drv.h>
#include "netif/ethernet.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/autoip.h"
#include "lwip/dhcp.h"
#include "lwip/init.h"
#include "lwip/stats.h"
#include "lwip/netif.h"

#include "lwip/timeouts.h"
#include "lwip/snmp.h"

#include "lwip/apps/mdns.h"


#include <lwip_api.h>
#include <tmos_drivers.h>



#if LWIP_RX_L2_QUE || LWIP_TX_L2_QUE
/**
 * Push a pbuf packet onto a pbuf packet queue
 *
 * @param p is the pbuf to push onto the packet queue.
 * @param q is the packet queue.
 *
 * @return 1 if successful, 0 if q is full.
 */
static int enqueue_packet(struct pbuf *p, struct pbufq *q)
{
	SYS_ARCH_DECL_PROTECT(lev);
	int ret;

	/**
	 * This entire function must run within a "critical section" to preserve
	 * the integrity of the transmit pbuf queue.
	 *
	 */
	SYS_ARCH_PROTECT(lev);

	if (!PBUF_QUEUE_FULL(q))
	{
		/**
		 * The queue isn't full so we add the new frame at the current
		 * write position and move the write pointer.
		 *
		 */
		q->pbuf[q->qwrite] = p;
		q->qwrite = ((q->qwrite + 1) % STELLARIS_NUM_PBUF_QUEUE);
		ret = 1;
	}
	else
	{
		/**
		 * The stack is full so we are throwing away this value.  Keep track
		 * of the number of times this happens.
		 *
		 */
		q->overflow++;
		ret = 0;
	}

	/* Return to prior interrupt state and return the pbuf pointer. */
	SYS_ARCH_UNPROTECT(lev);
	return (ret);
}


/**
 * Pop a pbuf packet from a pbuf packet queue
 *
 * @param q is the packet queue from which to pop the pbuf.
 *
 * @return pointer to pbuf packet if available, NULL otherswise.
 */
static struct pbuf* dequeue_packet(struct pbufq *q)
{
	struct pbuf *pBuf;
	SYS_ARCH_DECL_PROTECT(lev);

	/**
	 * This entire function must run within a "critical section" to preserve
	 * the integrity of the transmit pbuf queue.
	 *
	 */
	SYS_ARCH_PROTECT(lev);

	if (PBUF_QUEUE_EMPTY(q))
	{
		/* Return a NULL pointer if the queue is empty. */
		pBuf = (struct pbuf *) NULL;
	}
	else
	{
		/**
		 * The queue is not empty so return the next frame from it
		 * and adjust the read pointer accordingly.
		 *
		 */
		pBuf = q->pbuf[q->qread];
		q->qread = ((q->qread + 1) % STELLARIS_NUM_PBUF_QUEUE);
	}

	/* Return to prior interrupt state and return the pbuf pointer. */
	SYS_ARCH_UNPROTECT(lev);
	return (pBuf);
}
#endif

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf might be
 * chained.
 *
 */
static err_t low_level_transmit(struct netif *netif, struct pbuf *p)
{
	const LWIP_DRIVER_INFO* drv_info = ((LWIP_DRIVER_DATA*)netif)->drv_info;
	ETH_DMADescTypeDef* DmaTxDesc;
	err_t errval;
	uint32_t byteslefttocopy;
	uint32_t payloadoffset;
	uint32_t bufferoffset;
	uint32_t framelength = 0;
	struct pbuf *q;

	DmaTxDesc = drv_info->mac_cfg->mac_data->TxDesc;
	if (DmaTxDesc->Status & ETH_DMATXDESC_OWN)
	{
		errval = ERR_USE;
	} else
	{
		errval = ERR_OK;
		bufferoffset = 0;

		/* copy frame from pbufs to driver buffers */
		for (q = p; q != NULL; q = q->next)
		{

			/* Get bytes in current lwIP buffer */
			byteslefttocopy = q->len;
			payloadoffset = 0;

			/* Check if the length of data to copy is bigger than Tx buffer size*/
			while ((byteslefttocopy + bufferoffset) > ETH_TX_BUF_SIZE)
			{
				/* Copy data to Tx buffer*/
				memcpy( DmaTxDesc->Buffer1Addr + bufferoffset, (uint8_t*) q->payload + payloadoffset,
						ETH_TX_BUF_SIZE - bufferoffset);

				/* Point to next descriptor */
				DmaTxDesc = DmaTxDesc->Buffer2NextDescAddr;

				/* Check if the buffer is available */
				if (DmaTxDesc->Status & ETH_DMATXDESC_OWN)
				{
					errval = ERR_USE;
					break;
				}


				byteslefttocopy -=  (ETH_TX_BUF_SIZE - bufferoffset);
				payloadoffset +=  (ETH_TX_BUF_SIZE - bufferoffset);
				framelength += (ETH_TX_BUF_SIZE - bufferoffset);
				bufferoffset = 0;
			}
			if(errval != ERR_OK)
				break;

			/* Copy the remaining bytes */
			memcpy( DmaTxDesc->Buffer1Addr + bufferoffset,
					(uint8_t*) q->payload + payloadoffset,	byteslefttocopy);
			bufferoffset += byteslefttocopy;
			framelength += byteslefttocopy;
		}
	}

	if(errval == ERR_OK)
	{
		/* Dereference the pbuf from the queue. */
		pbuf_free(p);

		LINK_STATS_INC(link.xmit);
	} else
	{
		framelength = 0;
	}
	HAL_ETH_TransmitFrame(drv_info->hw_base, drv_info->mac_cfg->mac_data, framelength);

	return errval;

}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
#if LWIP_TX_L2_QUE
	LWIP_DRIVER_DATA* drv_data = (LWIP_DRIVER_DATA*)netif;
#endif
	err_t errval;
	SYS_ARCH_DECL_PROTECT(lev);

	/**
	 * This entire function must run within a "critical section" to preserve
	 * the integrity of the transmit pbuf queue.
	 *
	 */
	SYS_ARCH_PROTECT(lev);

	/**
	 * Bump the reference count on the pbuf to prevent it from being
	 * freed till we are done with it.
	 *
	 */
	pbuf_ref(p);

	/**
	 * If the transmitter is idle, and there is nothing on the queue,
	 * send the pbuf now.
	 *
	 */
	errval = low_level_transmit(netif, p);
	LWIP_DEBUGF(ETH_DEBUG, ("ETH: tx %x %u res %u\n", p, p->tot_len, errval));
	if(errval != ERR_OK)
	{
#if LWIP_TX_L2_QUE
		/* Otherwise place the pbuf on the transmit queue. */
		if (!enqueue_packet(p, &drv_data->txq))
		{
			/* if no room on the queue, free the pbuf reference and return error. */
			pbuf_free(p);
			errval = ERR_MEM;
		}
#else
		pbuf_free(p);
#endif

	}


	/* Return to prior interrupt state and return. */
	SYS_ARCH_UNPROTECT(lev);
	return (errval);
}


static struct pbuf *low_level_receive(struct netif *netif)
{
	const LWIP_DRIVER_INFO* drv_info = ((LWIP_DRIVER_DATA*) netif)->drv_info;
	struct pbuf *p, *q;
	ETH_DMADescTypeDef *dmarxdesc;
	uint32_t dtors;
	uint32_t bufferoffset, byteslefttocopy, payloadoffset;

	p = NULL;
	dtors = HAL_ETH_GetReceivedFrame_IT(drv_info->mac_cfg);
	if(dtors)
	{
	    /* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
	    p = pbuf_alloc(PBUF_RAW, drv_info->mac_cfg->mac_data->rx_len, PBUF_POOL);
	    if(p)
	    {
		    dmarxdesc = drv_info->mac_cfg->mac_data->RxDesc;
		    bufferoffset = 0;

		    for(q = p; q != NULL; q = q->next)
		    {
		      byteslefttocopy = q->len;
		      payloadoffset = 0;

		      /* Check if the length of bytes to copy in current pbuf is bigger than Rx buffer size*/
		      while( (byteslefttocopy + bufferoffset) > ETH_RX_BUF_SIZE )
		      {
		        /* Copy data to pbuf*/
		        memcpy( (u8_t*)q->payload + payloadoffset, dmarxdesc->Buffer1Addr + bufferoffset, (ETH_RX_BUF_SIZE - bufferoffset));

		        /* Point to next descriptor */
		        dmarxdesc = dmarxdesc->Buffer2NextDescAddr;

		        byteslefttocopy -=  (ETH_RX_BUF_SIZE - bufferoffset);
		        payloadoffset +=  (ETH_RX_BUF_SIZE - bufferoffset);
		        bufferoffset = 0;
		      }

		      /* Copy remaining data in pbuf */
		      memcpy( (u8_t*)q->payload + payloadoffset, dmarxdesc->Buffer1Addr + bufferoffset, byteslefttocopy);
		      bufferoffset = bufferoffset + byteslefttocopy;
		    }
			/* Adjust the link statistics */
			LINK_STATS_INC(link.recv);
	    } else
	    {
	    	// out of memory -> drop the frame
	        LINK_STATS_INC(link.memerr);
	        LINK_STATS_INC(link.drop);
	    }
    	HAL_ETH_ReleaseFrame(drv_info->mac_cfg->mac_data, dtors);
	}
	  if (drv_info->hw_base->ETH_DMASR & ETH_DMASR_RBUS)
	  {
	    /* Clear RBUS ETHERNET DMA flag */
		  drv_info->hw_base->ETH_DMASR = ETH_DMASR_RBUS;
	  }
	    /* Resume DMA reception */
	  drv_info->hw_base->ETH_DMARPDR = 0;
	return p;
}


/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void low_level_init(struct netif *netif)
{
	LWIP_DRIVER_DATA* drv_data = (LWIP_DRIVER_DATA*)netif;
	const LWIP_DRIVER_INFO* drv_info = drv_data->drv_info;

	EthernetMACAddrGet(drv_info->mac_cfg);

	/* configure ethernet peripheral (GPIOs, clocks, MAC, DMA) */
	HAL_ETH_Init(drv_info->hw_base, drv_info->mac_cfg);

	/* Initialize Tx Descriptors list: Chain Mode */
	HAL_ETH_DMATxDescListInit(drv_info->hw_base, drv_info->mac_cfg);

	/* Initialize Rx Descriptors list: Chain Mode  */
	HAL_ETH_DMARxDescListInit(drv_info->hw_base, drv_info->mac_cfg);

	/* set MAC hardware address length */
	netif->hwaddr_len = ETHARP_HWADDR_LEN;

	/* set netif MAC hardware address */
	memcpy(netif->hwaddr, drv_info->mac_cfg->mac_data->mac_adr, ETHARP_HWADDR_LEN);


	/* maximum transfer unit */
	netif->mtu = 1500;

	/* device capabilities */
	/* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
	netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP;

	  /**** Configure PHY to generate an interrupt when Eth Link state changes ****/
	HAL_ETH_PHY_INTs(drv_info->hw_base, drv_info->mac_cfg);


	/* Enable the Ethernet global Interrupt */
	drv_enable_isr(&drv_info->info);
}

WEAK_C void lwIPHostTimerHandler(void)
{
	//TRACE1("lwIP tmr");
}

WEAK_C char * get_netif_host_name()
{
	return nullptr;
}


/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t ethernetif_init(struct netif *netif)
{
#if LWIP_RX_L2_QUE || LWIP_TX_L2_QUE
	LWIP_DRIVER_DATA* drv_data = (LWIP_DRIVER_DATA*)netif;
#endif

	LWIP_ASSERT("netif != NULL", (netif != NULL));

#if LWIP_NETIF_HOSTNAME
	/* Initialize interface hostname */
	netif->hostname = get_netif_host_name();
#endif /* LWIP_NETIF_HOSTNAME */

	/*
	 * Initialize the snmp variables and counters inside the struct netif.
	 * The last argument should be replaced with your link speed, in units
	 * of bits per second.
	 */
	NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, 1000000);

//	netif->state = &ethernetif_data;
	netif->name[0] = IFNAME0;
	netif->name[1] = IFNAME1;
	/* We directly use etharp_output() here to save a function call.
	 * You can instead declare your own function an call etharp_output()
	 * from it if you have to do some checks before sending (e.g. if link
	 * is available...) */
	netif->output = etharp_output;
	netif->linkoutput = low_level_output;

//	drv_data->ethaddr = (struct eth_addr *) &(netif->hwaddr[0]);
#if LWIP_TX_L2_QUE
	drv_data->txq.qread = drv_data->txq.qwrite = 0;
	drv_data->txq.overflow = 0;
#endif
#if LWIP_RX_L2_QUE
	drv_data->rxq.qread = drv_data->rxq.qwrite = 0;
	drv_data->rxq.overflow = 0;
#endif

	/* initialize the hardware */
	low_level_init(netif);

	return (ERR_OK);
}

extern "C" void on_netif_status_changed(netif* netif, net_interface_status_t reason)
	__attribute__ ((weak, alias ("default_netif_status_changed")));
extern "C" void default_netif_status_changed(netif* netif, net_interface_status_t reason)
{
#if USE_LWIP_MDNS
	switch(reason){
	case net_intrface_up:
		mdns_resp_restart(netif);
		break;
	case net_intrface_down:
		break;
	case net_intrface_addres_changed:
		mdns_resp_announce(netif);
		break;
	}
#endif
}
// callback to be called when interface is brought up/down or address is changed while up
void ethernetif_status_changed(struct netif* netif)
{
	static bool netif_up = false;
	if(netif_is_up(netif)){
		if(netif_up){
			on_netif_status_changed(netif, net_intrface_addres_changed);
		}else{
			netif_up = true;
			on_netif_status_changed(netif, net_intrface_up);
		}
	}else{
		netif_up = false;
		on_netif_status_changed(netif, net_intrface_down);
	}
}

void ethernetif_link_status(LWIP_DRIVER_INFO* drv_info)
{
	uint32_t regvalue = 0;

	if(HAL_ETH_PHY_INT_LINK_STATUS(drv_info->hw_base,
			drv_info->mac_cfg, &regvalue) == RES_OK)
	{
		if (regvalue )
		{
			netif_set_link_up(&drv_info->drv_data->lwip_netif);
		}
		else
		{
			netif_set_link_down(&drv_info->drv_data->lwip_netif);
		}

	}
}

#if LWIP_NETIF_LINK_CALLBACK
/**
  * @brief  This function notify user about link status changement.
  * @param  netif: the network interface
  * @retval None
  */
WEAK void ethernetif_notify_conn_changed(struct netif *netif)
{
  /* NOTE : This is function clould be implemented in user file
            when the callback is needed,
  */
}

/**
  * @brief  Link callback function, this function is called on change of link status
  *         to update low level driver configuration.
  * @param  netif: The network interface
  * @retval None
  */
void lwip_sock_discard(struct netif *netif);

void ethernetif_update_config(struct netif *netif)
{
	LWIP_DRIVER_DATA* drv_data = (LWIP_DRIVER_DATA*)netif;
	const LWIP_DRIVER_INFO* drv_info = drv_data->drv_info;

	if (netif_is_link_up(netif))
	{
		HAL_ETH_Start (drv_info->hw_base);

		// Start DHCP, if enabled.
#if LWIP_DHCP
		if (drv_data->ip_addr_mode == IPADDR_USE_DHCP)
		{
			dhcp_start(netif);
		}
#endif

		// Start AutoIP, if enabled and DHCP is not.
#if LWIP_AUTOIP
		if (drv_data->ip_addr_mode == IPADDR_USE_AUTOIP)
		{
			autoip_start(netif);
		}
#endif
	}
	else
	{
		// Stop DHCP, if enabled.
#if LWIP_DHCP
		if (drv_data->ip_addr_mode == IPADDR_USE_DHCP)
		{
			dhcp_stop(netif);
		}
#endif

		// Stop AutoIP, if enabled and DHCP is not.
#if LWIP_AUTOIP
		if (drv_data->ip_addr_mode == IPADDR_USE_AUTOIP)
		{
			autoip_stop(netif);
		}
#endif
		/* Stop MAC interface */
		HAL_ETH_Stop(drv_info->hw_base);

		lwip_sock_discard(netif);
	}

	ethernetif_notify_conn_changed(netif);
}
#endif /* LWIP_NETIF_LINK_CALLBACK */

//*****************************************************************************
//
//! Initializes the lwIP TCP/IP stack.
//!
//! \param pucMAC is a pointer to a six byte array containing the MAC
//! address to be used for the interface.
//! \param ulIPAddr is the IP address to be used (static).
//! \param ulNetMask is the network mask to be used (static).
//! \param ulGWAddr is the Gateway address to be used (static).
//! \param ulIPMode is the IP Address Mode.  \b IPADDR_USE_STATIC will force
//! static IP addressing to be used, \b IPADDR_USE_DHCP will force DHCP with
//! fallback to Link Local (Auto IP), while \b IPADDR_USE_AUTOIP will force
//! Link Local only.
//!
//! This function performs initialization of the lwIP TCP/IP stack for the
//! Stellaris Ethernet MAC, including DHCP and/or AutoIP, as configured.
//!
//! \return None.
//
//*****************************************************************************

#if USE_LWIP_MDNS
extern "C" void mdns_report_cb(netif* netif, uint8_t result, int8_t service)
						__attribute__ ((weak, alias ("default_mdns_report_cb")));
extern "C" void default_mdns_report_cb(netif* netif, uint8_t result, int8_t service)
{
  TRACELN("mdns status[netif %d][service %u]: %d\n", netif->num, service, result);
}

static void mdns_service_txt(struct mdns_service *service, void *txt_userdata)
{
	TRACELN1("service: ");
	TRACE1((const char*)txt_userdata);
}

extern "C" err_t mdns_resp_add(netif* netif)
			__attribute__ ((weak, alias ("default_mdns_resp_add")));
extern "C" err_t default_mdns_resp_add(netif* netif)
{
	err_t res;
    res = mdns_resp_add_netif(netif, "lwip");
    if(res == ERR_OK) {
        res = mdns_resp_add_service(netif, "web", "_http", DNSSD_PROTO_TCP, 80, mdns_service_txt, (void *)"web");
    }
    if(res == ERR_OK) {
        res = mdns_resp_add_service(netif, "telnet", "_telnet", DNSSD_PROTO_TCP, 23, mdns_service_txt, (void *)"shell");
    }
    return res;
}

extern "C" void mdns_init(netif* net_if)
						__attribute__ ((weak, alias ("default_mdns_init")));
extern "C" void default_mdns_init(netif* netif)
{
    mdns_resp_register_name_result_cb(mdns_report_cb);
    mdns_resp_init();
    mdns_resp_add(netif);
    mdns_resp_announce(netif);
}
#endif

void lwIPInit(LWIP_DRIVER_INFO* drv_info, ip_adr_set* set)
{
	LWIP_DRIVER_DATA* drv_data = drv_info->drv_data;

    //
    // Check the parameters.
    //
#if LWIP_DHCP && LWIP_AUTOIP
    ASSERT((set->ip_addr_mode == IPADDR_USE_STATIC) ||
           (set->ip_addr_mode == IPADDR_USE_DHCP) ||
           (set->ip_addr_mode == IPADDR_USE_AUTOIP));
#elif LWIP_DHCP
    ASSERT((set->ip_addr_mode == IPADDR_USE_STATIC) ||
           (set->ip_addr_mode == IPADDR_USE_DHCP));
#elif LWIP_AUTOIP
    ASSERT((set->ip_addr_mode == IPADDR_USE_STATIC) ||
           (set->ip_addr_mode == IPADDR_USE_AUTOIP);)
#else
    ASSERT(set->ip_addr_mode == IPADDR_USE_STATIC);
#endif

	PIO_Cfg_List(drv_info->eth_pins);

    //
    // Enable the ethernet peripheral.
    //
	RCCPeripheralEnable (ID_PERIPH_ETHMAC);
	RCCPeripheralEnable (ID_PERIPH_ETHMACTX);
	RCCPeripheralEnable (ID_PERIPH_ETHMACRX);
	RCCPeripheralReset(ID_PERIPH_ETHMAC);
	RCCPeripheralReset(ID_PERIPH_ETHMACTX);
	RCCPeripheralReset(ID_PERIPH_ETHMACRX);

	if (drv_info->mac_cfg->mac_cfg_flags & MAC_CFG_FLG_MCO1)
	{
		RCC->RCC_CFGR = (RCC->RCC_CFGR & ~(RCC_CFGR_MCO1 | RCC_CFGR_MCO1PRE)) | drv_info->mac_cfg->RCC_CFGR;
	} else
		if (drv_info->mac_cfg->mac_cfg_flags & MAC_CFG_FLG_MCO2)
		{
			RCC->RCC_CFGR = (RCC->RCC_CFGR & ~(RCC_CFGR_MCO2 | RCC_CFGR_MCO2PRE)) | drv_info->mac_cfg->RCC_CFGR;
		}

    //
    // Initialize lwIP library modules
    //
    lwip_init();

    //
    // Create, configure and add the Ethernet controller interface with
    // default settings.
    //
    netif_add(
    		&drv_data->lwip_netif,	//pre-allocated netif structure
    		&set->ip_addr, 			//IP address for the new netif
    		&set->net_mask, 		//network mask for the new netif
    		&set->gw_addr, 			//gateway IP address for the new netif
    		drv_info->hw_base,		//opaque data passed to the new netif
            ethernetif_init,		//callback function that initializes the interface
            ip_input				//callback function that is called to pass
            						//ingress packets up in the protocol layer stack
            );

    netif_set_default(&drv_data->lwip_netif);

    //
    // mDNS initialization
    //
#if USE_LWIP_MDNS
    mdns_init(netif_default);
#endif
    //
    // Indicate that we are running in static IP mode.
    //
    drv_data->ip_addr_mode = set->ip_addr_mode;

    netif_set_link_callback(&drv_data->lwip_netif, ethernetif_update_config);
    netif_set_status_callback(&drv_data->lwip_netif, ethernetif_status_changed);


    netif_set_up(&drv_data->lwip_netif);

    if(set->ip_addr_mode == IPADDR_USE_STATIC){
    	netif_set_addr(&drv_data->lwip_netif,
    			&set->ip_addr,
				&set->net_mask,
				&set->gw_addr);
    }else if(set->ip_addr_mode == IPADDR_USE_DHCP){
    	dhcp_start(&drv_data->lwip_netif);
    }
//    ethernetif_update_config(&drv_data->lwip_netif);
    ethernetif_link_status(drv_info);

}

//*****************************************************************************
//
//! Change the configuration of the lwIP network interface.
//!
//! \param ulIPAddr is the new IP address to be used (static).
//! \param ulNetMask is the new network mask to be used (static).
//! \param ulGWAddr is the new Gateway address to be used (static).
//! \param ulIPMode is the IP Address Mode.  \b IPADDR_USE_STATIC 0 will force
//! static IP addressing to be used, \b IPADDR_USE_DHCP will force DHCP with
//! fallback to Link Local (Auto IP), while \b IPADDR_USE_AUTOIP will force
//! Link Local only.
//!
//! This function will evaluate the new configuration data.  If necessary, the
//! interface will be brought down, reconfigured, and then brought back up
//! with the new configuration.
//!
//! \return None.
//
//*****************************************************************************
void lwIPNetworkConfigChange(LWIP_DRIVER_INFO* drv_info, ip_adr_set* set)
{
    struct netif *netif = &drv_info->drv_data->lwip_netif;
	LWIP_DRIVER_DATA* drv_data = drv_info->drv_data;

	if(!set)
		return;

    //
    // Check the parameters.
    //
#if LWIP_DHCP && LWIP_AUTOIP
    ASSERT((set->ip_addr_mode == IPADDR_USE_STATIC) ||
           (set->ip_addr_mode == IPADDR_USE_DHCP) ||
           (set->ip_addr_mode == IPADDR_USE_AUTOIP));
#elif LWIP_DHCP
    ASSERT((set->ip_addr_mode == IPADDR_USE_STATIC) ||
           (set->ip_addr_mode == IPADDR_USE_DHCP));
#elif LWIP_AUTOIP
    ASSERT((set->ip_addr_mode == IPADDR_USE_STATIC) ||
           (set->ip_addr_mode == IPADDR_USE_AUTOIP));
#else
    ASSERT(set->ip_addr_mode == IPADDR_USE_STATIC);
#endif


    //
    // Switch on the current IP Address Aquisition mode.
    //
    switch(drv_data->ip_addr_mode)
    {
        //
        // Static IP
        //
        case IPADDR_USE_STATIC:
        {
            //
            // Set the new address parameters.  This will change the address
            // configuration in lwIP, and if necessary, will reset any links
            // that are active.  This is valid for all three modes.
            //
            netif_set_addr(netif, &set->ip_addr, &set->net_mask, &set->gw_addr);

            //
            // If we are going to DHCP mode, then start the DHCP server now.
            //
#if LWIP_DHCP
            if(set->ip_addr_mode == IPADDR_USE_DHCP)
            {
                dhcp_start(netif);
            }
#endif

            //
            // If we are going to AutoIP mode, then start the AutoIP process
            // now.
            //
#if LWIP_AUTOIP
            if(set->ip_addr_mode == IPADDR_USE_AUTOIP)
            {
                autoip_start(netif);
            }
#endif
            //
            // And we're done.
            //
            break;
        }

        //
        // DHCP (with AutoIP fallback).
        //
#if LWIP_DHCP
        case IPADDR_USE_DHCP:
        {
            //
            // If we are going to static IP addressing, then disable DHCP and
            // force the new static IP address.
            //
            if(set->ip_addr_mode == IPADDR_USE_STATIC)
            {
                dhcp_stop(netif);
                netif_set_addr(netif, &set->ip_addr, &set->net_mask, &set->gw_addr);
            }

            //
            // If we are going to AUTO IP addressing, then disable DHCP, set
            // the default addresses, and start AutoIP.
            //
#if LWIP_AUTOIP
            else if(set->ip_addr_mode == IPADDR_USE_AUTOIP)
            {
                dhcp_stop(netif);
                netif_set_addr(netif, &set->ip_addr, &set->net_mask, &set->gw_addr);
                autoip_start(netif);
            }
#endif
            break;
        }
#endif

        //
        // AUTOIP
        //
#if LWIP_AUTOIP
        case IPADDR_USE_AUTOIP:
        {
            //
            // If we are going to static IP addressing, then disable AutoIP and
            // force the new static IP address.
            //
            if(set->ip_addr_mode == IPADDR_USE_STATIC)
            {
                autoip_stop(netif);
                netif_set_addr(netif, &set->ip_addr, &set->net_mask, &set->gw_addr);
            }

            //
            // If we are going to DHCP addressing, then disable AutoIP, set the
            // default addresses, and start dhcp.
            //
#if LWIP_DHCP
            else if(set->ip_addr_mode == IPADDR_USE_AUTOIP)
            {
                autoip_stop(netif);
                netif_set_addr(netif, &set->ip_addr, &set->net_mask, &set->gw_addr);
                dhcp_start(netif);
            }
#endif
            break;
        }
#endif
    }

    //
    // Save the new mode.
    //
    drv_data->ip_addr_mode = set->ip_addr_mode;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************


#if LWIP_RX_L2_QUE
/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
int ethernetif_input(struct netif *netif)
{
	LWIP_DRIVER_DATA* drv_data = (LWIP_DRIVER_DATA*)netif;
	struct pbuf *p;
	int count = 0;

	/* move received packet into a new pbuf */
	while ((p = dequeue_packet(&drv_data->rxq)) != NULL)
	{
		count++;
		/* process the packet. */
		if (ethernet_input(p, netif) != ERR_OK)
		{
			LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: input error\n"));
			pbuf_free(p);
			p = NULL;
		}
	}

	return (count);
}
#else
void ethernetif_input(struct netif *netif)
{
	struct pbuf *p;

	while( (p = low_level_receive(netif)) )
	{
		LWIP_DEBUGF(ETH_DEBUG, ("ETH: rx %x %u \n", p, p->tot_len));
//		LWIP_PLATFORM_DIAG(("ETH: rx %x %u \n", p, p->tot_len));
		if (ethernet_input(p, netif) != ERR_OK)
		{
			LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: input error\n"));
			pbuf_free(p);
		}
	}
}
#endif

#if LWIP_RX_L2_QUE || LWIP_TX_L2_QUE
/**
 * Process tx and rx packets at the low-level interrupt.
 *
 *
 */
static void stm32_eth_interrupt(struct netif *netif)
{
	LWIP_DRIVER_DATA* drv_data = (LWIP_DRIVER_DATA*)netif;
	struct pbuf *p;

#if LWIP_RX_L2_QUE
	/*
	 * Process the transmit and receive queues as long as there is receive
	 * data available
	 *
	 */
	p = low_level_receive(netif);
	while (p != NULL)
	{
//    	TRACE1(">");
		/* Add the rx packet to the rx queue */
		if (!enqueue_packet(p, &drv_data->rxq))
		{
			/* Could not place the packet on the queue, bail out. */
			pbuf_free(p);
			break;
		}

#if LWIP_TX_L2_QUE
		/* Check if TX fifo is empty and packet available */
		if (!PBUF_QUEUE_EMPTY(&drv_data->txq))
		{
			p = drv_data->txq.pbuf[drv_data->txq.qread];
			if(p)
			{
				if(low_level_transmit(netif, p) == ERR_OK)
					p = NULL;
			}
			if(p == NULL)
				dequeue_packet(&drv_data->txq);
		}
#endif

		/* Read another packet from the RX fifo */
		p = low_level_receive(netif);
	}
#endif

#if LWIP_TX_L2_QUE
	/* One more check of the transmit queue/fifo */
	if (!PBUF_QUEUE_EMPTY(&drv_data->txq))
	{
		p = drv_data->txq.pbuf[drv_data->txq.qread];
		if(p)
		{
			if(low_level_transmit(netif, p) == ERR_OK)
				p = NULL;
		}
		if(p == NULL)
			dequeue_packet(&drv_data->txq);
	}
#endif
}
#endif

//*----------------------------------------------------------------------------
//*			lwIP thread
//*----------------------------------------------------------------------------

WEAK_C void lwip_default_settings(ip_adr_set *set)
{
	set->ip_addr.addr = 0;
	set->gw_addr.addr = 0;
	set->net_mask.addr = 0;
	set->ip_addr_mode = IPADDR_USE_DHCP;
}

#define LWIP_THREAD_RXTXSIG		1
void lwipdrv_thread(LWIP_DRIVER_INFO* drv_info)
{
	LWIP_DRIVER_DATA*  drv_data = drv_info->drv_data;
    CHandle helper;
    CHandle phy_int;

	ip_adr_set set;

	drv_data->drv_info = drv_info;
	ALLOCATE_SIGNAL(LWIP_THREAD_RXTXSIG);

	lwip_default_settings(&set);

	lwIPInit(drv_info, &set);
	helper.tsk_safe_open(drv_info->info.drv_index, 0);
	helper.mode0 = 0; //mode0=0 control handle
	helper.tsk_start_read(NULL, 0);
	if(drv_info->phy_int)
	{
		phy_int.tsk_safe_open(GPIO_IRQn, drv_info->phy_int);
		phy_int.len = 0;
		phy_int.set_res_cmd(CMD_READ|FLAG_LOCK);
		phy_int.dst.as_voidptr = nullptr;
		phy_int.tsk_start_handle();
	}

    while(1)
    {
    	unsigned int sig;

    	sig = tsk_wait_signal(SIGNAL_ANY, 10);

        // 1) process ISRs
    	if(sig)
    	{
#if LWIP_RX_L2_QUE || LWIP_TX_L2_QUE
			//queue in/out packets
            stm32_eth_interrupt(&drv_data->lwip_netif);
#endif

			//process input
		    ethernetif_input(&drv_data->lwip_netif);

		    //process phy interrupts
			if (sig & phy_int.signal)
			{
				ethernetif_link_status(drv_info);

				phy_int.set_res_cmd(CMD_READ|FLAG_LOCK);
				phy_int.tsk_start_handle();
		    }
    	}

        // 2) get waiting clients
		if(sig & helper.signal)
        {
			CSocket* client;

        	helper.res &= ~FLG_SIGNALED;

			while( (client = (CSocket*)helper.dst.as_voidptr) )
			{
				RES_CODE res  = RES_SIG_ERROR;

				helper.dst.as_voidptr = client->next;

			    if(client->cmd & FLAG_COMMAND)
			    {
			    	if(client->cmd != LWIP_CMD_TCP_CONFIG)
			    	{
			    		res = lwip_process_cmd(client, &drv_data->lwip_netif);

			    	}else
			    	{
			    		lwip_sock_discard(&drv_data->lwip_netif);
			    		lwIPNetworkConfigChange(drv_info, (ip_adr_set *)client->src.as_voidptr);
			    		res = RES_SIG_OK;
			    	}
			    } else
			    {
				    if(client->cmd & FLAG_WRITE)
				    {
				    	res = lwip_api_write(client, &drv_data->lwip_netif);
				    } else
					    if(client->cmd & FLAG_READ)
					    {
					    	res = lwip_api_read(client);
					    }
			    }

			    if(res & FLG_SIGNALED)
			    {
				    tsk_HND_SET_STATUS(client, res);
			    }
			}
			helper.tsk_start_read(NULL, 0);
        }

        // 3) process lwip timeouts
		sig = CURRENT_TIME;
    	if(sig != drv_data->timer_main)
    	{
    		if(ms_since(drv_data->timer_main) > 10){
    			sys_restart_timeouts();
    		}
        	drv_data->timer_main = sig;
            sys_check_timeouts();
    	}
    }


}
TASK_DECLARE_STATIC(lwipdrv_task, "LWIP", (void (*)(void))lwipdrv_thread, 60, 450 + TRACE_SIZE);


//*----------------------------------------------------------------------------
//*			DCR function
//*----------------------------------------------------------------------------
void LWIP_DCR(LWIP_DRIVER_INFO* drv_info, unsigned int reason, HANDLE param)
{
	LWIP_DRIVER_DATA* drv_data = drv_info->drv_data;

	switch(reason)
    {
        case DCR_RESET:
        	drv_info->drv_data->lwip_netif.state = drv_info->hw_base;
        	NVIC_SetPriority(drv_info->info.drv_index, drv_info->info.isr_priority);
//        	SysCtlPeripheralDisable(drv_info->info.peripheral_indx);
           	usr_task_init_static(&lwipdrv_task_desc, true);
           	lwipdrv_task.sp->r0.as_voidptr = drv_info;

        	break;

        case DCR_OPEN:
        	param->mode0 = 1;	// mark the handle as client
        						// helper must clear this field

			param->res = RES_OK;
        	break;

	    case DCR_CANCEL:
	    	if(param->mode0)
	    	{
	    		// this is a client handle...
	    		// first try the waiting
	    		if(!param->svc_list_cancel(drv_data->waiting))
	    		{
	    			//try to cancel read & accept
	    			if(param->res & FLG_BUSY)
	    			{
						if(locked_clr_byte(&param->mode1, TCPHS_CANCELABLE))
						{
							svc_HND_SET_STATUS(param, RES_SIG_CANCEL);
						}
	    			}

	    		}
	    	} else
	    	{
	    		// helper
	    		if(param == drv_data->helper)
	    		{
					//the helper task is waiting for object...
					drv_data->helper = NULL;
					param->dst.as_voidptr = NULL;
					svc_HND_SET_STATUS(param, RES_SIG_OK);
	    		}
	    	}

			break;

   }

}

//*----------------------------------------------------------------------------
//*			DSR function
//*----------------------------------------------------------------------------
void LWIP_DSR(LWIP_DRIVER_INFO* drv_info, HANDLE hnd)
{
	LWIP_DRIVER_DATA* drv_data = drv_info->drv_data;
	HANDLE tmp;

	if(hnd->mode0)
	{
		// this is a client handle...
		hnd->res = RES_BUSY;
		if( (tmp=drv_data->helper) )
		{
			//the helper task is waiting for object...
			hnd->next = NULL;
			drv_data->helper = NULL;
			tmp->dst.as_voidptr = hnd;
			svc_HND_SET_STATUS(tmp, RES_SIG_OK);
		} else
		{
			//queue the WINDOW object while helper task is busy
			hnd->next = drv_data->waiting;
			drv_data->waiting = hnd;
		}
	} else
	{
		// this should be the helper task
		if( (tmp=drv_data->waiting) )
		{
			drv_data->waiting = NULL;
			hnd->dst.as_voidptr = tmp;
			svc_HND_SET_STATUS(hnd, RES_SIG_OK);
		} else
		{
			hnd->res = RES_BUSY;
			drv_data->helper = hnd;
		}
	}
}

//*----------------------------------------------------------------------------
//*			ISR function
//*----------------------------------------------------------------------------
//*****************************************************************************
//
//! Handles Ethernet interrupts for the lwIP TCP/IP stack.
//!
//! This function handles Ethernet interrupts for the lwIP TCP/IP stack.  At
//! the lowest level, all receive packets are placed into a packet queue for
//! processing at a higher level.  Also, the transmit packet queue is checked
//! and packets are drained and transmitted through the Ethernet MAC as needed.
//! If the system is configured without an RTOS, additional processing is
//! performed at the interrupt level.  The packet queues are processed by the
//! lwIP TCP/IP code, and lwIP periodic timers are serviced (as needed).
//!
//! \return None.
//
//*****************************************************************************

void LWIP_ISR(LWIP_DRIVER_INFO* drv_info )
{
	ETH_TypeDef* mac = drv_info->hw_base;
	uint32_t status;

	status = mac->ETH_DMASR;
	LWIP_DEBUGF(ETH_DEBUG, ("ETH: irq %x\n", status) );
//	LWIP_PLATFORM_DIAG(("ETH: irq %x\n", status));
	if(status)
	{
		mac->ETH_DMASR = status;
    	usr_send_signal(&lwipdrv_task, LWIP_THREAD_RXTXSIG);
	}

}


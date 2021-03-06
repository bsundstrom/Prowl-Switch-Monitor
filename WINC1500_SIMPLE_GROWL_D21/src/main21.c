/**
 *
 * \file
 *
 * \brief WINC1500 Simple Growl Example.
 *
 * Copyright (c) 2016 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

/** \mainpage
 * \section intro Introduction
 * This example demonstrates the use of the WINC1500 with the SAMD21 Xplained Pro.
 * It basically transmits a notification from the WINC1500 Wi-Fi module (based on a
 * certain trigger) to a public remote server which in turn sends back a notification
 * to a specific phone application.<br>
 * The initiated notification from the WINC1500 device is directed to a certain
 * subscriber on the server.<br>
 * The supported applications are PROWL (for iPhone notifications) and NMA (for
 * ANDROID notifications).<br>
 * It uses the following hardware:
 * - the SAMD21 Xplained Pro.
 * - the WINC1500 on EXT1.
 *
 * \section files Main Files
 * - main.c : Initialize growl and send notification message.
 *
 * \section usage Usage
 * -# Build the program and download it into the board.
 * -# On the computer, open and configure a terminal application as the follows.
 * \code
 *    Baud Rate : 115200
 *    Data : 8bit
 *    Parity bit : none
 *    Stop bit : 1bit
 *    Flow control : none
 * \endcode

 *
 * This application supports sending GROWL notifications to the following servers.
 * -# PROWL for iOS push notifications (https://www.prowlapp.com/).
 * -# NMA for Android push notifications (http://www.notifymyandroid.com/).
 *
 * In order to enable the GROWL application (for sending notifications), apply the following instructions.
 * -# Create a NMA account at http://www.notifymyandroid.com/ and create an API key. Copy the obtained key string in the file
 *  main.h in the MACRO NMA_API_KEY as the following.
 * -# Create a PROWL account at https://www.prowlapp.com/ and create an API key. Copy the obtained API key string in the file
 *  main.h in the MACRO PROWL_API_KEY as the following.<br>
 *	#define NMA_API_KEY      "f8bd3e7c9c5c10183751ab010e57d8f73494b32da73292f6"<br>
 *	#define PROWL_API_KEY    "117911f8a4f2935b2d84abc934be9ff77d883678"
 *
 * \warning
 * \code
 *    For using the growl, the root certificate must be installed.
 *    Download the root certificate using the root_certificate_downloader. (Refer to WINC1500 Software User Guide.)
 * \endcode
 *
 * \section compinfo Compilation Information
 * This software was written for the GNU GCC compiler using Atmel Studio 6.2
 * Other compilers may or may not work.
 *
 * \section contactinfo Contact Information
 * For further information, visit
 * <A href="http://www.atmel.com">Atmel</A>.\n
 */

#include "asf.h"
#include "main.h"
#include "driver/source/nmasic.h"
#include "growl/growl.h"
#include "Timer.h"
#include "MyID.h"

uint8 pin_state;
uint32_t seconds_alive_cnt;
uint8 service_1s_flag;
connection_states connection_state;
uint32 growl_msg_tmr;
uint32 reconnect_tmr;
uint32 reconnect_cnt;

const char app_string[] = MY_APP_NAME; //pulled from MyID.h

char event_msg_buf[GROWL_DESCRIPTION_MAX_LENGTH + MAX_TIME_STAMP_LEN];

//! [setup]
void configure_extint_channel(void)
{
	struct extint_chan_conf config_extint_chan;
	extint_chan_get_config_defaults(&config_extint_chan);
	config_extint_chan.gpio_pin           = BUTTON_0_EIC_PIN;
	config_extint_chan.gpio_pin_mux       = BUTTON_0_EIC_MUX;
	config_extint_chan.gpio_pin_pull      = EXTINT_PULL_UP;
	config_extint_chan.detection_criteria = EXTINT_DETECT_BOTH;
	extint_chan_set_config(BUTTON_0_EIC_LINE, &config_extint_chan);
}

void configure_extint_callbacks(void)
{
	extint_register_callback(extint_detection_callback, BUTTON_0_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
	extint_chan_enable_callback(BUTTON_0_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
}

void extint_detection_callback(void)
{
	pin_state = port_pin_get_input_level(BUTTON_0_PIN);
}

/** UART module for debug. */
static struct usart_module cdc_uart_module;

/**
 * \brief Configure UART console.
 */
static void configure_console(void)
{
	struct usart_config usart_conf;

	usart_get_config_defaults(&usart_conf);
	usart_conf.mux_setting = EDBG_CDC_SERCOM_MUX_SETTING;
	usart_conf.pinmux_pad0 = EDBG_CDC_SERCOM_PINMUX_PAD0;
	usart_conf.pinmux_pad1 = EDBG_CDC_SERCOM_PINMUX_PAD1;
	usart_conf.pinmux_pad2 = EDBG_CDC_SERCOM_PINMUX_PAD2;
	usart_conf.pinmux_pad3 = EDBG_CDC_SERCOM_PINMUX_PAD3;
	usart_conf.baudrate    = 115200;

	stdio_serial_init(&cdc_uart_module, EDBG_CDC_MODULE, &usart_conf);
	usart_enable(&cdc_uart_module);
}

void ConvertSeconds2Timestamp(uint32 seconds, timestamp_t* my_time)
{
	uint32 remainder;
	my_time->days = seconds / (60 * 60 * 24);
	remainder = seconds % (60 * 60 * 24); //remainder of hours
	my_time->hours = remainder / (60 * 60);
	remainder = remainder % (60 * 60); //remainder of minutes
	my_time->minutes = remainder / 60;
	my_time->seconds = remainder % 60;
}

int send_prowl(const char* event_name, const char* event_msg)
{
	timestamp_t time_stamp;
	ConvertSeconds2Timestamp(seconds_alive_cnt, &time_stamp);
	sprintf(event_msg_buf, "%01ld:%02ld:%02ld:%02ld ", time_stamp.days, time_stamp.hours, time_stamp.minutes, time_stamp.seconds);
	strcat(event_msg_buf, event_msg);
	return growl_send_message_handler(app_string, event_name, event_msg_buf); // msg_buffer);
}

/**
 * \brief Send a specific notification to a registered Android(NMA) or IOS(PROWL)
 */
int growl_send_message_handler(const char* app_name, const char* event_name, const char* event_msg)
{
	printf("Sending Prowl: %s => ", event_msg);
	growl_msg_tmr = 119;
	NMI_GrowlSendNotification(PROWL_CLIENT, (uint8*) app_name, (uint8*) event_name, (uint8*) event_msg,PROWL_CONNECTION_TYPE); // send by PROWL */
	//NMI_GrowlSendNotification(NMA_CLIENT, (uint8_t *)"Growl_Sample", (uint8_t *)"Growl_Event", (uint8_t *)"growl_test", NMA_CONNECTION_TYPE);           /* send by NMA */
	return 0;
}

#ifdef ENABLE_WIFI_PROVISIONING
static void set_dev_name_to_mac(uint8_t *name, uint8_t *mac_addr)
{
	/* Name must be in the format WINC1500_00:00 */
	uint16 len;

	len = m2m_strlen(name);
	if (len >= 5) {
		name[len - 1] = MAIN_HEX2ASCII((mac_addr[5] >> 0) & 0x0f);
		name[len - 2] = MAIN_HEX2ASCII((mac_addr[5] >> 4) & 0x0f);
		name[len - 4] = MAIN_HEX2ASCII((mac_addr[4] >> 0) & 0x0f);
		name[len - 5] = MAIN_HEX2ASCII((mac_addr[4] >> 4) & 0x0f);
	}
}
#endif

/**
 * \brief Callback to get the Wi-Fi status update.
 *
 * \param[in] u8MsgType type of Wi-Fi notification. Possible types are:
 *  - [M2M_WIFI_RESP_CON_STATE_CHANGED](@ref M2M_WIFI_RESP_CON_STATE_CHANGED)
 *  - [M2M_WIFI_REQ_DHCP_CONF](@ref M2M_WIFI_REQ_DHCP_CONF)
 * \param[in] pvMsg A pointer to a buffer containing the notification parameters
 * (if any). It should be casted to the correct data type corresponding to the
 * notification type.
 */
static void wifi_cb(uint8_t u8MsgType, void *pvMsg)
{
	switch (u8MsgType) {
	case M2M_WIFI_RESP_CON_STATE_CHANGED:
	{
		tstrM2mWifiStateChanged *pstrWifiState = (tstrM2mWifiStateChanged *)pvMsg;
		if (pstrWifiState->u8CurrState == M2M_WIFI_CONNECTED) {
			printf("Wi-Fi connected\r\n");
			m2m_wifi_request_dhcp_client();
		} else if (pstrWifiState->u8CurrState == M2M_WIFI_DISCONNECTED) {
			printf("Wi-Fi disconnected - Will try to reconnect in 30 seconds...\r\n");
			reconnect_tmr = 30;
			connection_state = WIFI_NOT_CONNECTED;
		}
		break;
	}

	case M2M_WIFI_REQ_DHCP_CONF:
	{
		uint8_t *pu8IPAddress = (uint8_t *)pvMsg;
		printf("DHCP complete - Wi-Fi IP is %u.%u.%u.%u\r\n",
				pu8IPAddress[0], pu8IPAddress[1], pu8IPAddress[2], pu8IPAddress[3]);
#ifdef ENABLE_WIFI_PROVISIONING
		if (gbRespProvInfo) {
			/** init growl */
			NMI_GrowlInit((uint8_t *)PROWL_API_KEY, (uint8_t *)NMA_API_KEY);
			send_prowl("Status Update", "Connection Established");
		}
#else
		NMI_GrowlInit((uint8_t *)PROWL_API_KEY, (uint8_t *)NMA_API_KEY);
		if(reconnect_cnt)
		{
			send_prowl("Status Update", "Connection Re-established.");
		}
		else
			send_prowl("Status Update", "Connection Established");
#endif
		connection_state = WIFI_CONNECTED;
		break;
	}

	case M2M_WIFI_RESP_PROVISION_INFO:
	{
		tstrM2MProvisionInfo *pstrProvInfo = (tstrM2MProvisionInfo *)pvMsg;
		printf("wifi_cb: M2M_WIFI_RESP_PROVISION_INFO.\r\n");

		if (pstrProvInfo->u8Status == M2M_SUCCESS) {
			m2m_wifi_connect((char *)pstrProvInfo->au8SSID, strlen((char *)pstrProvInfo->au8SSID), pstrProvInfo->u8SecType,
					pstrProvInfo->au8Password, M2M_WIFI_CH_ALL);
			gbRespProvInfo = true;
		} else {
			printf("wifi_cb: Provision failed.\r\n");
		}
		break;
	}

	default:
		break;
	}
}

/**
 * \brief Growl notification callback.
 *	Pointer to a function delivering growl events.
 *
 *  \param	[u8Code] Possible error codes could be returned by the nma server and refer to the comments in the growl.h.
 *  - [20] GROWL_SUCCESS (@ref GROWL_SUCCESS)
 *  - [40] GROWL_ERR_BAD_REQUEST (@ref GROWL_ERR_BAD_REQUEST)
 *  - [41] GROWL_ERR_NOT_AUTHORIZED (@ref GROWL_ERR_NOT_AUTHORIZED)
 *  - [42] GROWL_ERR_NOT_ACCEPTED (@ref GROWL_ERR_NOT_ACCEPTED)
 *  - [46] GROWL_ERR_API_EXCEED (@ref GROWL_ERR_API_EXCEED)
 *  - [49] GROWL_ERR_NOT_APPROVED (@ref GROWL_ERR_NOT_APPROVED)
 *  - [50] GROWL_ERR_SERVER_ERROR (@ref GROWL_ERR_SERVER_ERROR)
 *  - [30] GROWL_ERR_LOCAL_ERROR (@ref GROWL_ERR_LOCAL_ERROR)
 *  - [10] GROWL_ERR_CONN_FAILED (@ref GROWL_ERR_CONN_FAILED)
 *  - [11] GROWL_ERR_RESOLVE_DNS (@GROWL_ERR_RESOLVE_DNS GROWL_RETRY)
 *  - [12] GROWL_RETRY (@ref GROWL_RETRY)
 *	\param	[u8ClientID] client id returned by the nma server.
 */
void GrowlCb(uint8_t u8Code, uint8_t u8ClientID)
{
	if(u8Code == 20)
		printf("Growl msg sent successfully.\r\n");
	else	
	{
		printf("ERROR: Growl CB Code: %d \r\n", u8Code);
		system_reset();
	}
	growl_msg_tmr = 0; //reset msg timer for growl response
}

void Service_1hr(void)
{
	static uint32 hour_cntr = 0;
	if(hour_cntr == 0)
	{
		hour_cntr = 23;
#ifdef HEARTBEAT_ENABLED
		if(connection_state == WIFI_CONNECTED)
			send_prowl("Heartbeat", "24Hr Heartbeat Ping");
#endif
	}
	hour_cntr--;
}

void Service_1s(void)
{
	static uint16 hour_tmr;
	seconds_alive_cnt++;
	
	if(reconnect_tmr > 0)
	{
		reconnect_tmr--;
		if(reconnect_tmr == 0)
		{
			reconnect_cnt++;
			printf("Trying to re-connect.\r\n");
			m2m_wifi_connect((char *)MAIN_WLAN_SSID, sizeof(MAIN_WLAN_SSID), MAIN_WLAN_AUTH, (char *)MAIN_WLAN_PSK, M2M_WIFI_CH_ALL);
		}
	}
	
	if(hour_tmr)
	{
		hour_tmr--;
	}
	else
	{
		hour_tmr = 60 * 60 - 1;
		Service_1hr();
	}
}

void TimerCallback(void) //called every .25s
{
	static uint8_t second_tmr;
	if(second_tmr)
	{
		second_tmr--;
	}
	else
	{
		second_tmr = 3;
		service_1s_flag = 1; //let main thread know to call Service1s()	
	}
	//Place any code that needs to be serviced at this interval here:
	if(growl_msg_tmr)
	{
		growl_msg_tmr--;
		putchar('.');
	}
}

/**
 * \brief Main application function.
 *
 * \return program return value.
 */
int main(void)
{
	tstrWifiInitParam param;
	int8_t ret;
	uint8 prev_pin_state = pin_state;

#ifdef ENABLE_WIFI_PROVISIONING
	uint8_t mac_addr[6];
	uint8_t u8IsMacAddrValid;
#endif

	/* Initialize the board. */
	system_init();

	/* Initialize the UART console. */
	configure_console();
	printf(PROWLER_STR);
	printf(STRING_HEADER);
	
	configure_tc();
	configure_tc_callbacks();

	//port_pin_set_output_level(LED0_PIN, 1);
	pin_state = port_pin_get_input_level(BUTTON_0_PIN);
	prev_pin_state = pin_state;
	//Configure interupt for SW0
	configure_extint_channel();
	configure_extint_callbacks();
	system_interrupt_enable_global();
	
	/* Initialize the BSP. */
	nm_bsp_init();

	/* Initialize Wi-Fi parameters structure. */
	memset((uint8_t *)&param, 0, sizeof(tstrWifiInitParam));

	/* Initialize Wi-Fi driver with data and status callbacks. */
	param.pfAppWifiCb = wifi_cb;
	ret = m2m_wifi_init(&param);
	if (M2M_SUCCESS != ret) 
	{
		printf("main: m2m_wifi_init call error!(%d)\r\n", ret);
		while (1);
	}

#ifdef ENABLE_WIFI_PROVISIONING
	m2m_wifi_get_otp_mac_address(mac_addr, &u8IsMacAddrValid);
	if (!u8IsMacAddrValid) {
		m2m_wifi_set_mac_address(gau8MacAddr);
	}

	m2m_wifi_get_mac_address(gau8MacAddr);
	set_dev_name_to_mac((uint8_t *)gacDeviceName, gau8MacAddr);
	set_dev_name_to_mac((uint8_t *)gstrM2MAPConfig.au8SSID, gau8MacAddr);
	m2m_wifi_set_device_name((uint8_t *)gacDeviceName, (uint8_t)m2m_strlen((uint8_t *)gacDeviceName));
	gstrM2MAPConfig.au8DHCPServerIP[0] = 0xC0; /* 192 */
	gstrM2MAPConfig.au8DHCPServerIP[1] = 0xA8; /* 168 */
	gstrM2MAPConfig.au8DHCPServerIP[2] = 0x01; /* 1 */
	gstrM2MAPConfig.au8DHCPServerIP[3] = 0x01; /* 1 */

	m2m_wifi_start_provision_mode((tstrM2MAPConfig *)&gstrM2MAPConfig, (char *)gacHttpProvDomainName, 1);
	printf("Provision Mode started.\r\nConnect to [%s] via AP[%s] and fill up the page.\r\n", MAIN_HTTP_PROV_SERVER_DOMAIN_NAME, gstrM2MAPConfig.au8SSID);
#else
	/* Connect to router. */
	m2m_wifi_connect((char *)MAIN_WLAN_SSID, sizeof(MAIN_WLAN_SSID),
	MAIN_WLAN_AUTH, (char *)MAIN_WLAN_PSK, M2M_WIFI_CH_ALL);
#endif

	while (1) 
	{
		/* Handle pending events from network controller. */
		while (m2m_wifi_handle_events(NULL) != M2M_SUCCESS);
		if(pin_state != prev_pin_state && growl_msg_tmr == 0)
		{
			prev_pin_state = pin_state;
			if(pin_state)
			{
				send_prowl("State Change", "Opened");
			}
			else
			{
				send_prowl("State Change", "Closed");
			}
		}
		if(service_1s_flag)
		{
			service_1s_flag = 0;
			Service_1s();
		}
	}

	return 0;
}

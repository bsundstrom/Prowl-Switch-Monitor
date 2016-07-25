/**
 * \file
 *
 * \brief MAIN configuration.
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

#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/include/m2m_wifi.h"

#define STRING_HEADER "Prowl-based switch monitor\r\nBoard:" BOARD_NAME "\r\n"	\
"Compiled: "__DATE__ " "__TIME__ " --\r\n"

#define PROWLER_STR \
" _____ _            ____                    _\r\n" \
"|_   _| |__   ___  |  _ \\ _ __ _____      _| | ___ _ __\r\n" \
"  | | | '_ \\ / _ \\ | |_) | '__/ _ \\ \\ /\\ / / |/ _ \\ '__|\r\n" \
"  | | | | | |  __/ |  __/| | | (_) \\ V  V /| |  __/ |\r\n" \
"  |_| |_| |_|\\___| |_|   |_|  \\___/ \\_/\\_/ |_|\\___|_|\r\n"

/** Growl Options */
#define PROWL_API_KEY            "dc3598fd664f6508f600bd1efdfe4ca5440fb308"
#define NMA_API_KEY              "91787604ed50a6cfc2d3f83d1ee196cbc30a3cb08a7e69a0" //"0757fe93214fc2cdf2ad42a5005ee0aa83a7a8ea242c0b80"
#define SSL_CONNECTION           1
#define NORMAL_CONNECTION        0
#define PROWL_CONNECTION_TYPE    NORMAL_CONNECTION
#define NMA_CONNECTION_TYPE      SSL_CONNECTION

#ifdef ENABLE_WIFI_PROVISIONING
#define MAIN_M2M_AP_SEC                  M2M_WIFI_SEC_OPEN
#define MAIN_M2M_AP_WEP_KEY              "1234567890"
#define MAIN_M2M_AP_SSID_MODE            SSID_MODE_VISIBLE
#else
/** Wi-Fi Settings */
#define MAIN_WLAN_SSID        "Degobah" /* < Destination SSID */
#define MAIN_WLAN_AUTH        M2M_WIFI_SEC_WPA_PSK /* < Security manner */
#define MAIN_WLAN_PSK         "Iamyourfather" /* < Password for Destination SSID */
#endif

#define MAIN_HTTP_PROV_SERVER_DOMAIN_NAME    "atmelconfig.com"

#define MAIN_M2M_DEVICE_NAME                 "WINC1500_00:00"
#define MAIN_MAC_ADDRESS                     {0xf8, 0xf0, 0x05, 0x45, 0xD4, 0x84}

#define MAIN_HEX2ASCII(x) (((x) >= 10) ? (((x) - 10) + 'A') : ((x) + '0'))

void TimerCallback(void);
void Service_1s(void);
void Service_1hr(void);
static int send_prowl(const char* event_name, const char* event_msg);
static int growl_send_message_handler(const char* app_name, const char* event_name, const char* event_msg);

#ifdef ENABLE_WIFI_PROVISIONING
static tstrM2MAPConfig gstrM2MAPConfig = {
	MAIN_M2M_DEVICE_NAME, 1, 0, WEP_40_KEY_STRING_SIZE, MAIN_M2M_AP_WEP_KEY, (uint8_t)MAIN_M2M_AP_SEC, MAIN_M2M_AP_SSID_MODE
};
static CONST char gacHttpProvDomainName[] = MAIN_HTTP_PROV_SERVER_DOMAIN_NAME;
static uint8_t gau8MacAddr[] = MAIN_MAC_ADDRESS;
static int8_t gacDeviceName[] = MAIN_M2M_DEVICE_NAME;
#endif

/** Provision status variable. */
static volatile bool gbRespProvInfo = false;

#ifdef __cplusplus
}
#endif

#endif /* MAIN_H_INCLUDED */

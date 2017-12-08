#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

/************************************************************************/
/* INCLUDES                                                             */
/************************************************************************/
#include "driver/include/m2m_wifi.h"
#include "bsp/include/nm_bsp.h"
#include "socket/include/socket.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/************************************************************************/
/* TC                                                                   */
/************************************************************************/
#define TC_CHANNEL 0

#define ON 1
#define OFF 0


/************************************************************************/
/* BUTTONS                                                              */
/************************************************************************/
#define BOARD_BUT_PIO_ID   ID_PIOA
#define BOARD_BUT_PIO      PIOA
#define BOARD_BUT_PIN      11
#define BOARD_BUT_PIN_MASK (1 << BOARD_BUT_PIN)

/************************************************************************/
/* LEDs                                                                 */
/************************************************************************/
#define LED0_PIO_ID   ID_PIOC
#define LED0_PIO      PIOC
#define LED0_PIN      12
#define LED0_PIN_MASK (1 << LED0_PIN)

#define LED1_PIO_ID   ID_PIOA
#define LED1_PIO      PIOA
#define LED1_PIN      18
#define LED1_PIN_MASK (1 << LED1_PIN)

#define LED2_PIO_ID   ID_PIOA
#define LED2_PIO      PIOA
#define LED2_PIN      30
#define LED2_PIN_MASK (1 << LED2_PIN)

#define LED3_PIO_ID   ID_PIOA
#define LED3_PIO      PIOA
#define LED3_PIN      25
#define LED3_PIN_MASK (1 << LED3_PIN)

/************************************************************************/
/* AFEC                                                                 */
/************************************************************************/
/** Reference voltage for AFEC,in mv. */
#define VOLT_REF        (3300)

/** The maximal digital value */
/** 2^12 - 1                  */
#define MAX_DIGITAL     (4095UL)

/* Canal do sensor de temperatura */
#define AFEC_CHANNEL_TEMP_SENSOR 0

/************************************************************************/
/* WIFI                                                                 */
/************************************************************************/
#define MAIN_WLAN_SSID            "ComputacaoEmbarcada"
#define MAIN_WLAN_PSK             "computacao"
#define MAIN_SERVER_NAME          "54.175.192.2"
#define MAIN_SERVER_PORT          (5000)
#define MAIN_WLAN_AUTH            M2M_WIFI_SEC_WPA_PSK
#define GET_SUFIX                 "HTTP/1.1\r\n"
#define POST_SUFIX                "HTTP/1.1\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: "
#define HTTP_END                  "\r\n"
#define MAIN_WIFI_M2M_BUFFER_SIZE 1400
#define IPV4_BYTE(val, index)     ((val >> (index * 8)) & 0xFF)
#define MAIN_HEX2ASCII(x)         (((x) >= 10) ? (((x) - 10) + 'A') : ((x) + '0'))

/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/
static void configure_console(void);
int inet_aton(const char *cp, in_addr *ap);
static void resolve_cb(uint8_t *hostName, uint32_t hostIp);
static void socket_cb(SOCKET sock, uint8_t u8Msg, void *pvMsg);
static void set_dev_name_to_mac(uint8_t *name, uint8_t *mac_addr);
static void wifi_cb(uint8_t u8MsgType, void *pvMsg);
void BUTTON_init(Pio *p_pio, uint32_t pio_id, uint32_t pin_mask, void (*p_handler)(uint32_t, uint32_t), uint32_t interruption_priority);
void build_get(uint8_t *buff, char *route);
void build_post(uint8_t *buff, char *route, char *query);
void LED_init(Pio *p_pio, uint32_t ul_id, uint32_t ul_mask, uint32_t ul_default_level);
void TC_init(Tc *p_tc, uint32_t ul_id, uint32_t ul_freq);
static void AFEC_Temp_callback(void);

/************************************************************************/
/* STATE MACHINE                                                         */
/************************************************************************/
#define WAIT      0
#define POST_LDR  1
#define GET_LEDS  2
#define POST_WAIT 3
#define GET_WAIT 4


#endif /* MAIN_H_INCLUDED */

#include "asf.h"
#include "main.h"

/************************************************************************/
/* GLOBALS                                                              */
/************************************************************************/
/** AFEC flag */
volatile bool afec_done = false;

/*  */
volatile bool receive_ok = false;
/** AFEC data value */
volatile uint32_t g_ul_value = 0;

/* State Machine init flag */
volatile bool init_state_machine = false;


uint8_t state;



/** Store POST variable */
static uint8_t ldr_buffer[MAIN_WIFI_M2M_BUFFER_SIZE] = {0};

/** GET buffer */
static uint8_t get_buffer[MAIN_WIFI_M2M_BUFFER_SIZE] = {0};

/** IP address of host. */
uint32_t gu32HostIp = 0;

/** TCP client socket handlers. */
static SOCKET tcp_client_socket = -1;

/* Send buffer */
static uint8_t send_buffer[MAIN_WIFI_M2M_BUFFER_SIZE] = {0};

/* Receive buffer definition. */
static uint8_t gau8Buffer[MAIN_WIFI_M2M_BUFFER_SIZE] = {0};

/** Wi-Fi status variable. */
static bool gbConnectedWifi = false;

/** Get host IP status variable. */
/** Wi-Fi connection state */
static uint8_t wifi_connected;

/** Instance of HTTP client module. */
static bool gbHostIpByName = false;

/** TCP Connection status variable. */
static bool gbTcpConnection = false;

/** Server host name. */
static char server_host_name[] = MAIN_SERVER_NAME;

int p = 0;

char socketConnected = false;

/************************************************************************/
/* AFEC                                                                 */
/************************************************************************/
static void AFEC_Temp_callback(void) {
    printf("!\n");

  g_ul_value = afec_channel_get_value(AFEC0, AFEC_CHANNEL_TEMP_SENSOR);

  afec_done = true;
}

static void AFEC_init(void) {
  afec_enable(AFEC0);
  struct afec_config afec_cfg;
  afec_get_config_defaults(&afec_cfg);
  afec_init(AFEC0, &afec_cfg);
  afec_set_trigger(AFEC0, AFEC_TRIG_SW);
  afec_set_callback(AFEC0, AFEC_INTERRUPT_EOC_0,	AFEC_Temp_callback, 1);
  struct afec_ch_config afec_ch_cfg;
  afec_ch_get_config_defaults(&afec_ch_cfg);
  afec_ch_cfg.gain = AFEC_GAINVALUE_0;
  afec_ch_set_config(AFEC0, AFEC_CHANNEL_TEMP_SENSOR, &afec_ch_cfg);
  afec_channel_set_analog_offset(AFEC0, AFEC_CHANNEL_TEMP_SENSOR, 0x200);
  struct afec_temp_sensor_config afec_temp_sensor_cfg;
  afec_temp_sensor_get_config_defaults(&afec_temp_sensor_cfg);
  afec_temp_sensor_set_config(AFEC0, &afec_temp_sensor_cfg);
  afec_channel_enable(AFEC0, AFEC_CHANNEL_TEMP_SENSOR);
  afec_start_software_conversion(AFEC0);
}

/************************************************************************/
/* CONSOLE                                                              */
/************************************************************************/
static void CONSOLE_init(void) {
	const usart_serial_options_t uart_serial_options = {
		.baudrate =		CONF_UART_BAUDRATE,
		.charlength =	CONF_UART_CHAR_LENGTH,
		.paritytype =	CONF_UART_PARITY,
		.stopbits =		CONF_UART_STOP_BITS,
	};

	/* Configure UART console. */
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	stdio_serial_init(CONF_UART, &uart_serial_options);
}

/************************************************************************/
/* HTTP METHODS                                                         */
/************************************************************************/
void build_get(uint8_t *buff, char *route) {
  sprintf(
  buff,
  "%s %s %s%s",
  "GET",
  route,
  GET_SUFIX,
  HTTP_END
  );
}

void build_post(uint8_t *buff, char *route, char *query) {

  // Get content length
  static uint8_t content_length[20] = {0};
  sprintf(content_length, "%lu", strlen(query));

  sprintf(
  buff,
  "%s %s %s%s%s%s%s",
  "POST",
  route,
  POST_SUFIX,
  content_length,
  HTTP_END,
  HTTP_END,
  query
  );
}

/************************************************************************/
/* LED                                                                  */
/************************************************************************/
void LED_init(Pio *p_pio, uint32_t ul_id, uint32_t ul_mask, uint32_t ul_default_level) {
  pmc_enable_periph_clk(ul_id);
  pio_set_output(p_pio, ul_mask, ul_default_level, 0, 0);
}

/************************************************************************/
/* TC                                                                   */
/************************************************************************/
void TC_init(Tc *p_tc, uint32_t ul_id, uint32_t ul_freq) {
  uint32_t ul_div;
  uint32_t ul_tcclks;
  uint32_t ul_sysclk = sysclk_get_cpu_hz();

  /* Configura o PMC */
  pmc_enable_periph_clk(ul_id);

  /** Configura o TC para operar em  4Mhz e interrupçcão no RC compare */
  tc_find_mck_divisor(ul_freq, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
  tc_init(p_tc, TC_CHANNEL, ul_tcclks | TC_CMR_CPCTRG);
  tc_write_rc(p_tc, TC_CHANNEL, (ul_sysclk / ul_div) / ul_freq);

  /* Configura e ativa interrupçcão no TC canal 0 */
  NVIC_EnableIRQ((IRQn_Type) ul_id);
  tc_enable_interrupt(p_tc, TC_CHANNEL, TC_IER_CPCS);

  /* Inicializa o canal 0 do TC */
  tc_start(p_tc, TC_CHANNEL);
}

void TC0_Handler(void) {
  volatile uint32_t ul_dummy;
  ul_dummy = tc_get_status(TC0, TC_CHANNEL);
  UNUSED(ul_dummy);

  if(socketConnected)
    init_state_machine = true;
}

void TC3_Handler(void) {
  volatile uint32_t ul_dummy;
  ul_dummy = tc_get_status(TC1, TC_CHANNEL);
  UNUSED(ul_dummy);

      printf("#\n");


  afec_start_software_conversion(AFEC0);
}

/************************************************************************/
/* WI-FI                                                                */
/************************************************************************/
int inet_aton(const char *cp, in_addr *ap) {
  int dots = 0;
  register u_long acc = 0, addr = 0;

  do {
	  register char cc = *cp;

	  switch (cc) {
	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	    case '8':
	    case '9':
	        acc = acc * 10 + (cc - '0');
	        break;

	    case '.':
	        if (++dots > 3) {
		    return 0;
	        }
	        /* Fall through */

	    case '\0':
	        if (acc > 255) {
		    return 0;
	        }
	        addr = addr << 8 | acc;
	        acc = 0;
	        break;

	    default:
	        return 0;
    }
  } while (*cp++) ;

  /* Normalize the address */
  if (dots < 3) {
	  addr <<= 8 * (3 - dots) ;
  }

  /* Store it if requested */
  if (ap) {
	  ap->s_addr = _htonl(addr);
  }

  return 1;
}

static void resolve_cb(uint8_t *hostName, uint32_t hostIp) {
	gu32HostIp = hostIp;
	gbHostIpByName = true;
	printf("resolve_cb: %s IP address is %d.%d.%d.%d\r\n\r\n", hostName,
			(int)IPV4_BYTE(hostIp, 0), (int)IPV4_BYTE(hostIp, 1),
			(int)IPV4_BYTE(hostIp, 2), (int)IPV4_BYTE(hostIp, 3));
}

static void socket_cb(SOCKET sock, uint8_t u8Msg, void *pvMsg) {

	/* Check for socket event on TCP socket. */
	if (sock == tcp_client_socket) {

		switch (u8Msg) {
		  case SOCKET_MSG_CONNECT:
      {
        printf("[SOCKET] Connected\n");
			  if (gbTcpConnection) {
				  memset(gau8Buffer, 0, sizeof(send_buffer));
          tstrSocketConnectMsg *pstrConnect = (tstrSocketConnectMsg *)pvMsg;
				  if (pstrConnect && pstrConnect->s8Error >= SOCK_ERR_NO_ERROR) {
            socketConnected = true;
				  }
          else {
					  printf("[SOCKET] Connection Error\n");
					  gbTcpConnection = false;
					  close(tcp_client_socket);
					  tcp_client_socket = -1;
				  }
			  }
		  }
		  break;

		  case SOCKET_MSG_RECV:
		  {
			  char *pcIndxPtr;
			  char * request_ok = NULL;
        char * request_content = NULL;
        char led0_on, led1_on, led2_on;

			  tstrSocketRecvMsg *pstrRecv = (tstrSocketRecvMsg *)pvMsg;
			  if (pstrRecv && pstrRecv->s16BufferSize > 0) {
          request_ok = strstr(pstrRecv->pu8Buffer, "200 OK");

          if((state == POST_WAIT) || (state == POST_LDR)){
            receive_ok = true;
            printf("TEST");
          }
          if(request_ok && ((state == GET_LEDS) || (state == GET_WAIT)) ) {
            recv(tcp_client_socket, gau8Buffer, MAIN_WIFI_M2M_BUFFER_SIZE, 0);
            //printf(gau8Buffer);
          }

          request_content = strstr(pstrRecv->pu8Buffer, "LEDS#");
          if(request_content != NULL){
            //printf(request_content);
            led0_on = request_content[6];
            led1_on = request_content[7];
            led2_on = request_content[8];

            printf("LED_STATES %c %c %c \n", led0_on, led1_on, led2_on);

            if(led0_on == '1')
            pio_set(LED0_PIO, LED0_PIN_MASK);
            else
            pio_clear(LED0_PIO, LED0_PIN_MASK);

            if(led1_on == '1')
            pio_set(LED1_PIO, LED1_PIN_MASK);
            else
            pio_clear(LED1_PIO, LED1_PIN_MASK);

            if(led2_on == '1')
            pio_set(LED2_PIO, LED2_PIN_MASK);
            else
            pio_clear(LED2_PIO, LED2_PIN_MASK);
          }
			  }
        else {
          socketConnected = 0;
				  printf("[SOCKET] Receive Error\n");
				  close(tcp_client_socket);
				  tcp_client_socket = -1;
			  }
		  }
		  break;

		default:
			break;
		}
	}
}

static void set_dev_name_to_mac(uint8_t *name, uint8_t *mac_addr) {
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

static void wifi_cb(uint8_t u8MsgType, void *pvMsg) {
	switch (u8MsgType) {
	case M2M_WIFI_RESP_CON_STATE_CHANGED:
	{
		tstrM2mWifiStateChanged *pstrWifiState = (tstrM2mWifiStateChanged *)pvMsg;
		if (pstrWifiState->u8CurrState == M2M_WIFI_CONNECTED) {
			printf("[WI-FI] Connected\n");
			m2m_wifi_request_dhcp_client();
		} else if (pstrWifiState->u8CurrState == M2M_WIFI_DISCONNECTED) {
			printf("[WI-FI] Disconnected\n");
			gbConnectedWifi = false;
 			wifi_connected = 0;
		}

		break;
	}

	case M2M_WIFI_REQ_DHCP_CONF:
	{
		uint8_t *pu8IPAddress = (uint8_t *)pvMsg;
		wifi_connected = M2M_WIFI_CONNECTED;
		break;
	}

	default:
	{
		break;
	}
	}
}

/************************************************************************/
/* MAIN                                                                 */
/************************************************************************/
int main(void) {
	tstrWifiInitParam param;
	int8_t ret;
	uint8_t mac_addr[6];
	struct sockaddr_in addr_in;

	/* Initialize the board. */
	sysclk_init();
	board_init();

	/* Initialize the UART console. */
	CONSOLE_init();
	printf("Home Controller v1.0\n");

	/* Initialize the BSP. */
	nm_bsp_init();

	/* Initialize Wi-Fi parameters structure. */
	memset((uint8_t *)&param, 0, sizeof(tstrWifiInitParam));

	/* Initialize Wi-Fi driver with data and status callbacks. */
	param.pfAppWifiCb = wifi_cb;
	ret = m2m_wifi_init(&param);
	if (M2M_SUCCESS != ret) {
		printf("m2m_wifi_init call error! (%d)\r\n", ret);
		while (1) {}
	}

	/* Initialize socket module. */
	socketInit();

	/* Register socket callback function. */
	registerSocketCallback(socket_cb, resolve_cb);

  /* Connect to router. */
	printf("[WI-FI] Connecting to SSID: %s\n", (char *)MAIN_WLAN_SSID);
	m2m_wifi_connect((char *)MAIN_WLAN_SSID, sizeof(MAIN_WLAN_SSID), MAIN_WLAN_AUTH, (char *)MAIN_WLAN_PSK, M2M_WIFI_CH_ALL);

	addr_in.sin_family = AF_INET;
	addr_in.sin_port = _htons(MAIN_SERVER_PORT);
  inet_aton(MAIN_SERVER_NAME, &addr_in.sin_addr);
  //printf("Inet aton : %d", addr_in.sin_addr);

  LED_init(LED0_PIO, LED0_PIO_ID, LED0_PIN_MASK, 1);
  LED_init(LED1_PIO, LED1_PIO_ID, LED1_PIN_MASK, 1);
  LED_init(LED2_PIO, LED2_PIO_ID, LED2_PIN_MASK, 1);
  LED_init(LED3_PIO, LED3_PIO_ID, LED3_PIN_MASK, 1);
  TC_init(TC0, ID_TC0, 1); // Every 1   second
  TC_init(TC1, ID_TC3, 4); // Every 0.5 second

  AFEC_init();

  while(1) {
 		m2m_wifi_handle_events(NULL);

   	if (wifi_connected == M2M_WIFI_CONNECTED) {
    	/* Open client socket. */
			if (tcp_client_socket < 0) {
        printf("[SOCKET] Initializing\n");
				if ((tcp_client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
					printf("[SOCKET] ERROR failed to create TCP client socket\n");
					continue;
				}

				/* Connect server */
        printf("[SOCKET] Connecting\n");

				if (connect(tcp_client_socket, (struct sockaddr *)&addr_in, sizeof(struct sockaddr_in)) != SOCK_ERR_NO_ERROR) {
					close(tcp_client_socket);
					tcp_client_socket = -1;
          printf("ERROR\n");
				}
        else
          gbTcpConnection = true;
			}
    }

    if(afec_done == true) {
      afec_done = false;

      if (g_ul_value < 3500) {
        printf("UL_VALUE");
        pio_clear(LED3_PIO, LED3_PIN_MASK);
      } else {
        printf("UL_VALUE_ELSE");
        pio_set(LED3_PIO, LED3_PIN_MASK);
      }
    }

    // STATE MACHINE
    switch (state) {

      case WAIT:
        receive_ok = false;
        if (init_state_machine)
          state = GET_LEDS;
        else
          state = WAIT;

        break;

      case POST_LDR:
        sprintf(ldr_buffer, "ldr=%lu", g_ul_value);
        build_post(gau8Buffer, "/ldrupdate", ldr_buffer);
        send(tcp_client_socket, gau8Buffer, strlen((char *)gau8Buffer), 0);
        memset(gau8Buffer, 0, sizeof(gau8Buffer));
        recv(tcp_client_socket, gau8Buffer, MAIN_WIFI_M2M_BUFFER_SIZE, 0);
        state = POST_WAIT;
        break;

      case POST_WAIT:
      printf("POST WAIT \n");
        if (receive_ok) {
          state = GET_LEDS;
          receive_ok = false;
        }
        else
          state = POST_WAIT;
        break;

      case GET_LEDS:
        printf("GET_LEDS\n");
        build_get(gau8Buffer, "/leds");
        send(tcp_client_socket, gau8Buffer, strlen((char *)gau8Buffer), 0);
        memset(gau8Buffer, 0, sizeof(gau8Buffer));
        recv(tcp_client_socket, gau8Buffer, MAIN_WIFI_M2M_BUFFER_SIZE, 0);
        state = WAIT;
        init_state_machine = false;
        printf("%d", g_ul_value);

        break;

       case GET_WAIT:
          printf("GET WAIT \n");

          if (receive_ok) {
            state = GET_LEDS;
            receive_ok = false;
          }
          else
          state = WAIT;
          break;

    }
  }

	return 0;
}

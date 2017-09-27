/**
 *    Computacao Embarcada - Computacao - Insper
 *
 *            Avaliacao Intermediaria
 *
 * Faça um firmware que permita a um usuário no computador acessar e configurar algumas
 * informações/ modos  de operação do microcontrolador. Essas funcionalidades devem ser
 * acessadas via comunicação serial (COM). Um menu deve informar ao usuário as possibilidades
 * e os comandos que devem ser digitados para operar o embarcado.
 *
 * Funcionalidades que o firmware deve possuir :
 *
 * 1. Exibir menu
 * 2. O usuário deve ser capaz de ligar/desligar o piscar led (led da placa)
 * 3. O usuário deve ser capaz de aumentar(+2 Hz) e diminuir (-2 Hz) a frequência do led
 * 4. O usuário deve ser capaz de ler o relógio do microcontrolador.
 *
 * Utilize o programa disponível no repositório (github.com/insper/Computacao-Embarcada/Avaliacoes/A1/)
 * como ponto de parida. O código deve fazer uso de interrupções e periféricos para gerenciar a
 * comunicação com o PC e o LED.
 *
 *  ## Extra (A)
 *
 *  1. O usuário deve ser capaz de entrar com um valor de frequência para o led de forma numérica no termina.
 *
 */

/************************************************************************/
/* Includes                                                              */
/************************************************************************/

#include "asf.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// PERIPHERALS
// USART
#define USART_COM     USART1
#define USART_COM_ID  ID_USART1

// LED
#define LED_PIO_ID ID_PIOC
#define LED_PIO PIOC
#define LED_PIN 8
#define LED_PIN_MASK (1 << LED_PIN)

// RTC
#define RTC_HOUR 0
#define RTC_MINUTE 0
#define RTC_SECOND 0
// END PERIPHERALS

// GLOBALS
uint8_t bufferRX[100];
uint8_t bufferTX[100];
uint32_t count = 0;
volatile uint32_t usart_transmission_done = 0;
volatile uint32_t led_blink = 0;
// END GLOBALS

// PROTOTYPES
void LED_init(int state);
void RTC_init();
void pin_toggle(Pio *pio, uint32_t mask);
uint32_t usart_puts(uint8_t *pstring);
uint32_t usart_gets(uint8_t *pstring);
// END PROTOTYPES

// FUNCTIONS
void display_menu() {
	usart_puts("Menu --------------------\n");
	usart_puts("[1] Turn On/Off\n");
	usart_puts("[2] Increase LED frequency\n");
	usart_puts("[3] Decrease LED frequency\n");
	usart_puts("[4] Set LED frequency\n");
	usart_puts("[5] Time\n");
	usart_puts("[h] Show this menu\n");
}

void pin_toggle(Pio *pio, uint32_t mask){
	if(pio_get_output_data_status(pio, mask))
		pio_clear(pio, mask);
	else
		pio_set(pio,mask);
}

void RTC_init() {
	/* Configura o PMC */
	pmc_enable_periph_clk(ID_RTC);

	/* Default RTC configuration, 24-hour mode */
	rtc_set_hour_mode(RTC, 0);

	/* Configura data e hora manualmente */
	rtc_set_time(RTC, RTC_HOUR, RTC_MINUTE, RTC_SECOND);
}

void LED_init(int state) {
	pmc_enable_periph_clk(LED_PIO_ID);
	pio_set_output(LED_PIO, LED_PIN_MASK, state, 0, 0);
};

static void USART1_init(void) {
	/* Configura USART1 Pinos */
	sysclk_enable_peripheral_clock(ID_PIOB);
	sysclk_enable_peripheral_clock(ID_PIOA);
	pio_set_peripheral(PIOB, PIO_PERIPH_D, PIO_PB4);  // RX
	pio_set_peripheral(PIOA, PIO_PERIPH_A, PIO_PA21); // TX
	MATRIX->CCFG_SYSIO |= CCFG_SYSIO_SYSIO4;

	/* Configura opcoes USART */
	const sam_usart_opt_t usart_settings = {
	.baudrate     = 115200,
	.char_length  = US_MR_CHRL_8_BIT,
	.parity_type  = US_MR_PAR_NO,
	.stop_bits    = US_MR_NBSTOP_1_BIT    ,
	.channel_mode = US_MR_CHMODE_NORMAL
	};

	/* Ativa Clock periferico USART0 */
	sysclk_enable_peripheral_clock(USART_COM_ID);

	/* Configura USART para operar em modo RS232 */
	usart_init_rs232(USART_COM, &usart_settings, sysclk_get_peripheral_hz());

	/* Enable the receiver and transmitter. */
	usart_enable_tx(USART_COM);
	usart_enable_rx(USART_COM);
	
	usart_enable_interrupt(USART_COM, US_IER_RXRDY);
	NVIC_EnableIRQ(USART_COM_ID);
}

static void TC1_init(uint32_t frequency) {
	uint32_t ul_div, ul_tcclks;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();

	pmc_enable_periph_clk(ID_TC1);

	// Configura o TC para operar em 1S e interrupcao no RC compare
	tc_find_mck_divisor(frequency, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
	tc_init(TC0, 1, ul_tcclks | TC_CMR_CPCTRG);
	tc_write_rc(TC0, 1, (ul_sysclk / ul_div) / frequency);

	NVIC_EnableIRQ((IRQn_Type) ID_TC1);
	tc_enable_interrupt(TC0, 1, TC_IER_CPCS);

	tc_start(TC0, 1);
}
// END FUNCTIONS

// HANDLERS
void TC1_Handler(void) {
	volatile uint32_t ul_dummy;
	ul_dummy = tc_get_status(TC0, 1);
	UNUSED(ul_dummy);
	
	if (led_blink)
		pin_toggle(LED_PIO, LED_PIN_MASK);
}

void USART1_Handler(void) {
	uint32_t ret = usart_get_status(USART_COM);
	uint8_t c = NULL;
	
	if(ret & US_IER_RXRDY) {
		usart_serial_getchar(USART_COM, &c);
		if(c != '\n') {
			bufferRX[count++] = c;
		}
		else {
			bufferRX[count] = 0x00;
			usart_transmission_done = 1;
			count = 0;
		}
	}
}

uint32_t usart_puts(uint8_t *pstring){
  uint32_t i = 0 ;

  while(*(pstring + i)){
    usart_serial_putchar(USART_COM, *(pstring+i++));
    while(!uart_is_tx_empty(USART_COM)){};
  }
  return(i);
}

/**
 * Busca do UART uma mensagem enviada pelo computador terminada em \n
 */
uint32_t usart_gets(uint8_t *pstring){
  uint32_t i = 0 ;
  usart_serial_getchar(USART_COM, (pstring+i));
  while(*(pstring+i) != '\n'){
    usart_serial_getchar(USART_COM, (pstring+(++i)));
  }
  *(pstring+i+1)= 0x00;
  return(i);

}

int compare_strings(char a[], char b[]) {
	int c = 0;
	
	while (a[c] == b[c]) {
		if (a[c] == '\0' || b[c] == '\0')
			break;
		c++;
	}
	
	if (a[c] == '\0' && b[c] == '\0')
		return 0;
	else
		return -1;
}

/************************************************************************/
/* Main Code	                                                        */
/************************************************************************/
int main(void) {
	
	board_init();
	sysclk_init();

	/* Disable the watchdog */
	WDT->WDT_MR = WDT_MR_WDDIS;

	/** Inicializa USART */
	USART1_init();
	TC1_init(10);
	LED_init(0);
	display_menu();
	led_blink = 1;
	uint32_t tc_frequency = 10;

	/** Super loop */
	while (1) {
		if(usart_transmission_done) {
			if (bufferRX[0] == '1')
				led_blink = !led_blink;
			else if (bufferRX[0] == '2') {
				tc_frequency += 2;
				TC1_init(tc_frequency);
			}
			else if (bufferRX[0] == '3') {
				tc_frequency -= 2;
				TC1_init(tc_frequency);
			}
			else if (bufferRX[0] == '4') {
				usart_puts("Enter the new frequency: ");
				usart_transmission_done = 0;
				while(!usart_transmission_done);
				usart_puts(bufferRX);
				usart_puts("\n");
				usart_transmission_done = 0;
				TC1_init(atoi(bufferRX));
			}
			else if (bufferRX[0] == '5') {
				uint32_t hour, minute, second;
				rtc_get_time(RTC, &hour, &minute, &second);
				uint8_t *string;
				sprintf(string, "TIME: %d:%d:%d\n", hour, minute, second);
				usart_puts(string);
			}
			else {
				display_menu();
			}
			usart_transmission_done = 0;
		}
	}
}

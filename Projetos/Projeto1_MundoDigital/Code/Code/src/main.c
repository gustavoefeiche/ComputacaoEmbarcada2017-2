/*
PERIPHERALS CONFIG

VCC: EXT1
GND: EXT1

TO BUZZER: PD20
FROM BUTTON1: PB0
FROM BUTTON2: PA3
FROM BUTTON3: PD28 
FROM REED: PC31
*/

#include "asf.h"
#include "conf_clock.h"
#include "conf_uart_serial.h"

#define ENABLE 1
#define DISABLE 0

#define LED_PIO_ID ID_PIOC
#define LED_PIO PIOC
#define LED_PIN 8
#define LED_PIN_MASK (1 << LED_PIN)

#define BUZZER_PIO_ID ID_PIOD
#define BUZZER_PIO PIOD
#define BUZZER_PIN 20
#define BUZZER_PIN_MASK (1 << BUZZER_PIN)

#define BUTTON1_PIO_ID ID_PIOB
#define BUTTON1_PIO PIOB
#define BUTTON1_PIN 0
#define BUTTON1_PIN_MASK (1 << BUTTON1_PIN)
#define BUTTON1_DEBOUNCING_VALUE 79

#define REED_PIO_ID ID_PIOC
#define REED_PIO PIOC
#define REED_PIN 31
#define REED_PIN_MASK (1 << REED_PIN)

// Prototypes
static void LED_init(int state);
static void BUZZER_init();
static void BUTTON1_init();
static void REED_init();

static void configure_console(void) {
	
	const usart_serial_options_t uart_serial_options = {
		.baudrate   = CONF_UART_BAUDRATE,
		.charlength = CONF_UART_CHAR_LENGTH,
		.paritytype = CONF_UART_PARITY,
		.stopbits   = CONF_UART_STOP_BITS,
	};

	// Configure console UART. //
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	stdio_serial_init(CONF_UART, &uart_serial_options);
}

void LED_init(int state) {
	pmc_enable_periph_clk(LED_PIO_ID);
	pio_set_output(LED_PIO, LED_PIN_MASK, state, 0, 0 );
};

static void BUZZER_init() {
	pmc_enable_periph_clk(BUZZER_PIO_ID);
	pio_set_output(BUZZER_PIO, BUZZER_PIN_MASK, 0, 0, 0);
	pio_pull_down(BUZZER_PIO, BUZZER_PIN_MASK, ENABLE);
}

static void BUTTON1_init() {
	pmc_enable_periph_clk(BUTTON1_PIO_ID);
	pio_set_input(BUTTON1_PIO, BUTTON1_PIN_MASK, PIO_PULLUP | PIO_DEBOUNCE);
}

static void REED_init() {
	pmc_enable_periph_clk(REED_PIO_ID);
	pio_set_input(REED_PIO, REED_PIN_MASK, PIO_PULLUP | PIO_DEBOUNCE);
}

int main(void) {
	
	sysclk_init();
	board_init();
	
	LED_init(0);
	BUZZER_init();
	BUTTON1_init();
	REED_init();
	
	while(1) {		
		uint32_t button_pressed = !pio_get(BUTTON1_PIO, PIO_INPUT, BUTTON1_PIN_MASK);
		uint32_t reed_closed = !pio_get(REED_PIO, PIO_INPUT, REED_PIN_MASK);
		
		if(button_pressed) {
			pio_set(BUZZER_PIO, BUZZER_PIN_MASK);
		} else {
			pio_clear(BUZZER_PIO, BUZZER_PIN_MASK);
		}
		
		if(reed_closed) {
			pio_set(LED_PIO, LED_PIN_MASK);
		} else {
			pio_clear(LED_PIO, LED_PIN_MASK);
		}
	}
}

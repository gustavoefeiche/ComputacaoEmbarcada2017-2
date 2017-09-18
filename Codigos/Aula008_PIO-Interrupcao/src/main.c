#include "asf.h"
#include "conf_clock.h"

// PERIPHERALS
/*
	LEDs:
		Board: PC8
		OLED1: PA0 (EXT1)
		OLED2: PC30 (EXT1)
		OLED3: PB2 (EXT1)
	
	Buttons:
		Board: PA11
		OLED1: PD28 (EXT1)
		OLED2: PC31 (EXT1)
		OLED3: PA19 (EXT1)
*/

#define BOARD_LED_PIO_ID ID_PIOC
#define BOARD_LED_PIO PIOC
#define BOARD_LED_PIN 8
#define BOARD_LED_PIN_MASK (1 << BOARD_LED_PIN)

#define OLED_LED1_PIO_ID ID_PIOA
#define OLED_LED1_PIO PIOA
#define OLED_LED1_PIN 0
#define OLED_LED1_PIN_MASK (1 << OLED_LED1_PIN)

#define OLED_LED2_PIO_ID ID_PIOC
#define OLED_LED2_PIO PIOC
#define OLED_LED2_PIN 30
#define OLED_LED2_PIN_MASK (1 << OLED_LED2_PIN)

#define OLED_LED3_PIO_ID ID_PIOB
#define OLED_LED3_PIO PIOB
#define OLED_LED3_PIN 2
#define OLED_LED3_PIN_MASK (1 << OLED_LED3_PIN)

#define BOARD_BUT_PIO_ID ID_PIOA
#define BOARD_BUT_PIO PIOA
#define BOARD_BUT_PIN 11
#define BOARD_BUT_PIN_MASK (1 << BOARD_BUT_PIN)

#define OLED_BUT1_PIO_ID ID_PIOD
#define OLED_BUT1_PIO PIOD
#define OLED_BUT1_PIN 28
#define OLED_BUT1_PIN_MASK (1 << OLED_BUT1_PIN)

#define OLED_BUT2_PIO_ID ID_PIOC
#define OLED_BUT2_PIO PIOC
#define OLED_BUT2_PIN 31
#define OLED_BUT2_PIN_MASK (1 << OLED_BUT2_PIN)

#define OLED_BUT3_PIO_ID ID_PIOA
#define OLED_BUT3_PIO PIOA
#define OLED_BUT3_PIN 19
#define OLED_BUT3_PIN_MASK (1 << OLED_BUT3_PIN)

#define BUT_DEBOUNCING_VALUE  79

// PROTOTYPES
void WATCHDOG_init(void);
void LED_init(Pio *p_pio, uint32_t pio_id, uint32_t pin_mask, uint32_t state);
void BUTTON_init(Pio *p_pio, uint32_t pio_id, uint32_t pin_mask, void (*p_handler)(uint32_t, uint32_t), uint32_t interruption_priority);
void BOARD_BUTTON_handler(uint32_t a, uint32_t b);
void OLED_BUTTON1_handler(uint32_t a, uint32_t b);
void OLED_BUTTON2_handler(uint32_t a, uint32_t b);
void OLED_BUTTON3_handler(uint32_t a, uint32_t b);

// HANDLERS
void BOARD_BUTTON_handler(uint32_t a, uint32_t b) {
	uint32_t pioIntStatus;
	pioIntStatus = pio_get_interrupt_status(BOARD_BUT_PIO);
	UNUSED(pioIntStatus);
	
	if(pio_get_output_data_status(BOARD_LED_PIO, BOARD_LED_PIN_MASK))
		pio_clear(BOARD_LED_PIO, BOARD_LED_PIN_MASK);
	else
		pio_set(BOARD_LED_PIO, BOARD_LED_PIN_MASK);
	
}

void OLED_BUTTON1_handler(uint32_t a, uint32_t b) {
	uint32_t pioIntStatus;
    pioIntStatus = pio_get_interrupt_status(OLED_BUT1_PIO);
	UNUSED(pioIntStatus);
	
	if(pio_get_output_data_status(OLED_LED1_PIO, OLED_LED1_PIN_MASK))
		pio_clear(OLED_LED1_PIO, OLED_LED1_PIN_MASK);
	else
		pio_set(OLED_LED1_PIO, OLED_LED1_PIN_MASK); 
}

void OLED_BUTTON2_handler(uint32_t a, uint32_t b) {
	uint32_t pioIntStatus;
	pioIntStatus = pio_get_interrupt_status(OLED_BUT2_PIO);
	UNUSED(pioIntStatus);
	
	if(pio_get_output_data_status(OLED_LED2_PIO, OLED_LED2_PIN_MASK))
		pio_clear(OLED_LED2_PIO, OLED_LED2_PIN_MASK);
	else
		pio_set(OLED_LED2_PIO, OLED_LED2_PIN_MASK);
	
}

void OLED_BUTTON3_handler(uint32_t a, uint32_t b) {
	uint32_t pioIntStatus;
	pioIntStatus = pio_get_interrupt_status(OLED_BUT3_PIO);
	UNUSED(pioIntStatus);
	
	if(pio_get_output_data_status(OLED_LED3_PIO, OLED_LED3_PIN_MASK))
		pio_clear(OLED_LED3_PIO, OLED_LED3_PIN_MASK);
	else
		pio_set(OLED_LED3_PIO, OLED_LED3_PIN_MASK);
	
}

// CONFIG
void WATCHDOG_init(void) {
	WDT->WDT_MR = WDT_MR_WDDIS;
}

void LED_init(Pio *p_pio, uint32_t pio_id, uint32_t pin_mask, uint32_t state) {
    pmc_enable_periph_clk(pio_id);
    pio_set_output(p_pio, pin_mask, !state, 0, 0);
};

void BUTTON_init(Pio *p_pio, uint32_t pio_id, uint32_t pin_mask, void (*p_handler)(uint32_t, uint32_t), uint32_t interrupt_type) {
    pmc_enable_periph_clk(pio_id);
    pio_set_input(p_pio, pin_mask, PIO_PULLUP | PIO_DEBOUNCE);
    
    // Config interrupt as falling edge and set handler
    pio_enable_interrupt(p_pio, pin_mask);
	
	pio_handler_set(p_pio, pio_id, pin_mask, interrupt_type, *p_handler);
	
	// Enable interruption for peripheral and set interrupt priority    
    NVIC_EnableIRQ(pio_id);
    NVIC_SetPriority(pio_id, 1);
};

// MAIN
int main(void) {
	board_init();
	sysclk_init();
	
	WATCHDOG_init();
	LED_init(BOARD_LED_PIO, BOARD_LED_PIO_ID, BOARD_LED_PIN_MASK, 1);
	LED_init(OLED_LED1_PIO, OLED_LED1_PIO_ID, OLED_LED1_PIN_MASK, 1);
	LED_init(OLED_LED2_PIO, OLED_LED2_PIO_ID, OLED_LED2_PIN_MASK, 1);
	LED_init(OLED_LED3_PIO, OLED_LED3_PIO_ID, OLED_LED3_PIN_MASK, 1);
    BUTTON_init(BOARD_BUT_PIO, BOARD_BUT_PIO_ID, BOARD_BUT_PIN_MASK, BOARD_BUTTON_handler, PIO_IT_FALL_EDGE);
	BUTTON_init(OLED_BUT1_PIO, OLED_BUT1_PIO_ID, OLED_BUT1_PIN_MASK, OLED_BUTTON1_handler, PIO_IT_FALL_EDGE);
	BUTTON_init(OLED_BUT2_PIO, OLED_BUT2_PIO_ID, OLED_BUT2_PIN_MASK, OLED_BUTTON2_handler, PIO_IT_RISE_EDGE);
	BUTTON_init(OLED_BUT3_PIO, OLED_BUT3_PIO_ID, OLED_BUT3_PIN_MASK, OLED_BUTTON3_handler, PIO_IT_EDGE);
	
	while(1) {
		// sleep mode
		pmc_sleep(SLEEPMGR_SLEEP_WFI);
		
		uint32_t i = 0;
		while(i <= 6) {
			delay_ms(250);
			pio_clear(BOARD_LED_PIO, BOARD_LED_PIN_MASK);
			delay_ms(250);
			pio_set(BOARD_LED_PIO, BOARD_LED_PIN_MASK);
			i++;
		}
	};
}
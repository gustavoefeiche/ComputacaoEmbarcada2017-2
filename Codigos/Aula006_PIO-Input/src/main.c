/*
OLED XPLAINED PRO

BUTTON 1 : PIN 9 -> PD28
BUTTON 2 : PIN 3 -> PC31
BUTTON 3 : PIN 4 -> PA19
LED 1    : PIN 7 -> PA0
LED 2    : PIN 8 -> PC30
LED 3    : PIN 6 -> PB2

*/

/* INCLUDES */
#include "asf.h"
#include "conf_clock.h"
/* /INCLUDES */

/* PERIPHERALS */
#define LED_PIO_ID ID_PIOC
#define LED_PIO PIOC
#define LED_PIN 8
#define LED_PIN_MASK (1 << LED_PIN)

#define BUT_PIO_ID ID_PIOA
#define BUT_PIO PIOA
#define BUT_PIN 11
#define BUT_PIN_MASK (1 << BUT_PIN)

#define OLED_BUT1_PIO_ID ID_PIOD
#define OLED_BUT1_PIO PIOD
#define OLED_BUT1_PIN 28
#define OLED_BUT1_PIN_MASK (1 << OLED_BUT1_PIN)

#define OLED_BUT3_PIO_ID ID_PIOA
#define OLED_BUT3_PIO PIOA
#define OLED_BUT3_PIN 19
#define OLED_BUT3_PIN_MASK (1 << OLED_BUT3_PIN)
/* /PERIPHERALS */

/* PROTOTYPES */
void BUTTON_init(int pio_id, Pio *p_pio, int pin_mask);
void LED_init(Pio *p_pio, uint32_t pio_id, uint32_t pin_mask, uint32_t state);
/* /PROTOTYPES */

/* INITIALIZATION */
void LED_init(Pio *p_pio, uint32_t pio_id, uint32_t pin_mask, uint32_t state) {
	pmc_enable_periph_clk(pio_id);
	pio_set_output(p_pio, pin_mask, !state, 0, 0);
}

void BUTTON_init(int pio_id, Pio *p_pio, int pin_mask) {
	pmc_enable_periph_clk(pio_id);
	pio_set_input(p_pio, pin_mask, PIO_PULLUP | PIO_DEBOUNCE);
}
/* /INITIALIZATION */

/* MAIN ROUTINE */
int main(void) {
	sysclk_init();
	board_init();
	
	uint32_t led_blink = 1;
	uint32_t blink_delay_ms = 100;

	WDT->WDT_MR = WDT_MR_WDDIS;
	
	LED_init(LED_PIO, LED_PIO_ID, LED_PIN_MASK, 0);
	BUTTON_init(BUT_PIO_ID, BUT_PIO, BUT_PIN_MASK);
	BUTTON_init(OLED_BUT1_PIO_ID, OLED_BUT1_PIO, OLED_BUT1_PIN_MASK);
	BUTTON_init(OLED_BUT3_PIO_ID, OLED_BUT3_PIO, OLED_BUT3_PIN_MASK);
	
	while(1) {
		delay_ms(100);
		uint32_t user_button_pressed = !pio_get(BUT_PIO, PIO_INPUT, BUT_PIN_MASK);
		uint32_t oled_but1_pressed = !pio_get(OLED_BUT1_PIO, PIO_INPUT, OLED_BUT1_PIN_MASK);
		uint32_t oled_but3_pressed = !pio_get(OLED_BUT3_PIO, PIO_INPUT, OLED_BUT3_PIN_MASK);
		
		if(user_button_pressed)
			led_blink = !led_blink;
			
		if(oled_but1_pressed)
			blink_delay_ms += 50;
			
		if(oled_but3_pressed)
			blink_delay_ms -= 50;
			
		if(led_blink) {
			pio_set(LED_PIO, LED_PIN_MASK);
			delay_ms(blink_delay_ms);
			pio_clear(LED_PIO, LED_PIN_MASK);
		}
		else {
			pio_set(LED_PIO, LED_PIN_MASK);
		}
	}	
}
/* /MAIN ROUTINE */
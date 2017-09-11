#include "asf.h"
#include "conf_clock.h"
#include "./driver/pio_insper.h"
#include "./driver/pmc_insper.h"

#define LED_PIO_ID ID_PIOC
#define LED_PIO PIOC
#define LED_PIN 8
#define LED_PIN_MASK (1 << LED_PIN)

#define BUT_PIO_ID ID_PIOA
#define BUT_PIO PIOA
#define BUT_PIN 11
#define BUT_PIN_MASK (1 << BUT_PIN)
#define BUT_DEBOUNCING_VALUE 79

void led_init(int state);
void but_init(void);

void led_init(int state) {
	_pmc_enable_periph_clock(LED_PIO_ID); // Enable PMC peripheral clock
	_pio_set_output(LED_PIO, LED_PIN_MASK, !state, 0); // Set LED pin as output
};

void but_init(void){
	_pmc_enable_periph_clock(BUT_PIO_ID); // Enable PMC peripheral clock
	_pio_set_input(BUT_PIO, BUT_PIN_MASK, PIO_PULLUP | PIO_DEBOUNCE); // Set button pin as input
};

int main(void) {
	sysclk_init();
	board_init();
	
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	led_init(1);
	but_init();

	while(1) {
	    if(_pio_get_output_data_status(LED_PIO, LED_PIN_MASK)) {
			_pio_clear(LED_PIO, LED_PIN_MASK);
			delay_ms(500);
        }
		else {
			_pio_set(LED_PIO, LED_PIN_MASK);
			delay_ms(500);
        }
	}
}

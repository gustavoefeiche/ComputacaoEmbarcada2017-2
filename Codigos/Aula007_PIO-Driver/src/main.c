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
	PMC->PMC_PCER0 = (1<<LED_PIO_ID); // Enable PMC peripheral clock
	_pio_set_output(LED_PIO, LED_PIN_MASK, 1, 0);
};

void but_init(void){
	PMC->PMC_PCER0       = (1<<BUT_PIO_ID);      // Enable PMC peripheral clock
	BUT_PIO->PIO_ODR	 = BUT_PIN_MASK;         // Disable output (Output DISABLE register)
	BUT_PIO->PIO_PER	 = BUT_PIN_MASK;         // Enable PIO controller for the pin (PIO ENABLE register)
	BUT_PIO->PIO_PUER	 = BUT_PIN_MASK;         // Enable pullup (PullUp ENABLE register)
	BUT_PIO->PIO_IFER	 = BUT_PIN_MASK;         // Enable debouncing
	BUT_PIO->PIO_IFSCER  = BUT_PIN_MASK;         // Enable debouncing clock
	BUT_PIO->PIO_SCDR	 = BUT_DEBOUNCING_VALUE; // Change debouncing frequency
};

int main(void) {
	sysclk_init();
	board_init();
	
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	led_init(1);
	but_init();

	while(1) {
	    if(BUT_PIO->PIO_PDSR & (BUT_PIN_MASK)) {
			LED_PIO->PIO_CODR = LED_PIN_MASK;
        }
		else {
			LED_PIO->PIO_SODR = LED_PIN_MASK;
        }
	}
}

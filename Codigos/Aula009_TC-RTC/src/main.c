#include "asf.h"

/* PERIPHERALS

	LEDs:
		BOARD: PC8
		OLED1: PA0 (EXT1)
		OLED2: PC30 (EXT1)
		OLED3: PB2 (EXT1)
	
	Buttons:
		BOARD: PA11
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

#define RTC_YEAR 2017
#define RTC_MONTH 9
#define RTC_WEEK 0
#define RTC_DAY 18
#define RTC_HOUR 17
#define RTC_MINUTE 0
#define RTC_SECOND 0

#define TC_FREQUENCY_HZ 4

// GLOBALS
volatile uint8_t flag_led0 = 1;

// PROTOTYPES
void BUT_init(void);
void LED_init(int state);
void TC1_init(uint32_t frequency_hz);
void RTC_init(void);
void pin_toggle(Pio *pio, uint32_t mask);

// HANDLERS
static void Button1_Handler(uint32_t id, uint32_t mask) {
	
}

void TC1_Handler(void) {
	volatile uint32_t ul_dummy;
	ul_dummy = tc_get_status(TC0, 1);
	UNUSED(ul_dummy);

    if(flag_led0)
        pin_toggle(BOARD_LED_PIO, BOARD_LED_PIN_MASK);
}

void RTC_Handler(void) {
	uint32_t ul_status = rtc_get_status(RTC);

	/* Second increment interrupt */
	if ((ul_status & RTC_SR_SEC) == RTC_SR_SEC)
		rtc_clear_status(RTC, RTC_SCCR_SECCLR);
	else {
		/* Time or date alarm */
		if ((ul_status & RTC_SR_ALARM) == RTC_SR_ALARM) {
			flag_led0 = 0;    
			rtc_clear_status(RTC, RTC_SCCR_ALRCLR);
		}
	}
}

// FUNCTIONS
void pin_toggle(Pio *pio, uint32_t mask) {
	if(pio_get_output_data_status(pio, mask))
		pio_clear(pio, mask);
	else
		pio_set(pio,mask);
}

void BUT_init(void) {
    pmc_enable_periph_clk(BOARD_BUT_PIO_ID);
    pio_set_input(BOARD_BUT_PIO, BOARD_BUT_PIN_MASK, PIO_PULLUP | PIO_DEBOUNCE);
    
    pio_enable_interrupt(BOARD_BUT_PIO, BOARD_BUT_PIN_MASK);
    pio_handler_set(BOARD_BUT_PIO, BOARD_BUT_PIO_ID, BOARD_BUT_PIN_MASK, PIO_IT_FALL_EDGE, Button1_Handler);

    NVIC_EnableIRQ(BOARD_BUT_PIO_ID);
    NVIC_SetPriority(BOARD_BUT_PIO_ID, 1);
}

void LED_init(int state) {
    pmc_enable_periph_clk(BOARD_LED_PIO_ID);
    pio_set_output(BOARD_LED_PIO, BOARD_LED_PIN_MASK, !state, 0, 0);
};

/**
 * Configura TimerCounter (TC0) para gerar uma interrupcao no canal 0-(ID_TC1) 
 * a cada 250 ms (4Hz)
 */
void TC1_init(uint32_t frequency_hz) {   
    uint32_t ul_div;
    uint32_t ul_tcclks;
    uint32_t ul_sysclk = sysclk_get_cpu_hz();
    
    uint32_t channel = 1;
    
    /* Configura o PMC */
    pmc_enable_periph_clk(ID_TC0);    

    /** Configura o TC para operar em  4Mhz e interrup?c?o no RC compare */
    tc_find_mck_divisor(frequency_hz, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
    tc_init(TC0, channel, ul_tcclks | TC_CMR_CPCTRG);
    tc_write_rc(TC0, channel, (ul_sysclk / ul_div) / frequency_hz);

    /* Configura e ativa interrupção no TC canal 0 */
    NVIC_EnableIRQ((IRQn_Type) ID_TC1);
    tc_enable_interrupt(TC0, channel, TC_IER_CPCS);

    /* Inicializa o canal 0 do TC */
    tc_start(TC0, channel);
}

/**
 * Configura o RTC para funcionar com interrupcao de alarme
 */
void RTC_init(){
    /* Configura o PMC */
    pmc_enable_periph_clk(ID_RTC);
        
    /* Default RTC configuration, 24-hour mode */
    rtc_set_hour_mode(RTC, 0);

    /* Configura data e hora manualmente */
    rtc_set_date(RTC, RTC_YEAR, RTC_MONTH, RTC_DAY, RTC_WEEK);
    rtc_set_time(RTC, RTC_HOUR, RTC_MINUTE, RTC_SECOND);

    /* Configure RTC interrupts */
    NVIC_DisableIRQ(RTC_IRQn);
    NVIC_ClearPendingIRQ(RTC_IRQn);
    NVIC_SetPriority(RTC_IRQn, 0);
    NVIC_EnableIRQ(RTC_IRQn);
    
    /* Ativa interrupcao via alarme */
    rtc_enable_interrupt(RTC,  RTC_IER_ALREN); 
    
}

/************************************************************************/
/* Main Code	                                                        */
/************************************************************************/
int main(void) {
	sysclk_init();
	board_init();

	WDT->WDT_MR = WDT_MR_WDDIS;

	LED_init(1);
	BUT_init();    
	TC1_init(TC_FREQUENCY_HZ);
	RTC_init();
    
	rtc_set_date_alarm(RTC, 1, RTC_MONTH, 1, RTC_DAY);
	rtc_set_time_alarm(RTC, 1, RTC_HOUR, 1, RTC_MINUTE + 1, 1, RTC_SECOND);
        
	while (1) {
		pmc_sleep(SLEEPMGR_SLEEP_WFI);
	}

}
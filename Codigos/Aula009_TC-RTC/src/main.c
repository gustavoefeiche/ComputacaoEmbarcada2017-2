/************************************************************************/
/* INCLUDES                                                             */
/************************************************************************/
#include "asf.h"

/************************************************************************/
/* DEFINES                                                              */
/************************************************************************/
#define RTC_YEAR        2017
#define RTC_MONTH       9
#define RTC_DAY         30
#define RTC_WEEK        1
#define RTC_HOUR        0
#define RTC_MINUTE      0
#define RTC_SECOND      0

#define TC_CHANNEL 0
#define TC_STARTED 1
#define TC_STOPPED 0

#define ON 1
#define OFF 0

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

/************************************************************************/
/* GLOBALS                                                              */
/************************************************************************/
volatile uint8_t TC_ALL_status = TC_STARTED;
volatile uint8_t TC0_status    = TC_STARTED;
volatile uint8_t TC1_status    = TC_STARTED;
volatile uint8_t TC2_status    = TC_STARTED;
volatile uint8_t TC3_status    = TC_STARTED;

/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/
void BUT_init(Pio *p_pio, uint32_t ul_id, uint32_t ul_mask, void (*p_handler)(uint32_t, uint32_t));
void LED_init(Pio *p_pio, uint32_t ul_id, uint32_t ul_mask, uint32_t ul_default_level);
void TC_init(Tc *p_tc, uint32_t ul_id, uint32_t ul_freq);
void RTC_init(void);
void pin_toggle(Pio *pio, uint32_t mask);
void BOARD_BUTTON_handler(uint32_t a, uint32_t b);
void OLED_BUTTON1_handler(uint32_t a, uint32_t b);
void OLED_BUTTON2_handler(uint32_t a, uint32_t b);
void OLED_BUTTON3_handler(uint32_t a, uint32_t b);

/************************************************************************/
/* PERIPHERALS INITIALIZATION                                           */
/************************************************************************/
void BUT_init(Pio *p_pio, uint32_t ul_id, uint32_t ul_mask, void (*p_handler)(uint32_t, uint32_t)) {
	/* config. pino botao em modo de entrada */
	pmc_enable_periph_clk(ul_id);
	pio_set_input(p_pio, ul_mask, PIO_PULLUP | PIO_DEBOUNCE);

	/* config. interrupcao em borda de descida no botao do kit */
	/* indica funcao (but_Handler) a ser chamada quando houver uma interrupção */
	pio_enable_interrupt(p_pio, ul_mask);
	pio_handler_set(p_pio, ul_id, ul_mask, PIO_IT_RISE_EDGE, *p_handler);

	/* habilita interrupçcão do PIO que controla o botao */
	/* e configura sua prioridade                        */
	NVIC_EnableIRQ(ul_id);
	NVIC_SetPriority(ul_id, 1);
};

void LED_init(Pio *p_pio, uint32_t ul_id, uint32_t ul_mask, uint32_t ul_default_level) {
	pmc_enable_periph_clk(ul_id);
	pio_set_output(p_pio, ul_mask, !ul_default_level, 0, 0);
};

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

void RTC_init(void) {
	/* Configura o PMC */
	pmc_enable_periph_clk(ID_RTC);

	/* Default RTC configuration, 24-hour mode */
	rtc_set_hour_mode(RTC, 0);

	rtc_set_date(RTC, RTC_YEAR, RTC_MONTH, RTC_DAY, RTC_WEEK);
	rtc_set_time(RTC, RTC_HOUR, RTC_MINUTE, RTC_SECOND);

	NVIC_DisableIRQ(RTC_IRQn);
	NVIC_ClearPendingIRQ(RTC_IRQn);
	NVIC_SetPriority(RTC_IRQn, 0);
	NVIC_EnableIRQ(RTC_IRQn);

	rtc_enable_interrupt(RTC,  RTC_IER_SECEN);
}

/************************************************************************/
/* HANDLERS                                                             */
/************************************************************************/
void BOARD_BUTTON_handler(uint32_t a, uint32_t b) {
	uint32_t pioIntStatus;
	pioIntStatus = pio_get_interrupt_status(BOARD_BUT_PIO);
	UNUSED(pioIntStatus);

	if(TC_ALL_status == TC_STARTED) {
		if(TC0_status == TC_STARTED) {
			tc_stop(TC0, TC_CHANNEL);
			TC0_status = TC_STOPPED;
		} else {
			tc_start(TC0, TC_CHANNEL);
			TC0_status = TC_STARTED;
		}
	}
}

void OLED_BUTTON1_handler(uint32_t a, uint32_t b) {
	uint32_t pioIntStatus;
	pioIntStatus = pio_get_interrupt_status(OLED_BUT1_PIO);
	UNUSED(pioIntStatus);

	if(TC_ALL_status == TC_STARTED) {
		if(TC1_status == TC_STARTED) {
			tc_stop(TC1, TC_CHANNEL);
			TC1_status = TC_STOPPED;
		} else {
			tc_start(TC1, TC_CHANNEL);
			TC1_status = TC_STARTED;
		}
	}
}

void OLED_BUTTON2_handler(uint32_t a, uint32_t b) {
	uint32_t pioIntStatus;
	pioIntStatus = pio_get_interrupt_status(OLED_BUT2_PIO);
	UNUSED(pioIntStatus);

	if(TC_ALL_status == TC_STARTED) {
		if(TC2_status == TC_STARTED) {
			tc_stop(TC2, TC_CHANNEL);
			TC2_status = TC_STOPPED;
		} else {
			tc_start(TC2, TC_CHANNEL);
			TC2_status = TC_STARTED;
		}
	}

}

void OLED_BUTTON3_handler(uint32_t a, uint32_t b) {
	uint32_t pioIntStatus;
	pioIntStatus = pio_get_interrupt_status(OLED_BUT3_PIO);
	UNUSED(pioIntStatus);

	if(TC_ALL_status == TC_STARTED) {
		if(TC3_status == TC_STARTED) {
			tc_stop(TC3, TC_CHANNEL);
			TC3_status = TC_STOPPED;
		} else {
			tc_start(TC3, TC_CHANNEL);
			TC3_status = TC_STARTED;
		}
	}

}

// TC0 channel 0
void TC0_Handler(void) {
	volatile uint32_t ul_dummy;
	ul_dummy = tc_get_status(TC0, TC_CHANNEL);
	UNUSED(ul_dummy);

    pin_toggle(BOARD_LED_PIO, BOARD_LED_PIN_MASK);
}

// TC1 channel 0
void TC3_Handler(void) {
	volatile uint32_t ul_dummy;
	ul_dummy = tc_get_status(TC1, TC_CHANNEL);
	UNUSED(ul_dummy);

	pin_toggle(OLED_LED1_PIO, OLED_LED1_PIN_MASK);
}

// TC2 channel 0
void TC6_Handler(void) {
	volatile uint32_t ul_dummy;
	ul_dummy = tc_get_status(TC2, TC_CHANNEL);
	UNUSED(ul_dummy);

	pin_toggle(OLED_LED2_PIO, OLED_LED2_PIN_MASK);
}

// TC3 channel 0
void TC9_Handler(void) {
	volatile uint32_t ul_dummy;
	ul_dummy = tc_get_status(TC3, TC_CHANNEL);
	UNUSED(ul_dummy);

	pin_toggle(OLED_LED3_PIO, OLED_LED3_PIN_MASK);
}

void RTC_Handler(void) {
	uint32_t ul_status = rtc_get_status(RTC);

	/* Second increment interrupt */
	if ((ul_status & RTC_SR_SEC) == RTC_SR_SEC) {
		rtc_clear_status(RTC, RTC_SCCR_SECCLR);
	}
	else {
		if ((ul_status & RTC_SR_ALARM) == RTC_SR_ALARM) {
			if(TC_ALL_status == TC_STARTED) {
				tc_stop(TC0, TC_CHANNEL);
				tc_stop(TC1, TC_CHANNEL);
				tc_stop(TC2, TC_CHANNEL);
				tc_stop(TC3, TC_CHANNEL);
				TC_ALL_status = TC_STOPPED;
			} else {
				tc_start(TC0, TC_CHANNEL);
				tc_start(TC1, TC_CHANNEL);
				tc_start(TC2, TC_CHANNEL);
				tc_start(TC3, TC_CHANNEL);
				TC_ALL_status = TC_STARTED;
			}
			rtc_clear_status(RTC, RTC_SCCR_ALRCLR);
		}
	}

	uint32_t ul_year, ul_month, ul_day, ul_week, ul_hour, ul_minute, ul_second;
	rtc_get_date(RTC, &ul_year, &ul_month, &ul_day, &ul_week);
	rtc_get_time(RTC, &ul_hour, &ul_minute, &ul_second);

	rtc_set_date_alarm(RTC, 1, ul_month, 1, ul_day);
	rtc_set_time_alarm(RTC, 1, ul_hour, 1, ul_minute + 1, 1, ul_second);
}

/************************************************************************/
/* FUNCTIONS                                                            */
/************************************************************************/
void pin_toggle(Pio *pio, uint32_t mask) {
	if(pio_get_output_data_status(pio, mask))
		pio_clear(pio, mask);
	else
		pio_set(pio,mask);
}

/************************************************************************/
/* MAIN CODE	                                                        */
/************************************************************************/
int main(void) {
	board_init();
	sysclk_init();
	wdt_disable(WDT);

	LED_init(BOARD_LED_PIO, BOARD_LED_PIO_ID, BOARD_LED_PIN_MASK, ON);
	LED_init(OLED_LED1_PIO, OLED_LED1_PIO_ID, OLED_LED1_PIN_MASK, ON);
	LED_init(OLED_LED2_PIO, OLED_LED2_PIO_ID, OLED_LED2_PIN_MASK, ON);
	LED_init(OLED_LED3_PIO, OLED_LED3_PIO_ID, OLED_LED3_PIN_MASK, ON);

	BUT_init(BOARD_BUT_PIO, BOARD_BUT_PIO_ID, BOARD_BUT_PIN_MASK, BOARD_BUTTON_handler);
	BUT_init(OLED_BUT1_PIO, OLED_BUT1_PIO_ID, OLED_BUT1_PIN_MASK, OLED_BUTTON1_handler);
	BUT_init(OLED_BUT2_PIO, OLED_BUT2_PIO_ID, OLED_BUT2_PIN_MASK, OLED_BUTTON2_handler);
	BUT_init(OLED_BUT3_PIO, OLED_BUT3_PIO_ID, OLED_BUT3_PIN_MASK, OLED_BUTTON3_handler);

	TC_init(TC0, ID_TC0, 2);
	TC_init(TC1, ID_TC3, 8);
	TC_init(TC2, ID_TC6, 11);
	TC_init(TC3, ID_TC9, 17);

	RTC_init();

	rtc_set_date_alarm(RTC, 1, RTC_MONTH, 1, RTC_DAY);
	rtc_set_time_alarm(RTC, 1, RTC_HOUR, 1, RTC_MINUTE + 1, 1, RTC_SECOND);

	while (1) {
		/* Entra em modo sleep */
	}
}

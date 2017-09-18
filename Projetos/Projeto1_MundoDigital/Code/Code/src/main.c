#include "asf.h"
#include "conf_clock.h"

#define ENABLE 1
#define DISABLE 0
#define ON 1
#define OFF 0

#define LED_PIO_ID ID_PIOC
#define LED_PIO PIOC
#define LED_PIN 8
#define LED_PIN_MASK (1 << LED_PIN)

#define BUZZER_PIO_ID ID_PIOD
#define BUZZER_PIO PIOD
#define BUZZER_PIN 30
#define BUZZER_PIN_MASK (1 << BUZZER_PIN)

#define BUTTON1_PIO_ID ID_PIOA
#define BUTTON1_PIO PIOA
#define BUTTON1_PIN 0
#define BUTTON1_PIN_MASK (1 << BUTTON1_PIN)
#define BUTTON1_DEBOUNCING_VALUE 79

#define BUTTON2_PIO_ID ID_PIOB
#define BUTTON2_PIO PIOB
#define BUTTON2_PIN 3
#define BUTTON2_PIN_MASK (1 << BUTTON2_PIN)
#define BUTTON2_DEBOUNCING_VALUE 79

#define BUTTON3_PIO_ID ID_PIOC
#define BUTTON3_PIO PIOC
#define BUTTON3_PIN 31
#define BUTTON3_PIN_MASK (1 << BUTTON3_PIN)
#define BUTTON3_DEBOUNCING_VALUE 79

#define REED_PIO_ID ID_PIOD
#define REED_PIO PIOC
#define REED_PIN 25
#define REED_PIN_MASK (1 << REED_PIN)

#define PRESENCE_PIO_ID ID_PIOD
#define PRESENCE_PIO PIOD
#define PRESENCE_PIN 25
#define PRESENCE_PIN_MASK (1 << PRESENCE_PIN)

#define RTC_YEAR 2017
#define RTC_MONTH 9
#define RTC_DAY 7
#define RTC_WEEK 36
#define RTC_HOUR 12
#define RTC_MINUTE 0
#define RTC_SECOND 0

#define TC_FREQUENCY 4 // X Hz = 1/X seconds

#define BUZZER_ALARM_LIMIT_S 10

#define ALARM_DELAY_MAX BUZZER_ALARM_LIMIT_S * TC_FREQUENCY

#define PASSWORD_LENGTH 5

// Globals
uint8_t has_presence = 0; // Presence Sensor
uint8_t is_buzzing = 0; // Buzzer
uint8_t is_correct = 0; // Button Password
uint8_t is_open = 0; // Reed Switch
uint32_t can_buzz = 1;
uint32_t alarm_set = 0;
uint32_t correct_password[] = {1, 2, 3, 2, 1};
uint32_t input_password[] = {0, 0, 0, 0, 0};
uint8_t curr_index = 0;
uint8_t input_password_len = 0; 
uint8_t presence_to_buzzer_interval = 0;
uint8_t presence_to_buzzer_max = BUZZER_ALARM_LIMIT_S * TC_FREQUENCY;
uint32_t correct = 0;
uint32_t rtc_interruption_count = 0;

// Prototypes
static void LED_init(int state);
static void BUZZER_init(void);
static void BUTTON_init(uint32_t pio_id, Pio *p_pio, uint32_t mask);
static void PRESENCE_init(void);
static void CONSOLE_init(void);
void BUTTON_handler();
void PRESENCE_handler();
static void pin_toggle(Pio *pio, uint32_t mask);
static void printv(uint32_t *v, uint32_t n);

static void LED_init(int state) {
	pmc_enable_periph_clk(LED_PIO_ID);
	pio_set_output(LED_PIO, LED_PIN_MASK, !state, 0, 0);
};

static void BUZZER_init(void) {
	pmc_enable_periph_clk(BUZZER_PIO_ID);
	pio_set_output(BUZZER_PIO, BUZZER_PIN_MASK, 0, 0, 0);
	pio_pull_down(BUZZER_PIO, BUZZER_PIN_MASK, ENABLE);
}

static void BUTTON_init(uint32_t pio_id, Pio *p_pio, uint32_t mask) {
	pmc_enable_periph_clk(pio_id);
	pio_set_input(p_pio, mask, PIO_PULLUP | PIO_DEBOUNCE);
	
	pio_enable_interrupt(p_pio, mask);
	pio_handler_set(p_pio, pio_id, mask, PIO_IT_FALL_EDGE, BUTTON_handler);
	    
	NVIC_EnableIRQ(pio_id);
	NVIC_SetPriority(pio_id, 1);
}

static void PRESENCE_init(void) {
	pmc_enable_periph_clk(PRESENCE_PIO_ID);
	pio_set_input(PRESENCE_PIO, PRESENCE_PIN_MASK, PIO_DEBOUNCE);
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

// void RTC_init(uint32_t year, uint32_t month, uint32_t day, uint32_t week,
// 			  uint32_t hour, uint32_t minute, uint32_t second) {
// 	/* Configura o PMC */
// 	pmc_enable_periph_clk(ID_RTC);
// 	
// 	/* Default RTC configuration, 24-hour mode */
// 	rtc_set_hour_mode(RTC, 0);
// 
// 	/* Configura data e hora manualmente */
// 	rtc_set_date(RTC, year, month, day, week);
// 	rtc_set_time(RTC, hour, minute, second);
// 
// 	/* Configure RTC interrupts */
// 	NVIC_DisableIRQ(RTC_IRQn);
// 	NVIC_ClearPendingIRQ(RTC_IRQn);
// 	NVIC_SetPriority(RTC_IRQn, 0);
// 	NVIC_EnableIRQ(RTC_IRQn);
// 	
// 	rtc_enable_interrupt(RTC, RTC_IER_ALREN); 
// }

static void CONSOLE_init(void) {
	
	const usart_serial_options_t uart_serial_options = {
		.baudrate   = CONF_UART_BAUDRATE,
		.charlength = CONF_UART_CHAR_LENGTH,
		.paritytype = CONF_UART_PARITY,
		.stopbits   = CONF_UART_STOP_BITS,
	};

	/* Configure console UART. */
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	stdio_serial_init(CONF_UART, &uart_serial_options);
}

// Handlers
void TC1_Handler(void) {
	volatile uint32_t ul_dummy;
	uint32_t found_presence, reed_is_closed;
	
	ul_dummy = tc_get_status(TC0, 1);
	UNUSED(ul_dummy);
	
	found_presence = pio_get(PRESENCE_PIO, PIO_INPUT, PRESENCE_PIN_MASK);
	
	if(found_presence)
		has_presence = 1;
	
	if(has_presence) {
		pio_clear(LED_PIO, LED_PIN_MASK);
		presence_to_buzzer_interval++;
	}
	
	if(has_presence) {
		for(int i = 0; i < PASSWORD_LENGTH; i++) {
			if (correct_password[i] != input_password[i])
				correct = 0;
			else
				correct = 1;
		}
	}

	printf("%d\n", presence_to_buzzer_interval);
	
	if(correct) {
		presence_to_buzzer_interval = 0;
		has_presence = 0;
		pio_clear(BUZZER_PIO, BUZZER_PIN_MASK);
	}
	
	if(presence_to_buzzer_interval >= ALARM_DELAY_MAX && !correct) {
		pio_set(BUZZER_PIO, BUZZER_PIN_MASK);
	}
	
}

void BUTTON_handler() {
	uint32_t pioIntStatus;
	pioIntStatus =  pio_get_interrupt_status(BUTTON1_PIO) | pio_get_interrupt_status(BUTTON2_PIO) | pio_get_interrupt_status(BUTTON3_PIO);
	
	uint8_t button1_pressed = !pio_get(BUTTON1_PIO, PIO_INPUT, BUTTON1_PIN_MASK);
	uint8_t button2_pressed = !pio_get(BUTTON2_PIO, PIO_INPUT, BUTTON2_PIN_MASK);
	uint8_t button3_pressed = !pio_get(BUTTON3_PIO, PIO_INPUT, BUTTON3_PIN_MASK);
	
	if (button1_pressed && input_password_len < 5) {
		input_password[curr_index] = 1;
		curr_index++;
		input_password_len++;
		printf("BUTTON 1\n");
	}
	else if (button2_pressed && input_password_len < 5) {
		input_password[curr_index] = 2;
		curr_index++;
		input_password_len++;
		printf("BUTTON 2\n");
	}
	else if (button3_pressed && input_password_len < 5) {
		input_password[curr_index] = 3;
		curr_index++;
		input_password_len++;
		printf("BUTTON 3\n");
	}
	
	if (input_password_len == 5) {
		input_password_len = 0;
		curr_index = 0;
	}
}

void RTC_Handler(void) {
	uint32_t ul_status = rtc_get_status(RTC);
	
	rtc_interruption_count++;
	
	printf("RTC INTERRUPTION %d\n", rtc_interruption_count);
	
	if (can_buzz)
		pio_set(BUZZER_PIO, BUZZER_PIN_MASK);

	if ((ul_status & RTC_SR_SEC) == RTC_SR_SEC)
		rtc_clear_status(RTC, RTC_SCCR_SECCLR);
	else {
		if ((ul_status & RTC_SR_ALARM) == RTC_SR_ALARM) {
			rtc_clear_status(RTC, RTC_SCCR_ALRCLR);
			
			/* Update datetime */
			rtc_set_time(RTC, RTC_HOUR, RTC_MINUTE, RTC_SECOND);
			
			/* Set new alarm */
			rtc_set_time_alarm(RTC, 0, hour, 0, minute, 1, RTC_SECOND + 10);
		}
	}
}

// Helpers
static void pin_toggle(Pio *pio, uint32_t mask){
	if(pio_get_output_data_status(pio, mask))
		pio_clear(pio, mask);
	else
	    pio_set(pio,mask);
}

static void printv(uint32_t *v, uint32_t n) {
	for (uint8_t i = 0; i < n; i++) {
		if (i == 0) {
			printf("[%d, ", v[i]);
		}
		else if (i == n - 1) {
			printf("%d]\n", v[i]);
		}
		else {
			printf("%d, ", v[i]);
		}
	}
}

// Firmware routine
int main(void) {
	
	sysclk_init();
	board_init();
	
	CONSOLE_init();
	
	TC1_init(TC_FREQUENCY);
	//RTC_init(RTC_YEAR, RTC_MONTH, RTC_DAY, RTC_WEEK, RTC_HOUR, RTC_MINUTE, RTC_SECOND);
	LED_init(OFF);
	BUTTON_init(BUTTON1_PIO_ID, BUTTON1_PIO, BUTTON1_PIN_MASK);
	BUTTON_init(BUTTON2_PIO_ID, BUTTON2_PIO, BUTTON2_PIN_MASK);
	BUTTON_init(BUTTON3_PIO_ID, BUTTON3_PIO, BUTTON3_PIN_MASK);
	BUZZER_init();
	PRESENCE_init();
	
	//rtc_set_time_alarm(RTC, 1, RTC_HOUR, 1, RTC_MINUTE, 1, RTC_SECOND + 5);
	
	while(1) {}
}

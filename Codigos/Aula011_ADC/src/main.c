/************************************************************************/
/* Includes                                                             */
/************************************************************************/
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "asf.h"

/************************************************************************/
/* Defines                                                              */
/************************************************************************/

#define TC_CHANNEL 0

#define RTC_YEAR        2016
#define RTC_MONTH       8
#define RTC_DAY         16
#define RTC_WEEK        0
#define RTC_HOUR        0
#define RTC_MINUTE      0
#define RTC_SECOND      0

/** Header printf */
#define STRING_EOL    "\r"
#define STRING_HEADER "-- AFEC Temperature Sensor Example --\r\n" \
"-- "BOARD_NAME" --\r\n" \
"-- Compiled: "__DATE__" "__TIME__" --"STRING_EOL

/** Reference voltage for AFEC,in mv. */
#define VOLT_REF        (3300)

/** The maximal digital value */
/** 2^12 - 1                  */
#define MAX_DIGITAL     (4095UL)

/** The conversion data is done flag */
volatile bool is_conversion_done = false;

/** The conversion data value */
volatile uint32_t g_ul_value = 0;

/* Canal do sensor de temperatura */
#define AFEC_CHANNEL_TEMP_SENSOR 11

/************************************************************************/
/* GLOBALS                                                              */
/************************************************************************/


/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/
void TC_init(Tc *p_tc, uint32_t ul_id, uint32_t ul_freq);

/************************************************************************/
/* Funcoes                                                              */
/************************************************************************/
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

void TC_init(Tc *p_tc, uint32_t ul_id, uint32_t ul_freq) {
  uint32_t ul_div;
  uint32_t ul_tcclks;
  uint32_t ul_sysclk = sysclk_get_cpu_hz();

  /* Configura o PMC */
  pmc_enable_periph_clk(ul_id);

  tc_find_mck_divisor(ul_freq, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
  tc_init(p_tc, TC_CHANNEL, ul_tcclks | TC_CMR_CPCTRG);
  tc_write_rc(p_tc, TC_CHANNEL, (ul_sysclk / ul_div) / ul_freq);

  /* Configura e ativa interrupção no TC canal 0 */
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

}

/**
 * converte valor lido do ADC para temperatura em graus celsius
 * input : ADC reg value
 * output: Temperature in celsius
 */
static int32_t convert_adc_to_temp(int32_t ADC_value) {

  int32_t ul_vol;
  int32_t ul_temp;

	ul_vol = ADC_value * VOLT_REF / MAX_DIGITAL;

  /*
   * According to datasheet, The output voltage VT = 0.72V at 27C
   * and the temperature slope dVT/dT = 2.33 mV/C
   */
  ul_temp = (ul_vol - 720)  * 100 / 233 + 27;
  return(ul_temp);
}

/************************************************************************/
/* Call backs / Handler                                                 */
/************************************************************************/

/**
 * \brief AFEC interrupt callback function.
 */
static void AFEC_Temp_callback(void) {
	g_ul_value = afec_channel_get_value(AFEC0, AFEC_CHANNEL_TEMP_SENSOR);
	is_conversion_done = true;
}

void TC0_Handler(void) {
  volatile uint32_t ul_dummy;
  ul_dummy = tc_get_status(TC0, TC_CHANNEL);
  UNUSED(ul_dummy);

  afec_start_software_conversion(AFEC0);
}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/
/**
 * \brief Application entry point.
 *
 * \return Unused (ANSI-C compatibility).
 */
int main(void) {

	/* Initialize the SAM system. */
	sysclk_init();
  ioport_init();
  board_init();

  /* inicializa delay */
  delay_init(sysclk_get_cpu_hz());

  /* inicializa console (printf) */
	CONSOLE_init();

  /* Init TC */
  TC_init(TC0, ID_TC0, 1);

  /* Init RTC */
  RTC_init();

  /*************************************
   * Ativa e configura AFEC
   *************************************/

  /* Ativa AFEC - 0 */
	afec_enable(AFEC0);

  /* struct de configuracao do AFEC */
	struct afec_config afec_cfg;

  /* Carrega parametros padrao */
	afec_get_config_defaults(&afec_cfg);

  /* Configura AFEC */
	afec_init(AFEC0, &afec_cfg);

  /* Configura trigger por software */
  afec_set_trigger(AFEC0, AFEC_TRIG_SW);

  /* configura call back */
 	afec_set_callback(AFEC0, AFEC_INTERRUPT_EOC_11,	AFEC_Temp_callback, 1);

  /*** Configuracao específica do canal AFEC ***/
  struct afec_ch_config afec_ch_cfg;
  afec_ch_get_config_defaults(&afec_ch_cfg);
  afec_ch_cfg.gain = AFEC_GAINVALUE_0;
  afec_ch_set_config(AFEC0, AFEC_CHANNEL_TEMP_SENSOR, &afec_ch_cfg);

  /*
   * Calibracao:
	 * Because the internal ADC offset is 0x200, it should cancel it and shift
	 * down to 0.
	 */
	afec_channel_set_analog_offset(AFEC0, AFEC_CHANNEL_TEMP_SENSOR, 0x200);

  /***  Configura sensor de temperatura ***/
	struct afec_temp_sensor_config afec_temp_sensor_cfg;

	afec_temp_sensor_get_config_defaults(&afec_temp_sensor_cfg);
	afec_temp_sensor_set_config(AFEC0, &afec_temp_sensor_cfg);

  /* Selecina canal e inicializa conversão */
	afec_channel_enable(AFEC0, AFEC_CHANNEL_TEMP_SENSOR);

  afec_start_software_conversion(AFEC0);

  uint32_t hour, minute, second;
  uint32_t day, month, year;

  while (1) {
		if(is_conversion_done == true) {
			is_conversion_done = false;

      rtc_get_date(RTC, &year, &month, &day, NULL);
      rtc_get_time(RTC, &hour, &minute, &second);

      printf("%02d/%02d/%04d - %02d:%02d:%02d %d °C\n",
        day, month, year, hour, minute, second, convert_adc_to_temp(g_ul_value));
		}
	}
}

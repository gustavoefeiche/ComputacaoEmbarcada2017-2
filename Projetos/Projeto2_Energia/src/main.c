#include "asf.h"
#include "main.h"

// PROTOTYPES
static void LED_init(const uint32_t ul_default_level);
static void TC1_init(uint32_t ul_freq);
static void WATCHDOG_init(uint8_t b_enable);
static void pin_toggle(const Pio *p_pio, const uint32_t ul_mask);

// HANDLERS
void TC1_Handler(void) {

  volatile uint32_t ul_dummy;
	ul_dummy = tc_get_status(TC0, 1);
	UNUSED(ul_dummy);

  if(g_led_blink)
    pin_toggle(LED_PIO, LED_PIN_MASK);

}

void USART1_Handler(void) {
  uint32_t ret = usart_get_status(USART_COM);
  uint8_t c = NULL;

  if(ret & US_IER_RXRDY) {
    usart_serial_getchar(USART_COM, &c);
    if(c != '\n') {
      g_bufferRX[g_count++] = c;
    }
    else {
      g_bufferRX[g_count] = 0x00;
      g_usart_transmission_done = 1;
      g_count = 0;
    }
  }
}

// INITIALIZATION
static void LED_init(const uint32_t ul_default_level) {

  pmc_enable_periph_clk(LED_PIO_ID);
  pio_set_output(LED_PIO, LED_PIN_MASK, !ul_default_level, 0, 0);

};

static void TC1_init(uint32_t ul_freq) {

  uint32_t ul_div;
  uint32_t ul_tcclks;
  uint32_t ul_sysclk = sysclk_get_cpu_hz();

  pmc_enable_periph_clk(ID_TC1);

  tc_find_mck_divisor(ul_freq, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
  tc_init(TC0, TC0_CHANNEL, ul_tcclks | TC0_MODE);

  // Enable TC interrupts
  tc_write_ra(TC0, TC0_CHANNEL, 1525); // 1525 = (ul_sysclk / ul_div) / 2
  tc_write_rc(TC0, TC0_CHANNEL, 1541); // 1541 = (ul_sysclk / ul_div) / 2

  NVIC_EnableIRQ((IRQn_Type) ID_TC1);
  tc_enable_interrupt(TC0, TC0_CHANNEL, TC0_INTERRUPT_SOURCE);

  tc_start(TC0, TC0_CHANNEL);

}

static void WATCHDOG_init(uint8_t b_enable) {

  if(!b_enable)
    wdt_disable(WDT);

}

// FUNCTIONS
void pin_toggle(const Pio *p_pio, const uint32_t ul_mask) {

  if(pio_get_output_data_status(p_pio, ul_mask))
    pio_clear(p_pio, ul_mask);
  else
    pio_set(p_pio, ul_mask);

}

uint32_t usart_puts(uint8_t *pstring) {
  uint32_t i = 0 ;

  while(*(pstring + i)){
    usart_serial_putchar(USART_COM, *(pstring+i++));
    while(!uart_is_tx_empty(USART_COM)){};
  }
  return(i);
}

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
    .stop_bits    = US_MR_NBSTOP_1_BIT,
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

// MAIN LOOP
int main(void) {

	sysclk_init();
  board_init();

  WATCHDOG_init(DISABLE);
  LED_init(ON);
  TC1_init(2);
  USART1_init();

	while(MAINLOOP) {
    if(g_usart_transmission_done) {
      if(g_bufferRX[0] == 't') {
        g_led_blink = !g_led_blink;

        if(g_led_blink)
          usart_puts("LED BLINK ON\n");
        else
          usart_puts("LED BLINK OFF\n");
      }
      g_usart_transmission_done = 0;
    }

    pmc_sleep(SLEEPMGR_SLEEP_WFI);
	}

  return 0;

}

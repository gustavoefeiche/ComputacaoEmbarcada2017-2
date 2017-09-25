#include "asf.h"

// PERIPHERALS
#define LED_PIO_ID ID_PIOC
#define LED_PIO PIOC
#define LED_PIN 8
#define LED_PIN_MASK (1 << LED_PIN)

#define BUT_PIO_ID ID_PIOA
#define BUT_PIO PIOA
#define BUT_PIN 11
#define BUT_PIN_MASK (1 << BUT_PIN)

#define USART_COM USART1
#define USART_COM_ID ID_USART1

// GLOBALS
uint8_t bufferRX[100];
uint8_t bufferTX[100];
uint32_t count = 0;
volatile uint32_t usart_transmission_done = 0;

// PROTOTYPES
void BUT_init(void);
void LED_init(int state);
void pin_toggle(Pio *pio, uint32_t mask);
uint32_t usart_puts(uint8_t *pstring);
uint32_t usart_gets(uint8_t *pstring);

// HANDLERS
static void Button1_Handler(uint32_t id, uint32_t mask) {
  pin_toggle(PIOD, (1<<28));
  pin_toggle(LED_PIO, LED_PIN_MASK);
}

void USART1_Handler(void){
	uint32_t ret = usart_get_status(USART_COM);
	uint8_t c = NULL;
	
	// If data arrived
	if(ret & US_IER_RXRDY) {
		// Get data
		usart_serial_getchar(USART_COM, &c);
		// If string is not finished
		if(c != '\n') {
			bufferRX[count] = c;
			count++;
		} else {
			usart_transmission_done = 1;
			count = 0;
		}
  }
}


/************************************************************************/
/* Funcoes                                                              */
/************************************************************************/

/** 
 *  Toggle pin controlado pelo PIO (out)
 */
void pin_toggle(Pio *pio, uint32_t mask){
   if(pio_get_output_data_status(pio, mask))
    pio_clear(pio, mask);
   else
    pio_set(pio,mask);
}

/**
 * @Brief Inicializa o pino do BUT
 */
void BUT_init(void){
    /* config. pino botao em modo de entrada */
    pmc_enable_periph_clk(BUT_PIO_ID);
    pio_set_input(BUT_PIO, BUT_PIN_MASK, PIO_PULLUP | PIO_DEBOUNCE);
    
    /* config. interrupcao em borda de descida no botao do kit */
    /* indica funcao (but_Handler) a ser chamada quando houver uma interrupção */
    pio_enable_interrupt(BUT_PIO, BUT_PIN_MASK);
    pio_handler_set(BUT_PIO, BUT_PIO_ID, BUT_PIN_MASK, PIO_IT_FALL_EDGE, Button1_Handler);
    
    /* habilita interrupçcão do PIO que controla o botao */
    /* e configura sua prioridade                        */
    NVIC_EnableIRQ(BUT_PIO_ID);
    NVIC_SetPriority(BUT_PIO_ID, 1);
};

/**
 * @Brief Inicializa o pino do LED
 */
void LED_init(int estado){
    pmc_enable_periph_clk(LED_PIO_ID);
    pio_set_output(LED_PIO, LED_PIN_MASK, estado, 0, 0 );
};

/**
 * \brief Configure UART console.
 */
static void USART1_init(void) {
	// Configure USART1 pins
	sysclk_enable_peripheral_clock(ID_PIOB);
	sysclk_enable_peripheral_clock(ID_PIOA);
	pio_set_peripheral(PIOB, PIO_PERIPH_D, PIO_PB4);  // RX
	pio_set_peripheral(PIOA, PIO_PERIPH_A, PIO_PA21); // TX
	MATRIX->CCFG_SYSIO |= CCFG_SYSIO_SYSIO4;
	
	// Configure USART options
	const sam_usart_opt_t usart_settings = {
		.baudrate     = 115200,
		.char_length  = US_MR_CHRL_8_BIT,
		.parity_type  = US_MR_PAR_NO,
		.stop_bits    = US_MR_NBSTOP_1_BIT,
		.channel_mode = US_MR_CHMODE_NORMAL
	};

	// Enable USART_COM_ID (defined above) peripheral
	sysclk_enable_peripheral_clock(USART_COM_ID);
  
	// Configure USART to operate under RS232
	usart_init_rs232(USART_COM, &usart_settings, sysclk_get_peripheral_hz());
  
	// Enable trasmitter and receiver
	usart_enable_tx(USART_COM);
	usart_enable_rx(USART_COM);
 
	// Configure USART interrupt when data arrives
	usart_enable_interrupt(USART_COM, US_IER_RXRDY);
	NVIC_EnableIRQ(USART_COM_ID);
 }

/**
 * Envia para o UART uma string
 * envia todos os dados do vetor até
 * encontrar o \NULL (0x00)
 *
 * Retorna a quantidade de char escritos
 */
uint32_t usart_puts(uint8_t *pstring) {
	uint32_t count = 0;
	while(*pstring) {
		if(usart_is_tx_empty(USART_COM)) {
			usart_serial_putchar(USART_COM, *pstring);
			pstring++;	
			count++;
		}
	}	
	return count;
}

/*
 * Usart get string
 * monta um buffer com valores recebidos da USART até 
 * encontrar o char '\n'
 *
 * Retorna a quantidade de char lidos
 */
uint32_t usart_gets(uint8_t *pstring) {
	uint32_t index = 0;
	char c = 0;
	
	while(c != '\n') {
		usart_serial_getchar(USART_COM, &c);
		pstring[index] = c;
		index++;	
	}
	
	pstring[index - 1] = '\0';
	
	return index;  
}

/************************************************************************/
/* Main Code	                                                        */
/************************************************************************/
int main(void){

  board_init();
  sysclk_init();
  
  WDT->WDT_MR = WDT_MR_WDDIS;
  LED_init(1);
  BUT_init();  
  USART1_init();
  delay_init(sysclk_get_cpu_hz());
        
	while (1) {
		if(usart_transmission_done) {
			usart_puts(bufferRX);
			usart_transmission_done = 0;
		}
	}
}

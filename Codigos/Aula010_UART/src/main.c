#include "asf.h"

/**
 * LEDs
 */
#define LED_PIO_ID		  ID_PIOC
#define LED_PIO         PIOC
#define LED_PIN		      8
#define LED_PIN_MASK    (1<<LED_PIN)

/**
 * Botão
 */
#define BUT_PIO_ID            ID_PIOA
#define BUT_PIO               PIOA
#define BUT_PIN		            11
#define BUT_PIN_MASK          (1 << BUT_PIN)
#define BUT_DEBOUNCING_VALUE  79

/** 
 *  USART
 */
#define USART_COM     USART1
#define USART_COM_ID  ID_USART1

/************************************************************************/
/* VAR globais                                                          */
/************************************************************************/

 /* buffer para recebimento de dados */
 uint8_t bufferRX[100];
 
 /* buffer para transmissão de dados */
 uint8_t bufferTX[100];

/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/

void BUT_init(void);
void LED_init(int estado);
void pin_toggle(Pio *pio, uint32_t mask);
uint32_t usart_puts(uint8_t *pstring);
uint32_t usart_gets(uint8_t *pstring);

/************************************************************************/
/* Handlers                                                             */
/************************************************************************/

/**
 *  Handle Interrupcao botao 1
 */
static void Button1_Handler(uint32_t id, uint32_t mask)
{
  pin_toggle(PIOD, (1<<28));
  pin_toggle(LED_PIO, LED_PIN_MASK);
}

void USART1_Handler(void){
  uint32_t ret = usart_get_status(USART_COM);
  uint8_t  c;
  
  // Verifica por qual motivo entrou na interrupçcao
  if(ret & US_IER_RXRDY){                     // Dado disponível para leitura
    usart_serial_getchar(USART_COM, &c);
    usart_puts(bufferTX);
  } else if(ret & US_IER_TXRDY){              // Transmissão finalizada
    
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
static void USART1_init(void){
  
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
    .stop_bits    = US_MR_NBSTOP_1_BIT    ,
    .channel_mode = US_MR_CHMODE_NORMAL
  };

  /* Ativa Clock periferico USART0 */
  sysclk_enable_peripheral_clock(USART_COM_ID);
  
  /* Configura USART para operar em modo RS232 */
  usart_init_rs232(USART_COM, &usart_settings, sysclk_get_peripheral_hz());
  
  /* Enable the receiver and transmitter. */
	usart_enable_tx(USART_COM);
	usart_enable_rx(USART_COM);
 }

/**
 * Envia para o UART uma string
 * envia todos os dados do vetor até
 * encontrar o \NULL (0x00)
 *
 * Retorna a quantidade de char escritos
 */
uint32_t usart_puts(uint8_t *pstring){
     
  return 0;
}

/*
 * Usart get string
 * monta um buffer com valores recebidos da USART até 
 * encontrar o char '\n'
 *
 * Retorna a quantidade de char lidos
 */
uint32_t usart_gets(uint8_t *pstring){

  return 0;  
}

/************************************************************************/
/* Main Code	                                                        */
/************************************************************************/
int main(void){


  /* Initialize the SAM system */
  sysclk_init();
   
  /* Disable the watchdog */
  WDT->WDT_MR = WDT_MR_WDDIS;

  /* Configura Leds */
  LED_init(1);
  
  /* Configura os botões */
  BUT_init();  
  
  /* Inicializa com serial com PC*/
  USART1_init();
 
  /* Inicializa funcao de delay */
  delay_init( sysclk_get_cpu_hz());
        
	while (1) {
    sprintf(bufferTX, "%s \n", "Ola Voce");
    //usart_puts(bufferTX);
   // usart_gets(bufferRX);
    delay_s(1);
	}
}

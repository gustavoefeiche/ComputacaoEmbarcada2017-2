#ifndef __MAIN_H__
#define __MAIN_H__

#define MAINLOOP 1
#define ON 1
#define OFF 0
#define ENABLE 1
#define DISABLE 0

#define LED_PIO_ID		ID_PIOC
#define LED_PIO       PIOC
#define LED_PIN		    13
#define LED_PIN_MASK  (1 << LED_PIN)

#define USART_COM     USART1
#define USART_COM_ID  ID_USART1

#define TC0_CHANNEL 1
#define TC0_MODE TC_CMR_WAVE
#define TC0_INTERRUPT_SOURCE (TC_IER_CPCS | TC_IER_CPAS)

// FLAGS
volatile uint32_t g_led_blink = 1;
volatile uint32_t g_usart_transmission_done = 0;

// GLOBALS
uint32_t g_count = 0;
uint8_t g_bufferRX[3];
uint8_t g_bufferTX[3];

#endif
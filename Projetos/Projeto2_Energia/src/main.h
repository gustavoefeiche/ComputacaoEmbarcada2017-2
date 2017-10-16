#ifndef __MAIN_H__
#define __MAIN_H__

#define YEAR        2017
#define MOUNTH      3
#define DAY         27
#define WEEK        13
#define HOUR        9
#define MINUTE      5
#define SECOND      0

#define LED_PIO_ID		ID_PIOC
#define LED_PIO       PIOC
#define LED_PIN		    13
#define LED_PIN_MASK  (1<<LED_PIN)

#define BUT_PIO_ID      ID_PIOA
#define BUT_PIO         PIOA
#define BUT_PIN		    11
#define BUT_PIN_MASK    (1 << BUT_PIN)
#define BUT_DEBOUNCING_VALUE  79

#define USART_COM     USART1
#define USART_COM_ID  ID_USART1

#endif
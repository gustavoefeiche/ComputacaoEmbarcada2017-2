#include "asf.h"
#include "image.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ioport.h"
#include "logo.h"

/** ASCII char definition for backspace. */
#define ASCII_BS    0x7F
/** ASCII char definition for carriage return. */
#define ASCII_CR    13

#define STRING_EOL    "\r\n"
#define STRING_HEADER "-- SAME70 LCD DEMO --"STRING_EOL	\
	"-- "BOARD_NAME " --"STRING_EOL	\
	"-- Compiled: "__DATE__ " "__TIME__ " --"STRING_EOL

struct ili9488_opt_t g_ili9488_display_opt;

/**
}
/**
 * \brief Configure UART console.
 */
static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate =		CONF_UART_BAUDRATE,
		.charlength =	CONF_UART_CHAR_LENGTH,
		.paritytype =	CONF_UART_PARITY,
		.stopbits =		CONF_UART_STOP_BITS,
	};

	/* Configure UART console. */
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	stdio_serial_init(CONF_UART, &uart_serial_options);
}

/**
 * \brief Main application function.
 *
 * Initialize system, UART console, network then start weather client.
 *
 * \return Program return value.
 */
int main(void)
{
	/* Initialize the board. */
	sysclk_init();
	board_init();
	ioport_init();

	/* Initialize the UART console. */
	configure_console();
	printf(STRING_HEADER);

	/* Set direction and pullup on the given button IOPORT */
	ioport_set_pin_dir(GPIO_PUSH_BUTTON_1, IOPORT_DIR_INPUT);
	ioport_set_pin_mode(GPIO_PUSH_BUTTON_1, IOPORT_MODE_PULLUP);

	/* Initialize display parameter */
	g_ili9488_display_opt.ul_width = ILI9488_LCD_WIDTH;
	g_ili9488_display_opt.ul_height = ILI9488_LCD_HEIGHT;
	g_ili9488_display_opt.foreground_color = COLOR_CONVERT(COLOR_WHITE);
	g_ili9488_display_opt.background_color = COLOR_CONVERT(COLOR_WHITE);

	/* Initialize LCD */
	ili9488_init(&g_ili9488_display_opt);
	ili9488_draw_filled_rectangle(0, 0, ILI9488_LCD_WIDTH-1, ILI9488_LCD_HEIGHT-1);
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_TOMATO));
	ili9488_draw_filled_rectangle(0, 0, ILI9488_LCD_WIDTH-1, 120-1);
	ili9488_draw_filled_rectangle(0, 360, ILI9488_LCD_WIDTH-1, 480-1);
	ili9488_draw_pixmap(0, 50, 319, 129, logoImage);

    /* Escreve na tela */
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_WHITE));
	ili9488_draw_filled_rectangle(0, 300, ILI9488_LCD_WIDTH-1, 315);
	ili9488_set_foreground_color(COLOR_CONVERT(COLOR_BLACK));
	ili9488_draw_string(10, 300, (uint8_t *)"Computacao Embarcada");

	while (1) {

	}
	return 0;
}

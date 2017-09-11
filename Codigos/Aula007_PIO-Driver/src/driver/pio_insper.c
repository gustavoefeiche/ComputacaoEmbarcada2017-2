#include "pio_insper.h"

/* Configure pin as output
   Parameters:
     p_pio - pointer to PIO instance
     ul_mask - bit mask (1 << n) with 0 <= n <= 31
	 ul_default_level - default output level (1 or 0)
	 ul_pull_up_enable - indicates if pull up should be enabled or not (1 or 0)
*/
void _pio_set_output(Pio *p_pio, const uint32_t ul_mask, const uint32_t ul_default_level, const uint32_t ul_pull_up_enable) {
	p_pio->PIO_PER = ul_mask;
	p_pio->PIO_OER = ul_mask;
		
	if (ul_default_level)
		p_pio->PIO_SODR = ul_mask; // Clear Output Data Register
	else
		p_pio->PIO_CODR = ul_mask; // Set Output Data Register
		
	if (ul_pull_up_enable)
		p_pio->PIO_PUER = ul_mask; // Pull Up Enable Register
	else
		p_pio->PIO_PUDR = ul_mask; // Pull Up Disable Register
}

/* Configure pin as output
   Parameters:
     p_pio - pointer to PIO instance
     ul_mask - bit mask (1 << n) with 0 <= n <= 31
	 ul_attribute - concatenation of attributes defined in pio.h
*/
void _pio_set_input(Pio *p_pio, const uint32_t ul_mask, const uint32_t ul_attribute) {
	p_pio->PIO_PER = ul_mask; // PIO Enable Register
	p_pio->PIO_ODR = ul_mask; // Output Disable Register
	
	if(ul_attribute & (PIO_DEBOUNCE | PIO_DEGLITCH)) {
		p_pio->PIO_IFER = ul_mask; // Glitch Input Filter Enable Register
		p_pio->PIO_IFSCER = ul_mask; // Input Filter Slow Clock Enable Register
	}
	else
		p_pio -> PIO_IFDR = ul_mask; // Glitch Input Filter Disable Register
	
	if(ul_attribute & PIO_PULLUP)
		p_pio->PIO_PUER = ul_mask; // Pull Up Enable Register
}

/* Configure pin pull up
   Parameters:
     p_pio - pointer to PIO instance
     ul_mask - bit mask (1 << n) with 0 <= n <= 31
	 ul_pull_up_enable - enable or disable (1 or 0)
*/
void _pio_pull_up(Pio *p_pio, const uint32_t ul_mask, const uint32_t ul_pull_up_enable) {
	if (ul_pull_up_enable)
		p_pio->PIO_PUER = ul_mask; // Pull Up Enable Register
	else
		p_pio->PIO_PUDR = ul_mask; // Pull Up Disable Register
}

/* Configure pin pull down
   Parameters:
     p_pio - pointer to PIO instance
     ul_mask - bit mask (1 << n) with 0 <= n <= 31
	 ul_pull_down_enable - enable or disable (1 or 0)
*/
void _pio_pull_down(Pio *p_pio, const uint32_t ul_mask, const uint32_t ul_pull_down_enable) {
	if (ul_pull_down_enable)
		p_pio->PIO_PPDER = ul_mask; // Pull Down Enable Register
	else
		p_pio->PIO_PPDDR = ul_mask; // // Pull Down Enable Register
}

/* Configure pin level as high
   Parameters:
     p_pio - pointer to PIO instance
     ul_mask - bit mask (1 << n) with 0 <= n <= 31
*/
void _pio_set(Pio *p_pio, const uint32_t ul_mask) {
	p_pio->PIO_SODR = ul_mask; // Set Output Data Register
}

/* Configure pin level as low
   Parameters:
     p_pio - pointer to PIO instance
     ul_mask - bit mask (1 << n) with 0 <= n <= 31
*/
void _pio_clear(Pio *p_pio, const uint32_t ul_mask) {
	p_pio->PIO_CODR = ul_mask; // Clear Output Data Register
}

/* Check if pins are set to output high or low
   Parameters:
     p_pio - pointer to PIO instance
     ul_mask - bit mask (1 << n) with 0 <= n <= 31
*/
uint32_t _pio_get_output_data_status(const Pio *p_pio, const uint32_t ul_mask) {
	if ((p_pio->PIO_ODSR & ul_mask) == 0)
		return 0;
	else
		return 1;
}
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include <stdio.h>
#include <stdlib.h>


#define PORT_LED GPIOC
#define PIN_LED GPIO8

int main(void)
{
	rcc_clock_setup_in_hsi_out_48mhz();

	/* LED on for boot progress */
	rcc_periph_clock_enable(RCC_GPIOC);
	gpio_mode_setup(PORT_LED, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PIN_LED);
	gpio_set(PORT_LED, PIN_LED);
	//gpio_clear(GPIOC, GPIO7);
	/* Blink the LED (PC8) on the board. */
	while (1) {
		gpio_toggle(PORT_LED, PIN_LED);	/* LED on/off */
		for (int i = 0; i < 1000000; i++) {	/* Wait a bit. */
			__asm__("nop");
		}
	}
}


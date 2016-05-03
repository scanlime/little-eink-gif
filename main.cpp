#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <string.h>
#include "spd2701.h"

SPD2701 display( /* SDA  */  GPIOD, GPIO10,
	             /* SCLK */  GPIOD, GPIO9,
	             /* CS   */  GPIOD, GPIO8,
	             /* DC   */  GPIOE, GPIO15,
	             /* BUSY */  GPIOE, GPIO13 );

static void gpio_setup(void)
{
	rcc_periph_clock_enable(RCC_GPIOD);
	rcc_periph_clock_enable(RCC_GPIOE);
	gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO8 | GPIO9 | GPIO10);
	gpio_set_mode(GPIOE, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO11 | GPIO14 | GPIO15);
	gpio_set_mode(GPIOE, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO13);

	// Enter reset
	gpio_clear(GPIOE, GPIO14);

	// Tie BS1 low, to indicate 4-wire SPI mode.
	gpio_clear(GPIOE, GPIO11);

	// Leave reset
	gpio_set(GPIOE, GPIO14);
}

int main(void)
{
	rcc_clock_setup_in_hse_8mhz_out_72mhz();
	gpio_setup();

	display.clear();
	for (int i = 0; i < display.width; i++)
  		for (int j = 0; j < display.height; j++)
			display.pixel(i, j, i+j);

	display.init();
	display.update();
	display.sleep();

	while (1);

	return 0;
}

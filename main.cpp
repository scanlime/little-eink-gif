#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>

uint32_t temp32;

static void gpio_setup(void)
{
	/* Enable GPIOB clock. */
	rcc_periph_clock_enable(RCC_GPIOB);

	/* Set GPIO6/7 (in GPIO port B) to 'output push-pull' for the LEDs. */
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO6);
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO7);

	/* A0 of DOGM128 */
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO14);
	/* Reset of DOGM128 */
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO10);

	/* DOGM128/SPI2 clock and MOSI and NSS(CS1) */
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO12);
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO13);
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO15);
}

static void spi_setup(void)
{
	/* The DOGM128 display is connected to SPI2, so initialise it. */

	rcc_periph_clock_enable(RCC_SPI2);

	spi_set_unidirectional_mode(SPI2); /* We want to send only. */
	spi_disable_crc(SPI2); /* No CRC for this slave. */
	spi_set_dff_8bit(SPI2); /* 8-bit dataword-length */
	spi_set_full_duplex_mode(SPI2); /* Not receive-only */
	/* We want to handle the CS signal in software. */
	spi_enable_software_slave_management(SPI2);
	spi_set_nss_high(SPI2);
	/* PCLOCK/256 as clock. */
	spi_set_baudrate_prescaler(SPI2, SPI_CR1_BR_FPCLK_DIV_256);
	/* We want to control everything and generate the clock -> master. */
	spi_set_master_mode(SPI2);
	spi_set_clock_polarity_1(SPI2); /* SCK idle state high. */
	/* Bit is taken on the second (rising edge) of SCK. */
	spi_set_clock_phase_1(SPI2);
	spi_enable_ss_output(SPI2);
	spi_enable(SPI2);
}

int main(void)
{
	rcc_clock_setup_in_hse_8mhz_out_72mhz();

	/* 72MHz / 8 => 9000000 counts per second */
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);

	/* 9000000/9000 = 1000 overflows per second - every 1ms one interrupt */
	/* SysTick interrupt every N clock pulses: set reload to N-1 */
	systick_set_reload(8999);

	systick_interrupt_enable();
	systick_counter_enable();

	gpio_setup();
	spi_setup();

	gpio_clear(GPIOB, GPIO7);	/* LED1 on */
	gpio_set(GPIOB, GPIO6);		/* LED2 off */

	gpio_set(GPIOB, GPIO7); /* LED1 off */
	while (1); /* Halt. */

	return 0;
}


void sys_tick_handler(void)
{
	temp32++;

	/* We call this handler every 1ms so 1000ms = 1s on/off. */
	if (temp32 == 1000) {
		gpio_toggle(GPIOB, GPIO6); /* LED2 on/off */
		temp32 = 0;
	}
}

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <string.h>
#include "spd2701.h"
#include "puff.h"

volatile uint32_t millis = 0;

extern const uint32_t _binary_images_bin_start[];
extern const uint32_t _binary_images_bin_end[];

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

static void millisec_timer_setup(void)
{
    // 72MHz / 8 => 9000000 counts per second
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);

    // 9000000/9000 = 1000 overflows per second - every 1ms one interrupt
    // SysTick interrupt every N clock pulses: set reload to N-1
    systick_set_reload(8999);

    systick_interrupt_enable();
    systick_counter_enable();
}

void sys_tick_handler(void)
{
    millis++;
}

static void delay_millisec(unsigned duration)
{
    uint32_t deadline = millis + duration;
    while (int32_t(millis - deadline) < 0);
}

const uint32_t* show_compressed_frame(const uint32_t *ptr)
{
    unsigned duration = ptr[0]; ptr++;
    unsigned compressed_size = ptr[0]; ptr++;

    unsigned long destlen = sizeof display.buffer;
    unsigned long srclen = compressed_size;

    display.clear();
    puff(display.buffer, &destlen, (unsigned char*) ptr, &srclen);
    ptr += compressed_size >> 2;

    display.update();
    delay_millisec(duration);

    return ptr;
}

int main(void)
{
    rcc_clock_setup_in_hse_8mhz_out_72mhz();
    gpio_setup();
    millisec_timer_setup();
    display.init();

    while (1) {
        const uint32_t *ptr = _binary_images_bin_start;
        while (ptr < _binary_images_bin_end) {
            ptr = show_compressed_frame(ptr);
        }
    }

    return 0;
}

/*
 * Driver for the SPD2701 EPD (electrophoretic display / e-ink) controller
 *
 * Copyright (c) 2016 Micah Elizabeth Scott
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <stdint.h>


class SPD2701 {
public:
    // Using four-wire SPI mode (BS1=L)
    // Bit-banging SPI for compatibility with the demo board.

    SPD2701(uint32_t SDA_port, uint32_t SDA_out_pin,        // SPI Data Out
            uint32_t SCLK_port, uint32_t SCLK_out_pin,      // SPI Clock Out
            uint32_t CS_port, uint32_t CS_out_pin,          // Chip Select
            uint32_t DC_port, uint32_t DC_out_pin,          // Data / Command
            uint32_t BUSY_port, uint32_t BUSY_in_pin)       // Busy Input

      : m_SDA_port(SDA_port),   m_SDA_out_pin(SDA_out_pin),
        m_SCLK_port(SCLK_port), m_SCLK_out_pin(SCLK_out_pin),
        m_CS_port(CS_port),     m_CS_out_pin(CS_out_pin),
        m_DC_port(DC_port),     m_DC_out_pin(DC_out_pin),
        m_BUSY_port(BUSY_port), m_BUSY_in_pin(BUSY_in_pin)
    {}

    void init();
    void sleep();
    void update();

    void writeCommand(uint8_t cmd);
    void writeData(uint8_t data);
    void writeData(const uint8_t* data, int count);

    static const int width = 72;
    static const int height = 172;

    uint8_t buffer[width/4 * height];

    void clear();
    void pixel(int x, int y, int color = 0);

private:
    uint32_t m_SDA_port, m_SDA_out_pin;
    uint32_t m_SCLK_port, m_SCLK_out_pin;
    uint32_t m_CS_port, m_CS_out_pin;
    uint32_t m_DC_port, m_DC_out_pin;
    uint32_t m_BUSY_port, m_BUSY_in_pin;
};


inline void SPD2701::init()
{
    // Default bus state
    gpio_clear(m_SDA_port, m_SDA_out_pin);
    gpio_clear(m_SCLK_port, m_SCLK_out_pin);
    gpio_set(m_CS_port, m_CS_out_pin);
    gpio_set(m_DC_port, m_DC_out_pin);

    // Semi-documented initialization commands from the display vendor,
    // seems not to be at all consistent with commands in the data sheet.

    writeCommand(0x10);  // Exit deep sleep mode
    writeData(0x00);

    writeCommand(0x11);  // Data entry mode
    writeData(0x03);

    writeCommand(0x44);  // RAM x address start/end
    writeData(0);
    writeData(width/4 - 1);
    writeCommand(0x45);  // RAM y address start/end
    writeData(0);
    writeData(height - 1);

    writeCommand(0x4E);  // RAM x address count
    writeData(0);
    writeCommand(0x4F);  // RAM y address count
    writeData(0);

    writeCommand(0x21);
    writeData(0x03);
    writeCommand(0xF0);  // Boost controller feedback
    writeData(0x1F);

    writeCommand(0x2C);  // VCOM supply
    writeData(0xA0);

    writeCommand(0x3C);  // board (?)
    writeData(0x63);
    writeCommand(0x22);  // Display update sequence option
    writeData(0xC4);     // Enable sequence: clk -> CP -> LUT -> initial display -> pattern display

    const unsigned char init_data[]={
    0x00,0x00,0x00,0x55,0x00,0x00,0x55,0x55,
    0x00,0x55,0x55,0x55,0x55,0x55,0x55,0x55,
    0x55,0xAA,0x55,0x55,0xAA,0xAA,0xAA,0xAA,
    0x05,0x05,0x05,0x05,0x15,0x15,0x15,0x15,
    0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x34,0x32,0xF1,0x74,0x14,0x00,0x00,0x00,
    0x00,0x00,};

    // Initialize LUT with data provided by the manufacturer
    writeCommand(0x32);
    writeData(init_data, sizeof init_data);
}

inline void SPD2701::update()
{
    writeCommand(0x24);     // Write image to SRAM
    writeData(buffer, sizeof buffer);
    writeCommand(0x20);     // Trigger EPD update sequence
}

inline void SPD2701::sleep()
{
    writeCommand(0x10);
    writeData(0x01);
}

inline void SPD2701::writeCommand(uint8_t cmd)
{
    gpio_clear(m_DC_port, m_DC_out_pin);
    writeData(cmd);
    gpio_set(m_DC_port, m_DC_out_pin);
}

inline void SPD2701::writeData(uint8_t data)
{
    // Wait for display ready
    while (gpio_port_read(m_BUSY_port) & m_BUSY_in_pin);

    gpio_clear(m_CS_port, m_CS_out_pin);
    for (unsigned bit = 0; bit < 8;) {
        if (data & 0x80) {
            gpio_set(m_SDA_port, m_SDA_out_pin);
        } else {
            gpio_clear(m_SDA_port, m_SDA_out_pin);
        }
        bit++;
        gpio_set(m_SCLK_port, m_SCLK_out_pin);
        data <<= 1;
        gpio_clear(m_SCLK_port, m_SCLK_out_pin);
    }
    gpio_set(m_CS_port, m_CS_out_pin);
}

inline void SPD2701::writeData(const uint8_t* data, int count)
{
    while (count > 0) {
        writeData(*data);
        data++;
        count--;
    }
}

inline void SPD2701::clear()
{
    memset(buffer, 0xFF, sizeof buffer);
}

inline void SPD2701::pixel(int x, int y, int color)
{
    if (x >= 0 && x < width && y >= 0 && y < height) {
        int byte = x >> 2;
        int shift = (3 - (x & 3)) << 1;
        const int mask = 3;
        color &= mask;
        uint8_t &ptr = buffer[y * width/4 + byte];
        ptr = (ptr & ~(mask << shift)) | (color << shift);
    }
}

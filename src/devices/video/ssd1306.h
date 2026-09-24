
#ifndef MAME_VIDEO_SSD1306_H
#define MAME_VIDEO_SSD1306_H

#pragma once

#include "screen.h"

// see datasheet, Table 7-1. bits are in order BS2..BS0
typedef enum
{
    SPI_4WIRE       = 0b000,
    SPI_3WIRE       = 0b001,
    I2C             = 0b010,
    PARALLEL_6800   = 0b100,
    PARALLEL_8080   = 0b110,
} ssd1306_interface_mode_t;

typedef enum
{
    HORIZONTAL = 0,
    VERTICAL,
    PAGE,
    INVALID
} ssd1306_addressing_mode_t;


class ssd1306_device :  public device_t,
						public device_video_interface,
						public device_palette_interface
{
public:


    void set_intf_mode(ssd1306_interface_mode_t mode);

    /** 
    * SPI bus mode (use only with ssd1306_interface_mode_t SPI_4WIRE or SPI_3WIRE)
    */
    void spi_cs_w(int state);
    void spi_si_w(int state);
    void spi_sck_w(int state);


    /** 
    * Standard MAME parallel read/write handlers
    */

    void write(offs_t offset, uint8_t data);
    u8   read(offs_t offset);


    /**
     * Set the state of the D/C# pin.
     * 
     * In four-wire SPI mode, if the D/C# pin is high,
     * then next write is data, otherwise, it's a command.
     * The same behavior applies for parallel bus modes.
     * In three-wire SPI mode, D/C# is set within the command itself.
     *  
     * In I2C mode, D/C# changes the I2C slave address.
     * If low, the address is 0x3C (0b0111100).
     * If high, it's 0x3D (0b0111101).
     *
     */
    void dc_w(int dc);

    /**
     * Set /RST pin.
     */
    void rst_w(int rst);

private:
    void raw_write(int dc_line, uint8_t data);
    u8   raw_read(int dc_line);

    void exec_command();
    void exec_command_2x();
    void exec_command_ax();
    void exec_command_dx();

    void update_scan_rate();

    bool m_reset_asserted;

    bool m_scroll_enable;
    bool m_display_awake;
    bool m_inverting_pixels;

    bool m_dc_line;
    bool m_dc_internal_state;


    uint8_t m_spi_shift;
    bool m_spi_cs_asserted;
    bool m_spi_si;
    
    int m_spi_bits_left;

    ssd1306_interface_mode_t  m_pending_interface_mode;
    ssd1306_interface_mode_t  m_current_interface_mode;

    ssd1306_addressing_mode_t m_addressing_mode;

    // display memory; 128x64 bits, divided into 8 pages
    uint8_t m_gddram[ 128 * 8 ];

    uint8_t m_pagemode_current_column_address;

    uint8_t m_pagemode_page_start_address;
    uint8_t m_pagemode_column_start_address;
    uint8_t m_pagemode_column_end_address;

    uint8_t m_hvmode_page_start_address;
    uint8_t m_hvmode_page_end_address;
    uint8_t m_hvmode_column_start_address;
    uint8_t m_hvmode_column_end_address;

    uint8_t m_page_address_pointer;
    uint8_t m_column_address_pointer;

    uint8_t m_vscroll_fixed_rows;
    uint8_t m_vscroll_scroll_rows;

    uint8_t m_command_fifo[0x100];
    uint8_t m_command_pointer;
    uint8_t m_command_bytes_left;

    uint8_t m_command_lengths[0x100];

    uint8_t m_phase_1_period;
    uint8_t m_phase_2_period;

    uint8_t m_clk_div;
    uint8_t m_osc_freq;

};



#endif
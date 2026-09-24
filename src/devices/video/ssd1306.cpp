/****************************************************************************

    Solomon Systech SSD1306 OLED display driver
    "128x64 Dot Matrix OLED/PLED Segment/Common Driver with Controller"

    Datasheet:
    https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf

    Graphics RAM is split into 8 pixel high "Pages" with one byte representing
    one vertical stripe of 8 pixels.

    Frame rate is determined by:

                                 1
        osc_value * ---------------------------
                     div * display_clocks * 64

    where display_clocks is:
        phase_1_period + phase_2_period + BANK0_pulse_width

    The way to set BANK0_pulse_width isn't really described in the datasheet.
    For now, we treat it as a constant 50.

 ****************************************************************************/

#include "emu.h"
#include "ssd1306.h"


#define BANK0_PULSE_WIDTH 50

#define KEEP_LOW_NIBBLE(x)  x & 0x0F
#define KEEP_HIGH_NIBBLE(x) x & 0xF0

#define LOW4_AS_HIGH_NIBBLE(x) (x & 0x0F) << 4
#define LOW4_AS_LOW_NIBBLE(x) (x & 0x0F)

#define SET_LOW_NIBBLE_FROM_LOW4(reg, val)  reg = KEEP_HIGH_NIBBLE(reg) | LOW4_AS_LOW_NIBBLE(val);
#define SET_HIGH_NIBBLE_FROM_LOW4(reg, val) reg = KEEP_LOW_NIBBLE(reg) | LOW4_AS_HIGH_NIBBLE(val);

#define COMPLAIN_INVALID_COMMAND logerror("%s: invalid/unimplemented command %02x\n", m_command_fifo[0]);


void ssd1306_device::device_init()
{
    // init all commandlengths to 0 (=256)
    memset(m_command_lengths, 0, sizeof(m_command_lengths));

    // 

}

void ssd1306_device::set_intf_mode(ssd1306_interface_mode_t mode)
{
    // in the datasheet and on the arduboy, the interface mode pins
    // are supposed to be always tied to VCC or ground.
    // the datasheet doesn't mention how these pins are read,
    // so if someone is insane enough to change interfacing modes,
    // let's assume the interface mode is latched only at reset
    m_pending_interface_mode = mode;
}

void ssd1306_device::device_reset()
{
    m_current_interface_mode = m_pending_interface_mode;


    m_display_awake = false;

    m_inverting_pixels = false;

    m_spi_bits_left = 0;
    m_spi_shift = 0;

    m_pagemode_column_start_address = 0;
    m_pagemode_column_end_address = 7;
    
    m_hvmode_page_start_address = 0x0d;
    m_hvmode_page_end_address   = 0x7d;

    m_vscroll_fixed_rows  = 0;
    m_vscroll_scroll_rows = 64;

    m_clk_div   = 0b0000;
    m_osc_freq  = 0b1000;

    m_phase_1_period = 2;
    m_phase_2_period = 2;

    m_addressing_mode = PAGE;
}


void ssd1306_device::exec_command_2x()
{
    switch(m_command_fifo[0] & 0x0F)
    {
        case 0x0:
            m_addressing_mode = static_cast<ssd1306_addressing_mode_t>(m_command_fifo[1] & 3);
            break;

        case 0x1:
            // h/v addressing mode: set column start/end address
            m_hvmode_column_start_address;
            m_hvmode_column_end_address;
            break;

        case 0x2:
            // h/v addressing mode: set page start/end address
            break;

        case 0x6:
        case 0x7:
            // init right scroll (6) or left scroll (7)
            break;

        case 0x9:
        case 0xA:
            // init vertical + right scroll (9) or left scroll (A)
            break;

        case 0xE:
            // scroll disable
            break;
        
        case 0xF:
            // scroll enable
            
            break;
        default:
            COMPLAIN_INVALID_COMMAND;
            break;
    }
}


void ssd1306_device::exec_command_ax()
{
    switch(m_command_fifo[0] & 0x0F)
    {
        case 0x0:
            // map column 0 to SEG0
            break;

        case 0x1:
            // map column 127 to SEG0
            break;

        case 0x3:
            // vertical scroll
            break;

        case 0x4:
            // display RAM contents
            break;

        case 0x5:
            // blank display (all pixels on???)
            break;

        case 0x6:
            m_inverting_pixels = false;
            break;

        case 0x7:
            m_inverting_pixels = true;
            break;

        case 0x8:
            // mux ratio
            break;

        case 0xE:
            m_display_awake = false;
            break;

        case 0xF:
            m_display_awake = true;
            break;

        default:
            COMPLAIN_INVALID_COMMAND;
            break;
    }
}

void ssd1306_device::exec_command_dx()
{
    switch(m_command_fifo[0] & 0x0F)
    {
        case 0x3:
            // TODO: set display offset
            break;

        case 0x5:
            m_clk_div  = m_command_fifo[1] & 0xf;
            m_osc_freq = m_command_fifo[1] >> 4;
            break;


        case 0x9:
            m_phase_1_period = m_command_fifo[1] & 0xf;
            m_phase_2_period = m_command_fifo[1] >> 4;
            break;

        case 0xA:
            // TODO: Set COM pins hardware config
            break;

        case 0xB:
            // "set Vcomh deselect level"
            break;
        
        default:
            COMPLAIN_INVALID_COMMAND;
            break;
    }
}


void ssd1306_device::exec_command()
{
    switch(m_command_fifo[0] >> 4)
    {
        case 0x0:
            SET_LOW_NIBBLE_FROM_LOW4(m_pagemode_column_start_address, m_command_fifo[0]);
            break;

        case 0x1:
            SET_HIGH_NIBBLE_FROM_LOW4(m_pagemode_column_start_address, m_command_fifo[0]);
            break;

        case 0x2:
            exec_command_2x();
            break;
        
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7:
            m_display_start_line = m_command_fifo[0] & 0x3f;
            break;

        case 0x8:
            // $81 = contrast control
            break;
        
        case 0xA:
            exec_command_ax();
            break;

        case 0xB:
            // page addressing mode: set page start address
            if (0xB0 <= m_command_fifo[0] && m_command_fifo[0] <= 0xB7)
            {
                m_pagemode_page_start_address = m_command_fifo[0] & 7;
                return;
            }
            COMPLAIN_INVALID_COMMAND;
            break;
        
        case 0xC:
            // column scan direction: $C0 normal, $C8 reverse
            break;
        
        case 0xD:
            exec_command_dx();
            break;

        default:
            if (m_command_fifo[0] == 0xE3)
            {
                // explicit NOP
                return;
            }

            COMPLAIN_INVALID_COMMAND;
            break;
    }
}


void ssd1306_device::raw_write(int dc_line, uint8_t data)
{
    if (dc_line)
    {
        // incoming write is a command
        if (m_command_pointer == 0)
        {
            m_command_bytes_left = m_command_lengths[data];
        }

        m_command_fifo[m_command_pointer++] = data;
        m_command_bytes_left --;

        if (m_command_bytes_left == 0)
        {
            exec_command();
        }

        return;
    }

    // otherwise, display data is inbound. cancel any previous command
    m_command_pointer    = 0;
    m_command_bytes_left = 0;

    if (m_scroll_enable)
    {
        logerror("%s: attempt to write display data while scrolling enabled\n");
        return;
    }

    // Graphics RAM is split into 8 pixel high "Pages" with one byte representing
    // one vertical stripe of 8 pixels.
    // 
    // "Page addressing" mode = write to single page
    // 
    // "Horizontal addressing" mode = write pixels left to right,
    // top to bottom
    // 
    // "Vertical addressing" mode = write pixels top to bottom,
    // left to right
  

    m_gddram[ (m_page_address_pointer * 128) + m_column_address_pointer ] = data;

    switch(m_addressing_mode)
    {
        case PAGE:
            m_page_address_pointer ++;
            if (m_page_address_pointer > m_pagemode_column_end_address)
            {
                m_page_address_pointer = m_pagemode_column_start_address;
            }
            break;
        
        case HORIZONTAL:
            m_column_address_pointer ++;
            if (m_column_address_pointer > m_hvmode_column_end_address)
            {
                m_column_address_pointer = m_hvmode_column_start_address;

                m_page_address_pointer ++;
                if (m_page_address_pointer > m_hvmode_page_end_address)
                {
                    m_page_address_pointer = m_hvmode_page_start_address;
                }
            }
            break;
        
        case VERTICAL:
            m_page_address_pointer ++;
            if (m_page_address_pointer > m_hvmode_page_end_address)
            {
                m_page_address_pointer = m_hvmode_page_start_address;
                m_column_address_pointer ++;
                if (m_column_address_pointer > m_hvmode_column_end_address)
                {
                    m_column_address_pointer = m_hvmode_column_start_address;
                }
            }
            break;
    }

}

u8 ssd1306_device::raw_read(int dc_line)
{
    if (!dc_line)
    {
        // TODO: status register
        return 0;
    }

    // TODO: display RAM read
    return 0;
}


   
void ssd1306_device::write(offs_t offset, uint8_t data)
{
    if (!(m_current_interface_mode == PARALLEL_6800 ||
          m_current_interface_mode == PARALLEL_8080))
    {
        logerror("%s: write() called when not in parallel mode!\n", tag());
        return;
    }

    raw_write(m_dc_internal_state, data);
}

uint8_t ssd1306_device::read(offs_t offset)
{
    if (!(m_current_interface_mode == PARALLEL_6800 ||
          m_current_interface_mode == PARALLEL_8080))
    {
        logerror("%s: read() called when not in parallel mode!\n", tag());
        return 0;
    }

    return raw_read(m_dc_internal_state);
}


void ssd1306_device::rst_w(int rst)
{
    if (!m_reset_asserted && !rst)
    {
        device_reset();
    }

    m_reset_asserted = !rst;
}

void ssd1306_device::dc_w(int dc)
{
    // store the state, but don't sample it yet
    m_dc_line = dc != 0;

    if (m_current_interface_mode == SPI_3WIRE)
    {
        // must be connected to ground in this mode
        logerror("%s: D/C pin changed in 3-wire mode\n", tag());
        return;
    }

    if (m_current_interface_mode == I2C)
    {
        // changes I2C slave address
        return;
    }
}

void ssd1306_device::update_scan_rate()
{
    double framerate = clock() * (1 / (m_clk_div * (m_phase_1_period + m_phase_2_period + ??) * 64));




                                    //  1
        // osc_value * ---------------------------
                    //  div * display_clocks * 64
}


void ssd1306_device::spi_cs_w(int state)
{
    m_spi_cs_asserted = !state;
}

void ssd1306_device::spi_si_w(int state)
{
    if (!(m_current_interface_mode == SPI_3WIRE ||
          m_current_interface_mode == SPI_4WIRE))
    {
        logerror("%s: spi_si_w called when not in SPI mode\n", tag());
        return;
    }

    if (!m_spi_cs_asserted) return;
    m_spi_si = state ? 1 : 0;
}
    
void ssd1306_device::spi_sck_w(int state)
{
    if (!m_spi_cs_asserted)
    {
        return;
    }

    if (!(m_current_interface_mode == SPI_3WIRE ||
          m_current_interface_mode == SPI_4WIRE))
    {
        logerror("%s: spi_clk_w called when not in SPI mode\n", tag());
        return;
    }

    if (m_spi_bits_left == 0)
    {
        m_spi_shift = 0;
        
        // the D/C# line is sampled only at the start of a field
        if (m_current_interface_mode == SPI_3WIRE)
        {
            m_dc_internal_state = m_spi_si ? 1 : 0;
            m_spi_bits_left = 8;
        }
        else
        {
            m_dc_internal_state = m_dc_line;
            m_spi_shift = m_spi_si != 0 ? 1 : 0;
            m_spi_bits_left = 7;
        }
        return;
    }

    m_spi_shift = (m_spi_shift << 1) | (m_spi_si != 0 ? 1 : 0);
    m_spi_bits_left --;

    if (m_spi_bits_left == 0)
    {
        raw_write(m_dc_internal_state, m_spi_shift);
    }
}



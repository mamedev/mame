/****************************************************************************

    Solomon Systech SSD1306 OLED display driver
    "128x64 Dot Matrix OLED/PLED Segment/Common Driver with Controller"

    Datasheet:
    https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf

    Also see the common Adafruit Arduino driver:
    https://github.com/adafruit/Adafruit_SSD1306/blob/master/Adafruit_SSD1306.cpp
    
    Graphics RAM is split into 8 pixel high "Pages" with one byte representing
    one vertical stripe of 8 pixels.

    Though this display driver is intended for 128x64 displays,
    it can be configured in software to draw to smaller OLED panels.

    Frame rate is determined by:

                                 1
        osc_value * ---------------------------
                     div * display_clocks * 64

    where display_clocks is:
        phase_1_period + phase_2_period + BANK0_pulse_width

 ****************************************************************************/

#include "emu.h"
#include "ssd1306.h"

// The way to set BANK0_pulse_width isn't really described in the datasheet.
// For now, we treat it as a constant 50.
#define BANK0_PULSE_WIDTH 50

#define KEEP_LOW_NIBBLE(x)  x & 0x0F
#define KEEP_HIGH_NIBBLE(x) x & 0xF0

#define LOW4_AS_HIGH_NIBBLE(x) (x & 0x0F) << 4
#define LOW4_AS_LOW_NIBBLE(x) (x & 0x0F)

#define SET_LOW_NIBBLE_FROM_LOW4(reg, val)  reg = KEEP_HIGH_NIBBLE(reg) | LOW4_AS_LOW_NIBBLE(val);
#define SET_HIGH_NIBBLE_FROM_LOW4(reg, val) reg = KEEP_LOW_NIBBLE(reg) | LOW4_AS_HIGH_NIBBLE(val);

// It isn't really possible to get the exact frequencies because the chip
// is usually embedded into the display panel itself.
static const int INTERNAL_OSCILLATOR_FREQUENCIES[] =
{
    280'000,  // 0 (known absolute lowest)
    291'250,  // 1 (guessed)
    302'500,  // 2 (guessed)
    313'750,  // 3 (guessed)
    325'000,  // 4 (guessed)
    336'250,  // 5 (guessed)
    347'500,  // 6 (guessed)
    358'750,  // 7 (guessed)
    370'000,  // 8 (known reset value)
    394'285,  // 9 (guessed)
    418'570,  // 10 (guessed)
    442'855,  // 11 (guessed)
    467'140,  // 12 (guessed)
    491'425,  // 13 (guessed)
    515'710,  // 14 (guessed)
    540'000,  // 15 (known absolute highest)
};

static const int SCROLL_FRAME_FREQUENCY_COUNT[8] = 
{
    5,    // 0b000
    64,   // 0b001
    128,  // 0b010
    256,  // 0b011
    3,    // 0b100
    4,    // 0b101
    25,   // 0b110
    2     // 0b111
};


DEFINE_DEVICE_TYPE(SSD1306, ssd1306_device, "ssd1306", "Solomon Systech SSD1306 OLED display driver")

ssd1306_device::ssd1306_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SSD1306, tag, owner, clock),
    device_video_interface(mconfig, *this),
	device_palette_interface(mconfig, *this)
{
}

void ssd1306_device::device_start()
{
    memset(m_gddram, 0, sizeof(m_gddram));
}

void ssd1306_device::device_reset()
{
    m_current_interface_mode = m_pending_interface_mode;

    // this is done as listed in order of Chapter 9 in the datasheet
    m_contrast = 0x7f;
    m_display_blanking = false;
    m_inverting_pixels = false;
    m_display_enabled = false;

    m_horizontal_scroll_pending = false;
    m_horizontal_scroll_enabled = false;
    m_vertical_scroll_pending = false;
    m_vertical_scroll_enabled = false;

    m_vertical_scroll_top_fixed_rows = 0;
    m_vertical_scroll_bottom_scrolled_rows = 64;

    m_addressing_mode = PAGE;
    m_pagemode_column_start_address = 0;
    m_hvmode_column_start_address = 0;
    m_hvmode_column_end_address = 127;
    m_hvmode_page_start_address = 0;
    m_hvmode_page_end_address = 7;

    m_display_start_line = 0;
    m_seg0_column_remapped = false;
    m_mux_ratio = 63;
    m_row_scan_direction_inverse = false;
    m_display_offset = 0;
    m_row_scan_interleaved = true;
    m_row_scan_split_invert = false;
    m_clk_div = 0;
    m_osc_freq = 8;

    m_phase_1_period = 2;
    m_phase_2_period = 2;
    m_vcomh_deselect_level = 0x20;

    m_spi_bits_left = 0;
    m_spi_shift = 0;

    update_scan_rate();
}

void ssd1306_device::set_external_oscillator(bool using_external_oscillator)
{
    m_using_external_oscillator = using_external_oscillator;
}


void ssd1306_device::set_intf_mode(ssd1306_interface_mode_t mode)
{
    // should be tied to VCC or ground
    // behavior when toggled between resets is undefined,
    // so assume it latches at reset
    m_pending_interface_mode = mode;
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// Command processing
// 
///////////////////////////////////////////////////////////////////////////////////////////

/**
 * Indicates a multi-byte command. Keep buffering the fifo until n bytes have been read,
 * then reset the FIFO pointer and fall through to the actual command handler.
 */
#define COMMAND_BUFFER_FIFO_UNTIL_N_BYTES(data, n) \
    m_command_fifo[m_command_pointer++] = data; \
    if (m_command_pointer < n) \
    { \
        return; \
    } \
    m_command_pointer = 0; \
    logerror("%s: command %02x (multi-byte)\n", tag(), m_command_fifo[0]);

/**
 * Indicates a single-byte command. The FIFO pointer is reset,
 * and execution falls through to the code below.
 */
#define COMMAND_IS_SINGLE_BYTE \
    m_command_pointer = 0; \
    logerror("%s: command %02x\n", tag(), m_command_fifo[0]);

/**
 * Indicates this command is invalid. The FIFO pointer is reset and an error is logged.
 */
#define COMMAND_IS_INVALID \
    COMMAND_IS_SINGLE_BYTE; \
    logerror("%s: invalid/unimplemented command %02x\n", tag(), m_command_fifo[0]);

#define DUMMY_BYTE_CHECK(fifopos, expected) \
    if (m_command_fifo[fifopos] != expected) \
    { \
        logerror("%s: dummy byte in FIFO pos %d should be %02x, was %02x\n", tag(), fifopos, m_command_fifo[fifopos]); \
    }; \
    logerror("%s: command %02x (multi-byte)\n", tag(), m_command_fifo[0]);

void ssd1306_device::exec_command_2x(uint8_t data)
{
    switch(m_command_fifo[0])
    {
        case 0x20:
            COMMAND_BUFFER_FIFO_UNTIL_N_BYTES(data, 2);
            m_addressing_mode = static_cast<ssd1306_addressing_mode_t>(m_command_fifo[1] & 3);
            switch(m_addressing_mode)
            {
                case PAGE:
                    m_page_address_pointer = m_pagemode_page_start_address;
                    m_column_address_pointer = m_pagemode_column_start_address;
                    break;
                case HORIZONTAL:
                case VERTICAL:
                    m_page_address_pointer = m_hvmode_page_start_address;
                    m_column_address_pointer = m_hvmode_column_start_address;
                    break;
                default:
                    break;
            }
            
            break;

        case 0x21:
            // h/v addressing mode: set column start/end address
            COMMAND_BUFFER_FIFO_UNTIL_N_BYTES(data, 3);
            m_hvmode_column_start_address = m_command_fifo[1] & 0x7f;
            m_hvmode_column_end_address   = m_command_fifo[2] & 0x7f;

            if (m_addressing_mode == HORIZONTAL || m_addressing_mode == VERTICAL)
            {
                m_column_address_pointer = m_hvmode_column_start_address;
            }
            break;

        case 0x22:
            // h/v addressing mode: set page start/end address
            COMMAND_BUFFER_FIFO_UNTIL_N_BYTES(data, 3);
            m_hvmode_page_start_address = m_command_fifo[1] & 7;
            m_hvmode_page_end_address = m_command_fifo[2] & 7;

            if (m_addressing_mode == HORIZONTAL || m_addressing_mode == VERTICAL)
            {
                m_page_address_pointer = m_hvmode_page_start_address;
            }
            break;

        case 0x26:
        case 0x27:
            COMMAND_BUFFER_FIFO_UNTIL_N_BYTES(data, 7);
            m_horizontal_scroll_pending = m_command_fifo[0] & 1;
            DUMMY_BYTE_CHECK(1, 0);
            m_horizontal_scroll_page_start_address_pending = m_command_fifo[2] & 7;
            m_horizontal_scroll_interval_pending = m_command_fifo[3] & 7;
            m_horizontal_scroll_page_end_address_pending = m_command_fifo[4] & 7;
            DUMMY_BYTE_CHECK(5, 0);
            DUMMY_BYTE_CHECK(6, 0xFF);
            break;

        case 0x29:
        case 0x2A:
            COMMAND_BUFFER_FIFO_UNTIL_N_BYTES(data, 6);
            m_horizontal_scroll_pending = m_command_fifo[0] & 1;
            DUMMY_BYTE_CHECK(1, 0);
            m_horizontal_scroll_page_start_address_pending = m_command_fifo[2] & 7;
            m_horizontal_scroll_interval_pending = m_command_fifo[3] & 7;
            m_horizontal_scroll_page_end_address_pending = m_command_fifo[4] & 7;
            m_vertical_scroll_offset_pending = m_command_fifo[5] & 0x3f;
            break;

        case 0x2E:
            COMMAND_IS_SINGLE_BYTE;

            // scroll disable
            m_horizontal_scroll_enabled = false;
            m_vertical_scroll_enabled   = false;
            m_horizontal_scroll_pending = false;
            m_vertical_scroll_pending   = false;
            break;
        
        case 0x2F:
            COMMAND_IS_SINGLE_BYTE;

            // scroll enable
            m_horizontal_scroll_enabled = m_horizontal_scroll_pending;
            m_vertical_scroll_enabled = m_vertical_scroll_pending;
            m_horizontal_scrolling_left = m_horizontal_scrolling_left_pending;
            m_horizontal_scroll_page_start_address = m_horizontal_scroll_page_start_address_pending;
            m_horizontal_scroll_interval = m_horizontal_scroll_interval_pending;
            m_horizontal_scroll_page_end_address = m_horizontal_scroll_page_end_address_pending;
            m_vertical_scroll_offset = m_vertical_scroll_offset_pending;
            break;

        default:
            COMMAND_IS_INVALID;
            return;
    }
}

void ssd1306_device::exec_command_ax(uint8_t data)
{
    switch(m_command_fifo[0])
    {
        case 0xA0:
        case 0xA1:
            // remap SEG0 column: false = SEG0 is 0, true = SEG0 is 127
            COMMAND_IS_SINGLE_BYTE;
            m_seg0_column_remapped = m_command_fifo[0] & 1;
            break;

        case 0xA3:
            // vertical scroll parameters
            COMMAND_BUFFER_FIFO_UNTIL_N_BYTES(data, 3);
            m_vertical_scroll_top_fixed_rows        = m_command_fifo[1] & 0x3f;
            m_vertical_scroll_bottom_scrolled_rows  = m_command_fifo[2] & 0x7f;
            break;

        case 0xA4:
        case 0xA5:
            COMMAND_IS_SINGLE_BYTE;
            m_display_blanking = m_command_fifo[0] & 1;
            break;

        case 0xA6:
        case 0xA7:
            COMMAND_IS_SINGLE_BYTE;
            m_inverting_pixels = m_command_fifo[0] & 1;
            break;

        case 0xA8:
            // mux ratio, i.e., total number of lines in framebuffer.
            // any line in the framebuffer past this point won't be drawn.
            COMMAND_BUFFER_FIFO_UNTIL_N_BYTES(data, 2);

            m_command_fifo[1] &= 0x3f;

            if (m_command_fifo[1] < 15)
            {
                logerror("%s: invalid mux ratio: %02x\n", m_command_fifo[1]);
                return;
            }

            m_mux_ratio = m_command_fifo[1];
            break;

        case 0xAE:
        case 0xAF:
            COMMAND_IS_SINGLE_BYTE;
            m_display_enabled = m_command_fifo[0] & 1;
            break;

        default:
            COMMAND_IS_INVALID;
            return;
    }
}

void ssd1306_device::exec_command_dx(uint8_t data)
{
    switch(m_command_fifo[0])
    {
        case 0xD3:
            // display offset (shifts image down by n lines vertically)
            COMMAND_BUFFER_FIFO_UNTIL_N_BYTES(data, 2);
            m_display_offset = m_command_fifo[1] & 0x3f;
            break;
 
        case 0xD5:
            COMMAND_BUFFER_FIFO_UNTIL_N_BYTES(data, 2);

            m_clk_div  = m_command_fifo[1] & 0xf;
            m_osc_freq = m_command_fifo[1] >> 4;
            break;


        case 0xD9:
            COMMAND_BUFFER_FIFO_UNTIL_N_BYTES(data, 2);

            m_phase_1_period = m_command_fifo[1] & 0xf;
            m_phase_2_period = m_command_fifo[1] >> 4;
            break;

        case 0xDA:
            // COM pin scan direction
            COMMAND_BUFFER_FIFO_UNTIL_N_BYTES(data, 2);

            if (!(m_command_fifo[1] & 2))
            {
                // bit should be set
            }

            m_row_scan_interleaved  = (m_command_fifo[1] & 0x10);
            m_row_scan_split_invert = (m_command_fifo[1] & 0x20);
            break;

        case 0xDB:
            if (m_command_fifo[1] & ~0x70)
            {
                // other bits should be zero
            }

            m_vcomh_deselect_level = m_command_fifo[1] & 0x70;
            break;
        
        default:
            COMMAND_IS_INVALID;
            return;
    }
}


void ssd1306_device::exec_command(uint8_t data)
{
    if (m_command_pointer == 0)
    {
        m_command_fifo[0] = data;
    }

    switch(m_command_fifo[0] & 0xF0)
    {
        case 0x00:
            COMMAND_IS_SINGLE_BYTE;
            SET_LOW_NIBBLE_FROM_LOW4(m_pagemode_column_start_address, m_command_fifo[0]);
            break;

        case 0x10:
            COMMAND_IS_SINGLE_BYTE;
            SET_HIGH_NIBBLE_FROM_LOW4(m_pagemode_column_start_address, m_command_fifo[0]);
            break;

        case 0x20:
            exec_command_2x(data);
            break;
        
        case 0x40:
        case 0x50:
        case 0x60:
        case 0x70:
            COMMAND_IS_SINGLE_BYTE;
            m_display_start_line = m_command_fifo[0] & 0x3f;
            break;

        case 0x80:
            if (m_command_fifo[0] == 0x8D)
            {
                COMMAND_BUFFER_FIFO_UNTIL_N_BYTES(data, 2);
                // charge pump setting, which amazingly isn't grouped
                // in with the rest of the commands...
                return;
            }

            if (m_command_fifo[0] != 0x81)
            {
                COMMAND_IS_INVALID;
                return;
            }

            COMMAND_BUFFER_FIFO_UNTIL_N_BYTES(data, 2);
            m_contrast = m_command_fifo[1];
            break;
        
        case 0xA0:
            exec_command_ax(data);
            break;

        case 0xB0:
            // page addressing mode: set page start address
            if (!(0xB0 <= m_command_fifo[0] && m_command_fifo[0] <= 0xB7))
            {
                COMMAND_IS_INVALID;
                return;
            }
            
            COMMAND_IS_SINGLE_BYTE;
            m_pagemode_page_start_address = m_command_fifo[0] & 7;
            break;
        
        case 0xC0:
            // COM (row) scan direction: $C0 normal, $C8 reverse
            if (!(m_command_fifo[0] == 0xC0 || m_command_fifo[0] == 0xC8))
            {
                COMMAND_IS_INVALID;
                return;
            }

            COMMAND_IS_SINGLE_BYTE;
            m_row_scan_direction_inverse = (m_command_fifo[0] & 8);
            break;
        
        case 0xD0:
            exec_command_dx(data);
            break;

        default:
            if (m_command_fifo[0] != 0xE3)
            {
                COMMAND_IS_INVALID;
                return;
            }

            COMMAND_IS_SINGLE_BYTE;
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// I/O handling
//
///////////////////////////////////////////////////////////////////////////////////////////

void ssd1306_device::raw_write(int dc_line, uint8_t data)
{
    if (!dc_line)
    {
        exec_command(data);
        return;
    }

    // otherwise, display data is inbound. cancel any previous command
    m_command_pointer    = 0;

    if (m_horizontal_scroll_enabled || m_vertical_scroll_enabled)
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

    int address = (m_page_address_pointer * 128) + m_column_address_pointer;
    // logerror("%s: write data %02x -> %04x (in addressing mode %d)\n",
    //          tag(),
    //          data,
    //          address,
    //          m_addressing_mode);

    m_gddram[address] = data;

    switch(m_addressing_mode)
    {
        case PAGE:
            m_column_address_pointer ++;
            if (m_column_address_pointer >= 128)  // m_pagemode_column_end_address)
            {
                m_column_address_pointer = m_pagemode_column_start_address;
            }
            break;
        
        case HORIZONTAL:
            m_column_address_pointer ++;
            if (m_column_address_pointer > std::min((int)m_hvmode_column_end_address, 127))
            {
                m_column_address_pointer = m_hvmode_column_start_address;

                m_page_address_pointer ++;
                if (m_page_address_pointer > std::min((int)m_hvmode_page_end_address, 7))
                {
                    m_page_address_pointer = m_hvmode_page_start_address;
                }
            }
            break;
        
        case VERTICAL:
            m_page_address_pointer ++;
            if (m_page_address_pointer > std::min((int)m_hvmode_page_end_address, 7))
            {
                m_page_address_pointer = m_hvmode_page_start_address;
                m_column_address_pointer ++;
                if (m_column_address_pointer > std::min((int)m_hvmode_column_end_address, 127))
                {
                    m_column_address_pointer = m_hvmode_column_start_address;
                }
            }
            break;
        
        default:
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
    // store the state, but don't sample it yet.
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


void ssd1306_device::spi_cs_w(int state)
{
    if (!(m_current_interface_mode == SPI_3WIRE ||
          m_current_interface_mode == SPI_4WIRE))
    {
        logerror("%s: spi_cs_w called when not in SPI mode\n", tag());
        return;
    }
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
    if (m_reset_asserted || !m_spi_cs_asserted)
    {
        return;
    }

    if (!(m_current_interface_mode == SPI_3WIRE ||
          m_current_interface_mode == SPI_4WIRE))
    {
        logerror("%s: spi_clk_w called when not in SPI mode\n", tag());
        return;
    }

    if (m_spi_sck_asserted || !state)
    {
        m_spi_sck_asserted = state != 0;
        return;
    }

    m_spi_sck_asserted = true;

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
            m_spi_shift = m_spi_si ? 1 : 0;
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

///////////////////////////////////////////////////////////////////////////////////////////
//
// Screen interface code
//
///////////////////////////////////////////////////////////////////////////////////////////

u32 ssd1306_device::palette_entries() const noexcept
{
	return 2; // monochrome
}

void ssd1306_device::update_scan_rate()
{
    if (!m_using_external_oscillator)
    {
        set_clock(INTERNAL_OSCILLATOR_FREQUENCIES[m_osc_freq]);
    }

    double display_clocks = (m_phase_1_period + m_phase_2_period + BANK0_PULSE_WIDTH);
    double framerate = clock() * (1 / ((m_clk_div + 1) * display_clocks * 64));

    screen().set_refresh_hz(framerate);
    screen().set_vblank_time(0);
}

uint32_t ssd1306_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
    bitmap.fill(rgb_t::black(), cliprect);
    if (!m_display_enabled)
    {
        return 0;
    }

    if (m_display_blanking)
    {
        bitmap.fill(rgb_t::white(), cliprect);
        return 0;
    }


    rgb_t on_pixel  = !m_inverting_pixels ? white_pen() : black_pen();
    rgb_t off_pixel = !m_inverting_pixels ? black_pen() : white_pen();
 
    // very simple rendering code for the time being...
    for (int y = 0; y < 64; y++)
    {
        for (int x = 0; x < 128; x++)
        {
            int real_y = (m_row_scan_direction_inverse ? 63-y : y);

            uint8_t stripe = m_gddram[(128 * (real_y / 8)) + x];

            bitmap.pix(y, x) = (stripe & (0x80 >> (real_y % 8))) ? on_pixel : off_pixel;
        }
    }
	return 0;
}

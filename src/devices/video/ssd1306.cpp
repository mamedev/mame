// license:BSD-3-Clause
// copyright-holders:wurthless-elektroniks
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

	m_addressing_mode = SSD1306_ADDRESSING_MODE_PAGE;
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

void ssd1306_device::set_intf_mode(uint8_t mode)
{
	// should be tied to VCC or ground
	// behavior when toggled between resets is undefined,
	// so assume it latches at reset
	m_pending_interface_mode = mode;
}

void ssd1306_device::set_base_rowscan_invert(bool base_rowscan_invert)
{
	// the OLED on the Adafruit display, as well as the one in the Arduboy,
	// is internally wired so that COM0 is wired to the bottom-most row.
	// this forces programmers to manually set the row scan invert flag at boot time.
	// this setting is here so that, if there's ever a display that isn't built like this,
	// then inverting the row scan actually flips the display vertically.
	m_base_rowscan_invert = base_rowscan_invert;
}

inline bool ssd1306_device::populate_fifo_until_n_bytes(uint8_t data, int num_bytes)
{
	m_command_fifo[m_command_pointer++] = data;
	if (m_command_pointer < num_bytes)
	{
		return false;
	}

	m_command_pointer = 0;
	return true;
}

inline void ssd1306_device::handle_invalid_command()
{
	m_command_pointer = 0;
	logerror("%s: invalid/unimplemented command %02x\n", machine().describe_context(), m_command_fifo[0]);
}

#define DUMMY_BYTE_CHECK(fifopos, expected) \
	if (m_command_fifo[fifopos] != expected) \
	{ \
		logerror("%s: dummy byte in FIFO pos %d should be %02x, was %02x\n", machine().describe_context(), fifopos, expected, m_command_fifo[fifopos]); \
	};

void ssd1306_device::exec_command_2x(uint8_t data)
{
	switch (m_command_fifo[0])
	{
		case 0x20: // set addressing mode
			if (!populate_fifo_until_n_bytes(data, 2)) return;

			m_addressing_mode = m_command_fifo[1] & 3;
			switch (m_addressing_mode)
			{
				case SSD1306_ADDRESSING_MODE_PAGE:
					m_page_address_pointer = m_pagemode_page_start_address;
					m_column_address_pointer = m_pagemode_column_start_address;
					break;
				case SSD1306_ADDRESSING_MODE_HORIZONTAL:
				case SSD1306_ADDRESSING_MODE_VERTICAL:
					m_page_address_pointer = m_hvmode_page_start_address;
					m_column_address_pointer = m_hvmode_column_start_address;
					break;
				default:
					break;
			}
			break;

		case 0x21: // h/v addressing mode: set column start/end address
			if (!populate_fifo_until_n_bytes(data, 3)) return;

			m_hvmode_column_start_address = m_command_fifo[1] & 0x7f;
			m_hvmode_column_end_address   = m_command_fifo[2] & 0x7f;

			if (m_addressing_mode == SSD1306_ADDRESSING_MODE_HORIZONTAL ||
				m_addressing_mode == SSD1306_ADDRESSING_MODE_VERTICAL)
			{
				m_column_address_pointer = m_hvmode_column_start_address;
			}
			break;

		case 0x22: // h/v addressing mode: set page start/end address
			if (!populate_fifo_until_n_bytes(data, 3)) return;

			m_hvmode_page_start_address = m_command_fifo[1] & 7;
			m_hvmode_page_end_address = m_command_fifo[2] & 7;

			if (m_addressing_mode == SSD1306_ADDRESSING_MODE_HORIZONTAL ||
				m_addressing_mode == SSD1306_ADDRESSING_MODE_VERTICAL)
			{
				m_page_address_pointer = m_hvmode_page_start_address;
			}
			break;

		case 0x26:
		case 0x27: // init left/right horizontal scroll
			if (!populate_fifo_until_n_bytes(data, 7)) return;

			m_horizontal_scroll_pending = true;
			m_horizontal_scrolling_left_pending = m_command_fifo[0] & 1;
			DUMMY_BYTE_CHECK(1, 0);
			m_horizontal_scroll_page_start_address_pending = m_command_fifo[2] & 7;
			m_horizontal_scroll_interval_pending = m_command_fifo[3] & 7;
			m_horizontal_scroll_page_end_address_pending = m_command_fifo[4] & 7;
			DUMMY_BYTE_CHECK(5, 0);
			DUMMY_BYTE_CHECK(6, 0xff);
			break;

		case 0x29:
		case 0x2a: // init left/right horizontal scroll + upward vertical scroll
			if (!populate_fifo_until_n_bytes(data, 6)) return;

			m_horizontal_scroll_pending = true;
			m_vertical_scroll_pending = true;
			m_horizontal_scrolling_left_pending = m_command_fifo[0] & 1;
			DUMMY_BYTE_CHECK(1, 0);
			m_horizontal_scroll_page_start_address_pending = m_command_fifo[2] & 7;
			m_horizontal_scroll_interval_pending = m_command_fifo[3] & 7;
			m_horizontal_scroll_page_end_address_pending = m_command_fifo[4] & 7;
			m_vertical_scroll_offset_pending = m_command_fifo[5] & 0x3f;
			break;

		case 0x2e: // scroll disable
			if (!populate_fifo_until_n_bytes(data, 1)) return;

			m_horizontal_scroll_enabled = false;
			m_vertical_scroll_enabled   = false;
			m_horizontal_scroll_pending = false;
			m_vertical_scroll_pending   = false;
			break;

		case 0x2f: // scroll enable
			if (!populate_fifo_until_n_bytes(data, 1)) return;

			m_horizontal_scroll_enabled = m_horizontal_scroll_pending;
			m_vertical_scroll_enabled = m_vertical_scroll_pending;
			m_horizontal_scrolling_left = m_horizontal_scrolling_left_pending;
			m_horizontal_scroll_page_start_address = m_horizontal_scroll_page_start_address_pending;
			m_horizontal_scroll_interval = m_horizontal_scroll_interval_pending;
			m_horizontal_scroll_page_end_address = m_horizontal_scroll_page_end_address_pending;
			m_vertical_scroll_offset = m_vertical_scroll_offset_pending;
			break;

		default:
			handle_invalid_command();
			return;
	}
}

void ssd1306_device::exec_command_ax(uint8_t data)
{
	switch (m_command_fifo[0])
	{
		case 0xa0:
		case 0xa1: // remap SEG0 column: false = SEG0 is 0, true = SEG0 is 127
			if (!populate_fifo_until_n_bytes(data, 1)) return;

			m_seg0_column_remapped = m_command_fifo[0] & 1;
			break;

		case 0xa3: // vertical scroll parameters
			if (!populate_fifo_until_n_bytes(data, 3)) return;

			m_vertical_scroll_top_fixed_rows        = m_command_fifo[1] & 0x3f;
			m_vertical_scroll_bottom_scrolled_rows  = m_command_fifo[2] & 0x7f;
			break;

		case 0xa4:
		case 0xa5: // draw GDDRAM contents, or blank display
			if (!populate_fifo_until_n_bytes(data, 1)) return;

			m_display_blanking = m_command_fifo[0] & 1;
			break;

		case 0xa6:
		case 0xa7: // normal/inverted pixels
			if (!populate_fifo_until_n_bytes(data, 1)) return;

			m_inverting_pixels = m_command_fifo[0] & 1;
			break;

		case 0xa8: // mux ratio (max lines to draw)
			if (!populate_fifo_until_n_bytes(data, 3)) return;

			m_command_fifo[1] &= 0x3f;

			if (m_command_fifo[1] < 15)
			{
				logerror("%s: invalid mux ratio: %02x\n", m_command_fifo[1]);
				return;
			}

			m_mux_ratio = m_command_fifo[1];
			break;

		case 0xae:
		case 0xaf: // enable/disable display completely
			if (!populate_fifo_until_n_bytes(data, 1)) return;

			m_display_enabled = m_command_fifo[0] & 1;
			break;

		default:
			handle_invalid_command();
			return;
	}
}

void ssd1306_device::exec_command_dx(uint8_t data)
{
	switch (m_command_fifo[0])
	{
		case 0xd3: // display offset (shifts image down by n lines vertically)
			if (!populate_fifo_until_n_bytes(data, 2)) return;

			m_display_offset = m_command_fifo[1] & 0x3f;
			break;

		case 0xd5: // clock divider and internal oscillator frequency select
			if (!populate_fifo_until_n_bytes(data, 2)) return;

			m_clk_div  = m_command_fifo[1] & 0xf;
			m_osc_freq = m_command_fifo[1] >> 4;
			break;

		case 0xd9: // precharge phase 1/2 control (affects scan rate)
			if (!populate_fifo_until_n_bytes(data, 2)) return;

			m_phase_1_period = m_command_fifo[1] & 0xf;
			m_phase_2_period = m_command_fifo[1] >> 4;
			break;

		case 0xda: // COM pin (row) scan direction and mode
			if (!populate_fifo_until_n_bytes(data, 2)) return;

			if (!(m_command_fifo[1] & 2))
			{
				logerror("%s: command da, byte 1: bit 1 should have been set, but wasn't.\n",
						machine().describe_context());
			}
			m_row_scan_interleaved  = (m_command_fifo[1] & 0x10);
			m_row_scan_split_invert = (m_command_fifo[1] & 0x20);
			break;

		case 0xdb: // VcomH deselect level (only hardware needs this)
			if (!populate_fifo_until_n_bytes(data, 2)) return;

			if (m_command_fifo[1] & ~0x70)
			{
				logerror("%s: command db, byte 1: bits other than 4-7 were set.\n",
						machine().describe_context());
			}
			m_vcomh_deselect_level = m_command_fifo[1] & 0x70;
			break;

		default:
			handle_invalid_command();
			return;
	}
}

void ssd1306_device::exec_command(uint8_t data)
{
	if (m_command_pointer == 0)
	{
		m_command_fifo[0] = data;
	}

	switch (m_command_fifo[0] & 0xf0)
	{
		case 0x00:
		case 0x10: // page mode column start address
			{
				if (!populate_fifo_until_n_bytes(data, 1)) return;

				uint8_t low4        = m_command_fifo[0] & 0xf;
				uint8_t keep_mask   = m_command_fifo[0] & 0x10 ? 0x0f : 0xf0;
				uint8_t shift_value = m_command_fifo[0] & 0x10 ? 4    : 0;

				m_pagemode_column_start_address =
					(m_pagemode_column_start_address & keep_mask) |
					(low4 << shift_value);

				if (m_addressing_mode == SSD1306_ADDRESSING_MODE_PAGE)
				{
					m_column_address_pointer = m_pagemode_column_start_address;
				}
			}
			break;

		case 0x20:
			exec_command_2x(data);
			break;

		case 0x40:
		case 0x50:
		case 0x60:
		case 0x70:
			if (!populate_fifo_until_n_bytes(data, 1)) return;

			m_display_start_line = m_command_fifo[0] & 0x3f;
			break;

		case 0x80:
			if (m_command_fifo[0] == 0x81)
			{
				if (!populate_fifo_until_n_bytes(data, 2)) return;

				m_contrast = m_command_fifo[1];
				return;
			}
			if (m_command_fifo[0] == 0x8d)
			{
				if (!populate_fifo_until_n_bytes(data, 2)) return;

				// charge pump setting; don't really need to implement it here
				return;
			}

			handle_invalid_command();
			break;

		case 0xa0:
			exec_command_ax(data);
			break;

		case 0xb0:
			// page addressing mode: set page start address
			if (0xb0 <= m_command_fifo[0] && m_command_fifo[0] <= 0xb7)
			{
				if (!populate_fifo_until_n_bytes(data, 1)) return;

				m_pagemode_page_start_address = m_command_fifo[0] & 7;

				if (m_addressing_mode == SSD1306_ADDRESSING_MODE_PAGE)
				{
					m_page_address_pointer = m_pagemode_page_start_address;
				}
				return;
			}

			handle_invalid_command();
			break;

		case 0xc0: // COM (row) scan direction: $C0 normal, $C8 reverse
			if (m_command_fifo[0] == 0xc0 || m_command_fifo[0] == 0xc8)
			{
				if (!populate_fifo_until_n_bytes(data, 1)) return;

				m_row_scan_direction_inverse = (m_command_fifo[0] & 8);	
				return;
			}
			handle_invalid_command();
			break;

		case 0xd0:
			exec_command_dx(data);
			break;

		default:
			if (m_command_fifo[0] == 0xe3)
			{
				if (!populate_fifo_until_n_bytes(data, 1)) return;

				// explicit NOP
				return;
			}
			handle_invalid_command();
			break;
	}
}

void ssd1306_device::raw_write(int dc_line, uint8_t data)
{
	if (!dc_line)
	{
		exec_command(data);
		return;
	}

	// if D/C line is high, then data's inbound, and we should cancel
	// whatever command was being written
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
	// 
	// The Adafruit SSD1306 driver and most Arduboy games seem to use
	// horizontal mode exclusively; drawing to a framebuffer on the
	// ATMega chip, then copying it over to the display in one shot.
	int address = (m_page_address_pointer * 128) + m_column_address_pointer;

	m_gddram[address] = data;

	switch (m_addressing_mode)
	{
		case SSD1306_ADDRESSING_MODE_PAGE:
			m_column_address_pointer ++;
			if (m_column_address_pointer >= 128)
			{
				m_column_address_pointer = m_pagemode_column_start_address;
			}
			break;

		case SSD1306_ADDRESSING_MODE_HORIZONTAL:
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

		case SSD1306_ADDRESSING_MODE_VERTICAL:
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

uint8_t ssd1306_device::raw_read(int dc_line)
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
	if (!(m_current_interface_mode == SSD1306_INTERFACE_MODE_PARALLEL_6800 ||
		  m_current_interface_mode == SSD1306_INTERFACE_MODE_PARALLEL_8080))
	{
		logerror("%s: write() called when not in parallel mode\n", machine().describe_context());
		return;
	}

	raw_write(m_dc_internal_state, data);
}

uint8_t ssd1306_device::read(offs_t offset)
{
	if (!(m_current_interface_mode == SSD1306_INTERFACE_MODE_PARALLEL_6800 ||
		  m_current_interface_mode == SSD1306_INTERFACE_MODE_PARALLEL_8080))
	{
		logerror("%s: read() called when not in parallel mode\n", machine().describe_context());
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

	if (m_current_interface_mode == SSD1306_INTERFACE_MODE_SPI_3WIRE)
	{
		// must be connected to ground in this mode
		logerror("%s: D/C pin changed in 3-wire mode\n", machine().describe_context());
		return;
	}

	if (m_current_interface_mode == SSD1306_INTERFACE_MODE_I2C)
	{
		// changes I2C slave address, probably only at reset
		return;
	}
}

void ssd1306_device::spi_cs_w(int state)
{
	if (!(m_current_interface_mode == SSD1306_INTERFACE_MODE_SPI_3WIRE ||
		  m_current_interface_mode == SSD1306_INTERFACE_MODE_SPI_4WIRE))
	{
		logerror("%s: spi_cs_w called when not in SPI mode\n", machine().describe_context());
		return;
	}
	m_spi_cs_asserted = !state;
}

void ssd1306_device::spi_si_w(int state)
{
	if (!(m_current_interface_mode == SSD1306_INTERFACE_MODE_SPI_3WIRE ||
		  m_current_interface_mode == SSD1306_INTERFACE_MODE_SPI_4WIRE))
	{
		logerror("%s: spi_si_w called when not in SPI mode\n", machine().describe_context());
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

	if (!(m_current_interface_mode == SSD1306_INTERFACE_MODE_SPI_3WIRE ||
		  m_current_interface_mode == SSD1306_INTERFACE_MODE_SPI_4WIRE))
	{
		logerror("%s: spi_clk_w called when not in SPI mode\n", machine().describe_context());
		return;
	}

	if (m_spi_sck_asserted || !state)
	{
		m_spi_sck_asserted = state != 0;
		return;
	}

	// only take action on rising edge of SCK
	m_spi_sck_asserted = true;

	if (m_spi_bits_left == 0)
	{
		m_spi_shift = 0;

		// the D/C# line is sampled only at the start of a field
		if (m_current_interface_mode == SSD1306_INTERFACE_MODE_SPI_3WIRE)
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

uint32_t ssd1306_device::palette_entries() const noexcept
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
	screen.palette().set_pen_color(0, rgb_t::black());
	screen.palette().set_pen_color(1, m_contrast, m_contrast, m_contrast);
	
	bitmap.fill(screen.palette().pen(0), cliprect);
	if (!m_display_enabled)
	{
		return 0;
	}

	if (m_display_blanking)
	{
		bitmap.fill(screen.palette().pen(1), cliprect);
		return 0;
	}

	rgb_t on_pixel  = screen.palette().pen(!m_inverting_pixels ? 1 : 0);
	rgb_t off_pixel = screen.palette().pen(!m_inverting_pixels ? 0 : 1);

	// very simple rendering code for the time being...
	//
	// TODO: scrolling (horizontal = left/right, vertical = up).
	// many Arduboy games do not use scrolling and instead draw everything in an internal
	// framebuffer that is then uploaded to the screen.
	//
	// advanced remapping modes may also need to be implemented...
	for (int y = 0; y < 64; y++)
	{
		for (int x = 0; x < 128; x++)
		{
			int real_y = ((m_base_rowscan_invert ^ m_row_scan_direction_inverse) ? 63-y : y);

			uint8_t stripe = m_gddram[(128 * (real_y >> 3)) + x];

			bitmap.pix(y, x) = (stripe & (1 << (real_y & 7))) ? on_pixel : off_pixel;
		}
	}
	return 0;
}

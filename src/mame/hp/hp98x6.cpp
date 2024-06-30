// license:BSD-3-Clause
// copyright-holders:F. Ulivi

// **************************
// Driver for HP 98x6 systems
// **************************
//
// What's in for 9816:
// - 8MHz M68000 CPU
// - Boot ROM
// - Configurable amount of RAM
// - Text video with attributes
// - Graphic video
// - HPIB interface
// - RS232 interface
// - HLE of 8041 UPI
// - Large keyboard (98203B)
// - Knob
// - Beeper
// - ID PROM
//
// Main references:
// - Olivier De Smet's standalone emulator:
//   https://sites.google.com/site/olivier2smet2/hp_projects/hp98x6
// - Tony Duell's reverse engineered schematics (available @ hpmuseum.net)

#include "emu.h"

#include "hp98x6_upi.h"

#include "bus/ieee488/ieee488.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/ins8250.h"
#include "machine/ram.h"
#include "machine/tms9914.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"

// Debugging
#define VERBOSE 0
#include "logmacro.h"

namespace {

// CPU clock
constexpr auto CPU_CLOCK = 16_MHz_XTAL / 2;

// Video dot clock
constexpr unsigned DOT_CLOCK = 19'988'000;

// HPIB and UPI clock
constexpr auto HPIB_CLOCK = 10_MHz_XTAL / 2;

// UART clock
constexpr auto UART_CLOCK = 2.4576_MHz_XTAL;

// Bit manipulation
template<typename T> constexpr T BIT_MASK(unsigned n)
{
	return (T)1U << n;
}

template<typename T> void BIT_CLR(T& w , unsigned n)
{
	w &= ~BIT_MASK<T>(n);
}

template<typename T> void BIT_SET(T& w , unsigned n)
{
	w |= BIT_MASK<T>(n);
}

// +--------------+
// | hp9816_state |
// +--------------+
class hp9816_state : public driver_device
{
public:
	hp9816_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_ram(*this, RAM_TAG)
		, m_crtc(*this, "crtc")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_upi(*this, "upi")
		, m_hpib(*this, "hpib")
		, m_uart(*this, "uart")
		, m_rs232(*this, "rs232")
		, m_sw2_dipswitches(*this, "sw2")
		, m_sw3_dipswitches(*this, "sw3")
		, m_chargen(*this, "chargen")
	{
	}

	void hp9816(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<m68000_device> m_cpu;
	required_device<ram_device> m_ram;
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<hp98x6_upi_device> m_upi;
	required_device<tms9914_device> m_hpib;
	required_device<ins8250_device> m_uart;
	required_device<rs232_port_device> m_rs232;
	required_ioport m_sw2_dipswitches;
	required_ioport m_sw3_dipswitches;

	// Character generator
	required_region_ptr<uint8_t> m_chargen;

private:
	static inline constexpr unsigned TEXT_VRAM_SIZE = 2048;
	static inline constexpr unsigned GRAPHIC_VRAM_SIZE = 16384;

	void cpu_mem_map(address_map &map);
	void diag_led_w(uint8_t data);
	uint16_t text_r(offs_t offset);
	void text_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t graphic_r(offs_t offset, uint16_t mem_mask);
	void graphic_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint8_t hpib_r(offs_t offset);
	void hpib_w(offs_t offset, uint8_t data);
	void uart_reset();
	void uart_update_irq();
	uint8_t uart_r(offs_t offset);
	void uart_w(offs_t offset, uint8_t data);

	MC6845_UPDATE_ROW(crtc_update_row);

	void cpu_reset_w(int state);
	void hpib_irq_w(int state);
	void upi_irq7_w(int state);
	void uart_irq_w(int state);

	uint16_t m_text_vram[TEXT_VRAM_SIZE];
	uint8_t m_graphic_vram[GRAPHIC_VRAM_SIZE];
	bool m_hsync_en;
	bool m_graphic_en;
	bool m_hpib_irq;
	bool m_hpib_dma_en;
	bool m_upi_irq7;
	bool m_uart_int_en;
	bool m_uart_irq;
	bool m_uart_ocd3;
	bool m_uart_ocd4;
};

void hp9816_state::hp9816(machine_config &config)
{
	M68000(config, m_cpu, CPU_CLOCK);
	m_cpu->set_addrmap(AS_PROGRAM, &hp9816_state::cpu_mem_map);
	m_cpu->reset_cb().set(FUNC(hp9816_state::cpu_reset_w));

	RAM(config, m_ram).set_default_size("256K").set_extra_options("512K,1M");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER, rgb_t::white());
	// Parameters for 50 Hz frame rate
	m_screen->set_raw(DOT_CLOCK, 1040, 0, 800, 384, 0, 300);
	m_screen->set_screen_update(m_crtc, FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);

	MC6845(config, m_crtc, DOT_CLOCK / 10);
	m_crtc->set_char_width(10);
	m_crtc->set_screen(m_screen);
	m_crtc->set_update_row_callback(FUNC(hp9816_state::crtc_update_row));
	m_crtc->set_show_border_area(false);

	HP98X6_UPI(config, m_upi, HPIB_CLOCK);
	m_upi->irq1_write_cb().set_inputline(m_cpu, M68K_IRQ_1);
	m_upi->irq7_write_cb().set(FUNC(hp9816_state::upi_irq7_w));

	TMS9914(config, m_hpib, HPIB_CLOCK);
	m_hpib->int_write_cb().set(FUNC(hp9816_state::hpib_irq_w));
	m_hpib->dio_read_cb().set(IEEE488_TAG, FUNC(ieee488_device::dio_r));
	m_hpib->dio_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dio_w));
	m_hpib->eoi_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_eoi_w));
	m_hpib->dav_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dav_w));
	m_hpib->nrfd_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_nrfd_w));
	m_hpib->ndac_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ndac_w));
	m_hpib->ifc_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ifc_w));
	m_hpib->srq_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_srq_w));
	m_hpib->atn_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_atn_w));
	m_hpib->ren_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ren_w));

	ieee488_device &ieee(IEEE488(config, IEEE488_TAG));
	ieee.eoi_callback().set(m_hpib , FUNC(tms9914_device::eoi_w));
	ieee.dav_callback().set(m_hpib , FUNC(tms9914_device::dav_w));
	ieee.nrfd_callback().set(m_hpib , FUNC(tms9914_device::nrfd_w));
	ieee.ndac_callback().set(m_hpib , FUNC(tms9914_device::ndac_w));
	ieee.ifc_callback().set(m_hpib , FUNC(tms9914_device::ifc_w));
	ieee.srq_callback().set(m_hpib , FUNC(tms9914_device::srq_w));
	ieee.atn_callback().set(m_hpib , FUNC(tms9914_device::atn_w));
	ieee.ren_callback().set(m_hpib , FUNC(tms9914_device::ren_w));
	IEEE488_SLOT(config, "ieee_rem", 0, remote488_devices, nullptr);

	INS8250(config, m_uart, UART_CLOCK);
	m_uart->out_int_callback().set(FUNC(hp9816_state::uart_irq_w));
	m_uart->out_tx_callback().set("rs232", FUNC(rs232_port_device::write_txd));
	m_uart->out_dtr_callback().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_uart->out_rts_callback().set("rs232", FUNC(rs232_port_device::write_rts));
	m_uart->out_out1_callback().set("rs232", FUNC(rs232_port_device::write_spds));
	// OUT2 -> SRTS

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_uart, FUNC(ins8250_device::rx_w));
	m_rs232->cts_handler().set(m_uart, FUNC(ins8250_device::cts_w));
	m_rs232->dsr_handler().set(m_uart, FUNC(ins8250_device::dsr_w));
	m_rs232->dcd_handler().set(m_uart, FUNC(ins8250_device::dcd_w));
	m_rs232->ri_handler().set(m_uart, FUNC(ins8250_device::ri_w));
}

void hp9816_state::machine_start()
{
	save_item(NAME(m_text_vram));
	save_item(NAME(m_graphic_vram));
	save_item(NAME(m_hsync_en));
	save_item(NAME(m_graphic_en));
	save_item(NAME(m_hpib_dma_en));
	save_item(NAME(m_uart_int_en));
	save_item(NAME(m_uart_ocd3));
	save_item(NAME(m_uart_ocd4));

	m_cpu->space(AS_PROGRAM).install_ram(0x1000000 - m_ram->size(), 0xffffff, m_ram->pointer());
}

void hp9816_state::machine_reset()
{
	m_hsync_en = false;
	m_graphic_en = false;
	m_hpib_dma_en = false;
	uart_reset();
	diag_led_w(0);
}

void hp9816_state::cpu_mem_map(address_map &map)
{
	map.unmap_value_high();
	// Range           DTACK Device
	// ==============================
	// 00'0000-00'ffff *     Boot ROM
	// 01'0000-01'ffff *     Diagnostic LEDs (write only)
	// 02'0000-3f'ffff *     None
	// 40'0000-40'ffff       None
	// 41'0000-41'ffff *     Text video memory & 6845 (mirror of 51'0000)
	// 42'0000-42'ffff *     Keyboard UPI
	// 43'0000-43'ffff *     Graphic video memory (Mirror of 53'0000)
	// 44'0000-46'ffff       None
	// 47'0000-47'ffff *     HPIB
	// 48'0000-50'ffff       None
	// 51'0000-51'ffff *     Text video memory & 6845
	// 52'0000-52'ffff *     Keyboard UPI (mirror of 42'0000)
	// 53'0000-53'ffff *     Graphic video memory
	// 54'0000-56'ffff       None
	// 57'0000-57'ffff *     HPIB (mirror of 47'0000)
	// 58'0000-68'ffff       None
	// 69'0000-69'ffff *     UART
	// 6a'0000-fb'ffff       None
	// fx'0000-ff'ffff *     RAM
	map(0x000000, 0x00ffff).rom().region("cpu", 0);
	map(0x010000, 0x01ffff).w(FUNC(hp9816_state::diag_led_w)).umask16(0x00ff);
	map(0x020000, 0x3fffff).noprw();
	map(0x400000, 0x40ffff).rw(m_cpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
	map(0x410000, 0x41ffff).mirror(0x100000).rw(FUNC(hp9816_state::text_r), FUNC(hp9816_state::text_w));
	map(0x420000, 0x42ffff).mirror(0x100000).mask(3).rw(m_upi, FUNC(hp98x6_upi_device::read), FUNC(hp98x6_upi_device::write)).umask16(0x00ff);
	map(0x430000, 0x43ffff).mirror(0x100000).rw(FUNC(hp9816_state::graphic_r), FUNC(hp9816_state::graphic_w));
	map(0x440000, 0x46ffff).rw(m_cpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
	map(0x470000, 0x47ffff).mirror(0x100000).rw(FUNC(hp9816_state::hpib_r), FUNC(hp9816_state::hpib_w)).umask16(0x00ff);
	map(0x480000, 0x50ffff).rw(m_cpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
	map(0x540000, 0x56ffff).rw(m_cpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
	map(0x580000, 0x68ffff).rw(m_cpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
	map(0x690000, 0x69ffff).rw(FUNC(hp9816_state::uart_r), FUNC(hp9816_state::uart_w)).umask16(0x00ff);
	map(0x6a0000, 0xfbffff).rw(m_cpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
}

void hp9816_state::diag_led_w(uint8_t data)
{
	LOG("LEDS=%02x %c%c%c%c %c%c%c%c\n" ,
		data,
		BIT(data , 7) ? '.' : '*',
		BIT(data , 6) ? '.' : '*',
		BIT(data , 5) ? '.' : '*',
		BIT(data , 4) ? '.' : '*',
		BIT(data , 3) ? '.' : '*',
		BIT(data , 2) ? '.' : '*',
		BIT(data , 1) ? '.' : '*',
		BIT(data , 0) ? '.' : '*');
}

uint16_t hp9816_state::text_r(offs_t offset)
{
	if (BIT(offset, 12)) {
		return m_text_vram[offset & (TEXT_VRAM_SIZE - 1)];
	} else if (BIT(offset, 0)) {
		return m_crtc->register_r();
	} else {
		LOG("Reading from non-existing register of 6845!\n");
		return 0;
	}
}

void hp9816_state::text_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (BIT(offset, 12)) {
		m_hsync_en = !BIT(offset, 14);
		COMBINE_DATA(&m_text_vram[offset & (TEXT_VRAM_SIZE - 1)]);
	} else if (ACCESSING_BITS_0_7) {
		if (BIT(offset, 0)) {
			m_crtc->register_w(uint8_t(data));
		} else {
			m_crtc->address_w(uint8_t(data));
		}
	} else {
		LOG("Access to 6845 on top byte!\n");
	}
}

uint16_t hp9816_state::graphic_r(offs_t offset, uint16_t mem_mask)
{
	m_graphic_en = !BIT(offset, 14);

	if (ACCESSING_BITS_0_7) {
		return m_graphic_vram[offset & (GRAPHIC_VRAM_SIZE - 1)];
	} else {
		return m_cpu->berr_r();
	}
}

void hp9816_state::graphic_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_graphic_en = !BIT(offset, 14);

	if (ACCESSING_BITS_0_7) {
		m_graphic_vram[offset & (GRAPHIC_VRAM_SIZE - 1)] = uint8_t(data);
	} else {
		return m_cpu->berr_w(0);
	}
}

uint8_t hp9816_state::hpib_r(offs_t offset)
{
	uint8_t res = 0;

	if (BIT(offset, 3)) {
		res = m_hpib->read(offset & 7);
	} else if (BIT(offset, 1)) {
		if (BIT(m_sw3_dipswitches->read(), 6)) {
			BIT_SET(res, 7);
		}
		if (!m_hpib->cont_r()) {
			BIT_SET(res, 6);
		}
		if (m_upi_irq7) {
			BIT_SET(res, 2);
		}
		if (BIT(m_sw3_dipswitches->read(), 7)) {
			BIT_SET(res, 0);
		}
	} else {
		if (BIT(m_sw3_dipswitches->read(), 5)) {
			BIT_SET(res, 7);
		}
		if (m_hpib_irq) {
			BIT_SET(res, 6);
		}
		BIT_SET(res, 2);
		if (m_hpib_dma_en) {
			BIT_SET(res, 0);
		}
	}

	return res;
}

void hp9816_state::hpib_w(offs_t offset, uint8_t data)
{
	if (BIT(offset, 3)) {
		m_hpib->write(offset & 7, data);
	} else {
		m_hpib_dma_en = BIT(data, 0);
	}
}

void hp9816_state::uart_reset()
{
	m_uart_int_en = false;
	m_uart_ocd4 = false;
	m_uart_ocd3 = false;
	m_uart->reset();
	uart_update_irq();
}

void hp9816_state::uart_update_irq()
{
	m_cpu->set_input_line(M68K_IRQ_4, m_uart_irq && m_uart_int_en);
}

uint8_t hp9816_state::uart_r(offs_t offset)
{
	uint8_t res = 0;

	if (BIT(offset, 3)) {
		res = m_uart->ins8250_r(offset & 7);
	} else {
		switch (offset & 3) {
		case 0:
			if (BIT(m_sw3_dipswitches->read(), 4)) {
				BIT_SET(res, 7);
			}
			BIT_SET(res, 1);
			break;
		case 1:
			if (m_uart_int_en) {
				BIT_SET(res, 7);
			}
			if (m_uart_irq) {
				BIT_SET(res, 6);
			}
			BIT_SET(res, 4);
			break;
		case 2:
			if (m_uart_ocd3) {
				BIT_SET(res, 7);
			}
			if (m_uart_ocd4) {
				BIT_SET(res, 6);
			}
			res |= (m_sw3_dipswitches->read() & 0x0f);
			break;
		case 3:
			res = uint8_t(m_sw2_dipswitches->read());
			break;
		}
	}

	return res;
}

void hp9816_state::uart_w(offs_t offset, uint8_t data)
{
	if (BIT(offset, 3)) {
		m_uart->ins8250_w(offset & 7, data);
	} else {
		switch (offset & 3) {
		case 0:
			uart_reset();
			break;
		case 1:
			m_uart_int_en = BIT(data, 7);
			uart_update_irq();
			break;
		case 2:
			m_uart_ocd3 = BIT(data, 7);
			m_uart_ocd4 = BIT(data, 6);
			break;
		}
	}
}

MC6845_UPDATE_ROW(hp9816_state::crtc_update_row)
{
	if (m_hsync_en) {
		const pen_t *pen = m_palette->pens();
		bool clen = (ma & 0x3800) != 0x2800;
		bool glb_blink = (m_screen->frame_number() & 0x30) == 0;

		// Text video
		for (int i = 0; i < x_count; i++) {
			uint16_t char_attr = m_text_vram[(ma + i) & (TEXT_VRAM_SIZE - 1)];
			// | Bit(s) | Meaning                    |
			// |--------+----------------------------|
			// |     15 | Select alternate char. set |
			// | 14..12 | N/U                        |
			// |     11 | Half brightness            |
			// |     10 | Underline                  |
			// |      9 | Blink                      |
			// |      8 | Inverted                   |
			// |   7..0 | Character code             |

			bool half_bright = BIT(char_attr, 11);
			bool underline;
			bool blink = glb_blink && BIT(char_attr, 9);
			bool invert;
			uint16_t char_pixels;
			bool cursor;
			if (clen) {
				unsigned chargen_addr = ra | ((char_attr & 0xff) << 4);
				if (BIT(char_attr, 15)) {
					BIT_SET(chargen_addr, 12);
				}
				char_pixels = uint16_t(m_chargen[chargen_addr]) << 1;
				underline = BIT(char_attr, 10) && ra == 11;
				invert = BIT(char_attr, 8);
				cursor = cursor_x == i;
			} else {
				char_pixels = 0;
				underline = false;
				invert = false;
				cursor = false;
			}

			// Truth table of 1st video combiner
			//
			// | blink | cursor | underline | out      |
			// |-------+--------+-----------+----------|
			// |     0 |      0 |         0 | chargen  |
			// |     0 |      0 |         1 | ~chargen |
			// |     0 |      1 |         0 | ~chargen |
			// |     0 |      1 |         1 | chargen  |
			// |     1 |      0 |         0 | 0        |
			// |     1 |      0 |         1 | 0        |
			// |     1 |      1 |         0 | ~0       |
			// |     1 |      1 |         1 | ~0       |
			if (blink) {
				if (cursor) {
					char_pixels = ~0;
				} else {
					char_pixels = 0;
				}
			} else if (cursor ^ underline) {
				char_pixels = ~char_pixels;
			}

			// Truth table of 2nd video combiner
			//
			// | half_bright | Graphic | invert | Text | out |
			// |-------------+---------+--------+------+-----|
			// |           0 |       0 |      0 |    0 | 0   |
			// |           0 |       0 |      0 |    1 | 2   |
			// |           0 |       0 |      1 |    0 | 2   |
			// |           0 |       0 |      1 |    1 | 0   |
			// |           0 |       1 |      0 |    0 | 2   |
			// |           0 |       1 |      0 |    1 | 0   |
			// |           0 |       1 |      1 |    0 | 0   |
			// |           0 |       1 |      1 |    1 | 2   |
			// |           1 |       0 |      0 |    0 | 0   |
			// |           1 |       0 |      0 |    1 | 1   |
			// |           1 |       0 |      1 |    0 | 1   |
			// |           1 |       0 |      1 |    1 | 0   |
			// |           1 |       1 |      0 |    0 | 2   |
			// |           1 |       1 |      0 |    1 | 2   |
			// |           1 |       1 |      1 |    0 | 2   |
			// |           1 |       1 |      1 |    1 | 2   |
			if (invert) {
				char_pixels = ~char_pixels;
			}
			for (unsigned col = 0; col < 10; col++) {
				bitmap.pix(y, i * 10 + col) = pen[BIT(char_pixels, 9 - col) ? (half_bright ? 1 : 2) : 0];
			}
		}

		// Graphic video
		if (m_graphic_en) {
			unsigned g_cols = unsigned(x_count) * 5;
			// Hw only works correctly when x_count is 80 (-> 50 bytes per graphic line)
			unsigned g_bytes_per_line = g_cols / 8;
			unsigned g_addr = y * g_bytes_per_line;
			for (unsigned i = 0; i < g_bytes_per_line; i++) {
				uint8_t graph_pixels = m_graphic_vram[(g_addr + i) & (GRAPHIC_VRAM_SIZE - 1)];
				for (unsigned col = 0; col < 8; col++) {
					if (BIT(graph_pixels, 7 - col)) {
						// A graphic pixel covers 2 text pixels
						// Also, the hw seems to offset graphic pixels 1 text pixel to the right
						// Here this offset is not applied (as rightmost graphic pixel would be clipped)
						unsigned x_pos = i * 16 + col * 2;
						{
							auto &pix = bitmap.pix(y, x_pos);
							// Overlay rules (see 2nd video combiner table)
							//
							// |  Text | Combined |
							// | video | Video    |
							// |-------+----------|
							// |     0 | 2        |
							// |     1 | 2        |
							// |     2 | 0        |
							pix = pen[(pix == pen[2]) ? 0 : 2];
						}
						x_pos++;
						{
							auto &pix = bitmap.pix(y, x_pos);
							pix = pen[(pix == pen[2]) ? 0 : 2];
						}
					}
				}
			}
		}
	} else {
		bitmap.fill(rgb_t::black(), rectangle(0, unsigned(x_count) * 10 - 1, y, y));
	}
}

void hp9816_state::cpu_reset_w(int state)
{
	if (state) {
		LOG("RESET from CPU\n");
		m_upi->reset();
		m_hpib->reset();
		m_hpib_dma_en = false;
		uart_reset();
		//m_crtc->reset();
		m_hsync_en = false;
		m_graphic_en = false;
	}
}

void hp9816_state::hpib_irq_w(int state)
{
	m_hpib_irq = bool(state);
	m_cpu->set_input_line(M68K_IRQ_3, state);
}

void hp9816_state::upi_irq7_w(int state)
{
	m_upi_irq7 = bool(state);
	m_cpu->set_input_line(M68K_IRQ_7, state);
}

void hp9816_state::uart_irq_w(int state)
{
	m_uart_irq = bool(state);
	uart_update_irq();
}

static INPUT_PORTS_START(hp9816)
	PORT_START("sw2")
	PORT_DIPNAME(0x30, 0x00, "Parity type")
	PORT_DIPLOCATION("S2:4,3")
	PORT_DIPSETTING(0x00, "Odd")
	PORT_DIPSETTING(0x10, "Even")
	PORT_DIPSETTING(0x20, "One")
	PORT_DIPSETTING(0x30, "Zero")
	PORT_DIPNAME(0x08, 0x00, "Parity enable")
	PORT_DIPLOCATION("S2:5")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x08, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x00, "Stop bits")
	PORT_DIPLOCATION("S2:6")
	PORT_DIPSETTING(0x00, "1")
	PORT_DIPSETTING(0x04, "2")
	PORT_DIPNAME(0x03, 0x03, "Bits/char")
	PORT_DIPLOCATION("S2:8,7")
	PORT_DIPSETTING(0x00, "5")
	PORT_DIPSETTING(0x01, "6")
	PORT_DIPSETTING(0x02, "7")
	PORT_DIPSETTING(0x03, "8")

	PORT_START("sw3")
	PORT_DIPNAME(0x0f, 0x0a, "Baud rate")
	PORT_DIPLOCATION("S3:4,3,2,1")
	PORT_DIPSETTING(0x00, "50")
	PORT_DIPSETTING(0x01, "75")
	PORT_DIPSETTING(0x02, "110")
	PORT_DIPSETTING(0x03, "134.5")
	PORT_DIPSETTING(0x04, "150")
	PORT_DIPSETTING(0x05, "200")
	PORT_DIPSETTING(0x06, "300")
	PORT_DIPSETTING(0x07, "600")
	PORT_DIPSETTING(0x08, "1200")
	PORT_DIPSETTING(0x09, "1800")
	PORT_DIPSETTING(0x0a, "2400")
	PORT_DIPSETTING(0x0b, "3600")
	PORT_DIPSETTING(0x0c, "4800")
	PORT_DIPSETTING(0x0d, "7200")
	PORT_DIPSETTING(0x0e, "9600")
	PORT_DIPSETTING(0x0f, "19200")
	PORT_DIPNAME(0x10, 0x00, "Remote keyboard")
	PORT_DIPLOCATION("S3:5")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x10, DEF_STR(On))
	PORT_DIPNAME(0x20, 0x20, "Continuous self-test")
	PORT_DIPLOCATION("S3:6")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPNAME(0x40, 0x40, "HPIB Sys controller")
	PORT_DIPLOCATION("S3:7")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x40, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, "CRT refresh rate")
	PORT_DIPLOCATION("S3:8")
	PORT_DIPSETTING(0x00, "50 Hz")
	PORT_DIPSETTING(0x80, "60 Hz")

INPUT_PORTS_END

ROM_START(hp9816a)
	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD("1818-3171.bin", 0, 0x2000, CRC(9cf4d4e1) SHA1(228484fd7ba7a4b59a051af083858383afe30179))

	ROM_REGION(0x10000, "cpu", 0)
	ROM_DEFAULT_BIOS("bios40")
	ROM_SYSTEM_BIOS(0, "bios40",  "BIOS v4.0")
	ROMX_LOAD("rom40.bin", 0x0000, 0x10000, CRC(36005480) SHA1(645a077ffd95e4c31f05cd8bbd6e4554b12813f1), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "bios30",  "BIOS v3.0")
	ROMX_LOAD("rom30.bin", 0x0000, 0x10000, CRC(05c07e75) SHA1(3066a65e6137482041f9a77d09ee2289fe0974aa), ROM_BIOS(1))
ROM_END

} // anonymous namespace

//   YEAR  NAME     PARENT  COMPAT  MACHINE INPUT   CLASS        INIT        COMPANY            FULLNAME    FLAGS
COMP(1982, hp9816a, 0,      0,      hp9816, hp9816, hp9816_state,empty_init, "Hewlett-Packard", "HP 9816A", 0)

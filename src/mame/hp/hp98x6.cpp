// license:BSD-3-Clause
// copyright-holders:F. Ulivi

// **************************
// Driver for HP 98x6 systems
// **************************
//
// What's in the emulated systems:
//
// |                                Model | 9 | 9 | 9 | 9 |
// |                                      | 8 | 8 | 8 | 8 |
// |                                      | 1 | 2 | 3 | 3 |
// |                                      | 6 | 6 | 6 | 6 |
// | Feature                              | A | A | A | C |
// |--------------------------------------+---+---+---+---|
// | 8MHz M68000 CPU                      | * | * | * | * |
// | Boot ROM                             | * | * | * | * |
// | Variable size RAM                    | * | * | * | * |
// | HPIB interface                       | * | * | * | * |
// | HLE of 8041 UPI                      | * | * | * | * |
// | Large keyboard (98203B)              | * | * | * | * |
// | Knob                                 | * | * | * | * |
// | Beeper                               | * | * | * | * |
// | ID PROM                              | * | * | * | * |
// | Option ROMs                          | * | * | * | * |
// | B/W 80x25 text video w/ attributes   | * |   | * |   |
// | B/W 50x25 text video w/ attributes   |   | * |   |   |
// | B/W 400x300 graphic video            | * | * |   |   |
// | B/W 512x390 graphic video            |   |   | * |   |
// | Color 80x25 text video w/ attributes |   |   |   | * |
// | Color 512x390 graphic video          |   |   |   | * |
// | Correct char. generator              | * | * | N | N |
// | 5.25 floppy drives                   |   | 1 | 2 | 2 |
// | RS232 interface                      | * |   |   |   |
//
// What's not in for 9836A/C models:
// - Correct character generator
//
// What's not in for all the models:
// - Expansion cards
//
// Main references:
// - Olivier De Smet's standalone emulator:
//   https://sites.google.com/site/olivier2smet2/hp_projects/hp98x6
// - Tony Duell's reverse engineered schematics (available @ hpmuseum.net)
// - HP 98615-90074, Feb 85, Pascal 3.0 System's Designer Guide (available @ hpmuseum.net)

#include "emu.h"

#include "hp98x6_optrom.h"
#include "hp98x6_upi.h"

#include "bus/ieee488/ieee488.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "imagedev/floppy.h"
#include "machine/74123.h"
#include "machine/ins8250.h"
#include "machine/ram.h"
#include "machine/rescap.h"
#include "machine/tms9914.h"
#include "machine/wd_fdc.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"

// Debugging
#define LOG_FDC_MASK    (LOG_GENERAL << 1)
#define LOG_FDC(...)    LOGMASKED(LOG_FDC_MASK, __VA_ARGS__)
#define LOG_FDC_DATA_MASK   (LOG_FDC_MASK << 1)
#define LOG_FDC_DATA(...)   LOGMASKED(LOG_FDC_DATA_MASK, __VA_ARGS__)
#undef VERBOSE
#define VERBOSE 0
#include "logmacro.h"

namespace {

// CPU clock
constexpr auto CPU_CLOCK = 16_MHz_XTAL / 2;

// 9816: video dot clock
constexpr unsigned DOT_CLOCK_9816 = 19'988'000;

// 9826: video dot clock
constexpr auto DOT_CLOCK_9826 = 10_MHz_XTAL;

// 9836(c): video dot clock
constexpr unsigned DOT_CLOCK_9836 = 25'771'500;

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

// +-------------------+
// | hp98x6_base_state |
// +-------------------+
//         ↑
//         +-------------------+
//         ↑                   ↑
// +-----------------+  +--------------+
// | hp9826_36_state |  | hp9816_state |
// +-----------------+  +--------------+
//         ↑
//         +----------------+------------------+
//         ↑                ↑                  ↑
// +--------------+  +--------------+  +---------------+
// | hp9826_state |  | hp9836_state |  | hp9836c_state |
// +--------------+  +--------------+  +---------------+
class hp98x6_base_state : public driver_device
{
protected:
	hp98x6_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_ram(*this, RAM_TAG)
		, m_crtc(*this, "crtc")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_upi(*this, "upi")
		, m_hpib(*this, "hpib")
		, m_chargen(*this, "chargen")
		, m_rom_drawers(*this, "drawer%u", 0U)
	{
	}

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void hp98x6_base(machine_config &mconfig, unsigned dot_clock, int char_width);
	virtual void cpu_mem_map(address_map &map) ATTR_COLD;
	void diag_led_w(uint8_t data);
	virtual void cpu_reset_w(int state);
	void hpib_irq_w(int state);
	void upi_irq7_w(int state);

	required_device<m68000_device> m_cpu;
	required_device<ram_device> m_ram;
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<hp98x6_upi_device> m_upi;
	required_device<tms9914_device> m_hpib;

	// Character generator
	required_region_ptr<uint8_t> m_chargen;

	required_device_array<hp98x6_optrom_device, 2> m_rom_drawers;

	bool m_hsync_en;
	bool m_graphic_en;
	bool m_hpib_irq;
	bool m_hpib_dma_en;
	bool m_upi_irq7;
};

void hp98x6_base_state::machine_start()
{
	save_item(NAME(m_hsync_en));
	save_item(NAME(m_graphic_en));
	save_item(NAME(m_hpib_irq));
	save_item(NAME(m_hpib_dma_en));
	save_item(NAME(m_upi_irq7));

	auto space = &m_cpu->space(AS_PROGRAM);

	space->install_ram(0x1000000 - m_ram->size(), 0xffffff, m_ram->pointer());

	for (auto &finder : m_rom_drawers) {
		finder->install_handlers(space);
	}
}

void hp98x6_base_state::machine_reset()
{
	m_hsync_en = false;
	m_graphic_en = false;
	m_hpib_dma_en = false;
	diag_led_w(0);
}

void hp98x6_base_state::hp98x6_base(machine_config &config, unsigned dot_clock, int char_width)
{
	M68000(config, m_cpu, CPU_CLOCK);
	m_cpu->set_addrmap(AS_PROGRAM, &hp98x6_base_state::cpu_mem_map);
	m_cpu->reset_cb().set(FUNC(hp98x6_base_state::cpu_reset_w));

	RAM(config, m_ram).set_default_size("256K").set_extra_options("512K,1M");

	MC6845(config, m_crtc, dot_clock / char_width);
	m_crtc->set_char_width(char_width);
	m_crtc->set_screen(m_screen);
	m_crtc->set_show_border_area(false);

	HP98X6_UPI(config, m_upi, HPIB_CLOCK);
	m_upi->irq1_write_cb().set_inputline(m_cpu, M68K_IRQ_1);
	m_upi->irq7_write_cb().set(FUNC(hp98x6_base_state::upi_irq7_w));

	TMS9914(config, m_hpib, HPIB_CLOCK);
	m_hpib->int_write_cb().set(FUNC(hp98x6_base_state::hpib_irq_w));
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
	IEEE488_SLOT(config, "ieee_dev", 0, hp_ieee488_devices, nullptr);
	IEEE488_SLOT(config, "ieee_rem", 0, remote488_devices, nullptr);

	// Optional ROM slots
	for (auto &finder : m_rom_drawers) {
		HP98X6_OPTROM(config, finder);
	}

	SOFTWARE_LIST(config, "optrom_list").set_original("hp98x6_rom");
}

void hp98x6_base_state::cpu_mem_map(address_map &map)
{
	map.unmap_value_high();
	// Range           DTACK Device
	// ==============================
	// 00'0000-00'ffff *     Boot ROM
	// 01'0000-01'ffff *     Diagnostic LEDs (write only)
	// 02'0000-3f'ffff *     None
	// fx'0000-ff'ffff *     RAM
	map(0x000000, 0x00ffff).rom().region("cpu", 0);
	map(0x010000, 0x01ffff).w(FUNC(hp98x6_base_state::diag_led_w)).umask16(0x00ff);
	map(0x020000, 0x3fffff).noprw();
}

void hp98x6_base_state::diag_led_w(uint8_t data)
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

void hp98x6_base_state::cpu_reset_w(int state)
{
	if (state) {
		LOG("RESET from CPU\n");
		m_upi->reset();
		m_hpib->reset();
		m_hpib_dma_en = false;
		//m_crtc->reset();
		m_hsync_en = false;
		m_graphic_en = false;
	}
}

void hp98x6_base_state::hpib_irq_w(int state)
{
	m_hpib_irq = bool(state);
	m_cpu->set_input_line(M68K_IRQ_3, state);
}

void hp98x6_base_state::upi_irq7_w(int state)
{
	m_upi_irq7 = bool(state);
	m_cpu->set_input_line(M68K_IRQ_7, state);
}

// +--------------+
// | hp9816_state |
// +--------------+
class hp9816_state : public hp98x6_base_state
{
public:
	hp9816_state(const machine_config &mconfig, device_type type, const char *tag)
		: hp98x6_base_state(mconfig, type, tag)
		, m_uart(*this, "uart")
		, m_rs232(*this, "rs232")
		, m_sw2_dipswitches(*this, "sw2")
		, m_sw3_dipswitches(*this, "sw3")
	{
	}

	void hp9816(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<ins8250_device> m_uart;
	required_device<rs232_port_device> m_rs232;
	required_ioport m_sw2_dipswitches;
	required_ioport m_sw3_dipswitches;

private:
	static inline constexpr unsigned TEXT_VRAM_SIZE = 2048;
	static inline constexpr unsigned GRAPHIC_VRAM_SIZE = 16384;

	virtual void cpu_mem_map(address_map &map) override ATTR_COLD;
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

	virtual void cpu_reset_w(int state) override;
	void uart_irq_w(int state);

	uint16_t m_text_vram[TEXT_VRAM_SIZE];
	uint8_t m_graphic_vram[GRAPHIC_VRAM_SIZE];
	bool m_uart_int_en;
	bool m_uart_irq;
	bool m_uart_ocd3;
	bool m_uart_ocd4;
};

void hp9816_state::hp9816(machine_config &config)
{
	hp98x6_base_state::hp98x6_base(config, DOT_CLOCK_9816, 10);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER, rgb_t::white());
	// Parameters for 50 Hz frame rate
	m_screen->set_raw(DOT_CLOCK_9816, 1040, 0, 800, 384, 0, 300);
	m_screen->set_screen_update(m_crtc, FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);

	m_crtc->set_update_row_callback(FUNC(hp9816_state::crtc_update_row));

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
	hp98x6_base_state::machine_start();

	save_item(NAME(m_text_vram));
	save_item(NAME(m_graphic_vram));
	save_item(NAME(m_uart_int_en));
	save_item(NAME(m_uart_ocd3));
	save_item(NAME(m_uart_ocd4));
}

void hp9816_state::machine_reset()
{
	hp98x6_base_state::machine_reset();
	uart_reset();
}

void hp9816_state::cpu_mem_map(address_map &map)
{
	hp98x6_base_state::cpu_mem_map(map);

	// Range           DTACK Device
	// ==============================
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
	hp98x6_base_state::cpu_reset_w(state);

	if (state) {
		uart_reset();
	}
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

	ROM_REGION(0x100, "upi:idprom", 0)
	ROM_LOAD("prom9816.bin", 0, 0x100, CRC(f2e57a04) SHA1(7436c1334f30e6de3cf1a7c30f714c9f203cb4d3))

	ROM_REGION(0x10000, "cpu", 0)
	ROM_DEFAULT_BIOS("bios40")
	ROM_SYSTEM_BIOS(0, "bios40",  "BIOS v4.0")
	ROMX_LOAD("rom40.bin", 0x0000, 0x10000, CRC(36005480) SHA1(645a077ffd95e4c31f05cd8bbd6e4554b12813f1), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "bios30",  "BIOS v3.0")
	ROMX_LOAD("rom30.bin", 0x0000, 0x10000, CRC(05c07e75) SHA1(3066a65e6137482041f9a77d09ee2289fe0974aa), ROM_BIOS(1))
ROM_END

// +-----------------+
// | hp9826_36_state |
// +-----------------+
class hp9826_36_state : public hp98x6_base_state
{
public:
	hp9826_36_state(const machine_config &mconfig, device_type type, const char *tag)
		: hp98x6_base_state(mconfig, type, tag)
		, m_fdc(*this, "fdc")
		, m_drive0(*this, "drive0")
		, m_drive1(*this, "drive1")
		, m_ss(*this, "ss%u", 0)
		, m_fdc_timer(*this, "fdc_tmr")
		, m_sw1(*this, "sw1")
		, m_sys_ctrl_sw(*this, "sys_ctrl_sw")
		, m_idprom(*this, "idprom")
	{
	}

protected:
	// Bits in m_floppy_ctrl
	static inline constexpr unsigned FC_DRIVE_SEL_BIT = 6;
	static inline constexpr unsigned FC_SEC_BUFF_EN_BIT = 5;
	static inline constexpr unsigned FC_SEC_BUFF_DIR_BIT = 4;
	static inline constexpr unsigned FC_FDC_RST_BIT = 3;
	static inline constexpr unsigned FC_SIDE_SEL_BIT = 2;
	static inline constexpr unsigned FC_DRIVE_EN_BIT = 0;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void device_post_load() override;

	TIMER_DEVICE_CALLBACK_MEMBER(fdc_ram_io);

	void hp9826_36(machine_config &mconfig, unsigned dot_clock, int char_width);
	virtual void cpu_mem_map(address_map &map) override ATTR_COLD;
	virtual void cpu_reset_w(int state) override;
	unsigned get_sel_floppy() const;
	floppy_image_device *get_drive(unsigned idx) const;
	unsigned get_floppy_idx(floppy_image_device *floppy) const;
	void floppy_reset();
	void floppy_update_sel();
	void floppy_update_mon_hlt();
	void floppy_wp_cb(floppy_image_device *floppy, int state);
	void floppy_index_cb(floppy_image_device *floppy, int state);
	void fdc_hld_w(int state);
	uint8_t floppy_r(offs_t offset);
	void floppy_w(offs_t offset, uint8_t data);
	void fdc_irq_w(int state);
	void fdc_drq_w(int state);
	uint8_t hpib_r(offs_t offset);
	void hpib_w(offs_t offset, uint8_t data);
	uint16_t id_prom_r(offs_t offset);

	required_device<fd1793_device> m_fdc;
	required_device<floppy_connector> m_drive0;
	optional_device<floppy_connector> m_drive1;
	required_device_array<ttl74123_device, 2> m_ss;
	required_device<timer_device> m_fdc_timer;
	required_ioport m_sw1;
	required_ioport m_sys_ctrl_sw;

	// ID PROM
	required_region_ptr<uint8_t> m_idprom;

	uint8_t m_floppy_ctrl;
	uint8_t m_floppy_buffer[256];
	uint8_t m_buffer_addr;
	bool m_fdc_hld;
	bool m_fdc_irq;
	bool m_fdc_drq;
	bool m_drive_changed[2];
	floppy_image_device *m_curr_floppy;
};

void hp9826_36_state::machine_start()
{
	hp98x6_base_state::machine_start();

	save_item(NAME(m_floppy_ctrl));
	save_item(NAME(m_floppy_buffer));
	save_item(NAME(m_buffer_addr));
	save_item(NAME(m_fdc_hld));
	save_item(NAME(m_fdc_irq));
	save_item(NAME(m_fdc_drq));
	save_item(NAME(m_drive_changed));

	auto drive0 = get_drive(0);
	LOG_FDC("drive0 = %p\n", drive0);
	drive0->setup_wpt_cb(floppy_image_device::wpt_cb(&hp9826_36_state::floppy_wp_cb, this));
	drive0->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(&hp9826_36_state::floppy_index_cb, this));
	auto drive1 = get_drive(1);
	LOG_FDC("drive1 = %p\n", drive1);
	if (drive1) {
		drive1->setup_wpt_cb(floppy_image_device::wpt_cb(&hp9826_36_state::floppy_wp_cb, this));
		drive1->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(&hp9826_36_state::floppy_index_cb, this));
	}
}

void hp9826_36_state::machine_reset()
{
	hp98x6_base_state::machine_reset();
	m_curr_floppy = nullptr;
	floppy_reset();
}

void hp9826_36_state::device_post_load()
{
	hp98x6_base_state::device_post_load();

	m_curr_floppy = nullptr;
	// Bring m_curr_floppy in synch
	floppy_update_sel();
}

static void floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

void hp9826_36_state::hp9826_36(machine_config &mconfig, unsigned dot_clock, int char_width)
{
	hp98x6_base(mconfig, dot_clock, char_width);

	FD1793(mconfig, m_fdc, HPIB_CLOCK / 5);
	m_fdc->set_force_ready(true);
	m_fdc->dden_w(0);
	m_fdc->intrq_wr_callback().set(FUNC(hp9826_36_state::fdc_irq_w));
	m_fdc->drq_wr_callback().set(FUNC(hp9826_36_state::fdc_drq_w));
	m_fdc->hld_wr_callback().set(FUNC(hp9826_36_state::fdc_hld_w));

	FLOPPY_CONNECTOR(mconfig, m_drive0, floppies, "525dd", floppy_image_device::default_mfm_floppy_formats, true);

	// ~225 ms duration
	TTL74123(mconfig, m_ss[0], RES_K(23.7), CAP_U(33));
	m_ss[0]->set_connection_type(TTL74123_NOT_GROUNDED_NO_DIODE);
	m_ss[0]->set_a_pin_value(0);
	m_ss[0]->set_clear_pin_value(1);
	TTL74123(mconfig, m_ss[1], RES_K(23.7), CAP_U(33));
	m_ss[1]->set_connection_type(TTL74123_NOT_GROUNDED_NO_DIODE);
	m_ss[1]->set_a_pin_value(0);
	m_ss[1]->set_clear_pin_value(1);

	TIMER(mconfig, m_fdc_timer).configure_generic(FUNC(hp9826_36_state::fdc_ram_io));
}

void hp9826_36_state::cpu_mem_map(address_map &map)
{
	hp98x6_base_state::cpu_mem_map(map);

	// Range           DTACK Device
	// ==============================
	// 40'0000-41'ffff *     None
	// 42'0000-42'ffff *     Keyboard UPI
	// 43'0000-43'ffff *     None
	// 44'0000-44'ffff *     Floppy
	// 45'0000-46'ffff *     None
	// 47'0000-47'ffff *     HPIB
	// 48'0000-4f'ffff *     None
	// 50'0000-5e'ffff       None
	// 5f'0000-5f'3fff *     ID PROM & DIP switches
	// 5f'4000-fb'ffff       None
	map(0x400000, 0x41ffff).noprw();
	map(0x420000, 0x42ffff).mask(3).rw(m_upi, FUNC(hp98x6_upi_device::read), FUNC(hp98x6_upi_device::write)).umask16(0x00ff);
	map(0x430000, 0x43ffff).noprw();
	map(0x440000, 0x44ffff).rw(FUNC(hp9826_36_state::floppy_r), FUNC(hp9826_36_state::floppy_w)).umask16(0xff00);
	map(0x450000, 0x46ffff).noprw();
	map(0x470000, 0x47ffff).rw(FUNC(hp9826_36_state::hpib_r), FUNC(hp9826_36_state::hpib_w)).umask16(0x00ff);
	map(0x480000, 0x4fffff).noprw();
	map(0x500000, 0x5effff).rw(m_cpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
	map(0x5f0000, 0x5f3fff).nopw().r(FUNC(hp9826_36_state::id_prom_r));
	map(0x5f4000, 0xfbffff).rw(m_cpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
}

void hp9826_36_state::cpu_reset_w(int state)
{
	hp98x6_base_state::cpu_reset_w(state);

	if (state) {
		floppy_reset();
	}
}

unsigned hp9826_36_state::get_sel_floppy() const
{
	return BIT(m_floppy_ctrl, FC_DRIVE_SEL_BIT) ? 1 : 0;
}

floppy_image_device *hp9826_36_state::get_drive(unsigned idx) const
{
	if (idx) {
		return bool(m_drive1) ? m_drive1->get_device() : nullptr;
	} else {
		return m_drive0->get_device();
	}
}

unsigned hp9826_36_state::get_floppy_idx(floppy_image_device *floppy) const
{
	return get_drive(0) == floppy ? 0 : 1;
}

void hp9826_36_state::floppy_reset()
{
	m_floppy_ctrl = 0;
	m_fdc->mr_w(0);
	floppy_update_sel();
}

void hp9826_36_state::floppy_update_sel()
{
	floppy_image_device *new_floppy = get_drive(get_sel_floppy());

	if (new_floppy != m_curr_floppy) {
		m_curr_floppy = new_floppy;
		m_fdc->set_floppy(m_curr_floppy);
	}
	if (m_curr_floppy) {
		m_curr_floppy->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(&hp9826_36_state::floppy_index_cb, this));
		m_curr_floppy->ss_w(BIT(m_floppy_ctrl, FC_SIDE_SEL_BIT));
	}
	floppy_update_mon_hlt();
}

void hp9826_36_state::floppy_update_mon_hlt()
{
	bool ss_state = bool(m_ss[get_sel_floppy()]->q_r());
	bool mon = BIT(m_floppy_ctrl, FC_DRIVE_EN_BIT) || (m_fdc_hld && ss_state);
	LOG_FDC("mon %d: hld %d ss %d\n", mon, m_fdc_hld, ss_state);
	get_drive(0)->mon_w(!mon);
	auto drive1 = get_drive(1);
	if (drive1) {
		drive1->mon_w(!mon);
	}
}

void hp9826_36_state::floppy_wp_cb(floppy_image_device *floppy, int state)
{
	LOG_FDC("floppy wp %p %d\n", floppy, state);
	if (state) {
		m_drive_changed[get_floppy_idx(floppy)] = true;
	}
}

void hp9826_36_state::floppy_index_cb(floppy_image_device *floppy, int state)
{
	LOG_FDC("floppy id %s %p %d\n", machine().time().as_string(), floppy, state);
	m_ss[get_floppy_idx(floppy)]->b_w(state);
	floppy_update_mon_hlt();
	if (floppy == m_curr_floppy) {
		m_fdc->index_callback(floppy, state);
	}
}

void hp9826_36_state::fdc_hld_w(int state)
{
	m_fdc_hld = bool(state);
	floppy_update_mon_hlt();
}

uint8_t hp9826_36_state::floppy_r(offs_t offset)
{
	uint8_t res = 0;
	if ((offset & 0x1e00) == 0x0800) {
		// Status register
		if (m_fdc_irq) {
			BIT_SET(res, 3);
		}
		// Margin error = 0 (bit 2)
		if (m_drive_changed[get_sel_floppy()]) {
			BIT_SET(res, 1);
		}
		if (m_fdc_drq) {
			BIT_SET(res, 0);
		}
	} else if (!BIT(m_floppy_ctrl, FC_SEC_BUFF_EN_BIT)) {
		if (BIT(offset, 12)) {
			// Buffer RAM
			m_buffer_addr = offset & 0xff;
			res = m_floppy_buffer[m_buffer_addr];
		} else if ((offset & 0x1800) == 0) {
			// FDC
			res = m_fdc->read(offset & 3);
		}
	}
	LOG_FDC_DATA("fdc R %04x=%02x\n", offset * 2, res);
	return res;
}

void hp9826_36_state::floppy_w(offs_t offset, uint8_t data)
{
	LOG_FDC_DATA("fdc W %s %04x=%02x\n", machine().time().as_string(), offset * 2, data);
	offs_t masked_off = offset & 0x1e00;
	if (masked_off == 0x800) {
		// Floppy control register
		m_floppy_ctrl = data;
		m_fdc->mr_w(BIT(m_floppy_ctrl, FC_FDC_RST_BIT));
		floppy_update_sel();
	} else if (masked_off == 0xa00) {
		// Disk changed reset command
		m_drive_changed[0] = false;
		m_drive_changed[1] = false;
	} else if (!BIT(m_floppy_ctrl, FC_SEC_BUFF_EN_BIT)) {
		if (BIT(offset, 12)) {
			// Buffer RAM
			m_buffer_addr = offset & 0xff;
			m_floppy_buffer[m_buffer_addr] = data;
		} else if ((offset & 0x1800) == 0) {
			// FDC
			m_fdc->write(offset & 3, data);
		}
	}
}

void hp9826_36_state::fdc_irq_w(int state)
{
	LOG_FDC("fdc IRQ %d\n", state);
	m_fdc_irq = bool(state);
	m_cpu->set_input_line(M68K_IRQ_2, state);
}

void hp9826_36_state::fdc_drq_w(int state)
{
	LOG_FDC("fdc drq %d\n", state);
	if (!m_fdc_drq && state && BIT(m_floppy_ctrl, FC_SEC_BUFF_EN_BIT)) {
		m_fdc_timer->adjust(attotime::zero);
	}
	m_fdc_drq = bool(state);
}

TIMER_DEVICE_CALLBACK_MEMBER(hp9826_36_state::fdc_ram_io)
{
	if (BIT(m_floppy_ctrl, FC_SEC_BUFF_EN_BIT)) {
		if (BIT(m_floppy_ctrl, FC_SEC_BUFF_DIR_BIT)) {
			// FDC -> RAM
			uint8_t data = m_fdc->data_r();
			m_floppy_buffer[m_buffer_addr] = data;
			LOG_FDC_DATA("fdc RAM W @%02x=%02x\n", m_buffer_addr, data);
		} else {
			// RAM -> FDC
			m_fdc->data_w(m_floppy_buffer[m_buffer_addr]);
			LOG_FDC_DATA("fdc RAM R @%02x=%02x\n", m_buffer_addr, m_floppy_buffer[m_buffer_addr]);
		}
		m_buffer_addr++;
	}
}

uint8_t hp9826_36_state::hpib_r(offs_t offset)
{
	uint8_t res = 0;

	if (BIT(offset, 3)) {
		res = m_hpib->read(offset & 7);
	} else if (BIT(offset, 1)) {
		if (m_sys_ctrl_sw->read()) {
			BIT_SET(res, 7);
		}
		if (!m_hpib->cont_r()) {
			BIT_SET(res, 6);
		}
		if (m_upi_irq7) {
			BIT_SET(res, 2);
		}
		BIT_SET(res, 0);
	} else {
		BIT_SET(res, 7);
		if (m_hpib_irq) {
			BIT_SET(res, 6);
		}
		if (m_hpib_dma_en) {
			BIT_SET(res, 0);
		}
	}

	return res;
}

void hp9826_36_state::hpib_w(offs_t offset, uint8_t data)
{
	if (BIT(offset, 3)) {
		m_hpib->write(offset & 7, data);
	} else {
		m_hpib_dma_en = BIT(data, 0);
	}
}

uint16_t hp9826_36_state::id_prom_r(offs_t offset)
{
	uint16_t res;

	res = m_idprom[offset & 0xff];
	res |= (m_sw1->read() << 8);

	return res;
}

static INPUT_PORTS_START(hp9826_36)
	PORT_START("sw1")
	PORT_CONFNAME(0x80, 0x80, "SW1-8")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x80, DEF_STR(Off))
	PORT_CONFNAME(0x40, 0x40, "SW1-7")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x40, DEF_STR(Off))
	PORT_CONFNAME(0x20, 0x20, "SW1-6")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x20, DEF_STR(Off))
	PORT_CONFNAME(0x10, 0x00, "SW1-5")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x10, DEF_STR(Off))
	PORT_CONFNAME(0x08, 0x00, "SW1-4")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x08, DEF_STR(Off))
	PORT_CONFNAME(0x04, 0x00, "SW1-3")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x04, DEF_STR(Off))
	PORT_CONFNAME(0x02, 0x00, "SW1-2")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x02, DEF_STR(Off))
	PORT_CONFNAME(0x01, 0x00, "SW1-1")
	PORT_CONFSETTING(0x00, DEF_STR(On))
	PORT_CONFSETTING(0x01, DEF_STR(Off))

	PORT_START("sys_ctrl_sw")
	PORT_CONFNAME(1, 1, "HPIB Sys controller")
	PORT_CONFSETTING(1, DEF_STR(On))
	PORT_CONFSETTING(0, DEF_STR(Off))
INPUT_PORTS_END

// +--------------+
// | hp9826_state |
// +--------------+
class hp9826_state : public hp9826_36_state
{
public:
	hp9826_state(const machine_config &mconfig, device_type type, const char *tag)
		: hp9826_36_state(mconfig, type, tag)
	{
	}

	void hp9826(machine_config &mconfig);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	static inline constexpr unsigned TEXT_VRAM_SIZE = 2048;
	static inline constexpr unsigned GRAPHIC_VRAM_SIZE = 16384;

	virtual void cpu_mem_map(address_map &map) override ATTR_COLD;
	uint16_t text_r(offs_t offset, uint16_t mem_mask);
	void text_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t graphic_r(offs_t offset, uint16_t mem_mask);
	void graphic_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	MC6845_UPDATE_ROW(crtc_update_row);

	uint8_t m_text_vram[TEXT_VRAM_SIZE];
	uint8_t m_graphic_vram[GRAPHIC_VRAM_SIZE];
};

void hp9826_state::hp9826(machine_config &mconfig)
{
	hp9826_36(mconfig, DOT_CLOCK_9826.value(), 8);

	SCREEN(mconfig, m_screen, SCREEN_TYPE_RASTER, rgb_t::white());
	m_screen->set_raw(DOT_CLOCK_9826, 520, 0, 400, 321, 0, 300);
	m_screen->set_screen_update(m_crtc, FUNC(mc6845_device::screen_update));

	PALETTE(mconfig, m_palette, palette_device::MONOCHROME_HIGHLIGHT);

	m_crtc->set_update_row_callback(FUNC(hp9826_state::crtc_update_row));
}

void hp9826_state::machine_start()
{
	hp9826_36_state::machine_start();
	save_item(NAME(m_text_vram));
	save_item(NAME(m_graphic_vram));
}

void hp9826_state::cpu_mem_map(address_map &map)
{
	hp9826_36_state::cpu_mem_map(map);

	// Range           DTACK Device
	// ==============================
	// 41'0000-41'ffff *     Text video memory & 6845 (mirror of 51'0000)
	// 43'0000-43'ffff *     Graphic video memory (Mirror of 53'0000)
	map(0x410000, 0x41ffff).mirror(0x100000).rw(FUNC(hp9826_state::text_r), FUNC(hp9826_state::text_w));
	map(0x430000, 0x43ffff).mirror(0x100000).rw(FUNC(hp9826_state::graphic_r), FUNC(hp9826_state::graphic_w));
}

uint16_t hp9826_state::text_r(offs_t offset, uint16_t mem_mask)
{
	if (BIT(offset, 12)) {
		if (ACCESSING_BITS_0_7) {
			return m_text_vram[offset & (TEXT_VRAM_SIZE - 1)];
		} else {
			return m_cpu->berr_r();
		}
	} else if (BIT(offset, 0)) {
		return m_crtc->register_r();
	} else {
		LOG("Reading from non-existing register of 6845!\n");
		return 0;
	}
}

void hp9826_state::text_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (BIT(offset, 12)) {
		m_hsync_en = !BIT(offset, 14);
		if (ACCESSING_BITS_0_7) {
			m_text_vram[offset & (TEXT_VRAM_SIZE - 1)] = uint8_t(data);
		} else {
			m_cpu->berr_w(data);
		}
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

uint16_t hp9826_state::graphic_r(offs_t offset, uint16_t mem_mask)
{
	m_graphic_en = !BIT(offset, 14);

	if (ACCESSING_BITS_0_7) {
		return m_graphic_vram[offset & (GRAPHIC_VRAM_SIZE - 1)];
	} else {
		return m_cpu->berr_r();
	}
}

void hp9826_state::graphic_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_graphic_en = !BIT(offset, 14);

	if (ACCESSING_BITS_0_7) {
		m_graphic_vram[offset & (GRAPHIC_VRAM_SIZE - 1)] = uint8_t(data);
	} else {
		return m_cpu->berr_w(0);
	}
}

MC6845_UPDATE_ROW(hp9826_state::crtc_update_row)
{
	const pen_t *pen = m_palette->pens();
	// Text enable multiplexer
	//
	// | MA13 | VSEL | RA=11 | text_en    |
	// |------+------+-------+------------|
	// |    0 |    0 |     0 | 0          |
	// |    0 |    0 |     1 | 0          |
	// |    0 |    1 |     0 | m_hsync_en |
	// |    0 |    1 |     1 | 0          |
	// |    1 |    0 |     0 | m_hsync_en |
	// |    1 |    0 |     1 | m_hsync_en |
	// |    1 |    1 |     0 | m_hsync_en |
	// |    1 |    1 |     1 | 0          |
	bool vmux[] = { false, false, m_hsync_en, false, m_hsync_en, m_hsync_en, m_hsync_en, false };
	unsigned vmux_idx = (ra & 0xb) == 0xb ? 1 : 0;
	uint16_t graph_addr = y * x_count;
	for (int i = 0; i < x_count; i++) {
		uint16_t text_addr = ma + i;
		bool vsel = (text_addr & 0x1800) == 0x1800;
		bool text_en = vmux[vmux_idx + (vsel ? 2 : 0) + (BIT(text_addr, 13) ? 4 : 0)];
		uint8_t text_pixels = text_en ? m_chargen[unsigned(m_text_vram[text_addr & (TEXT_VRAM_SIZE - 1)]) << 4 | ra] : 0;
		bool inv = text_en ? (vsel ^ (cursor_x == i)) : false;
		if (inv) {
			text_pixels = ~text_pixels;
		}
		uint8_t graph_pixels = m_graphic_en ? m_graphic_vram[(graph_addr + i) & (GRAPHIC_VRAM_SIZE - 1)] : 0;
		for (unsigned col = 0; col < 8; col++) {
			if (vsel ? BIT(graph_pixels, 7 - col) : BIT(graph_pixels, 7 - col) ^ BIT(text_pixels, 7 - col)) {
				bitmap.pix(y, i * 8 + col) = pen[2];
			} else if (vsel && BIT(text_pixels, 7 - col)) {
				bitmap.pix(y, i * 8 + col) = pen[1];
			} else {
				bitmap.pix(y, i * 8 + col) = pen[0];
			}
		}
	}
}

ROM_START(hp9826a)
	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("9826-chargen.bin", 0, 0x1000, CRC(dc070d09) SHA1(f11d45fb11b95cb02429ef54dd8a802b77436296))

	ROM_REGION(0x100, "idprom", 0)
	ROM_LOAD("prom9826.bin", 0, 0x100, CRC(358a1a8b) SHA1(2d87a445efdcd45a8606838c6d66a10cc68af85a))

	ROM_REGION(0x10000, "cpu", 0)
	ROM_DEFAULT_BIOS("bios40")
	ROM_SYSTEM_BIOS(0, "bios40",  "BIOS v4.0")
	ROMX_LOAD("rom40.bin", 0x0000, 0x10000, CRC(36005480) SHA1(645a077ffd95e4c31f05cd8bbd6e4554b12813f1), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "bios30",  "BIOS v3.0")
	ROMX_LOAD("rom30.bin", 0x0000, 0x10000, CRC(05c07e75) SHA1(3066a65e6137482041f9a77d09ee2289fe0974aa), ROM_BIOS(1))
ROM_END

// +--------------+
// | hp9836_state |
// +--------------+
class hp9836_state : public hp9826_36_state
{
public:
	hp9836_state(const machine_config &mconfig, device_type type, const char *tag)
		: hp9826_36_state(mconfig, type, tag)
		, m_frame_rate_sw(*this, "frame_rate_sw")
		, m_60hz(false)
	{
	}

	void hp9836(machine_config &mconfig);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	static inline constexpr unsigned TEXT_VRAM_SIZE = 2048;
	static inline constexpr unsigned GRAPHIC_VRAM_SIZE = 16384;

	virtual void cpu_mem_map(address_map &map) override ATTR_COLD;
	uint16_t text_r(offs_t offset, uint16_t mem_mask);
	void text_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t graphic_r(offs_t offset, uint16_t mem_mask);
	void graphic_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	MC6845_UPDATE_ROW(crtc_update_row);

	required_ioport m_frame_rate_sw;

	uint16_t m_text_vram[TEXT_VRAM_SIZE];
	uint16_t m_graphic_vram[GRAPHIC_VRAM_SIZE];
	bool m_60hz;
};

void hp9836_state::hp9836(machine_config &mconfig)
{
	// Screen is arranged in this way.
	//
	// ↑  +----------------------------------------+ ↑
	// 15 | Graphic only                           |
	// ↓  +----------------------------------------+ 390
	// ↑  | Graphic & text                         | lines
	// 375| Text: 25 rows, 15 scan lines each      |
	// ↓  +----------------------------------------+ ↓
	//    ← 512 graphic pixels                     →
	//    ← 16G →← 720 text pixels (80 x 9) →← 16G →
	//    ← 48H →← 1440 half pixels         →← 48H →
	//    ← 1536 half pixels                       →
	//
	// 1 graphic pixel = 1.5 text pixels
	// 2 half pixels = 1 text pixel
	// 3 half pixels = 1 graphic pixel
	//
	// Character cell: 15 lines x 9 columns (18 half pixels)
	//
	// MAME only knows about half pixels in the horizontal direction

	// In the horizontal direction everything is doubled to account for half pixels
	hp9826_36(mconfig, DOT_CLOCK_9836 * 2, 18);

	SCREEN(mconfig, m_screen, SCREEN_TYPE_RASTER, rgb_t::white());
	// Parameters for 50 Hz frame rate
	m_screen->set_raw(DOT_CLOCK_9836 * 2, 2070, 0, 1536, 498, 0, 390);
	m_screen->set_screen_update(m_crtc, FUNC(mc6845_device::screen_update));

	PALETTE(mconfig, m_palette, palette_device::MONOCHROME_HIGHLIGHT);

	m_crtc->set_update_row_callback(FUNC(hp9836_state::crtc_update_row));
	// Expand visible area to include 16 graphic pixels (48 text half-pixels) on either side and
	// 15 graphic lines at the top
	m_crtc->set_visarea_adjust(0, 96, 0, 15);

	FLOPPY_CONNECTOR(mconfig, m_drive1, floppies, "525dd", floppy_image_device::default_mfm_floppy_formats, true);
}

void hp9836_state::machine_start()
{
	hp9826_36_state::machine_start();
	save_item(NAME(m_text_vram));
	save_item(NAME(m_graphic_vram));
	save_item(NAME(m_60hz));
}

void hp9836_state::cpu_mem_map(address_map &map)
{
	hp9826_36_state::cpu_mem_map(map);

	// Range           DTACK Device
	// ==============================
	// 51'0000-51'ffff *     Text video memory & 6845
	// 53'0000-53'ffff *     Graphic video memory
	map(0x510000, 0x51ffff).rw(FUNC(hp9836_state::text_r), FUNC(hp9836_state::text_w));
	map(0x530000, 0x53ffff).rw(FUNC(hp9836_state::graphic_r), FUNC(hp9836_state::graphic_w));
}

uint16_t hp9836_state::text_r(offs_t offset, uint16_t mem_mask)
{
	if (BIT(offset, 12)) {
		if (BIT(offset, 13)) {
			bool new_60hz = BIT(offset, 1);
			if (new_60hz != m_60hz) {
				m_60hz = new_60hz;
				LOG("60/50 Hz FF set to %d\n", m_60hz);
			}
		}
		uint16_t res = m_text_vram[offset & (TEXT_VRAM_SIZE - 1)];
		BIT_CLR(res, 15);
		if (m_frame_rate_sw->read()) {
			BIT_SET(res, 15);
		}
		return res;
	} else if (BIT(offset, 0)) {
		return m_crtc->register_r();
	} else {
		LOG("Reading from non-existing register of 6845!\n");
		return 0;
	}
}

void hp9836_state::text_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (BIT(offset, 12)) {
		m_hsync_en = !BIT(offset, 14);
		if (BIT(offset, 13)) {
			bool new_60hz = BIT(offset, 1);
			if (new_60hz != m_60hz) {
				m_60hz = new_60hz;
				LOG("60/50 Hz FF set to %d\n", m_60hz);
			}
		}
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

uint16_t hp9836_state::graphic_r(offs_t offset, uint16_t mem_mask)
{
	m_graphic_en = !BIT(offset, 14);

	return m_graphic_vram[offset & (GRAPHIC_VRAM_SIZE - 1)];
}

void hp9836_state::graphic_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_graphic_en = !BIT(offset, 14);

	COMBINE_DATA(&m_graphic_vram[offset & (GRAPHIC_VRAM_SIZE - 1)]);
}

MC6845_UPDATE_ROW(hp9836_state::crtc_update_row)
{
	// 1536 (64 * 24) half pixels
	uint64_t gdots[24] = { 0 };
	uint64_t tdots[24] = { 0 };
	uint64_t invert[24] = { 0 };
	uint64_t half_b[24] = { 0 };

	if (y < 15 * 25) {
		// Text
		// 48 is the leftmost half-pixel of text part
		uint64_t tdots_mask = 1ULL << (63 - 48);
		unsigned tdots_pos = 0;
		// Text enable multiplexer
		//
		// | MA13 | MA12 | MA11 | text_en    |
		// |------+------+------+------------|
		// |    0 |    0 |    0 | 0          |
		// |    0 |    0 |    1 | 0          |
		// |    0 |    1 |    0 | 0          |
		// |    0 |    1 |    1 | m_hsync_en |
		// |    1 |    0 |    0 | m_hsync_en |
		// |    1 |    0 |    1 | 0          |
		// |    1 |    1 |    0 | m_hsync_en |
		// |    1 |    1 |    1 | m_hsync_en |
		bool ten[] = { false, false, false, m_hsync_en, m_hsync_en, false, m_hsync_en, m_hsync_en };

		for (int i = 0; i < x_count; i++) {
			uint16_t t_addr = ma + i;
			bool text_en = ten[(t_addr >> 11) & 7];
			uint16_t char_attr = m_text_vram[t_addr & (TEXT_VRAM_SIZE - 1)];
			// | Bit(s) | Meaning                    |
			// |--------+----------------------------|
			// | 15..12 | N/U                        |
			// |     11 | Half brightness            |
			// |     10 | Underline                  |
			// |      9 | Blink                      |
			// |      8 | Inverted                   |
			// |   7..0 | Character code             |
			bool hb = BIT(char_attr, 11);
			bool inv = false;
			uint16_t text_pixels = 0;
			bool half_shift = false;
			if (text_en) {
				inv = BIT(char_attr, 8);
				bool blink = BIT(char_attr, 9) && BIT(m_screen->frame_number(), 5);
				if (!blink) {
					uint8_t cg = m_chargen[ra | ((char_attr & 0xff) << 4)];
					text_pixels = (cg & 0x7f) << 1;
					if (cursor_x == i || (BIT(char_attr, 10) && ra == 14)) {
						text_pixels = ~text_pixels;
					}
					half_shift = BIT(cg, 7);
				}
			}
			for (unsigned j = 0; j < 18; j++) {
				unsigned bit_pos = half_shift ? (9 - (j + 1) / 2) : (8 - j / 2);
				if (text_pixels & (1 << bit_pos)) {
					tdots[tdots_pos] |= tdots_mask;
				}
				if (hb) {
					half_b[tdots_pos] |= tdots_mask;
				}
				if (inv) {
					invert[tdots_pos] |= tdots_mask;
				}
				tdots_mask >>= 1;
				if (!tdots_mask) {
					tdots_pos++;
					tdots_mask = 1ULL << 63;
				}
			}
		}
	}

	int real_y = (y + 15) % 390;
	if (m_graphic_en) {
		uint64_t gdots_mask = 1ULL << 63;
		unsigned gdots_pos = 0;
		// 32 16-bit words -> 512 pixels
		for (unsigned i = 0; i < 32; i++) {
			uint16_t w = m_graphic_vram[real_y * 32 + i];
			for (uint16_t mask = 0x8000; mask; mask >>= 1) {
				bool dot = (w & mask) != 0;
				// 1 graphic pixel = 3 half pixels
				if (dot) {
					gdots[gdots_pos] |= gdots_mask;
				}
				gdots_mask >>= 1;
				if (!gdots_mask) {
					gdots_pos++;
					gdots_mask = 1ULL << 63;
				}
				if (dot) {
					gdots[gdots_pos] |= gdots_mask;
				}
				gdots_mask >>= 1;
				if (!gdots_mask) {
					gdots_pos++;
					gdots_mask = 1ULL << 63;
				}
				if (dot) {
					gdots[gdots_pos] |= gdots_mask;
				}
				gdots_mask >>= 1;
				if (!gdots_mask) {
					gdots_pos++;
					gdots_mask = 1ULL << 63;
				}
			}
		}
	}

	// Combiner truth table
	//
	// | Gdots | Tdots | Inv | h_b | Video | Int | out |
	// |-------+-------+-----+-----+-------+-----+-----|
	// |     0 |     0 |   0 |   0 |     1 |   1 |   0 |
	// |     0 |     0 |   0 |   1 |     1 |   1 |   0 |
	// |     0 |     0 |   1 |   0 |     0 |   1 |   2 |
	// |     0 |     0 |   1 |   1 |     1 |   0 |   1 |
	// |     0 |     1 |   0 |   0 |     0 |   1 |   2 |
	// |     0 |     1 |   0 |   1 |     1 |   0 |   1 |
	// |     0 |     1 |   1 |   0 |     1 |   1 |   0 |
	// |     0 |     1 |   1 |   1 |     1 |   1 |   0 |
	// |     1 |     0 |   0 |   0 |     0 |   1 |   2 |
	// |     1 |     0 |   0 |   1 |     0 |   1 |   2 |
	// |     1 |     0 |   1 |   0 |     0 |   1 |   2 |
	// |     1 |     0 |   1 |   1 |     0 |   1 |   2 |
	// |     1 |     1 |   0 |   0 |     1 |   1 |   0 |
	// |     1 |     1 |   0 |   1 |     1 |   1 |   0 |
	// |     1 |     1 |   1 |   0 |     0 |   1 |   2 |
	// |     1 |     1 |   1 |   1 |     0 |   1 |   2 |
	const pen_t *pen = m_palette->pens();
	int real_x = 0;
	for (unsigned i = 0; i < 24; i++) {
		uint64_t gd = gdots[i];
		uint64_t not_gd = ~gd;
		uint64_t td = tdots[i];
		uint64_t not_td = ~td;
		uint64_t inv = invert[i];
		uint64_t not_inv = ~inv;
		uint64_t hb = half_b[i];
		uint64_t video = (not_gd & not_td & not_inv) |
			(gd & td & not_inv) |
			(not_gd & hb) |
			(not_gd & td & inv);
		uint64_t half_bright = ~(not_gd & (td ^ inv) & hb);
		for (uint64_t mask = 1ULL << 63; mask; mask >>= 1) {
			if (video & mask) {
				if (half_bright & mask) {
					// Video = 1 & half_bright = 1: OFF
					bitmap.pix(real_y, real_x) = pen[0];
				} else {
					// Video = 1 & half_bright = 0: MID
					bitmap.pix(real_y, real_x) = pen[1];
				}
			} else {
				// Video = 0 & half_bright = x: HI
				bitmap.pix(real_y, real_x) = pen[2];
			}
			real_x++;
		}
	}
}

static INPUT_PORTS_START(hp9836)
	PORT_INCLUDE(hp9826_36)

	PORT_START("frame_rate_sw")
	PORT_CONFNAME(1, 1, "CRT refresh rate")
	PORT_CONFSETTING(1, "50 Hz")
	PORT_CONFSETTING(0, "60 Hz")
INPUT_PORTS_END

ROM_START(hp9836a)
	ROM_REGION(0x1000, "chargen", 0)
	// Content of this ROM is not original, I reconstructed it by adapting 9816 character generator
	// AFAIK no dump is available
	ROM_LOAD("9836-chargen.bin", 0, 0x1000, BAD_DUMP CRC(69754023) SHA1(952dc7fd59e0039fa7c7436fb596ab460241f256))

	ROM_REGION(0x100, "idprom", 0)
	ROM_LOAD("prom9836.bin", 0, 0x100, CRC(52251c32) SHA1(3437c5a78b4c3323ce37340022a4843fcb9f4327))

	ROM_REGION(0x10000, "cpu", 0)
	ROM_DEFAULT_BIOS("bios40")
	ROM_SYSTEM_BIOS(0, "bios40",  "BIOS v4.0")
	ROMX_LOAD("rom40.bin", 0x0000, 0x10000, CRC(36005480) SHA1(645a077ffd95e4c31f05cd8bbd6e4554b12813f1), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "bios30",  "BIOS v3.0")
	ROMX_LOAD("rom30.bin", 0x0000, 0x10000, CRC(05c07e75) SHA1(3066a65e6137482041f9a77d09ee2289fe0974aa), ROM_BIOS(1))
ROM_END

// +---------------+
// | hp9836c_state |
// +---------------+
class hp9836c_state : public hp9826_36_state
{
public:
	hp9836c_state(const machine_config &mconfig, device_type type, const char *tag)
		: hp9826_36_state(mconfig, type, tag)
		, m_frame_rate_sw(*this, "frame_rate_sw")
		, m_clut(*this, "palette", 24 * 2, ENDIANNESS_LITTLE)
		, m_60hz(false)
	{
	}

	void hp9836c(machine_config &mconfig);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	static inline constexpr unsigned TEXT_VRAM_SIZE = 2048;
	static inline constexpr unsigned GRAPHIC_VRAM_SIZE = 131072;

	virtual void cpu_mem_map(address_map &map) override ATTR_COLD;
	uint16_t text_r(offs_t offset, uint16_t mem_mask);
	void text_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t graphic_r(offs_t offset, uint16_t mem_mask);
	void graphic_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	MC6845_UPDATE_ROW(crtc_update_row);

	required_ioport m_frame_rate_sw;
	memory_share_creator<uint8_t> m_clut;

	uint16_t m_text_vram[TEXT_VRAM_SIZE];
	uint16_t m_graphic_vram[GRAPHIC_VRAM_SIZE];
	bool m_60hz;
};

void hp9836c_state::hp9836c(machine_config &mconfig)
{
	// Screen layout is the same as that of 9836A (see hp9836_state::hp9836)
	// In the horizontal direction everything is doubled to account for half pixels
	hp9826_36(mconfig, DOT_CLOCK_9836 * 2, 18);

	SCREEN(mconfig, m_screen, SCREEN_TYPE_RASTER);
	// Parameters for 50 Hz frame rate
	m_screen->set_raw(DOT_CLOCK_9836 * 2, 2070, 0, 1536, 498, 0, 390);
	m_screen->set_screen_update(m_crtc, FUNC(mc6845_device::screen_update));

	PALETTE(mconfig, m_palette).set_format(palette_device::xRGB_444, 24);
	// Entries 0..7 are fixed for text colors
	// Entries 8..23 reflect the state of CLUT memory

	m_crtc->set_update_row_callback(FUNC(hp9836c_state::crtc_update_row));
	// Expand visible area to include 16 graphic pixels (48 text half-pixels) on either side and
	// 15 graphic lines at the top
	m_crtc->set_visarea_adjust(0, 96, 0, 15);

	FLOPPY_CONNECTOR(mconfig, m_drive1, floppies, "525dd", floppy_image_device::default_mfm_floppy_formats, true);
}

void hp9836c_state::machine_start()
{
	hp9826_36_state::machine_start();
	save_item(NAME(m_text_vram));
	save_item(NAME(m_graphic_vram));
	save_item(NAME(m_60hz));

	// Fixed text colors
	// Index:b2 = blue, b1 = not green, b0 = red
	m_palette->set_pen_color(0, 0, 255, 0);     // 000: Green
	m_palette->set_pen_color(1, 255, 255, 0);   // 001: Yellow
	m_palette->set_pen_color(2, 0, 0, 0);       // 010: Black
	m_palette->set_pen_color(3, 255, 0, 0);     // 011: Red
	m_palette->set_pen_color(4, 0, 255, 255);   // 100: Cyan
	m_palette->set_pen_color(5, 255, 255, 255); // 101: White
	m_palette->set_pen_color(6, 0, 0, 255);     // 110: Blue
	m_palette->set_pen_color(7, 255, 0, 255);   // 111: Magenta
}

void hp9836c_state::cpu_mem_map(address_map &map)
{
	hp9826_36_state::cpu_mem_map(map);

	// Range           DTACK Device
	// ==============================
	// 51'0000-51'ffff *     Text video memory, CLUT & 6845
	// 52'0000-55'ffff *     Graphic video memory
	map(0x510000, 0x51ffff).rw(FUNC(hp9836c_state::text_r), FUNC(hp9836c_state::text_w));
	map(0x520000, 0x55ffff).rw(FUNC(hp9836c_state::graphic_r), FUNC(hp9836c_state::graphic_w));
}

uint16_t hp9836c_state::text_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t res = 0;

	if (BIT(offset, 12)) {
		bool texttop = (offset & 0x7c00) == 0x7c00;
		if (texttop) {
			if (BIT(offset, 9)) {
				if (BIT(offset, 0)) {
					// ID register
					res = 0x0a30;
					if (m_frame_rate_sw->read()) {
						BIT_SET(res, 3);
					}
				} else {
					// Vblank
					if (m_screen->vpos() >= 390) {
						BIT_SET(res, 0);
					}
				}
			} else  {
				// Read CLUT
				res = m_palette->read16((offset & 0xf) + 8) | 0xf000;
			}
		} else {
			// Text VRAM
			res = m_text_vram[offset & (TEXT_VRAM_SIZE - 1)];
			BIT_CLR(res, 15);
		}
	} else if (BIT(offset, 0)) {
		res = m_crtc->register_r();
	} else {
		LOG("Reading from non-existing register of 6845!\n");
	}

	return res;
}

void hp9836c_state::text_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (BIT(offset, 12)) {
		bool texttop = (offset & 0x7c00) == 0x7c00;
		if (texttop) {
			if (BIT(offset, 9)) {
				if (BIT(offset, 0)) {
					LOG("Writing %04x to 51fffe/f\n", data);
				} else {
					// Schematics and Pascal doc disagree over the address of this register
					// Pascal doc makes more sense though
					m_graphic_en = BIT(data, 0);
				}
			} else {
				if (mem_mask != 0xffff) {
					LOG("Writing to CLUT a single byte!\n");
				}
				m_palette->write16((offset & 0xf) + 8, ~data & 0xfff);
			}
		} else {
			if (BIT(offset, 13)) {
				bool new_60hz = BIT(offset, 1);
				if (new_60hz != m_60hz) {
					m_60hz = new_60hz;
					LOG("60/50 Hz FF set to %d\n", m_60hz);
				}
			}
			m_hsync_en = !BIT(offset, 14);
			COMBINE_DATA(&m_text_vram[offset & (TEXT_VRAM_SIZE - 1)]);
		}
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

uint16_t hp9836c_state::graphic_r(offs_t offset, uint16_t mem_mask)
{
	return m_graphic_vram[offset & (GRAPHIC_VRAM_SIZE - 1)];
}

void hp9836c_state::graphic_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset &= (GRAPHIC_VRAM_SIZE - 1);
	COMBINE_DATA(&m_graphic_vram[offset]);
	// Only bits 11..8 and 3..0 are actually present
	m_graphic_vram[offset] &= 0x0f0f;
}

// Text enable multiplexer
//
// | MA13 | MA12 | MA11 | text_en    |
// |------+------+------+------------|
// |    0 |    0 |    0 | 0          |
// |    0 |    0 |    1 | 0          |
// |    0 |    1 |    0 | 0          |
// |    0 |    1 |    1 | 1          |
// |    1 |    0 |    0 | 1          |
// |    1 |    0 |    1 | 0          |
// |    1 |    1 |    0 | 1          |
// |    1 |    1 |    1 | 1          |
static const bool text_enable[] = { false, false, false, true, true, false, true, true };

MC6845_UPDATE_ROW(hp9836c_state::crtc_update_row)
{
	const pen_t *pen = m_palette->pens();
	int real_y = (y + 15) % 390;
	if (m_graphic_en) {
		// Graphic
		const uint16_t *ptr = &m_graphic_vram[real_y * 256];
		for (int real_x = 0; real_x < 1536; ptr++) {
			pen_t pen_pix = pen[8 + ((*ptr >> 8) & 0xf)];
			bitmap.pix(real_y, real_x++) = pen_pix;
			bitmap.pix(real_y, real_x++) = pen_pix;
			bitmap.pix(real_y, real_x++) = pen_pix;
			pen_pix = pen[8 + (*ptr & 0xf)];
			bitmap.pix(real_y, real_x++) = pen_pix;
			bitmap.pix(real_y, real_x++) = pen_pix;
			bitmap.pix(real_y, real_x++) = pen_pix;
		}
	} else {
		bitmap.fill(rgb_t::black(), rectangle(0, 1535, real_y, real_y));
	}
	if (y < 15 * 25) {
		// Text
		for (int i = 0; i < x_count; i++) {
			uint16_t t_addr = ma + i;
			bool text_en = m_hsync_en ? text_enable[(t_addr >> 11) & 7] : false;
			if (text_en) {
				// | Bit(s) | Meaning        |
				// |--------+----------------|
				// |     15 | N/U            |
				// |     14 | Blue           |
				// |     13 | !Green         |
				// |     12 | Red            |
				// |     11 | N/U            |
				// |     10 | Underline      |
				// |      9 | Blink          |
				// |      8 | Inverted       |
				// |   7..0 | Character code |
				uint16_t char_attr = m_text_vram[t_addr & (TEXT_VRAM_SIZE - 1)];
				bool blink = BIT(char_attr, 9) && BIT(m_screen->frame_number(), 5);
				uint16_t text_pixels;
				bool half_shift;
				if (blink) {
					text_pixels = 0;
					half_shift = false;
				} else {
					uint8_t cg = m_chargen[ra | ((char_attr & 0xff) << 4)];
					text_pixels = (cg & 0x7f) << 1;
					half_shift = BIT(cg, 7);
				}
				bool inv = BIT(char_attr, 8);
				if (inv ^ ((cursor_x == i) || (ra == 14 && BIT(char_attr, 10) && !blink))) {
					text_pixels = ~text_pixels;
				}
				if (text_pixels) {
					pen_t text_pen = pen[(char_attr >> 12) & 7];
					for (unsigned j = 0; j < 18; j++) {
						unsigned bit_pos = half_shift ? (9 - (j + 1) / 2) : (8 - j / 2);
						if (text_pixels & (1 << bit_pos)) {
							bitmap.pix(real_y, 48 + i * 18 + j) = text_pen;
						}
					}
				}
			}
		}
	}
}

ROM_START(hp9836c)
	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("9836-chargen.bin", 0, 0x1000, BAD_DUMP CRC(69754023) SHA1(952dc7fd59e0039fa7c7436fb596ab460241f256))

	ROM_REGION(0x100, "idprom", 0)
	ROM_LOAD("prom9836c.bin", 0, 0x100, CRC(092a1815) SHA1(1c143b0bb6e8715ba6fc7d45070820d7e0dba25e))

	ROM_REGION(0x10000, "cpu", 0)
	ROM_DEFAULT_BIOS("bios40")
	ROM_SYSTEM_BIOS(0, "bios40",  "BIOS v4.0")
	ROMX_LOAD("rom40.bin", 0x0000, 0x10000, CRC(36005480) SHA1(645a077ffd95e4c31f05cd8bbd6e4554b12813f1), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "bios30",  "BIOS v3.0")
	ROMX_LOAD("rom30.bin", 0x0000, 0x10000, CRC(05c07e75) SHA1(3066a65e6137482041f9a77d09ee2289fe0974aa), ROM_BIOS(1))
ROM_END

} // anonymous namespace

//   YEAR  NAME     PARENT  COMPAT  MACHINE INPUT     CLASS        INIT        COMPANY            FULLNAME    FLAGS
COMP(1982, hp9816a, 0,      0,      hp9816, hp9816,   hp9816_state,empty_init, "Hewlett-Packard", "HP 9816A", 0)
COMP(1981, hp9826a, 0,      0,      hp9826, hp9826_36,hp9826_state,empty_init, "Hewlett-Packard", "HP 9826A", 0)
COMP(1981, hp9836a, 0,      0,      hp9836, hp9836,   hp9836_state,empty_init, "Hewlett-Packard", "HP 9836A", 0)
COMP(1983, hp9836c, 0,      0,      hp9836c,hp9836,   hp9836c_state,empty_init,"Hewlett-Packard", "HP 9836C", 0)

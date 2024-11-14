// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for Systel System 1/System 100 computer.

    This minimalistic luggable computer has a green-screen monitor and
    5¼-inch floppy drive (Shugart SA455) built into the unit. The detached
    keyboard may be replaced with a standard electric typewriter.

***************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/clock.h"
#include "machine/input_merger.h"
#include "machine/i8251.h"
#include "machine/i8257.h"
#include "machine/wd_fdc.h"
#include "video/i8275.h"
#include "screen.h"


namespace {

class systel1_state : public driver_device
{
public:
	systel1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dmac(*this, "dmac")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:0")
		, m_rom(*this, "bootrom")
		, m_chargen(*this, "chargen")
	{
	}

	void systel1(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void hrq_w(int state);
	u8 memory_r(offs_t offset);
	void memory_w(offs_t offset, u8 data);
	void floppy_control_w(u8 data);
	void rts_w(int state);
	void dtr_w(int state);

	I8275_DRAW_CHARACTER_MEMBER(draw_character);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<i8257_device> m_dmac;
	required_device<fd1797_device> m_fdc;
	required_device<floppy_connector> m_floppy;
	required_region_ptr<u8> m_rom;
	required_region_ptr<u8> m_chargen;

	std::unique_ptr<u8[]> m_ram;

	bool m_boot_read = false;
};

void systel1_state::machine_start()
{
	m_ram = make_unique_clear<u8[]>(0x10000);
	m_fdc->set_floppy(m_floppy->get_device());

	save_item(NAME(m_boot_read));
	save_pointer(NAME(m_ram), 0x10000);
}

void systel1_state::machine_reset()
{
	m_boot_read = true;
}

void systel1_state::hrq_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);
	m_dmac->hlda_w(state);
}

u8 systel1_state::memory_r(offs_t offset)
{
	if (m_boot_read && offset < 0x4000)
		return m_rom[offset & 0x7ff];
	else
		return m_ram[offset];
}

void systel1_state::memory_w(offs_t offset, u8 data)
{
	m_ram[offset] = data;
}

I8275_DRAW_CHARACTER_MEMBER(systel1_state::draw_character)
{
	using namespace i8275_attributes;
	u8 dots = BIT(attrcode, LTEN) ? 0xff : BIT(attrcode, VSP) ? 0 : m_chargen[(charcode << 4) | linecount];
	if (BIT(attrcode, RVV))
		dots ^= 0xff;

	for (int i = 0; i < 7; i++)
	{
		bitmap.pix(y, x + i) = BIT(dots, 7) ? rgb_t::white() : rgb_t::black();
		dots <<= 1;
	}
}

void systel1_state::floppy_control_w(u8 data)
{
	if (m_floppy->get_device() != nullptr)
		m_floppy->get_device()->ss_w(!BIT(data, 0));

	m_boot_read = false;
}

void systel1_state::rts_w(int state)
{
	m_fdc->mr_w(state);
	if (m_floppy->get_device() != nullptr)
		m_floppy->get_device()->mon_w(!state);
}

void systel1_state::dtr_w(int state)
{
	// probably floppy-related
}

void systel1_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(systel1_state::memory_r), FUNC(systel1_state::memory_w));
}

void systel1_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("JUMPERS");
	map(0x10, 0x11).rw("usart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x20, 0x29).rw(m_dmac, FUNC(i8257_device::read), FUNC(i8257_device::write));
	map(0x30, 0x33).rw(m_fdc, FUNC(fd1797_device::read), FUNC(fd1797_device::write));
	map(0x40, 0x41).rw("crtc", FUNC(i8276_device::read), FUNC(i8276_device::write));
	map(0x70, 0x70).w(FUNC(systel1_state::floppy_control_w));
}

static INPUT_PORTS_START(systel1)
	PORT_START("JUMPERS")
	PORT_DIPNAME(0x01, 0x00, "System Mode") // TYPE/WP switch on the front of the unit
	PORT_DIPSETTING(0x01, "Typewriter")
	PORT_DIPSETTING(0x00, "Word Processor")
	PORT_DIPNAME(0x02, 0x00, "Screen Refresh") // probably at J6
	PORT_DIPSETTING(0x02, "50 Hz")
	PORT_DIPSETTING(0x00, "60 Hz")
INPUT_PORTS_END

static void systel1_floppies(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

static DEVICE_INPUT_DEFAULTS_START(keyboard)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_1200)
DEVICE_INPUT_DEFAULTS_END

void systel1_state::systel1(machine_config &config)
{
	Z80(config, m_maincpu, 10.8864_MHz_XTAL / 4); // Z8400A; clock verified
	m_maincpu->set_addrmap(AS_PROGRAM, &systel1_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &systel1_state::io_map);

	input_merger_device &mainint(INPUT_MERGER_ANY_HIGH(config, "mainint"));
	mainint.output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	I8257(config, m_dmac, 10.8864_MHz_XTAL / 4); // P8257-5
	m_dmac->out_hrq_cb().set(FUNC(systel1_state::hrq_w));
	m_dmac->in_memr_cb().set(FUNC(systel1_state::memory_r));
	m_dmac->out_memw_cb().set(FUNC(systel1_state::memory_w));
	m_dmac->in_ior_cb<1>().set(m_fdc, FUNC(fd1797_device::data_r));
	m_dmac->out_iow_cb<1>().set(m_fdc, FUNC(fd1797_device::data_w));
	m_dmac->out_iow_cb<2>().set("crtc", FUNC(i8276_device::dack_w));

	i8251_device &usart(I8251(config, "usart", 10.8864_MHz_XTAL / 4)); // AMD P8251A
	usart.rxrdy_handler().set("mainint", FUNC(input_merger_device::in_w<0>));
	usart.rts_handler().set(FUNC(systel1_state::rts_w));
	usart.dtr_handler().set(FUNC(systel1_state::dtr_w));

	clock_device &baudclock(CLOCK(config, "baudclock", 2_MHz_XTAL / 104)); // 19.23 kHz verified; rate probably fixed
	baudclock.signal_handler().set("usart", FUNC(i8251_device::write_rxc));
	baudclock.signal_handler().append("usart", FUNC(i8251_device::write_txc));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_raw(10.8864_MHz_XTAL, 672, 0, 560, 270, 0, 250);
	screen.set_screen_update("crtc", FUNC(i8276_device::screen_update));

	i8276_device &crtc(I8276(config, "crtc", 10.8864_MHz_XTAL / 7)); // WD8276PL-00
	crtc.set_screen("screen");
	crtc.set_character_width(7);
	crtc.set_display_callback(FUNC(systel1_state::draw_character));
	crtc.drq_wr_callback().set(m_dmac, FUNC(i8257_device::dreq2_w));
	crtc.irq_wr_callback().set("mainint", FUNC(input_merger_device::in_w<1>));

	FD1797(config, m_fdc, 2_MHz_XTAL / 2);
	m_fdc->drq_wr_callback().set(m_dmac, FUNC(i8257_device::dreq1_w));

	FLOPPY_CONNECTOR(config, m_floppy, systel1_floppies, "525dd", floppy_image_device::default_mfm_floppy_formats);
	m_floppy->set_fixed(true);
	m_floppy->enable_sound(true);

	rs232_port_device &kb(RS232_PORT(config, "kb", default_rs232_devices, "keyboard"));
	kb.set_option_device_input_defaults("keyboard", DEVICE_INPUT_DEFAULTS_NAME(keyboard));
	kb.rxd_handler().set("usart", FUNC(i8251_device::write_rxd));
}

// RAM: 8x MCM6665AP20
// XTALs: 10.8864 (Y1), 2.000 (Y2)
// Silkscreened on PCB: "SYSTEL / SYSTEM 1 / 1031-3101-00 REV H"
ROM_START(systel100)
	ROM_REGION(0x800, "bootrom", 0) // TMS2716JL-45
	ROM_LOAD("u11.bin", 0x000, 0x800, CRC(29f1414a) SHA1(c87adfaaec45d0e5f4cf5946d2f04de0332b3413))

	ROM_REGION(0x800, "chargen", 0) // TMS2516JL-45
	ROM_LOAD("u16.bin", 0x000, 0x800, CRC(61a8d742) SHA1(69dada638a17353f91bff34a1e2319a35d8a3ebf))
ROM_END

} // anonymous namespace


COMP(198?, systel100, 0, 0, systel1, systel1, systel1_state, empty_init, "Systel Computers", "System 100", MACHINE_IS_SKELETON)

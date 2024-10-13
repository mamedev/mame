// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Preliminary driver for Wicat T7000 terminal.

    This fairly typical ASCII video display terminal was sold for use with
    the Wicat System 100. It shows a basic 80x25 monochrome text display
    with an optional status line, and supports the VT52 and ANSI command
    sets. Keytronic Model L2207 is the specified keyboard. Settings can be
    saved to nonvolatile memory by typing "PERM" (all caps) in Set-Up mode.

    Currently the optional touch panel is not supported.

****************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/input_merger.h"
#include "machine/keytronic_l2207.h"
#include "machine/scn_pci.h"
#include "machine/x2212.h"
#include "video/i8275.h"

#include "screen.h"


namespace {

class t7000_state : public driver_device
{
public:
	t7000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mainint(*this, "mainint")
		, m_outlatch(*this, "outlatch")
		, m_pci(*this, "pci%u", 0U)
		, m_crtc(*this, "crtc%u", 0U)
		, m_vram(*this, "vram")
		, m_attr_vram(*this, "attr_vram", 0x4000, ENDIANNESS_LITTLE)
		, m_vram_view(*this, "vram")
		, m_chargen(*this, "chargen")
		, m_vblint(false)
		, m_attr_latch(0)
	{
	}

	void t7000(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	I8275_DRAW_CHARACTER_MEMBER(display_character);

	void vram_w(offs_t offset, u8 data);
	u8 vram_dma_r(offs_t offset);
	void attr_latch_w(u8 data);
	u8 vblint_status_r();
	void vblint_enable_w(int state);
	void dma_enable_w(int state);
	void vblint_w(int state);
	void crtc_combined_w(offs_t offset, u8 data);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<input_merger_device> m_mainint;
	required_device<ls259_device> m_outlatch;
	required_device_array<scn_pci_device, 2> m_pci;
	required_device_array<i8275_device, 2> m_crtc;
	required_shared_ptr<u8> m_vram;
	memory_share_creator<u8> m_attr_vram;
	memory_view m_vram_view;
	required_region_ptr<u8> m_chargen;

	bool m_vblint;
	u8 m_attr_latch;
};

void t7000_state::machine_start()
{
	save_item(NAME(m_vblint));
	save_item(NAME(m_attr_latch));
}

I8275_DRAW_CHARACTER_MEMBER(t7000_state::display_character)
{
	// TODO: blinking and blanked characters
	using namespace i8275_attributes;
	u16 dots = 0;
	if (!BIT(attrcode, VSP))
	{
		if (BIT(attrcode, LTEN + 8) || (BIT(charcode, 12) && linecount == 10))
			dots = 0x3ff; // underscore
		else
			dots = m_chargen[(charcode & 0x7f) << 4 | linecount] << 1;
		if (BIT(attrcode, RVV + 8) || BIT(charcode, 11))
			dots ^= 0x3ff; // reverse video
	}

	rgb_t fg = rgb_t::white();
	rgb_t bg = rgb_t::black();
	if (m_outlatch->q2_r())
	{
		using std::swap;
		swap(fg, bg);
	}
	if (BIT(charcode, 8))
		fg = rgb_t(0xc0, 0xc0, 0xc0); // reduced intensity

	for (int i = 0; i < 10; i++)
	{
		bitmap.pix(y, x + i) = BIT(dots, 9) ? fg : bg;
		dots <<= 1;
	}
}

void t7000_state::vram_w(offs_t offset, u8 data)
{
	m_vram[offset] = data;
	m_attr_vram[offset] = m_attr_latch;
}

u8 t7000_state::vram_dma_r(offs_t offset)
{
	u8 data = m_vram[offset];
	if (!machine().side_effects_disabled())
	{
		m_crtc[0]->dack_w(data);
		m_crtc[1]->dack_w(m_attr_vram[offset]);
	}
	return data;
}

void t7000_state::attr_latch_w(u8 data)
{
	m_attr_latch = data;
}

u8 t7000_state::vblint_status_r()
{
	// Only bit 5 is ever examined
	return m_vblint ? 0x20 : 0x00;
}

void t7000_state::vblint_enable_w(int state)
{
	if (!state && m_vblint)
	{
		m_vblint = false;
		m_mainint->in_w<4>(0);
	}
}

void t7000_state::dma_enable_w(int state)
{
	if (state)
		m_vram_view.select(0);
	else
		m_vram_view.disable();
}

void t7000_state::vblint_w(int state)
{
	if (state && m_outlatch->q0_r() && !m_vblint)
	{
		m_vblint = true;
		m_mainint->in_w<4>(1);
	}
}

void t7000_state::crtc_combined_w(offs_t offset, u8 data)
{
	for (auto &crtc : m_crtc)
		crtc->write(BIT(offset, 0), data);
}

void t7000_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("program", 0);
	map(0x4000, 0x7fff).ram().share(m_vram).w(FUNC(t7000_state::vram_w));
	map(0x4000, 0x7fff).view(m_vram_view);
	m_vram_view[0](0x4000, 0x7fff).r(FUNC(t7000_state::vram_dma_r));
	map(0x8000, 0x803f).rw("novram", FUNC(x2210_device::read), FUNC(x2210_device::write));
}

void t7000_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x80, 0x80).r(FUNC(t7000_state::vblint_status_r));
	map(0xa1, 0xa1).nopr(); // touch panel status?
	map(0xb0, 0xb1).w(FUNC(t7000_state::crtc_combined_w));
	map(0xb2, 0xb3).rw(m_crtc[0], FUNC(i8275_device::read), FUNC(i8275_device::write));
	map(0xb4, 0xb5).rw(m_crtc[1], FUNC(i8275_device::read), FUNC(i8275_device::write));
	map(0xc0, 0xc3).rw(m_pci[0], FUNC(scn_pci_device::read), FUNC(scn_pci_device::write));
	map(0xd0, 0xd3).rw(m_pci[1], FUNC(scn_pci_device::read), FUNC(scn_pci_device::write));
	map(0xe0, 0xe0).w(FUNC(t7000_state::attr_latch_w));
	map(0xf0, 0xf7).w("outlatch", FUNC(ls259_device::write_d0));
}

static INPUT_PORTS_START(t7000)
INPUT_PORTS_END

void t7000_state::t7000(machine_config &config)
{
	Z80(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &t7000_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &t7000_state::io_map);

	INPUT_MERGER_ANY_HIGH(config, m_mainint).output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	INPUT_MERGER_ALL_HIGH(config, "mainnmi").output_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);

	X2210(config, "novram"); // U39

	SCN2651(config, m_pci[0], 5.0688_MHz_XTAL);
	m_pci[0]->txd_handler().set("serial", FUNC(rs232_port_device::write_txd));
	m_pci[0]->rts_handler().set("serial", FUNC(rs232_port_device::write_rts));
	m_pci[0]->dtr_handler().set("serial", FUNC(rs232_port_device::write_dtr));
	m_pci[0]->txrdy_handler().set(m_mainint, FUNC(input_merger_device::in_w<0>));
	m_pci[0]->rxrdy_handler().set(m_mainint, FUNC(input_merger_device::in_w<1>));

	SCN2651(config, m_pci[1], 5.0688_MHz_XTAL);
	m_pci[1]->txd_handler().set("keyboard", FUNC(keytronic_l2207_device::ser_in_w));
	m_pci[1]->txrdy_handler().set(m_mainint, FUNC(input_merger_device::in_w<2>));
	m_pci[1]->rxrdy_handler().set(m_mainint, FUNC(input_merger_device::in_w<3>));

	KEYTRONIC_L2207(config, "keyboard").ser_out_callback().set(m_pci[1], FUNC(scn_pci_device::rxd_w));

	LS259(config, m_outlatch); // U43
	m_outlatch->q_out_cb<0>().set(FUNC(t7000_state::vblint_enable_w));
	m_outlatch->q_out_cb<1>().set(FUNC(t7000_state::dma_enable_w));
	m_outlatch->q_out_cb<5>().set("novram", FUNC(x2210_device::recall)).invert();
	m_outlatch->q_out_cb<6>().set("mainnmi", FUNC(input_merger_device::in_w<0>));
	m_outlatch->q_out_cb<7>().set("novram", FUNC(x2210_device::store));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green()); // "Phospher Type...P31-green (mediam persistence)" (manual p. 6-2)
	screen.set_raw(19.6608_MHz_XTAL, 1020, 0, 800, 324, 0, 300);
	screen.set_screen_update(m_crtc[0], FUNC(i8275_device::screen_update));

	I8276(config, m_crtc[0], 19.6608_MHz_XTAL / 10);
	m_crtc[0]->set_character_width(10);
	m_crtc[0]->set_display_callback(FUNC(t7000_state::display_character));
	m_crtc[0]->drq_wr_callback().set("mainnmi", FUNC(input_merger_device::in_w<1>));
	m_crtc[0]->vrtc_wr_callback().set(FUNC(t7000_state::vblint_w));
	m_crtc[0]->set_screen("screen");
	m_crtc[0]->set_next_crtc(m_crtc[1]);

	I8276(config, m_crtc[1], 19.6608_MHz_XTAL / 10);
	m_crtc[1]->set_character_width(10);
	m_crtc[1]->set_screen("screen");

	rs232_port_device &serial(RS232_PORT(config, "serial", default_rs232_devices, nullptr));
	serial.rxd_handler().set(m_pci[0], FUNC(scn_pci_device::rxd_w));
	serial.cts_handler().set(m_pci[0], FUNC(scn_pci_device::cts_w));

	// TODO: RS232C printer port ("same as above except SEND only")
}

ROM_START(t7000)
	ROM_REGION(0x4000, "program", 0)
	ROM_LOAD("t7000_0_8-17-82.u35", 0x0000, 0x1000, CRC(d1645232) SHA1(cdc203942af5b8b3e6bd189c4c7121e480ce1e17)) // all Intel D2732A-3 or MBM2732A-30
	ROM_LOAD("t7000_1_8-17-82.u36", 0x1000, 0x1000, CRC(3441e9cc) SHA1(323d97308170ec6a52a64a60bb8d4554e11e9c12))
	ROM_LOAD("t7000_2_8-17-82.u37", 0x2000, 0x1000, CRC(43a50f3e) SHA1(ae25d3d586ff7027d326e7ce061523b435c2d651))
	ROM_LOAD("t7000_3_8-17-82.u38", 0x3000, 0x1000, CRC(d4ef7293) SHA1(e8c331f629c29d9441723dad9e01f7447638202d))

	ROM_REGION(0x800, "chargen", 0)
	ROM_LOAD("blank.u45", 0x000, 0x800, CRC(0fadcbc8) SHA1(b939b204e76b5d390814a3e575f5473b0a4cbf9d)) // MM2716Q-1
ROM_END

} // anonymous namespace

SYST(1982, t7000, 0, 0, t7000, t7000, t7000_state, empty_init, "Wicat Systems", "T7000 Video Terminal", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE)

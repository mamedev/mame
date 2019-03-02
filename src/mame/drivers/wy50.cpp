// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Preliminary driver for Wyse WY-50 and similar display terminals.

    Wyse Technology introduced the WY-50 green screen terminal in the fall of
    1983. It was soon followed by the WY-75 ANSI X3.64-compatible terminal and
    the WY-350 64-color terminal. This generation of terminals quickly replaced
    the earlier WY-100, WY-200 and WY-300.

    The available WY-50 schematics document several revisions of the logic
    board, apparently all functionally equivalent. The earlier version encodes
    character attributes through a slew of TTL gates. A later version
    integrates this logic with a L1A0219 custom gate array (80-435-00), which
    also takes over the address decoding. Both currently dumped sets use this
    second hardware revision (with some minor difference as to the position of
    the beeper). The 80-435-11 gate array from a still later revision also
    generates the dot clock.

    Video memory is contained in two TMS4016-equivalent static RAMs (confirmed
    types include MSM2128-15RS and HM6116P-3). A third 4016-like RAM (usually
    SY2158A-2) is used for the row buffer, with A8-A10 tied to GND.

*******************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/er1400.h"
#include "machine/mc2661.h"
#include "machine/wy50kb.h"
#include "video/scn2674.h"
#include "screen.h"

class wy50_state : public driver_device
{
public:
	wy50_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_keyboard(*this, "keyboard")
		, m_earom(*this, "earom")
		, m_pvtc(*this, "pvtc")
		, m_sio(*this, "sio")
		, m_chargen(*this, "chargen")
		, m_videoram(*this, "videoram%u", 0U)
	{
	}

	void wy50(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	u8 pvtc_videoram_r(offs_t offset);
	SCN2672_DRAW_CHARACTER_MEMBER(draw_character);
	DECLARE_WRITE_LINE_MEMBER(mbc_attr_clock_w);

	u8 pvtc_r(offs_t offset);
	void pvtc_w(offs_t offset, u8 data);
	u8 sio_r(offs_t offset);
	void sio_w(offs_t offset, u8 data);
	u8 rbreg_r();
	void keyboard_w(u8 data);
	void earom_w(u8 data);
	u8 p1_r();
	void p1_w(u8 data);

	void prg_map(address_map &map);
	void io_map(address_map &map);
	void row_buffer_map(address_map &map);

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<wy50_keyboard_device> m_keyboard;
	required_device<er1400_device> m_earom;
	required_device<scn2672_device> m_pvtc;
	required_device<mc2661_device> m_sio;

	required_region_ptr<u8> m_chargen;
	required_shared_ptr_array<u8, 2> m_videoram;

	u8 m_cur_attr;
	u8 m_last_row_attr;
	u8 m_row_buffer_char;
	bool m_font2;
	bool m_rev_prot;
	bool m_is_132;
};

void wy50_state::machine_start()
{
	m_cur_attr = 0;
	m_last_row_attr = 0;
	m_row_buffer_char = 0;
	m_font2 = false;
	m_rev_prot = false;
	m_is_132 = false;

	save_item(NAME(m_cur_attr));
	save_item(NAME(m_last_row_attr));
	save_item(NAME(m_row_buffer_char));
	save_item(NAME(m_font2));
	save_item(NAME(m_rev_prot));
	save_item(NAME(m_is_132));
}

void wy50_state::machine_reset()
{
	keyboard_w(0);
	earom_w(0);
}

u8 wy50_state::pvtc_videoram_r(offs_t offset)
{
	m_row_buffer_char = m_videoram[BIT(offset, 13)][offset & 0x07ff];
	return m_row_buffer_char;
}

SCN2672_DRAW_CHARACTER_MEMBER(wy50_state::draw_character)
{
	// Attribute bit 0 = Dim
	// Attribute bit 1 = Blink
	// Attribute bit 2 = Blank
	// Attribute bit 3 = Underline
	// Attribute bit 4 = Reverse video

	const bool attr = (charcode & 0xe0) == 0x80;
	const bool prot = (charcode & 0xe0) > 0x80 && !m_font2;
	if (attr)
		m_cur_attr = charcode & 0x1f;
	else if (x == 0)
		m_cur_attr = m_last_row_attr;

	u16 dots = 0;
	if (!attr)
	{
		// Blinking suppresses underline but blank attribute doesn't
		if (!BIT(m_cur_attr, 1) || !blink)
		{
			// Shift register load inhibited by blanking conditions or underline
			if (BIT(m_cur_attr, 3) && ul)
				dots = 0x3ff;
			else if (!BIT(m_cur_attr, 2))
				dots = m_chargen[(charcode & (m_font2 ? 0xff : 0x7f)) << 4 | linecount] << 2;
		}

		// Reverse video for non-attribute characters (XOR of two conditions)
		if (BIT(m_cur_attr, 4) != (prot && m_rev_prot))
			dots = ~dots;
	}
	if (cursor)
		dots = ~dots;

	// Apply dimming conditions
	const rgb_t fg = BIT(m_cur_attr, 0) || (prot && !m_rev_prot) ? rgb_t(0xc0, 0xc0, 0xc0) : rgb_t::white();
	for (int i = 0; i < 9; i++)
	{
		bitmap.pix32(y, x++) = BIT(dots, 9) ? fg : rgb_t::black();
		dots <<= 1;
	}
	if (!m_is_132)
		bitmap.pix32(y, x++) = BIT(dots, 9) ? fg : rgb_t::black();
}

WRITE_LINE_MEMBER(wy50_state::mbc_attr_clock_w)
{
	if (state)
		m_last_row_attr = m_cur_attr;
}

u8 wy50_state::pvtc_r(offs_t offset)
{
	return m_pvtc->read(offset >> 8);
}

void wy50_state::pvtc_w(offs_t offset, u8 data)
{
	m_pvtc->write(offset >> 8, data);
}

u8 wy50_state::sio_r(offs_t offset)
{
	return m_sio->read(offset >> 8);
}

void wy50_state::sio_w(offs_t offset, u8 data)
{
	m_sio->write(offset >> 8, data);
}

u8 wy50_state::rbreg_r()
{
	// LS374 row buffer diagnostic register
	return m_row_buffer_char;
}

void wy50_state::keyboard_w(u8 data)
{
	// Bit 0 = J3-6
	// Bit 1 = J3-5
	// Bit 2 = J3-4
	// Bit 3 = J3-7
	// Bit 4 = J3-10
	// Bit 5 = J3-9
	// Bit 6 = J3-8
	// Bit 7 = /HSYNC CLAMP
	m_keyboard->scan_w(data & 0x7f);
}

void wy50_state::earom_w(u8 data)
{
	// Bit 0 = EAROM D
	// Bit 1 = EAROM CLK
	// Bit 2 = EAROM C3
	// Bit 3 = EAROM C2
	// Bit 4 = EAROM C1
	// Bit 5 = UPCHAR/NORM
	m_earom->clock_w(BIT(data, 1));
	m_earom->c3_w(BIT(data, 2));
	m_earom->c2_w(BIT(data, 3));
	m_earom->c1_w(BIT(data, 4));
	m_earom->data_w(BIT(data, 3) ? BIT(data, 0) : 0);
	m_font2 = BIT(data, 5);
}

u8 wy50_state::p1_r()
{
	// P1.0 = AUX RDY
	// P1.1 = NVD OUT
	// P1.4 = KEY (inverted, active high)
	return 0xed | (m_earom->data_r() << 1) | (m_keyboard->sense_r() ? 0x00 : 0x10);
}

void wy50_state::p1_w(u8 data)
{
	// P1.2 = EXFONT
	// P1.3 = AUX RTS
	// P1.5 = BEEPER
	// P1.6 = REV/DIM PROT
	// P1.7 (inverted) = 80/132

	m_rev_prot = BIT(data, 6);

	if (m_is_132 != BIT(data, 7))
	{
		m_is_132 = BIT(data, 7);
		m_pvtc->set_character_width(m_is_132 ? 9 : 10);
		m_pvtc->set_unscaled_clock(68.85_MHz_XTAL / (m_is_132 ? 18 : 30));
	}
}

void wy50_state::prg_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("maincpu", 0);
}

void wy50_state::io_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x1800).ram().share("videoram0");
	map(0x2000, 0x27ff).mirror(0x1800).ram().share("videoram1");
	map(0x4000, 0x47ff).mirror(0x1800).rw(FUNC(wy50_state::pvtc_r), FUNC(wy50_state::pvtc_w));
	map(0x6000, 0x63ff).mirror(0x0c00).r(FUNC(wy50_state::sio_r));
	map(0x7000, 0x73ff).mirror(0x0c00).w(FUNC(wy50_state::sio_w));
	map(0x8000, 0x8000).mirror(0x1fff).r(FUNC(wy50_state::rbreg_r));
	map(0xa000, 0xa000).mirror(0x1fff).w(FUNC(wy50_state::keyboard_w));
	map(0xc000, 0xc000).mirror(0x1fff).w(FUNC(wy50_state::earom_w));
}

void wy50_state::row_buffer_map(address_map &map)
{
	map.global_mask(0x0ff);
	map(0x000, 0x0ff).ram();
}

static INPUT_PORTS_START(wy50)
INPUT_PORTS_END

void wy50_state::wy50(machine_config &config)
{
	I8031(config, m_maincpu, 11_MHz_XTAL); // SAB8031P or SCN8031A
	m_maincpu->set_addrmap(AS_PROGRAM, &wy50_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &wy50_state::io_map);
	m_maincpu->port_in_cb<1>().set(FUNC(wy50_state::p1_r));
	m_maincpu->port_out_cb<1>().set(FUNC(wy50_state::p1_w));

	WY50_KEYBOARD(config, m_keyboard);

	ER1400(config, m_earom);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_raw(68.85_MHz_XTAL / 3, 102 * 10, 0, 80 * 10, 375, 0, 338);
	//screen.set_raw(68.85_MHz_XTAL / 2, 170 * 9, 0, 132 * 9, 375, 0, 338);
	screen.set_screen_update("pvtc", FUNC(scn2672_device::screen_update));

	SCN2672(config, m_pvtc, 68.85_MHz_XTAL / 30); // SCN2672A or SCN2672B
	m_pvtc->set_screen("screen");
	m_pvtc->set_character_width(10); // 9 in 132-column mode
	m_pvtc->set_addrmap(0, &wy50_state::row_buffer_map);
	m_pvtc->set_display_callback(FUNC(wy50_state::draw_character));
	m_pvtc->intr_callback().set_inputline(m_maincpu, MCS51_T0_LINE);
	m_pvtc->breq_callback().set_inputline(m_maincpu, MCS51_INT0_LINE);
	m_pvtc->mbc_callback().set(FUNC(wy50_state::mbc_attr_clock_w));
	m_pvtc->mbc_char_callback().set(FUNC(wy50_state::pvtc_videoram_r));

	MC2661(config, m_sio, 4.9152_MHz_XTAL); // SCN2661B
	m_sio->rxrdy_handler().set_inputline(m_maincpu, MCS51_INT1_LINE);
	m_sio->txd_handler().set("modem", FUNC(rs232_port_device::write_txd));
	m_sio->dtr_handler().set("modem", FUNC(rs232_port_device::write_dtr));
	m_sio->rts_handler().set("modem", FUNC(rs232_port_device::write_rts));

	rs232_port_device &modem(RS232_PORT(config, "modem", default_rs232_devices, "loopback"));
	modem.rxd_handler().set(m_sio, FUNC(mc2661_device::rx_w));
	modem.cts_handler().set(m_sio, FUNC(mc2661_device::cts_w));
	modem.dcd_handler().set(m_sio, FUNC(mc2661_device::dcd_w));
}

ROM_START(wy50)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("2301_e.u6", 0x0000, 0x2000, CRC(2a62ea25) SHA1(f69c596aab307ef1872df29d353b5a61ff77bb74)) // iFD2764-3

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("2201_b.u16", 0x0000, 0x1000, CRC(ee318814) SHA1(0ac64b60ff978e607a087e9e6f4d547811c015c5)) // 2716
ROM_END

ROM_START(wy75) // 8031, green, 101-key detached keyboard, EAROM labeled "MODE 1"
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("wy75_4001r.bin", 0x0000, 0x2000, CRC(d1e660e0) SHA1(81960e7780b86b9fe338b20d7bd50f7e991020a4))

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD("wy75_4101a.bin", 0x0000, 0x1000, CRC(96d377db) SHA1(9e059cf067d84267f4e1d92b0509f137fb2ceb19))
ROM_END

COMP(1984, wy50, 0, 0, wy50, wy50, wy50_state, empty_init, "Wyse Technology", "WY-50 (Rev. E)", MACHINE_IS_SKELETON)
COMP(1984, wy75, 0, 0, wy50, wy50, wy50_state, empty_init, "Wyse Technology", "WY-75 (Rev. H)", MACHINE_IS_SKELETON)
//COMP(1984, wy350, 0, 0, wy50, wy50, wy50_state, empty_init, "Wyse Technology", "WY-350", MACHINE_IS_SKELETON)

// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton driver for Wyse WY-50 and similar display terminals.

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
#include "cpu/mcs51/mcs51.h"
#include "machine/er1400.h"
#include "machine/mc2661.h"
#include "video/scn2674.h"
#include "screen.h"

class wy50_state : public driver_device
{
public:
	wy50_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_earom(*this, "earom")
		, m_pvtc(*this, "pvtc")
		, m_sio(*this, "sio")
		, m_chargen(*this, "chargen")
	{
	}

	void wy50(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	SCN2672_DRAW_CHARACTER_MEMBER(draw_character);

	DECLARE_READ8_MEMBER(pvtc_r);
	DECLARE_WRITE8_MEMBER(pvtc_w);
	DECLARE_READ8_MEMBER(sio_r);
	DECLARE_WRITE8_MEMBER(sio_w);
	u8 rbreg_r();
	void keyboard_w(u8 data);
	void earom_w(u8 data);
	u8 p1_r();
	void p1_w(u8 data);

	void prg_map(address_map &map);
	void io_map(address_map &map);

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<er1400_device> m_earom;
	required_device<scn2672_device> m_pvtc;
	required_device<mc2661_device> m_sio;

	required_region_ptr<u8> m_chargen;
};

void wy50_state::machine_start()
{
}

void wy50_state::machine_reset()
{
	keyboard_w(0);
	earom_w(0);
}

SCN2672_DRAW_CHARACTER_MEMBER(wy50_state::draw_character)
{
}

READ8_MEMBER(wy50_state::pvtc_r)
{
	return m_pvtc->read(space, offset >> 8);
}

WRITE8_MEMBER(wy50_state::pvtc_w)
{
	m_pvtc->write(space, offset >> 8, data);
}

READ8_MEMBER(wy50_state::sio_r)
{
	return m_sio->read(space, offset >> 8);
}

WRITE8_MEMBER(wy50_state::sio_w)
{
	m_sio->write(space, offset >> 8, data);
}

u8 wy50_state::rbreg_r()
{
	// LS374 row buffer diagnostic register
	return 0;
}

void wy50_state::keyboard_w(u8 data)
{
}

void wy50_state::earom_w(u8 data)
{
	m_earom->clock_w(BIT(data, 1));
	m_earom->c3_w(BIT(data, 2));
	m_earom->c2_w(BIT(data, 3));
	m_earom->c1_w(BIT(data, 4));
	m_earom->data_w(BIT(data, 3) ? BIT(data, 0) : 0);
	// Bit 5 = UPCHAR/NORM
}

u8 wy50_state::p1_r()
{
	// P1.0 = AUX RDY
	// P1.1 = NVD OUT
	// P1.4 = KEY (inverted)
	return 0xfd | (m_earom->data_r() << 1);
}

void wy50_state::p1_w(u8 data)
{
	// P1.2 = EXFONT
	// P1.3 = AUX RTS
	// P1.5 = BEEPER
	// P1.6 = REV/DIM PROT
	// P1.7 (inverted) = 80/132
}

void wy50_state::prg_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("maincpu", 0);
}

void wy50_state::io_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x1800).ram();
	map(0x2000, 0x27ff).mirror(0x1800).ram();
	map(0x4000, 0x47ff).mirror(0x1800).rw(FUNC(wy50_state::pvtc_r), FUNC(wy50_state::pvtc_w));
	map(0x6000, 0x63ff).mirror(0x1c00).rw(FUNC(wy50_state::sio_r), FUNC(wy50_state::sio_w));
	map(0x8000, 0x8000).mirror(0x1fff).r(FUNC(wy50_state::rbreg_r));
	map(0xa000, 0xa000).mirror(0x1fff).w(FUNC(wy50_state::keyboard_w));
	map(0xc000, 0xc000).mirror(0x1fff).w(FUNC(wy50_state::earom_w));
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

	ER1400(config, m_earom);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	screen.set_raw(68.85_MHz_XTAL / 3, 102 * 10, 0, 80 * 10, 375, 0, 338);
	//screen.set_raw(68.85_MHz_XTAL / 2, 170 * 9, 0, 132 * 9, 375, 0, 338);
	screen.set_screen_update("pvtc", FUNC(scn2672_device::screen_update));

	SCN2672(config, m_pvtc, 68.85_MHz_XTAL / 30); // SCN2672A or SCN2672B
	m_pvtc->set_screen("screen");
	m_pvtc->set_character_width(10); // 9 in 132-column mode
	m_pvtc->set_display_callback(FUNC(wy50_state::draw_character));
	m_pvtc->intr_callback().set_inputline(m_maincpu, MCS51_T0_LINE);
	m_pvtc->breq_callback().set_inputline(m_maincpu, MCS51_INT0_LINE);

	MC2661(config, m_sio, 4.9152_MHz_XTAL); // SCN2661B
	m_sio->txrdy_handler().set_inputline(m_maincpu, MCS51_INT1_LINE);
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

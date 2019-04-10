// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton driver for Wyse WY-85 VT220-compatible terminal.

    Unlike most later Wyse terminals, the WY-85 lacks a video gate array.
    However, the 105-key keyboard has its own gate array.

*******************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/er1400.h"
#include "machine/mc68681.h"
#include "video/scn2674.h"
#include "screen.h"

class wy85_state : public driver_device
{
public:
	wy85_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_earom(*this, "earom")
		, m_pvtc(*this, "pvtc")
		, m_duart(*this, "duart")
		, m_chargen(*this, "chargen")
	{
	}

	void wy85(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	SCN2672_DRAW_CHARACTER_MEMBER(draw_character);

	u8 pvtc_r(offs_t offset);
	void pvtc_w(offs_t offset, u8 data);
	u8 duart_r(offs_t offset);
	void duart_w(offs_t offset, u8 data);
	void earom_w(u8 data);
	u8 misc_r();
	u8 p1_r();
	void p1_w(u8 data);
	void p3_w(u8 data);

	void prg_map(address_map &map);
	void io_map(address_map &map);

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<er1400_device> m_earom;
	required_device<scn2672_device> m_pvtc;
	required_device<scn2681_device> m_duart;

	required_region_ptr<u8> m_chargen;
};

void wy85_state::machine_start()
{
}

void wy85_state::machine_reset()
{
	earom_w(0);
}

SCN2672_DRAW_CHARACTER_MEMBER(wy85_state::draw_character)
{
}

u8 wy85_state::pvtc_r(offs_t offset)
{
	return m_pvtc->read(offset >> 8);
}

void wy85_state::pvtc_w(offs_t offset, u8 data)
{
	m_pvtc->write(offset >> 8, data);
}

u8 wy85_state::duart_r(offs_t offset)
{
	return m_duart->read(offset >> 8);
}

void wy85_state::duart_w(offs_t offset, u8 data)
{
	m_duart->write(offset >> 8, data);
}

void wy85_state::earom_w(u8 data)
{
	// SN74LS174N latch + 7406 inverter
	m_earom->clock_w(BIT(data, 1));
	m_earom->c3_w(BIT(data, 2));
	m_earom->c2_w(BIT(data, 3));
	m_earom->c1_w(BIT(data, 4));
	m_earom->data_w(BIT(data, 3) ? BIT(data, 0) : 0);
}

u8 wy85_state::misc_r()
{
	// Bit 2 = EAROM output
	// Bit 3 = keyboard return line?
	return m_earom->data_r() << 2;
}

u8 wy85_state::p1_r()
{
	return 0xff;
}

void wy85_state::p1_w(u8 data)
{
	// P1.7 = 80/132 column switch
}

void wy85_state::p3_w(u8 data)
{
	// P3.5 (T1) = keyboard clocK?
}

void wy85_state::prg_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("maincpu", 0);
}

void wy85_state::io_map(address_map &map)
{
	map(0x0000, 0x1fff).ram(); // 4x HM6116P-3 (with 2 more in other capacities)
	map(0x2000, 0x2000).mirror(0xff).w(FUNC(wy85_state::earom_w));
	map(0x4000, 0x47ff).rw(FUNC(wy85_state::pvtc_r), FUNC(wy85_state::pvtc_w));
	map(0x6000, 0x6fff).rw(FUNC(wy85_state::duart_r), FUNC(wy85_state::duart_w));
	map(0xa000, 0xa000).mirror(0xff).r(FUNC(wy85_state::misc_r));
}

static INPUT_PORTS_START(wy85)
INPUT_PORTS_END

void wy85_state::wy85(machine_config &config)
{
	I8032(config, m_maincpu, 11_MHz_XTAL); // SCN8032H
	m_maincpu->set_addrmap(AS_PROGRAM, &wy85_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &wy85_state::io_map);
	m_maincpu->port_in_cb<1>().set(FUNC(wy85_state::p1_r));
	m_maincpu->port_out_cb<1>().set(FUNC(wy85_state::p1_w));
	m_maincpu->port_out_cb<3>().set(FUNC(wy85_state::p3_w));

	ER1400(config, m_earom); // M5G1400

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_color(rgb_t::green());
	//screen.set_raw(48.5568_MHz_XTAL / 3, 96 * 10, 0, 80 * 10, 281, 0, 260);
	screen.set_raw(48.5568_MHz_XTAL / 2, 160 * 9, 0, 132 * 9, 281, 0, 260);
	screen.set_screen_update("pvtc", FUNC(scn2672_device::screen_update));

	SCN2672(config, m_pvtc, 48.5568_MHz_XTAL / 30); // SCN2672B
	m_pvtc->set_screen("screen");
	m_pvtc->set_character_width(9); // 10 in 80-column mode
	m_pvtc->set_display_callback(FUNC(wy85_state::draw_character));
	//m_pvtc->intr_callback().set_inputline(m_maincpu, MCS51_T0_LINE);
	m_pvtc->breq_callback().set_inputline(m_maincpu, MCS51_INT0_LINE);

	SCN2681(config, m_duart, 3.6864_MHz_XTAL); // SCN2681A (+ 4x µA9636ATC drivers and UA9639CP receivers)
	m_duart->irq_cb().set_inputline(m_maincpu, MCS51_INT1_LINE);
}

ROM_START(wy85)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("250151-04_reva.5e", 0x0000, 0x4000, CRC(8fcb9f43) SHA1(6c7e1d27fa6014870c29ab2b8b856ae412bfc411)) // 27128

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD("am9265.1h", 0x0000, 0x2000, CRC(5ee65b55) SHA1(a0b38a38838f262aaea22d212351e7441e4b07e8)) // AM9265EPC
ROM_END

COMP(1985, wy85, 0, 0, wy85, wy85, wy85_state, empty_init, "Wyse Technology", "WY-85 (Rev. A)", MACHINE_IS_SKELETON)

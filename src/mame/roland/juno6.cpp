// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Roland Juno-6 analog synthesizer.

    The original JU-6 had no MIDI support, but this feature could be and was
    added through retrofits since the same 8049 program supports MIDI on the
    Juno-60.

****************************************************************************/

#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/74259.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"


namespace {

class juno6_state : public driver_device
{
public:
	juno6_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pit(*this, "pit%u", 1U)
		, m_latch(*this, "latch")
		, m_usart(*this, "usart")
	{
	}

	void juno6(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u8 keyboard_r();
	u8 db_r();
	void db_w(u8 data);
	void p2_w(u8 data);

	void ext_map(address_map &map) ATTR_COLD;

	required_device<mcs48_cpu_device> m_maincpu;
	required_device_array<pit8253_device, 2> m_pit;
	required_device<cd4099_device> m_latch;
	required_device<i8251_device> m_usart;

	u8 m_db = 0;
};

void juno6_state::machine_start()
{
	save_item(NAME(m_db));
}


u8 juno6_state::keyboard_r()
{
	return 0;
}

u8 juno6_state::db_r()
{
	u8 p2 = m_maincpu->p2_r();

	if (!BIT(p2, 3))
		return m_usart->read(p2 & 0x01);

	return 0xff;
}

void juno6_state::db_w(u8 data)
{
	u8 p2 = m_maincpu->p2_r();

	if (!BIT(p2, 3))
		m_usart->write(p2 & 0x01, data);
	if (!BIT(p2, 4))
		m_pit[0]->write(p2 & 0x03, data);
	if (!BIT(p2, 5))
		m_pit[1]->write(p2 & 0x03, data);

	m_db = data;
}

void juno6_state::p2_w(u8 data)
{
	if (!BIT(data, 7))
		m_latch->write_bit(data & 0x07, BIT(m_db, 7));
}

void juno6_state::ext_map(address_map &map)
{
	map(0x00, 0x00).mirror(0xff).r(FUNC(juno6_state::db_r));
}


static INPUT_PORTS_START(juno6)
INPUT_PORTS_END

void juno6_state::juno6(machine_config &config)
{
	I8049(config, m_maincpu, 11_MHz_XTAL);
	m_maincpu->set_addrmap(AS_IO, &juno6_state::ext_map);
	m_maincpu->p1_in_cb().set(FUNC(juno6_state::keyboard_r));
	m_maincpu->bus_out_cb().set(FUNC(juno6_state::db_w));
	m_maincpu->p2_out_cb().set(FUNC(juno6_state::p2_w));

	PIT8253(config, m_pit[0]);
	PIT8253(config, m_pit[1]);

	CD4099(config, m_latch);

	I8251(config, m_usart, 4_MHz_XTAL / 2);
	m_usart->rts_handler().set(m_usart, FUNC(i8251_device::write_cts));

	clock_device &midiclock(CLOCK(config, "midiclock", 4_MHz_XTAL / 8));
	midiclock.signal_handler().set(m_usart, FUNC(i8251_device::write_txc));
	midiclock.signal_handler().append(m_usart, FUNC(i8251_device::write_txc));
}

ROM_START(juno6)
	ROM_REGION(0x800, "maincpu", 0)
	ROM_LOAD("upd8049c-238.ic56", 0x000, 0x800, CRC(18fa7cc5) SHA1(eebda5bbe6c3153a5549de1853e69da403811df3))
ROM_END

} // anonymous namespace


SYST(1982, juno6, 0, 0, juno6, juno6, juno6_state, empty_init, "Roland", "Juno-6 (JU-6) Polyphonic Synthesizer", MACHINE_IS_SKELETON)

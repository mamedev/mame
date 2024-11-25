// license:BSD-3-Clause
// copyright-holders:AJR
/*
    Skeleton driver for Cromptons Leisure Machines' Frantic Fruits redemption game.

    Hardware overview:
    Main CPU: TS80C32X2-MCA
    Sound: CD sound?
    Other: M48T08 timekeeper RAM
    OSC: 11.0592 MHz
    Dips: 2 x 8 dips banks

    Video: https://www.youtube.com/watch?v=89XJpor9dSQ

    Other games using the same PCB include:
    * Casino Nights (coin pusher)
    * Chuck Rock (coin pusher)
    * Disco Fever (coin pusher)
    * Hi Ball (redemption)
    * Pharaoh's Treasure (coin pusher)
    * Pirates Treasure (coin pusher)
    * Rock 'n' Roll II (coin pusher)
    * Royal Casino (coin pusher)
    * Summer Time (coin pusher)
    * Wheel of Fortune (coin pusher)
*/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/74259.h"
#include "machine/timekpr.h"
#include "screen.h"


namespace {

class cromptons_state : public driver_device
{
public:
	cromptons_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_outlatch(*this, "outlatch%u", 0U)
		, m_inputs(*this, "IN%u", 0U)
	{ }

	void cromptons(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u8 port_r();
	void port_w(u8 data);

	required_device<mcs51_cpu_device> m_maincpu;
	required_device_array<hc259_device, 4> m_outlatch;
	required_ioport_array<4> m_inputs;

	void prg_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	u8 m_port_select = 0;
};

void cromptons_state::machine_start()
{
	save_item(NAME(m_port_select));
}

u8 cromptons_state::port_r()
{
	return BIT(m_inputs[(m_port_select & 0x18) >> 3]->read(), m_port_select & 0x07) ? 0xff : 0x7f;
}

void cromptons_state::port_w(u8 data)
{
	if (!BIT(data, 6))
		m_outlatch[(data & 0x18) >> 3]->write_bit(data & 0x07, BIT(data, 5));

	m_port_select = data;
}

void cromptons_state::prg_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region("maincpu", 0);
}

void cromptons_state::io_map(address_map &map)
{
	map(0xe000, 0xffff).rw("timekpr", FUNC(timekeeper_device::read), FUNC(timekeeper_device::write));
}

static INPUT_PORTS_START( cromptons )
	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("IN1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("IN2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")

	PORT_START("IN3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


void cromptons_state::cromptons(machine_config &config)
{
	/* basic machine hardware */
	I80C32(config, m_maincpu, 11.0592_MHz_XTAL); // TS80C32X2-MCA
	m_maincpu->set_addrmap(AS_PROGRAM, &cromptons_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &cromptons_state::io_map);
	m_maincpu->port_in_cb<1>().set(FUNC(cromptons_state::port_r));
	m_maincpu->port_out_cb<1>().set(FUNC(cromptons_state::port_w));

	MK48T08(config, "timekpr");

	HC259(config, m_outlatch[0]); // types not verified
	HC259(config, m_outlatch[1]);
	HC259(config, m_outlatch[2]);
	HC259(config, m_outlatch[3]);

	// sound ??
}

/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( ffruits )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "frntf5v6.ic11", 0x00000, 0x10000, CRC(ca60c557) SHA1(6f356827f0c93ec0376a7edc03963ef0748dccdb) ) // 27c512

	ROM_REGION(0x2000, "timekpr", 0)
	ROM_LOAD( "frntf_m48t08-100pc1.ic12", 0x0000, 0x2000, CRC(19f1c93a) SHA1(4fb01299c72e50c91af80939f8ffa8a8447f5211) )
ROM_END

} // anonymous namespace


GAME( 2000, ffruits,  0,   cromptons, cromptons, cromptons_state, empty_init, ROT0, "Cromptons Leisure Machines", "Frantic Fruits",  MACHINE_IS_SKELETON_MECHANICAL )

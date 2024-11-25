// license:BSD-3-Clause
// copyright-holders:

/*
Diga Mart (Sega, 1983)

It's a crane machine with prizes on a moving track/turntable under a dome,
with 4 player positions surrounding it.

Stickers on PCB:

834-5781
86012400242K

Main chips on PCB:

1x Z8400APS
1x HM6116LP-3
1x M5L8255AP-5
2x SN76489AN
1x 8 MHx XTAL
1x 8-dip bank
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/sn76496.h"

#include "speaker.h"


namespace {

class digamart_state : public driver_device
{
public:
	digamart_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{
	}

	void digamart(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void program_map(address_map &map) ATTR_COLD;
};


void digamart_state::program_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("maincpu", 0);
	map(0x2000, 0x27ff).ram();
	// map(0x4000, 0x400f).w()
	map(0x6000, 0x6003).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	// map(0x8000, 0x8000).r();
	// map(0xa000, 0xa000).r();
	map(0xc000, 0xc003).w("sn1", FUNC(sn76496_device::write));
	map(0xe000, 0xe003).w("sn2", FUNC(sn76496_device::write));
}


static INPUT_PORTS_START( digamart ) // 4 players, 2 buttons each
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


void digamart_state::digamart(machine_config &config)
{
	Z80(config, m_maincpu, 8_MHz_XTAL / 2); // divisor not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &digamart_state::program_map);
	m_maincpu->set_periodic_int(FUNC(digamart_state::irq0_line_hold), attotime::from_hz(4 * 60)); // TODO: wrong

	i8255_device &ppi(I8255(config, "ppi"));
	ppi.in_pa_callback().set([this] () { logerror("%s: PA read\n", machine().describe_context()); return uint8_t(0); });
	ppi.out_pa_callback().set([this] (uint8_t data) { logerror("%s: PA write %02X\n", machine().describe_context(), data); });
	ppi.in_pb_callback().set([this] () { logerror("%s: PB read\n", machine().describe_context()); return uint8_t(0); });
	ppi.out_pb_callback().set([this] (uint8_t data) { logerror("%s: PB write %02X\n", machine().describe_context(), data); });
	ppi.in_pc_callback().set([this] () { logerror("%s: PC read\n", machine().describe_context()); return uint8_t(0); });
	ppi.out_pc_callback().set([this] (uint8_t data) { logerror("%s: PC write %02X\n", machine().describe_context(), data); });


	SPEAKER(config, "mono").front_center();

	SN76489A(config, "sn1", 8_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.5); // divisor not verified

	SN76489A(config, "sn2", 8_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.5); // divisor not verified
}


ROM_START( digamart )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "epr-6970a.ic26", 0x0000, 0x2000, CRC(507de646) SHA1(a6934b3fdb0768691f3513d27767a99882078a1d) ) // handwritten label
ROM_END

} // anonymous namespace


GAME( 1983, digamart, 0, digamart, digamart, digamart_state, empty_init, ROT0, "Sega", "Diga Mart (rev. A)", MACHINE_IS_SKELETON_MECHANICAL )

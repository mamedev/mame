// license:BSD-3-Clause
// copyright-holders:

/*
ドラネコバンバン - Dora Neco BanBan (as transliterated on the cab)
Medal game by Kato's, whack-a-mole style

PCB is unmarked

Main components:
Sharp LH0080B Z80B-CPU
12.000 MHz XTAL
HM6116LP-3 Static RAM
2x NEC D71055C
OKI M6295GS
4-DIP bank
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/okim6295.h"

#include "speaker.h"


namespace {

class katosmedz80_state : public driver_device
{
public:
	katosmedz80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu (*this, "maincpu")
	{}

	void dnbanban(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	void program_map(address_map &map);
	void io_map(address_map &map);
};


void katosmedz80_state::program_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
}

void katosmedz80_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ppi0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x04, 0x07).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	// map(0x08, 0x08) // ??
	map(0x0c, 0x0c).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}


static INPUT_PORTS_START( dnbanban )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW") // 4-DIP bank
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void katosmedz80_state::dnbanban(machine_config &config)
{
	Z80(config, m_maincpu, 12_MHz_XTAL / 4); // divider unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &katosmedz80_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &katosmedz80_state::io_map);
	m_maincpu->set_periodic_int(FUNC(katosmedz80_state::irq0_line_hold), attotime::from_hz(4*60)); // wrong

	I8255(config, "ppi0"); // D71055C

	I8255(config, "ppi1"); // D71055C

	// 2x LEDs

	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 1.056_MHz_XTAL, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 0.65); // resonator value and pin 7 verified
}


ROM_START( dnbanban )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "g25_a.ic17", 0x0000, 0x8000, CRC(ef441127) SHA1(69fea4992abb2c4905d3831b6f18e464088f0ec7) ) // MBM27C256A, 1xxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "g25_v.ic7", 0x00000, 0x20000, CRC(87c7d45d) SHA1(3f035d5e62fe62111cee978ed1708e902c98526a) ) // MBM27C1000
ROM_END

} // anonymous namespace


GAME( 1993, dnbanban, 0, dnbanban, dnbanban, katosmedz80_state, empty_init, ROT0, "Kato's", "Dora Neco BanBan", MACHINE_IS_SKELETON_MECHANICAL )

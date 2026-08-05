// license:BSD-3-Clause
// copyright-holders:

/*
ハローキティの お城へいこう (Hello Kitty no Oshiro e Ikō) (Let’s Go to Hello Kitty’s Castle)

CPU-A PCB

Z0840004PSC CPU
4.0000 MHz XTAL
LH5116-15 RAM
NE555P
2x TMP82C265BF-10 PPI (datasheet describes one as almost equivalent to 2x 8255)
YM2413 OPL
OKI M6295 ADPCM
bank of 8 switches
4x connector
*/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"

#include "screen.h"
#include "speaker.h"


namespace {

class hkcastle_state : public driver_device
{
public:
	hkcastle_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void hkcastle(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


void hkcastle_state::program_map(address_map &map)
{
	map(0x0000, 0x9fff).rom();
	map(0xd000, 0xd7ff).ram();
	// map(0xf000, 0xf000).rw(); // IRQ ack?
}

void hkcastle_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
}


static INPUT_PORTS_START( hkcastle )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) ) PORT_DIPLOCATION("DS1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("DS1:3,4")
	PORT_DIPSETTING(    0x0c, "Level 1 (simplest)" )
	PORT_DIPSETTING(    0x08, "Level 2" )
	PORT_DIPSETTING(    0x04, "Level 3" )
	PORT_DIPSETTING(    0x00, "Level 4 (most difficult)" )
	PORT_DIPNAME( 0x10, 0x10, "Demo Mode" ) PORT_DIPLOCATION("DS1:5") // ゲームデモ ?
	PORT_DIPSETTING(    0x10, "Demo List" ) // デモリ ?
	PORT_DIPSETTING(    0x00, "Demo Screen" ) // デモナシ ?
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) ) PORT_DIPLOCATION("DS1:6") // the following are unused according to the manual
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) ) PORT_DIPLOCATION("DS1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) ) PORT_DIPLOCATION("DS1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


void hkcastle_state::hkcastle(machine_config &config)
{
	Z80(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &hkcastle_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &hkcastle_state::io_map);
	m_maincpu->set_periodic_int(FUNC(hkcastle_state::irq0_line_hold), attotime::from_hz(60*4)); // TODO: proper IRQs, possibly from the NE555?

	I8255A(config, "ppi0.a"); // first TMP82C265BF-10
	I8255A(config, "ppi0.b");

	I8255A(config, "ppi1.a"); // second TMP82C265BF-10
	I8255A(config, "ppi1.b");

	SPEAKER(config, "mono").front_center();

	YM2413(config, "ym", 4_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.5); // absence of divider not verified

	OKIM6295(config, "oki", 4_MHz_XTAL / 4, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.5); // divider and pin 7 not verified
}


ROM_START( hkcastle )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hkaa.ic26", 0x00000, 0x10000, CRC(e76c28e1) SHA1(de1612ff3cfb361580174b0c6c6ab7ddca69851b) )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE00 )
	ROM_LOAD( "am27c010.ic12", 0x00000, 0x20000, CRC(18d2feb1) SHA1(6e2f57c1deb188ea589b44bcc27bc59299bb7412) )
ROM_END

} // anonymous namespace


GAME( 1998, hkcastle,  0, hkcastle, hkcastle, hkcastle_state, empty_init, ROT0, "I'MAX", "Hello Kitty no Oshiro e Iko", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL ) // date taken from the manual

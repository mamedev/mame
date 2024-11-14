// license:BSD-3-Clause
// copyright-holders:

/*
Sega UFO Catcher on H8 based hardware

Stickers on PCB:

GAME BD UCS
834-14256
S/NO. 103294


REV.B

Chips on PCB:

1x SEGA 315-5338A
1x SEGA 315-5296
1x H8/3007 2D3  HD6413007F20 JAPAN
1x YAMAHA YM3438
*/

#include "emu.h"

#include "315_5296.h"
#include "315_5338a.h"

#include "cpu/h8/h83006.h"
#include "sound/ymopn.h"

#include "speaker.h"


namespace {

class segaufoh8_state : public driver_device
{
public:
	segaufoh8_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void segaufoh8(machine_config &config);

private:
	required_device<h83007_device> m_maincpu;

	void program_map(address_map &map) ATTR_COLD;
};


void segaufoh8_state::program_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("maincpu", 0);
	//map(0x?00000, 0x?0007f).rw("315_5296", FUNC(sega_315_5296_device::read), FUNC(sega_315_5296_device::write)).umask16(0xff00);
	//map(0x400000, 0x40001f).rw("315_5338a", FUNC(sega_315_5338a_device::read), FUNC(sega_315_5338a_device::write)).umask16(0xff00);
}


static INPUT_PORTS_START( ufo7 )
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1") // only one bank of dips on PCB, but there is a second empty space
	PORT_DIPNAME( 0x01, 0x01, "UNK1-01" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "UNK1-02" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "UNK1-04" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "UNK1-08" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "UNK1-10" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "UNK1-20" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "UNK1-40" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "UNK1-80" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


void segaufoh8_state::segaufoh8(machine_config &config)
{
	H83007(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &segaufoh8_state::program_map);

	SEGA_315_5296(config, "315_5296", 8_MHz_XTAL);

	SEGA_315_5338A(config, "315_5338a", 32_MHz_XTAL);

	SPEAKER(config, "mono").front_center();

	ym3438_device &ym(YM3438(config, "ym", 16_MHz_XTAL / 2)); // divider not verified
	//ym.irq_handler().set_inputline("maincpu", 0);
	ym.add_route(ALL_OUTPUTS, "mono", 0.40);
}


ROM_START( ufo7 )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD16_WORD_SWAP( "epr-23795b.ic408", 0x00000, 0x80000, CRC(ca153fc7) SHA1(f21878deaff2c86896912a8a0a4b17b44e5ef65c) )
ROM_END

} // anonymous namespace


GAME( 2001, ufo7, 0, segaufoh8, ufo7, segaufoh8_state, empty_init, ROT0, "Sega", "UFO Catcher 7", MACHINE_IS_SKELETON_MECHANICAL ) // UFO Catch 7 V20011112 in string in ROM

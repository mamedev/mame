// license:BSD-3-Clause
// copyright-holders:

/*
プチ☆ロット (Petit Lot) by Showa Giken (Shoken) - mechanical slot machine
Flyer shows at least two game titles: Time Cross and Hyper Rush.
Are those actually different ROMs or just different covers?

'MD06 MAIN' PCB:
KL5C80A12CFP CPU
IC62C256-70U SRAM (battery backed) under program ROM
RTC62423-A RTC
bank of 8 DIP switches
bank of 4 DIP switches
5 10-position rotary switches
push-button (reset?)
16.00000 MHz XTAL
unpopulates spaces marked for MSM9810B and ROM

DIP sheets are available

ポーラスター2 (Polar Star 2) by Showa Giken (Shoken)
'MB-01 MAIN-B' PCB:
KL5C80A12CFP CPU
RTC62423 RTC
push-button (reset?)
16.00000 MHz XTAL
*/


#include "emu.h"

#include "cpu/z80/kl5c80a12.h"
#include "machine/msm6242.h"
#include "sound/okim9810.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class shoken_md06_state : public driver_device
{
public:
	shoken_md06_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void petitlot(machine_config &config) ATTR_COLD;
	void polarstar(machine_config &config) ATTR_COLD;

private:
	required_device<kl5c80a12_device> m_maincpu;

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
};


void shoken_md06_state::program_map(address_map &map)
{
	map(0x00000, 0x08fff).rom();
	map(0x09000, 0x0cfff).ram();
	map(0xe0000, 0xe3fff).ram();
	map(0xfcc00, 0xfffff).ram();
}

void shoken_md06_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x40, 0x40).nopw();
	map(0x80, 0x80).noprw();
}

void shoken_md06_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x3b, 0x3b).rw("oki", FUNC(okim9810_device::read), FUNC(okim9810_device::write));
	map(0x40, 0x40).nopw();
	map(0x80, 0x80).noprw();
}

static INPUT_PORTS_START( petitlot )
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

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIP-A:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIP-A:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIP-A:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIP-A:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIP-A:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIP-A:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIP-A:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIP-A:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2") // 4 DIPs
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIP-B:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIP-B:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIP-B:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DIP-B:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT(           0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// TODO: 5 10-position rotary switches
INPUT_PORTS_END

static INPUT_PORTS_START( polarstar )
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
INPUT_PORTS_END


void shoken_md06_state::petitlot(machine_config &config)
{
	// basic machine hardware
	KL5C80A12(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &shoken_md06_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &shoken_md06_state::io_map);

	RTC62423(config, "rtc", 0);
}

void shoken_md06_state::polarstar(machine_config &config)
{
	petitlot(config);
	m_maincpu->set_addrmap(AS_IO, &shoken_md06_state::sound_io_map);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	okim9810_device &oki(OKIM9810(config, "oki", 16_MHz_XTAL / 12));
	oki.add_route(ALL_OUTPUTS, "mono", 1.0); //divider guessed
}

ROM_START( petitlot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "md06_ver4_1.ic6", 0x00000, 0x10000, CRC(ebc81f10) SHA1(28ac52aeadfbf792da95c01b16fb88f7a5eb1d4e) ) // 1xxxxxxxxxxxxxxx = 0xff
ROM_END

ROM_START( polstar2 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "polarstar2_mk04_ver.4.5.ic8", 0x00000, 0x20000, CRC(13a7b9dc) SHA1(64aa5591aa676cbb9e00327ac62209b6854c4416) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "polarstar_mb01_sound-b.ic35", 0x00000, 0x100000, CRC(606ae52c) SHA1(6a60f4b1c6ac893cafa373c8af7d2c826304f152) )
ROM_END

} // anonymous namespace

GAME( 2001, petitlot,   0, petitlot,  petitlot,  shoken_md06_state, empty_init, ROT0, "Shoken", "Petit Lot (ver. 4.1)",    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
GAME( 2004, polstar2,   0, polarstar, polarstar, shoken_md06_state, empty_init, ROT0, "Shoken", "Polar Star 2 (ver. 4.5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL )

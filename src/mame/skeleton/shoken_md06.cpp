// license:BSD-3-Clause
// copyright-holders:

/*
This driver currently contains 3 different types of PCBs, which all share the same CPU.

---

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
unpopulated spaces marked for MSM9810B and ROM

DIP sheets are available

---

ポーラスター Fantasy Island (Polar Star Fantasy Island) by Showa Giken (Shoken)
ポーラスター Moorry Fantasy (Polar Star Moorry Fantasy ) by Showa Giken (Shoken)
ポーラスター2 (Polar Star 2) by Showa Giken (Shoken)

'MB-01 MAIN-B' PCB:
KL5C80A12CFP CPU
RTC62423 RTC
OKIM9810B
push-button (reset?)
16.00000 MHz XTAL

---

Silver Rush by Showa Giken (Shoken)

'ML01MAIN-1' PCB
KL5C80A12C2 CPU
16.000 MHz XTAL
OKIM9810B
4x bank of 8 DIP switches
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
	map(0x00000, 0x0ffff).rom();
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
	map.unmap_value_high();

	map(0x40, 0x40).rw("oki", FUNC(okim9810_device::read), FUNC(okim9810_device::write)); // w?
	map(0x41, 0x41).w("oki", FUNC(okim9810_device::write_tmp_register)); // ?
	// map(0x40, 0x40).nopw(); // rw?
	// map(0x41, 0x41).nopw(); // w?
	map(0x50, 0x5f).rw("rtc", FUNC(rtc62423_device::read), FUNC(rtc62423_device::write));
	// map(0x80, 0x80) // r?
	// map(0x88, 0x88) // r?
	// map(0x90, 0x90) // r?
	// map(0x98, 0x98) // rw?
	// map(0xa0, 0xa0) // w?
	// map(0xa8, 0xa8) // w?
	map(0xe0, 0xe0).nopw(); // w?
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
	m_maincpu->in_p0_callback().set([this] () { logerror("%s CPU port 0 read\n", machine().describe_context()); return (uint8_t)0; });
	m_maincpu->out_p0_callback().set([this] (uint8_t data) { logerror("%s CPU port 0 write: %02x\n", machine().describe_context(), data); });
	m_maincpu->in_p1_callback().set([this] () { logerror("%s CPU port 1 read\n", machine().describe_context()); return (uint8_t)0; });
	m_maincpu->out_p1_callback().set([this] (uint8_t data) { logerror("%s CPU port 1 write: %02x\n", machine().describe_context(), data); });
	m_maincpu->in_p2_callback().set([this] () { logerror("%s CPU port 2 read\n", machine().describe_context()); return (uint8_t)0; });
	m_maincpu->out_p2_callback().set([this] (uint8_t data) { logerror("%s CPU port 2 write: %02x\n", machine().describe_context(), data); });
	m_maincpu->in_p3_callback().set([this] () { logerror("%s CPU port 3 read\n", machine().describe_context()); return (uint8_t)0; });
	m_maincpu->out_p3_callback().set([this] (uint8_t data) { logerror("%s CPU port 3 write: %02x\n", machine().describe_context(), data); }); // this is used
	m_maincpu->in_p4_callback().set([this] () { logerror("%s CPU port 4 read\n", machine().describe_context()); return (uint8_t)0; });
	m_maincpu->out_p4_callback().set([this] (uint8_t data) { logerror("%s CPU port 4 write: %02x\n", machine().describe_context(), data); });

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

// the following runs on the 'MD06 MAIN' PCB
ROM_START( petitlot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "md06_ver4_1.ic6", 0x00000, 0x10000, CRC(ebc81f10) SHA1(28ac52aeadfbf792da95c01b16fb88f7a5eb1d4e) ) // 1xxxxxxxxxxxxxxx = 0xFF
ROM_END

// the following sets run on the 'MB-01 MAIN-B' PCB
// they all share the same Oki samples ROM
// polstrfi and polstrmf have almost identical program ROMs. Probably just cab re-skins.
ROM_START( polstrfi )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "polarstar_fantasy_island_ver_3,2b.ic8", 0x00000, 0x20000, CRC(b63190d8) SHA1(370452857138c3afff5ee7f71fbbf2add27a2208) ) // 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "polarstar_mb01_sound-b.ic35", 0x000000, 0x100000, CRC(606ae52c) SHA1(6a60f4b1c6ac893cafa373c8af7d2c826304f152) )
ROM_END

ROM_START( polstrmf )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "polarstar_moorry_fantasy_ver_3,2b.ic8", 0x00000, 0x20000, CRC(015bedff) SHA1(567ed0c50f55c95a307c25bf8849138a16954ee7) ) // 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "polarstar_mb01_sound-b.ic35", 0x000000, 0x100000, CRC(606ae52c) SHA1(6a60f4b1c6ac893cafa373c8af7d2c826304f152) )
ROM_END

ROM_START( polstar2 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "polarstar2_mk04_ver.4.5.ic8", 0x00000, 0x20000, CRC(13a7b9dc) SHA1(64aa5591aa676cbb9e00327ac62209b6854c4416) )

	ROM_REGION( 0x100000, "oki", 0 )
	ROM_LOAD( "polarstar_mb01_sound-b.ic35", 0x000000, 0x100000, CRC(606ae52c) SHA1(6a60f4b1c6ac893cafa373c8af7d2c826304f152) )
ROM_END

// the following runs on the 'ML01MAIN-1' PCB. It's probably missing at least a video PCB, given reference video: https://www.youtube.com/watch?v=eTAPPweARss
ROM_START( silvrush )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "silver_rush_jp_limit_ver23.08.25.ic6", 0x00000, 0x10000, CRC(ac9e0c52) SHA1(c240a25ddf31913ac82ad7c96cea017890db7b8c) ) // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "type_sr_51f5_ver_1_0.ic55", 0x000000, 0x200000, CRC(d4f1ee94) SHA1(707e3337a18b043b742b64b6561a292f7fa81646) ) // 11xxxxxxxxxxxxxxxxxxx = 0x00
ROM_END

} // anonymous namespace


GAME( 2001, petitlot,   0, petitlot,  petitlot,  shoken_md06_state, empty_init, ROT0, "Shoken", "Petit Lot (ver. 4.1)",                    MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
GAME( 200?, polstrfi,   0, polarstar, polarstar, shoken_md06_state, empty_init, ROT0, "Shoken", "Polar Star (ver 3,2B, Fantasy Island)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
GAME( 200?, polstrmf,   0, polarstar, polarstar, shoken_md06_state, empty_init, ROT0, "Shoken", "Polar Star (Ver 3,2B, MooRrry Island)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
GAME( 2004, polstar2,   0, polarstar, polarstar, shoken_md06_state, empty_init, ROT0, "Shoken", "Polar Star 2 (ver. 4.5)",                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
GAME( 2011, silvrush,   0, polarstar, polarstar, shoken_md06_state, empty_init, ROT0, "Shoken", "Silver Rush (ver. 23.08.25)",             MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL )

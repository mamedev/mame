// license:BSD-3-Clause
// copyright-holders:

/*
Mahjongs / card games running on IAM2 custom CPU

IAMPCB0007-02
1184003B

The main components are:
IAM2 F99130265 custom CPU (?)
2 IS61C1024-20K RAMs (near IAM2)
N3412256P-15 SRAM (near IAM2)
24 MHz XTAL (near IAM2)
UT6264PC-70LL RAM (near GFX (?) ROM)
U6295 (Oki M6295 clone)
4 banks of 8 DIP switches

TODO:
- identify CPU arch
*/


#include "emu.h"

#include "cpu/arm7/arm7.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class iam2_state : public driver_device
{
public:
	iam2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void zhonggmj(machine_config &config) ATTR_COLD;
	void szjl(machine_config &config) ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


uint32_t iam2_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{

	return 0;
}

void iam2_state::video_start()
{
}


void iam2_state::program_map(address_map &map)
{
	map(0x00000000, 0x0003ffff).rom();
}


static INPUT_PORTS_START( zhonggmj )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW3:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW3:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW3:8")

	PORT_START("DSW4")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW4:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW4:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW4:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW4:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW4:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW4:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW4:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW4:8")
INPUT_PORTS_END


static GFXDECODE_START( gfx_iam2 )
	// TODO
GFXDECODE_END


void iam2_state::zhonggmj(machine_config &config)
{
	ARM7(config, m_maincpu, 24'000'000); // TODO: unidentified CPU arch and clock not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &iam2_state::program_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0, 64*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(iam2_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_iam2);

	PALETTE(config, "palette").set_entries(0x100); // TODO

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 24'000'000 / 24, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // pin 7 and clock not verified
}

void iam2_state::szjl(machine_config &config)
{
	zhonggmj(config);

	EEPROM_93C46_16BIT(config, "eeprom");
}


ROM_START( zhonggmj )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "zhonggmjmajiang_p28f020.u21", 0x00000, 0x40000, CRC(b6dda141) SHA1(bdaa88d5802226a252bbb222ae37cbc1a9bf0461) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "zhonggmjmajiang_p28f020.u6", 0x000000, 0x400000, CRC(156eca53) SHA1(7b044048b23b3472a27e32aa05a604a3a4bacd7b) ) // FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "zhonggmjmajiang_data.u15", 0x00000, 0x80000, CRC(99cb835d) SHA1(3f74e9dcfb9dfcc798fb9abb93afd865a1c6e200) )
ROM_END

// 神州接龙 (Shénzhōu Jiēlóng)
// this is on a different PCB with very similar components, adding a 93C46N EEPROM and a 12.2880 XTAL (in the audio section)
// the IAM2 PCB has number F99040223
ROM_START( szjl )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "gltk-17cn.u19", 0x00000, 0x20000, CRC(bc5fcea0) SHA1(1ba496fe9fcbfc3b24048b12ce11d5a6ec3651ef) )

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD( "gltk-dtwo.u20", 0x000000, 0x100000, CRC(672649fd) SHA1(5780ebd73277540d82adff41bb6e272c9b461afc) ) // FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "gltk-dath1.u21", 0x00000, 0x40000, CRC(a8b8ae92) SHA1(d73223922a0212988db191fb6d1f83251d37aef0) )
ROM_END

} // anonymous namespace


GAME( 200?, zhonggmj, 0, zhonggmj, zhonggmj, iam2_state, empty_init, ROT0, "I.A.M.", "Zhongguo Majiang", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 200?, szjl,     0, szjl,     zhonggmj, iam2_state, empty_init, ROT0, "I.A.M.", "Shenzhou Jielong", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

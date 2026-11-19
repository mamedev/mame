// license:BSD-3-Clause
// copyright-holders:

/*
HIGHWAY TECHNOLOGY CO.,LTD. TW9902 PCB

Main components are:
MC68000P10
10MHz XTAL
2x IS61C256AH-15N SRAM (near M68K)
2x Highway CY1011-R3 CONNY 9847A custom
4x IS61C1024-15N (2 for each CY1011)
Highway CY1012-R3 CONNY 9847A custom
2x 62256 SRAM (near CY1012-R3, exact model unreadable)
Highway CY1013-R3 6868A-10094 9903M EBG60 custom (near amp section)
2x UMC70C188 RAMDAC? (near CY1013-R3)
stickered PLCC-84 chip (possibly some kind of CPLD)
3x bank of 8 switches (SW1-SW3) + 3 more unpopulated spaces (SW4-SW6)
*/


#include "emu.h"

#include "cpu/m68000/m68000.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class highway_m68k_state : public driver_device
{
public:
	highway_m68k_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void highway_m68k(machine_config &config) ATTR_COLD;

	void init_tzfx() ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


uint32_t highway_m68k_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{

	return 0;
}

void highway_m68k_state::video_start()
{
}


void highway_m68k_state::program_map(address_map &map)
{
	map.unmap_value_high();

	map(0x000000, 0x7fffff).rom();
	map(0xe00000, 0xe003ff).ram();
	map(0xe40000, 0xe7ffff).ram();
	map(0xe80000, 0xe8ffff).ram();
	map(0xf00000, 0xf003ff).ram();
	map(0xf40000, 0xf7ffff).ram();
	map(0xff0000, 0xffffff).ram();
}


static INPUT_PORTS_START( tzfx )
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

	PORT_START("IN2")
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
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW1:8" )

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW2:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW2:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW3:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW3:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW3:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW3:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW3:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW3:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW3:8" )
INPUT_PORTS_END


// TODO
static GFXDECODE_START( gfx_highway_m68k )
GFXDECODE_END


void highway_m68k_state::highway_m68k(machine_config &config)
{
	M68000(config, m_maincpu, 10_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &highway_m68k_state::program_map);
	m_maincpu->set_vblank_int("screen", FUNC(highway_m68k_state::irq7_line_hold)); // 3, 5 and 6 also used

	screen_device &screen(SCREEN(config, "screen")); // TODO
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0, 64*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(highway_m68k_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_highway_m68k);

	PALETTE(config, "palette").set_entries(0x100); // TODO

	// sound hardware
	SPEAKER(config, "mono").front_center();

	// TODO: one of the customs?
}


// 天王之风花雪月 (Tiānwáng Zhī Fēnghuā Xuěyuè) (The Heavenly King’s Romance)
ROM_START( tzfx )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "p1_1_2d6e.u25",    0x000000, 0x200000, CRC(6f218c42) SHA1(1b8c3ce3b841d7c7929538d46eaa88bad76af06b) ) // the first 0x80000 are exactly the same as the "ROMBANK" below
	ROM_LOAD16_WORD_SWAP( "p1_2_6c30.u26",    0x200000, 0x200000, CRC(7fbdfc40) SHA1(d35425d6d25e91ad3e3ed23f06e02cf9ad64adea) )
	ROM_LOAD16_WORD_SWAP( "p1_3_3f18.u27",    0x400000, 0x200000, CRC(46562c93) SHA1(735b38ded2a10167ddc0f36afc7c5fc096ff0fb9) )
	ROM_LOAD16_WORD_SWAP( "p1_4_e57c.u28",    0x600000, 0x200000, CRC(622c8bed) SHA1(0db449855053c313252fa74971a906a42f103672) )
	ROM_LOAD16_WORD_SWAP( "prog1_2_f0b6.bin", 0x000000, 0x080000, CRC(cf704aa3) SHA1(31368ad5bd82e0502b77efb8b29be230cc8bfad1) ) // ROMBANK serigraphed under it. Possibly overlay for program updates?
ROM_END


void highway_m68k_state::init_tzfx()
{
	uint16_t *const rom = &memregion("maincpu")->as_u16();

	// XOR + bitswap, selected by A18-A20 (probably done by the stickered PLCC-84)
	for (int i = 0; i < 0x800000 / 2; i++)
	{
		switch (i & 0x1c0000 / 2)
		{
			case 0x000000 / 2: rom[i] = bitswap<16>(rom[i] ^ 0x7429, 6, 10, 13, 0, 7, 14, 8, 9, 11, 15, 5, 2, 1, 4, 3, 12); break;
			case 0x040000 / 2: rom[i] = bitswap<16>(rom[i] ^ 0xe287, 11, 10, 1, 5, 15, 14, 3, 7, 8, 6, 12, 2, 9, 4, 13, 0); break;
			case 0x080000 / 2: rom[i] = bitswap<16>(rom[i] ^ 0x8712, 2, 6, 13, 10, 9, 12, 11, 8, 0, 14, 4, 5, 3, 15, 1, 7); break;
			case 0x0c0000 / 2: rom[i] = bitswap<16>(rom[i] ^ 0x419c, 0, 1, 10, 12, 11, 13, 9, 8, 4, 5, 6, 7, 3, 2, 14, 15); break;
			case 0x100000 / 2: rom[i] = bitswap<16>(rom[i] ^ 0xae97, 8, 6, 12, 13, 7, 10, 0, 15, 11, 14, 5, 3, 4, 1, 2, 9); break;
			case 0x140000 / 2: rom[i] = bitswap<16>(rom[i] ^ 0x05aa, 11, 6, 0, 10, 15, 12, 9, 7, 8, 14, 4, 5, 3, 2, 1, 13); break;
			case 0x180000 / 2: rom[i] = bitswap<16>(rom[i] ^ 0x289f, 1, 5, 9, 3, 10, 11, 13, 0, 6, 7, 14, 2, 12, 4, 15, 8); break;
			case 0x1c0000 / 2: rom[i] = bitswap<16>(rom[i] ^ 0xf303, 10, 11, 3, 12, 14, 15, 1, 8, 4, 0, 5, 7, 13, 2, 9, 6); break;
		}
	}
}

} // anonymous namespace


GAME( 199?, tzfx, 0, highway_m68k, tzfx, highway_m68k_state, init_tzfx, ROT0, "Xin Yun", "Tianwang Zhi Fenghua Xueyue (v. 1.2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

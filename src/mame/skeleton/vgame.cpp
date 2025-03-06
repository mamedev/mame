// license:BSD-3-Clause
// copyright-holders:

/*
Video slots / mahjongs by VGame.

The main components are:
rectangular 128-pin chip, marked VGAME-007 (probably CPU)
square 208-pin chip, marked VGAME-008 (probably video chip)
44 MHz XTAL
LY62256SL SRAM (near VGAME-007)
2x LY61L256JL SRAM (near VGAME-008)
U6295 sound chip
2 banks of 8 DIP switches

TODO: everything. CPU core isn't identified and code is encrypted. Possibly internal ROM?
*/


#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class vgame_state : public driver_device
{
public:
	vgame_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void vgame(machine_config &config) ATTR_COLD;

	void init_vgame() ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


uint32_t vgame_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{

	return 0;
}

void vgame_state::video_start()
{
}


void vgame_state::program_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
}


static INPUT_PORTS_START( hilice )
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

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x0001, 0x0001, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x0002, 0x0002, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x0004, 0x0004, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x0008, 0x0008, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x0010, 0x0010, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x0020, 0x0020, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x0040, 0x0040, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x0080, 0x0080, "SW1:8")
	PORT_DIPUNKNOWN_DIPLOC(0x0100, 0x0100, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x0200, 0x0200, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x0400, 0x0400, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x0800, 0x0800, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x1000, 0x1000, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x2000, 0x2000, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x4000, 0x4000, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x8000, 0x8000, "SW2:8")
INPUT_PORTS_END


// TODO: wrong, just enough to glimpse some alphanumerics
static GFXDECODE_START( gfx_vgame )
	GFXDECODE_ENTRY( "gfx", 0, gfx_16x16x4_packed_lsb, 0, 1 )
GFXDECODE_END


void vgame_state::vgame(machine_config &config)
{
	M68000(config, m_maincpu, 44_MHz_XTAL); // CPU core and divider unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &vgame_state::program_map);
	// m_maincpu->set_vblank_int("screen", FUNC(vgame_state::irq0_line_hold));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0, 64*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(vgame_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx_vgame);

	PALETTE(config, "palette").set_entries(0x100); // TODO

	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 44_MHz_XTAL / 44, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // pin 7 and clock not verified
}

// VGAME-0030-02-AI PCB
ROM_START( hilice )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "hi_lice_v102fa.u13", 0x000000, 0x100000, BAD_DUMP CRC(4da87481) SHA1(5a20b254cfe8a2f087faa0dd17f682218a2ca1b2) ) // probably read as wrong ROM type
	ROM_CONTINUE(                               0x000000, 0x100000 )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD16_BYTE( "hi_lice_cg_01fu3.u3", 0x000000, 0x200000, BAD_DUMP CRC(8ad6b233) SHA1(deaffd391265c885afb2f171089c1b33429470f1) ) // probably read as wrong ROM type
	ROM_LOAD16_BYTE( "hi_lice_cg_01fu8.u8", 0x000001, 0x200000, BAD_DUMP CRC(b1070209) SHA1(4568977fca2ff96b756a9600ad9a4730a6f8749a) ) // probably read as wrong ROM type + didn't give consistent reads

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "hi_lice_sp_100f.u45", 0x000000, 0x200000, CRC(b2588f54) SHA1(0d046e56596611954a9d2a9a30746d8aa370431b) ) // 1xxxxxxxxxxxxxxxxxxxx = 0x00
ROM_END

// VGAME-0030-02-AG PCB, almost identical to the hilice one.
// while all labels have 麻將學園 (Mahjong School) prepended to what's below, title screen shows 麻將學園 2 - Mahjong School 2
ROM_START( mjxy2 )
	ROM_REGION( 0x200000, "maincpu", 0 ) // dumped as EV29LV160 (same rare ROM as some IGS titles)
	ROM_LOAD16_WORD_SWAP( "u12_v108tw.u12", 0x000000, 0x200000, CRC(a6d99849) SHA1(c280635517d5ffded524e15048568817bd927bf9) )

	ROM_REGION( 0x400000, "gfx", 0 ) // dumped as EV29LV160 (same rare ROM as some IGS titles)
	ROM_LOAD16_BYTE( "u3_cg_v105.u3", 0x000000, 0x200000, CRC(fda38fb1) SHA1(7bd744e42f619254ebad2fb60f3851f61073fe8c) ) // FIXED BITS (xxxxxxxx0xxxxxxx)
	ROM_LOAD16_BYTE( "u7_cg_v105.u7", 0x000001, 0x200000, CRC(5acf5b99) SHA1(ee638635c25ab9d392b8a7ff79209e657ccfd5c0) ) // FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "u43_sp_v105.u43", 0x000000, 0x200000, CRC(5d1ab8f1) SHA1(56473b632dfdb210208ce3b35cb6861f07861cd7) )
ROM_END

ROM_START( cjdn ) // Oct  1 2008 15:58:54 string in ROM
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "v305cn.u8", 0x000000, 0x200000, CRC(bffb20c0) SHA1(bf2293bfc4711d0b930c491f579418d5e7c0800f) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "gfx_flashrom", 0x000000, 0x400000, NO_DUMP ) // not dumped yet

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "sp_101g.u24", 0x000000, 0x200000, CRC(f361a725) SHA1(9404f955126f16c7f4a2c52e799791f6f2703b5f) )
ROM_END

ROM_START( cjsjh ) // Nov  2 2007 16:05:26 string in ROM
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "vxxxcn.u8", 0x000000, 0x200000, CRC(911d8dce) SHA1(1d9386c4a9e118d02b5b29f9e9ce90ef1d6f419f) ) // label not readable

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD( "gfx_flashrom", 0x000000, 0x400000, NO_DUMP ) // not dumped yet

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "sp.u24", 0x000000, 0x200000, CRC(ad11c8b9) SHA1(0547a57ff2183e65fa1d51234799a3d521b018c5) )
ROM_END


// TODO: very incomplete, just enough to show some strings
void vgame_state::init_vgame()
{
	uint8_t *rom = memregion("maincpu")->base();

	for (int i = 0; i < 0x200000; i += 2)
	{
		switch (i & 0x6232)
		{
			case 0x0000: rom[i] ^= 0x00; break;
			case 0x0002: rom[i] ^= 0x20; break;
			case 0x0010: rom[i] ^= 0x02; break;
			case 0x0012: rom[i] ^= 0x22; break;
			case 0x0020: rom[i] ^= 0x20; break;
			case 0x0022: rom[i] ^= 0x20; break;
			case 0x0030: rom[i] ^= 0x22; break;
			case 0x0032: rom[i] ^= 0x22; break;
			case 0x0200: rom[i] ^= 0x00; break;
			case 0x0202: rom[i] ^= 0x00; break;
			case 0x0210: rom[i] ^= 0x02; break;
			case 0x0212: rom[i] ^= 0x02; break;
			case 0x0220: rom[i] ^= 0x20; break;
			case 0x0222: rom[i] ^= 0x20; break;
			case 0x0230: rom[i] ^= 0x22; break;
			case 0x0232: rom[i] ^= 0x22; break;
			default: rom[i] ^= 0x22;
		}
	}
}

} // anonymous namespace


GAME( 200?, hilice,  0, vgame, hilice, vgame_state, init_vgame, ROT0, "VGame", "Hi Lice (V102FA)",                              MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 200?, mjxy2,   0, vgame, hilice, vgame_state, init_vgame, ROT0, "VGame", "Majiang Xueyuan 2 - Mahjong School (V108TW)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2008, cjdn,    0, vgame, hilice, vgame_state, init_vgame, ROT0, "VGame", "Chao Ji Dou Niu (V305CN)",                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2007, cjsjh,   0, vgame, hilice, vgame_state, init_vgame, ROT0, "VGame", "Chao Ji Sai Jin Hua (V201CN)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

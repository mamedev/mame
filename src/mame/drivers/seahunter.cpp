// license:BSD-3-Clause
// copyright-holders:

/*
Sea Hunter (?) by unknown manufacturer

The not working PCB has the following main components:

1 chip covered by the 'Sea Hunter' sticker (M68000?)
1 scratched off chip (near 2203)
1 YM2203C
2 8-dip banks (near 2203)
1 18 MHz OSC (near the chip covered by the sticker)
1 16 MHz OSC (near 2203 and scratched off chip)
4 ROMs (mix of 27C010A and 27C1001)
2 PALs
2 HM6264 SRAMs

Strings in ROMs seem to point to a tile-matching game from a Korean manufacturer.
*/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/2203intf.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class seahunter_state : public driver_device
{
public:
	seahunter_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void seahuntr(machine_config &config);

	void init_seahuntr();

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map);

	required_device<cpu_device> m_maincpu;
};


uint32_t seahunter_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void seahunter_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("maincpu", 0);
}

static INPUT_PORTS_START( seahuntr )
INPUT_PORTS_END


void seahunter_state::seahuntr(machine_config &config)
{
	M68000(config, m_maincpu, 18'000'000 / 2); // guess
	m_maincpu->set_addrmap(AS_PROGRAM, &seahunter_state::main_map);
	//m_maincpu->set_vblank_int("screen", FUNC(seahunter_state::irq1_line_hold));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: all wrong
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(seahunter_state::screen_update));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ym2203_device &ym(YM2203(config, "ym2203", 16_MHz_XTAL / 8)); // guess
	ym.add_route(ALL_OUTPUTS, "lspeaker", 0.47);
	ym.add_route(ALL_OUTPUTS, "rspeaker", 0.47);
}


ROM_START( seahuntr )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1-c25.e7", 0x00000, 0x20000, CRC(ff4dd98d) SHA1(69c9229537a25aaaa82cd6b80eea85a91b6243d1) )
	ROM_LOAD16_BYTE( "3-c35.f7", 0x00001, 0x20000, CRC(94297664) SHA1(2b8f979db92e4979e35ff22747a7076aa687e5da) )
	ROM_LOAD16_BYTE( "2-c26.e8", 0x40000, 0x20000, CRC(eb4f06c7) SHA1(3ef68edc48d33011d0f9eb78f3ad0cc58136e69c) )
	ROM_LOAD16_BYTE( "4-c36.f8", 0x40001, 0x20000, CRC(dcbf1619) SHA1(8333b661021bbe5de371bfcea121a69c2727df12) )

	ROM_REGION( 0x208, "plds", 0 )
	ROM_LOAD( "pal16l8.c13", 0x000, 0x104, NO_DUMP)
	ROM_LOAD( "pal16l8.c14", 0x104, 0x104, NO_DUMP)
ROM_END


void seahunter_state::init_seahuntr() // TODO: just a start, gives correct (?) strings i.e. at 0x76710, 0x78d10, 0x790b0
{
	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();

	for (int i = 0x00000; i < 0x80000 / 2; i++)
		rom[i] = bitswap<16>(rom[i], 15, 14, 13, 11, 12, 10, 9, 8, 7, 6, 5, 4, 3, 1, 2, 0);

	/*char filename[256];
	sprintf(filename,"p_decrypted_%s", machine().system().name);
	FILE *fp = fopen(filename, "w+b");
	if (fp)
	{
	    fwrite(rom, 0x80000, 1, fp);
	    fclose(fp);
	}*/
}

} // Anonymous namespace


GAME( 199?, seahuntr, 0, seahuntr, seahuntr, seahunter_state, init_seahuntr, ROT0, "<unknown>", "Sea Hunter", MACHINE_IS_SKELETON ) // title is taken from a sticker on the PCB

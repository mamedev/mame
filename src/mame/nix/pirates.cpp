// license:BSD-3-Clause
// copyright-holders:David Haywood,Nicola Salmoria,Paul Priest
/*---

Pirates      (c)1994 NIX  (DEC 14 1994 17:32:29) displayed in cabinet test mode
Genix Family (c)1994 NIX  (MAY 10 1994 14:21:20) displayed in cabinet test mode
driver by David Haywood and Nicola Salmoria

TODO:
- EEPROM doesn't work. I'm not sure what the program is trying to do.
  The EEPROM handling might actually be related to the protection which
  makes the game hang.
  See prot_r() for code which would work around the protection,
  but makes the game periodically hang for a couple of seconds; therefore,
  for now I'm just patching out the protection check.

- Protection is the same in Genix Family

-----

Here's some info about the dump:

Name:            Pirates
Manufacturer:    NIX
Year:            1994
Date Dumped:     14-07-2002 (DD-MM-YYYY)

CPU:             68000, possibly at 12mhz (prototype board does have a 16mhz one)
SOUND:           OKIM6295
GFX:             Unknown

CPU Roms at least are the same on the Prototype board (the rest of the roms probably are too)

-----

Program Roms are Scrambled (Data + Address Lines)
P Graphic Roms (Tilemap Tiles) are Scrambled (Data + Address Lines)
S Graphic Roms (Sprite Tiles) are Scrambled (Data + Address Lines)
OKI Samples Rom is Scrambled (Data + Address Lines)

68k interrupts (pirates)
lev 1 : 0x64 : 0000 bf84 - vbl?
lev 2 : 0x68 : 0000 4bc6 -
lev 3 : 0x6c : 0000 3bda -
lev 4 : 0x70 : 0000 3bf0 -
lev 5 : 0x74 : 0000 3c06 -
lev 6 : 0x78 : 0000 3c1c -
lev 7 : 0x7c : 0000 3c32 -

Inputs mapped by Stephh

The game hanging is an interesting issue, the board owner has 2 copies of this game, one a prototype,
one the final released version.  The roms on both boards are the same, however the prototype locks up
just as it does in Mame at the moment.  The final board does not.  It would appear the prototype
board does not have the protection hardware correctly in place


PCB Layout (Genix Family) (by Guru)
----------

|------------------------------------------------|
|     0.31       6116  6116             3.72     |
|         M6295  6116  6116             4.71     |
|                                       5.70     |
|         6264         6116             6.69     |
|         6264         6116                      |
|                                                |
|                                                |
| 93C46       Altera  24MHz   Altera             |
|             EPM7064         EPM7064            |
|                                                |
|                                                |
|             Altera          Altera  7.34  9.35 |
|             EPM7064         EPM7064 8.48 10.49 |
|                                                |
|        PAL                                     |
|                                                |
|        68000  1.15  62256  6264                |
| 32MHz         2.16  62256  6264                |
|               *                                |
|------------------------------------------------|

Notes:
      68000 clock: 16.000MHz
      M6295 clock: 1.33333MHz, Sample Rate: /165
      VSync: 60Hz
      HSync: 15.69kHz
      *    : unknown IC (18 pin DIP, surface scratched off)

---*/

#include "emu.h"
#include "pirates.h"

#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "screen.h"
#include "speaker.h"


void pirates_state::out_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		/* bits 0-2 control EEPROM */
		m_eeprom->di_write((data & 0x04) >> 2);
		m_eeprom->cs_write((data & 0x01) ? ASSERT_LINE : CLEAR_LINE);
		m_eeprom->clk_write((data & 0x02) ? ASSERT_LINE : CLEAR_LINE);

		/* bit 6 selects oki bank */
		m_oki->set_rom_bank((data >> 6) & 1);

		/* bit 7 used (function unknown) */
	}

//  logerror("%06x: out_w %04x\n",m_maincpu->pc(),data);
}

READ_LINE_MEMBER(pirates_state::prot_r)
{
//  static int prot = 0xa3;
//  offs_t pc;
	int bit;

//  logerror("%s: IN1_r\n",machine().describe_context());

#if 0
	/* Pirates protection workaround. It's more complicated than this... see code at
	   602e and 62a6 */
	/* For Genix, see 6576 for setting values and 67c2,d3b4 and dbc2 for tests. */

	pc = m_maincpu->pc();
	if (pc == 0x6134)
	{
		bit = prot & 1;
		prot = (prot >> 1) | (bit << 7);
	}
	else if (pc == 0x6020)
		bit = 0;
	else if (pc == 0x6168)
		bit = 0;
	else if (pc == 0x61cc)
		bit = 1;
	else
#endif
		bit = 1;

	return bit;
}



/* Memory Maps */

void pirates_state::pirates_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x10ffff).ram(); // main ram
	map(0x300000, 0x300001).portr("INPUTS");
	map(0x400000, 0x400001).portr("SYSTEM");
//  map(0x500000, 0x5007ff).ram();
	map(0x500000, 0x5007ff).writeonly().share("spriteram");
//  map(0x500800, 0x50080f).nopw();
	map(0x600000, 0x600001).w(FUNC(pirates_state::out_w));
	map(0x700000, 0x700001).writeonly().share("scroll");    // scroll reg
	map(0x800000, 0x803fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x900000, 0x90017f).ram();  // more of tilemaps ?
	map(0x900180, 0x90137f).ram().w(FUNC(pirates_state::tx_tileram_w)).share("tx_tileram");
	map(0x901380, 0x902a7f).ram().w(FUNC(pirates_state::fg_tileram_w)).share("fg_tileram");
//  map(0x902580, 0x902a7f).ram();  // more of tilemaps ?
	map(0x902a80, 0x904187).ram().w(FUNC(pirates_state::bg_tileram_w)).share("bg_tileram");
//  map(0x903c80, 0x904187).ram();  // more of tilemaps ?
	map(0xa00001, 0xa00001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}


/* Input Ports */

static INPUT_PORTS_START( pirates )
	PORT_START("INPUTS")    /* 0x300000.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")    /* 0x400000.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH,IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)  // EEPROM data
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN )     // seems checked in "test mode"
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )     // seems checked in "test mode"
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH,IPT_CUSTOM ) PORT_READ_LINE_MEMBER(pirates_state, prot_r)      // protection
	/* What do these bits do ? */
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 8*0, 8*1, 8*2, 8*3, 8*4, 8*5, 8*6, 8*7 },
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 7, 6, 5, 4, 3, 2, 1, 0,
		15,14,13,12,11,10, 9, 8 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16,10*16,11*16,12*16,13*16,14*16,15*16 },
	16*16
};

static GFXDECODE_START( gfx_pirates )

	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0x0000, 3*128 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0x1800,   128 )
GFXDECODE_END



/* Machine Driver + Related bits */

void pirates_state::pirates(machine_config &config)
{
	M68000(config, m_maincpu, 32_MHz_XTAL / 2); // 16MHz verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &pirates_state::pirates_map);
	m_maincpu->set_vblank_int("screen", FUNC(pirates_state::irq1_line_hold));

	EEPROM_93C46_16BIT(config, "eeprom");

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_pirates);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(36*8, 32*8);
	screen.set_visarea(0*8, 36*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(pirates_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x2000);

	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 24_MHz_XTAL / 18, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 1.0); // 1.3333333MHz verified on PCB
}


/* Rom Loading */

ROM_START( pirates )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code (encrypted) */
	ROM_LOAD16_BYTE( "r_449b.bin",  0x00000, 0x80000, CRC(224aeeda) SHA1(5b7e47a106af0debf8b07f120571f437ad6ab5c3) )
	ROM_LOAD16_BYTE( "l_5c1e.bin",  0x00001, 0x80000, CRC(46740204) SHA1(6f1da3b2cbea25bbfdec74c625c5fb23459b83b6) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* GFX (encrypted) */
	ROM_LOAD( "p4_4d48.bin", 0x000000, 0x080000, CRC(89fda216) SHA1(ea31e750460e67a24972b04171230633eb2b6d9d) )
	ROM_LOAD( "p2_5d74.bin", 0x080000, 0x080000, CRC(40e069b4) SHA1(515d12cbb29bdbf3f3016e5bbe14941209978095) )
	ROM_LOAD( "p1_7b30.bin", 0x100000, 0x080000, CRC(26d78518) SHA1(c293f1194f8ef38241d149cf1db1a511a7fb4936) )
	ROM_LOAD( "p8_9f4f.bin", 0x180000, 0x080000, CRC(f31696ea) SHA1(f5ab59e441317b02b615a1cdc6d075c5bdcdea73) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* GFX (encrypted) */
	ROM_LOAD( "s1_6e89.bin", 0x000000, 0x080000, CRC(c78a276f) SHA1(d5127593e68f9e8f2878803c652a35a1c6d82b2c) )
	ROM_LOAD( "s2_6df3.bin", 0x080000, 0x080000, CRC(9f0bad96) SHA1(b8f910aa259192e261815392f5d7c9c7dabe0b4d) )
	ROM_LOAD( "s4_fdcc.bin", 0x100000, 0x080000, CRC(8916ddb5) SHA1(f4f7da831ef929eb7575bbe69eae317f15cfd648) )
	ROM_LOAD( "s8_4b7c.bin", 0x180000, 0x080000, CRC(1c41bd2c) SHA1(fba264a3c195f303337223a74cbad5eec5c457ec) )

	ROM_REGION( 0x080000, "oki", 0) /* OKI samples (encrypted) */
	ROM_LOAD( "s89_49d4.bin", 0x000000, 0x080000, CRC(63a739ec) SHA1(c57f657225e62b3c9c5f0c7185ad7a87794d55f4) )
ROM_END

ROM_START( piratesb )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code (encrypted) */
	ROM_LOAD16_BYTE( "u15",  0x00000, 0x80000, CRC(0cfd6415) SHA1(ff5d3631702f64351afa3b7435a6977ae856dff7) )
	ROM_LOAD16_BYTE( "u16",  0x00001, 0x80000, CRC(98cece02) SHA1(79858623a2b6ae24067e0ba1af009444bafba490) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* GFX (encrypted) */
	ROM_LOAD( "u34", 0x000000, 0x080000, CRC(89fda216) SHA1(ea31e750460e67a24972b04171230633eb2b6d9d) )
	ROM_LOAD( "u35", 0x080000, 0x080000, CRC(40e069b4) SHA1(515d12cbb29bdbf3f3016e5bbe14941209978095) )
	ROM_LOAD( "u48", 0x100000, 0x080000, CRC(26d78518) SHA1(c293f1194f8ef38241d149cf1db1a511a7fb4936) )
	ROM_LOAD( "u49", 0x180000, 0x080000, CRC(f31696ea) SHA1(f5ab59e441317b02b615a1cdc6d075c5bdcdea73) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* GFX (encrypted) */
	ROM_LOAD( "u69", 0x000000, 0x080000, CRC(c78a276f) SHA1(d5127593e68f9e8f2878803c652a35a1c6d82b2c) )
	ROM_LOAD( "u70", 0x080000, 0x080000, CRC(9f0bad96) SHA1(b8f910aa259192e261815392f5d7c9c7dabe0b4d) )
	ROM_LOAD( "u71", 0x100000, 0x080000, CRC(0bb7c816) SHA1(bc786b6d04ae964f0ea5d6dd314fd7b18f8872e8) ) // 1 bit different, is one of them bad?
	ROM_LOAD( "u72", 0x180000, 0x080000, CRC(1c41bd2c) SHA1(fba264a3c195f303337223a74cbad5eec5c457ec) )

	ROM_REGION( 0x080000, "oki", 0) /* OKI samples (encrypted) */
	ROM_LOAD( "u31", 0x000000, 0x080000, CRC(63a739ec) SHA1(c57f657225e62b3c9c5f0c7185ad7a87794d55f4) )
ROM_END


ROM_START( genix )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code (encrypted) */
	ROM_LOAD16_BYTE( "11.u15.15c",  0x00000, 0x80000, CRC(d26abfb0) SHA1(4a89ba7504f86cb612796c376f359ab61ec3d902) )
	ROM_LOAD16_BYTE( "12.u16.16c",  0x00001, 0x80000, CRC(a14a25b4) SHA1(9fa64c6514bdee56b5654b001f8367283b461e8a) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* GFX (encrypted) */
	ROM_LOAD( "17.u34.12g", 0x000000, 0x040000, CRC(58da8aac) SHA1(bfc8449ba842f8ceac62ebdf6005d8f19d96afa6) )
	ROM_LOAD( "19.u35.12h", 0x080000, 0x040000, CRC(96bad9a8) SHA1(4e757cca0ab157f0c935087c9702c88741bf7a79) )
	ROM_LOAD( "18.u48.13g", 0x100000, 0x040000, CRC(0ddc58b6) SHA1(d52437607695ddebfe8494fd214efd20ba72d549) )
	ROM_LOAD( "20.u49.13h", 0x180000, 0x040000, CRC(2be308c5) SHA1(22fc0991557643c22f6763f186b74900a33a39e0) )

	ROM_REGION( 0x200000, "gfx2", 0 ) /* GFX (encrypted) */
	ROM_LOAD( "16.u69.6g", 0x000000, 0x040000, CRC(b8422af7) SHA1(d3290fc6ea2670c445731e2b493205874dc4b319) )
	ROM_LOAD( "15.u70.4g", 0x080000, 0x040000, CRC(e46125c5) SHA1(73d9a51f30a9c1a8397145d2a4397696ef37f4e5) )
	ROM_LOAD( "14.u71.3g", 0x100000, 0x040000, CRC(7a8ed21b) SHA1(f380156c44de2fc316f390adee09b6a3cd404dec) )
	ROM_LOAD( "13.u72.1g", 0x180000, 0x040000, CRC(f78bd6ca) SHA1(c70857b8053f9a6e3e15bbc9f7d13354b0966b30) )

	ROM_REGION( 0x080000, "oki", 0) /* OKI samples (encrypted) */
	ROM_LOAD( "10.u31.1b", 0x000000, 0x080000, CRC(80d087bc) SHA1(04d1aacc273c7ffa57b48bd043d55b5b3d993f74) )
ROM_END

/* Init */

void pirates_state::decrypt_68k()
{
	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();
	size_t rom_size = memregion("maincpu")->bytes();
	std::vector<uint16_t> buf(rom_size/2);

	memcpy (&buf[0], rom, rom_size);

	for (int i=0; i<rom_size/2; i++)
	{
		int adrl, adrr;
		uint8_t vl, vr;

		adrl = bitswap<24>(i,23,22,21,20,19,18,4,8,3,14,2,15,17,0,9,13,10,5,16,7,12,6,1,11);
		vl = bitswap<8>(buf[adrl],    4,2,7,1,6,5,0,3);

		adrr = bitswap<24>(i,23,22,21,20,19,18,4,10,1,11,12,5,9,17,14,0,13,6,15,8,3,16,7,2);
		vr = bitswap<8>(buf[adrr]>>8, 1,4,7,0,3,5,6,2);

		rom[i] = (vr<<8) | vl;
	}
}

void pirates_state::decrypt_p()
{
	int rom_size;
	uint8_t *rom;
	int i;

	rom_size = memregion("gfx1")->bytes();

	std::vector<uint8_t> buf(rom_size);

	rom = memregion("gfx1")->base();
	memcpy (&buf[0], rom, rom_size);

	for (i=0; i<rom_size/4; i++)
	{
		int adr = bitswap<24>(i,23,22,21,20,19,18,10,2,5,9,7,13,16,14,11,4,1,6,12,17,3,0,15,8);
		rom[adr+0*(rom_size/4)] = bitswap<8>(buf[i+0*(rom_size/4)], 2,3,4,0,7,5,1,6);
		rom[adr+1*(rom_size/4)] = bitswap<8>(buf[i+1*(rom_size/4)], 4,2,7,1,6,5,0,3);
		rom[adr+2*(rom_size/4)] = bitswap<8>(buf[i+2*(rom_size/4)], 1,4,7,0,3,5,6,2);
		rom[adr+3*(rom_size/4)] = bitswap<8>(buf[i+3*(rom_size/4)], 2,3,4,0,7,5,1,6);
	}
}

void pirates_state::decrypt_s()
{
	int rom_size;
	uint8_t *rom;
	int i;

	rom_size = memregion("gfx2")->bytes();

	std::vector<uint8_t> buf(rom_size);

	rom = memregion("gfx2")->base();
	memcpy (&buf[0], rom, rom_size);

	for (i=0; i<rom_size/4; i++)
	{
		int adr = bitswap<24>(i,23,22,21,20,19,18,17,5,12,14,8,3,0,7,9,16,4,2,6,11,13,1,10,15);
		rom[adr+0*(rom_size/4)] = bitswap<8>(buf[i+0*(rom_size/4)], 4,2,7,1,6,5,0,3);
		rom[adr+1*(rom_size/4)] = bitswap<8>(buf[i+1*(rom_size/4)], 1,4,7,0,3,5,6,2);
		rom[adr+2*(rom_size/4)] = bitswap<8>(buf[i+2*(rom_size/4)], 2,3,4,0,7,5,1,6);
		rom[adr+3*(rom_size/4)] = bitswap<8>(buf[i+3*(rom_size/4)], 4,2,7,1,6,5,0,3);
	}
}


void pirates_state::decrypt_oki()
{
	int rom_size;
	uint8_t *rom;
	int i;

	rom_size = memregion("oki")->bytes();

	std::vector<uint8_t> buf(rom_size);

	rom = memregion("oki")->base();
	memcpy (&buf[0], rom, rom_size);

	for (i=0; i<rom_size; i++)
	{
		int adr = bitswap<24>(i,23,22,21,20,19,10,16,13,8,4,7,11,14,17,12,6,2,0,5,18,15,3,1,9);
		rom[adr] = bitswap<8>(buf[i], 2,3,4,0,7,5,1,6);
	}
}


void pirates_state::init_pirates()
{
	uint16_t *rom = (uint16_t *)memregion("maincpu")->base();

	decrypt_68k();
	decrypt_p();
	decrypt_s();
	decrypt_oki();

	/* patch out protection check */
	rom[0x62c0/2] = 0x6006; // beq -> bra
}

uint16_t pirates_state::genix_prot_r(offs_t offset){ if(!offset) return 0x0004; else return 0x0000; }

void pirates_state::init_genix()
{
	decrypt_68k();
	decrypt_p();
	decrypt_s();
	decrypt_oki();

	/* If this value is increased then something has gone wrong and the protection failed */
	/* Write-protect it for now */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x109e98, 0x109e9b, read16sm_delegate(*this, FUNC(pirates_state::genix_prot_r)));
}

/* GAME */

GAME( 1994, pirates,  0,       pirates, pirates, pirates_state, init_pirates, 0, "NIX", "Pirates (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, piratesb, pirates, pirates, pirates, pirates_state, init_pirates, 0, "NIX", "Pirates (set 2)", MACHINE_SUPPORTS_SAVE ) // shows 'Copyright 1995' instead of (c)1994 Nix, but isn't unprotected, various changes to the names in the credis + a few other minor alterations

GAME( 1994, genix,    0,       pirates, pirates, pirates_state, init_genix,   0, "NIX", "Genix Family",    MACHINE_SUPPORTS_SAVE )

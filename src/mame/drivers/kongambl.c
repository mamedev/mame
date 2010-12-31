/*
  Konami Gambling Games ("Tasman" hardware)
  System GX derivative

  68EC020 @ 25 MHz
  K056832 + K058143 : GX tilemaps
  K055673(x2) + K053246A : (extended?) GX sprites
  K055555 : GX mixer/blender

  68000 @ 16 MHz + YMZ280B for audio

  Thanks to palindrome for PCB scans.
*/

#include "emu.h"
#include "video/konicdev.h"
#include "includes/konamigx.h"
#include "cpu/m68000/m68000.h"
#include "machine/eeprom.h"

static VIDEO_START(kongambl)
{
	device_t *k056832 = machine->device("k056832");

	k056832_set_layer_association(k056832, 0);
	k056832_set_layer_offs(k056832, 0, -2, 0);
	k056832_set_layer_offs(k056832, 1,  2, 0);
	k056832_set_layer_offs(k056832, 2,  4, 0);
	k056832_set_layer_offs(k056832, 3,  6, 0);
}

static VIDEO_UPDATE(kongambl)
{
	device_t *k056832 = screen->machine->device("k056832");

	bitmap_fill(bitmap, cliprect, 0);
	bitmap_fill(screen->machine->priority_bitmap, cliprect, 0);

//  k056832_tilemap_draw(k056832, bitmap, cliprect, 3, 0, 0);
//  k056832_tilemap_draw(k056832, bitmap, cliprect, 2, 0, 0);
	k056832_tilemap_draw(k056832, bitmap, cliprect, 1, 0, 0);
	k056832_tilemap_draw(k056832, bitmap, cliprect, 0, 0, 0);
	return 0;
}

static READ32_HANDLER( eeprom_r )
{
	if (ACCESSING_BITS_0_7)
	{
		UINT32 rv = input_port_read(space->machine, "SYSTEM") & ~0x1;

		return rv;	// bit 0 freezes the game if 1
	}

	return 0;
}

static WRITE32_HANDLER( eeprom_w )
{
	if (ACCESSING_BITS_8_15)
	{
		input_port_write(space->machine, "EEPROMOUT", (data>>8)&0xf, 0xff);
	}
}

static ADDRESS_MAP_START( kongambl_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM	// main program
	AM_RANGE(0x080000, 0x0fffff) AM_ROM AM_REGION("maincpu", 0)	// mirror
	AM_RANGE(0x100000, 0x11ffff) AM_RAM	// work RAM

	AM_RANGE(0x400000, 0x401fff) AM_DEVREADWRITE("k056832", k056832_ram_long_r, k056832_ram_long_w)

	AM_RANGE(0x420000, 0x43ffff) AM_DEVREADWRITE("k056832", k056832_unpaged_ram_long_r, k056832_unpaged_ram_long_w)

	AM_RANGE(0x440000, 0x443fff) AM_RAM

	AM_RANGE(0x460000, 0x47ffff) AM_RAM_WRITE(konamigx_palette_w) AM_BASE_GENERIC(paletteram)

	AM_RANGE(0x480000, 0x48003f) AM_DEVWRITE("k056832", k056832_long_w)

	AM_RANGE(0x700000, 0x700003) AM_READ( eeprom_r )
	AM_RANGE(0x780000, 0x780003) AM_WRITE( eeprom_w )
ADDRESS_MAP_END


static INPUT_PORTS_START( kongambl )
	PORT_START( "SYSTEM" )
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE("eeprom", eeprom_read_bit)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_write_bit)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_clock_line)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE("eeprom", eeprom_set_cs_line)
INPUT_PORTS_END

static ADDRESS_MAP_START( kongamaud_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM	// main program (mirrored?)
	AM_RANGE(0x100000, 0x10ffff) AM_RAM	// work RAM
	AM_RANGE(0x200000, 0x2000ff) AM_RAM	// unknown (YMZ280b?  Shared with 68020?)
ADDRESS_MAP_END

static void kongambl_sprite_callback( running_machine *machine, int *code, int *color, int *priority_mask )
{
}

static void kongambl_tile_callback( running_machine *machine, int layer, int *code, int *color, int *flags )
{
}

static const k056832_interface k056832_intf =
{
	"gfx1", 0,
	K056832_BPP_8TASMAN,
	0, 0,
	KONAMI_ROM_DEINTERLEAVE_NONE,
	kongambl_tile_callback, "none"
};

static const k053247_interface k053247_intf =
{
	"screen",
	"gfx2", 1,
	TASMAN_PLANE_ORDER,
	-48+1, 23,
	KONAMI_ROM_DEINTERLEAVE_NONE,
	kongambl_sprite_callback
};

static MACHINE_CONFIG_START( kongambl, driver_device )
	MCFG_CPU_ADD("maincpu", M68EC020, 25000000)
	MCFG_CPU_PROGRAM_MAP(kongambl_map)

	MCFG_CPU_ADD("sndcpu", M68000, 16000000)
	MCFG_CPU_PROGRAM_MAP(kongamaud_map)

	MCFG_EEPROM_93C46_ADD("eeprom")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 0*8, 32*8-1)

	MCFG_PALETTE_LENGTH(8192)

	MCFG_VIDEO_START(kongambl)
	MCFG_VIDEO_UPDATE(kongambl)

	MCFG_K053247_ADD("k053246", k053247_intf)
	MCFG_K056832_ADD("k056832", k056832_intf)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
MACHINE_CONFIG_END


ROM_START( kingtut )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68EC020 Code */
	ROM_LOAD32_WORD_SWAP( "kitp1b37_l.02", 0x000002, 0x40000, CRC(95c6da28) SHA1(3ef33f5d0748c80be82d33c21f0f8bb71909884e) )
	ROM_LOAD32_WORD_SWAP( "kitp1b37_h.01", 0x000000, 0x40000, CRC(16709625) SHA1(6b818a85724f87fed23a26978dd26b079f814134) )

	ROM_REGION( 0x80000, "sndcpu", 0 ) /* 68000 sound program */
	ROM_LOAD16_WORD_SWAP( "n12prog_ifu.41", 0x00000, 0x08000, CRC(dbb8a7e8) SHA1(9662b34e9332385d20e17ee1c92fd91935d4c3b2) )

	ROM_REGION( 0x100000, "gfx1", 0 ) // 8x8x8 tiles
	ROM_LOAD16_BYTE( "kit11_l1_vrm.21", 0x000000, 0x80000, CRC(431eb89f) SHA1(377c96f615b4b76314aeecad4e868edb66c72f33) )
	ROM_LOAD16_BYTE( "kit11_h1_vrm.23", 0x000001, 0x80000, CRC(7aa2f1bc) SHA1(d8aead9dedcc83d3dc574122103aaa2074011197) )

	ROM_REGION( 0x200000, "gfx2", 0 ) // 16x16x8 sprites
	ROM_LOAD16_BYTE( "kit11hh1_obj.11", 0x000000, 0x80000, CRC(a64d2382) SHA1(bb745a26ef6c076f3aa3ec476589a95915b359ed) )
	ROM_LOAD16_BYTE( "kit11hm1_obj.13", 0x000001, 0x80000, CRC(21cc4e40) SHA1(9e3735fc8cd53f7e831dc76697911216bd8bbc70) )
	ROM_LOAD16_BYTE( "kit11ll1_obj.17", 0x100000, 0x80000, CRC(a19338b8) SHA1(1aa68596e5bf493cb360495f1174dc1323086ad2) )
	ROM_LOAD16_BYTE( "kit11lm1_obj.15", 0x100001, 0x80000, CRC(1aea3f4d) SHA1(52fd1a7ffeeb3acce176ad3812a2ca146e02c324) )

	ROM_REGION( 0x100000, "snd", 0 )
	ROM_LOAD( "snd12sd1_snd.31", 0x000000, 0x80000, CRC(f4121baa) SHA1(723c6d96ecef5ef510d085f443d44bad07aa19e5) )
	ROM_LOAD( "kit11sd2_snd.32", 0x080000, 0x80000, CRC(647c6e2e) SHA1(e013239a73553e2993adabeda103f5b1cfee0f6c) )
ROM_END

ROM_START( moneybnk )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68EC020 Code */
	ROM_LOAD32_WORD_SWAP( "mobn6l29_l.02", 0x000002, 0x40000, CRC(9cd2754a) SHA1(2eb695cb4abab4a448711b8acf3f5b1bb169eb6f) )
	ROM_LOAD32_WORD_SWAP( "mobn6l29_h.01", 0x000000, 0x40000, CRC(952c376b) SHA1(0fc0499f5570b920c600ddd6a15751d72345c83e) )

	ROM_REGION( 0x80000, "sndcpu", 0 ) /* 68000 sound program */
	ROM_LOAD16_WORD_SWAP( "n12prog_ifu.41", 0x00000, 0x08000, CRC(dbb8a7e8) SHA1(9662b34e9332385d20e17ee1c92fd91935d4c3b2) ) // some kind of bios? same on both games

	ROM_REGION( 0x100000, "gfx1", 0 ) // 8x8x8 tiles
	ROM_LOAD16_BYTE( "mob11_l1_vrm.21", 0x000000, 0x80000, CRC(926fbd3b) SHA1(4f85ea63faff1508d5abf0ca0ebd16e802f8f45c) )
	ROM_LOAD16_BYTE( "mob11_h1_vrm.23", 0x000001, 0x80000, CRC(a119feaa) SHA1(567e319dfddb9ec04b9302af782e9baccab4f5a6) )

	ROM_REGION( 0x200000, "gfx2", 0 ) // 16x16x8 sprites
	ROM_LOAD16_BYTE( "mob11hh1_obj.11", 0x000000, 0x80000, CRC(fc2ebc0a) SHA1(7c61d05ae1644a2aafc2f81725658b29ce69a091) )
	ROM_LOAD16_BYTE( "mob11hm1_obj.13", 0x000001, 0x80000, CRC(6f84c287) SHA1(edccefa96d97c6f67a9cd02f70cf61385d70daae) )
	ROM_LOAD16_BYTE( "mob11ll1_obj.17", 0x100000, 0x80000, CRC(5c5959a3) SHA1(1eea6bf4c34aa05f45b2737eb6035f2762277cfb) )
	ROM_LOAD16_BYTE( "mob11lm1_obj.15", 0x100001, 0x80000, CRC(0b0e4e9b) SHA1(cbbbde7470f96e9f93fa848371e19ebfeea7fe4d) )

	ROM_REGION( 0x100000, "snd", 0 )
	ROM_LOAD( "snd11sd1_snd.31", 0x000000, 0x80000, CRC(cce53e79) SHA1(970507fcef309c6c81f7e1a8e90afa64f3f6e2ae) )
	ROM_LOAD( "mob11sd2_snd.32", 0x080000, 0x80000, CRC(71ecc441) SHA1(4c94fa3a4ab872b2b841d98b73da89eaec0f46f0) )
ROM_END


GAME( 199?, kingtut,    0,        kongambl,    kongambl,    0, ROT0,  "Konami", "King Tut (NSW)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 199?, moneybnk,   0,        kongambl,    kongambl,    0, ROT0,  "Konami", "Money In The Bank (NSW)", GAME_NOT_WORKING | GAME_NO_SOUND )

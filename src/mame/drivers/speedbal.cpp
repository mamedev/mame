// license:BSD-3-Clause
// copyright-holders:Joseba Epalza,Andreas Naive
/***************************************************************************

 Speed Ball / Music Ball

 this was available in a number of cabinet types including 'Super Pin-Ball'
 which mimicked a Pinball table in design, complete with 7-seg scoreboard.

driver by Joseba Epalza

- 4MHz XTAL, 20MHz XTAL
- Z80 main CPU (4MHz)
- Z80 sound CPU (4MHz)
- YM3812

Video frequency is ~56.4Hz
Interrupt frequency on audio CPU is not a periodical signal, but there are a lot of pulses of 1 MHz

 ======================================================================

  Colors : 2 bits for foreground characters =  4 colors * 16 palettes
           4 bits for background tiles      = 16 colors * 16 palettes
           4 bits for sprites               = 16 colors * 16 palettes

 Note:
 - To enter test mode, keep pressed COIN1 and COIN2 during boot,
   until the RAM / ROM tests are finished

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/3812intf.h"
#include "includes/speedbal.h"
#include "speedbal.lh"


void speedbal_state::machine_start()
{
	save_item(NAME(m_leds_start));
	save_item(NAME(m_leds_shiftreg));
}

WRITE8_MEMBER(speedbal_state::coincounter_w)
{
	machine().bookkeeping().coin_counter_w(0, data & 0x80);
	machine().bookkeeping().coin_counter_w(1, data & 0x40);
	flip_screen_set(data & 8); // also changes data & 0x10 at the same time too (flipx and flipy?)
	/* unknown: (data & 0x10) and (data & 4) */
}

static ADDRESS_MAP_START( main_cpu_map, AS_PROGRAM, 8, speedbal_state )
	AM_RANGE(0x0000, 0xdbff) AM_ROM
	AM_RANGE(0xdc00, 0xdfff) AM_RAM AM_SHARE("share1") // shared with SOUND
	AM_RANGE(0xe000, 0xe1ff) AM_RAM_WRITE(background_videoram_w) AM_SHARE("bg_videoram")
	AM_RANGE(0xe800, 0xefff) AM_RAM_WRITE(foreground_videoram_w) AM_SHARE("fg_videoram")
	AM_RANGE(0xf000, 0xf5ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xf600, 0xfeff) AM_RAM
	AM_RANGE(0xff00, 0xffff) AM_RAM AM_SHARE("spriteram")
ADDRESS_MAP_END

WRITE8_MEMBER(speedbal_state::maincpu_50_w)
{
	//logerror("%s: maincpu_50_w %02x\n", this->machine().describe_context(), data);
}

static ADDRESS_MAP_START( main_cpu_io_map, AS_IO, 8, speedbal_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("DSW2")
	AM_RANGE(0x10, 0x10) AM_READ_PORT("DSW1")
	AM_RANGE(0x20, 0x20) AM_READ_PORT("P1")
	AM_RANGE(0x30, 0x30) AM_READ_PORT("P2")
	AM_RANGE(0x40, 0x40) AM_WRITE(coincounter_w)
	AM_RANGE(0x50, 0x50) AM_WRITE(maincpu_50_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_cpu_map, AS_PROGRAM, 8, speedbal_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xd800, 0xdbff) AM_RAM
	AM_RANGE(0xdc00, 0xdfff) AM_RAM AM_SHARE("share1") // shared with MAIN CPU
ADDRESS_MAP_END



WRITE8_MEMBER(speedbal_state::leds_output_block)
{
	if (!m_leds_start)
		return;

	m_leds_start = false;

	// Each hypothetical led block has 3 7seg leds.
	// The shift register is 28 bits, led block number is in the upper bits
	// and the other 3 bytes in it go to each 7seg led of the current block.
	int block = m_leds_shiftreg >> 24 & 7;
	output().set_digit_value(10 * block + 0, ~m_leds_shiftreg >> 0 & 0xff);
	output().set_digit_value(10 * block + 1, ~m_leds_shiftreg >> 8 & 0xff);
	output().set_digit_value(10 * block + 2, ~m_leds_shiftreg >> 16 & 0xff);
}

WRITE8_MEMBER(speedbal_state::leds_start_block)
{
	m_leds_shiftreg = 0;
	m_leds_start = true;
}

WRITE8_MEMBER(speedbal_state::leds_shift_bit)
{
	m_leds_shiftreg <<= 1;
	m_leds_shiftreg |= (data & 1);
}



static ADDRESS_MAP_START( sound_cpu_io_map, AS_IO, 8, speedbal_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("ymsnd", ym3812_device, read, write)
	AM_RANGE(0x40, 0x40) AM_WRITE(leds_output_block)
	AM_RANGE(0x80, 0x80) AM_WRITE(leds_start_block)
	AM_RANGE(0x82, 0x82) AM_WRITENOP // ?
	AM_RANGE(0xc1, 0xc1) AM_WRITE(leds_shift_bit)
ADDRESS_MAP_END


static INPUT_PORTS_START( speedbal )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x06, "70000 200000 1M" )
	PORT_DIPSETTING(    0x07, "70000 200000" )
	PORT_DIPSETTING(    0x03, "100000 300000 1M" )
	PORT_DIPSETTING(    0x04, "100000 300000" )
	PORT_DIPSETTING(    0x01, "200000 1M" )
	PORT_DIPSETTING(    0x05, "200000" )
	PORT_DIPSETTING(    0x02, "200000 (duplicate)" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, "Difficulty 1" ) PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Difficulty 2" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW , IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW , IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW , IPT_BUTTON4 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW , IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW , IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW , IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW , IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW , IPT_COIN1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW , IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW , IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW , IPT_BUTTON4 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW , IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW , IPT_TILT    )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( musicbal )
	PORT_INCLUDE(speedbal)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x03, "1M 2M 2.5M" )
	PORT_DIPSETTING(    0x06, "1.2M 1.8M 2.5M" )
	PORT_DIPSETTING(    0x07, "1.2M 1.8M" )
	PORT_DIPSETTING(    0x04, "1.5M 2M" )
	PORT_DIPSETTING(    0x05, "1.5M" )
	PORT_DIPSETTING(    0x01, "1.8M 2.5M" )
	PORT_DIPSETTING(    0x02, "1.8M" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,2),   /* 1024 characters */
	4,      /* actually 2 bits per pixel - two of the planes are empty */
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{ 8+3, 8+2, 8+1, 8+0, 3, 2, 1, 0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },   /* characters are rotated 90 degrees */
	16*8       /* every char takes 16 bytes */
};

static const gfx_layout tilelayout =
{
	16,16,  /* 16*16 tiles */
	RGN_FRAC(1,1),   /* 1024 tiles */
	4,      /* 4 bits per pixel */
	{ 0, 2, 4, 6 }, /* the bitplanes are packed in one nibble */
	{ 0*8+0, 0*8+1, 7*8+0, 7*8+1, 6*8+0, 6*8+1, 5*8+0, 5*8+1,
			4*8+0, 4*8+1, 3*8+0, 3*8+1, 2*8+0, 2*8+1, 1*8+0, 1*8+1 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8  /* every sprite takes 128 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,1),    /* 512 sprites */
	4,      /* 4 bits per pixel */
	{ 0, 2, 4, 6 }, /* the bitplanes are packed in one nibble */
	{ 7*8+1, 7*8+0, 6*8+1, 6*8+0, 5*8+1, 5*8+0, 4*8+1, 4*8+0,
			3*8+1, 3*8+0, 2*8+1, 2*8+0, 1*8+1, 1*8+0, 0*8+1, 0*8+0 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8  /* every sprite takes 128 consecutive bytes */
};

static GFXDECODE_START( speedbal )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,  256, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,  512, 16 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout,   0, 16 )
GFXDECODE_END



static MACHINE_CONFIG_START( speedbal, speedbal_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_4MHz) // 4 MHz
	MCFG_CPU_PROGRAM_MAP(main_cpu_map)
	MCFG_CPU_IO_MAP(main_cpu_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", speedbal_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_4MHz) // 4 MHz
	MCFG_CPU_PROGRAM_MAP(sound_cpu_map)
	MCFG_CPU_IO_MAP(sound_cpu_io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(speedbal_state, irq0_line_hold, 1000/2) // approximate?

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(56.4) // measured
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(speedbal_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", speedbal)
	MCFG_PALETTE_ADD("palette", 768)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBxxxx)
	MCFG_PALETTE_ENDIANNESS(ENDIANNESS_BIG)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_4MHz) // 4 MHz(?)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


DRIVER_INIT_MEMBER(speedbal_state,speedbal)
{
	// sprite tiles are in an odd order, rearrange to simplify video drawing function
	UINT8* rom = memregion("sprites")->base();
	dynamic_buffer temp(0x200*128);

	for (int i=0;i<0x200;i++)
	{
		int j = BITSWAP16(i, 15,14,13,12,11,10,9,8,0,1,2,3,4,5,6,7);
		memcpy(&temp[i*128], rom+j*128, 128);
	}

	memcpy(rom,&temp[0],0x200*128);
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( speedbal )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64K for code: main */
	ROM_LOAD( "sb1.bin",  0x0000,  0x8000, CRC(1c242e34) SHA1(8b2e8983e0834c99761ce2b5ea765dba56e77964) )
	ROM_LOAD( "sb3.bin",  0x8000,  0x8000, CRC(7682326a) SHA1(15a72bf088a9adfaa50c11202b4970e07c309a21) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64K for second CPU: sound */
	ROM_LOAD( "sb2.bin",  0x0000,  0x8000, CRC(e6a6d9b7) SHA1(35d228d13d4305f606fdd84adad1d6e435f4b7ce) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "sb10.bin", 0x00000, 0x08000, CRC(36dea4bf) SHA1(60095f482af4595a39be5ae6def8cd30298c1ef8) )    /* chars */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "sb9.bin",  0x00000, 0x08000, CRC(b567e85e) SHA1(7036792ea70ad48384f348399ed9b136272fedb6) )    /* bg tiles */
	ROM_LOAD( "sb5.bin",  0x08000, 0x08000, CRC(b0eae4ba) SHA1(baee3fcb1399c56efaa5f97912de324d7b38f286) )
	ROM_LOAD( "sb8.bin",  0x10000, 0x08000, CRC(d2bfbdb6) SHA1(b552b055450f438729c83337f561d05b6518ae75) )
	ROM_LOAD( "sb4.bin",  0x18000, 0x08000, CRC(1d23a130) SHA1(aabf7c46f9299ffb8b8ca92839622d000a470a0b) )

	ROM_REGION( 0x10000, "sprites", ROMREGION_INVERT )
	ROM_LOAD( "sb7.bin",  0x00000, 0x08000, CRC(9f1b33d1) SHA1(1f8be8f8e6a2ee99a7dafeead142ccc629fa792d) )   /* sprites */
	ROM_LOAD( "sb6.bin",  0x08000, 0x08000, CRC(0e2506eb) SHA1(56f779266b977819063c475b84ca246fc6d8d6a7) )
ROM_END

ROM_START( musicbal )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64K for code: main - encrypted */
	ROM_LOAD( "01.bin",  0x0000,  0x8000, CRC(412298a2) SHA1(3c3247b466880cd78dd7f7f73911f475352c15df) )
	ROM_LOAD( "03.bin",  0x8000,  0x8000, CRC(fdf14446) SHA1(9e52810ebc2b18d83f349fb78884b3c380d93903) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     /* 64K for second CPU: sound */
	ROM_LOAD( "02.bin",  0x0000,  0x8000, CRC(b7d3840d) SHA1(825289c3ca51284a47cfc4937a18d098183c396a) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "10.bin",  0x00000, 0x08000, CRC(5afd3c42) SHA1(5b9a44ef03e5519c9601bb636eb26768cf800278) )    /* chars */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "09.bin",  0x00000, 0x08000, CRC(dcde4233) SHA1(99f204ddc97ee45330ea3cb0bc2971cd95f8e1ac) )    /* bg tiles */
	ROM_LOAD( "05.bin",  0x08000, 0x08000, CRC(e1eec437) SHA1(ce77f3bb01db80ec69da9639d193d8656f9c6692) )
	ROM_LOAD( "08.bin",  0x10000, 0x08000, CRC(7e7af52b) SHA1(3bb1c5abfb1fe53f01520e93124708df6750d8b5) )
	ROM_LOAD( "04.bin",  0x18000, 0x08000, CRC(bf931a33) SHA1(b2ab5c6103af0e0508f08fd58b425e5acfe9ef8a) )

	ROM_REGION( 0x10000, "sprites", ROMREGION_INVERT ) // still contain Speed Ball logos!
	ROM_LOAD( "07.bin",  0x00000, 0x08000, CRC(310e1e23) SHA1(290f3e1c7b907165fe60a4ebe7a8b04b2451b3b1) )   /* sprites */
	ROM_LOAD( "06.bin",  0x08000, 0x08000, CRC(2e7772f8) SHA1(caded1a72356501282e627e23718c30cb8f09370) )
ROM_END


DRIVER_INIT_MEMBER(speedbal_state,musicbal)
{
	UINT8* rom = memregion("maincpu")->base();

	const UINT8 xorTable[8] = {0x05, 0x06, 0x84, 0x84, 0x00, 0x87, 0x84, 0x84};     // XORs affecting bits #0, #1, #2 & #7
	const int swapTable[4][4] = {                                                   // 4 possible swaps affecting bits #0, #1, #2 & #7
		{1,0,7,2},
		{2,7,0,1},
		{7,2,1,0},
		{0,2,1,7}
	};

	for (int i=0;i<0x8000;i++)
	{
		int addIdx = BIT(i,3)^(BIT(i,5)<<1)^(BIT(i,9)<<2);  // 3 bits of address...
		int xorMask = xorTable[addIdx];                     // ... control the xor...
		int bswIdx = xorMask & 3;                           // ... and the bitswap

		// only bits #0, #1, #2 & #7 are affected
		rom[i] = BITSWAP8(rom[i], swapTable[bswIdx][3], 6,5,4,3, swapTable[bswIdx][2], swapTable[bswIdx][1], swapTable[bswIdx][0]) ^ xorTable[addIdx];
	}

	DRIVER_INIT_CALL(speedbal);
}



GAMEL( 1987, speedbal, 0,        speedbal, speedbal, speedbal_state, speedbal, ROT270, "Tecfri / Desystem S.A.", "Speed Ball", MACHINE_SUPPORTS_SAVE, layout_speedbal )
GAMEL( 1988, musicbal, 0,        speedbal, musicbal, speedbal_state, musicbal, ROT270, "Tecfri / Desystem S.A.", "Music Ball", MACHINE_SUPPORTS_SAVE, layout_speedbal )

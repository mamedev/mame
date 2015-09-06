// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Jaleco Exerion hardware

****************************************************************************

    Exerion is a unique driver in that it has idiosyncracies that are straight
    out of Bizarro World. I submit for your approval:

    * The mystery reads from $d802 - timer-based protection?
    * The freakish graphics encoding scheme, which no other MAME-supported game uses
    * The sprite-ram, and all the funky parameters that go along with it


Stephh's notes (based on the games Z80 code and some tests) :

1) 'exerion'

  - The coin insertion routine (code at 0x0066) is buggy as you get a credit
    on first coin after initialisation even if you need more than 1 coin for 1 credit :
      * when coinage is set to 2C_1C, you get a credit when inserting
        1, 2, 4, 6 ... multiples of 2 coins
      * when coinage is set to 3C_1C, you get a credit when inserting
        1, 3, 6, 9 ... multiples of 3 coins
      * when coinage is set to 4C_1C, you get a credit when inserting
        1, 4, 8, 12 ... multiples of 4 coins
      * when coinage is set to 5C_1C, you get a credit when inserting
        1, 5, 10, 15 ... multiples of 5 coins
  - According to the Dip Switches sheet, difficulty is handled by DSW0 bits 5 and 6.
    In fact, bit 6 determines the overall difficulty (0x40 = OFF easy - 0x00 = ON hard)
    while bit 5 determines enemies' number of bullets (0x20 = OFF for less bullets and
    0x00 = ON for more bullets).
  - When starting a 1 or 2 players game, 2 checksums are computed (code at 0x00e4) :
    one from 0x05f0 to 0x06ee (stored at 0x6030), one from 0x00d8 to 0x01d6 (stored
    at 0x6031). Contents of 0x0625 is also stored to 0x6032.
  - Each time before attract mode sequence starts, a checksum is computed from 0x0000
    to 0x1fff (code at 0x28b8) if 17th score in the high-score table is not 0.
    If checksum doesn't match the hardcoded value (0xb5), you get one more credit
    and you are allowed to continue the game with an extra life (score, charge and
    level are not reset to original values).
  - At the beginning of each life of each player, a checksum is computed from 0x4100
    to 0x4dff (code at 0x07d8) if 1st score in the high-score table is >= 80000.
    If checksum doesn't match the hardcoded value (0x63), you get 255 credits !
    Notice that the displayed number of credits won't be correct as the game
    isn't suppose to allow more than 9 credits.
  - In a 2 players game, when player changes, if player it was player 2 turn,
    values from 0x6030 to 0x6032 (see above) are compared with hard-coded values
    (code at 0x04c8). If they don't match respectively 0xfe, 0xb3 and 0x4c,
    and if 9th score in the high-score table is not 0, the game resets !
  - Before entering player's initials, a checksum is computed from 0x5f00 to 0x5fff
    (code at 0x5bd0) if player has reached level 6 (2nd kind of enemies after bonus
    stage). If checksum doesn't match the hardcoded value (0x9a), the game resets !
  - There is sort of protection routine at 0x4120 which has an effect when
    player loses a life on reaching first bonus stage or after. If values read
    from 0x6008 to 0x600b don't match values from ROM area 0x33c0-0x33ff,
    the game resets. See 'exerion_protection_r' read handler.
  - There is an unknown routine at 0x5f70 which is called when the game boots
    which reads value from 0x600c and see if it matches a hardcoded value (0xbe).
    If they don't match, the game resets after displaying the high-scores table.
  - There is another unknown routine at 0x414e which is called when a game is over
    which reads value from 0x600c and see if it matches value from ROM area
    0x4000-0x400f based on internal timer value for a game at 0x604a. If they don't
    match, its only effect is to set lives to 0, which is always the case when the
    game is over, so it doesn't seem to have any real effect.
    Was it supposed to be called at another time ?
  - The routine at 0x5f90 writes to addresses 0x6008-0x600c values read from AY port A
    (one write after one read). This routine is called by the 2 unknown routines.

2) 'exeriont'

  - The coin insertion routine is fixed in this set (see the subttle changes
    in the code from 0x0077 to 0x0082).
  - The routine at 0x28b8 is the same as in 'exerion' (same hardcoded value).
  - The routine at 0x07d8 is the same as in 'exerion' (same hardcoded value).
  - The routine at 0x04c8 is the same as in 'exerion' (same hardcoded values).
  - The routine at 0x5bd0 is the same as in 'exerion' (same hardcoded value).
  - The routine at 0x4120 is the same as in 'exerion', but data from 0x33c0 to 0x33ff
    is slightly different :

      address   exerion  exeriont
      0x33c1:     0x3e     0x36
      0x33c2:     0x37     0x32
      0x33c8:     0x76     0x7e
      0x33ca:     0x32     0x26
      0x33cb:     0x34     0x1e
      0x33d5:     0x07     0x3f
      0x33fc:     0x76     0x40
      0x33fd:     0x37     0x00
      0x33fe:     0x32     0x00
      0x33ff:     0x26     0x00

  - The routine at 0x5f70 is similar to the one in 'exerion' (hardcoded value = 0x9e).
  - The routine at 0x414e is the same as in 'exerion', but data from 0x4000 to 0x400f
    is slightly different :

      address   exerion  exeriont
      0x4002:     0xb2     0x9e
      0x400f:     0xbe     0x9e

  - The routine at 0x5f90 is the same as in 'exerion'.

3) 'exerionb'

  - This set is based on 'exerion' as the coin insertion routine at 0x0066
    (and as a consequence the bug) is the same.
  - The routine at 0x28b8 has been patched, so you can never see the "continue" feature.
  - The routine at 0x07d8 has been patched, so you can never get 255 credits.
  - The routine at 0x04c8 and the computed values from 0x6030 to 0x6032 are surprisingly
    the same as in 'exerion'.
  - The routine at 0x5bd0 has been patched, so the game can't reset.
  - The "protection" routine at 0x4120 has been patched, so the game can't reset.
  - The first unknown routine at 0x5f70 has been patched, so the game can't reset.
  - The second unknown routine at 0x414e has been patched, so lives can't be set to 0.
  - The routine at 0x5f90 is completely different : it reads values from AY port A,
    but nothing is written to addresses 0x6008-0x600c, and there are lots of writes
    to AY port B (0xd001) due to extra code at 0x0050 and extra data at 0x0040.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/exerion.h"
#include "sound/ay8910.h"


/*************************************
 *
 *  Interrupts & inputs
 *
 *************************************/

/* Players inputs are muxed at 0xa000 */
CUSTOM_INPUT_MEMBER(exerion_state::exerion_controls_r)
{
	static const char *const inname[2] = { "P1", "P2" };
	return ioport(inname[m_cocktail_flip])->read() & 0x3f;
}


INPUT_CHANGED_MEMBER(exerion_state::coin_inserted)
{
	/* coin insertion causes an NMI */
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}



/*************************************
 *
 *  Protection??
 *
 *************************************/

/* This is the first of many Exerion "features". No clue if it's */
/* protection or some sort of timer. */
READ8_MEMBER(exerion_state::exerion_porta_r)
{
	m_porta ^= 0x40;
	return m_porta;
}


WRITE8_MEMBER(exerion_state::exerion_portb_w)
{
	/* pull the expected value from the ROM */
	m_porta = memregion("maincpu")->base()[0x5f76];
	m_portb = data;

	logerror("Port B = %02X\n", data);
}


READ8_MEMBER(exerion_state::exerion_protection_r)
{
	if (space.device().safe_pc() == 0x4143)
		return memregion("maincpu")->base()[0x33c0 + (m_main_ram[0xd] << 2) + offset];
	else
		return m_main_ram[0x8 + offset];
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, exerion_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6008, 0x600b) AM_READ(exerion_protection_r)
	AM_RANGE(0x6000, 0x67ff) AM_RAM AM_SHARE("main_ram")
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x8800, 0x887f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x8800, 0x8bff) AM_RAM
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("IN0")
	AM_RANGE(0xa800, 0xa800) AM_READ_PORT("DSW0")
	AM_RANGE(0xb000, 0xb000) AM_READ_PORT("DSW1")
	AM_RANGE(0xc000, 0xc000) AM_WRITE(exerion_videoreg_w)
	AM_RANGE(0xc800, 0xc800) AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0xd000, 0xd001) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0xd800, 0xd801) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
	AM_RANGE(0xd802, 0xd802) AM_DEVREAD("ay2", ay8910_device, data_r)
ADDRESS_MAP_END



/*************************************
 *
 *  Sub CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( sub_map, AS_PROGRAM, 8, exerion_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x6000, 0x6000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x8000, 0x800c) AM_WRITE(exerion_video_latch_w)
	AM_RANGE(0xa000, 0xa000) AM_READ(exerion_video_timing_r)
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

/* verified from Z80 code */
static INPUT_PORTS_START( exerion )
	PORT_START("IN0")
	PORT_BIT( 0x3f, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, exerion_state,exerion_controls_r, (void *)0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x02, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x04, "5" )
//  PORT_DIPSETTING(    0x05, "5" )                         /* duplicated setting */
//  PORT_DIPSETTING(    0x06, "5" )                         /* duplicated setting */
	PORT_DIPSETTING(    0x07, "254 (Cheat)")
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x08, "20000" )
	PORT_DIPSETTING(    0x10, "30000" )
	PORT_DIPSETTING(    0x18, "40000" )
	PORT_DIPNAME( 0x60, 0x00, DEF_STR( Difficulty ) )       /* see notes */
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_DIPNAME( 0x0e, 0x00, DEF_STR( Coinage ) )          /* see notes */
	PORT_DIPSETTING(    0x0e, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, exerion_state,coin_inserted, 0)

	PORT_START("P1")          /* fake input port */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_START("P2")          /* fake input port */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
INPUT_PORTS_END



/*************************************
 *
 *  Graphics layouts
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0 },
	{ 16*0, 16*1, 16*2, 16*3, 16*4, 16*5, 16*6, 16*7 },
	16*8
};


/* 16 x 16 sprites -- requires reorganizing characters in init_exerion() */
static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{  3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0,
			16+3, 16+2, 16+1, 16+0, 24+3, 24+2, 24+1, 24+0 },
	{ 32*0, 32*1, 32*2, 32*3, 32*4, 32*5, 32*6, 32*7,
			32*8, 32*9, 32*10, 32*11, 32*12, 32*13, 32*14, 32*15 },
	64*8
};


static GFXDECODE_START( exerion )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,         0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,     256, 64 )
	GFXDECODE_SCALE( "gfx2", 0, spritelayout,     256, 64, 2, 2 )
GFXDECODE_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void exerion_state::machine_start()
{
	save_item(NAME(m_porta));
	save_item(NAME(m_portb));
	save_item(NAME(m_cocktail_flip));
	save_item(NAME(m_char_palette));
	save_item(NAME(m_sprite_palette));
	save_item(NAME(m_char_bank));
	save_item(NAME(m_background_latches));
}

void exerion_state::machine_reset()
{
	int i;

	m_porta = 0;
	m_portb = 0;
	m_cocktail_flip = 0;
	m_char_palette = 0;
	m_sprite_palette = 0;
	m_char_bank = 0;

	for (i = 0; i < 13; i++)
		m_background_latches[i] = 0;
}

static MACHINE_CONFIG_START( exerion, exerion_state )

	MCFG_CPU_ADD("maincpu", Z80, EXERION_CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(main_map)

	MCFG_CPU_ADD("sub", Z80, EXERION_CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(sub_map)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(EXERION_PIXEL_CLOCK, EXERION_HTOTAL, EXERION_HBEND, EXERION_HBSTART, EXERION_VTOTAL, EXERION_VBEND, EXERION_VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(exerion_state, screen_update_exerion)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", exerion)
	MCFG_PALETTE_ADD("palette", 256*3)
	MCFG_PALETTE_INDIRECT_ENTRIES(32)
	MCFG_PALETTE_INIT_OWNER(exerion_state, exerion)

	/* audio hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, EXERION_AY8910_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_SOUND_ADD("ay2", AY8910, EXERION_AY8910_CLOCK)
	MCFG_AY8910_PORT_A_READ_CB(READ8(exerion_state, exerion_porta_r))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(exerion_state, exerion_portb_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( exerion )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "exerion.07",   0x0000, 0x2000, CRC(4c78d57d) SHA1(ac702e9ad2bc05493fb1355858667c31c36acfe4) )
	ROM_LOAD( "exerion.08",   0x2000, 0x2000, CRC(dcadc1df) SHA1(91388f617cfaa4289ca1c84c697fcfdd8834ae15) )
	ROM_LOAD( "exerion.09",   0x4000, 0x2000, CRC(34cc4d14) SHA1(511c9de038f7bcaf6f7c96f2cbbe50a80673fa72) )

	ROM_REGION( 0x2000, "sub", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "exerion.05",   0x0000, 0x2000, CRC(32f6bff5) SHA1(a4d0289f9d1d9eea7ca9a32a0616af48da74b401) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "exerion.06",   0x00000, 0x2000, CRC(435a85a4) SHA1(f6846bfee11df754405d4d796e7d8ac0321b6eb6) ) /* fg chars */

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "exerion.11",   0x00000, 0x2000, CRC(f0633a09) SHA1(8989bcb12abadde34777f7c189cfa6e2dfe92d62) ) /* sprites */
	ROM_LOAD( "exerion.10",   0x02000, 0x2000, CRC(80312de0) SHA1(4fa3bb9d5c62e41a54e8909f8d3b47637137e913) )

	ROM_REGION( 0x08000, "gfx3", 0 )
	ROM_LOAD( "exerion.03",   0x00000, 0x2000, CRC(790595b8) SHA1(8016ac2394b25db38e962bcff4805380082f6683) ) /* bg data */
	ROM_LOAD( "exerion.04",   0x02000, 0x2000, CRC(d7abd0b9) SHA1(ca6413ecd324cf84e11b703a4eda2c1e6d28ff15) )
	ROM_LOAD( "exerion.01",   0x04000, 0x2000, CRC(5bb755cb) SHA1(ec92c518c116a78dbb23381468cefb3f930212cc) )
	ROM_LOAD( "exerion.02",   0x06000, 0x2000, CRC(a7ecbb70) SHA1(3c359d5bb21290a45d3eb18fea2b1f9439b931be) )

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "exerion.e1",   0x0000, 0x0020, CRC(2befcc20) SHA1(a24d3f691413378fde545a6ddcef7e5118e74019) ) /* palette */
	ROM_LOAD( "exerion.i8",   0x0020, 0x0100, CRC(31db0e08) SHA1(1041a778e86d3fe6f057cf40a0a08b30760f3887) ) /* fg char lookup table */
	ROM_LOAD( "exerion.h10",  0x0120, 0x0100, CRC(63b4c555) SHA1(30243041be4fa77ada71e8b29d721cad51640c29) ) /* sprite lookup table */
	ROM_LOAD( "exerion.i3",   0x0220, 0x0100, CRC(fe72ab79) SHA1(048a72e6db4768df687df927acaa70ef906b3dc0) ) /* bg char lookup table */
	ROM_LOAD( "exerion.k4",   0x0320, 0x0100, CRC(ffc2ba43) SHA1(03be1c41d6ac3fc11439caef04ef5ffa60d6aec4) ) /* bg char mixer */
ROM_END


ROM_START( exeriont )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "prom5.4p",     0x0000, 0x4000, CRC(58b4dc1b) SHA1(3e34d1eda0b0537dac1062e96259d4cc7c64049c) )
	ROM_LOAD( "prom6.4s",     0x4000, 0x2000, CRC(fca18c2d) SHA1(31077dada3ed4aa2e26af933f589e01e0c71e5cd) )

	ROM_REGION( 0x2000, "sub", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "exerion.05",   0x0000, 0x2000, CRC(32f6bff5) SHA1(a4d0289f9d1d9eea7ca9a32a0616af48da74b401) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "exerion.06",   0x00000, 0x2000, CRC(435a85a4) SHA1(f6846bfee11df754405d4d796e7d8ac0321b6eb6) ) /* fg chars */

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "exerion.11",   0x00000, 0x2000, CRC(f0633a09) SHA1(8989bcb12abadde34777f7c189cfa6e2dfe92d62) ) /* sprites */
	ROM_LOAD( "exerion.10",   0x02000, 0x2000, CRC(80312de0) SHA1(4fa3bb9d5c62e41a54e8909f8d3b47637137e913) )

	ROM_REGION( 0x08000, "gfx3", 0 )
	ROM_LOAD( "exerion.03",   0x00000, 0x2000, CRC(790595b8) SHA1(8016ac2394b25db38e962bcff4805380082f6683) ) /* bg data */
	ROM_LOAD( "exerion.04",   0x02000, 0x2000, CRC(d7abd0b9) SHA1(ca6413ecd324cf84e11b703a4eda2c1e6d28ff15) )
	ROM_LOAD( "exerion.01",   0x04000, 0x2000, CRC(5bb755cb) SHA1(ec92c518c116a78dbb23381468cefb3f930212cc) )
	ROM_LOAD( "exerion.02",   0x06000, 0x2000, CRC(a7ecbb70) SHA1(3c359d5bb21290a45d3eb18fea2b1f9439b931be) )

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "exerion.e1",   0x0000, 0x0020, CRC(2befcc20) SHA1(a24d3f691413378fde545a6ddcef7e5118e74019) ) /* palette */
	ROM_LOAD( "exerion.i8",   0x0020, 0x0100, CRC(31db0e08) SHA1(1041a778e86d3fe6f057cf40a0a08b30760f3887) ) /* fg char lookup table */
	ROM_LOAD( "exerion.h10",  0x0120, 0x0100, CRC(63b4c555) SHA1(30243041be4fa77ada71e8b29d721cad51640c29) ) /* sprite lookup table */
	ROM_LOAD( "exerion.i3",   0x0220, 0x0100, CRC(fe72ab79) SHA1(048a72e6db4768df687df927acaa70ef906b3dc0) ) /* bg char lookup table */
	ROM_LOAD( "exerion.k4",   0x0320, 0x0100, CRC(ffc2ba43) SHA1(03be1c41d6ac3fc11439caef04ef5ffa60d6aec4) ) /* bg char mixer */
ROM_END


ROM_START( exerionb )
	ROM_REGION( 0x6000, "maincpu", 0 )
	ROM_LOAD( "eb5.bin",      0x0000, 0x4000, CRC(da175855) SHA1(11ea46fd1d504e16e5ffc604d74c1ce210d6be1c) )
	ROM_LOAD( "eb6.bin",      0x4000, 0x2000, CRC(0dbe2eff) SHA1(5b0e5e8453619beec46c4350d1b2ed571fe3dc24) )

	ROM_REGION( 0x2000, "sub", 0 )     /* 64k for the second CPU */
	ROM_LOAD( "exerion.05",   0x0000, 0x2000, CRC(32f6bff5) SHA1(a4d0289f9d1d9eea7ca9a32a0616af48da74b401) )

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "exerion.06",   0x00000, 0x2000, CRC(435a85a4) SHA1(f6846bfee11df754405d4d796e7d8ac0321b6eb6) ) /* fg chars */

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "exerion.11",   0x00000, 0x2000, CRC(f0633a09) SHA1(8989bcb12abadde34777f7c189cfa6e2dfe92d62) ) /* sprites */
	ROM_LOAD( "exerion.10",   0x02000, 0x2000, CRC(80312de0) SHA1(4fa3bb9d5c62e41a54e8909f8d3b47637137e913) )

	ROM_REGION( 0x08000, "gfx3", 0 )
	ROM_LOAD( "exerion.03",   0x00000, 0x2000, CRC(790595b8) SHA1(8016ac2394b25db38e962bcff4805380082f6683) ) /* bg data */
	ROM_LOAD( "exerion.04",   0x02000, 0x2000, CRC(d7abd0b9) SHA1(ca6413ecd324cf84e11b703a4eda2c1e6d28ff15) )
	ROM_LOAD( "exerion.01",   0x04000, 0x2000, CRC(5bb755cb) SHA1(ec92c518c116a78dbb23381468cefb3f930212cc) )
	ROM_LOAD( "exerion.02",   0x06000, 0x2000, CRC(a7ecbb70) SHA1(3c359d5bb21290a45d3eb18fea2b1f9439b931be) )

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "exerion.e1",   0x0000, 0x0020, CRC(2befcc20) SHA1(a24d3f691413378fde545a6ddcef7e5118e74019) ) /* palette */
	ROM_LOAD( "exerion.i8",   0x0020, 0x0100, CRC(31db0e08) SHA1(1041a778e86d3fe6f057cf40a0a08b30760f3887) ) /* fg char lookup table */
	ROM_LOAD( "exerion.h10",  0x0120, 0x0100, CRC(63b4c555) SHA1(30243041be4fa77ada71e8b29d721cad51640c29) ) /* sprite lookup table */
	ROM_LOAD( "exerion.i3",   0x0220, 0x0100, CRC(fe72ab79) SHA1(048a72e6db4768df687df927acaa70ef906b3dc0) ) /* bg char lookup table */
	ROM_LOAD( "exerion.k4",   0x0320, 0x0100, CRC(ffc2ba43) SHA1(03be1c41d6ac3fc11439caef04ef5ffa60d6aec4) ) /* bg char mixer */
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(exerion_state,exerion)
{
	UINT32 oldaddr, newaddr, length;
	UINT8 *src, *dst;

	/* allocate some temporary space */
	dynamic_buffer temp(0x10000);

	/* make a temporary copy of the character data */
	src = &temp[0];
	dst = memregion("gfx1")->base();
	length = memregion("gfx1")->bytes();
	memcpy(src, dst, length);

	/* decode the characters */
	/* the bits in the ROM are ordered: n8-n7 n6 n5 n4-v2 v1 v0 n3-n2 n1 n0 h2 */
	/* we want them ordered like this:  n8-n7 n6 n5 n4-n3 n2 n1 n0-v2 v1 v0 h2 */
	for (oldaddr = 0; oldaddr < length; oldaddr++)
	{
		newaddr = ((oldaddr     ) & 0x1f00) |       /* keep n8-n4 */
					((oldaddr << 3) & 0x00f0) |       /* move n3-n0 */
					((oldaddr >> 4) & 0x000e) |       /* move v2-v0 */
					((oldaddr     ) & 0x0001);        /* keep h2 */
		dst[newaddr] = src[oldaddr];
	}

	/* make a temporary copy of the sprite data */
	src = &temp[0];
	dst = memregion("gfx2")->base();
	length = memregion("gfx2")->bytes();
	memcpy(src, dst, length);

	/* decode the sprites */
	/* the bits in the ROMs are ordered: n9 n8 n3 n7-n6 n5 n4 v3-v2 v1 v0 n2-n1 n0 h3 h2 */
	/* we want them ordered like this:   n9 n8 n7 n6-n5 n4 n3 n2-n1 n0 v3 v2-v1 v0 h3 h2 */
	for (oldaddr = 0; oldaddr < length; oldaddr++)
	{
		newaddr = ((oldaddr << 1) & 0x3c00) |       /* move n7-n4 */
					((oldaddr >> 4) & 0x0200) |       /* move n3 */
					((oldaddr << 4) & 0x01c0) |       /* move n2-n0 */
					((oldaddr >> 3) & 0x003c) |       /* move v3-v0 */
					((oldaddr     ) & 0xc003);        /* keep n9-n8 h3-h2 */
		dst[newaddr] = src[oldaddr];
	}
}


DRIVER_INIT_MEMBER(exerion_state,exerionb)
{
	UINT8 *ram = memregion("maincpu")->base();
	int addr;

	/* the program ROMs have data lines D1 and D2 swapped. Decode them. */
	for (addr = 0; addr < 0x6000; addr++)
		ram[addr] = (ram[addr] & 0xf9) | ((ram[addr] & 2) << 1) | ((ram[addr] & 4) >> 1);

	/* also convert the gfx as in Exerion */
	DRIVER_INIT_CALL(exerion);
}



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1983, exerion,  0,       exerion, exerion, exerion_state, exerion,  ROT90, "Jaleco", "Exerion", MACHINE_SUPPORTS_SAVE )
GAME( 1983, exeriont, exerion, exerion, exerion, exerion_state, exerion,  ROT90, "Jaleco (Taito America license)", "Exerion (Taito)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, exerionb, exerion, exerion, exerion, exerion_state, exerionb, ROT90, "bootleg", "Exerion (bootleg)", MACHINE_SUPPORTS_SAVE )

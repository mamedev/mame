// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Pinball Action memory map (preliminary)

driver by Nicola Salmoria


0000-9fff ROM
d000-d3ff Video RAM
d400-d7ff Color RAM
d800-dbff Background Video RAM
dc00-dfff Background Color RAM
e000-e07f Sprites
e400-e5ff Palette RAM

read:
e600      IN0
e601      IN1
e602      IN2
e604      DSW1
e605      DSW2
e606      watchdog reset????

write:
e600      interrupt enable
e604      flip screen
e606      bg scroll? not sure
e800      command for the sound CPU


Notes:
- pbactio2 has a ROM for a third Z80, not emulated, function unknown


Stephh's notes (based on the game Z80 code and some tests) :

  - There is an ingame bug that prevents you to get a bonus life at 1000000 points
    when you set the "Bonus Life" Dip Switch to "200k 1000k" :
      * Bonus life table index starts at 0x63c6 (8 * 2 butes, LSB first) :

          63C6: D6 63   "70k 200k"          -> 04 07 03 02 01 10
          63C8: DC 63   "70k 200k 1000k"    -> 04 07 03 02 02 01 01 10
          63CA: E4 63   "100k"              -> 03 01 01 10
          63CC: E8 63   "100k 300k"         -> 03 01 03 03 01 10
          63CE: EE 63   "100k 300k 1000k"   -> 03 01 03 03 02 01 01 10
          63D0: F6 63   "200k"              -> 03 02 01 10
          63D2: FA 63   "200k 1000k"        -> 03 02 01 10 !!!
          63D4: FE 63   "None"              -> 01 10

      * Each "pair" determines the digit number, then what shall be its value :

          digit  : 12 345 67
          number : 99.999.990

        Note that digit 1 is displayed outside the score box.
      * As each digit value can only be 00 to 09, 10 as a bonus life value
        means that you can't get anymore bonus life.
      * Now look at 7th table : first, you notice that it's the same as the
        6th table; then you find that the first bonus life at 200k and you see
        the end of table "marker" (01 10) instead of having 02 01.
      * As addresses and data are the same (after decryption in 'pbactio3'),
        this bug affects the 3 sets.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "machine/segacrpt.h"
#include "includes/pbaction.h"


WRITE8_MEMBER(pbaction_state::pbaction_sh_command_w)
{
	soundlatch_byte_w(space, offset, data);
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0x00);
}

WRITE8_MEMBER(pbaction_state::nmi_mask_w)
{
	m_nmi_mask = data & 1;
}

static ADDRESS_MAP_START( pbaction_map, AS_PROGRAM, 8, pbaction_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xcfff) AM_RAM AM_SHARE("work_ram")
	AM_RANGE(0xd000, 0xd3ff) AM_RAM_WRITE(pbaction_videoram2_w) AM_SHARE("videoram2")
	AM_RANGE(0xd400, 0xd7ff) AM_RAM_WRITE(pbaction_colorram2_w) AM_SHARE("colorram2")
	AM_RANGE(0xd800, 0xdbff) AM_RAM_WRITE(pbaction_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xdc00, 0xdfff) AM_RAM_WRITE(pbaction_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xe000, 0xe07f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xe400, 0xe5ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xe600, 0xe600) AM_READ_PORT("P1") AM_WRITE(nmi_mask_w)
	AM_RANGE(0xe601, 0xe601) AM_READ_PORT("P2")
	AM_RANGE(0xe602, 0xe602) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xe604, 0xe604) AM_READ_PORT("DSW1") AM_WRITE(pbaction_flipscreen_w)
	AM_RANGE(0xe605, 0xe605) AM_READ_PORT("DSW2")
	AM_RANGE(0xe606, 0xe606) AM_READNOP /* ??? */ AM_WRITE(pbaction_scroll_w)
	AM_RANGE(0xe800, 0xe800) AM_WRITE(pbaction_sh_command_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 8, pbaction_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_SHARE("decrypted_opcodes")
	AM_RANGE(0x8000, 0xbfff) AM_ROM AM_REGION("maincpu", 0x8000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( pbaction_sound_map, AS_PROGRAM, 8, pbaction_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x4000, 0x47ff) AM_RAM
	AM_RANGE(0x8000, 0x8000) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xffff, 0xffff) AM_WRITENOP    /* watchdog? */
ADDRESS_MAP_END


static ADDRESS_MAP_START( pbaction_sound_io_map, AS_IO, 8, pbaction_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x11) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0x20, 0x21) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
	AM_RANGE(0x30, 0x31) AM_DEVWRITE("ay3", ay8910_device, address_data_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( pbaction )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:!1,!2,!3")
	PORT_DIPSETTING(    0x01, "70k 200k 1000k" )
	PORT_DIPSETTING(    0x04, "100k 300k 1000k" )
	PORT_DIPSETTING(    0x00, "70k 200k" )
	PORT_DIPSETTING(    0x03, "100k 300k" )
	PORT_DIPSETTING(    0x06, "200k 1000k" )                /* see notes */
	PORT_DIPSETTING(    0x02, "100k" )
	PORT_DIPSETTING(    0x05, "200k" )
	PORT_DIPSETTING(    0x07, DEF_STR( None ) )
	PORT_DIPNAME( 0x08, 0x00, "Extra" )         PORT_DIPLOCATION("SW2:!4")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x30, 0x00, "Difficulty (Flippers)" ) PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc0, 0x00, "Difficulty (Outlanes)" ) PORT_DIPLOCATION("SW2:!7,!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Hardest ) )
INPUT_PORTS_END



static const gfx_layout charlayout1 =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};
static const gfx_layout charlayout2 =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};
static const gfx_layout spritelayout1 =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(0,1), STEP8(64,1) },
	{ STEP8(0,8), STEP8(128,8) },
	32*8
};
static const gfx_layout spritelayout2 =
{
	32,32,
	RGN_FRAC(1,6),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(0,1), STEP8(64,1), STEP8(256,1), STEP8(320,1) },
	{ STEP8(0,8), STEP8(128,8), STEP8(512,8), STEP8(640,8) },
	128*8
};



static GFXDECODE_START( pbaction )
	GFXDECODE_ENTRY( "fgchars", 0x00000, charlayout1,    0, 16 )    /*   0-127 characters */
	GFXDECODE_ENTRY( "bgchars", 0x00000, charlayout2,  128,  8 )    /* 128-255 background */
	GFXDECODE_ENTRY( "sprites", 0x00000, spritelayout1,  0, 16 )    /*   0-127 normal sprites */
	GFXDECODE_ENTRY( "sprites", 0x01000, spritelayout2,  0, 16 )    /*   0-127 large sprites */
GFXDECODE_END


INTERRUPT_GEN_MEMBER(pbaction_state::pbaction_interrupt)
{
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0x02); /* the CPU is in Interrupt Mode 2 */
}


void pbaction_state::machine_start()
{
	save_item(NAME(m_scroll));
}

void pbaction_state::machine_reset()
{
	m_scroll = 0;
}

INTERRUPT_GEN_MEMBER(pbaction_state::vblank_irq)
{
	if(m_nmi_mask)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_START( pbaction, pbaction_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)   /* 4 MHz? */
	MCFG_CPU_PROGRAM_MAP(pbaction_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pbaction_state,  vblank_irq)

	MCFG_CPU_ADD("audiocpu", Z80, 3072000)
	MCFG_CPU_PROGRAM_MAP(pbaction_sound_map)
	MCFG_CPU_IO_MAP(pbaction_sound_io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(pbaction_state, pbaction_interrupt, 2*60)  /* ??? */
									/* IRQs are caused by the main CPU */


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(pbaction_state, screen_update_pbaction)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pbaction)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay2", AY8910, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay3", AY8910, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pbactionx, pbaction )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( pbaction )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b-p7.bin",     0x0000, 0x4000, CRC(8d6dcaae) SHA1(c9e605f9d291cb8c7163655ea96c605b7d30365f) )
	ROM_LOAD( "b-n7.bin",     0x4000, 0x4000, CRC(d54d5402) SHA1(a4c3205bfe5fba8bb1ff3ad15941a77c35b44a27) )
	ROM_LOAD( "b-l7.bin",     0x8000, 0x2000, CRC(e7412d68) SHA1(e75731d9bea80e0dc09798dd46e3b947fdb54aaa) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound board */
	ROM_LOAD( "a-e3.bin",     0x0000,  0x2000, CRC(0e53a91f) SHA1(df2827197cd55c3685e5ac8b26c20800623cb932) )

	ROM_REGION( 0x06000, "fgchars", 0 )
	ROM_LOAD( "a-s6.bin",     0x00000, 0x2000, CRC(9a74a8e1) SHA1(bd27439b91f41db3fd7eedb44e828d61b793bda0) )
	ROM_LOAD( "a-s7.bin",     0x02000, 0x2000, CRC(5ca6ad3c) SHA1(7c8eff087f18cc2ff0572ea45e681a3a1ec94fad) )
	ROM_LOAD( "a-s8.bin",     0x04000, 0x2000, CRC(9f00b757) SHA1(74b6d926b8f456c8d0101f0232c5d3662423b396) )

	ROM_REGION( 0x10000, "bgchars", 0 )
	ROM_LOAD( "a-j5.bin",     0x00000, 0x4000, CRC(21efe866) SHA1(0c0a05a26d793ba98b0f421d464ff4b1d301ff9e) )
	ROM_LOAD( "a-j6.bin",     0x04000, 0x4000, CRC(7f984c80) SHA1(18795ecbcd2da94f1cfcce5559d652388d1b8bc0) )
	ROM_LOAD( "a-j7.bin",     0x08000, 0x4000, CRC(df69e51b) SHA1(52ab15c63332f0fa98884fa9adc8d35b93c939c4) )
	ROM_LOAD( "a-j8.bin",     0x0c000, 0x4000, CRC(0094cb8b) SHA1(58f48d24903b797e8451bf231f9e8df621685d9f) )

	ROM_REGION( 0x06000, "sprites", 0 )
	ROM_LOAD( "b-c7.bin",     0x00000, 0x2000, CRC(d1795ef5) SHA1(69ad8e419e340d2f548468ed7838102789b978da) )
	ROM_LOAD( "b-d7.bin",     0x02000, 0x2000, CRC(f28df203) SHA1(060f70ed6386c808303a488c97691257681bd8f3) )
	ROM_LOAD( "b-f7.bin",     0x04000, 0x2000, CRC(af6e9817) SHA1(56f47d25761b3850c49a3a81b5ea35f12bd77b14) )
ROM_END


ROM_START( pbaction2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pba16.bin",     0x0000, 0x4000, CRC(4a239ebd) SHA1(74e6da0485ac78093b4f09953fa3accb14bc3e43) )
	ROM_LOAD( "pba15.bin",     0x4000, 0x4000, CRC(3afef03a) SHA1(dec714415d2fd00c9021171a48f6c94b40888ae8) )
	ROM_LOAD( "pba14.bin",     0x8000, 0x2000, CRC(c0a98c8a) SHA1(442f37af31db13fd98602dd7f9eeae5529da0f44) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound board */
	ROM_LOAD( "pba1.bin",     0x0000,  0x2000, CRC(8b69b933) SHA1(eb0762579d52ed9f5b1a002ffe7e517c59650e22) )

	ROM_REGION( 0x10000, "cpu2", 0 )    /* 64k for a third Z80 (not emulated) */
	ROM_LOAD( "pba17.bin",    0x0000,  0x4000, CRC(2734ae60) SHA1(4edcdfac1611c49c4f890609efbe8352b8161f8e) )

	ROM_REGION( 0x06000, "fgchars", 0 )
	ROM_LOAD( "a-s6.bin",     0x00000, 0x2000, CRC(9a74a8e1) SHA1(bd27439b91f41db3fd7eedb44e828d61b793bda0) )
	ROM_LOAD( "a-s7.bin",     0x02000, 0x2000, CRC(5ca6ad3c) SHA1(7c8eff087f18cc2ff0572ea45e681a3a1ec94fad) )
	ROM_LOAD( "a-s8.bin",     0x04000, 0x2000, CRC(9f00b757) SHA1(74b6d926b8f456c8d0101f0232c5d3662423b396) )

	ROM_REGION( 0x10000, "bgchars", 0 )
	ROM_LOAD( "a-j5.bin",     0x00000, 0x4000, CRC(21efe866) SHA1(0c0a05a26d793ba98b0f421d464ff4b1d301ff9e) )
	ROM_LOAD( "a-j6.bin",     0x04000, 0x4000, CRC(7f984c80) SHA1(18795ecbcd2da94f1cfcce5559d652388d1b8bc0) )
	ROM_LOAD( "a-j7.bin",     0x08000, 0x4000, CRC(df69e51b) SHA1(52ab15c63332f0fa98884fa9adc8d35b93c939c4) )
	ROM_LOAD( "a-j8.bin",     0x0c000, 0x4000, CRC(0094cb8b) SHA1(58f48d24903b797e8451bf231f9e8df621685d9f) )

	ROM_REGION( 0x06000, "sprites", 0 )
	ROM_LOAD( "b-c7.bin",     0x00000, 0x2000, CRC(d1795ef5) SHA1(69ad8e419e340d2f548468ed7838102789b978da) )
	ROM_LOAD( "b-d7.bin",     0x02000, 0x2000, CRC(f28df203) SHA1(060f70ed6386c808303a488c97691257681bd8f3) )
	ROM_LOAD( "b-f7.bin",     0x04000, 0x2000, CRC(af6e9817) SHA1(56f47d25761b3850c49a3a81b5ea35f12bd77b14) )
ROM_END

ROM_START( pbaction3 )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "14.bin",     0x0000, 0x4000, CRC(f17a62eb) SHA1(8dabfc0ad127c154c0293a65df32d52d57dd9755) )
	ROM_LOAD( "12.bin",     0x4000, 0x4000, CRC(ec3c64c6) SHA1(6130b80606d717f95e219316c2d3fa0a1980ea1d) )
	ROM_LOAD( "13.bin",     0x8000, 0x4000, CRC(c93c851e) SHA1(b41077708fce4ccbcecdeae32af8821ca5322e87) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound board */
	ROM_LOAD( "pba1.bin",     0x0000,  0x2000, CRC(8b69b933) SHA1(eb0762579d52ed9f5b1a002ffe7e517c59650e22) )

	ROM_REGION( 0x06000, "fgchars", 0 )
	ROM_LOAD( "a-s6.bin",     0x00000, 0x2000, CRC(9a74a8e1) SHA1(bd27439b91f41db3fd7eedb44e828d61b793bda0) )
	ROM_LOAD( "a-s7.bin",     0x02000, 0x2000, CRC(5ca6ad3c) SHA1(7c8eff087f18cc2ff0572ea45e681a3a1ec94fad) )
	ROM_LOAD( "a-s8.bin",     0x04000, 0x2000, CRC(9f00b757) SHA1(74b6d926b8f456c8d0101f0232c5d3662423b396) )

	ROM_REGION( 0x10000, "bgchars", 0 )
	ROM_LOAD( "a-j5.bin",     0x00000, 0x4000, CRC(21efe866) SHA1(0c0a05a26d793ba98b0f421d464ff4b1d301ff9e) )
	ROM_LOAD( "a-j6.bin",     0x04000, 0x4000, CRC(7f984c80) SHA1(18795ecbcd2da94f1cfcce5559d652388d1b8bc0) )
	ROM_LOAD( "a-j7.bin",     0x08000, 0x4000, CRC(df69e51b) SHA1(52ab15c63332f0fa98884fa9adc8d35b93c939c4) )
	ROM_LOAD( "a-j8.bin",     0x0c000, 0x4000, CRC(0094cb8b) SHA1(58f48d24903b797e8451bf231f9e8df621685d9f) )

	ROM_REGION( 0x06000, "sprites", 0 )
	ROM_LOAD( "b-c7.bin",     0x00000, 0x2000, CRC(d1795ef5) SHA1(69ad8e419e340d2f548468ed7838102789b978da) )
	ROM_LOAD( "b-d7.bin",     0x02000, 0x2000, CRC(f28df203) SHA1(060f70ed6386c808303a488c97691257681bd8f3) )
	ROM_LOAD( "b-f7.bin",     0x04000, 0x2000, CRC(af6e9817) SHA1(56f47d25761b3850c49a3a81b5ea35f12bd77b14) )
ROM_END

ROM_START( pbaction4 )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "pinball_09.bin",     0x0000, 0x4000, CRC(c8e81ece) SHA1(04eafbd79263225f6c6fb5f04951b54179144f17) )
	ROM_IGNORE(0x4000)
	ROM_LOAD( "pinball_10.bin",     0x4000, 0x8000, CRC(04b56c7c) SHA1(d09c22fd0235e1c6a9b1978ba69338bb1ae5667d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound board */
	ROM_LOAD( "pinball_01.bin",     0x0000,  0x2000, CRC(8b69b933) SHA1(eb0762579d52ed9f5b1a002ffe7e517c59650e22) )

	ROM_REGION( 0x06000, "fgchars", 0 )
	ROM_LOAD( "pinball_06.bin",     0x00000, 0x2000, CRC(9a74a8e1) SHA1(bd27439b91f41db3fd7eedb44e828d61b793bda0) )
	ROM_LOAD( "pinball_07.bin",     0x02000, 0x2000, CRC(5ca6ad3c) SHA1(7c8eff087f18cc2ff0572ea45e681a3a1ec94fad) )
	ROM_LOAD( "pinball_08.bin",     0x04000, 0x2000, CRC(9f00b757) SHA1(74b6d926b8f456c8d0101f0232c5d3662423b396) )

	ROM_REGION( 0x10000, "bgchars", 0 )
	ROM_LOAD( "pinball_02.bin",     0x00000, 0x4000, CRC(01ba32c9) SHA1(196f8c6e037a7ebdcefc80b453f3801b3b6eb075) )
	ROM_IGNORE(0x4000)
	ROM_LOAD( "pinball_03.bin",     0x04000, 0x4000, CRC(f605ae40) SHA1(9a28869014d2df513090d15ccb478de9f5d65b24) )
	ROM_IGNORE(0x4000)
	ROM_LOAD( "pinball_04.bin",     0x08000, 0x4000, CRC(9e23a780) SHA1(2d49ee79b9261bdb8367b28bfca9940b4527c87a) )
	ROM_IGNORE(0x4000)
	ROM_LOAD( "pinball_05.bin",     0x0c000, 0x4000, CRC(c9a4dfea) SHA1(38fb34f21773d652b14108e4a083d7c7acecdd03) )
	ROM_IGNORE(0x4000)

	ROM_REGION( 0x06000, "sprites", 0 )
	ROM_LOAD( "pinball_14.bin",     0x00000, 0x2000, CRC(d1795ef5) SHA1(69ad8e419e340d2f548468ed7838102789b978da) )
	ROM_LOAD( "pinball_13.bin",     0x02000, 0x2000, CRC(f28df203) SHA1(060f70ed6386c808303a488c97691257681bd8f3) )
	ROM_LOAD( "pinball_12.bin",     0x04000, 0x2000, CRC(af6e9817) SHA1(56f47d25761b3850c49a3a81b5ea35f12bd77b14) )
ROM_END

ROM_START( pbaction5 )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "p16.bin",     0x0000, 0x4000, CRC(ad20b360) SHA1(91e3cdceb1c170580d926b2ed8359c3100f71b11) )
	ROM_LOAD( "c15.bin",     0x4000, 0x4000, CRC(057acfe3) SHA1(49c184d7caea0c0e9f0d0e163f2ef42bb9aebf16) )
	ROM_LOAD( "p14.bin",     0x8000, 0x2000, CRC(e7412d68) SHA1(e75731d9bea80e0dc09798dd46e3b947fdb54aaa) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for sound board */
	ROM_LOAD( "p1.bin",     0x0000,  0x2000, CRC(8b69b933) SHA1(eb0762579d52ed9f5b1a002ffe7e517c59650e22) )

	ROM_REGION( 0x06000, "fgchars", 0 )
	ROM_LOAD( "p7.bin",     0x00000, 0x2000, CRC(9a74a8e1) SHA1(bd27439b91f41db3fd7eedb44e828d61b793bda0) )
	ROM_LOAD( "p8.bin",     0x02000, 0x2000, CRC(5ca6ad3c) SHA1(7c8eff087f18cc2ff0572ea45e681a3a1ec94fad) )
	ROM_LOAD( "p9.bin",     0x04000, 0x2000, CRC(9f00b757) SHA1(74b6d926b8f456c8d0101f0232c5d3662423b396) )

	ROM_REGION( 0x10000, "bgchars", 0 )
	ROM_LOAD( "p2.bin",     0x00000, 0x4000, CRC(21efe866) SHA1(0c0a05a26d793ba98b0f421d464ff4b1d301ff9e) )
	ROM_LOAD( "p3.bin",     0x04000, 0x4000, CRC(7f984c80) SHA1(18795ecbcd2da94f1cfcce5559d652388d1b8bc0) )
	ROM_LOAD( "p4.bin",     0x08000, 0x4000, CRC(df69e51b) SHA1(52ab15c63332f0fa98884fa9adc8d35b93c939c4) )
	ROM_LOAD( "p5.bin",     0x0c000, 0x4000, CRC(0094cb8b) SHA1(58f48d24903b797e8451bf231f9e8df621685d9f) )

	ROM_REGION( 0x06000, "sprites", 0 )
	ROM_LOAD( "p11.bin",     0x00000, 0x2000, CRC(d1795ef5) SHA1(69ad8e419e340d2f548468ed7838102789b978da) )
	ROM_LOAD( "p12.bin",     0x02000, 0x2000, CRC(f28df203) SHA1(060f70ed6386c808303a488c97691257681bd8f3) )
	ROM_LOAD( "p13.bin",     0x04000, 0x2000, CRC(af6e9817) SHA1(56f47d25761b3850c49a3a81b5ea35f12bd77b14) )
ROM_END

READ8_MEMBER(pbaction_state::pbactio3_prot_kludge_r)
{
	/* on startup, the game expect this location to NOT act as RAM */
	if (space.device().safe_pc() == 0xab80)
		return 0;

	return m_work_ram[0];
}

DRIVER_INIT_MEMBER(pbaction_state,pbactio3)
{
	int i;
	UINT8 *rom = memregion("maincpu")->base();

	/* first of all, do a simple bitswap */
	for (i = 0; i < 0xc000; i++)
	{
		rom[i] = BITSWAP8(rom[i], 7,6,5,4,1,2,3,0);
	}

	/* then do the standard Sega decryption */
	DRIVER_INIT_CALL(pbactio4);

	/* install a protection (?) workaround */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc000, 0xc000, read8_delegate(FUNC(pbaction_state::pbactio3_prot_kludge_r),this) );
}

DRIVER_INIT_MEMBER(pbaction_state,pbactio4)
{
	static const UINT8 convtable[32][4] =
	{
		/*       opcode                   data                     address      */
		/*  A    B    C    D         A    B    C    D                           */
		{ 0xa8,0xa0,0x88,0x80 }, { 0x28,0xa8,0x08,0x88 },   /* ...0...0...0...0 */
		{ 0x28,0x08,0xa8,0x88 }, { 0xa8,0xa0,0x88,0x80 },   /* ...0...0...0...1 */
		{ 0x28,0x20,0xa8,0xa0 }, { 0x28,0xa8,0x08,0x88 },   /* ...0...0...1...0 */
		{ 0x28,0x08,0xa8,0x88 }, { 0x28,0x20,0xa8,0xa0 },   /* ...0...0...1...1 */
		{ 0xa8,0xa0,0x88,0x80 }, { 0xa8,0xa0,0x88,0x80 },   /* ...0...1...0...0 */
		{ 0x28,0x20,0xa8,0xa0 }, { 0x28,0x20,0xa8,0xa0 },   /* ...0...1...0...1 */
		{ 0x28,0x20,0xa8,0xa0 }, { 0x28,0x20,0xa8,0xa0 },   /* ...0...1...1...0 */
		{ 0xa8,0xa0,0x88,0x80 }, { 0x28,0x20,0xa8,0xa0 },   /* ...0...1...1...1 */
		{ 0xa8,0xa0,0x88,0x80 }, { 0x28,0x20,0xa8,0xa0 },   /* ...1...0...0...0 */
		{ 0x28,0x20,0xa8,0xa0 }, { 0xa8,0xa0,0x88,0x80 },   /* ...1...0...0...1 */
		{ 0x28,0x20,0xa8,0xa0 }, { 0xa0,0x80,0xa8,0x88 },   /* ...1...0...1...0 */
		{ 0x28,0x08,0xa8,0x88 }, { 0x28,0x08,0xa8,0x88 },   /* ...1...0...1...1 */
		{ 0xa0,0x80,0xa8,0x88 }, { 0xa8,0xa0,0x88,0x80 },   /* ...1...1...0...0 */
		{ 0x28,0x20,0xa8,0xa0 }, { 0xa8,0x28,0xa0,0x20 },   /* ...1...1...0...1 */
		{ 0xa0,0x80,0xa8,0x88 }, { 0xa8,0xa0,0x88,0x80 },   /* ...1...1...1...0 */
		{ 0xa8,0xa0,0x88,0x80 }, { 0xa8,0x28,0xa0,0x20 }    /* ...1...1...1...1 */
	};

	/* this one only has the Sega decryption */
	sega_decode(memregion("maincpu")->base(), m_decrypted_opcodes, 0x8000, convtable);
}



GAME( 1985, pbaction,  0,        pbaction,  pbaction, driver_device,  0,        ROT90, "Tehkan", "Pinball Action (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, pbaction2, pbaction, pbaction,  pbaction, driver_device,  0,        ROT90, "Tehkan", "Pinball Action (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, pbaction3, pbaction, pbactionx, pbaction, pbaction_state, pbactio3, ROT90, "Tehkan", "Pinball Action (set 3, encrypted)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, pbaction4, pbaction, pbactionx, pbaction, pbaction_state, pbactio4, ROT90, "Tehkan", "Pinball Action (set 4, encrypted)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, pbaction5, pbaction, pbactionx, pbaction, pbaction_state, pbactio4, ROT90, "Tehkan", "Pinball Action (set 5, encrypted)", MACHINE_SUPPORTS_SAVE )

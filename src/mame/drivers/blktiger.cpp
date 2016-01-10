// license:???
// copyright-holders:Paul Leaman
/***************************************************************************

    Black Tiger

    Driver provided by Paul Leaman

    Thanks to Ishmair for providing information about the screen
    layout on level 3.

    Notes:
    - sprites/tile priority is a guess. I didn't find a PROM that would simply
      translate to the scheme I implemented.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "cpu/mcs51/mcs51.h"
#include "includes/blktiger.h"


/**************************************************

Protection comms between main cpu and i8751

**************************************************/

READ8_MEMBER(blktiger_state::blktiger_from_mcu_r)
{
	return m_i8751_latch;
}

WRITE8_MEMBER(blktiger_state::blktiger_to_mcu_w)
{
	m_mcu->set_input_line(MCS51_INT1_LINE, ASSERT_LINE);
	m_z80_latch = data;
}

READ8_MEMBER(blktiger_state::blktiger_from_main_r)
{
	m_mcu->set_input_line(MCS51_INT1_LINE, CLEAR_LINE);
	//printf("%02x read\n",latch);
	return m_z80_latch;
}

WRITE8_MEMBER(blktiger_state::blktiger_to_main_w)
{
	//printf("%02x write\n",data);
	m_i8751_latch = data;
}



WRITE8_MEMBER(blktiger_state::blktiger_bankswitch_w)
{
	membank("bank1")->set_entry(data & 0x0f);
}

WRITE8_MEMBER(blktiger_state::blktiger_coinlockout_w)
{
	if (ioport("COIN_LOCKOUT")->read() & 0x01)
	{
		machine().bookkeeping().coin_lockout_w(0,~data & 0x01);
		machine().bookkeeping().coin_lockout_w(1,~data & 0x02);
	}
}


static ADDRESS_MAP_START( blktiger_map, AS_PROGRAM, 8, blktiger_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xcfff) AM_READWRITE(blktiger_bgvideoram_r, blktiger_bgvideoram_w)
	AM_RANGE(0xd000, 0xd7ff) AM_RAM_WRITE(blktiger_txvideoram_w) AM_SHARE("txvideoram")
	AM_RANGE(0xd800, 0xdbff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xdc00, 0xdfff) AM_RAM_DEVWRITE("palette", palette_device, write_ext) AM_SHARE("palette_ext")
	AM_RANGE(0xe000, 0xfdff) AM_RAM
	AM_RANGE(0xfe00, 0xffff) AM_RAM AM_SHARE("spriteram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( blktiger_io_map, AS_IO, 8, blktiger_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0")    AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")    AM_WRITE(blktiger_bankswitch_w)
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN2")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSW0")   AM_WRITE(blktiger_coinlockout_w)
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSW1")   AM_WRITE(blktiger_video_control_w)
	AM_RANGE(0x05, 0x05) AM_READ_PORT("FREEZE")
	AM_RANGE(0x06, 0x06) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x07, 0x07) AM_READWRITE(blktiger_from_mcu_r,blktiger_to_mcu_w)     /* Software protection (7) */
	AM_RANGE(0x08, 0x09) AM_WRITE(blktiger_scrollx_w)
	AM_RANGE(0x0a, 0x0b) AM_WRITE(blktiger_scrolly_w)
	AM_RANGE(0x0c, 0x0c) AM_WRITE(blktiger_video_enable_w)
	AM_RANGE(0x0d, 0x0d) AM_WRITE(blktiger_bgvideoram_bank_w)
	AM_RANGE(0x0e, 0x0e) AM_WRITE(blktiger_screen_layout_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( blktigerbl_io_map, AS_IO, 8, blktiger_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0")    AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")    AM_WRITE(blktiger_bankswitch_w)
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN2")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSW0")   AM_WRITE(blktiger_coinlockout_w)
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSW1")   AM_WRITE(blktiger_video_control_w)
	AM_RANGE(0x05, 0x05) AM_READ_PORT("FREEZE")
	AM_RANGE(0x06, 0x06) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x07, 0x07) AM_NOP  /* Software protection (7) */
	AM_RANGE(0x08, 0x09) AM_WRITE(blktiger_scrollx_w)
	AM_RANGE(0x0a, 0x0b) AM_WRITE(blktiger_scrolly_w)
	AM_RANGE(0x0c, 0x0c) AM_WRITE(blktiger_video_enable_w)
	AM_RANGE(0x0d, 0x0d) AM_WRITE(blktiger_bgvideoram_bank_w)
	AM_RANGE(0x0e, 0x0e) AM_WRITE(blktiger_screen_layout_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( blktiger_sound_map, AS_PROGRAM, 8, blktiger_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xc800, 0xc800) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xe000, 0xe001) AM_DEVREADWRITE("ym1", ym2203_device, read, write)
	AM_RANGE(0xe002, 0xe003) AM_DEVREADWRITE("ym2", ym2203_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( blktiger_mcu_map, AS_PROGRAM, 8, blktiger_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( blktiger_mcu_io_map, AS_IO, 8, blktiger_state )
	AM_RANGE(MCS51_PORT_P0,MCS51_PORT_P0) AM_READWRITE(blktiger_from_main_r,blktiger_to_main_w)
	AM_RANGE(MCS51_PORT_P1,MCS51_PORT_P3) AM_WRITENOP   /* other ports unknown */
ADDRESS_MAP_END



static INPUT_PORTS_START( blktiger )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* probably unused */

	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION( "SW1:1,2,3" )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )       PORT_DIPLOCATION( "SW1:4,5,6" )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION( "SW1:7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Test ) )         PORT_DIPLOCATION( "SW1:8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION( "SW2:1,2" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7")
	PORT_DIPNAME( 0x1c, 0x0c, DEF_STR( Difficulty ) )   PORT_DIPLOCATION( "SW2:3,4,5" )
	PORT_DIPSETTING(    0x1c, "1 (Easiest)")
	PORT_DIPSETTING(    0x18, "2" )
	PORT_DIPSETTING(    0x14, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x0c, "5 (Normal)" )
	PORT_DIPSETTING(    0x08, "6" )
	PORT_DIPSETTING(    0x04, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hardest)" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION( "SW2:6" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION( "SW2:7" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION( "SW2:8" )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("FREEZE")
	PORT_DIPNAME( 0x01, 0x01, "Freeze" )    /* could be VBLANK */
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COIN_LOCKOUT")
	PORT_CONFNAME( 0x01, 0x01, "Coin Lockout Hardware Present" )
	PORT_CONFSETTING( 0x01, DEF_STR( Yes ) )
	PORT_CONFSETTING( 0x00, DEF_STR( No ) )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 4, 0 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			16*16+0, 16*16+1, 16*16+2, 16*16+3, 16*16+8+0, 16*16+8+1, 16*16+8+2, 16*16+8+3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	32*16
};

static GFXDECODE_START( blktiger )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0x300, 32 )   /* colors 0x300-0x37f */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0x000, 16 )   /* colors 0x000-0x0ff */
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout, 0x200,  8 )   /* colors 0x200-0x27f */
GFXDECODE_END



/* handler called by the 2203 emulator when the internal timers cause an IRQ */
WRITE_LINE_MEMBER(blktiger_state::irqhandler)
{
	m_audiocpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

void blktiger_state::machine_start()
{
	/* configure bankswitching */
	membank("bank1")->configure_entries(0, 16, memregion("maincpu")->base() + 0x10000, 0x4000);

	save_item(NAME(m_scroll_bank));
	save_item(NAME(m_screen_layout));
	save_item(NAME(m_chon));
	save_item(NAME(m_objon));
	save_item(NAME(m_bgon));
	save_item(NAME(m_z80_latch));
	save_item(NAME(m_i8751_latch));
	save_item(NAME(m_scroll_x));
	save_item(NAME(m_scroll_y));
}

void blktiger_state::machine_reset()
{
	/* configure bankswitching */
	membank("bank1")->configure_entries(0, 16, memregion("maincpu")->base() + 0x10000, 0x4000);

	m_scroll_x[0] = 0;
	m_scroll_x[1] = 0;
	m_scroll_y[0] = 0;
	m_scroll_y[1] = 0;
	m_scroll_bank = 0;
	m_screen_layout = 0;
	m_z80_latch = 0;
	m_i8751_latch = 0;
}

static MACHINE_CONFIG_START( blktiger, blktiger_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_24MHz/4)  /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(blktiger_map)
	MCFG_CPU_IO_MAP(blktiger_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", blktiger_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_3_579545MHz) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(blktiger_sound_map)

	MCFG_CPU_ADD("mcu", I8751, XTAL_24MHz/4) /* ??? */
	MCFG_CPU_PROGRAM_MAP(blktiger_mcu_map)
	MCFG_CPU_IO_MAP(blktiger_mcu_io_map)
	//MCFG_CPU_VBLANK_INT_DRIVER("screen", blktiger_state,  irq0_line_hold)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(blktiger_state, screen_update_blktiger)
	MCFG_SCREEN_VBLANK_DEVICE("spriteram", buffered_spriteram8_device, vblank_copy_rising)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", blktiger)

	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(xxxxBBBBRRRRGGGG)

	MCFG_BUFFERED_SPRITERAM8_ADD("spriteram")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, XTAL_3_579545MHz) /* verified on pcb */
	MCFG_YM2203_IRQ_HANDLER(WRITELINE(blktiger_state, irqhandler))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)

	MCFG_SOUND_ADD("ym2", YM2203, XTAL_3_579545MHz) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( blktigerbl, blktiger )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(blktigerbl_io_map)

	MCFG_DEVICE_REMOVE("mcu")
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( blktiger )
	ROM_REGION( 0x50000, "maincpu", 0 ) /* 64k for code + banked ROMs images */
	ROM_LOAD( "bdu-01a.5e",  0x00000, 0x08000, CRC(a8f98f22) SHA1(f77c0d0ebf3e52a21d2c0c5004350a408b8e6d24) )   /* CODE */
	ROM_LOAD( "bdu-02a.6e",  0x10000, 0x10000, CRC(7bef96e8) SHA1(6d05a73d8400dead78c561b904bf6ef8311e7b91) )   /* 0+1 */
	ROM_LOAD( "bdu-03a.8e",  0x20000, 0x10000, CRC(4089e157) SHA1(7972b1c745057802d4fd66d88b0101eb3c03e701) )   /* 2+3 */
	ROM_LOAD( "bd-04.9e",    0x30000, 0x10000, CRC(ed6af6ec) SHA1(bed303c51bcddf233ad0701306d557a60ce9f5a5) )   /* 4+5 */
	ROM_LOAD( "bd-05.10e",   0x40000, 0x10000, CRC(ae59b72e) SHA1(6e72214b71f2f337af236c8be891a18570cb6fbb) )   /* 6+7 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bd-06.1l",  0x0000, 0x8000, CRC(2cf54274) SHA1(87df100c65999ba1e9d358ffd0fe4bba23ae0efb) )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "bd.6k",  0x0000, 0x1000, CRC(ac7d14f1) SHA1(46fd6b43f10312e3e8d3c9e0c0fd616af98fdbad) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "bd-15.2n",  0x00000, 0x08000, CRC(70175d78) SHA1(2f02be2785d1824002145ea20db79821d0393929) ) /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "bd-12.5b",  0x00000, 0x10000, CRC(c4524993) SHA1(9aa6c58004ca1117e5ac44ba8fc51e9128b921b8) ) /* tiles */
	ROM_LOAD( "bd-11.4b",  0x10000, 0x10000, CRC(7932c86f) SHA1(b3b1bc1e2b0db5c2eb8772f8a2c35129cc80d511) )
	ROM_LOAD( "bd-14.9b",  0x20000, 0x10000, CRC(dc49593a) SHA1(e4ef42ba9f238fd43c8217657c92896f31d3912c) )
	ROM_LOAD( "bd-13.8b",  0x30000, 0x10000, CRC(7ed7a122) SHA1(3acc6d4c9731db0609c2e26e3bd255847149ca33) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "bd-08.5a",   0x00000, 0x10000, CRC(e2f17438) SHA1(3e5fdae07d40febedc59c7c7c4d9c6f0d72b58b5) )    /* sprites */
	ROM_LOAD( "bd-07.4a",   0x10000, 0x10000, CRC(5fccbd27) SHA1(33c55aa9c12b3121ca5c3b4c39a9b152b6946461) )
	ROM_LOAD( "bd-10.9a",   0x20000, 0x10000, CRC(fc33ccc6) SHA1(d492626a88565c2626f98ecb1d74535f1ad68e4c) )
	ROM_LOAD( "bd-09.8a",   0x30000, 0x10000, CRC(f449de01) SHA1(f6b40e9eb2471b89c42ab84f4214295d284db0c3) )

	ROM_REGION( 0x0400, "proms", 0 )    /* PROMs (function unknown) */
	ROM_LOAD( "bd01.8j",   0x0000, 0x0100, CRC(29b459e5) SHA1(0034734a533df3dea16b7b48e072485d7f26f850) )
	ROM_LOAD( "bd02.9j",   0x0100, 0x0100, CRC(8b741e66) SHA1(6c1fda59936a7217b05949f5c54b1f91f4b49dbe) )
	ROM_LOAD( "bd03.11k",  0x0200, 0x0100, CRC(27201c75) SHA1(c54d87f06bfe0b0908389c005014d97156e272c2) )
	ROM_LOAD( "bd04.11l",  0x0300, 0x0100, CRC(e5490b68) SHA1(40f9f92efe7dd97b49144aec02eb509834056915) )
ROM_END

ROM_START( blktigera )
	ROM_REGION( 0x50000, "maincpu", 0 ) /* 64k for code + banked ROMs images */
	ROM_LOAD( "bdu-01.5e",  0x00000, 0x08000, CRC(47b13922) SHA1(a722048d48171b68119b7ef1af6e06953c238ad6) )    /* CODE */
	ROM_LOAD( "bdu-02.6e",  0x10000, 0x10000, CRC(2e0daf1b) SHA1(dbcaf2bb1b2c9cd4b2ca1d52b81d6e33b5c7eee9) )    /* 0+1 */
	ROM_LOAD( "bdu-03.8e",  0x20000, 0x10000, CRC(3b67dfec) SHA1(f9d83f2bb1fbf05d80f6870d91411e9b7bbea917) )    /* 2+3 */
	ROM_LOAD( "bd-04.9e",    0x30000, 0x10000, CRC(ed6af6ec) SHA1(bed303c51bcddf233ad0701306d557a60ce9f5a5) )   /* 4+5 */
	ROM_LOAD( "bd-05.10e",   0x40000, 0x10000, CRC(ae59b72e) SHA1(6e72214b71f2f337af236c8be891a18570cb6fbb) )   /* 6+7 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bd-06.1l",  0x0000, 0x8000, CRC(2cf54274) SHA1(87df100c65999ba1e9d358ffd0fe4bba23ae0efb) )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "bd.6k",  0x0000, 0x1000, CRC(ac7d14f1) SHA1(46fd6b43f10312e3e8d3c9e0c0fd616af98fdbad) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "bd-15.2n",  0x00000, 0x08000, CRC(70175d78) SHA1(2f02be2785d1824002145ea20db79821d0393929) ) /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "bd-12.5b",  0x00000, 0x10000, CRC(c4524993) SHA1(9aa6c58004ca1117e5ac44ba8fc51e9128b921b8) ) /* tiles */
	ROM_LOAD( "bd-11.4b",  0x10000, 0x10000, CRC(7932c86f) SHA1(b3b1bc1e2b0db5c2eb8772f8a2c35129cc80d511) )
	ROM_LOAD( "bd-14.9b",  0x20000, 0x10000, CRC(dc49593a) SHA1(e4ef42ba9f238fd43c8217657c92896f31d3912c) )
	ROM_LOAD( "bd-13.8b",  0x30000, 0x10000, CRC(7ed7a122) SHA1(3acc6d4c9731db0609c2e26e3bd255847149ca33) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "bd-08.5a",   0x00000, 0x10000, CRC(e2f17438) SHA1(3e5fdae07d40febedc59c7c7c4d9c6f0d72b58b5) )    /* sprites */
	ROM_LOAD( "bd-07.4a",   0x10000, 0x10000, CRC(5fccbd27) SHA1(33c55aa9c12b3121ca5c3b4c39a9b152b6946461) )
	ROM_LOAD( "bd-10.9a",   0x20000, 0x10000, CRC(fc33ccc6) SHA1(d492626a88565c2626f98ecb1d74535f1ad68e4c) )
	ROM_LOAD( "bd-09.8a",   0x30000, 0x10000, CRC(f449de01) SHA1(f6b40e9eb2471b89c42ab84f4214295d284db0c3) )

	ROM_REGION( 0x0400, "proms", 0 )    /* PROMs (function unknown) */
	ROM_LOAD( "bd01.8j",   0x0000, 0x0100, CRC(29b459e5) SHA1(0034734a533df3dea16b7b48e072485d7f26f850) )
	ROM_LOAD( "bd02.9j",   0x0100, 0x0100, CRC(8b741e66) SHA1(6c1fda59936a7217b05949f5c54b1f91f4b49dbe) )
	ROM_LOAD( "bd03.11k",  0x0200, 0x0100, CRC(27201c75) SHA1(c54d87f06bfe0b0908389c005014d97156e272c2) )
	ROM_LOAD( "bd04.11l",  0x0300, 0x0100, CRC(e5490b68) SHA1(40f9f92efe7dd97b49144aec02eb509834056915) )
ROM_END

ROM_START( blktigerb1 )
	ROM_REGION( 0x50000, "maincpu", 0 ) /* 64k for code + banked ROMs images */
	ROM_LOAD( "btiger1.f6",   0x00000, 0x08000, CRC(9d8464e8) SHA1(c847ee9a22b8b636e85427214747e6bd779023e8) )  /* CODE */
	ROM_LOAD( "bdu-02a.6e",   0x10000, 0x10000, CRC(7bef96e8) SHA1(6d05a73d8400dead78c561b904bf6ef8311e7b91) )  /* 0+1 */
	ROM_LOAD( "btiger3.j6",   0x20000, 0x10000, CRC(52c56ed1) SHA1(b6ea61869dcfcedb8cfc14c613440e3f4649866f) )  /* 2+3 */
	ROM_LOAD( "bd-04.9e",     0x30000, 0x10000, CRC(ed6af6ec) SHA1(bed303c51bcddf233ad0701306d557a60ce9f5a5) )  /* 4+5 */
	ROM_LOAD( "bd-05.10e",    0x40000, 0x10000, CRC(ae59b72e) SHA1(6e72214b71f2f337af236c8be891a18570cb6fbb) )  /* 6+7 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bd-06.1l",  0x0000, 0x8000, CRC(2cf54274) SHA1(87df100c65999ba1e9d358ffd0fe4bba23ae0efb) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "bd-15.2n",  0x00000, 0x08000, CRC(70175d78) SHA1(2f02be2785d1824002145ea20db79821d0393929) ) /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "bd-12.5b",  0x00000, 0x10000, CRC(c4524993) SHA1(9aa6c58004ca1117e5ac44ba8fc51e9128b921b8) ) /* tiles */
	ROM_LOAD( "bd-11.4b",  0x10000, 0x10000, CRC(7932c86f) SHA1(b3b1bc1e2b0db5c2eb8772f8a2c35129cc80d511) )
	ROM_LOAD( "bd-14.9b",  0x20000, 0x10000, CRC(dc49593a) SHA1(e4ef42ba9f238fd43c8217657c92896f31d3912c) )
	ROM_LOAD( "bd-13.8b",  0x30000, 0x10000, CRC(7ed7a122) SHA1(3acc6d4c9731db0609c2e26e3bd255847149ca33) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "bd-08.5a",  0x00000, 0x10000, CRC(e2f17438) SHA1(3e5fdae07d40febedc59c7c7c4d9c6f0d72b58b5) ) /* sprites */
	ROM_LOAD( "bd-07.4a",  0x10000, 0x10000, CRC(5fccbd27) SHA1(33c55aa9c12b3121ca5c3b4c39a9b152b6946461) )
	ROM_LOAD( "bd-10.9a",  0x20000, 0x10000, CRC(fc33ccc6) SHA1(d492626a88565c2626f98ecb1d74535f1ad68e4c) )
	ROM_LOAD( "bd-09.8a",  0x30000, 0x10000, CRC(f449de01) SHA1(f6b40e9eb2471b89c42ab84f4214295d284db0c3) )

	ROM_REGION( 0x0400, "proms", 0 )    /* PROMs (function unknown) */
	ROM_LOAD( "bd01.8j",   0x0000, 0x0100, CRC(29b459e5) SHA1(0034734a533df3dea16b7b48e072485d7f26f850) )
	ROM_LOAD( "bd02.9j",   0x0100, 0x0100, CRC(8b741e66) SHA1(6c1fda59936a7217b05949f5c54b1f91f4b49dbe) )
	ROM_LOAD( "bd03.11k",  0x0200, 0x0100, CRC(27201c75) SHA1(c54d87f06bfe0b0908389c005014d97156e272c2) )
	ROM_LOAD( "bd04.11l",  0x0300, 0x0100, CRC(e5490b68) SHA1(40f9f92efe7dd97b49144aec02eb509834056915) )
ROM_END

ROM_START( blktigerb2 )
	ROM_REGION( 0x50000, "maincpu", 0 ) /* 64k for code + banked ROMs images */
	ROM_LOAD( "1.bin",        0x00000, 0x08000, CRC(47e2b21e) SHA1(3f03543ace435239978a95f569ac89f6762253c0) )  /* CODE */
	ROM_LOAD( "bdu-02a.6e",   0x10000, 0x10000, CRC(7bef96e8) SHA1(6d05a73d8400dead78c561b904bf6ef8311e7b91) )  /* 0+1 */
	ROM_LOAD( "3.bin",        0x20000, 0x10000, CRC(52c56ed1) SHA1(b6ea61869dcfcedb8cfc14c613440e3f4649866f) )  /* 2+3 : same crc of btiger3.j6 from bktigerb */
	ROM_LOAD( "bd-04.9e",     0x30000, 0x10000, CRC(ed6af6ec) SHA1(bed303c51bcddf233ad0701306d557a60ce9f5a5) )  /* 4+5 */
	ROM_LOAD( "bd-05.10e",    0x40000, 0x10000, CRC(ae59b72e) SHA1(6e72214b71f2f337af236c8be891a18570cb6fbb) )  /* 6+7 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bd-06.1l",  0x0000, 0x8000, CRC(2cf54274) SHA1(87df100c65999ba1e9d358ffd0fe4bba23ae0efb) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "bd-15.2n",  0x00000, 0x08000, CRC(70175d78) SHA1(2f02be2785d1824002145ea20db79821d0393929) ) /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "bd-12.5b",  0x00000, 0x10000, CRC(c4524993) SHA1(9aa6c58004ca1117e5ac44ba8fc51e9128b921b8) ) /* tiles */
	ROM_LOAD( "bd-11.4b",  0x10000, 0x10000, CRC(7932c86f) SHA1(b3b1bc1e2b0db5c2eb8772f8a2c35129cc80d511) )
	ROM_LOAD( "bd-14.9b",  0x20000, 0x10000, CRC(dc49593a) SHA1(e4ef42ba9f238fd43c8217657c92896f31d3912c) )
	ROM_LOAD( "bd-13.8b",  0x30000, 0x10000, CRC(7ed7a122) SHA1(3acc6d4c9731db0609c2e26e3bd255847149ca33) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "bd-08.5a",  0x00000, 0x10000, CRC(e2f17438) SHA1(3e5fdae07d40febedc59c7c7c4d9c6f0d72b58b5) ) /* sprites */
	ROM_LOAD( "bd-07.4a",  0x10000, 0x10000, CRC(5fccbd27) SHA1(33c55aa9c12b3121ca5c3b4c39a9b152b6946461) )
	ROM_LOAD( "bd-10.9a",  0x20000, 0x10000, CRC(fc33ccc6) SHA1(d492626a88565c2626f98ecb1d74535f1ad68e4c) )
	ROM_LOAD( "bd-09.8a",  0x30000, 0x10000, CRC(f449de01) SHA1(f6b40e9eb2471b89c42ab84f4214295d284db0c3) )

	ROM_REGION( 0x0400, "proms", 0 )    /* PROMs (function unknown) */
	ROM_LOAD( "bd01.8j",   0x0000, 0x0100, CRC(29b459e5) SHA1(0034734a533df3dea16b7b48e072485d7f26f850) )
	ROM_LOAD( "bd02.9j",   0x0100, 0x0100, CRC(8b741e66) SHA1(6c1fda59936a7217b05949f5c54b1f91f4b49dbe) )
	ROM_LOAD( "bd03.11k",  0x0200, 0x0100, CRC(27201c75) SHA1(c54d87f06bfe0b0908389c005014d97156e272c2) )
	ROM_LOAD( "bd04.11l",  0x0300, 0x0100, CRC(e5490b68) SHA1(40f9f92efe7dd97b49144aec02eb509834056915) )
ROM_END

ROM_START( blkdrgon )
	ROM_REGION( 0x50000, "maincpu", 0 ) /* 64k for code + banked ROMs images */
	ROM_LOAD( "bd_01.5e",  0x00000, 0x08000, CRC(27ccdfbc) SHA1(3caafe00735ba9b24d870ee61ad2cae541551024) ) /* CODE */
	ROM_LOAD( "bd_02.6e",  0x10000, 0x10000, CRC(7d39c26f) SHA1(562a3f578e109ae020f65e341c876ad7e510a311) ) /* 0+1 */
	ROM_LOAD( "bd_03.8e",  0x20000, 0x10000, CRC(d1bf3757) SHA1(b19f8b986406bde65ac7f0d55d54f87b37f5e42f) ) /* 2+3 */
	ROM_LOAD( "bd_04.9e",  0x30000, 0x10000, CRC(4d1d6680) SHA1(e137624c59392de6aaffeded99b024938360bd25) ) /* 4+5 */
	ROM_LOAD( "bd_05.10e", 0x40000, 0x10000, CRC(c8d0c45e) SHA1(66c2e5a74c5875a2c8e28740fe944bd943246ce5) ) /* 6+7 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bd_06.1l",  0x0000, 0x8000, CRC(2cf54274) SHA1(87df100c65999ba1e9d358ffd0fe4bba23ae0efb) )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "bd.6k",  0x0000, 0x1000, CRC(ac7d14f1) SHA1(46fd6b43f10312e3e8d3c9e0c0fd616af98fdbad) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "bd_15.2n",  0x00000, 0x08000, CRC(3821ab29) SHA1(576f1839f63b0cad6b851d6e6a3e9dec21ac811d) ) /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "bd_12.5b",  0x00000, 0x10000, CRC(22d0a4b0) SHA1(f9402ea9ffedcb280497a63c5eb352de9d4ca3fd) ) /* tiles */
	ROM_LOAD( "bd_11.4b",  0x10000, 0x10000, CRC(c8b5fc52) SHA1(621e899285ce6302e5b25d133d9cd52c09b7b202) )
	ROM_LOAD( "bd_14.9b",  0x20000, 0x10000, CRC(9498c378) SHA1(841934ddef724faf04162c4be4aea1684d8d8e0f) )
	ROM_LOAD( "bd_13.8b",  0x30000, 0x10000, CRC(5b0df8ce) SHA1(57d10b48bd61b0224ce21b36bde8d2479e8e5df4) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "bd_08.5a",  0x00000, 0x10000, CRC(e2f17438) SHA1(3e5fdae07d40febedc59c7c7c4d9c6f0d72b58b5) ) /* sprites */
	ROM_LOAD( "bd_07.4a",  0x10000, 0x10000, CRC(5fccbd27) SHA1(33c55aa9c12b3121ca5c3b4c39a9b152b6946461) )
	ROM_LOAD( "bd_10.9a",  0x20000, 0x10000, CRC(fc33ccc6) SHA1(d492626a88565c2626f98ecb1d74535f1ad68e4c) )
	ROM_LOAD( "bd_09.8a",  0x30000, 0x10000, CRC(f449de01) SHA1(f6b40e9eb2471b89c42ab84f4214295d284db0c3) )

	ROM_REGION( 0x0400, "proms", 0 )    /* PROMs (function unknown) */
	ROM_LOAD( "bd01.8j",   0x0000, 0x0100, CRC(29b459e5) SHA1(0034734a533df3dea16b7b48e072485d7f26f850) )
	ROM_LOAD( "bd02.9j",   0x0100, 0x0100, CRC(8b741e66) SHA1(6c1fda59936a7217b05949f5c54b1f91f4b49dbe) )
	ROM_LOAD( "bd03.11k",  0x0200, 0x0100, CRC(27201c75) SHA1(c54d87f06bfe0b0908389c005014d97156e272c2) )
	ROM_LOAD( "bd04.11l",  0x0300, 0x0100, CRC(e5490b68) SHA1(40f9f92efe7dd97b49144aec02eb509834056915) )
ROM_END


ROM_START( blkdrgonb )
	ROM_REGION( 0x50000, "maincpu", 0 ) /* 64k for code + banked ROMs images */
	ROM_LOAD( "a1",           0x00000, 0x08000, CRC(7caf2ba0) SHA1(57b17caff67d36b24075f5865d433bfc8bcc9bc2) )  /* CODE */
	ROM_LOAD( "blkdrgon.6e",  0x10000, 0x10000, CRC(7d39c26f) SHA1(562a3f578e109ae020f65e341c876ad7e510a311) )  /* 0+1 */
	ROM_LOAD( "a3",           0x20000, 0x10000, CRC(f4cd0f39) SHA1(9efc5161c861c7ec8ae72509e71c6d7b71b22fc6) )  /* 2+3 */
	ROM_LOAD( "blkdrgon.9e",  0x30000, 0x10000, CRC(4d1d6680) SHA1(e137624c59392de6aaffeded99b024938360bd25) )  /* 4+5 */
	ROM_LOAD( "blkdrgon.10e", 0x40000, 0x10000, CRC(c8d0c45e) SHA1(66c2e5a74c5875a2c8e28740fe944bd943246ce5) )  /* 6+7 */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "bd-06.1l",  0x0000, 0x8000, CRC(2cf54274) SHA1(87df100c65999ba1e9d358ffd0fe4bba23ae0efb) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "b5",           0x00000, 0x08000, CRC(852ad2b7) SHA1(9f30c0d7e1127589b03d8f45ea50e0f907181a4b) )  /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "blkdrgon.5b",  0x00000, 0x10000, CRC(22d0a4b0) SHA1(f9402ea9ffedcb280497a63c5eb352de9d4ca3fd) )  /* tiles */
	ROM_LOAD( "b1",           0x10000, 0x10000, CRC(053ab15c) SHA1(f0ddc71009ab5dd69ae463c3636ec2332c0556f8) )
	ROM_LOAD( "blkdrgon.9b",  0x20000, 0x10000, CRC(9498c378) SHA1(841934ddef724faf04162c4be4aea1684d8d8e0f) )
	ROM_LOAD( "b3",           0x30000, 0x10000, CRC(9dc6e943) SHA1(0818c1fb2cc8ff403a479457b268bba6ec0730bc) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "bd-08.5a",  0x00000, 0x10000, CRC(e2f17438) SHA1(3e5fdae07d40febedc59c7c7c4d9c6f0d72b58b5) ) /* sprites */
	ROM_LOAD( "bd-07.4a",  0x10000, 0x10000, CRC(5fccbd27) SHA1(33c55aa9c12b3121ca5c3b4c39a9b152b6946461) )
	ROM_LOAD( "bd-10.9a",  0x20000, 0x10000, CRC(fc33ccc6) SHA1(d492626a88565c2626f98ecb1d74535f1ad68e4c) )
	ROM_LOAD( "bd-09.8a",  0x30000, 0x10000, CRC(f449de01) SHA1(f6b40e9eb2471b89c42ab84f4214295d284db0c3) )

	ROM_REGION( 0x0400, "proms", 0 )    /* PROMs (function unknown) */
	ROM_LOAD( "bd01.8j",   0x0000, 0x0100, CRC(29b459e5) SHA1(0034734a533df3dea16b7b48e072485d7f26f850) )
	ROM_LOAD( "bd02.9j",   0x0100, 0x0100, CRC(8b741e66) SHA1(6c1fda59936a7217b05949f5c54b1f91f4b49dbe) )
	ROM_LOAD( "bd03.11k",  0x0200, 0x0100, CRC(27201c75) SHA1(c54d87f06bfe0b0908389c005014d97156e272c2) )
	ROM_LOAD( "bd04.11l",  0x0300, 0x0100, CRC(e5490b68) SHA1(40f9f92efe7dd97b49144aec02eb509834056915) )
ROM_END


ROM_START( blktigerb3 )
	ROM_REGION( 0x50000, "maincpu", 0 ) /* 64k for code + banked ROMs images */ // == same as blktigerb2 maincpu
	ROM_LOAD( "1.5e",  0x00000, 0x08000, CRC(47e2b21e) SHA1(3f03543ace435239978a95f569ac89f6762253c0) )   /* CODE */
	ROM_LOAD( "2.6e",  0x10000, 0x10000, CRC(7bef96e8) SHA1(6d05a73d8400dead78c561b904bf6ef8311e7b91) )   /* 0+1 */
	ROM_LOAD( "3.8e",  0x20000, 0x10000, CRC(52c56ed1) SHA1(b6ea61869dcfcedb8cfc14c613440e3f4649866f) )   /* 2+3 */
	ROM_LOAD( "4.9e",    0x30000, 0x10000, CRC(ed6af6ec) SHA1(bed303c51bcddf233ad0701306d557a60ce9f5a5) )   /* 4+5 */
	ROM_LOAD( "5.10e",   0x40000, 0x10000, CRC(ae59b72e) SHA1(6e72214b71f2f337af236c8be891a18570cb6fbb) )   /* 6+7 */

	ROM_REGION( 0x10000, "audiocpu", 0 ) // == same as other sets but with an address swap
	ROM_LOAD( "6.1l",  0x0000, 0x8000, CRC(6dfab115) SHA1(05f10bdfa4dff50ccc7707a7dbd8eab1680e09b9) )

	ROM_REGION( 0x08000, "gfx1", 0 ) // == same as blkdrgon
	ROM_LOAD( "15.2n",  0x00000, 0x08000, CRC(3821ab29) SHA1(576f1839f63b0cad6b851d6e6a3e9dec21ac811d) ) /* characters */

	ROM_REGION( 0x40000, "gfx2", 0 ) // == same as other sets
	ROM_LOAD( "12.5b",  0x00000, 0x10000, CRC(c4524993) SHA1(9aa6c58004ca1117e5ac44ba8fc51e9128b921b8) ) /* tiles */
	ROM_LOAD( "11.4b",  0x10000, 0x10000, CRC(7932c86f) SHA1(b3b1bc1e2b0db5c2eb8772f8a2c35129cc80d511) )
	ROM_LOAD( "14.9b",  0x20000, 0x10000, CRC(dc49593a) SHA1(e4ef42ba9f238fd43c8217657c92896f31d3912c) )
	ROM_LOAD( "13.8b",  0x30000, 0x10000, CRC(7ed7a122) SHA1(3acc6d4c9731db0609c2e26e3bd255847149ca33) )

	ROM_REGION( 0x40000, "gfx3", 0 ) // == same as other sets
	ROM_LOAD( "8.5a",   0x00000, 0x10000, CRC(e2f17438) SHA1(3e5fdae07d40febedc59c7c7c4d9c6f0d72b58b5) )    /* sprites */
	ROM_LOAD( "7.4a",   0x10000, 0x10000, CRC(5fccbd27) SHA1(33c55aa9c12b3121ca5c3b4c39a9b152b6946461) )
	ROM_LOAD( "10.9a",  0x20000, 0x10000, CRC(fc33ccc6) SHA1(d492626a88565c2626f98ecb1d74535f1ad68e4c) )
	ROM_LOAD( "9.8a",   0x30000, 0x10000, CRC(f449de01) SHA1(f6b40e9eb2471b89c42ab84f4214295d284db0c3) )

	ROM_REGION( 0x0400, "proms", 0 )    /* PROMs (function unknown) */ // misisng in this dump
	ROM_LOAD( "bd01.8j",   0x0000, 0x0100, CRC(29b459e5) SHA1(0034734a533df3dea16b7b48e072485d7f26f850) )
	ROM_LOAD( "bd02.9j",   0x0100, 0x0100, CRC(8b741e66) SHA1(6c1fda59936a7217b05949f5c54b1f91f4b49dbe) )
	ROM_LOAD( "bd03.11k",  0x0200, 0x0100, CRC(27201c75) SHA1(c54d87f06bfe0b0908389c005014d97156e272c2) )
	ROM_LOAD( "bd04.11l",  0x0300, 0x0100, CRC(e5490b68) SHA1(40f9f92efe7dd97b49144aec02eb509834056915) )
ROM_END

DRIVER_INIT_MEMBER(blktiger_state,blktigerb3)
{
	UINT8 *src = memregion("audiocpu")->base();
	int len = 0x8000;
	dynamic_buffer buffer(len);

	for (int i = 0; i < len; i++)
	{
		int addr;

		addr = BITSWAP16(i, 15,14,13,12,11,10,9,8, 3,4,5,6, 7,2,1,0);
		buffer[i] = src[addr];

	}

	memcpy(src, &buffer[0], len);
}
GAME( 1987, blktiger,   0,        blktiger,   blktiger, driver_device, 0, ROT0, "Capcom",  "Black Tiger",                 MACHINE_SUPPORTS_SAVE )
GAME( 1987, blktigera,  blktiger, blktiger,   blktiger, driver_device, 0, ROT0, "Capcom",  "Black Tiger (older)",         MACHINE_SUPPORTS_SAVE )
GAME( 1987, blktigerb1, blktiger, blktigerbl, blktiger, driver_device, 0, ROT0, "bootleg", "Black Tiger (bootleg set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, blktigerb2, blktiger, blktigerbl, blktiger, driver_device, 0, ROT0, "bootleg", "Black Tiger (bootleg set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, blkdrgon,   blktiger, blktiger,   blktiger, driver_device, 0, ROT0, "Capcom",  "Black Dragon (Japan)",        MACHINE_SUPPORTS_SAVE )
GAME( 1987, blkdrgonb,  blktiger, blktigerbl, blktiger, driver_device, 0, ROT0, "bootleg", "Black Dragon (bootleg)",      MACHINE_SUPPORTS_SAVE )
// this board has Capcom markings (boards 87118-A-X1 / 87118-B-X1, but no MCU, a mix of bootleg Black Tiger and Black Dragon roms, and an address swapped sound rom? is the latter an alternative security measure?
GAME( 1987, blktigerb3, blktiger, blktigerbl, blktiger, blktiger_state, blktigerb3, ROT0, "bootleg", "Black Tiger / Black Dragon (mixed bootleg?)", MACHINE_SUPPORTS_SAVE )

// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Atari Runaway hardware

    Games supported:
        * Qwak (prototype)
        * Runaway (prototype)

    original Qwak driver written by Mike Balfour

****************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/atari_vg.h"
#include "sound/pokey.h"
#include "includes/runaway.h"


TIMER_CALLBACK_MEMBER(runaway_state::interrupt_callback)
{
	/* assume Centipede-style interrupt timing */
	int scanline = param;

	m_maincpu->set_input_line(0, (scanline & 32) ? ASSERT_LINE : CLEAR_LINE);

	scanline += 32;

	if (scanline >= 263)
		scanline = 16;

	m_interrupt_timer->adjust(m_screen->time_until_pos(scanline), scanline);
}

void runaway_state::machine_start()
{
	m_interrupt_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(runaway_state::interrupt_callback),this));
}

void runaway_state::machine_reset()
{
	m_interrupt_timer->adjust(m_screen->time_until_pos(16), 16);
}


READ8_MEMBER(runaway_state::runaway_input_r)
{
	UINT8 val = 0;

	if (ioport("3000D7")->read() & (1 << offset))
	{
		val |= 0x80;
	}
	if (ioport("3000D6")->read() & (1 << offset))
	{
		val |= 0x40;
	}

	return val;
}


READ8_MEMBER(runaway_state::runaway_pot_r)
{
	return (ioport("7000")->read() << (7 - offset)) & 0x80;
}


WRITE8_MEMBER(runaway_state::runaway_led_w)
{
	output().set_led_value(offset, ~data & 1);
}


WRITE8_MEMBER(runaway_state::runaway_irq_ack_w)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}


static ADDRESS_MAP_START( runaway_map, AS_PROGRAM, 8, runaway_state )
	AM_RANGE(0x0000, 0x03ff) AM_RAM
	AM_RANGE(0x0400, 0x07bf) AM_RAM_WRITE(runaway_video_ram_w) AM_SHARE("video_ram")
	AM_RANGE(0x07c0, 0x07ff) AM_RAM AM_SHARE("sprite_ram")
	AM_RANGE(0x1000, 0x1000) AM_WRITE(runaway_irq_ack_w)
	AM_RANGE(0x1400, 0x143f) AM_DEVWRITE("earom", atari_vg_earom_device, write)
	AM_RANGE(0x1800, 0x1800) AM_DEVWRITE("earom", atari_vg_earom_device, ctrl_w)
	AM_RANGE(0x1c00, 0x1c0f) AM_WRITE(runaway_paletteram_w)
	AM_RANGE(0x2000, 0x2000) AM_WRITENOP /* coin counter? */
	AM_RANGE(0x2001, 0x2001) AM_WRITENOP /* coin counter? */
	AM_RANGE(0x2003, 0x2004) AM_WRITE(runaway_led_w)
	AM_RANGE(0x2005, 0x2005) AM_WRITE(runaway_tile_bank_w)

	AM_RANGE(0x3000, 0x3007) AM_READ(runaway_input_r)
	AM_RANGE(0x4000, 0x4000) AM_READ_PORT("4000")
	AM_RANGE(0x5000, 0x5000) AM_DEVREAD("earom", atari_vg_earom_device, read)
	AM_RANGE(0x6000, 0x600f) AM_DEVREADWRITE("pokey1", pokey_device, read, write)
	AM_RANGE(0x7000, 0x700f) AM_DEVREADWRITE("pokey2", pokey_device, read, write)
	AM_RANGE(0x8000, 0xcfff) AM_ROM
	AM_RANGE(0xf000, 0xffff) AM_ROM /* for the interrupt vectors */
ADDRESS_MAP_END


static INPUT_PORTS_START( qwak )
	PORT_START("3000D7")    /* 3000 D7 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START("3000D6")    /* 3000 D6 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("4000")  /* 4000 */
	PORT_DIPNAME( 0x01, 0x00, "DIP 1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x02, 0x00, "DIP 2" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x04, 0x00, "DIP 3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x08, 0x00, "DIP 4" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x10, 0x00, "DIP 5" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, "DIP 6" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, "DIP 7" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "DIP 8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))

	PORT_START("6008") /* 6008 not used */
	PORT_START("7000") /* 7000 not used */
INPUT_PORTS_END


static INPUT_PORTS_START( runaway )
	PORT_START("3000D7") /* 3000 D7 */
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT ( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START("3000D6") /* 3000 D6 */
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_START1 )    /* also level skip if invincibility is on */
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("4000") /* 4000 */
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ))
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ))
	PORT_DIPNAME( 0x0c, 0x04, "Coin 3 Multiplier" )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x10, 0x10, "Coin 2 Multiplier" )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))

	PORT_START("6008") /* 6008 */
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ))
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Bonus_Life ))
	PORT_DIPSETTING(    0x00, "Never" )
	PORT_DIPSETTING(    0x04, "Every 5000" )
	PORT_DIPSETTING(    0x08, "Every 10000" )
	PORT_DIPSETTING(    0x0c, "Every 15000" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Difficulty ))
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x00, "Invincibility" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ))

	PORT_START("7000") /* 7000 */
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_TOGGLE
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x60, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x20, DEF_STR( German ) )
	PORT_DIPSETTING(    0x40, DEF_STR( French ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Spanish ) )
	PORT_BIT ( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static const gfx_layout runaway_tile_layout =
{
	8, 8,
	256,
	3,
	{
		RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3)
	},
	{
		0, 1, 2, 3, 4, 5, 6, 7
	},
	{
		0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38
	},
	0x40
};


static const gfx_layout qwak_tile_layout =
{
	8, 8,
	256,
	4,
	{
		RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4)
	},
	{
		0, 1, 2, 3, 4, 5, 6, 7
	},
	{
		0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38
	},
	0x40
};


static const gfx_layout runaway_sprite_layout =
{
	8, 16,
	384,
	3,
	{
		RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3)
	},
	{
		0, 1, 2, 3, 4, 5, 6, 7
	},
	{
		0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38,
		0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x78
	},
	0x80
};


static const gfx_layout qwak_sprite_layout =
{
	8, 16,
	128,
	4,
	{
		RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4)
	},
	{
		0, 1, 2, 3, 4, 5, 6, 7
	},
	{
		0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38,
		0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x78
	},
	0x80
};


static GFXDECODE_START( runaway )
	GFXDECODE_ENTRY( "gfx1", 0x000, runaway_tile_layout,   0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x800, runaway_sprite_layout, 8, 1 )
GFXDECODE_END


static GFXDECODE_START( qwak )
	GFXDECODE_ENTRY( "gfx1", 0x800, qwak_tile_layout,   0, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x000, qwak_sprite_layout, 0, 1 )
GFXDECODE_END


static MACHINE_CONFIG_START( runaway, runaway_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 12096000 / 8) /* ? */
	MCFG_CPU_PROGRAM_MAP(runaway_map)


	MCFG_ATARIVGEAROM_ADD("earom")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(256, 263)
	MCFG_SCREEN_VISIBLE_AREA(0, 255, 0, 239)
	MCFG_SCREEN_UPDATE_DRIVER(runaway_state, screen_update_runaway)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", runaway)
	MCFG_PALETTE_ADD("palette", 16)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("pokey1", POKEY, 12096000 / 8)
	MCFG_POKEY_ALLPOT_R_CB(IOPORT("6008"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("pokey2", POKEY, 12096000 / 8)
	MCFG_POKEY_POT0_R_CB(READ8(runaway_state, runaway_pot_r))
	MCFG_POKEY_POT1_R_CB(READ8(runaway_state, runaway_pot_r))
	MCFG_POKEY_POT2_R_CB(READ8(runaway_state, runaway_pot_r))
	MCFG_POKEY_POT3_R_CB(READ8(runaway_state, runaway_pot_r))
	MCFG_POKEY_POT4_R_CB(READ8(runaway_state, runaway_pot_r))
	MCFG_POKEY_POT5_R_CB(READ8(runaway_state, runaway_pot_r))
	MCFG_POKEY_POT6_R_CB(READ8(runaway_state, runaway_pot_r))
	MCFG_POKEY_POT7_R_CB(READ8(runaway_state, runaway_pot_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( qwak, runaway )

	/* basic machine hardware */

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", qwak)

	MCFG_VIDEO_START_OVERRIDE(runaway_state,qwak)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(runaway_state, screen_update_qwak)

MACHINE_CONFIG_END


ROM_START( runaway )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "choo8000.d1", 0x8000, 0x1000, CRC(35794abe) SHA1(5ce872bda8bb2ed4888ba8b47ecd1afbe24b22eb) )
	ROM_LOAD( "choo9000.d1", 0x9000, 0x1000, CRC(0d63756d) SHA1(2549a57ca106635f5c53ea1b03f5a0d6e901ab47) )
	ROM_LOAD( "chooa000.e1", 0xa000, 0x1000, CRC(e6806b6b) SHA1(c260eaa35cbc46f0c0fd4006ec6d04315c3bb851) )
	ROM_LOAD( "choob000.f1", 0xb000, 0x1000, CRC(6aa52bc4) SHA1(5992d441ae8607859abd111c946783036ef6253f) )
	ROM_LOAD( "chooc000",    0xc000, 0x1000, CRC(452ddea2) SHA1(1072de8935aae23eb1ef7b16e308180cd3e91da2) )
	ROM_RELOAD(              0xf000, 0x1000 )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "chop0", 0x0000, 0x2000, CRC(225a8c5e) SHA1(13394320640355d67414e085ad28364814147b63) )
	ROM_LOAD( "chop1", 0x2000, 0x2000, CRC(70389c0f) SHA1(6baf4a17c11e9b27a1e09cce301f931f5099978d) )
	ROM_LOAD( "chop2", 0x4000, 0x2000, CRC(63655f1c) SHA1(c235be3945067c873c03ce8a0c5cfb76984f66ff) )
ROM_END


ROM_START( qwak )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "qwak8000.bin", 0x8000, 0x1000, CRC(4d002d8a) SHA1(8621e7ec1ab3cb8d003858227e858354cd79dbf1) )
	ROM_LOAD( "qwak9000.bin", 0x9000, 0x1000, CRC(e0c78fd7) SHA1(f5f397950971d12a7ae47fc64aa8f5751463b8a5) )
	ROM_LOAD( "qwaka000.bin", 0xa000, 0x1000, CRC(e5770fc9) SHA1(c9556e9c2f7b6c37755ac9f10d95027118317b4a) )
	ROM_LOAD( "qwakb000.bin", 0xb000, 0x1000, CRC(90771cc0) SHA1(5715e5bfccb05c51d871b443e42b0950ec23e330) )
	ROM_RELOAD(               0xf000, 0x1000 )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "qwakgfx0.bin", 0x0000, 0x1000, CRC(bed2c067) SHA1(53d909b414042d54fe2e86ae0d6c7a4ded16b87e) )
	ROM_LOAD( "qwakgfx1.bin", 0x1000, 0x1000, CRC(73a31d28) SHA1(bbe076432866398bcd02962dd90eb178e3a38fb1) )
	ROM_LOAD( "qwakgfx2.bin", 0x2000, 0x1000, CRC(07fd9e80) SHA1(83d5f22b8316ac7e88d8ecdb238182a35a6f6362) )
	ROM_LOAD( "qwakgfx3.bin", 0x3000, 0x1000, CRC(e8416f2b) SHA1(171f6539575f2c06b431ab5118e5cbaf740f557d) )
ROM_END


GAME( 1982, qwak,    0, qwak,    qwak, driver_device,    0, ROT270, "Atari", "Qwak (prototype)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, runaway, 0, runaway, runaway, driver_device, 0, ROT0,   "Atari", "Runaway (prototype)", MACHINE_SUPPORTS_SAVE )

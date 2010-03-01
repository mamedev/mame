/***************************************************************************

Othello Derby

driver by Nicola Salmoria

Video IC is S951060-VGP

Notes:
- Sprite/tile priorities are NOT orthogonal to sprite/sprite priorities:
  sprites with a higher priority appear over sprites with a lower priority,
  regardless of their order in the sprite list. Therefore, the current
  implementation is correct.

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "includes/othldrby.h"


static READ16_HANDLER( pip )
{
	othldrby_state *state = (othldrby_state *)space->machine->driver_data;
	return state->toggle ^= 1;
}

static READ16_HANDLER( pap )
{
	return mame_rand(space->machine);
}


static WRITE16_DEVICE_HANDLER( oki_bankswitch_w )
{
	if (ACCESSING_BITS_0_7)
		okim6295_set_bank_base(device, (data & 1) * 0x40000);
}

static WRITE16_HANDLER( coinctrl_w )
{
	if (ACCESSING_BITS_0_7)
	{
		coin_counter_w(space->machine, 0, data & 1);
		coin_counter_w(space->machine, 1, data & 2);
		coin_lockout_w(space->machine, 0, ~data & 4);
		coin_lockout_w(space->machine, 1, ~data & 8);
	}
}

static WRITE16_HANDLER( calendar_w )
{
}

static READ16_HANDLER( calendar_r )
{
	mame_system_time systime;

	mame_get_base_datetime(space->machine, &systime);

	switch (offset)
	{
		case 0:
			return ((systime.local_time.second/10)<<4) + (systime.local_time.second%10);
		case 1:
			return ((systime.local_time.minute/10)<<4) + (systime.local_time.minute%10);
		case 2:
			return ((systime.local_time.hour/10)<<4) + (systime.local_time.hour%10);
		case 3:
			return systime.local_time.weekday;
		case 4:
			return ((systime.local_time.mday/10)<<4) + (systime.local_time.mday%10);
		case 5:
			return (systime.local_time.month + 1);
		case 6:
			return (((systime.local_time.year%100)/10)<<4) + (systime.local_time.year%10);
		case 7:
		default:
			return 0;	/* status? the other registers are read only when bit 0 is clear */
	}
}


static ADDRESS_MAP_START( othldrby_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM
	AM_RANGE(0x200000, 0x20000f) AM_READWRITE(calendar_r, calendar_w)
	AM_RANGE(0x300000, 0x300001) AM_WRITE(othldrby_videoram_addr_w)
	AM_RANGE(0x300004, 0x300007) AM_READWRITE(othldrby_videoram_r, othldrby_videoram_w)
	AM_RANGE(0x300008, 0x300009) AM_WRITE(othldrby_vreg_addr_w)
	AM_RANGE(0x30000c, 0x30000d) AM_READ(pip)	// vblank?
	AM_RANGE(0x30000c, 0x30000f) AM_WRITE(othldrby_vreg_w)
	AM_RANGE(0x400000, 0x400fff) AM_RAM_WRITE(paletteram16_xBBBBBGGGGGRRRRR_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x600000, 0x600001) AM_DEVREADWRITE8("oki", okim6295_r, okim6295_w, 0x00ff)
	AM_RANGE(0x700000, 0x700001) AM_READ(pap)	// scanline???
	AM_RANGE(0x700004, 0x700005) AM_READ_PORT("DSW1")
	AM_RANGE(0x700008, 0x700009) AM_READ_PORT("DSW2")
	AM_RANGE(0x70000c, 0x70000d) AM_READ_PORT("P1")
	AM_RANGE(0x700010, 0x700011) AM_READ_PORT("P2")
	AM_RANGE(0x70001c, 0x70001d) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x700030, 0x700031) AM_DEVWRITE("oki", oki_bankswitch_w)
	AM_RANGE(0x700034, 0x700035) AM_WRITE(coinctrl_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( othldrby )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE2 )	/* TEST */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END



static const gfx_layout spritelayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+0, RGN_FRAC(0,2)+8, RGN_FRAC(0,2)+0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+0, RGN_FRAC(0,2)+8, RGN_FRAC(0,2)+0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16 },
	16*32
};

static GFXDECODE_START( othldrby )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 0, 0x80 )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,   0, 0x80 )
GFXDECODE_END



static MACHINE_START( othldrby )
{
	othldrby_state *state = (othldrby_state *)machine->driver_data;

	state_save_register_global(machine, state->toggle);
	state_save_register_global(machine, state->vram_addr);
	state_save_register_global(machine, state->vreg_addr);
	state_save_register_global_array(machine, state->vreg);
}

static MACHINE_RESET( othldrby )
{
	othldrby_state *state = (othldrby_state *)machine->driver_data;

	state->toggle = 0xff;
	state->vram_addr = 0;
	state->vreg_addr = 0;

	memset(state->vreg, 0, ARRAY_LENGTH(state->vreg));
}

static MACHINE_DRIVER_START( othldrby )

	/* driver data */
	MDRV_DRIVER_DATA(othldrby_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 16000000)
	MDRV_CPU_PROGRAM_MAP(othldrby_map)
	MDRV_CPU_VBLANK_INT("screen", irq4_line_hold)

	MDRV_MACHINE_START(othldrby)
	MDRV_MACHINE_RESET(othldrby)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(12*8, (64-12)*8-1, 1*8, 31*8-1 )

	MDRV_GFXDECODE(othldrby)
	MDRV_PALETTE_LENGTH(0x800)

	MDRV_VIDEO_START(othldrby)
	MDRV_VIDEO_EOF(othldrby)
	MDRV_VIDEO_UPDATE(othldrby)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("oki", OKIM6295, 1584000)
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( othldrby )
	ROM_REGION( 0x080000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "db0.1",        0x00000, 0x80000, CRC(6b4008d3) SHA1(4cf838c47563ba482be8364b2e115569a4a06c83) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD( "db0-r2",       0x000000, 0x200000, CRC(4efff265) SHA1(4cd239ff42f532495946cb52bd1fee412f84e192) )
	ROM_LOAD( "db0-r3",       0x200000, 0x200000, CRC(5c142b38) SHA1(5466a8b061a0f2545493de0f96fd4387beea276a) )

	ROM_REGION( 0x080000, "oki", 0 )	/* OKIM6295 samples */
	ROM_LOAD( "db0.4",        0x00000, 0x80000, CRC(a9701868) SHA1(9ee89556666d358e8d3915622573b3ba660048b8) )
ROM_END

GAME( 1995, othldrby, 0, othldrby, othldrby, 0, ROT0, "Sunwise", "Othello Derby (Japan)", GAME_SUPPORTS_SAVE )

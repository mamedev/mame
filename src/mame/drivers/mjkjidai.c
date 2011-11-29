/***************************************************************************

Mahjong Kyou Jidai     (c)1986 Sanritsu

CPU :Z80
Sound   :SN76489*2 CUSTOM
OSC :10MHz ??MHz

driver by Nicola Salmoria

TODO:
- Dip switches.

- Several imperfections with sprites rendering:
  - some sprites are misplaced by 1pixel vertically
  - during the tile distribution at the beginning of a match, there's something
    wrong with the stacks moved around, they are misaligned and something is
    missing.

- unknown reads from port 01. Only the top two bits seem to be used.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "sound/okim6295.h"
#include "includes/mjkjidai.h"

/* Start of ADPCM custom chip code */
typedef struct _mjkjidai_adpcm_state mjkjidai_adpcm_state;
struct _mjkjidai_adpcm_state
{
	adpcm_state m_adpcm;
	sound_stream *m_stream;
	UINT32 m_current;
	UINT32 m_end;
	UINT8 m_nibble;
	UINT8 m_playing;
	UINT8 *m_base;
} _mjkjidai_adpcm_state_dummy;

static STREAM_UPDATE( mjkjidai_adpcm_callback )
{
	mjkjidai_adpcm_state *state = (mjkjidai_adpcm_state *)param;
	stream_sample_t *dest = outputs[0];

	while (state->m_playing && samples > 0)
	{
		int val = (state->m_base[state->m_current] >> state->m_nibble) & 15;

		state->m_nibble ^= 4;
		if (state->m_nibble == 4)
		{
			state->m_current++;
			if (state->m_current >= state->m_end)
				state->m_playing = 0;
		}

		*dest++ = state->m_adpcm.clock(val) << 4;
		samples--;
	}
	while (samples > 0)
	{
		*dest++ = 0;
		samples--;
	}
}

static DEVICE_START( mjkjidai_adpcm )
{
	running_machine &machine = device->machine();
	mjkjidai_adpcm_state *state = (mjkjidai_adpcm_state *)downcast<legacy_device_base *>(device)->token();

	state->m_playing = 0;
	state->m_stream = device->machine().sound().stream_alloc(*device, 0, 1, device->clock(), state, mjkjidai_adpcm_callback);
	state->m_base = machine.region("adpcm")->base();
	state->m_adpcm.reset();
}

DEVICE_GET_INFO( mjkjidai_adpcm )
{
	switch (state)
	{
		case DEVINFO_INT_TOKEN_BYTES: info->i = sizeof(mjkjidai_adpcm_state); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(mjkjidai_adpcm);break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Custom ADPCM");				break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}

DECLARE_LEGACY_SOUND_DEVICE(MJKJIDAI, mjkjidai_adpcm);
DEFINE_LEGACY_SOUND_DEVICE(MJKJIDAI, mjkjidai_adpcm);


static void mjkjidai_adpcm_play (mjkjidai_adpcm_state *state, int offset, int length)
{
	state->m_current = offset;
	state->m_end = offset + length/2;
	state->m_nibble = 4;
	state->m_playing = 1;
}

static WRITE8_DEVICE_HANDLER( adpcm_w )
{
	mjkjidai_adpcm_state *state = (mjkjidai_adpcm_state *)downcast<legacy_device_base *>(device)->token();
	mjkjidai_adpcm_play (state, (data & 0x07) * 0x1000, 0x1000 * 2);
}
/* End of ADPCM custom chip code */


static READ8_HANDLER( keyboard_r )
{
	mjkjidai_state *state = space->machine().driver_data<mjkjidai_state>();
	int res = 0x3f,i;
	static const char *const keynames[] = { "PL2_1", "PL2_2", "PL2_3", "PL2_4", "PL2_5", "PL2_6", "PL1_1", "PL1_2", "PL1_3", "PL1_4", "PL1_5", "PL1_6" };

//  logerror("%04x: keyboard_r\n", cpu_get_pc(&space->device()));

	for (i = 0; i < 12; i++)
	{
		if (~state->m_keyb & (1 << i))
		{
			res = input_port_read(space->machine(), keynames[i]) & 0x3f;
			break;
		}
	}

	res |= (input_port_read(space->machine(), "IN3") & 0xc0);

	if (state->m_nvram_init_count)
	{
		state->m_nvram_init_count--;
		res &= 0xbf;
	}

	return res;
}

static WRITE8_HANDLER( keyboard_select_w )
{
	mjkjidai_state *state = space->machine().driver_data<mjkjidai_state>();

//  logerror("%04x: keyboard_select %d = %02x\n",cpu_get_pc(&space->device()),offset,data);

	switch (offset)
	{
		case 0: state->m_keyb = (state->m_keyb & 0xff00) | (data);      break;
		case 1: state->m_keyb = (state->m_keyb & 0x00ff) | (data << 8); break;
	}
}

static NVRAM_HANDLER( mjkjidai )
{
	mjkjidai_state *state = machine.driver_data<mjkjidai_state>();

	if (read_or_write)
		file->write(state->m_nvram, state->m_nvram_size);
	else if (file)
		file->read(state->m_nvram, state->m_nvram_size);
	else
	{
		state->m_nvram_init_count = 1;
	}
}



static ADDRESS_MAP_START( mjkjidai_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xcfff) AM_RAM
	AM_RANGE(0xd000, 0xdfff) AM_RAM	AM_BASE_SIZE_MEMBER(mjkjidai_state,m_nvram,m_nvram_size)	// cleared and initialized on startup if bit 6 if port 00 is 0
	AM_RANGE(0xe000, 0xe01f) AM_RAM AM_BASE_MEMBER(mjkjidai_state,m_spriteram1)			// shared with tilemap ram
	AM_RANGE(0xe800, 0xe81f) AM_RAM AM_BASE_MEMBER(mjkjidai_state,m_spriteram2)		// shared with tilemap ram
	AM_RANGE(0xf000, 0xf01f) AM_RAM AM_BASE_MEMBER(mjkjidai_state,m_spriteram3)		// shared with tilemap ram
	AM_RANGE(0xe000, 0xf7ff) AM_RAM_WRITE(mjkjidai_videoram_w) AM_BASE_MEMBER(mjkjidai_state,m_videoram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mjkjidai_io_map, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(keyboard_r)
	AM_RANGE(0x01, 0x01) AM_READNOP	// ???
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN2")
	AM_RANGE(0x01, 0x02) AM_WRITE(keyboard_select_w)
	AM_RANGE(0x10, 0x10) AM_WRITE(mjkjidai_ctrl_w)	// rom bank, coin counter, flip screen etc
	AM_RANGE(0x11, 0x11) AM_READ_PORT("IN0")
	AM_RANGE(0x12, 0x12) AM_READ_PORT("IN1")
	AM_RANGE(0x20, 0x20) AM_DEVWRITE("sn1", sn76496_w)
	AM_RANGE(0x30, 0x30) AM_DEVWRITE("sn2", sn76496_w)
	AM_RANGE(0x40, 0x40) AM_DEVWRITE("adpcm", adpcm_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( mjkjidai )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x04, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE )	// service mode
	PORT_DIPNAME( 0x20, 0x20, "Statistics" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )

	PORT_START("IN3")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )	// reinitialize NVRAM and reset the game
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("PL1_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x3e, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x38, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1_4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1_5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL1_6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x3e, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x38, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};

static GFXDECODE_START( mjkjidai )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 0, 16 )
GFXDECODE_END

static INTERRUPT_GEN( vblank_irq )
{
	mjkjidai_state *state = device->machine().driver_data<mjkjidai_state>();

	if(state->m_nmi_mask)
		device_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_START( mjkjidai, mjkjidai_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,10000000/2)	/* 5 MHz ??? */
	MCFG_CPU_PROGRAM_MAP(mjkjidai_map)
	MCFG_CPU_IO_MAP(mjkjidai_io_map)
	MCFG_CPU_VBLANK_INT("screen", vblank_irq)

	MCFG_NVRAM_HANDLER(mjkjidai)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(3*8, 61*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE(mjkjidai)

	MCFG_GFXDECODE(mjkjidai)
	MCFG_PALETTE_LENGTH(0x100)

	MCFG_PALETTE_INIT(RRRR_GGGG_BBBB)
	MCFG_VIDEO_START(mjkjidai)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("sn1", SN76489, 10000000/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("sn2", SN76489, 10000000/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("adpcm", MJKJIDAI, 6000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( mjkjidai )
	ROM_REGION( 0x1c000, "maincpu", 0 )
	ROM_LOAD( "mkj-00.14g",   0x00000, 0x8000, CRC(188a27e9) SHA1(2306ad112aaf8d9ac77a89d0e4c3a17f36945130) )
	ROM_LOAD( "mkj-01.15g",   0x08000, 0x4000, CRC(a6a5e9c7) SHA1(974f4343f4347a0065f833c1fdcc47e96d42932d) )	/* banked, there is code flowing from 7fff to this bank */
	ROM_CONTINUE(             0x10000, 0x4000 )
	ROM_LOAD( "mkj-02.16g",   0x14000, 0x8000, CRC(fb312927) SHA1(b71db72ba881474f9c2523d0617757889af9f28e) )

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "mkj-20.4e",    0x00000, 0x8000, CRC(8fc66bce) SHA1(4f1006bc5168e39eb7a1f6a4b3c3f5aaa3c1c7dd) )
	ROM_LOAD( "mkj-21.5e",    0x08000, 0x8000, CRC(4dd41a9b) SHA1(780f9e5bbf9dc47e931cebd67d89122209f573a2) )
	ROM_LOAD( "mkj-22.6e",    0x10000, 0x8000, CRC(70ac2bd7) SHA1(8ddb00a24f2b49b9eb1a70ae95fcd6bb0820be50) )
	ROM_LOAD( "mkj-23.7e",    0x18000, 0x8000, CRC(f9313dde) SHA1(787577ccdc7e7030439159c194ca6719df80ad2f) )
	ROM_LOAD( "mkj-24.8e",    0x20000, 0x8000, CRC(aa5130d0) SHA1(1dbaf2ba9ed97c22dc74d12471fc54b0f7ce2f25) )
	ROM_LOAD( "mkj-25.9e",    0x28000, 0x8000, CRC(c12c3fe0) SHA1(0acd3f8e8d849a09b187cd83852593a64aa87451) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "mkj-60.13a",   0x0000, 0x0100, CRC(5dfaba60) SHA1(7c821a5e951ccf9d86d98aa8dc75d847ab579496) )
	ROM_LOAD( "mkj-61.14a",   0x0100, 0x0100, CRC(e9e90d55) SHA1(a14177df3bab59e0f9ce41094e03ef3593329149) )
	ROM_LOAD( "mkj-62.15a",   0x0200, 0x0100, CRC(934f1d53) SHA1(2b3b2dc77789b814810b25cda3f5adcfd7e0e57e) )

	ROM_REGION( 0x8000, "adpcm", 0 )	/* ADPCM samples */
	ROM_LOAD( "mkj-40.14c",   0x00000, 0x8000, CRC(4d8fcc4a) SHA1(24c2b8031367035c89c6649a084bce0714f3e8d4) )
ROM_END


GAME( 1986, mjkjidai, 0, mjkjidai, mjkjidai, 0, ROT0, "Sanritsu",  "Mahjong Kyou Jidai (Japan)", GAME_IMPERFECT_GRAPHICS )


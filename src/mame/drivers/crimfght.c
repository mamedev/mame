/***************************************************************************

    Crime Fighters (Konami GX821) (c) 1989 Konami

    Preliminary driver by:
        Manuel Abadia <manu@teleline.es>


    2008-08
    Dip locations verified with manual (US)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/konami/konami.h" /* for the callback and the firq irq definition */
#include "video/konicdev.h"
#include "sound/2151intf.h"
#include "sound/k007232.h"
#include "includes/konamipt.h"
#include "includes/crimfght.h"

/* prototypes */
static KONAMI_SETLINES_CALLBACK( crimfght_banking );


WRITE8_MEMBER(crimfght_state::crimfght_coin_w)
{
	coin_counter_w(machine(), 0, data & 1);
	coin_counter_w(machine(), 1, data & 2);
}

WRITE8_MEMBER(crimfght_state::crimfght_sh_irqtrigger_w)
{
	soundlatch_w(space, offset, data);
	device_set_input_line_and_vector(m_audiocpu, 0, HOLD_LINE, 0xff);
}

static WRITE8_DEVICE_HANDLER( crimfght_snd_bankswitch_w )
{
	/* b1: bank for channel A */
	/* b0: bank for channel B */

	crimfght_state *state = device->machine().driver_data<crimfght_state>();
	int bank_A = BIT(data, 1);
	int bank_B = BIT(data, 0);

	k007232_set_bank(state->m_k007232, bank_A, bank_B );
}

READ8_MEMBER(crimfght_state::k052109_051960_r)
{

	if (k052109_get_rmrd_line(m_k052109) == CLEAR_LINE)
	{
		if (offset >= 0x3800 && offset < 0x3808)
			return k051937_r(m_k051960, offset - 0x3800);
		else if (offset < 0x3c00)
			return k052109_r(m_k052109, offset);
		else
			return k051960_r(m_k051960, offset - 0x3c00);
	}
	else
		return k052109_r(m_k052109, offset);
}

WRITE8_MEMBER(crimfght_state::k052109_051960_w)
{

	if (offset >= 0x3800 && offset < 0x3808)
		k051937_w(m_k051960, offset - 0x3800, data);
	else if (offset < 0x3c00)
		k052109_w(m_k052109, offset, data);
	else
		k051960_w(m_k051960, offset - 0x3c00, data);
}

/********************************************/

static ADDRESS_MAP_START( crimfght_map, AS_PROGRAM, 8, crimfght_state )
	AM_RANGE(0x0000, 0x03ff) AM_RAMBANK("bank1")					/* banked RAM */
	AM_RANGE(0x0400, 0x1fff) AM_RAM												/* RAM */
	AM_RANGE(0x3f80, 0x3f80) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x3f81, 0x3f81) AM_READ_PORT("P1")
	AM_RANGE(0x3f82, 0x3f82) AM_READ_PORT("P2")
	AM_RANGE(0x3f83, 0x3f83) AM_READ_PORT("DSW2")
	AM_RANGE(0x3f84, 0x3f84) AM_READ_PORT("DSW3")
	AM_RANGE(0x3f85, 0x3f85) AM_READ_PORT("P3")
	AM_RANGE(0x3f86, 0x3f86) AM_READ_PORT("P4")
	AM_RANGE(0x3f87, 0x3f87) AM_READ_PORT("DSW1")
	AM_RANGE(0x3f88, 0x3f88) AM_READ(watchdog_reset_r) AM_WRITE(crimfght_coin_w)	/* watchdog reset */
	AM_RANGE(0x3f8c, 0x3f8c) AM_WRITE(crimfght_sh_irqtrigger_w)	/* cause interrupt on audio CPU? */
	AM_RANGE(0x2000, 0x5fff) AM_READWRITE(k052109_051960_r, k052109_051960_w)	/* video RAM + sprite RAM */
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("bank2")						/* banked ROM */
	AM_RANGE(0x8000, 0xffff) AM_ROM												/* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( crimfght_sound_map, AS_PROGRAM, 8, crimfght_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM												/* ROM 821l01.h4 */
	AM_RANGE(0x8000, 0x87ff) AM_RAM												/* RAM */
	AM_RANGE(0xa000, 0xa001) AM_DEVREADWRITE_LEGACY("ymsnd", ym2151_r, ym2151_w)			/* YM2151 */
	AM_RANGE(0xc000, 0xc000) AM_READ(soundlatch_r)								/* soundlatch_r */
	AM_RANGE(0xe000, 0xe00d) AM_DEVREADWRITE_LEGACY("k007232", k007232_r, k007232_w)	/* 007232 registers */
ADDRESS_MAP_END

/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( crimfght )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/99 Credits" )
	PORT_DIPUNUSED_DIPLOC( 0xf0, 0xf0, "SW1:5,6,7,8" ) /* Manual says these are unused */

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW2:1" ) /* Manual says these are unused */
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW2:2" ) /* Manual says these are unused */
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW2:3" ) /* Manual says these are unused */
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW2:4" ) /* Manual says these are unused */
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW2:5" ) /* Manual says these are unused */
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW3:2" ) /* Manual says these are unused */
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_HIGH, "SW3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW3:4" ) /* Manual says these are unused */
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	KONAMI8_B12_UNK(1)

	PORT_START("P2")
	KONAMI8_B12_UNK(2)

	PORT_START("P3")
	KONAMI8_B12_UNK(3)

	PORT_START("P4")
	KONAMI8_B12_UNK(4)

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE4 )
INPUT_PORTS_END

static INPUT_PORTS_START( crimfghtj )
	PORT_INCLUDE( crimfght )

	PORT_MODIFY("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )

	PORT_MODIFY("P1")
	KONAMI8_B123_START(1)

	PORT_MODIFY("P2")
	KONAMI8_B123_START(2)

	PORT_MODIFY("P3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("P4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



/***************************************************************************

    Machine Driver

***************************************************************************/

static const ym2151_interface ym2151_config =
{
	0,
	crimfght_snd_bankswitch_w
};

static void volume_callback( device_t *device, int v )
{
	k007232_set_volume(device, 0, (v & 0x0f) * 0x11, 0);
	k007232_set_volume(device, 1, 0, (v >> 4) * 0x11);
}

static const k007232_interface k007232_config =
{
	volume_callback	/* external port callback */
};


static const k052109_interface crimfght_k052109_intf =
{
	"gfx1", 0,
	NORMAL_PLANE_ORDER,
	KONAMI_ROM_DEINTERLEAVE_2,
	crimfght_tile_callback
};

static const k051960_interface crimfght_k051960_intf =
{
	"gfx2", 1,
	NORMAL_PLANE_ORDER,
	KONAMI_ROM_DEINTERLEAVE_2,
	crimfght_sprite_callback
};

static MACHINE_START( crimfght )
{
	crimfght_state *state = machine.driver_data<crimfght_state>();
	UINT8 *ROM = machine.region("maincpu")->base();

	memory_configure_bank(machine, "bank2", 0, 12, &ROM[0x10000], 0x2000);
	memory_set_bank(machine, "bank2", 0);

	state->m_maincpu = machine.device("maincpu");
	state->m_audiocpu = machine.device("audiocpu");
	state->m_k052109 = machine.device("k052109");
	state->m_k051960 = machine.device("k051960");
	state->m_k007232 = machine.device("k007232");
}

static MACHINE_RESET( crimfght )
{
	konami_configure_set_lines(machine.device("maincpu"), crimfght_banking);
}

static MACHINE_CONFIG_START( crimfght, crimfght_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", KONAMI, 3000000)		/* ? */
	MCFG_CPU_PROGRAM_MAP(crimfght_map)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 3579545)	/* verified with PCB */
	MCFG_CPU_PROGRAM_MAP(crimfght_sound_map)

	MCFG_MACHINE_START(crimfght)
	MCFG_MACHINE_RESET(crimfght)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(54)	/* adjusted - compared with PCB speed */
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(13*8, (64-13)*8-1, 2*8, 30*8-1 )
	MCFG_SCREEN_UPDATE_STATIC(crimfght)

	MCFG_PALETTE_LENGTH(512)

	MCFG_VIDEO_START(crimfght)

	MCFG_K052109_ADD("k052109", crimfght_k052109_intf)
	MCFG_K051960_ADD("k051960", crimfght_k051960_intf)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2151, 3579545)	/* verified with PCB */
	MCFG_SOUND_CONFIG(ym2151_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)

	MCFG_SOUND_ADD("k007232", K007232, 3579545)
	MCFG_SOUND_CONFIG(k007232_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.20)
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.20)
	MCFG_SOUND_ROUTE(1, "lspeaker", 0.20)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.20)
MACHINE_CONFIG_END

/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( crimfght )
	ROM_REGION( 0x28000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "821l02.f24", 0x10000, 0x18000, CRC(588e7da6) SHA1(285febb3bcca31f82b34af3695a59eafae01cd30) )
	ROM_CONTINUE(           0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "821l01.h4",  0x0000, 0x8000, CRC(0faca89e) SHA1(21c9c6d736b398a29e8709e1187c5bf3cacdc99d) )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "821k06.k13", 0x000000, 0x040000, CRC(a1eadb24) SHA1(ca305b904b34e03918ad07281fda86ad63caa44f) )	/* characters */
	ROM_LOAD( "821k07.k19", 0x040000, 0x040000, CRC(060019fa) SHA1(c3bca007aaa5f1c534d2a75fe4f96d01a740dd58) )

	ROM_REGION( 0x100000, "gfx2", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "821k04.k2",  0x000000, 0x080000, CRC(00e0291b) SHA1(39d5db6cf36826e47cdf5308eff9bfa8afc82050) )	/* sprites */
	ROM_LOAD( "821k05.k8",  0x080000, 0x080000, CRC(e09ea05d) SHA1(50ac9a2117ce63fe774c48d769ec445a83f1269e) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "821a08.i15", 0x0000, 0x0100, CRC(7da55800) SHA1(3826f73569c8ae0431510a355bdfa082152b74a5) )	/* priority encoder (not used) */

	ROM_REGION( 0x40000, "k007232", 0 )	/* data for the 007232 */
	ROM_LOAD( "821k03.e5",  0x00000, 0x40000, CRC(fef8505a) SHA1(5c5121609f69001838963e961cb227d6b64e4f5f) )
ROM_END

ROM_START( crimfghtj )
	ROM_REGION( 0x28000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "821p02.bin", 0x10000, 0x18000, CRC(f33fa2e1) SHA1(00fc9e8250fa51386f3af2fca0f137bec9e1c220) )
	ROM_CONTINUE(           0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "821l01.h4",  0x0000, 0x8000, CRC(0faca89e) SHA1(21c9c6d736b398a29e8709e1187c5bf3cacdc99d) )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "821k06.k13", 0x000000, 0x040000, CRC(a1eadb24) SHA1(ca305b904b34e03918ad07281fda86ad63caa44f) )	/* characters */
	ROM_LOAD( "821k07.k19", 0x040000, 0x040000, CRC(060019fa) SHA1(c3bca007aaa5f1c534d2a75fe4f96d01a740dd58) )

	ROM_REGION( 0x100000, "gfx2", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "821k04.k2",  0x000000, 0x080000, CRC(00e0291b) SHA1(39d5db6cf36826e47cdf5308eff9bfa8afc82050) )	/* sprites */
	ROM_LOAD( "821k05.k8",  0x080000, 0x080000, CRC(e09ea05d) SHA1(50ac9a2117ce63fe774c48d769ec445a83f1269e) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "821a08.i15", 0x0000, 0x0100, CRC(7da55800) SHA1(3826f73569c8ae0431510a355bdfa082152b74a5) )	/* priority encoder (not used) */

	ROM_REGION( 0x40000, "k007232", 0 )	/* data for the 007232 */
	ROM_LOAD( "821k03.e5",  0x00000, 0x40000, CRC(fef8505a) SHA1(5c5121609f69001838963e961cb227d6b64e4f5f) )
ROM_END

ROM_START( crimfght2 )
ROM_REGION( 0x28000, "maincpu", 0 ) /* code + banked roms */
	ROM_LOAD( "crimefb.r02", 0x10000, 0x18000, CRC(4ecdd923) SHA1(78e5260c4bb9b18d7818fb6300d7e1d3a577fb63) )
	ROM_CONTINUE(           0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "821l01.h4",  0x0000, 0x8000, CRC(0faca89e) SHA1(21c9c6d736b398a29e8709e1187c5bf3cacdc99d) )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "821k06.k13", 0x000000, 0x040000, CRC(a1eadb24) SHA1(ca305b904b34e03918ad07281fda86ad63caa44f) )	/* characters */
	ROM_LOAD( "821k07.k19", 0x040000, 0x040000, CRC(060019fa) SHA1(c3bca007aaa5f1c534d2a75fe4f96d01a740dd58) )

	ROM_REGION( 0x100000, "gfx2", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "821k04.k2",  0x000000, 0x080000, CRC(00e0291b) SHA1(39d5db6cf36826e47cdf5308eff9bfa8afc82050) )	/* sprites */
	ROM_LOAD( "821k05.k8",  0x080000, 0x080000, CRC(e09ea05d) SHA1(50ac9a2117ce63fe774c48d769ec445a83f1269e) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "821a08.i15", 0x0000, 0x0100, CRC(7da55800) SHA1(3826f73569c8ae0431510a355bdfa082152b74a5) )	/* priority encoder (not used) */

	ROM_REGION( 0x40000, "k007232", 0 )	/* data for the 007232 */
	ROM_LOAD( "821k03.e5",  0x00000, 0x40000, CRC(fef8505a) SHA1(5c5121609f69001838963e961cb227d6b64e4f5f) )
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

static KONAMI_SETLINES_CALLBACK( crimfght_banking )
{
	crimfght_state *state = device->machine().driver_data<crimfght_state>();

	/* bit 5 = select work RAM or palette */
	if (lines & 0x20)
	{
		device->memory().space(AS_PROGRAM)->install_read_bank(0x0000, 0x03ff, "bank3");
		device->memory().space(AS_PROGRAM)->install_write_handler(0x0000, 0x03ff, write8_delegate(FUNC(crimfght_state::paletteram_xBBBBBGGGGGRRRRR_be_w), state));
		memory_set_bankptr(device->machine(), "bank3", state->m_generic_paletteram_8);
	}
	else
		device->memory().space(AS_PROGRAM)->install_readwrite_bank(0x0000, 0x03ff, "bank1");								/* RAM */

	/* bit 6 = enable char ROM reading through the video RAM */
	k052109_set_rmrd_line(state->m_k052109, (lines & 0x40) ? ASSERT_LINE : CLEAR_LINE);

	memory_set_bank(device->machine(), "bank2", lines & 0x0f);
}

GAME( 1989, crimfght,  0,        crimfght, crimfght, 0, ROT0, "Konami", "Crime Fighters (US 4 players)", GAME_SUPPORTS_SAVE )
GAME( 1989, crimfght2, crimfght, crimfght, crimfghtj,0, ROT0, "Konami", "Crime Fighters (World 2 Players)", GAME_SUPPORTS_SAVE )
GAME( 1989, crimfghtj, crimfght, crimfght, crimfghtj,0, ROT0, "Konami", "Crime Fighters (Japan 2 Players)", GAME_SUPPORTS_SAVE )

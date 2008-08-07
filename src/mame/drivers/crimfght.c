/***************************************************************************

Crime Fighters (Konami GX821) (c) 1989 Konami

Preliminary driver by:
    Manuel Abadia <manu@teleline.es>


2008-08
Dip locations verified with manual (US)

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/konami/konami.h" /* for the callback and the firq irq definition */
#include "video/konamiic.h"
#include "sound/2151intf.h"
#include "sound/k007232.h"


/* prototypes */
static MACHINE_RESET( crimfght );
static void crimfght_banking( int lines );

VIDEO_START( crimfght );
VIDEO_UPDATE( crimfght );


static WRITE8_HANDLER( crimfght_coin_w )
{
	coin_counter_w(0,data & 1);
	coin_counter_w(1,data & 2);
}

static WRITE8_HANDLER( crimfght_sh_irqtrigger_w )
{
	soundlatch_w(machine,offset,data);
	cpunum_set_input_line_and_vector(machine, 1,0,HOLD_LINE,0xff);
}

static WRITE8_HANDLER( crimfght_snd_bankswitch_w )
{
	/* b1: bank for channel A */
	/* b0: bank for channel B */

	int bank_A = ((data >> 1) & 0x01);
	int bank_B = ((data) & 0x01);
	K007232_set_bank( 0, bank_A, bank_B );
}


/********************************************/

static ADDRESS_MAP_START( crimfght_readmem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x03ff) AM_READ(SMH_BANK1)			/* banked RAM */
	AM_RANGE(0x0400, 0x1fff) AM_READ(SMH_RAM)			/* RAM */
	AM_RANGE(0x3f80, 0x3f80) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x3f81, 0x3f81) AM_READ_PORT("P1")
	AM_RANGE(0x3f82, 0x3f82) AM_READ_PORT("P2")
	AM_RANGE(0x3f83, 0x3f83) AM_READ_PORT("DSW2")
	AM_RANGE(0x3f84, 0x3f84) AM_READ_PORT("DSW3")
	AM_RANGE(0x3f85, 0x3f85) AM_READ_PORT("P3")
	AM_RANGE(0x3f86, 0x3f86) AM_READ_PORT("P4")
	AM_RANGE(0x3f87, 0x3f87) AM_READ_PORT("DSW1")
	AM_RANGE(0x3f88, 0x3f88) AM_READ(watchdog_reset_r)	/* watchdog reset */
	AM_RANGE(0x2000, 0x5fff) AM_READ(K052109_051960_r)	/* video RAM + sprite RAM */
	AM_RANGE(0x6000, 0x7fff) AM_READ(SMH_BANK2)			/* banked ROM */
	AM_RANGE(0x8000, 0xffff) AM_READ(SMH_ROM)			/* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( crimfght_writemem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x03ff) AM_WRITE(SMH_BANK1)					/* banked RAM */
	AM_RANGE(0x0400, 0x1fff) AM_WRITE(SMH_RAM)					/* RAM */
	AM_RANGE(0x3f88, 0x3f88) AM_WRITE(crimfght_coin_w)			/* coin counters */
	AM_RANGE(0x3f8c, 0x3f8c) AM_WRITE(crimfght_sh_irqtrigger_w)	/* cause interrupt on audio CPU? */
	AM_RANGE(0x2000, 0x5fff) AM_WRITE(K052109_051960_w)			/* video RAM + sprite RAM */
	AM_RANGE(0x6000, 0x7fff) AM_WRITE(SMH_ROM)					/* banked ROM */
	AM_RANGE(0x8000, 0xffff) AM_WRITE(SMH_ROM)					/* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( crimfght_readmem_sound, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_READ(SMH_ROM)				/* ROM 821l01.h4 */
	AM_RANGE(0x8000, 0x87ff) AM_READ(SMH_RAM)				/* RAM */
	AM_RANGE(0xa001, 0xa001) AM_READ(YM2151_status_port_0_r)	/* YM2151 */
	AM_RANGE(0xc000, 0xc000) AM_READ(soundlatch_r)			/* soundlatch_r */
	AM_RANGE(0xe000, 0xe00d) AM_READ(K007232_read_port_0_r)	/* 007232 registers */
ADDRESS_MAP_END

static ADDRESS_MAP_START( crimfght_writemem_sound, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_WRITE(SMH_ROM)					/* ROM 821l01.h4 */
	AM_RANGE(0x8000, 0x87ff) AM_WRITE(SMH_RAM)					/* RAM */
	AM_RANGE(0xa000, 0xa000) AM_WRITE(YM2151_register_port_0_w)	/* YM2151 */
	AM_RANGE(0xa001, 0xa001) AM_WRITE(YM2151_data_port_0_w)		/* YM2151 */
	AM_RANGE(0xe000, 0xe00d) AM_WRITE(K007232_write_port_0_w)		/* 007232 registers */
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
/*  PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
    PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
    PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
    PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
    PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
    PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
    PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
    PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
    PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
    PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
    PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
    PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
    PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
    PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
    PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
    PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
    PORT_DIPSETTING(    0x00, "Invalid" ) */

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW2:1" ) /* Manual says these are unused */
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW2:2" ) /* Manual says these are unused */
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW2:3" ) /* Manual says these are unused */
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW2:4" ) /* Manual says these are unused */
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW2:5" ) /* Manual says these are unused */
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, "Difficult" )
	PORT_DIPSETTING(    0x00, "Very difficult" )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

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

static INPUT_PORTS_START( crimfgtj )
	PORT_INCLUDE( crimfght )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
//  PORT_DIPSETTING(    0x00, "Invalid" )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )

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

static const struct YM2151interface ym2151_interface =
{
	0,
	crimfght_snd_bankswitch_w
};

static void volume_callback(int v)
{
	K007232_set_volume(0,0,(v & 0x0f) * 0x11,0);
	K007232_set_volume(0,1,0,(v >> 4) * 0x11);
}

static const struct K007232_interface k007232_interface =
{
	volume_callback	/* external port callback */
};



static MACHINE_DRIVER_START( crimfght )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", KONAMI, 3000000)		/* ? */
	MDRV_CPU_PROGRAM_MAP(crimfght_readmem,crimfght_writemem)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	MDRV_CPU_ADD("audio", Z80, 3579545)	/* verified with PCB */
	MDRV_CPU_PROGRAM_MAP(crimfght_readmem_sound,crimfght_writemem_sound)

	MDRV_MACHINE_RESET(crimfght)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(54)	/* adjusted - compared with PCB speed */
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(13*8, (64-13)*8-1, 2*8, 30*8-1 )

	MDRV_PALETTE_LENGTH(512)

	MDRV_VIDEO_START(crimfght)
	MDRV_VIDEO_UPDATE(crimfght)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD("ym", YM2151, 3579545)	/* verified with PCB */
	MDRV_SOUND_CONFIG(ym2151_interface)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)

	MDRV_SOUND_ADD("konami", K007232, 3579545)
	MDRV_SOUND_CONFIG(k007232_interface)
	MDRV_SOUND_ROUTE(0, "left", 0.20)
	MDRV_SOUND_ROUTE(0, "right", 0.20)
	MDRV_SOUND_ROUTE(1, "left", 0.20)
	MDRV_SOUND_ROUTE(1, "right", 0.20)
MACHINE_DRIVER_END

/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( crimfght )
	ROM_REGION( 0x28000, "main", 0 ) /* code + banked roms */
	ROM_LOAD( "821l02.f24", 0x10000, 0x18000, CRC(588e7da6) SHA1(285febb3bcca31f82b34af3695a59eafae01cd30) )
	ROM_CONTINUE(           0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audio", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "821l01.h4",  0x0000, 0x8000, CRC(0faca89e) SHA1(21c9c6d736b398a29e8709e1187c5bf3cacdc99d) )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "821k06.k13", 0x000000, 0x040000, CRC(a1eadb24) SHA1(ca305b904b34e03918ad07281fda86ad63caa44f) )	/* characters */
	ROM_LOAD( "821k07.k19", 0x040000, 0x040000, CRC(060019fa) SHA1(c3bca007aaa5f1c534d2a75fe4f96d01a740dd58) )

	ROM_REGION( 0x100000, "gfx2", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "821k04.k2",  0x000000, 0x080000, CRC(00e0291b) SHA1(39d5db6cf36826e47cdf5308eff9bfa8afc82050) )	/* sprites */
	ROM_LOAD( "821k05.k8",  0x080000, 0x080000, CRC(e09ea05d) SHA1(50ac9a2117ce63fe774c48d769ec445a83f1269e) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "821a08.i15", 0x0000, 0x0100, CRC(7da55800) SHA1(3826f73569c8ae0431510a355bdfa082152b74a5) )	/* priority encoder (not used) */

	ROM_REGION( 0x40000, "konami", 0 )	/* data for the 007232 */
	ROM_LOAD( "821k03.e5",  0x00000, 0x40000, CRC(fef8505a) SHA1(5c5121609f69001838963e961cb227d6b64e4f5f) )
ROM_END

ROM_START( crimfgtj )
	ROM_REGION( 0x28000, "main", 0 ) /* code + banked roms */
	ROM_LOAD( "821p02.bin", 0x10000, 0x18000, CRC(f33fa2e1) SHA1(00fc9e8250fa51386f3af2fca0f137bec9e1c220) )
	ROM_CONTINUE(           0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audio", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "821l01.h4",  0x0000, 0x8000, CRC(0faca89e) SHA1(21c9c6d736b398a29e8709e1187c5bf3cacdc99d) )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "821k06.k13", 0x000000, 0x040000, CRC(a1eadb24) SHA1(ca305b904b34e03918ad07281fda86ad63caa44f) )	/* characters */
	ROM_LOAD( "821k07.k19", 0x040000, 0x040000, CRC(060019fa) SHA1(c3bca007aaa5f1c534d2a75fe4f96d01a740dd58) )

	ROM_REGION( 0x100000, "gfx2", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "821k04.k2",  0x000000, 0x080000, CRC(00e0291b) SHA1(39d5db6cf36826e47cdf5308eff9bfa8afc82050) )	/* sprites */
	ROM_LOAD( "821k05.k8",  0x080000, 0x080000, CRC(e09ea05d) SHA1(50ac9a2117ce63fe774c48d769ec445a83f1269e) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "821a08.i15", 0x0000, 0x0100, CRC(7da55800) SHA1(3826f73569c8ae0431510a355bdfa082152b74a5) )	/* priority encoder (not used) */

	ROM_REGION( 0x40000, "konami", 0 )	/* data for the 007232 */
	ROM_LOAD( "821k03.e5",  0x00000, 0x40000, CRC(fef8505a) SHA1(5c5121609f69001838963e961cb227d6b64e4f5f) )
ROM_END

ROM_START( crimfgt2 )
ROM_REGION( 0x28000, "main", 0 ) /* code + banked roms */
	ROM_LOAD( "crimefb.r02", 0x10000, 0x18000, CRC(4ecdd923) SHA1(78e5260c4bb9b18d7818fb6300d7e1d3a577fb63) )
	ROM_CONTINUE(           0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audio", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "821l01.h4",  0x0000, 0x8000, CRC(0faca89e) SHA1(21c9c6d736b398a29e8709e1187c5bf3cacdc99d) )

	ROM_REGION( 0x080000, "gfx1", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "821k06.k13", 0x000000, 0x040000, CRC(a1eadb24) SHA1(ca305b904b34e03918ad07281fda86ad63caa44f) )	/* characters */
	ROM_LOAD( "821k07.k19", 0x040000, 0x040000, CRC(060019fa) SHA1(c3bca007aaa5f1c534d2a75fe4f96d01a740dd58) )

	ROM_REGION( 0x100000, "gfx2", 0 ) /* graphics ( don't dispose as the program can read them, 0 ) */
	ROM_LOAD( "821k04.k2",  0x000000, 0x080000, CRC(00e0291b) SHA1(39d5db6cf36826e47cdf5308eff9bfa8afc82050) )	/* sprites */
	ROM_LOAD( "821k05.k8",  0x080000, 0x080000, CRC(e09ea05d) SHA1(50ac9a2117ce63fe774c48d769ec445a83f1269e) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "821a08.i15", 0x0000, 0x0100, CRC(7da55800) SHA1(3826f73569c8ae0431510a355bdfa082152b74a5) )	/* priority encoder (not used) */

	ROM_REGION( 0x40000, "konami", 0 )	/* data for the 007232 */
	ROM_LOAD( "821k03.e5",  0x00000, 0x40000, CRC(fef8505a) SHA1(5c5121609f69001838963e961cb227d6b64e4f5f) )
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

static void crimfght_banking( int lines )
{
	UINT8 *RAM = memory_region(Machine, "main");
	int offs = 0;

	/* bit 5 = select work RAM or palette */
	if (lines & 0x20)
	{
		memory_install_readwrite8_handler(Machine, 0, ADDRESS_SPACE_PROGRAM, 0x0000, 0x03ff, 0, 0, SMH_BANK3, paletteram_xBBBBBGGGGGRRRRR_be_w);
		memory_set_bankptr(3, paletteram);
	}
	else
		memory_install_readwrite8_handler(Machine, 0, ADDRESS_SPACE_PROGRAM, 0x0000, 0x03ff, 0, 0, SMH_BANK1, SMH_BANK1);								/* RAM */

	/* bit 6 = enable char ROM reading through the video RAM */
	K052109_set_RMRD_line((lines & 0x40) ? ASSERT_LINE : CLEAR_LINE);

	offs = 0x10000 + ((lines & 0x0f) * 0x2000);
	memory_set_bankptr(2, &RAM[offs]);
}

static MACHINE_RESET( crimfght )
{
	UINT8 *RAM = memory_region(machine, "main");

	cpunum_set_info_fct(0, CPUINFO_PTR_KONAMI_SETLINES_CALLBACK, (genf *)crimfght_banking);

	/* init the default bank */
	memory_set_bankptr( 2, &RAM[0x10000] );
}

static DRIVER_INIT( crimfght )
{
	konami_rom_deinterleave_2("gfx1");
	konami_rom_deinterleave_2("gfx2");
}



GAME( 1989, crimfght, 0,        crimfght, crimfght, crimfght, ROT0, "Konami", "Crime Fighters (US 4 players)", 0 )
GAME( 1989, crimfgt2, crimfght, crimfght, crimfgtj, crimfght, ROT0, "Konami", "Crime Fighters (World 2 Players)", 0 )
GAME( 1989, crimfgtj, crimfght, crimfght, crimfgtj, crimfght, ROT0, "Konami", "Crime Fighters (Japan 2 Players)", 0 )

/*
King Pin (c) 1983 American Communication Laboratories Inc.
Driver by Andrew Gardner

Notes:
    There are some writes around 0xe000 in the multi-game set that can't
        possibly go anywhere on the board I own.  A bigger RAM chip would
        accommodate them though.
    There are 6 pots labeled vr2-vr7.  Color adjustments?
    The edge-connectors are non-jamma on this board.

Todo:
    Hook up sound
        -two crystals on the board : 3.579545 for sound(?) and main CPU
                                     10.7386 for the tms9928ANL
    Figure out DIPs, buttons, and outputs
        -DIPs are weird.  setting them to ~0x6A and resetting the machine enters 'setup' mode
        -0x30-0x70 are very likely lights - do the buttons light up?
        -what really is I/O 0x02?
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/tms9928a.h"
#include "sound/ay8910.h"
#include "machine/nvram.h"


class kingpin_state : public driver_device
{
public:
	kingpin_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_code_base;
	DECLARE_READ8_MEMBER(io_read_missing_dips);
};


READ8_MEMBER(kingpin_state::io_read_missing_dips)
{
	return 0x00;
}


/* Ports */
static INPUT_PORTS_START( kingpin )
	PORT_START("IN0")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) /* Likely unused */
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME( "Start" )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME( "Apply Credit" )

	PORT_START("IN1")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_BUTTON9 )
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_BUTTON10 )
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_BUTTON11 )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_BUTTON12 )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_NAME( "Cash Out" )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_BUTTON14 )

	/* There are 3 banks of solder pads, but only one is poupulated with DIPs */
	PORT_START("DSW")
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
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END


/* PROGRAM MAPS */

/* Main CPU */
/* There's an OKI MSM5126-25RS in here - (2k RAM) */
/* A 3.6V battery traces directly to U19, rendering it nvram */
static ADDRESS_MAP_START( kingpin_program_map, AS_PROGRAM, 8, kingpin_state )
	AM_RANGE(0x0000, 0xdfff) AM_ROM
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( kingpin_io_map, AS_IO, 8, kingpin_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(io_read_missing_dips)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("DSW")
/*  AM_RANGE(0x02, 0x02) AM_READ(io_read_missing_dips) */
/*  AM_RANGE(0x02, 0x02) AM_WRITE_LEGACY(NO IDEA) */
	AM_RANGE(0x10, 0x10) AM_READ_PORT("IN0")
	AM_RANGE(0x11, 0x11) AM_READ_PORT("IN1")
/*  AM_RANGE(0x12, 0x12) AM_WRITE_LEGACY(NO IDEA) */
/*  AM_RANGE(0x13, 0x13) AM_WRITE_LEGACY(NO IDEA) */
	AM_RANGE(0x20, 0x20) AM_DEVREADWRITE("tms9928a", tms9928a_device, vram_read, vram_write)
	AM_RANGE(0x21, 0x21) AM_DEVREADWRITE("tms9928a", tms9928a_device, register_read, register_write)
/*  AM_RANGE(0x30, 0x30) AM_WRITE_LEGACY(LIKELY LIGHTS) */
/*  AM_RANGE(0x40, 0x40) AM_WRITE_LEGACY(LIKELY LIGHTS) */
/*  AM_RANGE(0x50, 0x50) AM_WRITE_LEGACY(LIKELY LIGHTS) */
/*  AM_RANGE(0x60, 0x60) AM_WRITE_LEGACY(LIKELY LIGHTS) */
/*  AM_RANGE(0x70, 0x70) AM_WRITE_LEGACY(LIKELY LIGHTS) */
ADDRESS_MAP_END


/* Sound CPU */
/* There's an OKI MSM5126-25RS in here - (2k RAM) */
static ADDRESS_MAP_START( kingpin_sound_map, AS_PROGRAM, 8, kingpin_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x8400, 0x8bff) AM_RAM
ADDRESS_MAP_END


static WRITE_LINE_DEVICE_HANDLER(vdp_interrupt)
{
	cputag_set_input_line(device->machine(), "maincpu", 0, HOLD_LINE);
}

static TMS9928A_INTERFACE(kingpin_tms9928a_interface)
{
	"screen",
	0x4000,
	DEVCB_LINE(vdp_interrupt)
};

static MACHINE_CONFIG_START( kingpin, kingpin_state )
/*  MAIN CPU */
	MCFG_CPU_ADD("maincpu", Z80, 3579545)
	MCFG_CPU_PROGRAM_MAP(kingpin_program_map)
	MCFG_CPU_IO_MAP(kingpin_io_map)

/*  SOUND CPU */
	MCFG_CPU_ADD("audiocpu", Z80, 3579545)
	MCFG_CPU_PROGRAM_MAP(kingpin_sound_map)
	/*MCFG_CPU_IO_MAP(sound_io_map)*/

/*  VIDEO */
	MCFG_TMS9928A_ADD( "tms9928a", TMS9928A, kingpin_tms9928a_interface )
	MCFG_TMS9928A_SCREEN_ADD_NTSC( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9928a", tms9928a_device, screen_update )

	MCFG_NVRAM_ADD_0FILL("nvram")

/* Sound chip is a AY-3-8912 */
/*
    MCFG_SPEAKER_STANDARD_MONO("mono")

    MCFG_SOUND_ADD("aysnd", AY8912, 1500000)
    MCFG_SOUND_CONFIG(ay8912_interface)
    MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
*/
MACHINE_CONFIG_END

static DRIVER_INIT( kingpin )
{
	kingpin_state *state = machine.driver_data<kingpin_state>();

	/* Hacks to keep the emu a'runnin */
	state->m_code_base = machine.region("maincpu")->base();
	state->m_code_base[0x17d4] = 0xc3;	/* Maybe sound related? */
}

ROM_START( kingpin )
	ROM_REGION( 0xe000, "maincpu", 0 )
	ROM_LOAD( "1.u12", 0x0000, 0x2000, CRC(5ba9aca3) SHA1(480bfcf4d6223c00f50ff9ef9dc3b5a7a8a2982c) )
	ROM_LOAD( "2.u13", 0x2000, 0x2000, CRC(aedb5cc6) SHA1(7800d8d757180089d5ff4de0386bbb264b9f65e0) )
	ROM_LOAD( "3.u14", 0x4000, 0x2000, CRC(27849017) SHA1(60dd3d0448b5ee96df207c57644569dab630e3e6) )
	ROM_LOAD( "4.u15", 0x6000, 0x2000, CRC(1a483d5c) SHA1(b0775f70be7fff334fd7991d8852127739373b3b) )
	ROM_LOAD( "5.u16", 0x8000, 0x2000, CRC(70a52bcd) SHA1(9c72e501777d4d36933242276a5b0c4a01bc5543) )

	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD( "7.u22", 0x0000, 0x2000, CRC(077f533d) SHA1(74d0115b2cef5c35294ecb29771689b40ad1c25a) )

	ROM_REGION( 0x40, "user1", 0 )
	ROM_LOAD( "n82s123n.u29", 0x00, 0x20, CRC(ce8b1a6f) SHA1(9b8f564efa4efea867884970f4a5850d598bc7a7) )
	ROM_LOAD( "n82s123n.u43", 0x20, 0x20, CRC(55569a2a) SHA1(5b0482546161c9d14a7d2c719d40774539cb41ca) )
ROM_END

ROM_START( kingpinm )
	ROM_REGION( 0xe000, "maincpu", 0 )
	ROM_LOAD( "mdc0.u12", 0x0000, 0x2000, CRC(0a73dd98) SHA1(ef3e20ecae646c2eda7364f566f3841747f982a5) )
	ROM_LOAD( "mdc1.u13", 0x2000, 0x2000, CRC(18c2550c) SHA1(1466f7d9601c336b4c802821bd2ba0091c9ff143) )
	ROM_LOAD( "mdc2.u14", 0x4000, 0x2000, CRC(ae2dd544) SHA1(c1380be538e4e952fad30a1725b23eb7358889dd) )
	ROM_LOAD( "mdc3.u15", 0x6000, 0x2000, CRC(f9e178e5) SHA1(66a8dbe5dbe595c9a3e083fc8cb89aa66d5cdabc) )
	ROM_LOAD( "mdc4.u16", 0x8000, 0x2000, CRC(a6b364b8) SHA1(a5d782f2e89ec8770407b247306a69cdd90a1214) )
	ROM_LOAD( "mdc5.u17", 0xa000, 0x2000, CRC(1df82ad1) SHA1(03efe6fb5362a7488e325f1f7e35376e6b7455b2) )
	ROM_LOAD( "mdc6.u18", 0xc000, 0x2000, CRC(c59f8f92) SHA1(d95e38bec50f0e6522e4d75a50702e09aced3d1c) )

	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD( "7.u22", 0x0000, 0x2000, CRC(077f533d) SHA1(74d0115b2cef5c35294ecb29771689b40ad1c25a) )

	ROM_REGION( 0x40, "user1", 0 )
	ROM_LOAD( "n82s123n.u29", 0x00, 0x20, CRC(ce8b1a6f) SHA1(9b8f564efa4efea867884970f4a5850d598bc7a7) )
	ROM_LOAD( "n82s123n.u43", 0x20, 0x20, CRC(55569a2a) SHA1(5b0482546161c9d14a7d2c719d40774539cb41ca) )
ROM_END

GAME( 1983, kingpin,  0, kingpin, kingpin, kingpin, 0, "American Communication Laboratories Inc.", "King Pin",GAME_NO_SOUND)
GAME( 1983, kingpinm, 0, kingpin, kingpin, kingpin, 0, "American Communication Laboratories Inc.", "King Pin Multi-Game",GAME_NO_SOUND)

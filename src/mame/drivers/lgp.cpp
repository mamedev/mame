// license:BSD-3-Clause
// copyright-holders:Andrew Gardner
/*
Taito Laser Grand Prix laserdisc hardware
Driver by Andrew Gardner with help from Daphne Source

Notes:
    Engrish abounds.

Todo:
    Sound.
    Figure out palette.
    Figure out when NMI fires & hook it up.
    Convert to tilemaps - hook up control_ram properly.
    Plenty of missing writes.
    Figure out inputs:
        High score reset switch.
        Score display + time display + today's ranking (on LED panel).
        1 wheel + "pulse cam" (analog)
        1 accelerator pedal = pot (analog)
        1 break pedal = switch
        1 shifter (two settings - two switches)
        "VR-1" pot that says it "adjusts the video synchronisation"

Dumping Notes:
    Dumped by italiandoh on January, 27th, 2005.

    CPU PCB | J10 00041B, K10 00202B, M4200366B
    1 x Z80-A CPU
    1 x 8 MHz xtal
    2 x 6116
    4 x 2114-2
    2 x RCA CDP1855CDX 201
    3 x 8 dip switches bank
    1 x 1 kohm trimmer

    Chip    Label
    2764    A02_01, A02_02, A02_03, A02_04, A02_41
    2732    A02_05
    27128   A02_06, A02_07, A02_08, A02_09, A02_10, A02_11, A02_12, A02_13, A02_14, A02_15, A02_16, A02_17
    82S129  A02_31, A02_33, A02_34, A02_32

    --------------------------------

    OBJ PCB | J10 00042B, K10 00203B
    10 x TMS4416-15NL
    1 x 20 MHz xtal

    Chip    Label
    2764    A02_18
    27128   A02_19, A02_20, A02_21, A02_22, A02_23, A02_24, A02_25, A02_26, A02_27, A02_28
    82S123  A02_37
    82S129  A02_35, A02_36, A02_38, A02_39, A02_40

    --------------------------------

    SOUND PCB | J20 00002B, K20 00032B
    1 x Z80-A CPU
    1 x 6 MHx xtal
    2 x 2114-2
    3 x GI SOUND AY-3-8910

    Chip    Label
    2764    A02_29, A02_30
*/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "render.h"
#include "machine/ldv1000.h"


class lgp_state : public driver_device
{
public:
	lgp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_laserdisc(*this, "laserdisc") ,
		m_tile_ram(*this, "tile_ram"),
		m_tile_control_ram(*this, "tile_ctrl_ram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_device<pioneer_ldv1000_device> m_laserdisc;
	required_shared_ptr<UINT8> m_tile_ram;
	required_shared_ptr<UINT8> m_tile_control_ram;
	emu_timer *m_irq_timer;
	DECLARE_READ8_MEMBER(ldp_read);
	DECLARE_WRITE8_MEMBER(ldp_write);
	DECLARE_DRIVER_INIT(lgp);
	virtual void machine_start() override;
	UINT32 screen_update_lgp(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_callback_lgp);
	TIMER_CALLBACK_MEMBER(irq_stop);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


/* From italiandoh's notes */
#define CPU_PCB_CLOCK (8000000)
#define SOUND_PCB_CLOCK (6000000)


/* VIDEO GOODS */
UINT32 lgp_state::screen_update_lgp(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int charx, chary;

	/* make color 0 transparent */
	m_palette->set_pen_color(0, rgb_t(0,0,0,0));

	/* clear */
	bitmap.fill(0, cliprect);

	/* Draw tiles */
	for (charx = 0; charx < 32; charx++)
	{
		for (chary = 0; chary < 32; chary++)
		{
			int current_screen_character = (chary*32) + charx;

			/* Somewhere there's a flag that offsets the tilemap by 0x100*x */
			/* Palette is likely set somewhere as well (tile_control_ram?) */
			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
					m_tile_ram[current_screen_character],
					0,
					0, 0, charx*8, chary*8, 0);
		}
	}

	return 0;
}


/* MEMORY HANDLERS */
/* Main Z80 R/W */
READ8_MEMBER(lgp_state::ldp_read)
{
	return m_laserdisc->status_r();
}

WRITE8_MEMBER(lgp_state::ldp_write)
{
	m_laserdisc->data_w(data);
}


/* Sound Z80 R/W */


/* PROGRAM MAPS */
static ADDRESS_MAP_START( main_program_map, AS_PROGRAM, 8, lgp_state )
	AM_RANGE(0x0000,0x7fff) AM_ROM
	AM_RANGE(0xe000,0xe3ff) AM_RAM AM_SHARE("tile_ram")
	AM_RANGE(0xe400,0xe7ff) AM_RAM AM_SHARE("tile_ctrl_ram")

//  AM_RANGE(0xef00,0xef00) AM_READ_PORT("IN_TEST")
	AM_RANGE(0xef80,0xef80) AM_READWRITE(ldp_read,ldp_write)
	AM_RANGE(0xefb8,0xefb8) AM_READ(ldp_read)       /* Likely not right, calms it down though */
	AM_RANGE(0xefc0,0xefc0) AM_READ_PORT("DSWA")    /* Not tested */
	AM_RANGE(0xefc8,0xefc8) AM_READ_PORT("DSWB")
	AM_RANGE(0xefd0,0xefd0) AM_READ_PORT("DSWC")
	AM_RANGE(0xefd8,0xefd8) AM_READ_PORT("IN0")
	AM_RANGE(0xefe0,0xefe0) AM_READ_PORT("IN1")
	AM_RANGE(0xf000,0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_program_map, AS_PROGRAM, 8, lgp_state )
	AM_RANGE(0x0000,0x3fff) AM_ROM
	AM_RANGE(0x8000,0x83ff) AM_RAM
	AM_RANGE(0x8400,0x8407) AM_RAM      /* Needs handler!  Communications? */
	AM_RANGE(0x8800,0x8803) AM_RAM      /* Needs handler!  Communications? */
ADDRESS_MAP_END


/* IO MAPS */
static ADDRESS_MAP_START( main_io_map, AS_IO, 8, lgp_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
//  AM_RANGE(0xfd,0xfd) AM_READ_PORT("IN_TEST")
//  AM_RANGE(0xfe,0xfe) AM_READ_PORT("IN_TEST")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_io_map, AS_IO, 8, lgp_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END


/* PORTS */
/*  (DIPLOCATION diplay inverted) */
static INPUT_PORTS_START( lgp )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x03, "1st Round" ) PORT_DIPLOCATION("SWA:2,1")
	PORT_DIPSETTING(    0x03, "68 Seconds" )
	PORT_DIPSETTING(    0x02, "66 Seconds" )
	PORT_DIPSETTING(    0x01, "64 Seconds" )
	PORT_DIPSETTING(    0x00, "62 Seconds" )
	PORT_DIPNAME( 0x0c, 0x0c, "2nd Round" ) PORT_DIPLOCATION("SWA:4,3")
	PORT_DIPSETTING(    0x0c, "62 Seconds" )
	PORT_DIPSETTING(    0x08, "60 Seconds" )
	PORT_DIPSETTING(    0x04, "58 Seconds" )
	PORT_DIPSETTING(    0x00, "56 Seconds" )
	PORT_DIPNAME( 0x30, 0x30, "3rd & 4th Rounds" ) PORT_DIPLOCATION("SWA:6,5")
	PORT_DIPSETTING(    0x30, "60 Seconds" )
	PORT_DIPSETTING(    0x20, "58 Seconds" )
	PORT_DIPSETTING(    0x10, "56 Seconds" )
	PORT_DIPSETTING(    0x00, "54 Seconds" )
	PORT_DIPNAME( 0xc0, 0xc0, "Spark Race" ) PORT_DIPLOCATION("SWA:8,7")
	PORT_DIPSETTING(    0xc0, "44 Seconds" )
	PORT_DIPSETTING(    0x80, "42 Seconds" )
	PORT_DIPSETTING(    0x40, "40 Seconds" )
	PORT_DIPSETTING(    0x00, "38 Seconds" )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SWB:4,3,2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SWB:8,7,6,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_8C ) )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SWC:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("SWC:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("SWC:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Coinage Display" ) PORT_DIPLOCATION("SWC:4")
	PORT_DIPSETTING(    0x00, "With" )
	PORT_DIPSETTING(    0x08, "Without" )
	PORT_DIPNAME( 0x10, 0x00, "Year Display" ) PORT_DIPLOCATION("SWC:5")
	PORT_DIPSETTING(    0x00, "With" )
	PORT_DIPSETTING(    0x10, "Without" )
	PORT_DIPNAME( 0x20, 0x00, "Hit Detection" ) PORT_DIPLOCATION("SWC:6")
	PORT_DIPSETTING(    0x00, "Normal Game" )
	PORT_DIPSETTING(    0x20, "No Hit" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) ) PORT_DIPLOCATION("SWC:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Coin Slot" ) PORT_DIPLOCATION("SWC:8")
	PORT_DIPSETTING(    0x00, "1-Way" )
	PORT_DIPSETTING(    0x80, "2-Ways" )


	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE )   /* Manual says service switch simply increases credit count. */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON5 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON6 )

/*
    PORT_START("IN_TEST")
    PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
    PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 )
    PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 )
    PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 )
    PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 )
    PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 )
    PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 )
    PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON8 )
*/
INPUT_PORTS_END

static const gfx_layout lgp_gfx_layout =
{
	8,8,
	0x4000/8,
	4,
	{ 0, 0x4000*8, 0x8000*8, 0xc000*8 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

/* Silly, but it's here to show off the oddball a02_18 */
static const gfx_layout lgp_gfx_layout_16x32 =
{
	16,32,
	0x2000/0x200,
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	{ 0*128, 1*128, 2*128, 3*128, 4*128, 5*128, 6*128, 7*128,
		8*128, 9*128,10*128,11*128,12*128,13*128,14*128,15*128,
		16*128,17*128,18*128,19*128,20*128,21*128,22*128,23*128,
		24*128,25*128,26*128,27*128,28*128,29*128,30*128,31*128},
	32*128
};

static GFXDECODE_START( lgp )
	GFXDECODE_ENTRY("gfx1", 0, lgp_gfx_layout, 0x0, 0x100)
	GFXDECODE_ENTRY("gfx4", 0, lgp_gfx_layout_16x32, 0x0, 0x100)
GFXDECODE_END

TIMER_CALLBACK_MEMBER(lgp_state::irq_stop)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

INTERRUPT_GEN_MEMBER(lgp_state::vblank_callback_lgp)
{
	// NMI
	//device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);

	// IRQ
	device.execute().set_input_line(0, ASSERT_LINE);
	m_irq_timer->adjust(attotime::from_usec(50));
}


void lgp_state::machine_start()
{
	m_irq_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(lgp_state::irq_stop),this));
}


/* DRIVER */
static MACHINE_CONFIG_START( lgp, lgp_state )
	/* main cpu */
	MCFG_CPU_ADD("maincpu", Z80, CPU_PCB_CLOCK)
	MCFG_CPU_PROGRAM_MAP(main_program_map)
	MCFG_CPU_IO_MAP(main_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", lgp_state,  vblank_callback_lgp)

	/* sound cpu */
	MCFG_CPU_ADD("audiocpu", Z80, SOUND_PCB_CLOCK)
	MCFG_CPU_PROGRAM_MAP(sound_program_map)
	MCFG_CPU_IO_MAP(sound_io_map)


	MCFG_LASERDISC_LDV1000_ADD("laserdisc")
	MCFG_LASERDISC_OVERLAY_DRIVER(256, 256, lgp_state, screen_update_lgp)
	MCFG_LASERDISC_OVERLAY_PALETTE("palette")

	/* video hardware */
	MCFG_LASERDISC_SCREEN_ADD_NTSC("screen", "laserdisc")

	MCFG_PALETTE_ADD("palette", 256)
	/* MCFG_PALETTE_INIT_OWNER(lgp_state,lgp) */

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", lgp)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_MODIFY("laserdisc")
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END


ROM_START( lgp )
	/* CPU PCB */
	/* Main program */
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "a02_01.63", 0x0000, 0x2000, CRC(088ca6e1) SHA1(b3f0869b0c333d991363ac46a1c53daa3f6c85e9) )
	ROM_LOAD( "a02_02.62", 0x2000, 0x2000, CRC(8e1be578) SHA1(cfad7cb72c7d13b2b614680bccc6f807521f3bf4) )
	ROM_LOAD( "a02_03.61", 0x4000, 0x2000, CRC(4978953a) SHA1(eec6596430238ffffb0d173852bdd7f11c60e9b2) )
	ROM_LOAD( "a02_04.60", 0x6000, 0x2000, CRC(903d0ae2) SHA1(bd04182dd77eb5a4ef28d7127f631689b6695e17) )

	/* Tiles */
	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "a02_14.13", 0x0000, 0x4000, CRC(28996b3c) SHA1(664aa4d2582784bb7c56cfb8883294578fe18dd9) )
	ROM_LOAD( "a02_15.12", 0x4000, 0x4000, CRC(6b7abbb2) SHA1(890777d99b646147359d09e42692a272e570113c) )
	ROM_LOAD( "a02_16.11", 0x8000, 0x4000, CRC(aad4fb3c) SHA1(2cd4a331a8e0f010d60b594655361776fa24255f) )
	ROM_LOAD( "a02_17.10", 0xc000, 0x4000, CRC(db8b9723) SHA1(1319a3f35dffdcf65fe7496cfabaf5787989d0bc) )

	/* ??? */
	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "a02_06.53", 0x0000, 0x4000, CRC(c724f96c) SHA1(6531bd57c87118479700e5f8708060deb6b21d91) )
	ROM_LOAD( "a02_07.52", 0x4000, 0x4000, CRC(4b81eb3b) SHA1(76f5c0b2b4d450fd633199ac4aba66ed5a8a530b) )
	ROM_LOAD( "a02_08.51", 0x8000, 0x4000, CRC(deb7e494) SHA1(cc5b2f9c622ec3b599fe6752233d44a820951a98) )
	ROM_LOAD( "a02_09.50", 0xc000, 0x4000, CRC(6d077a30) SHA1(00888e0f54a5b4d647921caf37f7b3e4a7934a1f) )

	/* ??? */
	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "a02_10.49", 0x0000, 0x4000, CRC(56bbe961) SHA1(97ebaeb3bc3948724e19a7279e13314a3470e5ef) )
	ROM_LOAD( "a02_11.48", 0x4000, 0x4000, CRC(5d0aebb2) SHA1(4cdfb82fa459c9d168a79b5a643fdde45906872b) )
	ROM_LOAD( "a02_12.47", 0x8000, 0x4000, CRC(1b7c578c) SHA1(6bf2d6e908176e5ef83649b34df9976a55319176) )
	ROM_LOAD( "a02_13.46", 0xc000, 0x4000, CRC(d7fccfb1) SHA1(9b8a2813b98eb00c2adc347a902a40e7a462070d) )

	/* Lame lookup table? */
	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD( "a02_05.54", 0x0000, 0x1000, CRC(f5e06a8b) SHA1(d2659bc185ecb66f9ac8e015a60259efd13ba84a) )

	/* Color ROMs? */
	ROM_REGION( 0x400, "user2", 0 )
	ROM_LOAD( "a02_31.106", 0x000, 0x100, CRC(ff586bfd) SHA1(61631bae9ba87f2ced142fb8907b313e5424374f) )
	ROM_LOAD( "a02_32.111", 0x100, 0x100, CRC(8e00230e) SHA1(5dc1d01c0c3e34cdfcde81227c48a7293115d257) )
	ROM_LOAD( "a02_33.116", 0x200, 0x100, CRC(507fb884) SHA1(7bae761d69dfb035b71b4650f226df4b3d0df67d) )
	ROM_LOAD( "a02_34.122", 0x300, 0x100, CRC(fb2e6898) SHA1(a78e45b015edbe91912f8bf915761daf683126a7) )

	/* Nearly-unused ROM */
	ROM_REGION( 0x2000, "user3", 0 )
	ROM_LOAD( "a02_41.59", 0x0000, 0x2000, CRC(2c55a3c0) SHA1(7e25217ed8e65549eb0043a5d2dc83e0ffea1177) )


	/* SOUND PCB */
	/* Sound CPU - on Sound PCB */
	ROM_REGION( 0x4000, "audiocpu", 0 )
	ROM_LOAD( "a02_29.ic11", 0x0000, 0x2000, CRC(c44026db) SHA1(93a6e8f272ca826c05a7be59e14a1a0c848fbaa0) )
	ROM_LOAD( "a02_30.ic17", 0x2000, 0x2000, CRC(8c324556) SHA1(9e1f6f00d4023d9cfd414d3cc02af55be49dde2c) ) /* Sound data? */


	/* OBJ PCB */
	/* Zig-Zag bitmaps? */
	ROM_REGION( 0x2000, "gfx4", 0 )
	ROM_LOAD( "a02_18.143", 0x0000, 0x2000, CRC(1b4e1980) SHA1(9dffb6a047427290ad63e3d7df7c5942c2b2dfc1) )

	/* Misc bitmaps? */
	ROM_REGION( 0x28000, "gfx5", 0 )
	ROM_LOAD( "a02_19.140", 0x00000, 0x4000, CRC(eedd3167) SHA1(a17976425b2e208485bcb189ef2e69dd709a9957) )
	ROM_LOAD( "a02_20.139", 0x04000, 0x4000, CRC(5182f87b) SHA1(13d466c18fe5cc5da8e931130ee7defd3a9ebf12) )
	ROM_LOAD( "a02_21.138", 0x08000, 0x4000, CRC(ca16a6e3) SHA1(a82396ea51fa6a320e501d86a23e1e1ebe8d29d5) )
	ROM_LOAD( "a02_22.137", 0x0c000, 0x4000, CRC(479b3d95) SHA1(b082d4a5e11040daa1f1a687233776fe95a2c77d) )
	ROM_LOAD( "a02_23.136", 0x10000, 0x4000, CRC(59ee0aa6) SHA1(cac307ef417d97f13ba3051d886eadf5ea10fb0d) )
	ROM_LOAD( "a02_24.118", 0x14000, 0x4000, CRC(04564330) SHA1(579a0e964fd4ca83f05bd20534fa6f1a91b9c355) )
	ROM_LOAD( "a02_25.117", 0x18000, 0x4000, CRC(57c2377a) SHA1(70442b4811fa319cb97fb9d8f35ebc5603cf4942) )
	ROM_LOAD( "a02_26.116", 0x1c000, 0x4000, CRC(e2dc72fc) SHA1(4168f8df8124d9c83bec0abf72955df76c000ed8) )
	ROM_LOAD( "a02_27.115", 0x20000, 0x4000, CRC(9a9e6b3f) SHA1(c9a1a24d7a93929379a0c5d4d2f5df1da0136348) )
	ROM_LOAD( "a02_28.114", 0x24000, 0x4000, CRC(cd69ed20) SHA1(d60782637085491527814889856eb3553950ab55) )

	/* Small ROM dumping ground - color? */
	ROM_REGION( 0x520, "user4", 0 )
	ROM_LOAD( "a02_35.23",  0x00000, 0x100, CRC(7b9d44f1) SHA1(bbd7c35a03ca6de116a01f6dcfa2ecd13a7ddb53) )
	ROM_LOAD( "a02_36.24",  0x00100, 0x100, CRC(169c4216) SHA1(23921e9ef61a68fdd8afceb3b95bbac48190cf1a) )
	ROM_LOAD( "a02_37.43",  0x00200, 0x20,  CRC(925ba961) SHA1(6715d80f2346374a0e880cf44cadc36e4a5316ed) )
	ROM_LOAD( "a02_38.44",  0x00220, 0x100, CRC(6f37212a) SHA1(32b891dc9b97637620b2f1f9d9d76509c333cb2d) )
	ROM_LOAD( "a02_39.109", 0x00320, 0x100, CRC(88363809) SHA1(b22a7bd8ce6b28bf7cfa64c3a08e4cf7f9b4cd20) )
	ROM_LOAD( "a02_40.110", 0x00420, 0x100, CRC(fdfc7aac) SHA1(2413f7f9ad11c91d2adc0aab37bf70ff5c68ab6f) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "lgp", 0, NO_DUMP )
ROM_END

ROM_START( lgpalt )
	/* CPU PCB */
	/* Main program */
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "a02_01a.bin", 0x0000, 0x2000, CRC(09037833) SHA1(3bd892952af3d6aeb7eae31c30a2b654732641ef) )
	ROM_LOAD( "a02_02a.bin", 0x2000, 0x2000, CRC(536faed2) SHA1(8ea4b6617fb2c77f7c374fc15141c16125a23b30) )
	ROM_LOAD( "a02_03a.bin", 0x4000, 0x2000, CRC(679b90c1) SHA1(bc8c243cfd6e3d1660db0a47ebe4ba2aa4663a75) )
	ROM_LOAD( "a02_04a.bin", 0x6000, 0x2000, CRC(b2936139) SHA1(b9eebc55a32cc345b19607e389ef7911af998677) )

	/* Tiles */
	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "a02_14a.bin", 0x0000, 0x4000, CRC(025c397a) SHA1(9e2c1167ca717988203e25691a5d9b7a59ae3671) )
	ROM_LOAD( "a02_15a.bin", 0x4000, 0x4000, CRC(e5b77360) SHA1(911983886ef6835744c49a50028a95aba9de46fe) )
	ROM_LOAD( "a02_16a.bin", 0x8000, 0x4000, CRC(31412e6b) SHA1(50096cfe14ffa77bd1131737e586afb2b571ff9c) )
	ROM_LOAD( "a02_17a.bin", 0xc000, 0x4000, CRC(e9a0a4bd) SHA1(732f2d743e3d066e1067a51656498130f38766dc) )

	/* ??? */
	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "a02_06.53", 0x0000, 0x4000, CRC(c724f96c) SHA1(6531bd57c87118479700e5f8708060deb6b21d91) )
	ROM_LOAD( "a02_07.52", 0x4000, 0x4000, CRC(4b81eb3b) SHA1(76f5c0b2b4d450fd633199ac4aba66ed5a8a530b) )
	ROM_LOAD( "a02_08.51", 0x8000, 0x4000, CRC(deb7e494) SHA1(cc5b2f9c622ec3b599fe6752233d44a820951a98) )
	ROM_LOAD( "a02_09.50", 0xc000, 0x4000, CRC(6d077a30) SHA1(00888e0f54a5b4d647921caf37f7b3e4a7934a1f) )

	/* ??? */
	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "a02_10.49", 0x0000, 0x4000, CRC(56bbe961) SHA1(97ebaeb3bc3948724e19a7279e13314a3470e5ef) )
	ROM_LOAD( "a02_11.48", 0x4000, 0x4000, CRC(5d0aebb2) SHA1(4cdfb82fa459c9d168a79b5a643fdde45906872b) )
	ROM_LOAD( "a02_12.47", 0x8000, 0x4000, CRC(1b7c578c) SHA1(6bf2d6e908176e5ef83649b34df9976a55319176) )
	ROM_LOAD( "a02_13.46", 0xc000, 0x4000, CRC(d7fccfb1) SHA1(9b8a2813b98eb00c2adc347a902a40e7a462070d) )

	/* Lame lookup table? */
	ROM_REGION( 0x1000, "user1", 0 )
	ROM_LOAD( "a02_05.54", 0x0000, 0x1000, CRC(f5e06a8b) SHA1(d2659bc185ecb66f9ac8e015a60259efd13ba84a) )

	/* Color ROMs? */
	ROM_REGION( 0x400, "user2", 0 )
	ROM_LOAD( "a02_31.106", 0x000, 0x100, CRC(ff586bfd) SHA1(61631bae9ba87f2ced142fb8907b313e5424374f) )
	ROM_LOAD( "a02_32.111", 0x100, 0x100, CRC(8e00230e) SHA1(5dc1d01c0c3e34cdfcde81227c48a7293115d257) )
	ROM_LOAD( "a02_33.116", 0x200, 0x100, CRC(507fb884) SHA1(7bae761d69dfb035b71b4650f226df4b3d0df67d) )
	ROM_LOAD( "a02_34.122", 0x300, 0x100, CRC(fb2e6898) SHA1(a78e45b015edbe91912f8bf915761daf683126a7) )

	/* Nearly-unused ROM */
	ROM_REGION( 0x2000, "user3", 0 )
	ROM_LOAD( "a02_41a.bin", 0x0000, 0x2000, CRC(1e3f608c) SHA1(0e4b75c6cea6d7a65f25eb86915f7e03a58c8cf6) )


	/* SOUND PCB */
	/* Sound CPU - on Sound PCB */
	ROM_REGION( 0x4000, "audiocpu", 0 )
	ROM_LOAD( "a02_29.ic11", 0x0000, 0x2000, CRC(c44026db) SHA1(93a6e8f272ca826c05a7be59e14a1a0c848fbaa0) )
	ROM_LOAD( "a02_30.ic17", 0x2000, 0x2000, CRC(8c324556) SHA1(9e1f6f00d4023d9cfd414d3cc02af55be49dde2c) ) /* Sound data? */


	/* OBJ PCB */
	/* Zig-Zag bitmaps? */
	ROM_REGION( 0x2000, "gfx4", 0 )
	ROM_LOAD( "a02_18.143", 0x0000, 0x2000, CRC(1b4e1980) SHA1(9dffb6a047427290ad63e3d7df7c5942c2b2dfc1) )

	/* Misc bitmaps? */
	ROM_REGION( 0x28000, "gfx5", 0 )
	ROM_LOAD( "a02_19.140", 0x00000, 0x4000, CRC(eedd3167) SHA1(a17976425b2e208485bcb189ef2e69dd709a9957) )
	ROM_LOAD( "a02_20.139", 0x04000, 0x4000, CRC(5182f87b) SHA1(13d466c18fe5cc5da8e931130ee7defd3a9ebf12) )
	ROM_LOAD( "a02_21.138", 0x08000, 0x4000, CRC(ca16a6e3) SHA1(a82396ea51fa6a320e501d86a23e1e1ebe8d29d5) )
	ROM_LOAD( "a02_22.137", 0x0c000, 0x4000, CRC(479b3d95) SHA1(b082d4a5e11040daa1f1a687233776fe95a2c77d) )
	ROM_LOAD( "a02_23.136", 0x10000, 0x4000, CRC(59ee0aa6) SHA1(cac307ef417d97f13ba3051d886eadf5ea10fb0d) )
	ROM_LOAD( "a02_24.118", 0x14000, 0x4000, CRC(04564330) SHA1(579a0e964fd4ca83f05bd20534fa6f1a91b9c355) )
	ROM_LOAD( "a02_25.117", 0x18000, 0x4000, CRC(57c2377a) SHA1(70442b4811fa319cb97fb9d8f35ebc5603cf4942) )
	ROM_LOAD( "a02_26.116", 0x1c000, 0x4000, CRC(e2dc72fc) SHA1(4168f8df8124d9c83bec0abf72955df76c000ed8) )
	ROM_LOAD( "a02_27.115", 0x20000, 0x4000, CRC(9a9e6b3f) SHA1(c9a1a24d7a93929379a0c5d4d2f5df1da0136348) )
	ROM_LOAD( "a02_28.114", 0x24000, 0x4000, CRC(cd69ed20) SHA1(d60782637085491527814889856eb3553950ab55) )

	/* Small ROM dumping ground - color? */
	ROM_REGION( 0x520, "user4", 0 )
	ROM_LOAD( "a02_35.23",  0x00000, 0x100, CRC(7b9d44f1) SHA1(bbd7c35a03ca6de116a01f6dcfa2ecd13a7ddb53) )
	ROM_LOAD( "a02_36.24",  0x00100, 0x100, CRC(169c4216) SHA1(23921e9ef61a68fdd8afceb3b95bbac48190cf1a) )
	ROM_LOAD( "a02_37.43",  0x00200, 0x20,  CRC(925ba961) SHA1(6715d80f2346374a0e880cf44cadc36e4a5316ed) )
	ROM_LOAD( "a02_38.44",  0x00220, 0x100, CRC(6f37212a) SHA1(32b891dc9b97637620b2f1f9d9d76509c333cb2d) )
	ROM_LOAD( "a02_39.109", 0x00320, 0x100, CRC(88363809) SHA1(b22a7bd8ce6b28bf7cfa64c3a08e4cf7f9b4cd20) )
	ROM_LOAD( "a02_40.110", 0x00420, 0x100, CRC(fdfc7aac) SHA1(2413f7f9ad11c91d2adc0aab37bf70ff5c68ab6f) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "lgp", 0, NO_DUMP )
ROM_END

DRIVER_INIT_MEMBER(lgp_state,lgp)
{
}

/*    YEAR  NAME PARENT   MACHINE INPUT INIT MONITOR  COMPANY   FULLNAME             FLAGS) */
GAME( 1983, lgp, 0,       lgp,    lgp, lgp_state,  lgp, ROT0,    "Taito",  "Laser Grand Prix",  MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
GAME( 1983, lgpalt, lgp,  lgp,    lgp, lgp_state,  lgp, ROT0,    "Taito",  "Laser Grand Prix (alternate)",  MACHINE_NOT_WORKING|MACHINE_NO_SOUND)

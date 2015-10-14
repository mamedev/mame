// license:BSD-3-Clause
// copyright-holders:David Haywood, Farfetch'd
/*** DRIVER INFO & NOTES ******************************************************
 Speed Spin (c)1994 TCH
  driver by David Haywood & Farfetch'd

Notes:
- To enter "easy" service mode, keep some input pressed during boot,
  e.g. BUTTON 1.

TODO:
- Unknown Port Writes:
  cpu #0 (PC=00000D88): unmapped port byte write to 00000001 = 02
  cpu #0 (PC=00006974): unmapped port byte write to 00000010 = 10
  cpu #0 (PC=00006902): unmapped port byte write to 00000010 = 20
  etc.
- Writes to ROM regions
  cpu #0 (PC=00001119): byte write to ROM 0000C8B9 = 01
  cpu #0 (PC=00001119): byte write to ROM 0000C899 = 01
  etc.

******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/okim6295.h"
#include "includes/speedspn.h"

/*** README INFO **************************************************************

ROMSET: speedspn

Speed Spin
1994, TCH

PCB No: PR02/2
CPU   : Z80 (6Mhz)
SOUND : Z80 (6Mhz), OKI M6295
RAM   : 62256 (x1), 6264 (x1), 6116 (x6)
XTAL  : 12.000MHz (near Z80's), 10.000MHz (near PLCC84)
DIP   : 8 position (x2)
OTHER : TPC1020AFN-084C (PLCC84, Gfx controller)

ROMs          Type    Used            C'sum
-------------------------------------------
TCH-SS1.u78   27C040  Main Program    C246h
TCH-SS2.u96   27C512  Sound Program   5D04h
TCH-SS3.u95   27C040  Oki Samples     7340h
TCH-SS4.u70   27C010  \               ECD8h
TCH-SS5.u69     "     |               9E7Bh
TCH-SS6.u60     "     |               E844h
TCH-SS7.u59     "     |  Gfx          3DDah
TCH-SS8.u39     "     |               A9B5h
TCH-SS9.u34     "     /               AB2Bh


******************************************************************************/

READ8_MEMBER(speedspn_state::irq_ack_r)
{
	// I think this simply acknowledges the IRQ #0, it's read within the handler and the
	//  value is discarded
	return 0;
}

WRITE8_MEMBER(speedspn_state::rombank_w)
{
	if (data > 8)
	{
		popmessage ("Unmapped Bank Write %02x", data);
		data = 0;
	}
	m_prgbank->set_entry(data);
}

/*** SOUND RELATED ***********************************************************/

WRITE8_MEMBER(speedspn_state::sound_w)
{
	soundlatch_byte_w(space, 1, data);
	m_audiocpu->set_input_line(0, HOLD_LINE);
}

WRITE8_MEMBER(speedspn_state::okibank_w)
{
	m_okibank->set_entry(data & 3);
}

/*** MEMORY MAPS *************************************************************/

/* main cpu */

static ADDRESS_MAP_START( program_map, AS_PROGRAM, 8, speedspn_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette") /* RAM COLOUR */
	AM_RANGE(0x8800, 0x8fff) AM_RAM_WRITE(attram_w) AM_SHARE("attram")
	AM_RANGE(0x9000, 0x9fff) AM_READWRITE(vidram_r, vidram_w)  /* RAM FIX / RAM OBJECTS (selected by bit 0 of port 17) */
	AM_RANGE(0xa000, 0xa7ff) AM_RAM
	AM_RANGE(0xa800, 0xafff) AM_RAM
	AM_RANGE(0xb000, 0xbfff) AM_RAM                                             /* RAM PROGRAM */
	AM_RANGE(0xc000, 0xffff) AM_ROMBANK("prgbank")                              /* banked ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, speedspn_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x07, 0x07) AM_WRITE(display_disable_w)
	AM_RANGE(0x10, 0x10) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x11, 0x11) AM_READ_PORT("P1")
	AM_RANGE(0x12, 0x12) AM_READ_PORT("P2") AM_WRITE(rombank_w)
	AM_RANGE(0x13, 0x13) AM_READ_PORT("DSW1") AM_WRITE(sound_w)
	AM_RANGE(0x14, 0x14) AM_READ_PORT("DSW2")
	AM_RANGE(0x16, 0x16) AM_READ(irq_ack_r) // @@@ could be watchdog, value is discarded
	AM_RANGE(0x17, 0x17) AM_WRITE(vidram_bank_w)
ADDRESS_MAP_END

/* sound cpu */

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, speedspn_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0x9000, 0x9000) AM_WRITE(okibank_w)
	AM_RANGE(0x9800, 0x9800) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( oki_map, AS_0, 8, speedspn_state )
	AM_RANGE(0x00000, 0x1ffff) AM_ROM
	AM_RANGE(0x20000, 0x3ffff) AM_ROMBANK("okibank")
ADDRESS_MAP_END

/*** INPUT PORT **************************************************************/

static INPUT_PORTS_START( speedspn )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:8,7,6,5")
	PORT_DIPSETTING(    0x01, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x00, "5 Coins/2 Credits" )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x05, "3 Coins/5 Credits" )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:4,3,2,1")
	PORT_DIPSETTING(    0x10, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x00, "5 Coins/2 Credits" )
	PORT_DIPSETTING(    0xa0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x50, "3 Coins/5 Credits" )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "World Cup" )         PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Backhand" )          PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x02, "Automatic" )
	PORT_DIPSETTING(    0x00, "Manual" )
	PORT_DIPNAME( 0x0c, 0x0c, "Points to Win" )     PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(    0x0c, "11 Points and 1 Advantage" )
	PORT_DIPSETTING(    0x08, "11 Points and 2 Advantage" )
	PORT_DIPSETTING(    0x04, "21 Points and 1 Advantage" )
	PORT_DIPSETTING(    0x00, "21 Points and 2 Advantage" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x080, IP_ACTIVE_LOW, "SW2:1" )
INPUT_PORTS_END

/*** GFX DECODE **************************************************************/

static const gfx_layout speedspn_charlayout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(2,4), RGN_FRAC(3,4), RGN_FRAC(0,4), RGN_FRAC(1,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	8*8
};

static const gfx_layout speedspn_spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 4, 0, RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0 },
	{ 16*16+11, 16*16+10, 16*16+9, 16*16+8, 16*16+3, 16*16+2, 16*16+1, 16*16+0,
			11,       10,       9,       8,       3,       2,       1,       0  },
	{ 8*16+7*16, 8*16+6*16, 8*16+5*16, 8*16+4*16, 8*16+3*16, 8*16+2*16, 8*16+1*16, 8*16+0*16,
			7*16,      6*16,      5*16,      4*16,      3*16,      2*16,      1*16,      0*16  },
	32*16
};


static GFXDECODE_START( speedspn )
	GFXDECODE_ENTRY( "gfx1", 0, speedspn_charlayout,   0x000, 0x40 )
	GFXDECODE_ENTRY( "gfx2", 0, speedspn_spritelayout, 0x000, 0x40 )
GFXDECODE_END

/*** MACHINE DRIVER **********************************************************/

void speedspn_state::machine_start()
{
	/* is this weird banking some form of protection? */
	UINT8 *rom = memregion("maincpu")->base();
	m_prgbank->configure_entry(0, &rom[0x28000]);
	m_prgbank->configure_entry(1, &rom[0x14000]);
	m_prgbank->configure_entry(2, &rom[0x1c000]);
	m_prgbank->configure_entry(3, &rom[0x54000]);
	m_prgbank->configure_entry(4, &rom[0x48000]);
	m_prgbank->configure_entry(5, &rom[0x3c000]);
	m_prgbank->configure_entry(6, &rom[0x18000]);
	m_prgbank->configure_entry(7, &rom[0x4c000]);
	m_prgbank->configure_entry(8, &rom[0x50000]);
	m_prgbank->set_entry(0);

	m_okibank->configure_entries(0, 4, memregion("oki")->base(), 0x20000);
	m_okibank->set_entry(0);
}


static MACHINE_CONFIG_START( speedspn, speedspn_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80,6000000)      /* 6 MHz */
	MCFG_CPU_PROGRAM_MAP(program_map)
	MCFG_CPU_IO_MAP(io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", speedspn_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80,6000000)        /* 6 MHz */
	MCFG_CPU_PROGRAM_MAP(sound_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(8*8, 56*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(speedspn_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", speedspn)
	MCFG_PALETTE_ADD("palette", 0x400)
	MCFG_PALETTE_FORMAT(xxxxRRRRGGGGBBBB)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1122000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, oki_map)
MACHINE_CONFIG_END

/*** ROM LOADING *************************************************************/

ROM_START( speedspn )
	ROM_REGION( 0x080000, "maincpu", 0 )    /* CPU1 code */
	/* most of this is probably actually banked */
	ROM_LOAD( "tch-ss1.u78", 0x00000, 0x080000, CRC(41b6b45b) SHA1(d969119959db4cc3be50f188bfa41e4b4896eaca) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* CPU2 code */
	ROM_LOAD( "tch-ss2.u96", 0x00000, 0x10000, CRC(4611fd0c) SHA1(b49ad6a8be6ccfef0b2ed187fb3b008fb7eeb2b5) ) // FIRST AND SECOND HALF IDENTICAL

	ROM_REGION( 0x080000, "oki", 0 )  /* Samples */
	ROM_LOAD( "tch-ss3.u95", 0x00000, 0x080000, CRC(1c9deb5e) SHA1(89f01a8e8bdb0eee47e9195b312d2e65d41d3548) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT ) /* GFX */
	ROM_LOAD( "tch-ss4.u70", 0x00000, 0x020000, CRC(41517859) SHA1(3c5102e41c5a70e02ed88ea43ca63edf13f4c1b9) )
	ROM_LOAD( "tch-ss5.u69", 0x20000, 0x020000, CRC(832b2f34) SHA1(7a3060869a9698c9ed4187b239a70e273de64e3c) )
	ROM_LOAD( "tch-ss6.u60", 0x40000, 0x020000, CRC(f1fd7289) SHA1(8950ef58efdffc45d68152257ca36aedf5ddf677) )
	ROM_LOAD( "tch-ss7.u59", 0x60000, 0x020000, CRC(c4958543) SHA1(c959b440801707c30a8968a1f44abe5442d03eff) )

	ROM_REGION( 0x40000, "gfx2", ROMREGION_INVERT ) /* GFX */
	ROM_LOAD( "tch-ss8.u39", 0x00000, 0x020000, CRC(2f27b16d) SHA1(7cc017fa08573f8a9d94d017abb987f8288bcd29) )
	ROM_LOAD( "tch-ss9.u34", 0x20000, 0x020000, CRC(c372f8ec) SHA1(514bef0859c0adfd9cdd22864230fc83e9b1962d) )
ROM_END



/*** GAME DRIVERS ************************************************************/

GAME( 1994, speedspn, 0, speedspn, speedspn, driver_device, 0, ROT180, "TCH", "Speed Spin", MACHINE_SUPPORTS_SAVE )

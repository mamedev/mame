// license:BSD-3-Clause
// copyright-holders:Carlos A. Lozano, Uki
/****************************************************************************

Formation Z / Aeroboto

PCB ID: JALECO FZ-8420

Driver by Carlos A. Lozano

TODO:
- star field
  Uki's report:
  - The color of stars:
    at 1st title screen = neutral tints of blue and aqua (1 color only)
    at 2nd title screen and attract mode (purple surface) = light & dark aqua
    This color will not be affected by scroll. Leftmost 8pixels are light, next
    16 pixels are dark, the next 16 pixels are light, and so on.

Revisions:
- Updated starfield according to Uki's report. (AT)

*note: Holding any key at boot puts the game in MCU test. Press F3 to quit.

****************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "sound/ay8910.h"
#include "includes/aeroboto.h"


READ8_MEMBER(aeroboto_state::aeroboto_201_r)
{
	/* if you keep a button pressed during boot, the game will expect this */
	/* serie of values to be returned from 3004, and display "PASS 201" if it is */
	static const UINT8 res[4] = { 0xff, 0x9f, 0x1b, 0x03 };

	logerror("PC %04x: read 3004\n", space.device().safe_pc());
	return res[(m_count++) & 3];
}


INTERRUPT_GEN_MEMBER(aeroboto_state::aeroboto_interrupt)
{
	if (!m_disable_irq)
		device.execute().set_input_line(0, ASSERT_LINE);
	else
		m_disable_irq--;
}

READ8_MEMBER(aeroboto_state::aeroboto_irq_ack_r)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	return 0xff;
}

READ8_MEMBER(aeroboto_state::aeroboto_2973_r)
{
	m_mainram[0x02be] = 0;
	return 0xff;
}

WRITE8_MEMBER(aeroboto_state::aeroboto_1a2_w)
{
	m_mainram[0x01a2] = data;
	if (data)
		m_disable_irq = 1;
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, aeroboto_state )
	AM_RANGE(0x01a2, 0x01a2) AM_WRITE(aeroboto_1a2_w)           // affects IRQ line (more protection?)
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("mainram") // main  RAM
	AM_RANGE(0x0800, 0x08ff) AM_RAM                             // tile color buffer; copied to 0x2000
	AM_RANGE(0x0900, 0x09ff) AM_WRITEONLY                       // a backup of default tile colors
	AM_RANGE(0x1000, 0x17ff) AM_RAM_WRITE(aeroboto_videoram_w) AM_SHARE("videoram")     // tile RAM
	AM_RANGE(0x1800, 0x183f) AM_RAM AM_SHARE("hscroll") // horizontal scroll regs
	AM_RANGE(0x2000, 0x20ff) AM_RAM_WRITE(aeroboto_tilecolor_w) AM_SHARE("tilecolor")   // tile color RAM
	AM_RANGE(0x1840, 0x27ff) AM_WRITENOP                    // cleared during custom LSI test
	AM_RANGE(0x2800, 0x28ff) AM_RAM AM_SHARE("spriteram")   // sprite RAM
	AM_RANGE(0x2900, 0x2fff) AM_WRITENOP                    // cleared along with sprite RAM
	AM_RANGE(0x2973, 0x2973) AM_READ(aeroboto_2973_r)           // protection read
	AM_RANGE(0x3000, 0x3000) AM_READWRITE(aeroboto_in0_r, aeroboto_3000_w)
	AM_RANGE(0x3001, 0x3001) AM_READ_PORT("DSW1") AM_WRITE(soundlatch_byte_w)
	AM_RANGE(0x3002, 0x3002) AM_READ_PORT("DSW2") AM_WRITE(soundlatch2_byte_w)
	AM_RANGE(0x3003, 0x3003) AM_WRITEONLY AM_SHARE("vscroll")
	AM_RANGE(0x3004, 0x3004) AM_READ(aeroboto_201_r) AM_WRITEONLY AM_SHARE("starx")
	AM_RANGE(0x3005, 0x3005) AM_WRITEONLY AM_SHARE("stary") // usable but probably wrong
	AM_RANGE(0x3006, 0x3006) AM_WRITEONLY AM_SHARE("bgcolor")
	AM_RANGE(0x3800, 0x3800) AM_READ(aeroboto_irq_ack_r)        // watchdog or IRQ ack
	AM_RANGE(0x4000, 0xffff) AM_ROM                             // main ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, aeroboto_state )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x9000, 0x9001) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0x9002, 0x9002) AM_DEVREAD("ay1", ay8910_device, data_r)
	AM_RANGE(0xa000, 0xa001) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
	AM_RANGE(0xa002, 0xa002) AM_DEVREAD("ay2", ay8910_device, data_r)
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END



static INPUT_PORTS_START( formatz )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "Infinite (Cheat)")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "30000" )
	PORT_DIPSETTING(    0x08, "40000" )
	PORT_DIPSETTING(    0x04, "70000" )
	PORT_DIPSETTING(    0x00, "100000" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "SW1:5" )        /* Listed as "Unused" */
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	/* The last dip switch is directly connected to the video hardware and
	   flips the screen. The program instead sees the coin input, which must
	   stay low for exactly 2 frames to be consistently recognized. */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2) PORT_DIPLOCATION("SW1:8") /* "Screen Inversion" */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Hard ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SW2:6" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW2:7" )        /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW2:8" )        /* Listed as "Unused" */
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ 4, 0 },
	{ 0, 1, 2, 3, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+1, RGN_FRAC(1,2)+2, RGN_FRAC(1,2)+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};
/*
// exact star layout unknown... could be anything
static const gfx_layout starlayout =
{
    8,8,
    RGN_FRAC(1,1),
    1,
    { 0 },
    { 0, 1, 2, 3, 4, 5, 6, 7 },
    { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
    8*8
};
*/
static const gfx_layout spritelayout =
{
	8,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*8
};

static GFXDECODE_START( aeroboto )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0,  64 )     /* chars */
//  GFXDECODE_ENTRY( "gfx2", 0, starlayout,     0, 128 )     /* sky */
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout,   0,   8 )
GFXDECODE_END

void aeroboto_state::machine_start()
{
	m_stars_rom = memregion("gfx2")->base();
	m_stars_length = memregion("gfx2")->bytes();

	save_item(NAME(m_disable_irq));
	save_item(NAME(m_count));
}

void aeroboto_state::machine_reset()
{
	m_disable_irq = 0;
	m_count = 0;

	m_charbank = 0;
	m_starsoff = 0;
	m_ox = 0;
	m_oy = 0;
	m_sx = 0;
	m_sy = 0;
}

static MACHINE_CONFIG_START( formatz, aeroboto_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, XTAL_10MHz/8) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", aeroboto_state,  aeroboto_interrupt)

	MCFG_CPU_ADD("audiocpu", M6809, XTAL_10MHz/16) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", aeroboto_state,  irq0_line_hold)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 31*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(aeroboto_state, screen_update_aeroboto)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", aeroboto)

	MCFG_PALETTE_ADD_RRRRGGGGBBBB_PROMS("palette", 256)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, XTAL_10MHz/8) /* verified on pcb */
	MCFG_AY8910_PORT_A_READ_CB(READ8(driver_device, soundlatch_byte_r))
	MCFG_AY8910_PORT_B_READ_CB(READ8(driver_device, soundlatch2_byte_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay2", AY8910, XTAL_10MHz/16) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( formatz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "format_z.8",   0x4000, 0x4000, CRC(81a2416c) SHA1(d43c6bcc079847cb4c8e77fdc4d9d5bb9c2cc41a) )
	ROM_LOAD( "format_z.7",   0x8000, 0x4000, CRC(986e6052) SHA1(4d39eda38fa17695f8217b0032a750cbe71c5674) )
	ROM_LOAD( "format_z.6",   0xc000, 0x4000, CRC(baa0d745) SHA1(72b6cf31c9bbf9b5c55ef3f4ca5877ce576beda9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "format_z.9",   0xf000, 0x1000, CRC(6b9215ad) SHA1(3ab416d070bf6b9a8be3e19d4dbc3a399d9ab5cb) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "format_z.5",   0x0000, 0x2000, CRC(ba50be57) SHA1(aa37b644e8c1944b4c0ba81164d5a52be8ab491f) )  /* characters */

	ROM_REGION( 0x2000, "gfx2", 0 ) // starfield data
	ROM_LOAD( "format_z.4",   0x0000, 0x2000, CRC(910375a0) SHA1(1044e0f45ce34c15986d9ab520c0e7d08fd46dde) )  /* characters */

	ROM_REGION( 0x3000, "gfx3", 0 )
	ROM_LOAD( "format_z.1",   0x0000, 0x1000, CRC(5739afd2) SHA1(3a645bc8a5ac69f1dc878a589c580f2bf033d3cb) )  /* sprites */
	ROM_LOAD( "format_z.2",   0x1000, 0x1000, CRC(3a821391) SHA1(476507ba5e5d64ca3729244590beadb9b3a6a018) )  /* sprites */
	ROM_LOAD( "format_z.3",   0x2000, 0x1000, CRC(7d1aec79) SHA1(bb19d6c91a14df26706226cfe22853bb8383c63d) )  /* sprites */

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "10c",          0x0000, 0x0100, CRC(b756dd6d) SHA1(ea79f87f84ded2f0a66458af24cbc792e5ff77e3) )
	ROM_LOAD( "10b",          0x0100, 0x0100, CRC(00df8809) SHA1(f4539c052a5ce8a63662db070c3f52139afef23d) )
	ROM_LOAD( "10a",          0x0200, 0x0100, CRC(e8733c8f) SHA1(105b44c9108ee173a417f8c79ec8381f824dd675) )
ROM_END

ROM_START( aeroboto )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "aeroboto.8",   0x4000, 0x4000, CRC(4d3fc049) SHA1(6efb8c58c025a69ac2dce99049128861f7ede690) )
	ROM_LOAD( "aeroboto.7",   0x8000, 0x4000, CRC(522f51c1) SHA1(4ea47d0b8b65e711c99701c055dbaf70a003d441) )
	ROM_LOAD( "aeroboto.6",   0xc000, 0x4000, CRC(1a295ffb) SHA1(990b3f2f883717c180089b6ba5ae381ed9272341) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "format_z.9",   0xf000, 0x1000, CRC(6b9215ad) SHA1(3ab416d070bf6b9a8be3e19d4dbc3a399d9ab5cb) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "aeroboto.5",   0x0000, 0x2000, CRC(32fc00f9) SHA1(fd912fe2ab0101057c15c846f0cc4259cd94b035) )  /* characters */

	ROM_REGION( 0x2000, "gfx2", 0 ) // starfield data
	ROM_LOAD( "format_z.4",   0x0000, 0x2000, CRC(910375a0) SHA1(1044e0f45ce34c15986d9ab520c0e7d08fd46dde) )  /* characters */

	ROM_REGION( 0x3000, "gfx3", 0 )
	ROM_LOAD( "aeroboto.1",   0x0000, 0x1000, CRC(7820eeaf) SHA1(dedd15295bb02f417d0f51a29df686b66b94dee1) )  /* sprites */
	ROM_LOAD( "aeroboto.2",   0x1000, 0x1000, CRC(c7f81a3c) SHA1(21476a4146d5c57e2b15125c304fc61d82edf4af) )  /* sprites */
	ROM_LOAD( "aeroboto.3",   0x2000, 0x1000, CRC(5203ad04) SHA1(d16eb370de9033793a502e23c82a3119cd633aa9) )  /* sprites */

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "10c",          0x0000, 0x0100, CRC(b756dd6d) SHA1(ea79f87f84ded2f0a66458af24cbc792e5ff77e3) )
	ROM_LOAD( "10b",          0x0100, 0x0100, CRC(00df8809) SHA1(f4539c052a5ce8a63662db070c3f52139afef23d) )
	ROM_LOAD( "10a",          0x0200, 0x0100, CRC(e8733c8f) SHA1(105b44c9108ee173a417f8c79ec8381f824dd675) )
ROM_END



GAME( 1984, formatz,  0,       formatz, formatz, driver_device, 0, ROT0, "Jaleco", "Formation Z", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1984, aeroboto, formatz, formatz, formatz, driver_device, 0, ROT0, "Jaleco (Williams license)", "Aeroboto", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_GRAPHICS )

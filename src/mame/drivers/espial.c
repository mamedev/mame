// license:BSD-3-Clause
// copyright-holders:Brad Oliver
/***************************************************************************

 Espial hardware games

Espial: The Orca logo is displayed, but looks to be "blacked out" via the
        color proms by having 0x1c & 0x1d set to black.

TODO:
- merge with zodiack.c

Stephh's notes (based on the games Z80 code and some tests) :

1) 'espial*'

  - The games read both players controls for player 2 when "Cabinet" is set
    to "Upright" (code at 0x0321).
  - The games read both buttons status regardless of settings. They are
    then comnbined if Dip Switch is set to "1" (code at 0x32a).
  - The "CRE." displayed at the bottom right of the screen is in fact
    not really the number of credits (especially when coinage isn't 1C_1C)
    as it relies on a transformation of real number of credits (stored at
    0x5802) based on settings (coins needed stored at 0x5806 and credits
    awarded at 0x5804). Check code at 0x080b which displays the odd value.

2) 'netwars'

  - The game reads both players controls for player 2 when "Cabinet" is set
    to "Upright" (code at 0x038e).
  - The "CREDIT" displayed at the bottom right of the screen is in fact
    not really the number of credits (especially when coinage isn't 1C_1C)
    as it relies on a transformation of real number of credits (stored at
    0x5802) based on settings (coins needed stored at 0x5806 and credits
    awarded at 0x5804). Check code at 0x0147 which displays the odd value.
  - When you get a perfect in the bonus game, you are only awarded 1000 points
    even if the game tells you that you have been awarded 10000 points.

***************************************************************************/

#include "emu.h"
#include "includes/espial.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"




void espial_state::machine_reset()
{
	m_flipscreen = 0;

	m_main_nmi_enabled = FALSE;
	m_sound_nmi_enabled = FALSE;
}

void espial_state::machine_start()
{
	//save_item(NAME(mcu_out[1]));
	save_item(NAME(m_sound_nmi_enabled));
}


WRITE8_MEMBER(espial_state::espial_master_interrupt_mask_w)
{
	m_main_nmi_enabled = ~(data & 1);
}


WRITE8_MEMBER(espial_state::espial_sound_nmi_mask_w)
{
	m_sound_nmi_enabled = data & 1;
}

TIMER_DEVICE_CALLBACK_MEMBER(espial_state::espial_scanline)
{
	int scanline = param;

	if(scanline == 240 && m_main_nmi_enabled) // vblank-out irq
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);

	if(scanline == 16) // timer irq, checks soundlatch port then updates some sound related work RAM buffers
		m_maincpu->set_input_line(0, HOLD_LINE);
}


INTERRUPT_GEN_MEMBER(espial_state::espial_sound_nmi_gen)
{
	if (m_sound_nmi_enabled)
		nmi_line_pulse(device);
}


WRITE8_MEMBER(espial_state::espial_master_soundlatch_w)
{
	soundlatch_byte_w(space, offset, data);
	m_audiocpu->set_input_line(0, HOLD_LINE);
}


static ADDRESS_MAP_START( espial_map, AS_PROGRAM, 8, espial_state )
	AM_RANGE(0x0000, 0x4fff) AM_ROM
	AM_RANGE(0x5800, 0x5fff) AM_RAM
	AM_RANGE(0x6081, 0x6081) AM_READ_PORT("IN0")
	AM_RANGE(0x6082, 0x6082) AM_READ_PORT("DSW1")
	AM_RANGE(0x6083, 0x6083) AM_READ_PORT("IN1")
	AM_RANGE(0x6084, 0x6084) AM_READ_PORT("IN2")
	AM_RANGE(0x6090, 0x6090) AM_READ(soundlatch2_byte_r) AM_WRITE(espial_master_soundlatch_w)
	AM_RANGE(0x7000, 0x7000) AM_READWRITE(watchdog_reset_r, watchdog_reset_w)
	AM_RANGE(0x7100, 0x7100) AM_WRITE(espial_master_interrupt_mask_w)
	AM_RANGE(0x7200, 0x7200) AM_WRITE(espial_flipscreen_w)
	AM_RANGE(0x8000, 0x801f) AM_RAM AM_SHARE("spriteram_1")
	AM_RANGE(0x8020, 0x803f) AM_READONLY
	AM_RANGE(0x8400, 0x87ff) AM_RAM_WRITE(espial_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x8800, 0x880f) AM_WRITEONLY AM_SHARE("spriteram_3")
	AM_RANGE(0x8c00, 0x8fff) AM_RAM_WRITE(espial_attributeram_w) AM_SHARE("attributeram")
	AM_RANGE(0x9000, 0x901f) AM_RAM AM_SHARE("spriteram_2")
	AM_RANGE(0x9020, 0x903f) AM_RAM_WRITE(espial_scrollram_w) AM_SHARE("scrollram")
	AM_RANGE(0x9400, 0x97ff) AM_RAM_WRITE(espial_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xc000, 0xcfff) AM_ROM
ADDRESS_MAP_END


/* there are a lot of unmapped reads from all over memory as the
   code uses POP instructions in a delay loop */
static ADDRESS_MAP_START( netwars_map, AS_PROGRAM, 8, espial_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x5800, 0x5fff) AM_RAM
	AM_RANGE(0x6081, 0x6081) AM_READ_PORT("IN0")
	AM_RANGE(0x6082, 0x6082) AM_READ_PORT("DSW1")
	AM_RANGE(0x6083, 0x6083) AM_READ_PORT("IN1")
	AM_RANGE(0x6084, 0x6084) AM_READ_PORT("IN2")
	AM_RANGE(0x6090, 0x6090) AM_READ(soundlatch2_byte_r) AM_WRITE(espial_master_soundlatch_w)
	AM_RANGE(0x7000, 0x7000) AM_READWRITE(watchdog_reset_r, watchdog_reset_w)
	AM_RANGE(0x7100, 0x7100) AM_WRITE(espial_master_interrupt_mask_w)
	AM_RANGE(0x7200, 0x7200) AM_WRITE(espial_flipscreen_w)
	AM_RANGE(0x8000, 0x801f) AM_RAM AM_SHARE("spriteram_1")
	AM_RANGE(0x8000, 0x87ff) AM_RAM_WRITE(espial_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x8800, 0x880f) AM_RAM AM_SHARE("spriteram_3")
	AM_RANGE(0x8800, 0x8fff) AM_RAM_WRITE(espial_attributeram_w) AM_SHARE("attributeram")
	AM_RANGE(0x9000, 0x901f) AM_RAM AM_SHARE("spriteram_2")
	AM_RANGE(0x9020, 0x903f) AM_RAM_WRITE(espial_scrollram_w) AM_SHARE("scrollram")
	AM_RANGE(0x9000, 0x97ff) AM_RAM_WRITE(espial_colorram_w) AM_SHARE("colorram")
ADDRESS_MAP_END


static ADDRESS_MAP_START( espial_sound_map, AS_PROGRAM, 8, espial_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x4000, 0x4000) AM_WRITE(espial_sound_nmi_mask_w)
	AM_RANGE(0x6000, 0x6000) AM_READWRITE(soundlatch_byte_r, soundlatch2_byte_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( espial_sound_io_map, AS_IO, 8, espial_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
ADDRESS_MAP_END


/* verified from Z80 code */
static INPUT_PORTS_START( espial )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x00, "Number of Buttons" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x02, 0x02, "Enemy Bullets Vulnerable" )  /* you can shoot bullets */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_HIGH )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x1c, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Bonus_Life ) )       /* code at 0x43e1 in 'espial' and 0x44b5 in 'espialu' */
	PORT_DIPSETTING(    0x00, "20k 70k 70k+" )              /* last bonus life at 980k : max. 15 bonus lives */
	PORT_DIPSETTING(    0x20, "50k 100k 100k+" )            /* last bonus life at 900k : max. 10 bonus lives */
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, "Reset on Check Error" )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY
INPUT_PORTS_END


/* verified from Z80 code */
static INPUT_PORTS_START( netwars )
	PORT_START("IN0")
	PORT_DIPUNUSED( 0x01, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Difficulty ) )       /* when enemies shoot - code at 0x2216 */
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPUNUSED( 0x04, IP_ACTIVE_HIGH )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_HIGH )                  /* no effect due to code at 0x0e0a */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x1c, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Bonus_Life ) )       /* code at 0x2383 */
	PORT_DIPSETTING(    0x00, "20k and 50k" )
	PORT_DIPSETTING(    0x20, "40k and 70k" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, "Reset on Check Error" )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_4WAY
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(2,2),
	2,
	{ 0, 4 },
	{ STEP4(0,1), STEP4(8*8,1) },
	{ STEP8(0,8) },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ STEP8(0,1), STEP8(8*8,1) },
	{ STEP8(0,8), STEP8(16*8,8) },
	32*8
};


static GFXDECODE_START( espial )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,    0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,  0, 64 )
GFXDECODE_END




static MACHINE_CONFIG_START( espial, espial_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 3072000)   /* 3.072 MHz */
	MCFG_CPU_PROGRAM_MAP(espial_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", espial_state, espial_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("audiocpu", Z80, 3072000)  /* 2 MHz?????? */
	MCFG_CPU_PROGRAM_MAP(espial_sound_map)
	MCFG_CPU_IO_MAP(espial_sound_io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(espial_state, espial_sound_nmi_gen, 4*60)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(espial_state, screen_update_espial)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", espial)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(espial_state, espial)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", AY8910, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( netwars, espial )

	/* basic machine hardware */

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(netwars_map)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE(32*8, 64*8)

	MCFG_VIDEO_START_OVERRIDE(espial_state,netwars)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( espial )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "esp3.4f",      0x0000, 0x2000, CRC(0973c8a4) SHA1(d1fc6775870710b3dfea4e58a937ab996021adb1) )
	ROM_LOAD( "esp4.4h",      0x2000, 0x2000, CRC(6034d7e5) SHA1(62c9699088f4ee1c69ec10a2f82feddd4083efef) )
	ROM_LOAD( "esp6.bin",     0x4000, 0x1000, CRC(357025b4) SHA1(8bc62f564fcbe37bd490452b2d569d1981f76db1) )
	ROM_LOAD( "esp5.bin",     0xc000, 0x1000, CRC(d03a2fc4) SHA1(791d70e4354350507f4c39d6115c046254168895) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "esp1.4n",      0x0000, 0x1000, CRC(fc7729e9) SHA1(96dfec574521fa4fe2588fbac2ef1caba6c1b884) )
	ROM_LOAD( "esp2.4r",      0x1000, 0x1000, CRC(e4e256da) SHA1(8007471405bdcf90e29657a3ac2c2f84c9db7c9b) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "espial8.4b",   0x0000, 0x2000, CRC(2f43036f) SHA1(316e9fab778d6c0abb0b6673aba33dfbe44b1262) )
	ROM_LOAD( "espial7.4a",   0x2000, 0x1000, CRC(ebfef046) SHA1(5aa6efb7254fb42e814c1a29c5363f2d0727452f) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "espial10.4e",  0x0000, 0x1000, CRC(de80fbc1) SHA1(f5601eac8cb35a92c51bf81e5ac5a2b79bcde28f) )
	ROM_LOAD( "espial9.4d",   0x1000, 0x1000, CRC(48c258a0) SHA1(55e72b9072ddc05f848e5a6fae159c554102010b) )

	ROM_REGION( 0x0200, "proms", 0 ) /* The MMI6301 Bipolar PROM is compatible to the 82s129 */
	ROM_LOAD( "mmi6301.1f",   0x0000, 0x0100, CRC(d12de557) SHA1(53e8a57dfab677cc5b9cdd83d2fbeb93169bcefd) ) /* palette low 4 bits */
	ROM_LOAD( "mmi6301.1h",   0x0100, 0x0100, CRC(4c84fe70) SHA1(7ac52bd5b19663b9526ecb678e61db9939d2285d) ) /* palette high 4 bits */
ROM_END

ROM_START( espialu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "espial3.4f",   0x0000, 0x2000, CRC(10f1da30) SHA1(8954ca3c7fccb8dd8433015ee303bb75a98f3474) )
	ROM_LOAD( "espial4.4h",   0x2000, 0x2000, CRC(d2adbe39) SHA1(13c6041fd0e7c49988af89e3bab1b20999336928) )
	ROM_LOAD( "espial.6",     0x4000, 0x1000, CRC(baa60bc1) SHA1(fc3d3f2e0316efb31161b28984fc8bd94473b783) )
	ROM_LOAD( "espial.5",     0xc000, 0x1000, CRC(6d7bbfc1) SHA1(d886a76ce4a23c1310135bf1e4ffeda6d44625e7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "espial1.4n",   0x0000, 0x1000, CRC(1e5ec20b) SHA1(f3bee38737321edf2d1ea753124421416441666e) )
	ROM_LOAD( "espial2.4r",   0x1000, 0x1000, CRC(3431bb97) SHA1(97343bfb5e49cd1d26799723d8c5a31eff7b1170) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "espial8.4b",   0x0000, 0x2000, CRC(2f43036f) SHA1(316e9fab778d6c0abb0b6673aba33dfbe44b1262) )
	ROM_LOAD( "espial7.4a",   0x2000, 0x1000, CRC(ebfef046) SHA1(5aa6efb7254fb42e814c1a29c5363f2d0727452f) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "espial10.4e",  0x0000, 0x1000, CRC(de80fbc1) SHA1(f5601eac8cb35a92c51bf81e5ac5a2b79bcde28f) )
	ROM_LOAD( "espial9.4d",   0x1000, 0x1000, CRC(48c258a0) SHA1(55e72b9072ddc05f848e5a6fae159c554102010b) )

	ROM_REGION( 0x0200, "proms", 0 ) /* The MMI6301 Bipolar PROM is compatible to the 82s129 */
	ROM_LOAD( "mmi6301.1f",   0x0000, 0x0100, CRC(d12de557) SHA1(53e8a57dfab677cc5b9cdd83d2fbeb93169bcefd) ) /* palette low 4 bits */
	ROM_LOAD( "mmi6301.1h",   0x0100, 0x0100, CRC(4c84fe70) SHA1(7ac52bd5b19663b9526ecb678e61db9939d2285d) ) /* palette high 4 bits */
ROM_END

ROM_START( netwars )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "netw3.4f",     0x0000, 0x2000, CRC(8e782991) SHA1(4fd533035b61b7006ef94300bb63474fb9e1c9f0) )
	ROM_LOAD( "netw4.4h",     0x2000, 0x2000, CRC(6e219f61) SHA1(a27304017251777be501861e106a670fff078d54) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "netw1.4n",     0x0000, 0x1000, CRC(53939e16) SHA1(938f505db0cfcfafb751378ae0c139b7f32404cb) )
	ROM_LOAD( "netw2.4r",     0x1000, 0x1000, CRC(c096317a) SHA1(e61a3e9107481fd80309172a1a9a431903e02489) )

	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "netw8.4b",     0x0000, 0x2000, CRC(2320277e) SHA1(4e05f6833de89f8f7cc0a0d1cbec03656f8b54a1) )
	ROM_LOAD( "netw7.4a",     0x2000, 0x2000, CRC(25cc5b7f) SHA1(2e089c3d5f8ebba676a959ba71bc9c1750312721) )

	ROM_REGION( 0x2000, "gfx2", 0 )
	ROM_LOAD( "netw10.4e",    0x0000, 0x1000, CRC(87b65625) SHA1(a702726c0fbe7669604f48bf2c19a54031645731) )
	ROM_LOAD( "netw9.4d",     0x1000, 0x1000, CRC(830d0218) SHA1(c726a4a9dd1f10279f79cbe5fdd693a62d9d3ac5) )

	ROM_REGION( 0x0200, "proms", 0 ) /* The MMI6301 Bipolar PROM is compatible to the 82s129 */
	ROM_LOAD( "mmi6301.1f",   0x0000, 0x0100, CRC(f3ae1fe2) SHA1(4f259f8da3c9ecdc6010f83b6abc1371366bd0ab) ) /* palette low 4 bits */
	ROM_LOAD( "mmi6301.1h",   0x0100, 0x0100, CRC(c44c3771) SHA1(c86125fac28fafc744957258bf3bb5a6dc664b54) ) /* palette high 4 bits */
ROM_END



GAME( 1983, espial,  0,      espial,  espial, driver_device,  0, ROT0,  "Orca / Thunderbolt", "Espial (Europe)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, espialu, espial, espial,  espial, driver_device,  0, ROT0,  "Orca / Thunderbolt", "Espial (US?)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, netwars, 0,      netwars, netwars, driver_device, 0, ROT90, "Orca (Esco Trading Co license)", "Net Wars", MACHINE_SUPPORTS_SAVE )

// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

Truco Clemente (c) 1991 Miky SRL

driver by Ernesto Corvi

Notes:
- Sloppy coin insertion, needs to stay high for 60 Hz wtf?
- Audio is almost there.
- I think this runs on a heavily modified PacMan type of board.

----------------------------------
Additional Notes (Roberto Fresca):
----------------------------------
Mainboard: Pacman bootleg jamma board.
Daughterboard: Custom made, plugged in the 2 roms and Z80 mainboard sockets.

  - 01 x Z80
  - 03 x 27c010
  - 02 x am27s19
  - 03 x GAL 16v8b      (All of them have the same contents... Maybe read protected.)
  - 01 x PAL CE 20v8h   (The fuse map is suspect too)
  - 01 x lm324n

  To not overload the driver, I put the rest of technical info in
  http://robbie.mameworld.info/trucocl.htm

- Added 2 "hidden" color proms (am27s19)
- One GAL is connected to the color proms inputs.
- The name of the company is "Miky SRL" instead of "Caloi Miky SRL".
  Caloi (Carlos Loiseau), is the Clemente's creator.

***************************************************************************/

#include "emu.h"
#include "includes/trucocl.h"

#include "cpu/z80/z80.h"
#include "machine/watchdog.h"
#include "screen.h"
#include "speaker.h"


// TODO: doesn't seem suited to neither irq nor nmi
void trucocl_state::irq_enable_w(uint8_t data)
{
	m_irq_mask = (data & 1) ^ 1;
}


TIMER_CALLBACK_MEMBER(trucocl_state::dac_irq)
{
	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


void trucocl_state::audio_dac_w(uint8_t data)
{
	uint8_t *rom = memregion("maincpu")->base();
	int dac_address = ( data & 0xf0 ) << 8;
	int sel = ( ( (~data) >> 1 ) & 2 ) | ( data & 1 );

	if ( m_cur_dac_address != dac_address )
	{
		m_cur_dac_address_index = 0;
		m_cur_dac_address = dac_address;
	}
	else
	{
		m_cur_dac_address_index++;
	}

	if ( sel & 1 )
		dac_address += 0x10000;

	if ( sel & 2 )
		dac_address += 0x10000;

	dac_address += 0x10000;

	m_dac->write(rom[dac_address+m_cur_dac_address_index]);

	m_dac_irq_timer->adjust(attotime::from_hz( 16000 ));
}

void trucocl_state::main_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x43ff).ram().w(FUNC(trucocl_state::trucocl_videoram_w)).share("videoram");
	map(0x4400, 0x47ff).ram().w(FUNC(trucocl_state::trucocl_colorram_w)).share("colorram");
	map(0x4800, 0x4fff).ram();
	map(0x5000, 0x5000).w(FUNC(trucocl_state::irq_enable_w));
	map(0x5000, 0x503f).portr("IN0");
	map(0x5080, 0x5080).portr("DSW").w(FUNC(trucocl_state::audio_dac_w));
	map(0x50c0, 0x50c0).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x8000, 0xffff).rom();
}

void trucocl_state::main_io(address_map &map)
{
	map(0x0000, 0xffff).nopr(); // read then always discarded?
}

static INPUT_PORTS_START( trucocl )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 ) //PORT_IMPULSE(60)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_IMPULSE(2)

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Enable BGM fanfare" ) // enables extra BGMs on attract mode
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "Nudity" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	// TODO: more are tested ingame
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout tilelayout =
{
	8,8,        /* 8*8 characters */
	0x10000/32, /* 2048 characters */
	4,          /* 4 bits per pixel */
	{ 0, 1,2,3 },
	{ 0, 4, 0x8000*8+0,0x8000*8+4, 8*8+0, 8*8+4, 0x8000*8+8*8+0,0x8000*8+8*8+4 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8        /* every char takes 16 consecutive bytes */
};



static GFXDECODE_START( gfx_trucocl )
	GFXDECODE_ENTRY( "gfx1", 0,       tilelayout,      0, 2 )
	GFXDECODE_ENTRY( "gfx1", 0x10000, tilelayout,      0, 2 )
GFXDECODE_END

void trucocl_state::machine_start()
{
	m_dac_irq_timer = timer_alloc(FUNC(trucocl_state::dac_irq), this);
}

void trucocl_state::machine_reset()
{
	m_cur_dac_address = -1;
	m_cur_dac_address_index = 0;
	m_dac_irq_timer->adjust(attotime::never);
}

INTERRUPT_GEN_MEMBER(trucocl_state::trucocl_interrupt)
{
//  if(m_irq_mask)
		device.execute().set_input_line(0, HOLD_LINE);
}

void trucocl_state::trucocl(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 18432000/6);
	m_maincpu->set_addrmap(AS_PROGRAM, &trucocl_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &trucocl_state::main_io);
	m_maincpu->set_vblank_int("screen", FUNC(trucocl_state::trucocl_interrupt));

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(trucocl_state::screen_update_trucocl));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_trucocl);
	PALETTE(config, "palette", FUNC(trucocl_state::trucocl_palette), 32);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // unknown DAC
}

/***************************************************************************

  ROM definitions

***************************************************************************/

ROM_START( trucocl )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* ROMs + space for additional RAM + samples */
	ROM_LOAD( "trucocl.01", 0x00000, 0x20000, CRC(c9511c37) SHA1(d6a0fa573c8d2faf1a94a2be26fcaafe631d0699) )
	ROM_LOAD( "trucocl.03", 0x20000, 0x20000, CRC(b37ce38c) SHA1(00bd506e9a03cb8ed65b0b599514db6b9b0ee5f3) ) /* samples */

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "trucocl.02", 0x0000, 0x20000, CRC(bda803e5) SHA1(e4fee42f23be4e0dc8926b6294e4b3e4a38ff185) ) /* tiles */

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "27s19.u2",    0x0000, 0x0020, CRC(75aeff6a) SHA1(fecd117ec9bb8ac2834d422eb507ec78410aff0f) )
	ROM_LOAD( "27s19.u1",    0x0020, 0x0020, CRC(f952f823) SHA1(adc6a05827b1bc47d84827808c324d93ee0f32b9) )
ROM_END

/*************************************
 *
 *  Game drivers
 *
 *************************************/

//    YEAR  NAME      PARENT  MACHINE  INPUT    STATE          INIT          MONITOR
GAME( 1991, trucocl,  0,      trucocl, trucocl, trucocl_state, empty_init,   ROT0, "Miky SRL", "Truco Clemente", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )

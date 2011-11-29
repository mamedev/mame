/***************************************************************************

Truco Clemente (c) 1991 Miky SRL

driver by Ernesto Corvi

Notes:
- After one game you can't play anymore.
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
#include "cpu/z80/z80.h"
#include "sound/dac.h"
#include "includes/trucocl.h"

static WRITE8_HANDLER( irq_enable_w )
{
	trucocl_state *state = space->machine().driver_data<trucocl_state>();

	state->m_irq_mask = (data & 1) ^ 1;
}


static TIMER_CALLBACK( dac_irq )
{
	cputag_set_input_line(machine, "maincpu", INPUT_LINE_NMI, PULSE_LINE );
}

static WRITE8_DEVICE_HANDLER( audio_dac_w )
{
	trucocl_state *state = device->machine().driver_data<trucocl_state>();
	UINT8 *rom = device->machine().region("maincpu")->base();
	int	dac_address = ( data & 0xf0 ) << 8;
	int	sel = ( ( (~data) >> 1 ) & 2 ) | ( data & 1 );

	if ( state->m_cur_dac_address != dac_address )
	{
		state->m_cur_dac_address_index = 0;
		state->m_cur_dac_address = dac_address;
	}
	else
	{
		state->m_cur_dac_address_index++;
	}

	if ( sel & 1 )
		dac_address += 0x10000;

	if ( sel & 2 )
		dac_address += 0x10000;

	dac_address += 0x10000;

	dac_data_w( device, rom[dac_address+state->m_cur_dac_address_index] );

	device->machine().scheduler().timer_set( attotime::from_hz( 16000 ), FUNC(dac_irq ));
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM_WRITE(trucocl_videoram_w) AM_BASE_MEMBER(trucocl_state, m_videoram)
	AM_RANGE(0x4400, 0x47ff) AM_RAM_WRITE(trucocl_colorram_w) AM_BASE_MEMBER(trucocl_state, m_colorram)
	AM_RANGE(0x4c00, 0x4fff) AM_RAM
	AM_RANGE(0x5000, 0x5000) AM_WRITE(irq_enable_w)
	AM_RANGE(0x5000, 0x503f) AM_READ_PORT("IN0")
	AM_RANGE(0x5080, 0x5080) AM_DEVWRITE("dac", audio_dac_w)
	AM_RANGE(0x50c0, 0x50c0) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( trucocl )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_IMPULSE(2)
INPUT_PORTS_END


static const gfx_layout tilelayout =
{
	8,8,		/* 8*8 characters */
	0x10000/32,	/* 2048 characters */
	4,			/* 4 bits per pixel */
	{ 0, 1,2,3 },
	{ 0, 4, 0x8000*8+0,0x8000*8+4, 8*8+0, 8*8+4, 0x8000*8+8*8+0,0x8000*8+8*8+4 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8		/* every char takes 16 consecutive bytes */
};



static GFXDECODE_START( trucocl )
	GFXDECODE_ENTRY( "gfx1", 0, 		tilelayout,      0, 2 )
	GFXDECODE_ENTRY( "gfx1", 0x10000, tilelayout,      0, 2 )
GFXDECODE_END

static INTERRUPT_GEN( trucocl_interrupt )
{
	trucocl_state *state = device->machine().driver_data<trucocl_state>();

	if(state->m_irq_mask)
		device_set_input_line(device, 0, HOLD_LINE);

}

static MACHINE_CONFIG_START( trucocl, trucocl_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 18432000/6)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT("screen", trucocl_interrupt)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE(trucocl)

	MCFG_GFXDECODE(trucocl)
	MCFG_PALETTE_LENGTH(32)

	MCFG_PALETTE_INIT(trucocl)
	MCFG_VIDEO_START(trucocl)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( trucocl )
	ROM_REGION( 0x40000, "maincpu", 0 )	/* ROMs + space for additional RAM + samples */
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

static DRIVER_INIT( trucocl )
{
	trucocl_state *state = machine.driver_data<trucocl_state>();
	state->m_cur_dac_address = -1;
	state->m_cur_dac_address_index = 0;
}



/******************************************************************************/
/*    YEAR   NAME     PARENT  MACHINE  INPUT    INIT     MONITOR  */

GAME( 1991, trucocl,  0,      trucocl, trucocl, trucocl, ROT0, "Miky SRL", "Truco Clemente", GAME_IMPERFECT_SOUND | GAME_NOT_WORKING )

// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/*

    United Amusements PC-Engine based hardware
    Driver by Mariusz Wojcieszek
    Thanks for Charles MacDonald for hardware docs

 Overview

 The system consists of a stock PC-Engine console, JAMMA interface board,
 and several cables to connect certain pins from the PCE backplane connector
 and pad connector to the interface board.

 History

 In 1989 United Amusements (a large operator of arcades in the US at that
 time) developed a JAMMA interface for the PC-Engine with NEC's blessing. NEC
 pulled funding for the project before mass production began, and it never
 took off.

 Driver notes:
 - game time is controlled using software loop - with current clock it takes lots
   of time until credit expires. Should Z80 clock be raised?
 - tone played by jamma if board is not emulated

 A kit owned by collector has following contents:
 -x1 NOS boxed NEC arcade unit with all connectors
 -x1 NOS boxed Keith Courage (custom/prototype PCB) with NOS marquee
 -x1 Pacland standard Hucard WITH arcade cabinet marquee (this proves that while this is a standard card, it was sold by United Amusements with the intention of being used in the arcade unit)
 -x1 Alien Crush standard Hucard with arcade "how to play" placard. Same as above.... proof this was sold for the arcade unit and not as a home card
 -x1 Ninja Warriors standard Hucard
 -x1 Victory Run standardd Hucard
 -x1 Plexiglass control panel overlay (used)

In the February 1990 issue of Video Games & Computer Entertainment magazine, there was a list of
all of the available UA produced Hu-Cards (at the current time of the article was published).
The article mentions that the UA Hu-Cards were not compatible with the TG-16 gaming console.
- Keith Courage in the Alpha Zones
- World Class Baseball
- Blazing Lazers
- Alien Crush
- China Warrior

Blazing Lazers dump notes:
----------------------------------------------------------------------------
Game software
----------------------------------------------------------------------------

An EPROM-based HuCard manufactured by NEC (PCB ID: PWD-703) is used to
store the game program. It supports four uPD27C2001 (256Kx8) EPROMs as
well as 27C010s for total ROM capacity of 1MB down to 128K in various
configurations.

Jumper settings:

J1
Pos. 1 : Map IC1 to 000000-03FFFF (256K)
Pos. 2 : Map IC1 to 000000-01FFFF (128K)

J2
Pos. 1 : Map IC2 to 040000-07FFFF (256K)
Pos. 2 : Map IC2 to 080000-09FFFF (128K)
Pos. 3 : Map IC2 to 020000-03FFFF (128K)

J3
Pos. 1 : Map IC3 to 080000-0BFFFF (256K)
Pos. 2 : Map IC3 to 080000-09FFFF (128K)
Pos. 3 : Map IC3 to 040000-05FFFF (128K)

J4
Pos. 1 : Map IC4 to 0C0000-0FFFFF (256K)
Pos. 2 : Map IC4 to 0A0000-0BFFFF (128K)
Pos. 3 : Map IC4 to 060000-07FFFF (128K)

The board came with three EPROMs in the following configuration:

IC1 is a 27C010 mapped to 000000-01FFFF
IC2 is a 27C010 mapped to 020000-03FFFF
IC3 is a 27C010 mapped to 080000-09FFFF
IC4 is unpopulated

The software was a modified version of "Blazing Lazers", identical to the
USA retail version with the region check (offsets 0 through $18) patched
with NOP to remove it. The interface board is connected to a PC-Engine
console rather than a TurboGrafx-16, which would cause the region check to
fail.


Keith Courage In Alpha Zones: dump was made from PC-Engine game dump of US version,
 with region check nop'ed out.
Alien Crush & Pac_Land: dumps made from PC-Engine dumps of JP versions
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/h6280/h6280.h"
#include "video/huc6260.h"
#include "video/huc6270.h"
#include "sound/c6280.h"
#include "machine/pcecommn.h"
#include "sound/discrete.h"


class uapce_state : public pce_common_state
{
public:
	uapce_state(const machine_config &mconfig, device_type type, const char *tag)
		: pce_common_state(mconfig, type, tag),
		m_discrete(*this, "discrete") { }

	UINT8 m_jamma_if_control_latch;
	DECLARE_WRITE8_MEMBER(jamma_if_control_latch_w);
	DECLARE_READ8_MEMBER(jamma_if_control_latch_r);
	DECLARE_READ8_MEMBER(jamma_if_read_dsw);
	virtual UINT8 joy_read();
	virtual void machine_reset();
	required_device<discrete_device> m_discrete;
	DECLARE_WRITE_LINE_MEMBER(pce_irq_changed);
};

#define UAPCE_SOUND_EN  NODE_10
#define UAPCE_TONE_752  NODE_11

static DISCRETE_SOUND_START(uapce)
	DISCRETE_INPUT_LOGIC(UAPCE_SOUND_EN)
	DISCRETE_SQUAREWFIX(UAPCE_TONE_752, UAPCE_SOUND_EN, 752, DEFAULT_TTL_V_LOGIC_1, 50, DEFAULT_TTL_V_LOGIC_1, 0)   // 752Hz
	DISCRETE_OUTPUT(UAPCE_TONE_752, 100)
DISCRETE_SOUND_END


WRITE8_MEMBER(uapce_state::jamma_if_control_latch_w)
{
	UINT8 diff = data ^ m_jamma_if_control_latch;
	m_jamma_if_control_latch = data;

/*  D7 : Controls relay which connects the PCE R-AUDIO output to the common audio path.
    (1= Relay closed, 0= Relay open) */
	machine().sound().system_enable( (data >> 7) & 1 );

/* D6 : Output to JAMMA connector KEY pin. Connected to /RESET on the PCE backplane connector.
    (1= /RESET not asserted, 0= /RESET asserted) */

	if ( diff & 0x40 )
	{
		m_maincpu->set_input_line(INPUT_LINE_RESET, (data & 0x40) ? CLEAR_LINE : ASSERT_LINE);
	}

/* D5 : Connected to a TIP31 which may control the coin meter:
      B = Latched D5
      C = JAMMA pin 'z'
      E = Ground

   Pin 'z' is a normally ground connection, but on this board it is isolated from ground.
   The wiring harness also has the corresponding wire separate from the others. */
	coin_counter_w(machine(), 0, BIT(data,5));

/* D4 : Connects the START1 switch input from the JAMMA connector to the
    "RUN" key input of the control pad multiplexer.
    (1= "RUN" input connected to START1 switch, 0= "RUN" input always '1') */

/* D3 : Controls the RESET input of timer B of a 556 IC, enabling a
      752 Hz (D-3) square wave to be output on the common audio path.
      (1= Tone output ON, 0= Tone output OFF) */

	m_discrete->write(space, UAPCE_SOUND_EN, BIT(data,3));

/* D2 : Not latched, though software writes to this bit like it is. */

/* D1 : Not latched. */

/* D0 : Not latched. */
}

READ8_MEMBER(uapce_state::jamma_if_control_latch_r)
{
	return m_jamma_if_control_latch & 0x08;
}

READ8_MEMBER(uapce_state::jamma_if_read_dsw)
{
	UINT8 dsw_val;

	dsw_val = ioport("DSW" )->read();

	if ( BIT( offset, 7 ) == 0 )
	{
		dsw_val >>= 7;
	}
	else if ( BIT( offset, 6 ) == 0 )
	{
		dsw_val >>= 6;
	}
	else if ( BIT( offset, 5 ) == 0 )
	{
		dsw_val >>= 5;
	}
	else if ( BIT( offset, 4 ) == 0 )
	{
		dsw_val >>= 4;
	}
	else if ( BIT( offset, 3 ) == 0 )
	{
		dsw_val >>= 3;
	}
	else if ( BIT( offset, 2 ) == 0 )
	{
		dsw_val >>= 2;
	}
	else if ( BIT( offset, 1 ) == 0 )
	{
		dsw_val >>= 1;
	}
	else if ( BIT( offset, 0 ) == 0 )
	{
		dsw_val >>= 0;
	}

	return dsw_val & 1;
}

UINT8 uapce_state::joy_read()
{
	if ( m_jamma_if_control_latch & 0x10 )
	{
		return ioport("JOY" )->read();
	}
	else
	{
		return ioport("JOY" )->read() | 0x08;
	}
}

void uapce_state::machine_reset()
{
	m_jamma_if_control_latch = 0;
}

static ADDRESS_MAP_START( z80_map, AS_PROGRAM, 8, uapce_state )
	AM_RANGE( 0x0000, 0x07FF) AM_ROM
	AM_RANGE( 0x0800, 0x0FFF) AM_RAM
	AM_RANGE( 0x1000, 0x17FF) AM_WRITE(jamma_if_control_latch_w )
	AM_RANGE( 0x1800, 0x1FFF) AM_READ(jamma_if_read_dsw )
	AM_RANGE( 0x2000, 0x27FF) AM_READ_PORT( "COIN" )
	AM_RANGE( 0x2800, 0x2FFF) AM_READ(jamma_if_control_latch_r )
ADDRESS_MAP_END


static INPUT_PORTS_START( uapce )
	PORT_START( "JOY" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) /* button I */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) /* button II */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) /* select */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 ) /* run */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )

	PORT_START( "DSW" )
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x38, 0x10, "Time" )
	PORT_DIPSETTING(    0x00, "Untimed Play" )
	PORT_DIPSETTING(    0x08, "1 minute Timed Play" )
	PORT_DIPSETTING(    0x10, "3 minute Timed Play" )
	PORT_DIPSETTING(    0x18, "5 minute Timed Play" )
	PORT_DIPSETTING(    0x20, "7 minute Timed Play" )
	PORT_DIPSETTING(    0x28, "10 minute Timed Play" )
	PORT_DIPSETTING(    0x30, "12 minute Timed Play" )
	PORT_DIPSETTING(    0x38, "20 minute Timed Play" )
	PORT_DIPNAME( 0x40, 0x40, "Buy-In Feature" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START( "COIN" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
INPUT_PORTS_END

static ADDRESS_MAP_START( pce_mem , AS_PROGRAM, 8, uapce_state )
	AM_RANGE( 0x000000, 0x09FFFF) AM_ROM
	AM_RANGE( 0x1F0000, 0x1F1FFF) AM_RAM AM_MIRROR(0x6000)
	AM_RANGE( 0x1FE000, 0x1FE3FF) AM_DEVREADWRITE( "huc6270", huc6270_device, read, write )
	AM_RANGE( 0x1FE400, 0x1FE7FF) AM_DEVREADWRITE( "huc6260", huc6260_device, read, write )
	AM_RANGE( 0x1FE800, 0x1FEBFF) AM_DEVREADWRITE("c6280", c6280_device, c6280_r, c6280_w )
	AM_RANGE( 0x1FEC00, 0x1FEFFF) AM_DEVREADWRITE("maincpu", h6280_device, timer_r, timer_w )
	AM_RANGE( 0x1FF000, 0x1FF3FF) AM_READWRITE(pce_joystick_r, pce_joystick_w )
	AM_RANGE( 0x1FF400, 0x1FF7FF) AM_DEVREADWRITE("maincpu", h6280_device, irq_status_r, irq_status_w )
ADDRESS_MAP_END

static ADDRESS_MAP_START( pce_io , AS_IO, 8, uapce_state )
	AM_RANGE( 0x00, 0x03) AM_DEVREADWRITE( "huc6270", huc6270_device, read, write )
ADDRESS_MAP_END

WRITE_LINE_MEMBER(uapce_state::pce_irq_changed)
{
	m_maincpu->set_input_line(0, state);
}


static MACHINE_CONFIG_START( uapce, uapce_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", H6280, PCE_MAIN_CLOCK/3)
	MCFG_CPU_PROGRAM_MAP(pce_mem)
	MCFG_CPU_IO_MAP(pce_io)

	MCFG_CPU_ADD("sub", Z80, 1400000)
	MCFG_CPU_PROGRAM_MAP(z80_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PCE_MAIN_CLOCK, HUC6260_WPF, 64, 64 + 1024 + 64, HUC6260_LPF, 18, 18 + 242)
	MCFG_SCREEN_UPDATE_DRIVER( pce_common_state, screen_update )
	MCFG_SCREEN_PALETTE("huc6260:palette")

	MCFG_DEVICE_ADD( "huc6260", HUC6260, PCE_MAIN_CLOCK )
	MCFG_HUC6260_NEXT_PIXEL_DATA_CB(DEVREAD16("huc6270", huc6270_device, next_pixel))
	MCFG_HUC6260_TIME_TIL_NEXT_EVENT_CB(DEVREAD16("huc6270", huc6270_device, time_until_next_event))
	MCFG_HUC6260_VSYNC_CHANGED_CB(DEVWRITELINE("huc6270", huc6270_device, vsync_changed))
	MCFG_HUC6260_HSYNC_CHANGED_CB(DEVWRITELINE("huc6270", huc6270_device, hsync_changed))
	MCFG_DEVICE_ADD( "huc6270", HUC6270, 0 )
	MCFG_HUC6270_VRAM_SIZE(0x10000)
	MCFG_HUC6270_IRQ_CHANGED_CB(WRITELINE(uapce_state, pce_irq_changed))

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker","rspeaker")
	MCFG_SOUND_ADD("c6280", C6280, PCE_MAIN_CLOCK/6)
	MCFG_C6280_CPU("maincpu")
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.5)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.5)

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(uapce)
	MCFG_SOUND_ROUTE(0, "rspeaker", 1.00)
MACHINE_CONFIG_END

ROM_START(blazlaz)
	ROM_REGION( 0x0a0000, "maincpu", 0 )
	ROM_LOAD( "ic1.bin", 0x000000, 0x020000, CRC(c86d44fe) SHA1(070512918e4d305db48bbda374eaff2d121909c5) )
	ROM_LOAD( "ic2.bin", 0x020000, 0x020000, CRC(fb813309) SHA1(60d1ea45717a04d776b6837377467e81431f9bc6) )
	ROM_LOAD( "ic3.bin", 0x080000, 0x020000, CRC(d30a2ecf) SHA1(8328b303e2ffeb694b472719e59044d41471725f) )

	ROM_REGION( 0x800, "sub", 0 )
	ROM_LOAD( "u1.bin", 0x0000, 0x800, CRC(f5e538a9) SHA1(19ac9525c9ad6bea1789cc9e63cdb7fe949867d9) )
ROM_END

ROM_START(keith)
	ROM_REGION( 0x0a0000, "maincpu", 0 )
	ROM_LOAD( "keith.ic1", 0x000000, 0x020000, BAD_DUMP CRC(d49d4e0d) SHA1(4ff8e7025dd1023e54a0f5bac52d7cecb7cb1430) )
	ROM_LOAD( "keith.ic2", 0x020000, 0x020000, BAD_DUMP CRC(133e5c8b) SHA1(8ebacd7b3887d10b28d8c672396c8850e8c0cdaf) )

	ROM_REGION( 0x800, "sub", 0 )
	ROM_LOAD( "u1.bin", 0x0000, 0x800, CRC(f5e538a9) SHA1(19ac9525c9ad6bea1789cc9e63cdb7fe949867d9) )
ROM_END

ROM_START(aliencr)
	ROM_REGION( 0x0a0000, "maincpu", 0 )
	ROM_LOAD( "aliencr.bin", 0x000000, 0x040000, CRC(60edf4e1) SHA1(a1e6ad82b66a82c25e0a9e25fb8be370f49c8c2d) )

	ROM_REGION( 0x800, "sub", 0 )
	ROM_LOAD( "u1.bin", 0x0000, 0x800, CRC(f5e538a9) SHA1(19ac9525c9ad6bea1789cc9e63cdb7fe949867d9) )
ROM_END

ROM_START(paclandp)
	ROM_REGION( 0x0a0000, "maincpu", 0 )
	ROM_LOAD( "paclandp.bin", 0x000000, 0x040000, CRC(14fad3ba) SHA1(fc0166da82ed3cf4a4e06fc6c73fd3184ba8bb3b) )

	ROM_REGION( 0x800, "sub", 0 )
	ROM_LOAD( "u1.bin", 0x0000, 0x800, CRC(f5e538a9) SHA1(19ac9525c9ad6bea1789cc9e63cdb7fe949867d9) )
ROM_END

GAME( 1989, blazlaz, 0, uapce, uapce, pce_common_state, pce_common, ROT0, "Hudson Soft", "Blazing Lazers", MACHINE_IMPERFECT_SOUND )
GAME( 1989, keith,   0, uapce, uapce, pce_common_state, pce_common, ROT0, "Hudson Soft", "Keith Courage In Alpha Zones", MACHINE_IMPERFECT_SOUND )
GAME( 1989, aliencr, 0, uapce, uapce, pce_common_state, pce_common, ROT0, "Hudson Soft", "Alien Crush", MACHINE_IMPERFECT_SOUND )
GAME( 1989, paclandp,0, uapce, uapce, pce_common_state, pce_common, ROT0, "Namco", "Pac-Land (United Amusements PC Engine)", MACHINE_IMPERFECT_SOUND )

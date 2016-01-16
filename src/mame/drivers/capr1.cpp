// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Taito Capriccio Z80 crane hardware (let's call it 1st generation)

  These are presumed to be on similar hardware:
  - Capriccio         1991
  - New Capriccio     1992
  - Caprina           1993
  - New Capriccio 2   1993
  - Capriccio Spin    1994
  - Capriccio Spin 2  1996

  The next released game of this series is Capriccio Cyclone, see caprcyc.c
  More games were released after this.

TODO:
- get cspin2 working a bit:
  * unknown reads and writes
  * should have a rombank somewhere
  * what causes the nmi?
  * where's adpcm hooked up?
  * 2 players, 1 7seg led on each cpanel, 3 7seg leds on cranes
- get more dumps, find out technical differences between games and document them
- the rest can come later

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "sound/okim6295.h"


class capr1_state : public driver_device
{
public:
	capr1_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;

	DECLARE_WRITE_LINE_MEMBER(ym2203_irq);
};


/***************************************************************************

  I/O

***************************************************************************/

static ADDRESS_MAP_START( cspin2_map, AS_PROGRAM, 8, capr1_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_RAM
//  AM_RANGE(0xa000, 0xa01f) AM_RAM // wrong
	AM_RANGE(0xc000, 0xc001) AM_DEVREADWRITE("ym", ym2203_device, read, write)
//  AM_RANGE(0xc004, 0xc005) AM_WRITENOP
//  AM_RANGE(0xc008, 0xc009) AM_WRITENOP
//  AM_RANGE(0xc00c, 0xc00d) AM_WRITENOP
//  AM_RANGE(0xc00d, 0xc00d) AM_DEVREADWRITE("oki", okim6295_device, read, write)
ADDRESS_MAP_END



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( cspin2 )
	// just some test stuff
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 )

	PORT_START("IN1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW1:8" )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

WRITE_LINE_MEMBER(capr1_state::ym2203_irq)
{
	m_maincpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

static MACHINE_CONFIG_START( cspin2, capr1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000) // clock frequency unknown
	MCFG_CPU_PROGRAM_MAP(cspin2_map)
	//MCFG_CPU_PERIODIC_INT_DRIVER(capr1_state, nmi_line_pulse, 20)

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym", YM2203, 4000000) // clock frequency unknown
	MCFG_YM2203_IRQ_HANDLER(WRITELINE(capr1_state, ym2203_irq))
	//MCFG_AY8910_PORT_A_READ_CB(IOPORT("IN0"))
	//MCFG_AY8910_PORT_B_READ_CB(IOPORT("IN1"))
	MCFG_SOUND_ROUTE(0, "mono", 0.15)
	MCFG_SOUND_ROUTE(1, "mono", 0.15)
	MCFG_SOUND_ROUTE(2, "mono", 0.15)
	MCFG_SOUND_ROUTE(3, "mono", 0.40)

	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



/***************************************************************************

  Game drivers

***************************************************************************/

/*

CAPRICCIO SPIN 2
(c)1996 TAITO

CPU   : Z80
SOUND : YM2203 MSM6295

E30-01-1.BIN ; MAIN PRG
E30-02.BIN   ; ADPCM
*/

ROM_START( cspin2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "e30-01-1.bin",  0x000000, 0x010000, CRC(30bc0620) SHA1(965d43cbddbd809ebbfdd78ebeb0b87e441d9849) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "e30-02.bin",    0x000000, 0x040000, CRC(519e5474) SHA1(04b344b34d780f2f83207bf6eee2573cc0ce421e) )
ROM_END


GAME (1996, cspin2, 0, cspin2, cspin2, driver_device, 0, ROT0, "Taito", "Capriccio Spin 2", MACHINE_IS_SKELETON_MECHANICAL )

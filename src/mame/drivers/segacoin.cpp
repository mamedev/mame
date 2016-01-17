// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Sega Z80 Coin Pusher hardware

  1992 - Western Dream
  * 2 x Z80 (prg, sound), 3 x YM3438 (6ch), ..
  Hexagon shaped cab, with a toy train riding circles in the top compartment.
  6 players, each with a coin pusher, and a LED roulette on the back panel.

  more...


TODO:
- everything

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
//#include "sound/2612intf.h"


class segacoin_state : public driver_device
{
public:
	segacoin_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
};


/***************************************************************************

  I/O

***************************************************************************/

/* Memory maps */

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, segacoin_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( main_portmap, AS_IO, 8, segacoin_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, segacoin_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_portmap, AS_IO, 8, segacoin_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( westdrm )
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

static MACHINE_CONFIG_START( westdrm, segacoin_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 8000000) // clock frequency unknown
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(main_portmap)

	MCFG_CPU_ADD("audiocpu", Z80, 8000000) // clock frequency unknown
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_IO_MAP(sound_portmap)

	/* no video! */

	/* sound hardware */
	//..
MACHINE_CONFIG_END



/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( westdrm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-15151a.bin",  0x00000, 0x10000, CRC(b0911826) SHA1(77435d2b9c78275f2c21db994d2203528e69fe1f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "epr-15152.bin",   0x00000, 0x10000, CRC(565d6559) SHA1(2c7d961b6dc5020994cbd005efbfd27ccf59569d) ) // mostly empty
	ROM_IGNORE(                           0x10000 )
ROM_END


GAME (1992, westdrm, 0, westdrm, westdrm, driver_device, 0, ROT0, "Sega", "Western Dream", MACHINE_IS_SKELETON_MECHANICAL )

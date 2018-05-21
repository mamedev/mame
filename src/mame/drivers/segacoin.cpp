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
#include "machine/315_5338a.h"
#include "machine/pit8253.h"
#include "sound/2612intf.h"
#include "speaker.h"


class segacoin_state : public driver_device
{
public:
	segacoin_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	void westdrm(machine_config &config);
	void main_map(address_map &map);
	void main_portmap(address_map &map);
	void sound_map(address_map &map);
	void sound_portmap(address_map &map);
};


/***************************************************************************

  I/O

***************************************************************************/

/* Memory maps */

void segacoin_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xe000, 0xffff).ram();
}

void segacoin_state::main_portmap(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).nopw(); // watchdog?
	map(0x10, 0x13).rw("pit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x20, 0x2f).rw("io", FUNC(sega_315_5338a_device::read), FUNC(sega_315_5338a_device::write));
}


void segacoin_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xe000, 0xffff).ram();
}

void segacoin_state::sound_portmap(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ym0", FUNC(ym3438_device::read), FUNC(ym3438_device::write));
	map(0x40, 0x43).rw("ym1", FUNC(ym3438_device::read), FUNC(ym3438_device::write));
	map(0x80, 0x83).rw("ym2", FUNC(ym3438_device::read), FUNC(ym3438_device::write));
}



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

MACHINE_CONFIG_START(segacoin_state::westdrm)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", Z80, 8000000) // clock frequency unknown
	MCFG_DEVICE_PROGRAM_MAP(main_map)
	MCFG_DEVICE_IO_MAP(main_portmap)

	MCFG_DEVICE_ADD("pit", PIT8253, 0)
	MCFG_PIT8253_CLK2(1000000) // clock frequency unknown

	MCFG_DEVICE_ADD("io", SEGA_315_5338A, 0)

	MCFG_DEVICE_ADD("audiocpu", Z80, 8000000) // clock frequency unknown
	MCFG_DEVICE_PROGRAM_MAP(sound_map)
	MCFG_DEVICE_IO_MAP(sound_portmap)

	/* no video! */

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	MCFG_DEVICE_ADD("ym0", YM3438, 8000000) // clock frequency unknown
	MCFG_SOUND_ROUTE(0, "mono", 0.40)
	MCFG_SOUND_ROUTE(1, "mono", 0.40)

	MCFG_DEVICE_ADD("ym1", YM3438, 8000000) // clock frequency unknown
	MCFG_SOUND_ROUTE(0, "mono", 0.40)
	MCFG_SOUND_ROUTE(1, "mono", 0.40)

	MCFG_DEVICE_ADD("ym2", YM3438, 8000000) // clock frequency unknown
	MCFG_SOUND_ROUTE(0, "mono", 0.40)
	MCFG_SOUND_ROUTE(1, "mono", 0.40)
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


GAME( 1992, westdrm, 0, westdrm, westdrm, segacoin_state, empty_init, ROT0, "Sega", "Western Dream", MACHINE_IS_SKELETON_MECHANICAL )

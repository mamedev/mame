// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Midway's Submarine, game number 760

TODO:
- needs extensive interactive artwork
- discrete sound
- identify sensors

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"

#include "submar.lh"


class submar_state : public driver_device
{
public:
	submar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;

	DECLARE_READ8_MEMBER(submar_sensor0_r);
	DECLARE_READ8_MEMBER(submar_sensor1_r);
	DECLARE_WRITE8_MEMBER(submar_motor_w);
	DECLARE_WRITE8_MEMBER(submar_lamp_w);
	DECLARE_WRITE8_MEMBER(submar_solenoid_w);
	DECLARE_WRITE8_MEMBER(submar_sound_w);
	DECLARE_WRITE8_MEMBER(submar_led_w);
	DECLARE_WRITE8_MEMBER(submar_irq_clear_w);
};


/***************************************************************************

  I/O, Memorymap

***************************************************************************/

READ8_MEMBER(submar_state::submar_sensor0_r)
{
	// ?
	return 0;
}

READ8_MEMBER(submar_state::submar_sensor1_r)
{
	// ?
	return (ioport("IN1")->read() & 0x70) | 0x8f;
}

WRITE8_MEMBER(submar_state::submar_motor_w)
{
	// d0: torpedo follow
	// d1: ship movement
	// d2: n/c
	// d3: torpedo timing
	// d4: torpedo timing
	// d5: target shipsink
	// d6: stir water
	// d7: n/c
	for (int i = 0; i < 8; i++)
		output().set_indexed_value("motor", i, data >> i & 1);
}

WRITE8_MEMBER(submar_state::submar_lamp_w)
{
	// d0: torpedo
	// d1: target ship on water
	// d2: target ship under water
	// d3: explosion
	// d4: extended play
	// d5: game over
	// d6: front ship hit
	// d7: scenery
	for (int i = 0; i < 8; i++)
		output().set_lamp_value(i, data >> i & 1);
}

WRITE8_MEMBER(submar_state::submar_solenoid_w)
{
	// d0-d4: ship1-5
	// d5-d7: n/c
	for (int i = 0; i < 8; i++)
		output().set_indexed_value("solenoid", i, data >> i & 1);
}

WRITE8_MEMBER(submar_state::submar_sound_w)
{
	// d0: torpedo
	// d1: "summer"
	// d2: ship hit
	// d3: target ship hit
	// d4: sonar circuit
	// d5: sonar circuit
	// d6: n/c
	// d7: n/c
}

WRITE8_MEMBER(submar_state::submar_led_w)
{
	// 7447 (BCD to LED segment)
	const UINT8 _7447_map[16] =
		{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0x00 };

	// 2 digits per write. port 4: time, port 5: score
	output().set_digit_value((offset << 1 & 2) | 0, _7447_map[data >> 4]);
	output().set_digit_value((offset << 1 & 2) | 1, _7447_map[data & 0x0f]);
}

WRITE8_MEMBER(submar_state::submar_irq_clear_w)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}


static ADDRESS_MAP_START( submar_map, AS_PROGRAM, 8, submar_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x207f) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( submar_portmap, AS_IO, 8, submar_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READWRITE(submar_sensor0_r, submar_motor_w)
	AM_RANGE(0x01, 0x01) AM_READWRITE(submar_sensor1_r, submar_lamp_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(submar_solenoid_w)
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSW") AM_WRITE(submar_sound_w)
	AM_RANGE(0x04, 0x05) AM_WRITE(submar_led_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x07, 0x07) AM_WRITE(submar_irq_clear_w)
ADDRESS_MAP_END



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( submar )
	PORT_START("IN1")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x00, "Extended Time" ) PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(    0x00, "15 seconds at 1000" ) PORT_CONDITION("DSW", 0x30, EQUALS, 0x00) // @ 60 seconds
	PORT_DIPSETTING(    0x04, "15 seconds at 2000" ) PORT_CONDITION("DSW", 0x30, EQUALS, 0x00)
	PORT_DIPSETTING(    0x08, "30 seconds at 2000" ) PORT_CONDITION("DSW", 0x30, EQUALS, 0x00)
	PORT_DIPSETTING(    0x0c, "30 seconds at 3000" ) PORT_CONDITION("DSW", 0x30, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "15 seconds at 2000" ) PORT_CONDITION("DSW", 0x30, EQUALS, 0x10) // @ 70 seconds
	PORT_DIPSETTING(    0x04, "15 seconds at 3000" ) PORT_CONDITION("DSW", 0x30, EQUALS, 0x10)
	PORT_DIPSETTING(    0x08, "30 seconds at 3000" ) PORT_CONDITION("DSW", 0x30, EQUALS, 0x10)
	PORT_DIPSETTING(    0x0c, "30 seconds at 4000" ) PORT_CONDITION("DSW", 0x30, EQUALS, 0x10)
	PORT_DIPSETTING(    0x00, "15 seconds at 3000" ) PORT_CONDITION("DSW", 0x30, EQUALS, 0x20) // @ 80 seconds
	PORT_DIPSETTING(    0x04, "15 seconds at 4000" ) PORT_CONDITION("DSW", 0x30, EQUALS, 0x20)
	PORT_DIPSETTING(    0x08, "30 seconds at 4000" ) PORT_CONDITION("DSW", 0x30, EQUALS, 0x20)
	PORT_DIPSETTING(    0x0c, "30 seconds at 5000" ) PORT_CONDITION("DSW", 0x30, EQUALS, 0x20)
	PORT_DIPSETTING(    0x00, "15 seconds at 4000" ) PORT_CONDITION("DSW", 0x30, EQUALS, 0x30) // @ 90 seconds
	PORT_DIPSETTING(    0x04, "15 seconds at 5000" ) PORT_CONDITION("DSW", 0x30, EQUALS, 0x30)
	PORT_DIPSETTING(    0x08, "30 seconds at 5000" ) PORT_CONDITION("DSW", 0x30, EQUALS, 0x30)
	PORT_DIPSETTING(    0x0c, "30 seconds at 6000" ) PORT_CONDITION("DSW", 0x30, EQUALS, 0x30)
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Game_Time ) ) PORT_DIPLOCATION("SW:5,6")
	PORT_DIPSETTING(    0x00, "60 seconds" )
	PORT_DIPSETTING(    0x10, "70 seconds" )
	PORT_DIPSETTING(    0x20, "80 seconds" )
	PORT_DIPSETTING(    0x30, "90 seconds" )
	PORT_DIPNAME( 0x40, 0x00, "Alignment Mode" ) PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_HIGH, "SW:8" )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

static MACHINE_CONFIG_START( submar, submar_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_19_968MHz/8)
	MCFG_CPU_PERIODIC_INT_DRIVER(submar_state, irq0_line_assert, 124.675) // 555 IC
	MCFG_CPU_PROGRAM_MAP(submar_map)
	MCFG_CPU_IO_MAP(submar_portmap)

	/* no video! */

	/* sound hardware */
	//...
MACHINE_CONFIG_END



/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( submar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sub.a1", 0x0000, 0x0800, CRC(bcef5db4) SHA1(8ae5099672fbdb7bcdc617e1f8cbc5435fbb738a) )
	ROM_LOAD( "sub.a2", 0x0800, 0x0800, CRC(f5780dd0) SHA1(f775dd6f64a730a2fb6c9baf5787698434150bc5) )
ROM_END


GAMEL( 1979, submar, 0, submar, submar, driver_device, 0, ROT0, "Midway", "Submarine (Midway)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL, layout_submar )

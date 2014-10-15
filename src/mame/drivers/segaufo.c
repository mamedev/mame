// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Sega UFO Catcher, Z80 type hardware
  The list underneath is not complete. A # before the name means it's not dumped yet.

  1st gen
    * ?
  - # UFO Catcher (1985)
  
  2nd gen:
    * ?
  - # UFO Catcher DX (1987)
  - # UFO Catcher DX II (1987)

  3rd gen - UFO brd
    * Z80, 2 Sega 315-5296(I/O), YM3438, NEC uPD71054C
  - # Dream Town (1990)
  - New UFO Catcher (1991) (2P) - probably the most popular cabinet of all UFO Catcher series
  - UFO Catcher Mini (1991) (1P)
  - # UFO Catcher Sega Sonic (1991)
  - # School Kids (1993)
  
  4th gen - EX brd
    * Z80, 2 Sega 315-5296(I/O), YM3438, NEC uPD71054C, optional NEC uPD7759C
  - # Dream Palace (1992)
  - # Dream Kitchen (1994)
  - # UFO Catcher Excellent (1994)
  - # UFO A La Carte (1996)
  - UFO Catcher 21 (1996) (2P)
  - UFO Catcher 800 (1998) (1P)
  - # Baby UFO (1998)
  - # Prize Sensor (1998)
  
  More games were released after 2000, assumed to be on more modern hardware.


  TODO:
  - make the other games work (for now only newufo+clones work)
  - add layout

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/pit8253.h"
#include "machine/315-5296.h"
#include "sound/2612intf.h"


/* simulation parameters */
// x/y/z cabinet dimensions per player (motor range)
#define CABINET_WIDTH	400
#define CABINET_DEPTH   400
#define CABINET_HEIGHT  250

// x/y/z motor speed in hertz
#define MOTOR_SPEED     100

// crane size (stepper motor range)
// note: the game expects this to be around 350 steps per quarter rotation
#define CRANE_SIZE      350



class ufo_state : public driver_device
{
public:
	ufo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;
	
	struct Player
	{
		struct Motor
		{
			UINT8 running;
			UINT8 direction;
			float position;
			float speed;
		} motor[4];
	} m_player[2];
	
	UINT8 m_stepper;
	
	void motor_tick(int p, int m);

	DECLARE_WRITE_LINE_MEMBER(pit_out0);
	DECLARE_WRITE_LINE_MEMBER(pit_out1);
	DECLARE_WRITE_LINE_MEMBER(pit_out2);
	DECLARE_WRITE_LINE_MEMBER(ym3438_irq);
	DECLARE_READ8_MEMBER(crane_limits_r);
	DECLARE_WRITE8_MEMBER(stepper_w);
	DECLARE_WRITE8_MEMBER(cp_lamps_w);
	DECLARE_WRITE8_MEMBER(cp_digits_w);
	DECLARE_WRITE8_MEMBER(crane_xyz_w);
	DECLARE_WRITE8_MEMBER(ufo_lamps_w);
	
	virtual void machine_reset();
	virtual void machine_start();
	TIMER_DEVICE_CALLBACK_MEMBER(simulate_xyz);
};




void ufo_state::motor_tick(int p, int m)
{
	float delta = m_player[p].motor[m].speed;
	if (m_player[p].motor[m].direction)
		delta = -delta;
	
	if (m_player[p].motor[m].running)
		m_player[p].motor[m].position += delta;
	
	if (m_player[p].motor[m].position < 0)
		m_player[p].motor[m].position = 0;
	if (m_player[p].motor[m].position > 1)
		m_player[p].motor[m].position = 1;
}


TIMER_DEVICE_CALLBACK_MEMBER(ufo_state::simulate_xyz)
{
	for (int p = 0; p < 2; p++)
		for (int m = 0; m < 3; m++)
			motor_tick(p, m);
}

/***************************************************************************

  I/O

***************************************************************************/

WRITE_LINE_MEMBER(ufo_state::pit_out0)
{
	// ?
}

WRITE_LINE_MEMBER(ufo_state::pit_out1)
{
	// ?
}

WRITE_LINE_MEMBER(ufo_state::pit_out2)
{
	// NMI?
	if (state)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


WRITE8_MEMBER(ufo_state::stepper_w)
{
	for (int p = 0; p < 2; p++)
	{
		// The crane stepper motor is set up as a rotating ellipse disc under the crane,
		// controlled with 4 output bits connected to a Toshiba TB6560AHQ motor driver.
		// I don't know which bits connect to which pins specifically.
		// To run it, the game writes a continuous sequence of $5, $9, $a, $6, ..
		static const UINT8 sequence[4] =
			{ 0x5, 0x9, 0xa, 0x6 };

		// d0-d3: p1, d4-d7: p2
		UINT8 cur = data >> (p*4) & 0xf;
		UINT8 prev = m_stepper >> (p*4) & 0xf;
		
		for (int i = 0; i < 4; i++)
		{
			if (sequence[i] == prev && sequence[(i+1) & 3] == cur)
			{
				m_player[p].motor[3].running = 1;
				motor_tick(p, 3);
				
				// change direction after each quarter rotate
				if (m_player[p].motor[3].position <= 0 || m_player[p].motor[3].position >= 1)
					m_player[p].motor[3].direction ^= 1;
				
				break;
			}
		}
	}
	
	m_stepper = data;
}

WRITE8_MEMBER(ufo_state::cp_lamps_w)
{
	// d0-d3: p1/p2 button lamps
	// other bits: ?
	for (int i = 0; i < 4; i++)
		output_set_lamp_value(i, ~data >> i & 1);
}

WRITE8_MEMBER(ufo_state::cp_digits_w)
{
	static const UINT8 lut_7448[0x10] =
		{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0x00 };
	
	// d0-d3: cpanel digit
	// other bits: ?
	output_set_digit_value(offset & 1, lut_7448[data & 0xf]);
}

WRITE8_MEMBER(ufo_state::crane_xyz_w)
{
	int p = offset & 1;
	
	// d0: x/z axis (0:x, 1:z + halt x/y)
	// d1: x/z direction
	// d2: y direction
	// d3: x/z running
	// d4: y running
	// other bits: ?
	m_player[p].motor[0].running = (data & 9) == 8;
	m_player[p].motor[0].direction = data & 2;
	m_player[p].motor[1].running = (data & 0x11) == 0x10;
	m_player[p].motor[1].direction = data & 4;
	m_player[p].motor[2].running = (data & 9) == 9;
	m_player[p].motor[2].direction = data & 2;
}

WRITE8_MEMBER(ufo_state::ufo_lamps_w)
{
	;
}


READ8_MEMBER(ufo_state::crane_limits_r)
{
	int p = offset & 1;
	UINT8 ret = 0xff;

	// d0: left limit sw (right for p2)
	// d1: right limit sw (left for p2)
	// d2: back limit sw
	// d3: front limit sw
	// d4: down limit sw
	// d5: up limit sw
	for (int i = 0; i < 3; i++)
	{
		ret ^= (m_player[p].motor[i].position >= 1) << (i*2 + 0);
		ret ^= (m_player[p].motor[i].position <= 0) << (i*2 + 1);
	}

	// d6: crane open sensor (reflective sticker on the stepper motor rotation disc)
	if (m_player[p].motor[3].position >= 0.97)
		ret ^= 0x40;
	
	// d7: ?

	return ret;
}




static ADDRESS_MAP_START( ufo_map, AS_PROGRAM, 8, ufo_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( ufo_portmap, AS_PROGRAM, 8, ufo_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("pit", pit8254_device, read, write)
	AM_RANGE(0x40, 0x43) AM_DEVREADWRITE("ym", ym3438_device, read, write)
	AM_RANGE(0x80, 0x8f) AM_DEVREADWRITE("io1", sega_315_5296_device, read, write)
	AM_RANGE(0xc0, 0xcf) AM_DEVREADWRITE("io2", sega_315_5296_device, read, write)
ADDRESS_MAP_END



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( ufo )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("P1 Coin 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("P1 Coin 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("P1 Test")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P1 Service Coin")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("P1 Credit Clear")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Prize Fell")

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("P2 Coin 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("P2 Coin 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Test") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("P2 Service Coin")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE4 ) PORT_NAME("P2 Credit Clear")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Prize Fell")

	PORT_START("DSW1") // coinage
	PORT_DIPNAME( 0x01, 0x01, "UNK1-01" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "UNK1-02" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "UNK1-04" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "UNK1-08" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "UNK1-10" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "UNK1-20" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "UNK1-40" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "UNK1-80" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "UNK2-01 Demo Music Off" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "UNK2-02" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "UNK2-04 Initial Motor Test" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "UNK2-08" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "UNK2-10 Enable Prize Sensor" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "UNK2-20" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "UNK2-40" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "UNK2-80" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

void ufo_state::machine_reset()
{
}

void ufo_state::machine_start()
{
	// init/zerofill
	static const float motor_speeds[4] =
		{ 1.0/CABINET_WIDTH, 1.0/CABINET_DEPTH, 1.0/CABINET_HEIGHT, 1.0/CRANE_SIZE };
	
	for (int p = 0; p < 2; p++)
	{
		for (int m = 0; m < 4; m++)
		{
			m_player[p].motor[m].running = 0;
			m_player[p].motor[m].direction = 0;
			m_player[p].motor[m].position = 0.5;
			m_player[p].motor[m].speed = motor_speeds[m];
		}
	}
}

WRITE_LINE_MEMBER(ufo_state::ym3438_irq)
{
	m_maincpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

static MACHINE_CONFIG_START( ufo, ufo_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 8000000)
	MCFG_CPU_PROGRAM_MAP(ufo_map)
	MCFG_CPU_IO_MAP(ufo_portmap)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("motor_timer", ufo_state, simulate_xyz, attotime::from_hz(MOTOR_SPEED))
	
	MCFG_DEVICE_ADD("io1", SEGA_315_5296, 16000000)
	// all ports set to input
	MCFG_315_5296_IN_PORTA_CB(READ8(ufo_state, crane_limits_r))
	MCFG_315_5296_IN_PORTB_CB(READ8(ufo_state, crane_limits_r))
//	MCFG_315_5296_IN_PORTC_CB(NOOP)
//	MCFG_315_5296_IN_PORTD_CB(NOOP)
	MCFG_315_5296_IN_PORTE_CB(IOPORT("P1"))
	MCFG_315_5296_IN_PORTF_CB(IOPORT("DSW1"))
	MCFG_315_5296_IN_PORTG_CB(IOPORT("DSW2"))
	MCFG_315_5296_IN_PORTH_CB(IOPORT("P2"))

	MCFG_DEVICE_ADD("io2", SEGA_315_5296, 16000000)
	// all ports set to output
	MCFG_315_5296_OUT_PORTA_CB(WRITE8(ufo_state, stepper_w))
	MCFG_315_5296_OUT_PORTB_CB(WRITE8(ufo_state, cp_lamps_w))
	MCFG_315_5296_OUT_PORTC_CB(WRITE8(ufo_state, cp_digits_w))
	MCFG_315_5296_OUT_PORTD_CB(WRITE8(ufo_state, cp_digits_w))
	MCFG_315_5296_OUT_PORTE_CB(WRITE8(ufo_state, crane_xyz_w))
	MCFG_315_5296_OUT_PORTF_CB(WRITE8(ufo_state, crane_xyz_w))
	MCFG_315_5296_OUT_PORTG_CB(WRITE8(ufo_state, ufo_lamps_w))
//	MCFG_315_5296_OUT_PORTH_CB(NOOP)

	MCFG_DEVICE_ADD("pit", PIT8254, 0) // uPD71054C, configuration is unknown
	MCFG_PIT8253_CLK0(8000000/256)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(ufo_state, pit_out0))
	MCFG_PIT8253_CLK1(8000000/256)
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(ufo_state, pit_out1))
	MCFG_PIT8253_CLK2(8000000/256)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(ufo_state, pit_out2))

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym", YM3438, 8000000)
	MCFG_YM2612_IRQ_HANDLER(WRITELINE(ufo_state, ym3438_irq))
	MCFG_SOUND_ROUTE(0, "mono", 0.40)
	MCFG_SOUND_ROUTE(1, "mono", 0.40)
MACHINE_CONFIG_END



/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( newufo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-13896.bin",   0x000000, 0x010000, CRC(ca94be57) SHA1(acb6a22940c5e9ce639c7c30eb3948324b223090) )
ROM_END

ROM_START( newufo_sonic )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-14124.bin",   0x000000, 0x010000, CRC(2bdbad89) SHA1(10de4b266471a68083ec4bc439b301b6587ccfd6) )
ROM_END

ROM_START( newufo_nfl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-15261.bin",   0x000000, 0x010000, CRC(338c00d3) SHA1(03152956c6f1e4d5a1a11ee49f94a8c5eb550815) )
ROM_END

ROM_START( newufo_xmas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-15340.bin",   0x000000, 0x010000, CRC(6287c9ac) SHA1(bc6bc84bb432424e1d25e01113e8e331fa64f96f) )
ROM_END


ROM_START( ufomini )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-14355.bin",   0x000000, 0x010000, CRC(fbc969c5) SHA1(4a99dcd36bc48b6472988e0bc679fd61af17359c) )
ROM_END


ROM_START( ufo21 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-19063a.bin",  0x000000, 0x010000, CRC(2e13e3e9) SHA1(6908f7db79c1a1da4ebc0456afc50ff18f2e8cf3) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "epr-19064.bin",   0x000000, 0x040000, CRC(ab62c1f0) SHA1(8791a88546ae69e710e128ffc2ea6e9b464f0631) )
ROM_END

ROM_START( ufo800 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-20413a.bin",  0x000000, 0x010000, CRC(36e9da6d) SHA1(8e1dbf8b24bc31be7de28f4d562838c291af7c7b) )
ROM_END


GAME (1991, newufo,       0,      ufo, ufo, driver_device, 0, ROT0, "Sega", "New UFO Catcher (standard)", GAME_MECHANICAL )
GAME (1991, newufo_sonic, newufo, ufo, ufo, driver_device, 0, ROT0, "Sega", "New UFO Catcher (Sonic The Hedgehog)", GAME_MECHANICAL )
GAME (1991, newufo_nfl,   newufo, ufo, ufo, driver_device, 0, ROT0, "Sega", "New UFO Catcher (Team NFL)", GAME_MECHANICAL )
GAME (1991, newufo_xmas,  newufo, ufo, ufo, driver_device, 0, ROT0, "Sega", "New UFO Catcher (Christmas season ROM kit)", GAME_MECHANICAL )
GAME (1991, ufomini,      0,      ufo, ufo, driver_device, 0, ROT0, "Sega", "UFO Catcher Mini", GAME_NOT_WORKING | GAME_MECHANICAL )
GAME (1996, ufo21,        0,      ufo, ufo, driver_device, 0, ROT0, "Sega", "UFO Catcher 21", GAME_NOT_WORKING | GAME_MECHANICAL )
GAME (1998, ufo800,       0,      ufo, ufo, driver_device, 0, ROT0, "Sega", "UFO Catcher 800", GAME_NOT_WORKING | GAME_MECHANICAL )

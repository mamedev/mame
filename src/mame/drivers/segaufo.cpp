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
    * Z80, 2 Sega 315-5296(I/O), 315-5338A, YM3438, NEC uPD71054C, optional NEC uPD7759C
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
  - add dipswitches
  - prize sensor for ufo21/ufo800

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/pit8253.h"
#include "machine/315_5296.h"
#include "sound/2612intf.h"
#include "sound/upd7759.h"

// the layouts are very similar to eachother
#include "newufo.lh"
#include "ufomini.lh"
#include "ufo21.lh"
#include "ufo800.lh"


/* simulation parameters */
// x/y/z cabinet dimensions per player (motor range)
#define CABINET_WIDTH   400
#define CABINET_DEPTH   400
#define CABINET_HEIGHT  300

// x/y/z motor speed in hertz
#define MOTOR_SPEED     100

// crane size (stepper motor range)
// note: UFO board/EX board expects this to be around 350 steps per quarter rotation
#define CRANE_SIZE      350



class ufo_state : public driver_device
{
public:
	ufo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_io1(*this, "io1"),
		m_io2(*this, "io2"),
		m_upd(*this, "upd")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<sega_315_5296_device> m_io1;
	required_device<sega_315_5296_device> m_io2;
	optional_device<upd7759_device> m_upd;

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

	DECLARE_READ8_MEMBER(ex_crane_limits_r);
	DECLARE_READ8_MEMBER(ex_crane_open_r);
	DECLARE_WRITE8_MEMBER(ex_stepper_w);
	DECLARE_WRITE8_MEMBER(ex_cp_lamps_w);
	DECLARE_WRITE8_MEMBER(ex_crane_xyz_w);
	DECLARE_WRITE8_MEMBER(ex_ufo21_lamps_w);
	DECLARE_WRITE8_MEMBER(ex_ufo800_lamps_w);
	DECLARE_READ8_MEMBER(ex_upd_busy_r);
	DECLARE_WRITE8_MEMBER(ex_upd_start_w);

	virtual void machine_reset();
	virtual void machine_start();
	TIMER_DEVICE_CALLBACK_MEMBER(simulate_xyz);
	TIMER_DEVICE_CALLBACK_MEMBER(update_info);
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


TIMER_DEVICE_CALLBACK_MEMBER(ufo_state::update_info)
{
	// output ufo motor positions
	// 0 X: 000 = right,  100 = left (player 1)
	// 1 Y: 000 = front,  100 = back
	// 2 Z: 000 = up,     100 = down
	// 3 C: 000 = closed, 100 = open
	for (int p = 0; p < 2; p++)
		for (int m = 0; m < 4; m++)
			output_set_indexed_value("counter", p*4 + m, (UINT8)(m_player[p].motor[m].position * 100));

#if 0
	char msg1[0x100] = {0};
	char msg2[0x100] = {0};
	for (int i = 0; i < 8; i++)
	{
		sprintf(msg2, "%02X ", m_io2->debug_peek_output(i));
		strcat(msg1, msg2);
	}
	popmessage("%s", msg1);
#endif
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
	// NMI?
	if (state)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

WRITE_LINE_MEMBER(ufo_state::pit_out2)
{
	// ?
}


/* generic / UFO board handlers */

/* io1 */

READ8_MEMBER(ufo_state::crane_limits_r)
{
	int p = offset & 1;
	UINT8 ret = 0x7f;

	// d0: left limit sw (right for p2)
	// d1: right limit sw (left for p2)
	// d2: back limit sw
	// d3: front limit sw
	// d4: down limit sw
	// d5: up limit sw
	for (int m = 0; m < 3; m++)
	{
		ret ^= (m_player[p].motor[m].position >= 1) << (m*2 + 0);
		ret ^= (m_player[p].motor[m].position <= 0) << (m*2 + 1);
	}

	// d6: crane open sensor (reflective sticker on the stepper motor rotation disc)
	if (m_player[p].motor[3].position >= 0.97f)
		ret ^= 0x40;

	// d7: prize sensor (mirror?)
	ret |= (ioport(p ? "IN2" : "IN1")->read() & 0x80);

	return ret;
}

/* io2 */

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
	// d0-d3: ufo leds (2 bits per player)
	// 3 sets of two red/green leds, each set is wired to the same control 2 bits
	// 00 = off,   off
	// 11 = red,   red
	// 01 = green, red
	// 10 = red,   green
	output_set_lamp_value(10, data & 3);
	output_set_lamp_value(11, data >> 2 & 3);

	// d4,d5: ?
	// d6,d7: coincounters
	coin_counter_w(machine(), 0, data & 0x40); // 100 Y
	coin_counter_w(machine(), 1, data & 0x80); // 500 Y
}


/* EX board specific handlers */

/* io1 */

READ8_MEMBER(ufo_state::ex_crane_limits_r)
{
	int p = offset & 1;
	UINT8 ret = 0xf0;

	// d0: left limit sw (invert)
	// d1: right limit sw (invert)
	// d2: back limit sw (invert)
	// d3: front limit sw (invert)
	// d4: ?
	// d5: down limit sw
	// d6: up limit sw
	// d7: ?
	for (int m = 0; m < 3; m++)
	{
		int shift = (m*2) + (m == 2);
		ret ^= (m_player[p].motor[m].position >= 1) << shift;
		ret ^= (m_player[p].motor[m].position <= 0) << (shift+1);
	}

	return ret;
}

READ8_MEMBER(ufo_state::ex_crane_open_r)
{
	// d0-d3: p1, d4-d7: p2
	UINT8 ret = 0xff;

	for (int p = 0; p < 2; p++)
	{
		// d0: crane open sensor
		if (m_player[p].motor[3].position >= 0.97f)
			ret ^= (1 << (p*4));

		// d1: coincounter is plugged in (ufo800 gives error 14 otherwise)
		// d2,d3: ?
	}

	return ret;
}

/* io2 */

WRITE8_MEMBER(ufo_state::ex_stepper_w)
{
	// stepper motor sequence is: 6 c 9 3 6 c 9 3..
	// which means d0 and d3 are swapped when compared with UFO board hardware
	stepper_w(space, offset, BITSWAP8(data,4,6,5,7,0,2,1,3));
}

WRITE8_MEMBER(ufo_state::ex_cp_lamps_w)
{
	// d0,d1,d4,d5: p1/p2 button lamps
	for (int i = 0; i < 4; i++)
		output_set_lamp_value(i, ~data >> ((i&1) + (i&2) * 2) & 1);

	// d2,d3,d6,d7: p1/p2 coincounters
	for (int i = 0; i < 4; i++)
		coin_counter_w(machine(), i, data >> (2 + (i&1) + (i&2) * 2) & 1);
}

WRITE8_MEMBER(ufo_state::ex_crane_xyz_w)
{
	int p = offset & 1;

	// more straightforward setup than on UFO board hardware
	// d0: move left
	// d1: move right
	// d2: move back
	// d3: move front
	// d4: move down
	// d5: move up
	for (int m = 0; m < 3; m++)
	{
		int bits = data >> (m*2) & 3;
		m_player[p].motor[m].running = (bits == 1 || bits == 2) ? 1 : 0;
		m_player[p].motor[m].direction = bits & 2;
	}
}

WRITE8_MEMBER(ufo_state::ex_ufo800_lamps_w)
{
	// d0-d4: 5 red leds on ufo
	// other bits: ?
	for (int i = 0; i < 5; i++)
		output_set_lamp_value(10 + i, data >> i & 1);
}

/* 315-5338A */

WRITE8_MEMBER(ufo_state::ex_ufo21_lamps_w)
{
	// d0: ? (ufo21 reads from it too, but value is discarded)
	// d1-d6 are the 6 red leds on each ufo
	// d7: ?
	for (int i = 1; i < 7; i++)
		output_set_lamp_value(10 + offset * 10 + i, data >> i & 1);
}

WRITE8_MEMBER(ufo_state::ex_upd_start_w)
{
	// d0: upd7759c start sample
	// other bits: unused?
	m_upd->start_w(~data & 1);
}

READ8_MEMBER(ufo_state::ex_upd_busy_r)
{
	// d0: upd7759c busy
	// other bits: unused?
	int d0 = m_upd->busy_r() ? 1 : 0;
	return 0xfe | d0;
}


/* Memory maps */

static ADDRESS_MAP_START( ufo_map, AS_PROGRAM, 8, ufo_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( ufo_portmap, AS_IO, 8, ufo_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("pit", pit8254_device, read, write)
	AM_RANGE(0x40, 0x43) AM_DEVREADWRITE("ym", ym3438_device, read, write)
	AM_RANGE(0x80, 0xbf) AM_DEVREADWRITE("io1", sega_315_5296_device, read, write)
	AM_RANGE(0xc0, 0xff) AM_DEVREADWRITE("io2", sega_315_5296_device, read, write)
ADDRESS_MAP_END


static ADDRESS_MAP_START( ex_ufo21_portmap, AS_IO, 8, ufo_state )
	AM_RANGE(0x20, 0x20) AM_DEVWRITE("upd", upd7759_device, port_w)
	AM_RANGE(0x60, 0x60) AM_WRITE(ex_upd_start_w) AM_READNOP
	AM_RANGE(0x61, 0x61) AM_READ(ex_upd_busy_r)
	AM_RANGE(0x64, 0x65) AM_WRITE(ex_ufo21_lamps_w) AM_READNOP
//  AM_RANGE(0x68, 0x68) AM_WRITENOP // ?
	AM_IMPORT_FROM( ufo_portmap )
ADDRESS_MAP_END

static ADDRESS_MAP_START( ex_ufo800_portmap, AS_IO, 8, ufo_state )
//  AM_RANGE(0x60, 0x67) AM_NOP // unused?
//  AM_RANGE(0x68, 0x68) AM_WRITENOP // ?
	AM_IMPORT_FROM( ufo_portmap )
ADDRESS_MAP_END



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( newufo )
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("P1 Coin 1") // 100 Y
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("P1 Coin 2") // 500 Y
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("P1 Test")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P1 Service Coin")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("P1 Credit Clear")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("P1 Prize Sensor")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("P2 Coin 1") // 100 Y
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("P2 Coin 2") // 500 Y
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Test") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("P2 Service Coin")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE4 ) PORT_NAME("P2 Credit Clear")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Prize Sensor")

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
	PORT_DIPNAME( 0x10, 0x10, "UNK2-10 Disable Prize Sensor" )
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

static INPUT_PORTS_START( ufomini )
	PORT_INCLUDE( newufo )

	PORT_MODIFY("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( ufo21 )
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Test Button")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P1 Service Coin")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("P1 Coin 1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("P2 Service Coin")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("P2 Coin 1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_PLAYER(2)

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
	PORT_DIPNAME( 0x01, 0x01, "UNK2-01" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "UNK2-02" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "UNK2-04" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "UNK2-08 Demo Music On" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "UNK2-10" )
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

static INPUT_PORTS_START( ufo800 )
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Test Button")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("P1 Service Coin")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("P1 Coin 1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("P1 Coin 2")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

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
	PORT_DIPNAME( 0x01, 0x01, "UNK2-01 BGM Select" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "UNK2-02 BGM Select" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "UNK2-04" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "UNK2-08" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "UNK2-10" )
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
	// init/zerofill/register for savestates
	static const float motor_speeds[4] =
		{ 1.0f/CABINET_WIDTH, 1.0f/CABINET_DEPTH, 1.0f/CABINET_HEIGHT, 1.0f/CRANE_SIZE };

	for (int m = 0; m < 4; m++)
	{
		for (auto & elem : m_player)
		{
			elem.motor[m].running = 0;
			elem.motor[m].direction = 0;
			elem.motor[m].position = 0.5;
			elem.motor[m].speed = motor_speeds[m];
		}

		save_item(NAME(m_player[0].motor[m].running), m);
		save_item(NAME(m_player[0].motor[m].direction), m);
		save_item(NAME(m_player[0].motor[m].position), m);

		save_item(NAME(m_player[1].motor[m].running), m);
		save_item(NAME(m_player[1].motor[m].direction), m);
		save_item(NAME(m_player[1].motor[m].position), m);
	}

	m_stepper = 0;
	save_item(NAME(m_stepper));
}

WRITE_LINE_MEMBER(ufo_state::ym3438_irq)
{
	m_maincpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

static MACHINE_CONFIG_START( newufo, ufo_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_16MHz/2)
	MCFG_CPU_PROGRAM_MAP(ufo_map)
	MCFG_CPU_IO_MAP(ufo_portmap)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("motor_timer", ufo_state, simulate_xyz, attotime::from_hz(MOTOR_SPEED))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("update_timer", ufo_state, update_info, attotime::from_hz(60))

	MCFG_DEVICE_ADD("io1", SEGA_315_5296, XTAL_16MHz)
	// all ports set to input
	MCFG_315_5296_IN_PORTA_CB(READ8(ufo_state, crane_limits_r))
	MCFG_315_5296_IN_PORTB_CB(READ8(ufo_state, crane_limits_r))
	MCFG_315_5296_IN_PORTE_CB(IOPORT("IN1"))
	MCFG_315_5296_IN_PORTF_CB(IOPORT("DSW1"))
	MCFG_315_5296_IN_PORTG_CB(IOPORT("DSW2"))
	MCFG_315_5296_IN_PORTH_CB(IOPORT("IN2"))

	MCFG_DEVICE_ADD("io2", SEGA_315_5296, XTAL_16MHz)
	// all ports set to output
	MCFG_315_5296_OUT_PORTA_CB(WRITE8(ufo_state, stepper_w))
	MCFG_315_5296_OUT_PORTB_CB(WRITE8(ufo_state, cp_lamps_w))
	MCFG_315_5296_OUT_PORTC_CB(WRITE8(ufo_state, cp_digits_w))
	MCFG_315_5296_OUT_PORTD_CB(WRITE8(ufo_state, cp_digits_w))
	MCFG_315_5296_OUT_PORTE_CB(WRITE8(ufo_state, crane_xyz_w))
	MCFG_315_5296_OUT_PORTF_CB(WRITE8(ufo_state, crane_xyz_w))
	MCFG_315_5296_OUT_PORTG_CB(WRITE8(ufo_state, ufo_lamps_w))

	MCFG_DEVICE_ADD("pit", PIT8254, XTAL_16MHz/2) // uPD71054C, configuration is unknown
	MCFG_PIT8253_CLK0(XTAL_16MHz/2/256)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(ufo_state, pit_out0))
	MCFG_PIT8253_CLK1(XTAL_16MHz/2/256)
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(ufo_state, pit_out1))
	MCFG_PIT8253_CLK2(XTAL_16MHz/2/256)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(ufo_state, pit_out2))

	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym", YM3438, XTAL_16MHz/2)
	MCFG_YM2612_IRQ_HANDLER(WRITELINE(ufo_state, ym3438_irq))
	MCFG_SOUND_ROUTE(0, "mono", 0.40)
	MCFG_SOUND_ROUTE(1, "mono", 0.40)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ufomini, newufo )

	/* basic machine hardware */
	MCFG_DEVICE_MODIFY("io1")
	MCFG_315_5296_IN_PORTC_CB(IOPORT("IN1"))
	MCFG_315_5296_IN_PORTE_CB(NULL)
	MCFG_315_5296_IN_PORTH_CB(NULL)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( ufo21, newufo )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(ex_ufo21_portmap)

	MCFG_DEVICE_MODIFY("io1")
	MCFG_315_5296_IN_PORTA_CB(READ8(ufo_state, ex_crane_limits_r))
	MCFG_315_5296_IN_PORTB_CB(READ8(ufo_state, ex_crane_limits_r))
	MCFG_315_5296_IN_PORTC_CB(READ8(ufo_state, ex_crane_open_r))

	MCFG_DEVICE_MODIFY("io2")
	MCFG_315_5296_OUT_PORTA_CB(WRITE8(ufo_state, ex_stepper_w))
	MCFG_315_5296_OUT_PORTB_CB(WRITE8(ufo_state, ex_cp_lamps_w))
	MCFG_315_5296_OUT_PORTE_CB(WRITE8(ufo_state, ex_crane_xyz_w))
	MCFG_315_5296_OUT_PORTF_CB(WRITE8(ufo_state, ex_crane_xyz_w))
	MCFG_315_5296_OUT_PORTG_CB(NULL)

	/* sound hardware */
	MCFG_SOUND_ADD("upd", UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ufo800, newufo )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(ex_ufo800_portmap)

	MCFG_DEVICE_MODIFY("io1")
	MCFG_315_5296_IN_PORTA_CB(READ8(ufo_state, ex_crane_limits_r))
	MCFG_315_5296_IN_PORTB_CB(IOPORT("IN2"))
	MCFG_315_5296_IN_PORTC_CB(READ8(ufo_state, ex_crane_open_r))
	MCFG_315_5296_IN_PORTD_CB(IOPORT("IN1"))
	MCFG_315_5296_IN_PORTE_CB(NULL)
	MCFG_315_5296_IN_PORTH_CB(NULL)

	MCFG_DEVICE_MODIFY("io2")
	MCFG_315_5296_OUT_PORTA_CB(WRITE8(ufo_state, ex_stepper_w))
	MCFG_315_5296_OUT_PORTB_CB(WRITE8(ufo_state, ex_cp_lamps_w))
	MCFG_315_5296_OUT_PORTE_CB(WRITE8(ufo_state, ex_crane_xyz_w))
	MCFG_315_5296_OUT_PORTF_CB(WRITE8(ufo_state, ex_ufo800_lamps_w))
	MCFG_315_5296_OUT_PORTG_CB(NULL)
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


GAMEL( 1991, newufo,       0,      newufo,  newufo,  driver_device, 0, ROT0, "Sega", "New UFO Catcher (standard)", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_newufo )
GAMEL( 1991, newufo_sonic, newufo, newufo,  newufo,  driver_device, 0, ROT0, "Sega", "New UFO Catcher (Sonic The Hedgehog)", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_newufo )
GAMEL( 1991, newufo_nfl,   newufo, newufo,  newufo,  driver_device, 0, ROT0, "Sega", "New UFO Catcher (Team NFL)", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_newufo )
GAMEL( 1991, newufo_xmas,  newufo, newufo,  newufo,  driver_device, 0, ROT0, "Sega", "New UFO Catcher (Christmas season ROM kit)", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_newufo )
GAMEL( 1991, ufomini,      0,      ufomini, ufomini, driver_device, 0, ROT0, "Sega", "UFO Catcher Mini", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_ufomini )
GAMEL( 1996, ufo21,        0,      ufo21,   ufo21,   driver_device, 0, ROT0, "Sega", "UFO Catcher 21", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_ufo21 )
GAMEL( 1998, ufo800,       0,      ufo800,  ufo800,  driver_device, 0, ROT0, "Sega", "UFO Catcher 800", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_ufo800 )

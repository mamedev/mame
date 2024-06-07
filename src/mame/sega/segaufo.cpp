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
    * Z80 (sticker obscures label), 16MHz XTAL, 2 Sega 315-5296(I/O), YM3438, NEC uPD71054C
  - # Dream Town (1990)
  - New UFO Catcher (1991) (2P) - probably the most popular cabinet of all UFO Catcher series
  - UFO Catcher Mini (1991) (1P)
  - # UFO Catcher Sega Sonic (1991)
  - # School Kids (1993)

  4th gen - EX brd
    * Z80 Z0840008PSC, 8MHz XTAL, 32MHz XTAL, 2 Sega 315-5296(I/O), 315-5338A, YM3438,
      NEC uPD71054C, optional NEC uPD7759C
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

#include "315_5296.h"
#include "315_5338a.h"

#include "cpu/z80/z80.h"
#include "machine/pit8253.h"
#include "machine/timer.h"
#include "sound/upd7759.h"
#include "sound/ymopn.h"

#include "speaker.h"

#include <iomanip>
#include <sstream>

// the layouts are very similar to eachother
#include "newufo.lh"
#include "ufomini.lh"
#include "ufo21.lh"
#include "ufo800.lh"


namespace {

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
	ufo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_io1(*this, "io1"),
		m_io2(*this, "io2"),
		m_upd(*this, "upd"),
		m_counters(*this, "counter%u", 0U),
		m_digits(*this, "digit%u", 0U),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void ufomini(machine_config &config);
	void ufo21(machine_config &config);
	void newufo(machine_config &config);
	void ufo800(machine_config &config);

private:
	void motor_tick(int p, int m);

	void pit_out0(int state);
	void pit_out1(int state);
	void pit_out2(int state);
	uint8_t crane_limits_r(offs_t offset);
	void stepper_w(uint8_t data);
	void cp_lamps_w(uint8_t data);
	void cp_digits_w(offs_t offset, uint8_t data);
	void crane_xyz_w(offs_t offset, uint8_t data);
	void ufo_lamps_w(uint8_t data);

	uint8_t ex_crane_limits_r(offs_t offset);
	uint8_t ex_crane_open_r();
	void ex_stepper_w(uint8_t data);
	void ex_cp_lamps_w(uint8_t data);
	void ex_crane_xyz_w(offs_t offset, uint8_t data);
	void ex_ufo21_lamps1_w(uint8_t data);
	void ex_ufo21_lamps2_w(uint8_t data);
	void ex_ufo800_lamps_w(uint8_t data);
	uint8_t ex_upd_busy_r();
	void ex_upd_start_w(uint8_t data);

	virtual void machine_reset() override;
	virtual void machine_start() override;
	TIMER_DEVICE_CALLBACK_MEMBER(simulate_xyz);
	TIMER_DEVICE_CALLBACK_MEMBER(update_info);

	void ufo_map(address_map &map);
	void ufo_portmap(address_map &map);
	void ex_ufo21_portmap(address_map &map);
	void ex_ufo800_portmap(address_map &map);

	struct Player
	{
		struct Motor
		{
			uint8_t running = 0;
			uint8_t direction = 0;
			float position = 0;
			float speed = 0;
		} motor[4];
	} m_player[2];

	uint8_t m_stepper = 0;

	required_device<cpu_device> m_maincpu;
	required_device<sega_315_5296_device> m_io1;
	required_device<sega_315_5296_device> m_io2;
	optional_device<upd7759_device> m_upd;
	output_finder<2 * 4> m_counters;
	output_finder<2> m_digits;
	output_finder<28> m_lamps;
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
	{
		for (int m = 0; m < 4; m++)
			m_counters[(p << 2) | m] = uint8_t(m_player[p].motor[m].position * 100);
	}

#if 0
	std::ostringstream msg;
	msg << std::hex << std::setfill('0');
	for (int i = 0; i < 8; i++)
		msg << std::setw(2) << m_io2->debug_peek_output(i);
	popmessage("%s", std::move(msg).str());
#endif
}



/***************************************************************************

  I/O

***************************************************************************/

void ufo_state::pit_out0(int state)
{
	// ?
}

void ufo_state::pit_out1(int state)
{
	// NMI?
	if (state)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void ufo_state::pit_out2(int state)
{
	// ?
}


/* generic / UFO board handlers */

/* io1 */

uint8_t ufo_state::crane_limits_r(offs_t offset)
{
	int p = offset & 1;
	uint8_t ret = 0x7f;

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

void ufo_state::stepper_w(uint8_t data)
{
	for (int p = 0; p < 2; p++)
	{
		// The crane stepper motor is set up as a rotating ellipse disc under the crane,
		// controlled with 4 output bits connected to a Toshiba TB6560AHQ motor driver.
		// I don't know which bits connect to which pins specifically.
		// To run it, the game writes a continuous sequence of $5, $9, $a, $6, ..
		static const uint8_t sequence[4] =
			{ 0x5, 0x9, 0xa, 0x6 };

		// d0-d3: p1, d4-d7: p2
		uint8_t cur = data >> (p*4) & 0xf;
		uint8_t prev = m_stepper >> (p*4) & 0xf;

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

void ufo_state::cp_lamps_w(uint8_t data)
{
	// d0-d3: p1/p2 button lamps
	// other bits: ?
	for (int i = 0; i < 4; i++)
		m_lamps[i] = BIT(~data, i);
}

void ufo_state::cp_digits_w(offs_t offset, uint8_t data)
{
	static constexpr uint8_t lut_7448[0x10] =
			{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0x00 };

	// d0-d3: cpanel digit
	// other bits: ?
	m_digits[offset & 1] = lut_7448[data & 0xf];
}

void ufo_state::crane_xyz_w(offs_t offset, uint8_t data)
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

void ufo_state::ufo_lamps_w(uint8_t data)
{
	// d0-d3: ufo leds (2 bits per player)
	// 3 sets of two red/green leds, each set is wired to the same control 2 bits
	// 00 = off,   off
	// 11 = red,   red
	// 01 = green, red
	// 10 = red,   green
	m_lamps[10] = data & 0x03;
	m_lamps[11] = (data >> 2) & 0x03;

	// d4,d5: ?
	// d6,d7: coincounters
	machine().bookkeeping().coin_counter_w(0, data & 0x40); // 100 Y
	machine().bookkeeping().coin_counter_w(1, data & 0x80); // 500 Y
}


/* EX board specific handlers */

/* io1 */

uint8_t ufo_state::ex_crane_limits_r(offs_t offset)
{
	int p = offset & 1;
	uint8_t ret = 0xf0;

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

uint8_t ufo_state::ex_crane_open_r()
{
	// d0-d3: p1, d4-d7: p2
	uint8_t ret = 0xff;

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

void ufo_state::ex_stepper_w(uint8_t data)
{
	// stepper motor sequence is: 6 c 9 3 6 c 9 3..
	// which means d0 and d3 are swapped when compared with UFO board hardware
	stepper_w(bitswap<8>(data,4,6,5,7,0,2,1,3));
}

void ufo_state::ex_cp_lamps_w(uint8_t data)
{
	// d0,d1,d4,d5: p1/p2 button lamps
	for (int i = 0; i < 4; i++)
		m_lamps[i] = BIT(~data, ((i&1) + (i&2) * 2));

	// d2,d3,d6,d7: p1/p2 coincounters
	for (int i = 0; i < 4; i++)
		machine().bookkeeping().coin_counter_w(i, data >> (2 + (i&1) + (i&2) * 2) & 1);
}

void ufo_state::ex_crane_xyz_w(offs_t offset, uint8_t data)
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

void ufo_state::ex_ufo800_lamps_w(uint8_t data)
{
	// d0-d4: 5 red leds on ufo
	// other bits: ?
	for (int i = 0; i < 5; i++)
		m_lamps[10 + i] = BIT(data, i);
}

/* 315-5338A */

void ufo_state::ex_ufo21_lamps1_w(uint8_t data)
{
	// d0: ? (ufo21 reads from it too, but value is discarded)
	// d1-d6 are the 6 red leds on each ufo
	// d7: ?
	for (int i = 1; i < 7; i++)
		m_lamps[10 + i] = BIT(data, i);
}

void ufo_state::ex_ufo21_lamps2_w(uint8_t data)
{
	for (int i = 1; i < 7; i++)
		m_lamps[20 + i] = BIT(data, i);
}

void ufo_state::ex_upd_start_w(uint8_t data)
{
	// d0: upd7759c start sample
	// other bits: unused?
	m_upd->start_w(~data & 1);
}

uint8_t ufo_state::ex_upd_busy_r()
{
	// d0: upd7759c busy
	// other bits: unused?
	int d0 = m_upd->busy_r() ? 1 : 0;
	return 0xfe | d0;
}


/* Memory maps */

void ufo_state::ufo_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xe000, 0xffff).ram();
}

void ufo_state::ufo_portmap(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x03).rw("pit", FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x40, 0x43).rw("ym", FUNC(ym3438_device::read), FUNC(ym3438_device::write));
	map(0x80, 0xbf).rw(m_io1, FUNC(sega_315_5296_device::read), FUNC(sega_315_5296_device::write));
	map(0xc0, 0xff).rw(m_io2, FUNC(sega_315_5296_device::read), FUNC(sega_315_5296_device::write));
}

void ufo_state::ex_ufo21_portmap(address_map &map)
{
	ufo_portmap(map);
	map(0x20, 0x20).w(m_upd, FUNC(upd7759_device::port_w));
	map(0x60, 0x6f).rw("io3", FUNC(sega_315_5338a_device::read), FUNC(sega_315_5338a_device::write));
}

void ufo_state::ex_ufo800_portmap(address_map &map)
{
	ufo_portmap(map);
//  map(0x60, 0x67).noprw(); // unused?
//  map(0x68, 0x68).nopw(); // ?
}



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
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x02, DEF_STR( 5C_1C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08) // SETTING 1
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_1C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x01, DEF_STR( 5C_2C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_3C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_2C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_3C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_5C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)

	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00) // SETTING 2
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_2C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_8C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x08, "1 Coin/10 Credits" ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x07, "1 Coin/11 Credits" ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x06, "1 Coin/12 Credits" ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)

	PORT_DIPNAME( 0xf0, 0xa0, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(    0x20, DEF_STR( 5C_1C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08) // SETTING 1
	PORT_DIPSETTING(    0x40, DEF_STR( 4C_1C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x60, DEF_STR( 3C_1C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x10, DEF_STR( 5C_2C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_3C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x50, DEF_STR( 3C_2C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x30, DEF_STR( 4C_3C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_3C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0x70, DEF_STR( 2C_5C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)

	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00) // SETTING 2
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_1C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x30, DEF_STR( 3C_1C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_1C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x20, DEF_STR( 3C_2C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_3C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_8C ) )    PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x80, "1 Coin/10 Credits" ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x70, "1 Coin/11 Credits" ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)
	PORT_DIPSETTING(    0x60, "1 Coin/12 Credits" ) PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Demo_Sounds ) )              PORT_DIPLOCATION("SW2:1") // Manual states "Sounds"
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )                                                 // Manual states "Always"
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )                                                // Manual states "Only During Play"
	PORT_DIPNAME( 0x02, 0x02, "Arm X, Y-Move" )                     PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, "Each Once" )
	PORT_DIPSETTING(    0x00, "Unrestricted" )
	PORT_DIPNAME( 0x04, 0x04, "Initial Mechanism Motion Check" )    PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Coinage ) )                  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "Setting 1" )
	PORT_DIPSETTING(    0x00, "Setting 2" )
	// Manual states "5 ~ 8 must be set OFF."
	PORT_DIPNAME( 0x10, 0x10, "Ignore Prize Sensor" )               PORT_DIPLOCATION("SW2:5") // "When DIP SW 2 #5 is set to ON, the game can be played normally without error even if the prize sensor is malfunctioning."
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) ) // Manual states "Fanfare and CPanel Digit will not work though."
	PORT_DIPUNKNOWN( 0x20, IP_ACTIVE_LOW )                          PORT_DIPLOCATION("SW2:6")
	PORT_DIPUNKNOWN( 0x40, IP_ACTIVE_LOW )                          PORT_DIPLOCATION("SW2:7")
	PORT_DIPUNKNOWN( 0x80, IP_ACTIVE_LOW )                          PORT_DIPLOCATION("SW2:8")
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
	m_counters.resolve();
	m_digits.resolve();
	m_lamps.resolve();

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

void ufo_state::newufo(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(16'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &ufo_state::ufo_map);
	m_maincpu->set_addrmap(AS_IO, &ufo_state::ufo_portmap);

	TIMER(config, "motor_timer").configure_periodic(FUNC(ufo_state::simulate_xyz), attotime::from_hz(MOTOR_SPEED));
	TIMER(config, "update_timer").configure_periodic(FUNC(ufo_state::update_info), attotime::from_hz(60));

	SEGA_315_5296(config, m_io1, XTAL(16'000'000));
	// all ports set to input
	m_io1->in_pa_callback().set(FUNC(ufo_state::crane_limits_r));
	m_io1->in_pb_callback().set(FUNC(ufo_state::crane_limits_r));
	m_io1->in_pe_callback().set_ioport("IN1");
	m_io1->in_pf_callback().set_ioport("DSW1");
	m_io1->in_pg_callback().set_ioport("DSW2");
	m_io1->in_ph_callback().set_ioport("IN2");

	SEGA_315_5296(config, m_io2, XTAL(16'000'000));
	// all ports set to output
	m_io2->out_pa_callback().set(FUNC(ufo_state::stepper_w));
	m_io2->out_pb_callback().set(FUNC(ufo_state::cp_lamps_w));
	m_io2->out_pc_callback().set(FUNC(ufo_state::cp_digits_w));
	m_io2->out_pd_callback().set(FUNC(ufo_state::cp_digits_w));
	m_io2->out_pe_callback().set(FUNC(ufo_state::crane_xyz_w));
	m_io2->out_pf_callback().set(FUNC(ufo_state::crane_xyz_w));
	m_io2->out_pg_callback().set(FUNC(ufo_state::ufo_lamps_w));

	pit8254_device &pit(PIT8254(config, "pit", XTAL(16'000'000)/2)); // uPD71054C, configuration is unknown
	pit.set_clk<0>(XTAL(16'000'000)/2/256);
	pit.out_handler<0>().set(FUNC(ufo_state::pit_out0));
	pit.set_clk<1>(XTAL(16'000'000)/2/256);
	pit.out_handler<1>().set(FUNC(ufo_state::pit_out1));
	pit.set_clk<2>(XTAL(16'000'000)/2/256);
	pit.out_handler<2>().set(FUNC(ufo_state::pit_out2));

	/* no video! */

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ym3438_device &ym(YM3438(config, "ym", XTAL(16'000'000)/2));
	ym.irq_handler().set_inputline("maincpu", 0);
	ym.add_route(0, "mono", 0.40);
	ym.add_route(1, "mono", 0.40);
}

void ufo_state::ufomini(machine_config &config)
{
	newufo(config);

	/* basic machine hardware */
	m_io1->in_pc_callback().set_ioport("IN1");
	m_io1->in_pe_callback().set_constant(0);
	m_io1->in_ph_callback().set_constant(0);
}


void ufo_state::ufo21(machine_config &config)
{
	newufo(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &ufo_state::ex_ufo21_portmap);

	m_io1->in_pa_callback().set(FUNC(ufo_state::ex_crane_limits_r));
	m_io1->in_pb_callback().set(FUNC(ufo_state::ex_crane_limits_r));
	m_io1->in_pc_callback().set(FUNC(ufo_state::ex_crane_open_r));

	m_io2->out_pa_callback().set(FUNC(ufo_state::ex_stepper_w));
	m_io2->out_pb_callback().set(FUNC(ufo_state::ex_cp_lamps_w));
	m_io2->out_pe_callback().set(FUNC(ufo_state::ex_crane_xyz_w));
	m_io2->out_pf_callback().set(FUNC(ufo_state::ex_crane_xyz_w));
	m_io2->out_pg_callback().set_nop();

	sega_315_5338a_device &io3(SEGA_315_5338A(config, "io3", 0));
	io3.out_pa_callback().set(FUNC(ufo_state::ex_upd_start_w));
	io3.in_pb_callback().set(FUNC(ufo_state::ex_upd_busy_r));
	io3.out_pe_callback().set(FUNC(ufo_state::ex_ufo21_lamps1_w));
	io3.out_pf_callback().set(FUNC(ufo_state::ex_ufo21_lamps2_w));

	/* sound hardware */
	UPD7759(config, m_upd);
	m_upd->add_route(ALL_OUTPUTS, "mono", 0.75);
}

void ufo_state::ufo800(machine_config &config)
{
	newufo(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_IO, &ufo_state::ex_ufo800_portmap);

	m_io1->in_pa_callback().set(FUNC(ufo_state::ex_crane_limits_r));
	m_io1->in_pb_callback().set_ioport("IN2");
	m_io1->in_pc_callback().set(FUNC(ufo_state::ex_crane_open_r));
	m_io1->in_pd_callback().set_ioport("IN1");
	m_io1->in_pe_callback().set_constant(0);
	m_io1->in_ph_callback().set_constant(0);

	m_io2->out_pa_callback().set(FUNC(ufo_state::ex_stepper_w));
	m_io2->out_pb_callback().set(FUNC(ufo_state::ex_cp_lamps_w));
	m_io2->out_pe_callback().set(FUNC(ufo_state::ex_crane_xyz_w));
	m_io2->out_pf_callback().set(FUNC(ufo_state::ex_ufo800_lamps_w));
	m_io2->out_pg_callback().set_nop();
}



/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( newufo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-13896.ic21",  0x000000, 0x010000, CRC(ca94be57) SHA1(acb6a22940c5e9ce639c7c30eb3948324b223090) )
ROM_END

ROM_START( newufo_sonic )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-14124.ic21",  0x000000, 0x010000, CRC(2bdbad89) SHA1(10de4b266471a68083ec4bc439b301b6587ccfd6) )
ROM_END

ROM_START( newufo_nfl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-15261.ic21",  0x000000, 0x010000, CRC(338c00d3) SHA1(03152956c6f1e4d5a1a11ee49f94a8c5eb550815) )
ROM_END

ROM_START( newufo_xmas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-15340.ic21",  0x000000, 0x010000, CRC(6287c9ac) SHA1(bc6bc84bb432424e1d25e01113e8e331fa64f96f) )
ROM_END


ROM_START( ufomini )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-14355.ic21",   0x000000, 0x010000, CRC(fbc969c5) SHA1(4a99dcd36bc48b6472988e0bc679fd61af17359c) )
ROM_END


ROM_START( ufo21 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-19063a.ic33",  0x000000, 0x010000, CRC(2e13e3e9) SHA1(6908f7db79c1a1da4ebc0456afc50ff18f2e8cf3) )

	ROM_REGION( 0x40000, "upd", 0 )
	ROM_LOAD( "epr-19064.ic32",   0x000000, 0x040000, CRC(ab62c1f0) SHA1(8791a88546ae69e710e128ffc2ea6e9b464f0631) )

	ROM_REGION( 0x1000, "gals", 0 )
	ROM_LOAD( "315-5766.ic40.jed", 0, 0x359, CRC(cb7a531f) SHA1(ef80f2701781a180e9087ca52c887d96a23127cc) ) // GAL16V8D
ROM_END

ROM_START( ufo800 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-20413a.ic33",  0x000000, 0x010000, CRC(36e9da6d) SHA1(8e1dbf8b24bc31be7de28f4d562838c291af7c7b) )
ROM_END

} // anonymous namespace


GAMEL( 1991, newufo,       0,      newufo,  newufo,  ufo_state, empty_init, ROT0, "Sega", "New UFO Catcher (standard)", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_newufo )
GAMEL( 1991, newufo_sonic, newufo, newufo,  newufo,  ufo_state, empty_init, ROT0, "Sega", "New UFO Catcher (Sonic The Hedgehog)", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_newufo )
GAMEL( 1991, newufo_nfl,   newufo, newufo,  newufo,  ufo_state, empty_init, ROT0, "Sega", "New UFO Catcher (Team NFL)", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_newufo )
GAMEL( 1991, newufo_xmas,  newufo, newufo,  newufo,  ufo_state, empty_init, ROT0, "Sega", "New UFO Catcher (Christmas season ROM kit)", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_newufo )
GAMEL( 1991, ufomini,      0,      ufomini, ufomini, ufo_state, empty_init, ROT0, "Sega", "UFO Catcher Mini", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_ufomini )
GAMEL( 1996, ufo21,        0,      ufo21,   ufo21,   ufo_state, empty_init, ROT0, "Sega", "UFO Catcher 21", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_ufo21 )
GAMEL( 1998, ufo800,       0,      ufo800,  ufo800,  ufo_state, empty_init, ROT0, "Sega", "UFO Catcher 800", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE, layout_ufo800 )

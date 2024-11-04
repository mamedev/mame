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
#include "machine/watchdog.h"

#include "submar.lh"


namespace {

class submar_state : public driver_device
{
public:
	submar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_motors(*this, "motor%u", 0U)
		, m_lamps(*this, "lamp%u", 0U)
		, m_solenoids(*this, "solenoid%u", 0U)
		, m_digits(*this, "digit%u", 0U)
	{ }

	void submar(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;

	uint8_t submar_sensor0_r();
	uint8_t submar_sensor1_r();
	void submar_motor_w(uint8_t data);
	void submar_lamp_w(uint8_t data);
	void submar_solenoid_w(uint8_t data);
	void submar_sound_w(uint8_t data);
	void submar_led_w(offs_t offset, uint8_t data);
	void submar_irq_clear_w(uint8_t data);

	void submar_map(address_map &map) ATTR_COLD;
	void submar_portmap(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	output_finder<8> m_motors;
	output_finder<8> m_lamps;
	output_finder<8> m_solenoids;
	output_finder<4> m_digits;
};


/***************************************************************************

  I/O, Memorymap

***************************************************************************/

void submar_state::machine_start()
{
	m_motors.resolve();
	m_lamps.resolve();
	m_solenoids.resolve();
	m_digits.resolve();
}

uint8_t submar_state::submar_sensor0_r()
{
	// ?
	return 0;
}

uint8_t submar_state::submar_sensor1_r()
{
	// ?
	return (ioport("IN1")->read() & 0x70) | 0x8f;
}

void submar_state::submar_motor_w(uint8_t data)
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
		m_motors[i] = BIT(data, i);
}

void submar_state::submar_lamp_w(uint8_t data)
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
		m_lamps[i] = BIT(data, i);
}

void submar_state::submar_solenoid_w(uint8_t data)
{
	// d0-d4: ship1-5
	// d5-d7: n/c
	for (int i = 0; i < 8; i++)
		m_solenoids[i] = BIT(data, i);
}

void submar_state::submar_sound_w(uint8_t data)
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

void submar_state::submar_led_w(offs_t offset, uint8_t data)
{
	// 7447 (BCD to LED segment)
	static constexpr uint8_t _7447_map[16] =
			{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0x00 };

	// 2 digits per write. port 4: time, port 5: score
	m_digits[((offset << 1) & 2) | 0] = _7447_map[data >> 4];
	m_digits[((offset << 1) & 2) | 1] = _7447_map[data & 0x0f];
}

void submar_state::submar_irq_clear_w(uint8_t data)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}


void submar_state::submar_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x207f).ram();
}

void submar_state::submar_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw(FUNC(submar_state::submar_sensor0_r), FUNC(submar_state::submar_motor_w));
	map(0x01, 0x01).rw(FUNC(submar_state::submar_sensor1_r), FUNC(submar_state::submar_lamp_w));
	map(0x02, 0x02).w(FUNC(submar_state::submar_solenoid_w));
	map(0x03, 0x03).portr("DSW").w(FUNC(submar_state::submar_sound_w));
	map(0x04, 0x05).w(FUNC(submar_state::submar_led_w));
	map(0x06, 0x06).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x07, 0x07).w(FUNC(submar_state::submar_irq_clear_w));
}



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

void submar_state::submar(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(19'968'000)/8);
	m_maincpu->set_periodic_int(FUNC(submar_state::irq0_line_assert), attotime::from_hz(124.675)); // 555 IC
	m_maincpu->set_addrmap(AS_PROGRAM, &submar_state::submar_map);
	m_maincpu->set_addrmap(AS_IO, &submar_state::submar_portmap);

	WATCHDOG_TIMER(config, "watchdog");

	/* no video! */

	/* sound hardware */
	//...
}



/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( submar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sub.a1", 0x0000, 0x0800, CRC(bcef5db4) SHA1(8ae5099672fbdb7bcdc617e1f8cbc5435fbb738a) )
	ROM_LOAD( "sub.a2", 0x0800, 0x0800, CRC(f5780dd0) SHA1(f775dd6f64a730a2fb6c9baf5787698434150bc5) )
ROM_END

} // anonymous namespace


GAMEL( 1979, submar, 0, submar, submar, submar_state, empty_init, ROT0, "Midway", "Submarine (Midway)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL, layout_submar )

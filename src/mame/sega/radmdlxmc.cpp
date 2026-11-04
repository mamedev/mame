// license:BSD-3-Clause
// copyright-holders:Tomás García-Merás Capote (ClawGrip)

/*******************************************************************************************

    Sega 837-7753 drive board for the Rad Mobile deluxe moving cabinet.

    The board banks the cabinet with a DC motor, reading the bank position and the motor
    shaft through two incremental encoders.

    Hardware:
     - NEC D70008AC-8 (Z80) and 8 MHz oscillator.
     - EPR-13686A (27C256) program ROM.
     - Fujitsu MB8464A-10LL SRAM, with an unpopulated socket for a second 6264.
     - 2 x NEC D71054GB (8254 timer) and 2 x NEC D71055GB (8255 PPI).
     - NEC D4701AC encoder counter: X is the bank position, Y the motor shaft.
     - Fujitsu MB3773 reset and watchdog.
     - 93C46 EEPROM, unused by the program (blank).
     - 3 x PEEL18CV8P (315-5475-01, 315-5475-02, 315-5475-03A).
     - 4 x NEC PS2501-4 optocouplers, 1 bank of 4 DIP switches.
     - Unpopulated: ADC0809, 2 x TLP521, LEDs and the bus buffers for CN2.

    The main board talks to it through its I/O chip: port G is the address (bit 7 set for
    reads), port C the data and port D bit 4 the strobe. Each access goes to a 128-byte RAM
    window at C000, whose base is set by the A7-A13 jumpers:
      00-01  w   target bank position
      02-03  r   left limit position
      04-05  r   right limit position
      06     w   lamps (PPI 1 port A)
      07     w   d0: vibration
      08     rw  d0: new target (set by the host, cleared by the board), d1: ready,
                 d2: over-current, d3: over-temperature, d4-d7: sensors A-D
      09     r   d0: error, d1: right limit switch, d2: left limit switch, d3: center
                 sensor, d4: motor not moving, d5: cabinet not moving, d6: motor encoder,
                 d7: bank encoder
      0a-0b  r   bank position
      0c     rw  link check, written and read back by the host
      0e     r   friction level measured while homing
      0f     r   PPI 0 port B

    Timer 1 makes the motor PWM: counter 0 sets a 15.625 kHz period and presumably triggers
    counter 1, whose count sets the duty cycle. From the way the program uses it, 50% duty
    gives no torque (locked anti-phase drive). Counter 2 is set up the same way for a second channel
    that the program never uses. Timer 1 isn't clocked here, as emulating the PWM edges
    slows the whole system down several times; the drive comes from the counter 1 count.

    TODO:
    - Timer 0 counters 1 and 2 ("safety clocks") feed logic that isn't emulated.
    - The cabinet mechanics (motor speed, gearing, travel and switch positions) are guesses,
      and the over-current and over-temperature inputs never trip.

*******************************************************************************************/

#include "emu.h"
#include "radmdlxmc.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"

#include <algorithm>
#include <cmath>


namespace {

// cabinet mechanics, all guessed
constexpr u32 MOTION_RATE = 1000; // simulation steps per second
constexpr double MOTOR_MAX_SPEED = 94000.0; // motor encoder counts per second at full drive
constexpr double MOTOR_TIME_CONSTANT = 0.1; // seconds
constexpr double GEAR_RATIO = 40.0; // motor encoder counts per bank encoder count
constexpr s32 BANK_LIMIT = 400; // limit switches, in bank encoder counts from the center
constexpr s32 BANK_STOP = 440; // mechanical end stops
constexpr s32 BANK_CENTER = 16; // half width of the center sensor vane

} // anonymous namespace


DEFINE_DEVICE_TYPE(RADM_DLX_MOVCTRL, radmdlxmc_device, "radmdlxmc", "Sega Rad Mobile Deluxe Cabinet Moving Control")

radmdlxmc_device::radmdlxmc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RADM_DLX_MOVCTRL, tag, owner, clock)
	, m_maincpu(*this, "maincpu")
	, m_pit1(*this, "pit1")
	, m_upd4701(*this, "upd4701")
	, m_ram(*this, "ram")
	, m_lamps_cb(*this)
	, m_bank_position(*this, "bank_position")
	, m_bank_motor(*this, "bank_motor")
	, m_host_addr(0)
	, m_host_data(0)
	, m_host_strobe(true)
	, m_drive_enable(0)
	, m_pwm_duty(0x7f)
	, m_motor_speed(0.0)
	, m_motor_pos(0.0)
{
}

void radmdlxmc_device::device_start()
{
	attotime const period = attotime::from_hz(MOTION_RATE);
	timer_alloc(FUNC(radmdlxmc_device::motion_tick), this)->adjust(period, 0, period);

	save_item(NAME(m_host_addr));
	save_item(NAME(m_host_data));
	save_item(NAME(m_host_strobe));
	save_item(NAME(m_drive_enable));
	save_item(NAME(m_pwm_duty));
	save_item(NAME(m_motor_speed));
	save_item(NAME(m_motor_pos));
}


/*******************************************************************************************
    Host interface
*******************************************************************************************/

u8 radmdlxmc_device::data_r()
{
	// the board only drives the data lines during a read strobe
	if (m_host_strobe || !BIT(m_host_addr, 7))
		return 0xff;

	return m_ram[m_host_addr & 0x7f];
}

void radmdlxmc_device::data_w(u8 data)
{
	m_host_data = data;
}

void radmdlxmc_device::addr_w(u8 data)
{
	m_host_addr = data;
}

void radmdlxmc_device::strobe_w(int state)
{
	if (!state && m_host_strobe && !BIT(m_host_addr, 7))
		m_ram[m_host_addr & 0x7f] = m_host_data;

	m_host_strobe = state;
}


/*******************************************************************************************
    Board I/O
*******************************************************************************************/

IRQ_CALLBACK_MEMBER(radmdlxmc_device::irq_ack)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	return 0xff;
}

void radmdlxmc_device::timer_irq_w(int state)
{
	if (!state)
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
}

u8 radmdlxmc_device::switches_r()
{
	// d0: right limit, d1: left limit, d2: center, d4: low on over-current, d5: low on over-temperature
	s32 const pos = bank_position();
	u8 data = 0xf8;
	if (pos >= BANK_LIMIT)
		data |= 0x01;
	if (pos <= -BANK_LIMIT)
		data |= 0x02;
	if (std::abs(pos) <= BANK_CENTER)
		data |= 0x04;

	return data;
}

void radmdlxmc_device::drive_enable_w(u8 data)
{
	// d0: set when driving left, d1: set when driving right, both clear to stop the motor; d4-d6 always set
	m_drive_enable = data;
}

void radmdlxmc_device::pit1_w(offs_t offset, u8 data)
{
	m_pit1->write(offset, data);

	// counter 1 is loaded LSB only, its output pulse ends the PWM high time N + 1 clocks into the 256-clock period
	if (offset == 1)
		m_pwm_duty = data;
}


/*******************************************************************************************
    Cabinet mechanics
*******************************************************************************************/

TIMER_CALLBACK_MEMBER(radmdlxmc_device::motion_tick)
{
	// More than 50% duty drives left, less drives right. PA0/PA1 follow the sign of the servo
	// output; either is taken to enable the motor, as the vibration added on top needs both ways.
	double const drive = (m_drive_enable & 0x03) ? ((127.0 - m_pwm_duty) / 128.0) : 0.0;

	m_motor_speed += (drive * MOTOR_MAX_SPEED - m_motor_speed) / (MOTOR_TIME_CONSTANT * MOTION_RATE);
	m_motor_pos += m_motor_speed / MOTION_RATE;

	double const stop = BANK_STOP * GEAR_RATIO;
	if (m_motor_pos >= stop)
	{
		m_motor_pos = stop;
		m_motor_speed = std::min(m_motor_speed, 0.0);
	}
	else if (m_motor_pos <= -stop)
	{
		m_motor_pos = -stop;
		m_motor_speed = std::max(m_motor_speed, 0.0);
	}

	m_upd4701->update();

	m_bank_position = bank_position();
	m_bank_motor = s32(std::lround(drive * 128.0));
}

s32 radmdlxmc_device::bank_position() const
{
	return s32(std::floor(m_motor_pos / GEAR_RATIO));
}

ioport_value radmdlxmc_device::bank_encoder_r()
{
	return bank_position() & 0xfff;
}

ioport_value radmdlxmc_device::motor_encoder_r()
{
	return s32(std::floor(m_motor_pos)) & 0xfff;
}


/*******************************************************************************************
    Machine configuration
*******************************************************************************************/

void radmdlxmc_device::program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8003).rw("ppi0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x8100, 0x8103).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x8200, 0x8203).rw("pit0", FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x8300, 0x8303).r(m_pit1, FUNC(pit8254_device::read)).w(FUNC(radmdlxmc_device::pit1_w));
	map(0x8400, 0x8403).r(m_upd4701, FUNC(upd4701_device::read_xy));
	map(0x8600, 0x8600).nopw(); // over-current latch reset
	map(0x8c00, 0x8c03).nopw(); // monitor: motor speed, bank position, target and drive
	map(0xc000, 0xdfff).ram().share(m_ram);
}

void radmdlxmc_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_maincpu, 8_MHz_XTAL); // NEC D70008AC-8
	m_maincpu->set_addrmap(AS_PROGRAM, &radmdlxmc_device::program_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(radmdlxmc_device::irq_ack));

	// PC3 toggles on every interrupt, probably the MB3773 watchdog clock
	i8255_device &ppi0(I8255(config, "ppi0")); // NEC D71055GB
	ppi0.out_pa_callback().set(FUNC(radmdlxmc_device::drive_enable_w));
	ppi0.in_pb_callback().set(FUNC(radmdlxmc_device::switches_r));
	ppi0.in_pc_callback().set_ioport("DSW");

	i8255_device &ppi1(I8255(config, "ppi1")); // NEC D71055GB
	ppi1.out_pa_callback().set([this] (u8 data) { m_lamps_cb(data); });
	ppi1.in_pb_callback().set_ioport("SENSORS");

	pit8254_device &pit0(PIT8254(config, "pit0")); // NEC D71054GB
	pit0.set_clk<0>(8_MHz_XTAL / 2);
	pit0.set_clk<1>(8_MHz_XTAL / 1024); // SFTYCLK jumper set to 8K
	pit0.set_clk<2>(8_MHz_XTAL / 1024);
	pit0.out_handler<0>().set(FUNC(radmdlxmc_device::timer_irq_w));

	PIT8254(config, m_pit1); // NEC D71054GB, its 4 MHz clock is left unconnected (see above)

	UPD4701A(config, m_upd4701); // NEC D4701AC
	m_upd4701->set_portx_tag("BANK");
	m_upd4701->set_porty_tag("MOTOR");
}


/*******************************************************************************************
    Inputs
*******************************************************************************************/

static INPUT_PORTS_START(radmdlxmc)
	PORT_START("DSW")
	PORT_DIPNAME(0x30, 0x00, "Motion Strength") PORT_DIPLOCATION("DSW:1,2")
	PORT_DIPSETTING(   0x20, DEF_STR(Low))
	PORT_DIPSETTING(   0x00, DEF_STR(Medium))
	PORT_DIPSETTING(   0x10, DEF_STR(High))
	PORT_DIPSETTING(   0x30, DEF_STR(High))
	PORT_DIPUNUSED_DIPLOC(0x40, 0x40, "DSW:3")
	PORT_DIPNAME(0x80, 0x80, "Sensors C/D") PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(   0x80, DEF_STR(Normal))
	PORT_DIPSETTING(   0x00, "Swapped")

	// any sensor holds the cabinet where it is
	PORT_START("SENSORS")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Sensor A (Cockpit)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Sensor B (Base Right Front)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Sensor C (Base Right Rear)")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Sensor D (Base Left)")
	PORT_BIT(0x70, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN) // when low the board moves the cabinet at random, ignoring the host

	PORT_START("BANK")
	PORT_BIT(0xfff, 0x000, IPT_CUSTOM) PORT_CUSTOM_MEMBER(FUNC(radmdlxmc_device::bank_encoder_r))

	PORT_START("MOTOR")
	PORT_BIT(0xfff, 0x000, IPT_CUSTOM) PORT_CUSTOM_MEMBER(FUNC(radmdlxmc_device::motor_encoder_r))
INPUT_PORTS_END

ioport_constructor radmdlxmc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(radmdlxmc);
}


/*******************************************************************************************
    ROM definition
*******************************************************************************************/

ROM_START(radmdlxmc)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("epr-13686a.bin", 0x0000, 0x8000, CRC(317a2857) SHA1(e0788dc7a7d214d9c4d26b24e44c1a0dc9ae477c)) // "RAD MOBILE DELUX CABINET MOVING CONTROL"

	ROM_REGION(0x400, "plds", 0)
	ROM_LOAD("315-5475-01_peel18cv8p.ic16", 0x000, 0x155, NO_DUMP)
	ROM_LOAD("315-5475-02_peel18cv8p.ic26", 0x155, 0x155, NO_DUMP)
	ROM_LOAD("315-5475-03a_peel18cv8p.ic37", 0x2aa, 0x155, NO_DUMP)
ROM_END

const tiny_rom_entry *radmdlxmc_device::device_rom_region() const
{
	return ROM_NAME(radmdlxmc);
}

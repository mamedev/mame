// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Explorer 85

    12/05/2009 Skeleton driver.
    29/12/2023 Reworked driver to enable extended RAM memory,
               MS BASIC and disable ROM mirror after boot/interrupt.

    Setting Up
    ==========
    The terminal must be set for
    - Baud: 9600
    - Format: 7E1
    If it isn't, adjust the settings, then restart the system.

    Once started, press Space. The system will start up.
    All input must be in upper case.

    Microsoft Basic is executed entering the following 2 commands
    in the monitor prompt:
    .XS nnnn-F87F mmmm-C000 <CR>
    .G <CR>

    where nnnn is the previous value of the stack pointer, and mmmm is the
    previous value of the program counter.

    BASIC will request the amount of RAM memory available, 8192 (bytes) must
    be entered.


****************************************************************************/

/*

    TODO:

    - dump of the hexadecimal keyboard monitor ROM
    - 64K RAM expansion
    - Disk drive and CP/M OS support

*/


#include "emu.h"

#include "machine/i8155.h"
#include "machine/i8355.h"
#include "machine/ram.h"
#include "speaker.h"

#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "imagedev/cassette.h"
#include "sound/spkrdev.h"

namespace {

static constexpr int LOW_MEMORY_ROM_MIRROR_ENTRY = 0;
static constexpr int LOW_MEMORY_RAM_ENTRY        = 1;

class exp85_state : public driver_device
{
public:
	exp85_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "u100")
		, m_i8355(*this, "u105")
		, m_i8155(*this, "u106")
		, m_rs232(*this, "rs232")
		, m_cassette(*this, "cassette")
		, m_speaker(*this, "speaker")
		, m_rom(*this, "u105") // the 8355 chip contains the monitor ROM
		, m_low_memory_view(*this, "low_memory_view")
		, m_is_preparing_interrupt_call(false)
		, m_ignore_timer_out(true)
		, m_tape_control(0)
		, m_timer(nullptr)
	{ }

	void exp85(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER( trigger_reset );
	DECLARE_INPUT_CHANGED_MEMBER( trigger_rst75 );

private:
	virtual void machine_start() override ATTR_COLD;

	uint8_t i8355_a_r();
	void i8355_a_w(uint8_t data);
	int sid_r();
	void sod_w(int state);

	void status_out(u8 status);

	void exp85_io(address_map &map) ATTR_COLD;
	void exp85_mem(address_map &map) ATTR_COLD;

	void to_change(int to);

	TIMER_CALLBACK_MEMBER(trap_delay);

	// Member variables
	required_device<i8085a_cpu_device> m_maincpu;
	required_device<i8355_device> m_i8355;
	required_device<i8155_device> m_i8155;
	required_device<rs232_port_device> m_rs232;
	required_device<cassette_image_device> m_cassette;
	required_device<speaker_sound_device> m_speaker;
	required_memory_region m_rom;
	memory_view m_low_memory_view;

	bool m_is_preparing_interrupt_call;
	bool m_ignore_timer_out;
	bool m_tape_control;
	emu_timer *m_timer;
};


/* Memory Maps */
void exp85_state::exp85_mem(address_map &map)
{
	map.unmap_value_high();
	// Extended RAM or mapped monitor ROM (only during interrupt and reset)
	map(0x0000, 0x1fff).view(m_low_memory_view);
	// Microsoft BASIC ROM
	map(0xc000, 0xdfff).rom();
	// Monitor ROM in the 8355 chip
	map(0xf000, 0xf7ff).rom().region(m_rom, 0);
	// 256 bytes of RAM of level A in the 8155 chip.
	map(0xf800, 0xf8ff).rw(m_i8155, FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));

	// monitor ROM mirror
	m_low_memory_view[LOW_MEMORY_ROM_MIRROR_ENTRY](0x0000, 0x07ff).rom().region(m_rom, 0);
	// extended RAM
	m_low_memory_view[LOW_MEMORY_RAM_ENTRY](0x0000, 0x1fff).ram();
}

void exp85_state::exp85_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0xf0, 0xf3).rw(m_i8355, FUNC(i8355_device::io_r), FUNC(i8355_device::io_w));
	map(0xf8, 0xfd).rw(m_i8155, FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
}

/* Input Ports */

INPUT_CHANGED_MEMBER( exp85_state::trigger_reset )
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);
	m_low_memory_view.select(LOW_MEMORY_ROM_MIRROR_ENTRY);
}

INPUT_CHANGED_MEMBER( exp85_state::trigger_rst75 )
{
	m_maincpu->set_input_line(I8085_RST75_LINE, newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( exp85 )
	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("R") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, exp85_state, trigger_reset, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("I") PORT_CODE(KEYCODE_F2) PORT_CHANGED_MEMBER(DEVICE_SELF, exp85_state, trigger_rst75, 0)
INPUT_PORTS_END

/* 8355 Interface */

uint8_t exp85_state::i8355_a_r()
{
	/*

	    bit     description

	    PA0     tape control
	    PA1     jumper S17 (open=+5V closed=GND)
	    PA2     J5:13
	    PA3
	    PA4     J2:22
	    PA5
	    PA6
	    PA7     speaker output

	*/

	return 0x02;
}

void exp85_state::i8355_a_w(uint8_t data)
{
	/*

	    bit     description

	    PA0     tape control
	    PA1     jumper S17 (open=+5V closed=GND)
	    PA2     J5:13
	    PA3
	    PA4     J2:22
	    PA5
	    PA6
	    PA7     speaker output

	*/

	/* tape control */
	m_tape_control = BIT(data, 0);

	/* speaker output */
	m_speaker->level_w(!BIT(data, 7));
}

/* I8085A Interface */

int exp85_state::sid_r()
{
	int data = 1;

	if (m_tape_control)
	{
		data = (m_cassette->input() > +0.0);
	}
	else
	{
		data = m_rs232->rxd_r();
	}

	return data;
}

void exp85_state::sod_w(int state)
{
	if (m_tape_control)
	{
		m_cassette->output(state ? -1.0 : +1.0);
	}
	else
	{
		m_rs232->write_txd(state);
	}
}

/* Memory mapping mgmt during reset and interrupt */

void exp85_state::status_out(u8 status)
{
	// In the real hardware this is monitored via the IO/M, S0, S1
	// and ALE output pins of the 8085.
	// Since these pins are not emulated, then we must explicitly get the internal
	// interrupt acknowledge state (0x23 or 0x26) and behave as the monitor expects.
	auto current_pc = m_maincpu->pc();
	if (status == 0x23 || status == 0x26)
	{
		// When an interrupt is triggered the low memory shall be set to the ROM mirror
		m_is_preparing_interrupt_call = true;
		m_low_memory_view.select(LOW_MEMORY_ROM_MIRROR_ENTRY);
	}
	else if (m_is_preparing_interrupt_call && (current_pc & 0xff00) == 0x0000)
	{
		// Avoids setting the lower memory back to RAM until
		// it branches to the interrupt handler.
		m_is_preparing_interrupt_call = false;
	}
	else if (!m_is_preparing_interrupt_call && (current_pc & 0xf000) == 0xf000)
	{
		// When the interrupt handler is executing and the address is >= 0xf000
		// the low memory is mapped to RAM
		m_low_memory_view.select(LOW_MEMORY_RAM_ENTRY);
	}

}

//**************************************************************************
//  INPUT PORTS
//**************************************************************************


/* Terminal Interface */

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

/* Machine Initialization */

void exp85_state::machine_start()
{
	/* clear the trap line that is asserted on start */
	m_maincpu->set_input_line(I8085_TRAP_LINE, CLEAR_LINE);

	/* setup memory mirror switch */
	m_low_memory_view.select(LOW_MEMORY_ROM_MIRROR_ENTRY);

	save_item(NAME(m_tape_control));

	m_timer = timer_alloc(FUNC(exp85_state::trap_delay), this);
}

void exp85_state::to_change(int to)
{
	// In the 8155 implementation the TIMER-OUT line (to) is
	// asserted by default on initialization; this generates a spurious exception
	// avoided here via an ignore flag.
	if (m_ignore_timer_out)
	{
		m_ignore_timer_out = false;
		return;
	}

	// The 8155 has a max TIMER-IN to TIMER-OUT delay of 400ns (datasheet page 3-256). In a real system
	// this delay was measured in ~70ns. This is enough for the next instruction cycle to start
	// and for the interrupt to be acknowledged one instruction later. This effect is fundamental
	// for the ROM monitor stepping functionality, since it depends heavily on interrupting when the
	// next instruction of the user program is being executed.
	// MAME allows to set a timer of 70ns but that is not enough to get the CPU scheduled for at least 1 cycle
	// before the interrupt is serviced, so we program a timer to expire beyond a full clock cycle.
	m_timer->adjust(m_i8155->clocks_to_attotime(2), to);
}

TIMER_CALLBACK_MEMBER(exp85_state::trap_delay)
{
	// The 8155 TIMER-OUT line is connected to the TRAP input line of the 8085;
	// that line is set from the start high, and since there is no change in that line, the
	// interrup is never requested (datasheet, page 6-13).
	m_maincpu->set_input_line(I8085_TRAP_LINE, param == 1 ? ASSERT_LINE : CLEAR_LINE);
}

/* Machine Driver */
void exp85_state::exp85(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_maincpu, 6.144_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &exp85_state::exp85_mem);
	m_maincpu->set_addrmap(AS_IO, &exp85_state::exp85_io);
	m_maincpu->in_sid_func().set(FUNC(exp85_state::sid_r));
	m_maincpu->out_sod_func().set(FUNC(exp85_state::sod_w));
	m_maincpu->out_status_func().set(FUNC(exp85_state::status_out));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);

	/* devices */

	// The Explorer-85 uses a 6.144MHz clock, but the Intel 8085 divides by 2 this
	// input frequency to give the processor internal operating frequency.
	// The bus is connected to the CLK (out) of the 8085, running also
	// at 6.144MHz divided by 2.
	I8155(config, m_i8155, 6.144_MHz_XTAL/2);
	m_i8155->out_to_callback().set(FUNC(exp85_state::to_change));


	I8355(config, m_i8355, 6.144_MHz_XTAL/2);
	m_i8355->in_pa().set(FUNC(exp85_state::i8355_a_r));
	m_i8355->out_pa().set(FUNC(exp85_state::i8355_a_w));

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	RS232_PORT(config, "rs232", default_rs232_devices, "terminal").set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

}

/* ROMs */

ROM_START( exp85 )
	ROM_REGION( 0x10000, "u100", 0 ) // Microsoft BASIC
	ROM_DEFAULT_BIOS("eia")
	ROM_LOAD( "c000.bin", 0xc000, 0x0800, CRC(73ce4aad) SHA1(2c69cd0b6c4bdc92f4640bce18467e4e99255bab) )
	ROM_LOAD( "c800.bin", 0xc800, 0x0800, CRC(eb3fdedc) SHA1(af92d07f7cb7533841b16e1176401363176857e1) )
	ROM_LOAD( "d000.bin", 0xd000, 0x0800, CRC(c10c4a22) SHA1(30588ba0b27a775d85f8c581ad54400c8521225d) )
	ROM_LOAD( "d800.bin", 0xd800, 0x0800, CRC(dfa43ef4) SHA1(56a7e7a64928bdd1d5f0519023d1594cacef49b3) )

	ROM_REGION( 0x800, "u105", 0 ) // Explorer 85 Monitor
	ROM_SYSTEM_BIOS( 0, "eia", "EIA Terminal" ) // Serial terminal ROM
	ROMX_LOAD( "ex 85.u105", 0x0000, 0x0800, CRC(1a99d0d9) SHA1(57b6d48e71257bc4ef2d3dddc9b30edf6c1db766), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "hex", "Hex Keyboard" ) // Keypad ROM (not available)
	ROMX_LOAD( "1kbd.u105", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(1) )
ROM_END

} // anonymous namespace


/* System Drivers */
//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY      FULLNAME       FLAGS
COMP( 1979, exp85, 0,      0,      exp85,   exp85, exp85_state, empty_init, "Netronics", "Explorer/85", MACHINE_SUPPORTS_SAVE )

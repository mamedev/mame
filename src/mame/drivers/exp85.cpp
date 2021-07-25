// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Explorer 85

    12/05/2009 Skeleton driver.

    Setting Up
    ==========
    The terminal must be set for
    - Baud: 9600
    - Format: 7E1
    If it isn't, adjust the settings, then restart the system.

    Once started, press Space. The system will start up.
    All input must be in upper case.

****************************************************************************/

/*

    TODO:

    - dump of the hexadecimal keyboard monitor ROM
    - disable ROM mirror after boot
    - RAM expansions

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

#define I8085A_TAG      "u100"
#define I8155_TAG       "u106"
#define I8355_TAG       "u105"

class exp85_state : public driver_device
{
public:
	exp85_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, I8085A_TAG)
		, m_rs232(*this, "rs232")
		, m_cassette(*this, "cassette")
		, m_speaker(*this, "speaker")
		, m_rom(*this, I8085A_TAG)
		, m_tape_control(0)
	{ }

	void exp85(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER( trigger_reset );
	DECLARE_INPUT_CHANGED_MEMBER( trigger_rst75 );

private:
	required_device<i8085a_cpu_device> m_maincpu;
	required_device<rs232_port_device> m_rs232;
	required_device<cassette_image_device> m_cassette;
	required_device<speaker_sound_device> m_speaker;
	required_memory_region m_rom;

	virtual void machine_start() override;

	uint8_t i8355_a_r();
	void i8355_a_w(uint8_t data);
	DECLARE_READ_LINE_MEMBER( sid_r );
	DECLARE_WRITE_LINE_MEMBER( sod_w );

	/* cassette state */
	bool m_tape_control;
	void exp85_io(address_map &map);
	void exp85_mem(address_map &map);
};

/* Memory Maps */

void exp85_state::exp85_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).bankr("bank1");
	map(0xc000, 0xdfff).rom();
	map(0xf000, 0xf7ff).rom();
	map(0xf800, 0xf8ff).ram();
}

void exp85_state::exp85_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0xf0, 0xf3).rw(I8355_TAG, FUNC(i8355_device::io_r), FUNC(i8355_device::io_w));
	map(0xf8, 0xfd).rw(I8155_TAG, FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
//  map(0xfe, 0xff).rw(I8279_TAG, FUNC(i8279_device::read), FUNC(i8279_device::write));
}

/* Input Ports */

INPUT_CHANGED_MEMBER( exp85_state::trigger_reset )
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);
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

READ_LINE_MEMBER( exp85_state::sid_r )
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

WRITE_LINE_MEMBER( exp85_state::sod_w )
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
	/* setup memory banking */
	membank("bank1")->configure_entry(0, m_rom->base() + 0xf000);
	membank("bank1")->configure_entry(1, m_rom->base());
	membank("bank1")->set_entry(0);

	save_item(NAME(m_tape_control));
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

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.25);

	/* devices */
	I8155(config, I8155_TAG, 6.144_MHz_XTAL/2);

	i8355_device &i8355(I8355(config, I8355_TAG, 6.144_MHz_XTAL/2));
	i8355.in_pa().set(FUNC(exp85_state::i8355_a_r));
	i8355.out_pa().set(FUNC(exp85_state::i8355_a_w));

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	RS232_PORT(config, "rs232", default_rs232_devices, "terminal").set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("256").set_extra_options("4K");
}

/* ROMs */

ROM_START( exp85 )
	ROM_REGION( 0x10000, I8085A_TAG, 0 )
	ROM_DEFAULT_BIOS("eia")
	ROM_LOAD( "c000.bin", 0xc000, 0x0800, CRC(73ce4aad) SHA1(2c69cd0b6c4bdc92f4640bce18467e4e99255bab) )
	ROM_LOAD( "c800.bin", 0xc800, 0x0800, CRC(eb3fdedc) SHA1(af92d07f7cb7533841b16e1176401363176857e1) )
	ROM_LOAD( "d000.bin", 0xd000, 0x0800, CRC(c10c4a22) SHA1(30588ba0b27a775d85f8c581ad54400c8521225d) )
	ROM_LOAD( "d800.bin", 0xd800, 0x0800, CRC(dfa43ef4) SHA1(56a7e7a64928bdd1d5f0519023d1594cacef49b3) )
	ROM_SYSTEM_BIOS( 0, "eia", "EIA Terminal" )
	ROMX_LOAD( "ex 85.u105", 0xf000, 0x0800, CRC(1a99d0d9) SHA1(57b6d48e71257bc4ef2d3dddc9b30edf6c1db766), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "hex", "Hex Keyboard" )
	ROMX_LOAD( "1kbd.u105", 0xf000, 0x0800, NO_DUMP, ROM_BIOS(1) )

	ROM_REGION( 0x800, I8355_TAG, ROMREGION_ERASE00 )

/*  ROM_DEFAULT_BIOS("terminal")
    ROM_SYSTEM_BIOS( 0, "terminal", "Terminal" )
    ROMX_LOAD( "eia.u105", 0xf000, 0x0800, CRC(1a99d0d9) SHA1(57b6d48e71257bc4ef2d3dddc9b30edf6c1db766), ROM_BIOS(0) )
    ROM_SYSTEM_BIOS( 1, "hexkbd", "Hex Keyboard" )
    ROMX_LOAD( "hex.u105", 0xf000, 0x0800, NO_DUMP, ROM_BIOS(1) )*/
ROM_END

/* System Drivers */
//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY      FULLNAME       FLAGS
COMP( 1979, exp85, 0,      0,      exp85,   exp85, exp85_state, empty_init, "Netronics", "Explorer/85", MACHINE_SUPPORTS_SAVE )

// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Atlantis Computers MCG-85

    After RESET press SPACE on terminal for Expeditor to detect baud rate (50-19.2K).

******************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8155.h"


namespace {

class mcg85_state : public driver_device
{
public:
	mcg85_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void mcg85(machine_config &config);

private:
	required_device<i8085a_cpu_device> m_maincpu;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


void mcg85_state::mem_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0x1fff).ram(); // optional RAM
	map(0x2000, 0x20ff).mirror(0x700).rw("i8155", FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
}

void mcg85_state::io_map(address_map &map)
{
	map(0x20, 0x27).rw("i8155", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
}


static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END


void mcg85_state::mcg85(machine_config &config)
{
	I8085A(config, m_maincpu, 6.144_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mcg85_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &mcg85_state::io_map);
	m_maincpu->in_sid_func().set("rs232", FUNC(rs232_port_device::rxd_r));
	m_maincpu->out_sod_func().set("rs232", FUNC(rs232_port_device::write_txd));

	I8155(config, "i8155", 6.144_MHz_XTAL / 2);

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
}


ROM_START( mcg85 )
	ROM_REGION(0x1000, "maincpu", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS( 0, "24", "Expeditor Monitor V2.4" )
	ROMX_LOAD("expdr_monitor_2.4.bin", 0x0000, 0x0800, CRC(54a75a26) SHA1(f19294599e8b89a7036ffd1b7a97026d71837c5a), ROM_BIOS(0))
ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT  COMPAT  MACHINE   INPUT   CLASS         INIT         COMPANY                FULLNAME   FLAGS
COMP( 1981, mcg85,  0,      0,      mcg85,    0,      mcg85_state,  empty_init,  "Atlantis Computers",  "MCG-85",  MACHINE_NO_SOUND_HW )

// license:BSD-3-Clause
// copyright-holders:Devin Acker
/**********************************************************************

    Xetec C-5181 serial printer interface

	This interface allows printing to the IBM 5181 thermal printer and other Epson-style
	1200-baud serial printers.

	Currently this device prints to a generic RS232 printer instead.
	The RS232 port must be set to 1200 baud before printing.

	The original printer and interface were also rebranded as the Protecto "Big Blue" printer.
	Manual with interface-specific info and control characters:
	http://commodore.bombjack.org/commodore/printers/Protecto_Big_Blue_Printer_Interface_by_Supra.pdf

	There are some undocumented secondary addresses that behave similarly to other Xetec interfaces:
	- 1, 5, 8: same as 0, 4, 7 but without automatic line feed
	- 15: command channel, responds to any of the following characters:
		- '?': print hex dump of RAM
		- 'I': print interface info
		- 'Z': print Xetec logo image
		- 'U': unlock secondary address
		- '6': print 6 lines per inch (same as ESC 2)
		- '8': print 8 or 9 lines per inch (same as ESC 1, result depends on printer)
		- 'G': print graphic characters normally
		- 'A': print graphic characters' ASCII values
		- 'K': print graphic characters' keystrokes
		- 'M': print control codes as mnemonics
		- 'V': print control codes as symbols
		- 'X': print dot graphics data normally
		- 'N': print 7-bit dot graphics only

**********************************************************************/

#include "emu.h"
#include "c5181.h"

#include "bus/rs232/null_modem.h"
#include "bus/rs232/printer.h"
#include "bus/rs232/pty.h"


DEFINE_DEVICE_TYPE(XETEC_C5181, xetec_c5181_device, "xetec_c5181", "Xetec C-5181 Serial Printer Interface")

ROM_START( xetec_c5181 )
	ROM_REGION( 0x2000, "cpu", 0 )
	ROM_LOAD( "xetec c-5181 v1.0.bin", 0x0000, 0x2000, CRC(7ede21e7) SHA1(431c57e876c7432c07469c461e7438678ebb0043) )
ROM_END

const tiny_rom_entry *xetec_c5181_device::device_rom_region() const
{
	return ROM_NAME( xetec_c5181 );
}


xetec_c5181_device::xetec_c5181_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, XETEC_C5181, tag, owner, clock)
	, device_cbm_iec_interface(mconfig, *this)
	, m_cpu(*this, "cpu")
	, m_rs232(*this, "serial")
{
}

void xetec_c5181_device::map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
}

static DEVICE_INPUT_DEFAULTS_START(null_modem)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD",    0x00ff, RS232_BAUD_1200)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD",    0x00ff, RS232_BAUD_1200)
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START(printer)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD",    0x00ff, RS232_BAUD_1200)
DEVICE_INPUT_DEFAULTS_END

static void serial_devices(device_slot_interface &device)
{
	// TODO: replace this with the actual IBM 5181 printer
	device.option_add("null_modem",  NULL_MODEM);
	device.option_add("printer",     SERIAL_PRINTER);
	device.option_add("pty",         PSEUDO_TERMINAL);
}

void xetec_c5181_device::device_add_mconfig(machine_config &config)
{
	M146805E2(config, m_cpu, 5_MHz_XTAL);
	m_cpu->set_addrmap(AS_PROGRAM, &xetec_c5181_device::map);
	m_cpu->porta_r().set(FUNC(xetec_c5181_device::pa_r));
	m_cpu->porta_w().set(FUNC(xetec_c5181_device::pa_w));

	RS232_PORT(config, m_rs232, serial_devices, "printer");
	m_rs232->set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(null_modem));
	m_rs232->set_option_device_input_defaults("printer",    DEVICE_INPUT_DEFAULTS_NAME(printer));
	m_rs232->set_option_device_input_defaults("pty",        DEVICE_INPUT_DEFAULTS_NAME(null_modem));
	m_rs232->cts_handler().set_inputline(m_cpu, M6805_IRQ_LINE).invert();
}

void xetec_c5181_device::device_start()
{
}

void xetec_c5181_device::device_reset()
{
}

u8 xetec_c5181_device::pa_r()
{
	u8 data = 0x1f;

	if (m_bus->atn_r())   data |= 0x20;
	if (!m_bus->clk_r())  data |= 0x40;
	if (!m_bus->data_r()) data |= 0x80;

	return data;
}

void xetec_c5181_device::pa_w(u8 data)
{
	m_rs232->write_txd(BIT(data, 0));
	m_bus->data_w(this, BIT(~data, 1));
}

// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Preliminary driver for Space Byte 8085 CPU.

    This is an early Intel 8085-based single-board computer whose onboard
    I/O consists of RS232 ports for a CRT and serial printer and a parallel
    interface intended for use with ICOM's FD 3700 or Frugal Floppy disk
    controller. The resident monitor program also supports the Tarbell
    cassette tape interface and the PolyMorphic Systems Video Terminal
    Interface.

    The bus connector is only somewhat S-100 compatible. Anticipating the
    draft IEEE standard, no clock signal is placed on pin 25 or 49 (though
    documentation recommends jumpering pin 49 to the 3.072 MHz clock on pin
    24 for the VTI). The conventional INT and VI0-7 pins are disregarded in
    favor of collecting the four 8085-specific interrupts on pins 12-15 in
    ascending order of priority. SDSB/CDSB/ASDB/DODSB are also disregarded,
    and there is an alternate PHOLD input on pin 16.

    Space Byte also advertised a 16K static RAM board and a 2708/2716
    EPROM programmer for use in this system.

    Getting it to work with the terminal:
    1. Slot CRT: choose Terminal. (The VTI should be removed from S100 slot
       1 or else its keyboard must be disabled later.)
    2. Reboot
    3. Press A (certain other keys may work here). It will autodetect the
       baud rate and start up. (Only 110, 150, 300, 1200, 2400, 4800 and
       9600 baud are detected properly.)
    4. Now you can enter commands (such as D000,FFF)
    5. If it says KEYBOARD ERROR it doesn't mean a problem with the
       keyboard, it means you made a mistake.

    To use the monitor with the VTI, start by typing A (in uppercase).

***************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "bus/rs232/rs232.h"
#include "bus/s100/s100.h"
#include "bus/s100/polyvti.h"
#include "bus/s100/seals8k.h"
#include "machine/i8155.h"
#include "machine/i8251.h"


namespace {

class sb8085_state : public driver_device
{
public:
	sb8085_state(const machine_config &config, device_type type, const char *tag)
		: driver_device(config, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_i8155(*this, "i8155")
		, m_usart(*this, "usart")
		, m_crt(*this, "crt")
		, m_printer(*this, "printer")
		, m_s100(*this, "s100")
		, m_program(*this, "program")
		, m_config_select(*this, "SELECT")
		, m_portd(*this, "PORTD")
	{
	}

	static constexpr feature_type unemulated_features() { return feature::DISK | feature::TAPE; }

	void sb8085(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	bool board_selected(offs_t offset);
	u8 mem_r(offs_t offset);
	void mem_w(offs_t offset, u8 data);
	u8 in_r(offs_t offset);
	void out_w(offs_t offset, u8 data);
	void phantom_disable_w(int state);

	void printer_select_w(int state);
	void usart_txd_w(int state);
	void crt_rts_w(int state);
	void printer_rts_w(int state);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<i8085a_cpu_device> m_maincpu;
	required_device<i8155_device> m_i8155;
	required_device<i8251_device> m_usart;
	required_device<rs232_port_device> m_crt;
	required_device<rs232_port_device> m_printer;
	required_device<s100_bus_device> m_s100;
	required_region_ptr<u8> m_program;
	required_ioport m_config_select;
	required_ioport m_portd;

	bool m_phantom;
	bool m_printer_select;
	bool m_usart_txd;
	bool m_crt_rts;
	bool m_printer_rts;
};

void sb8085_state::machine_start()
{
	m_printer_select = false;
	m_usart_txd = true;
	m_crt_rts = true;
	m_printer_rts = true;

	// Pins 6, 8, 5 (printer) are tied to +12
	m_crt->write_dtr(0);
	m_printer->write_rts(0);
	m_printer->write_dtr(0);

	save_item(NAME(m_phantom));
	save_item(NAME(m_printer_select));
	save_item(NAME(m_usart_txd));
	save_item(NAME(m_crt_rts));
	save_item(NAME(m_printer_rts));
}

void sb8085_state::machine_reset()
{
	m_phantom = false;
}

bool sb8085_state::board_selected(offs_t offset)
{
	if (!m_phantom)
		return true;

	ioport_value config = m_config_select->read();
	return !BIT(config, 1) || (offset & 0xe000) == (BIT(config, 0) ? 0xe000 : 0xc000);
}

u8 sb8085_state::mem_r(offs_t offset)
{
	if (board_selected(offset))
	{
		if ((offset & 0x1c00) < 0x0c00)
			return m_program[offset & 0xfff];
		else if ((offset & 0x1c00) >= 0x1800)
			return m_i8155->memory_r(offset & 0xff);
		else
			return 0xff;
	}
	else
		return m_s100->smemr_r(offset);
}

void sb8085_state::mem_w(offs_t offset, u8 data)
{
	if (board_selected(offset))
	{
		if ((offset & 0x1c00) >= 0x1800)
			m_i8155->memory_w(offset & 0xff, data);
	}
	else
		m_s100->mwrt_w(offset, data);
}

u8 sb8085_state::in_r(offs_t offset)
{
	offset |= offset << 8;
	if (board_selected(offset))
	{
		if ((offset & 0x1c00) == 0x0000)
			return m_usart->read(BIT(offset, 8));
		else if ((offset & 0x1c00) == 0x0c00)
			return m_portd->read() ^ 0xff;
		else if ((offset & 0x1c00) >= 0x1800)
			return m_i8155->io_r(offset & 7);
		else
			return 0xff;
	}
	else
		return m_s100->sinp_r(offset);
}

void sb8085_state::out_w(offs_t offset, u8 data)
{
	offset |= offset << 8;
	if (board_selected(offset))
	{
		if ((offset & 0x1c00) == 0x0000)
			m_usart->write(BIT(offset, 8), data);
		else if ((offset & 0x1c00) >= 0x1800)
			m_i8155->io_w(offset & 7, data);
	}
	else
		m_s100->sout_w(offset, data);
}

void sb8085_state::phantom_disable_w(int state)
{
	if (!state)
		m_phantom = true;
}

void sb8085_state::printer_select_w(int state)
{
	if (state && m_printer_select)
	{
		m_printer_select = false;
		m_crt->write_txd(m_usart_txd);
		m_printer->write_txd(1);
		m_usart->write_cts(m_crt_rts);
	}
	else if (!state && !m_printer_select)
	{
		m_printer_select = true;
		m_crt->write_txd(1);
		m_printer->write_txd(m_usart_txd);
		m_usart->write_cts(m_printer_rts);
	}
}

void sb8085_state::usart_txd_w(int state)
{
	m_usart_txd = state;
	if (m_printer_select)
		m_printer->write_txd(state);
	else
		m_crt->write_txd(state);
}

void sb8085_state::crt_rts_w(int state)
{
	m_crt_rts = state;
	if (!m_printer_select)
		m_usart->write_cts(state);
}

void sb8085_state::printer_rts_w(int state)
{
	m_printer_rts = state;
	if (m_printer_select)
		m_usart->write_cts(state);
}

void sb8085_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(sb8085_state::mem_r), FUNC(sb8085_state::mem_w));
}

void sb8085_state::io_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(sb8085_state::in_r), FUNC(sb8085_state::out_w));
}

INPUT_PORTS_START(sb8085)
	PORT_START("SELECT")
	PORT_CONFNAME(3, 2, "Board select")
	PORT_CONFSETTING(0, "Single board")
	PORT_CONFSETTING(2, "C000-DFFFH")
	PORT_CONFSETTING(3, "E000-FFFFH")

	PORT_START("PORTD")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN) // from disk connector
INPUT_PORTS_END


static void sb8085_s100_devices(device_slot_interface &device)
{
	device.option_add("vti", S100_POLY_VTI);
	device.option_add("8ksc", S100_8K_SC);
	device.option_add("8kscbb", S100_8K_SC_BB);
}

static DEVICE_INPUT_DEFAULTS_START(terminal)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_9600)
	DEVICE_INPUT_DEFAULTS("RS232_DATABITS", 0xff, RS232_DATABITS_7)
	DEVICE_INPUT_DEFAULTS("RS232_PARITY", 0xff, RS232_PARITY_EVEN)
	DEVICE_INPUT_DEFAULTS("RS232_STOPBITS", 0xff, RS232_STOPBITS_2)
DEVICE_INPUT_DEFAULTS_END

void sb8085_state::sb8085(machine_config &config)
{
	I8085A(config, m_maincpu, 6.144_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &sb8085_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &sb8085_state::io_map);
	m_maincpu->in_sid_func().set(m_crt, FUNC(rs232_port_device::rxd_r));

	I8155(config, m_i8155, 6.144_MHz_XTAL / 2);
	m_i8155->out_pc_callback().set(FUNC(sb8085_state::phantom_disable_w)).bit(5);
	m_i8155->out_to_callback().set(m_usart, FUNC(i8251_device::write_txc));
	m_i8155->out_to_callback().append(m_usart, FUNC(i8251_device::write_rxc));

	I8251(config, m_usart, 6.144_MHz_XTAL / 2);
	m_usart->rts_handler().set(m_crt, FUNC(rs232_port_device::write_rts));
	m_usart->txd_handler().set(FUNC(sb8085_state::usart_txd_w));
	m_usart->dtr_handler().set(FUNC(sb8085_state::printer_select_w));

	RS232_PORT(config, m_crt, default_rs232_devices, nullptr);
	m_crt->rxd_handler().set(m_usart, FUNC(i8251_device::write_rxd));
	m_crt->cts_handler().set(FUNC(sb8085_state::crt_rts_w));
	m_crt->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	RS232_PORT(config, m_printer, default_rs232_devices, nullptr);
	m_printer->cts_handler().set(FUNC(sb8085_state::printer_rts_w));

	S100_BUS(config, m_s100, 6.144_MHz_XTAL / 2);
	S100_SLOT(config, "s100:1", sb8085_s100_devices, "vti");
	S100_SLOT(config, "s100:2", sb8085_s100_devices, nullptr);
	S100_SLOT(config, "s100:3", sb8085_s100_devices, nullptr);
	S100_SLOT(config, "s100:4", sb8085_s100_devices, nullptr);
}


ROM_START(sb8085)
	ROM_REGION(0xc00, "program", 0)
	// From manual listing titled "SPACE BYTE 8085 PROM MONITOR 11-14-77" to be programmed on 3 TMS2708JL
	ROM_LOAD("1.u19", 0x000, 0x400, CRC(9e1f11dd) SHA1(4c4482baa133dd6d437aaf749e31b72c9718eb97))
	ROM_LOAD("2.u20", 0x400, 0x400, CRC(2426af98) SHA1(b6e37041f997aeea13be79df10dc410f4b0c51a6))
	ROM_LOAD("3.u21", 0x800, 0x400, CRC(088ad01b) SHA1(6832e63dc1769db09107bc09f4c2cfb158dd8d33))
ROM_END

} // anonymous namespace


COMP(1977, sb8085, 0, 0, sb8085, sb8085, sb8085_state, empty_init, "Space Byte", "Space Byte 8085", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)

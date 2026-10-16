// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Pet Keitai Link cartridge with an EEPROM and an infrared transceiver. Its
    ROM implements IrLAP, IrLMP, TinyTP and OBEX, including SIR framing and
    checksums. PIO2.0 drives active-low transmit pulses; received pulses latch
    I/O event 0.

    Peer SIR bytes use a serial port at 9600 baud, 8N1, the only rate the ROM
    accepts in negotiation. This is an emulator transport; the cartridge has
    no physical RS-232 connector.

    TODO:
    - Identify the physical transceiver and verify its pulse/turnaround
      limits.

***************************************************************************/

#include "emu.h"
#include "suprtvpc_cart_ir.h"

DEFINE_DEVICE_TYPE(SUPRTVPC_ROM_24C64_IR, suprtvpc_rom_24c64_ir_device, "suprtvpc_rom_24c64_ir", "Super TV-PC Cartridge with 24C64 EEPROM and Infrared")

suprtvpc_rom_24c64_ir_device::suprtvpc_rom_24c64_ir_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: suprtvpc_rom_i2c_device(mconfig, SUPRTVPC_ROM_24C64_IR, tag, owner, clock)
	, device_serial_interface(mconfig, *this)
	, m_serial(*this, "serial")
{
}

void suprtvpc_rom_24c64_ir_device::device_add_mconfig(machine_config &config)
{
	I2C_24C64(config, m_eeprom);
	RS232_PORT(config, m_serial, default_rs232_devices, nullptr);
	m_serial->rxd_handler().set(FUNC(suprtvpc_rom_24c64_ir_device::rxd_w));
}

void suprtvpc_rom_24c64_ir_device::device_start()
{
	m_start_timer = timer_alloc(FUNC(suprtvpc_rom_24c64_ir_device::start_bit), this);
	m_pulse_timer = timer_alloc(FUNC(suprtvpc_rom_24c64_ir_device::pulse_end), this);
	save_item(NAME(m_tx_level));
	save_item(NAME(m_rxd));
}

void suprtvpc_rom_24c64_ir_device::device_reset()
{
	// The ROM negotiates only 9600 baud, 8N1; its rate-change routine has no effect.
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	m_start_timer->adjust(attotime::never);
	m_pulse_timer->adjust(attotime::never);
	set_rate(9600);
	m_rxd = true;
	rx_w(1);
	rx_clock_w(0);
	tx_clock_w(0);
	m_tx_level = true;
	m_serial->write_txd(1);
}

void suprtvpc_rom_24c64_ir_device::ir_tx_w(int state)
{
	if (m_tx_level && !state)
	{
		// Expand each SIR zero pulse across the UART sampling point.  A one bit
		// has no optical pulse.
		m_serial->write_txd(0);
		m_pulse_timer->adjust(attotime::from_hz(9600) * 3 / 4);
	}
	m_tx_level = bool(state);
}

TIMER_CALLBACK_MEMBER(suprtvpc_rom_24c64_ir_device::pulse_end)
{
	m_serial->write_txd(1);
}

void suprtvpc_rom_24c64_ir_device::rxd_w(int state)
{
	bool const receiving = is_receive_register_synchronized();
	m_rxd = bool(state);
	rx_w(state);
	if (!receiving && is_receive_register_synchronized())
		m_start_timer->adjust(attotime::from_hz(9600) / 2);
}

TIMER_CALLBACK_MEMBER(suprtvpc_rom_24c64_ir_device::start_bit)
{
	if (!m_rxd)
	{
		ir_rx_pulse();
	}
	else
	{
		// Reject a short start-bit glitch before sampling any data bits.
		set_rcv_rate(9600);
		rx_w(1);
		rx_clock_w(0);
	}
}

void suprtvpc_rom_24c64_ir_device::rcv_callback()
{
	// Sample at the same phase as the start token, one bit apart.  Each zero
	// represents an optical pulse, including consecutive zero data bits.
	if (!m_rxd)
		ir_rx_pulse();
	device_serial_interface::rcv_callback();
}

void suprtvpc_rom_24c64_ir_device::rcv_complete()
{
	receive_register_extract();
}

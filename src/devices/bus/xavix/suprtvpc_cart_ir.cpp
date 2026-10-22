// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn
/***************************************************************************

    Pet Keitai Link cartridge with an EEPROM and an infrared transceiver.
    PIO2.0 drives active-low transmit pulses; received pulses latch I/O
    event 0. A byte-stream image connects an external SIR peer at 9600
    baud, 8N1. Protocol framing and checksums remain in the ROM and peer.

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
	, m_stream(*this, "stream")
{
}

void suprtvpc_rom_24c64_ir_device::device_add_mconfig(machine_config &config)
{
	I2C_24C64(config, m_eeprom);
	BITBANGER(config, m_stream);
}

void suprtvpc_rom_24c64_ir_device::device_start()
{
	suprtvpc_rom_i2c_device::device_start();
	m_poll_timer = timer_alloc(FUNC(suprtvpc_rom_24c64_ir_device::poll_stream), this);
	m_transmit_timer = timer_alloc(FUNC(suprtvpc_rom_24c64_ir_device::transmit_pulse_end), this);
	m_receive_timer = timer_alloc(FUNC(suprtvpc_rom_24c64_ir_device::receive_pulse), this);
	save_item(NAME(m_tx_level));
}

void suprtvpc_rom_24c64_ir_device::device_reset()
{
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rate(BAUD);
	m_transmit_timer->adjust(attotime::never);
	m_receive_timer->adjust(attotime::never);
	m_tx_level = true;
	rx_w(1);
	rx_clock_w(0);
	tx_clock_w(0);
	m_poll_timer->adjust(attotime::zero);
}

void suprtvpc_rom_24c64_ir_device::ir_tx_w(int state)
{
	if (m_tx_level && !state)
	{
		// Expand an optical zero pulse across the UART sampling point.
		rx_w(0);
		m_transmit_timer->adjust(attotime::from_hz(BAUD) * 3 / 4);
	}
	m_tx_level = bool(state);
}

TIMER_CALLBACK_MEMBER(suprtvpc_rom_24c64_ir_device::transmit_pulse_end)
{
	rx_w(1);
}

void suprtvpc_rom_24c64_ir_device::rcv_complete()
{
	receive_register_extract();
	m_stream->output(get_received_char());
}

TIMER_CALLBACK_MEMBER(suprtvpc_rom_24c64_ir_device::poll_stream)
{
	u8 data;
	if (m_stream->input(&data, 1))
		transmit_register_setup(data);
	else
		m_poll_timer->adjust(attotime::from_hz(BAUD));
}

void suprtvpc_rom_24c64_ir_device::tra_callback()
{
	// Each zero emits one receive pulse at the middle of its bit cell.
	if (!transmit_register_get_data_bit())
		m_receive_timer->adjust(attotime::from_hz(BAUD) / 2);
}

void suprtvpc_rom_24c64_ir_device::tra_complete()
{
	poll_stream(0);
}

TIMER_CALLBACK_MEMBER(suprtvpc_rom_24c64_ir_device::receive_pulse)
{
	ir_rx_pulse();
}

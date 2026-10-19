// license:BSD-3-Clause
// copyright-holders:Felipe Sanches
/***************************************************************************

    TOSHIBA TLCS900 - TMP95C061 SERIAL CHANNEL

***************************************************************************/

#include "emu.h"
#include "tmp95c061_serial.h"


DEFINE_DEVICE_TYPE(TMP95C061_SERIAL, tmp95c061_serial_device, "tmp95c061_serial", "TMP95C061 Serial Channel")

tmp95c061_serial_device::tmp95c061_serial_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TMP95C061_SERIAL, tag, owner, clock),
	device_serial_interface(mconfig, *this),
	m_setint_cb(*this),
	m_txd_cb(*this),
	m_tx_byte_cb(*this)
{
}


void tmp95c061_serial_device::device_start()
{
	save_item(NAME(m_cr));
	save_item(NAME(m_mod));
	save_item(NAME(m_brcr));
	save_item(NAME(m_rx_data));
	save_item(NAME(m_tx_hold));
	save_item(NAME(m_tx_hold_full));
	save_item(NAME(m_tx_busy));
	save_item(NAME(m_pin_enabled));
}


void tmp95c061_serial_device::device_reset()
{
	// bit 7 of SCxCR and SCxMOD survives a reset, as it did when the CPU held
	// these registers itself
	m_cr &= 0x80;
	m_mod &= 0x80;
	m_brcr = 0;
	m_rx_data = m_tx_hold = 0;
	m_tx_hold_full = m_tx_busy = false;

	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	transmit_register_reset();
	receive_register_reset();
	m_txd_cb(1);                             // TXD idles high
}


uint8_t tmp95c061_serial_device::sccr_r()
{
	uint8_t const reg = m_cr;
	if (!machine().side_effects_disabled())
		m_cr &= 0xe3;                        // the error flags clear on read
	return reg;
}


void tmp95c061_serial_device::scmod_w(uint8_t data)
{
	m_mod = data;
	update_rate();
}


//  BRxCR: bits 3:0 are the divisor and bits 5:4 pick the prescaler tap, each
//  tap two shifts below the last.  The asynchronous modes divide by a further
//  16.  A divisor of zero stops the generator.
void tmp95c061_serial_device::brcr_w(uint8_t data)
{
	m_brcr = data;
	update_rate();
}


void tmp95c061_serial_device::update_rate()
{
	if (!bit_mode())
		return;

	unsigned const divisor = m_brcr & 0x0f;
	if (!divisor)
	{
		set_rate(0);
		return;
	}
	unsigned const shift = (((m_brcr >> 4) & 3) + 1) * 2;
	set_rate(clock() / (divisor << shift) / 16);
}


void tmp95c061_serial_device::scbuf_w(uint8_t data)
{
	if (!bit_mode())
	{
		// byte granularity: hand the whole byte over and report the transmit
		// finished at once, which is what this core did before the channel
		// could shift bits.
		m_setint_cb(0x80);
		if ((m_mod & MOD_SM_MASK) != 0 || m_pin_enabled)
			m_tx_byte_cb(data);
		return;
	}

	if (m_tx_busy)
	{
		m_tx_hold = data;                    // SCxBUF double buffering
		m_tx_hold_full = true;
		return;
	}

	m_tx_busy = true;
	transmit_register_setup(data);
}


void tmp95c061_serial_device::tra_complete()
{
	m_setint_cb(0x80);

	if (m_tx_hold_full)
	{
		m_tx_hold_full = false;
		transmit_register_setup(m_tx_hold);
	}
	else
	{
		m_tx_busy = false;
	}
}


void tmp95c061_serial_device::rcv_complete()
{
	receive_register_extract();
	rx_byte(get_received_char());
}


void tmp95c061_serial_device::rx_byte(uint8_t data)
{
	if (!(m_mod & MOD_RXE))
		return;

	m_rx_data = data;
	m_setint_cb(0x08);
}

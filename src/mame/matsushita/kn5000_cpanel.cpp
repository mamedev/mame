// license:GPL2+
// copyright-holders:Felipe Sanches
/***************************************************************************

	KN5000 control panel

***************************************************************************/

#include "emu.h"
#include "kn5000_cpanel.h"

#include "logmacro.h"

DEFINE_DEVICE_TYPE(KN5000_CPANEL, kn5000_cpanel_device, "kn5000_cpanel", "KN5000 Control Panel")

kn5000_cpanel_device::kn5000_cpanel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, KN5000_CPANEL, tag, owner, clock),
	m_sck_out_cb(*this),
	m_serial_out_cb(*this),
	m_serial_in(0),
	m_sck_in(1),
	m_sck_out(1),
	m_clock_count(0),
	m_tx_shift_register(0),
	m_rx_shift_register(0)
{
}

void kn5000_cpanel_device::device_start()
{
	save_item(NAME(m_sck_in));
	save_item(NAME(m_sck_out));
	save_item(NAME(m_serial_in));
	save_item(NAME(m_clock_count));
	save_item(NAME(m_tx_shift_register));
	save_item(NAME(m_rx_shift_register));

	m_sck_out_cb(m_sck_out);
	sck_in(0);
	serial_in(0);
}

void kn5000_cpanel_device::device_reset()
{
}

void kn5000_cpanel_device::sck_in(int state)
{
	if (m_sck_in == state)
		return;

	m_sck_in = state;

	// logerror("sck_in state=%d\n", state);
	if (state != 1)
		return;

	m_rx_shift_register >>= 1;
	m_rx_shift_register |= (m_serial_in << 7);

	logerror("           rx bit #%d: %d  cur_value=%02X\n", m_clock_count, m_serial_in, m_rx_shift_register);

	if (++m_clock_count == 8)
	{
		logerror("           Received: %02X\n", m_rx_shift_register);
		m_clock_count = 0;
	}
}

void kn5000_cpanel_device::serial_in(int state)
{
	m_serial_in = state;
}

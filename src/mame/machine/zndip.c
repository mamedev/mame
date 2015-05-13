// license:BSD-3-Clause
// copyright-holders:smf
#include "zndip.h"

const device_type ZNDIP = &device_creator<zndip_device>;

zndip_device::zndip_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ZNDIP, "Sony ZNDIP", tag, owner, clock, "zndip", __FILE__),
	m_data_handler(*this),
	m_dataout_handler(*this),
	m_dsr_handler(*this)
{
}

void zndip_device::device_start()
{
	m_data_handler.resolve_safe( 0 );
	m_dataout_handler.resolve_safe();
	m_dsr_handler.resolve_safe();

	m_dip_timer = timer_alloc( 0 );

	m_dataout_handler(1);
	m_dsr_handler(1);
}

WRITE_LINE_MEMBER(zndip_device::write_select)
{
	if (!state && m_select)
	{
		m_bit = 0;
		m_dip_timer->adjust( attotime::from_usec( 100 ), 0 );
	}
	else
	{
		m_dataout_handler(1);
		m_dsr_handler(1);
	}

	m_select = state;
}

WRITE_LINE_MEMBER(zndip_device::write_clock)
{
	if (!state && m_clock && !m_select)
	{
		int dip = m_data_handler();
		int bit = ( ( dip >> m_bit ) & 1 );
//      verboselog( machine, 2, "read dip %02x -> %02x\n", n_data, bit * PSX_SIO_IN_DATA );
		m_dataout_handler(bit);
		m_bit++;
		m_bit &= 7;
	}

	m_clock = state;
}

void zndip_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	m_dsr_handler(param);

	if( !param )
	{
		m_dip_timer->adjust( attotime::from_usec( 50 ), 1 );
	}
}

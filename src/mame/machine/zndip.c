#include "zndip.h"

const device_type ZNDIP = &device_creator<zndip_device>;

zndip_device::zndip_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	psxsiodev_device(mconfig, ZNDIP, "ZNDIP", tag, owner, clock),
	m_data_handler(*this)
{
}

void zndip_device::device_start()
{
	psxsiodev_device::device_start();

	m_data_handler.resolve_safe( 0 );

	m_dip_timer = timer_alloc( 0 );
}

void zndip_device::select(int select)
{
	if (m_select != select)
	{
		if (!select)
		{
			m_bit = 0;
			m_dip_timer->adjust( attotime::from_usec( 100 ), 1 );
		}
		else
		{
			data_out(0, PSX_SIO_IN_DATA | PSX_SIO_IN_DSR);
		}

		m_select = select;
	}
}

void zndip_device::data_in( int data, int mask )
{
	if( !m_select && ( mask & PSX_SIO_OUT_CLOCK ) != 0 && ( data & PSX_SIO_OUT_CLOCK ) == 0)
	{
		int dip = m_data_handler();
		int bit = ( ( dip >> m_bit ) & 1 );
//      verboselog( machine, 2, "read dip %02x -> %02x\n", n_data, bit * PSX_SIO_IN_DATA );
		data_out( bit * PSX_SIO_IN_DATA, PSX_SIO_IN_DATA );
		m_bit++;
		m_bit &= 7;
	}
}

void zndip_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	data_out( param * PSX_SIO_IN_DSR, PSX_SIO_IN_DSR );

	if( param )
	{
		m_dip_timer->adjust( attotime::from_usec( 50 ), 0 );
	}
}

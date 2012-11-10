#include "sio.h"
#include "siodev.h"

psxsiodev_device::psxsiodev_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, type, name, tag, owner, clock)
{
}

void psxsiodev_device::device_start()
{
	m_dataout = 0;
}

void psxsiodev_device::data_out( int data, int mask )
{
	m_dataout = ( m_dataout & ~mask ) | ( data & mask );

	m_psxsio->input_update();
}

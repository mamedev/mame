#include "k573cass.h"

const device_type KONAMI573CASSETTE = &device_creator<konami573cassette_device>;

konami573cassette_device::konami573cassette_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	psxsiodev_device(mconfig, KONAMI573CASSETTE, "KONAMI 573 CASSETTE", tag, owner, clock)
{
}

void konami573cassette_device::device_start()
{
	psxsiodev_device::device_start();

	data_out( PSX_SIO_IN_DSR, PSX_SIO_IN_DSR );
}

void konami573cassette_device::data_in( int data, int mask )
{
}

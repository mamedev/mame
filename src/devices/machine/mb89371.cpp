// license:BSD-3-Clause
// copyright-holders:smf
/*
 * MB89371
 *
 * Fujitsu
 * Dual Serial UART
 *
 */

#include "mb89371.h"

const device_type MB89371 = &device_creator<mb89371_device>;

mb89371_device::mb89371_device( const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock )
	: device_t(mconfig, MB89371, "MB89371 Dual Serial UART", tag, owner, clock, "mb89371", __FILE__)
{
}

void mb89371_device::device_start()
{
}

WRITE8_MEMBER(mb89371_device::write)
{
}

READ8_MEMBER(mb89371_device::read)
{
	return 0xff;
}

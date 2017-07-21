// license:BSD-3-Clause
// copyright-holders:smf
/*
 * MB89371
 *
 * Fujitsu
 * Dual Serial UART
 *
 */

#include "emu.h"
#include "mb89371.h"

DEFINE_DEVICE_TYPE(MB89371, mb89371_device, "mb89371", "MB89371 Dual Serial UART")

mb89371_device::mb89371_device( const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock )
	: device_t(mconfig, MB89371, tag, owner, clock)
{
}

void mb89371_device::device_start()
{
}

WRITE8_MEMBER(mb89371_device::write)
{
	logerror("MB89371 unimplemented write @%X = %02X & %02X\n", offset, data, mem_mask);
}

READ8_MEMBER(mb89371_device::read)
{
	logerror("MB89371 unimplemented read @%X & %02X\n", offset, mem_mask);
	return 0xff;
}

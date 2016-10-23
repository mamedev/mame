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

mb89371_device::mb89371_device( const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock )
	: device_t(mconfig, MB89371, "MB89371 Dual Serial UART", tag, owner, clock, "mb89371", __FILE__)
{
}

void mb89371_device::device_start()
{
}

void mb89371_device::write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
}

uint8_t mb89371_device::read(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return 0xff;
}

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

void mb89371_device::write(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	switch (offset)
	{
	case 0: // data
		//printf("%c", data);
		break;
	case 1: // control (0x40 = error reset)
	case 2: // baud (9600 = 2)
	case 3: // mode (8251 compatible?)
		break;
	}
	logerror("MB89371 unimplemented write @%X = %02X & %02X\n", offset, data, mem_mask);
}

uint8_t mb89371_device::read(offs_t offset, uint8_t mem_mask)
{
	switch (offset)
	{
	case 0x00: // data
		break;
	case 0x01: // control
		// bit 0 = txrdy, bit 1 = rxrdy
		break;
	}
	logerror("MB89371 unimplemented read @%X & %02X\n", offset, mem_mask);
	return 0xff;
}

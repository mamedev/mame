// license:BSD-3-Clause
// copyright-holders:smf
/*
 * MB89371
 *
 * Fujitsu
 * Dual Serial UART
 *
 */

#ifndef __MB89371_H__
#define __MB89371_H__

#include "emu.h"

class mb89371_device : public device_t
{
public:
	// construction/destruction
	mb89371_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

protected:

	// device-level overrides
	virtual void device_start() override;
};


// device type definition
extern const device_type MB89371;

#endif

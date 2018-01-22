// license:BSD-3-Clause
// copyright-holders:smf
/*
 * MB89371
 *
 * Fujitsu
 * Dual Serial UART
 *
 */

#ifndef MAME_MACHINE_MB89371_H
#define MAME_MACHINE_MB89371_H

#pragma once


class mb89371_device : public device_t
{
public:
	// construction/destruction
	mb89371_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

protected:
	// device-level overrides
	virtual void device_start() override;
};


// device type definition
DECLARE_DEVICE_TYPE(MB89371, mb89371_device)

#endif // MAME_MACHINE_MB89371_H

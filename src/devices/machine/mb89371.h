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
	mb89371_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

protected:

	// device-level overrides
	virtual void device_start() override;
};


// device type definition
extern const device_type MB89371;

#endif

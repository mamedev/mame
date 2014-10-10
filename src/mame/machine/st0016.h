/* ST0016 - CPU (z80) + Sound + Video */

#pragma once

#ifndef __ST0016_CPU__
#define __ST0016_CPU__

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/st0016.h"

extern UINT8 *st0016_charram;


class st0016_cpu_device : public z80_device
{
public:
	st0016_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_start();
	virtual void device_reset();

	const address_space_config m_io_space_config;
	const address_space_config m_space_config;


	const address_space_config *memory_space_config(address_spacenum spacenum) const
	{
		switch (spacenum)
		{
			case AS_IO: return &m_io_space_config;
			case AS_PROGRAM: return &m_space_config;
			default: return z80_device::memory_space_config(spacenum);
		}
	}


private:

};


// device type definition
extern const device_type ST0016_CPU;


#endif /// __ST0016_CPU__

// license:BSD-3-Clause
// copyright-holders:
#ifndef __VCS_DPCPLUS_H
#define __VCS_DPCPLUS_H

#include "rom.h"
#include "cpu/arm7/arm7.h"


// ======================> a26_rom_dpcplus_device

class a26_rom_dpcplus_device : public a26_rom_f8_device
{
public:
	// construction/destruction
	a26_rom_dpcplus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_reset();

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom);
	virtual DECLARE_WRITE8_MEMBER(write_bank);


protected:
};


// device type definition
extern const device_type A26_ROM_DPCPLUS;

#endif

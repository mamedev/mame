// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __SAT_DRAM_H
#define __SAT_DRAM_H

#include "sat_slot.h"


// ======================> saturn_dram_device

class saturn_dram_device : public device_t,
							public device_sat_cart_interface
{
public:
	// construction/destruction
	saturn_dram_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// reading and writing
	virtual DECLARE_READ32_MEMBER(read_ext_dram0) override;
	virtual DECLARE_READ32_MEMBER(read_ext_dram1) override;
	virtual DECLARE_WRITE32_MEMBER(write_ext_dram0) override;
	virtual DECLARE_WRITE32_MEMBER(write_ext_dram1) override;
};

class saturn_dram8mb_device : public saturn_dram_device
{
public:
	// construction/destruction
	saturn_dram8mb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class saturn_dram32mb_device : public saturn_dram_device
{
public:
	// construction/destruction
	saturn_dram32mb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};



// device type definition
extern const device_type SATURN_DRAM_8MB;
extern const device_type SATURN_DRAM_32MB;

#endif

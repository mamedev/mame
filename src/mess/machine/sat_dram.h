#ifndef __SAT_DRAM_H
#define __SAT_DRAM_H

#include "machine/sat_slot.h"


// ======================> saturn_dram_device

class saturn_dram_device : public device_t,
							public device_sat_cart_interface
{
public:
	// construction/destruction
	saturn_dram_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 size);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "sat_dram"; }

	// reading and writing
	virtual DECLARE_READ32_MEMBER(read_ext_dram0);
	virtual DECLARE_READ32_MEMBER(read_ext_dram1);
	virtual DECLARE_WRITE32_MEMBER(write_ext_dram0);
	virtual DECLARE_WRITE32_MEMBER(write_ext_dram1);

	UINT32 m_size;  // this is the size of DRAM0 + DRAM1 in dword units, so accesses to each bank go up to (m_size/2)-1
};

class saturn_dram8mb_device : public saturn_dram_device
{
public:
	// construction/destruction
	saturn_dram8mb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_config_complete() { m_shortname = "sat_dram_8mb"; }
};

class saturn_dram32mb_device : public saturn_dram_device
{
public:
	// construction/destruction
	saturn_dram32mb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_config_complete() { m_shortname = "sat_dram_32mb"; }
};



// device type definition
extern const device_type SATURN_DRAM_8MB;
extern const device_type SATURN_DRAM_32MB;

#endif

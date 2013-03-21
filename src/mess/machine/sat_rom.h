#ifndef __SAT_ROM_H
#define __SAT_ROM_H

#include "machine/sat_slot.h"


// ======================> saturn_rom_device

class saturn_rom_device : public device_t,
						public device_sat_cart_interface
{
public:
	// construction/destruction
	saturn_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	saturn_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "sat_rom"; }

	// reading and writing
	virtual DECLARE_READ32_MEMBER(read_rom);
};



// device type definition
extern const device_type SATURN_ROM;


#endif

// license:<license_opt>
// copyright-holders:<author_name>
/***************************************************************************

<device_longname>

***************************************************************************/

#ifndef MAME_MACHINE_<device_header>_H
#define MAME_MACHINE_<device_header>_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class <device_classname>_device : public device_t
{
public:
	// construction/destruction
	<device_classname>_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = ~0);

protected:
	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
};


// device type definition
DECLARE_DEVICE_TYPE(<device_typename>, <device_classname>_device)

#endif // MAME_MACHINE_<device_header>_H

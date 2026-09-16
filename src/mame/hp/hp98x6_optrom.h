// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp98x6_optrom.h

    Optional ROMs for HP98x6 systems

*********************************************************************/
#ifndef MAME_HP_HP98X6_OPTROM_H
#define MAME_HP_HP98X6_OPTROM_H

#pragma once

#include "imagedev/cartrom.h"

class hp98x6_optrom_device : public device_t,
							 public device_rom_image_interface
{
public:
	// construction/destruction
	hp98x6_optrom_device(machine_config const &mconfig, char const *tag, device_t *owner);
	hp98x6_optrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp98x6_optrom_device();

	void install_handlers(address_space *space_r);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "hp98x6_rom"; }
	virtual const char *file_extensions() const noexcept override { return "bin"; }

	address_space *m_space_r;
};

// device type definition
DECLARE_DEVICE_TYPE(HP98X6_OPTROM, hp98x6_optrom_device)

#endif /* MAME_HP_HP98X6_OPTROM_H */

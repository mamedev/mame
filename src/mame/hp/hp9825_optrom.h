// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp9825_optrom.h

    Optional ROMs for HP9825 systems

*********************************************************************/
#ifndef MAME_HP_HP9825_OPTROM_H
#define MAME_HP_HP9825_OPTROM_H

#pragma once

#include "machine/bankdev.h"
#include "imagedev/cartrom.h"

class hp9825_optrom_device : public device_t,
							 public device_rom_image_interface
{
public:
	// construction/destruction
	hp9825_optrom_device(machine_config const &mconfig, char const *tag, device_t *owner);
	hp9825_optrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp9825_optrom_device();

	void set_rom_limit(offs_t rom_limit) { m_rom_limit = rom_limit; }

	void install_rw_handlers(address_space *space_r , address_space *space_w);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "hp9825_rom"; }
	virtual const char *file_extensions() const noexcept override { return "bin"; }

	offs_t m_rom_limit;
	unsigned m_loaded_regions;
	address_space *m_space_r;
	required_device<address_map_bank_device> m_bank;
};

// device type definition
DECLARE_DEVICE_TYPE(HP9825_OPTROM, hp9825_optrom_device)

#endif /* MAME_HP_HP9825_OPTROM_H */

// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp9825_optrom.h

    Optional ROMs for HP9825 systems

*********************************************************************/
#ifndef MAME_MACHINE_HP9825_OPTROM_H
#define MAME_MACHINE_HP9825_OPTROM_H

#pragma once

#include "machine/bankdev.h"
#include "softlist_dev.h"

class hp9825_optrom_device : public device_t,
							 public device_image_interface
{
public:
	// construction/destruction
	hp9825_optrom_device(machine_config const &mconfig, char const *tag, device_t *owner);
	hp9825_optrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp9825_optrom_device();

	void set_rom_limit(offs_t rom_limit) { m_rom_limit = rom_limit; }

	void install_rw_handlers(address_space *space_r , address_space *space_w);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override;
	virtual const software_list_loader &get_software_list_loader() const override { return rom_software_list_loader::instance(); }

	virtual iodevice_t image_type() const noexcept override { return IO_ROM; }
	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return false; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool must_be_loaded() const noexcept override { return false; }
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

#endif /* MAME_MACHINE_HP9825_OPTROM_H */

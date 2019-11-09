// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp80_optrom.h

    Optional ROMs for HP80 systems

*********************************************************************/

#ifndef MAME_MACHINE_HP80_OPTROM_H
#define MAME_MACHINE_HP80_OPTROM_H

#pragma once

#include "softlist_dev.h"

// Size of optional ROMs (8k)
static constexpr offs_t HP80_OPTROM_SIZE = 0x2000;

class hp80_optrom_device : public device_t,
						   public device_image_interface
{
public:
	// construction/destruction
	hp80_optrom_device(machine_config const &mconfig, char const *tag, device_t *owner)
		: hp80_optrom_device(mconfig, tag, owner, (uint32_t)0)
	{
	}

	hp80_optrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp80_optrom_device();

	void install_read_handler(address_space& space);

protected:
	// device-level overrides
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
	virtual const char *image_interface() const noexcept override { return "hp80_rom"; }
	virtual const char *file_extensions() const noexcept override { return "bin"; }

	uint8_t m_select_code;
};

// device type definition
DECLARE_DEVICE_TYPE(HP80_OPTROM, hp80_optrom_device)

#endif // MAME_MACHINE_HP80_OPTROM_H

// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp9845_optrom.h

    Optional ROMs for HP9845 systems

*********************************************************************/

#ifndef MAME_MACHINE_HP9845_OPTROM_H
#define MAME_MACHINE_HP9845_OPTROM_H

#pragma once

#include "softlist_dev.h"

class hp9845_optrom_device : public device_t,
							 public device_image_interface
{
public:
		// construction/destruction
	hp9845_optrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp9845_optrom_device();

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
	virtual const char *image_interface() const noexcept override { return "hp9845b_rom"; }
	virtual const char *file_extensions() const noexcept override { return "bin"; }

	offs_t m_base_addr;
	offs_t m_end_addr;
};

// device type definition
DECLARE_DEVICE_TYPE(HP9845_OPTROM, hp9845_optrom_device)

#endif // MAME_MACHINE_HP9845_OPTROM_H

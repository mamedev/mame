// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp9845_optrom.h

    Optional ROMs for HP9845 systems

*********************************************************************/

#ifndef MAME_HP_HP9845_OPTROM_H
#define MAME_HP_HP9845_OPTROM_H

#pragma once

#include "imagedev/cartrom.h"

class hp9845_optrom_device : public device_t,
							 public device_rom_image_interface
{
public:
	// construction/destruction
	hp9845_optrom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp9845_optrom_device();

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_rom_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "hp9845b_rom"; }
	virtual const char *file_extensions() const noexcept override { return "bin"; }

	offs_t m_base_addr;
	offs_t m_end_addr;
};

// device type definition
DECLARE_DEVICE_TYPE(HP9845_OPTROM, hp9845_optrom_device)

#endif // MAME_HP_HP9845_OPTROM_H

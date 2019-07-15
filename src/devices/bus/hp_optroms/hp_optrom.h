// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp_optrom.h

    Optional ROMs for HP9845 systems

*********************************************************************/

#ifndef MAME_BUS_HP_OPTROMS_HP_OPTROM_H
#define MAME_BUS_HP_OPTROMS_HP_OPTROM_H

#pragma once

#include "softlist_dev.h"


class hp_optrom_cart_device : public device_t,
								public device_slot_card_interface
{
public:
	// construction/destruction
	hp_optrom_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	hp_optrom_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { }
};

class hp_optrom_slot_device : public device_t,
								public device_image_interface,
								public device_slot_interface
{
public:
		// construction/destruction
	hp_optrom_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp_optrom_slot_device();

protected:
	// device-level overrides
	virtual void device_start() override;

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override;
	virtual const software_list_loader &get_software_list_loader() const override { return rom_software_list_loader::instance(); }

	virtual iodevice_t image_type() const override { return IO_ROM; }
	virtual bool is_readable()  const override { return true; }
	virtual bool is_writeable() const override { return false; }
	virtual bool is_creatable() const override { return false; }
	virtual bool must_be_loaded() const override { return false; }
	virtual bool is_reset_on_load() const override { return true; }
	virtual const char *image_interface() const override { return "hp9845b_rom"; }
	virtual const char *file_extensions() const override { return "bin"; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	hp_optrom_cart_device *m_cart;
	std::vector<uint8_t> m_content;
	offs_t m_base_addr;
	offs_t m_end_addr;
};

// device type definition
DECLARE_DEVICE_TYPE(HP_OPTROM_SLOT, hp_optrom_slot_device)
DECLARE_DEVICE_TYPE(HP_OPTROM_CART, hp_optrom_cart_device)

#endif // MAME_BUS_HP_OPTROMS_HP_OPTROM_H

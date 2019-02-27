// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp80_optrom.h

    Optional ROMs for HP80 systems

*********************************************************************/

#ifndef MAME_BUS_HP80_OPTROMS_HP80_OPTROM_H
#define MAME_BUS_HP80_OPTROMS_HP80_OPTROM_H

#pragma once

#include "softlist_dev.h"

// Size of optional ROMs (8k)
static constexpr offs_t HP80_OPTROM_SIZE = 0x2000;

void hp80_optrom_slot_devices(device_slot_interface &device);

class hp80_optrom_cart_device : public device_t,
								public device_slot_card_interface
{
public:
	// construction/destruction
	hp80_optrom_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	hp80_optrom_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { }
};

class hp80_optrom_slot_device : public device_t,
								public device_image_interface,
								public device_slot_interface
{
public:
	// construction/destruction
	hp80_optrom_slot_device(machine_config const &mconfig, char const *tag, device_t *owner)
		: hp80_optrom_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		hp80_optrom_slot_devices(*this);
		set_default_option(nullptr);
		set_fixed(false);
	}

	hp80_optrom_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp80_optrom_slot_device();

	void install_read_handler(address_space& space);

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
	virtual const char *image_interface() const override { return "hp80_rom"; }
	virtual const char *file_extensions() const override { return "bin"; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	hp80_optrom_cart_device *m_cart;
	uint8_t m_select_code;
};

// device type definition
DECLARE_DEVICE_TYPE(HP80_OPTROM_SLOT, hp80_optrom_slot_device)
DECLARE_DEVICE_TYPE(HP80_OPTROM_CART, hp80_optrom_cart_device)

#endif // MAME_BUS_HP80_OPTROMS_HP80_OPTROM_H

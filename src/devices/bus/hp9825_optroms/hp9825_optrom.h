// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp9825_optrom.h

    Optional ROMs for HP9825 systems

*********************************************************************/
#ifndef MAME_BUS_HP9825_OPTROMS_HP9825_OPTROM_H
#define MAME_BUS_HP9825_OPTROMS_HP9825_OPTROM_H

#pragma once

#include "softlist_dev.h"
#include "machine/bankdev.h"

class hp9825_optrom_cart_device : public device_t,
								  public device_slot_card_interface
{
public:
	// construction/destruction
	hp9825_optrom_cart_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	hp9825_optrom_cart_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override { }
};

class hp9825_optrom_slot_device : public device_t,
								  public device_image_interface,
								  public device_slot_interface
{
public:
	// construction/destruction
	hp9825_optrom_slot_device(machine_config const &mconfig, char const *tag, device_t *owner);
	hp9825_optrom_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp9825_optrom_slot_device();

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

	virtual iodevice_t image_type() const override { return IO_ROM; }
	virtual bool is_readable()  const override { return true; }
	virtual bool is_writeable() const override { return false; }
	virtual bool is_creatable() const override { return false; }
	virtual bool must_be_loaded() const override { return false; }
	virtual bool is_reset_on_load() const override { return true; }
	virtual const char *image_interface() const override { return "hp9825_rom"; }
	virtual const char *file_extensions() const override { return "bin"; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	hp9825_optrom_cart_device *m_cart;
	offs_t m_rom_limit;
	unsigned m_loaded_regions;
	address_space *m_space_r;
	required_device<address_map_bank_device> m_bank;
};

// device type definition
DECLARE_DEVICE_TYPE(HP9825_OPTROM_SLOT, hp9825_optrom_slot_device)
DECLARE_DEVICE_TYPE(HP9825_OPTROM_CART, hp9825_optrom_cart_device)

#endif /* MAME_BUS_HP9825_OPTROMS_HP9825_OPTROM_H */

// license:BSD-3-Clause
// copyright-holders:tim lindner
/*********************************************************************

    mc10_cart.h

    MC-10 / Alice cartridge management

*********************************************************************/

#ifndef MAME_BUS_MC10_MC10CART_H
#define MAME_BUS_MC10_MC10CART_H

#pragma once

#include "imagedev/cartrom.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


// ======================> mc10cart_slot_device
class device_mc10cart_interface;

class mc10cart_slot_device final : public device_t,
								public device_single_card_slot_interface<device_mc10cart_interface>,
								public device_cartrom_image_interface
{
public:

	// construction/destruction
	template <typename T>
	mc10cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&opts, const char *dflt)
		: mc10cart_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	mc10cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// inline configuration
	template <typename T> void set_memspace(T &&tag, int spacenum) { m_memspace.set_tag(std::forward<T>(tag), spacenum); }
	auto nmi_callback() { return m_nmi_callback.bind(); }

	// address map manipulations
	address_space &memspace() const { return *m_memspace; }

	// device-level overrides
	virtual void device_start() override;

	// image-level overrides
	virtual image_init_result call_load() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "mc10_cart"; }
	virtual const char *file_extensions() const noexcept override { return "mcc,rom"; }

	// manipulation of nmi line
	void set_nmi_line(int state);
	devcb_write_line m_nmi_callback;

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

private:

	// cartridge
	device_mc10cart_interface *m_cart;

protected:
	required_address_space m_memspace;
};

// device type definition
DECLARE_DEVICE_TYPE(MC10CART_SLOT, mc10cart_slot_device)

class device_mc10cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_mc10cart_interface();

	virtual int max_rom_length() const;
	virtual image_init_result load();

protected:
	void raise_cart_nmi() { m_owning_slot->set_nmi_line(ASSERT_LINE); }
	void lower_cart_nmi() { m_owning_slot->set_nmi_line(CLEAR_LINE); }

	virtual void interface_config_complete() override;
	virtual void interface_pre_start() override;

	device_mc10cart_interface(const machine_config &mconfig, device_t &device);

	// accessors for containers
	mc10cart_slot_device &owning_slot() const { assert(m_owning_slot); return *m_owning_slot; }

private:
	mc10cart_slot_device * m_owning_slot;
};

// methods for configuring MC-10 slot devices
void mc10_cart_add_basic_devices(device_slot_interface &device);
void alice_cart_add_basic_devices(device_slot_interface &device);
void alice32_cart_add_basic_devices(device_slot_interface &device);

#endif // MAME_BUS_MC10_MC10CART_H

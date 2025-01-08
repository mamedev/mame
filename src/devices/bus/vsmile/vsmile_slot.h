// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
#ifndef MAME_BUS_VSMILE_VSMILE_SLOT_H
#define MAME_BUS_VSMILE_VSMILE_SLOT_H

#pragma once

#include "imagedev/cartrom.h"


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

enum
{
	VSMILE_STD = 0,
	VSMILE_NVRAM
};

// ======================> device_vsmile_cart_interface

class device_vsmile_cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_vsmile_cart_interface();

	// reading and writing
	virtual uint16_t bank0_r(offs_t offset) { return 0; }
	virtual uint16_t bank1_r(offs_t offset) { return 0; }
	virtual uint16_t bank2_r(offs_t offset) { return 0; }
	virtual uint16_t bank3_r(offs_t offset) { return 0; }
	virtual void bank0_w(offs_t offset, uint16_t data) { }
	virtual void bank1_w(offs_t offset, uint16_t data) { }
	virtual void bank2_w(offs_t offset, uint16_t data) { }
	virtual void bank3_w(offs_t offset, uint16_t data) { }

	// banking
	virtual void set_cs2(bool cs2) = 0;

	void rom_alloc(uint32_t size);
	void nvram_alloc(uint32_t size);
	uint16_t* get_rom_base() { return m_rom; }
	uint16_t* get_nvram_base() { return &m_nvram[0]; }
	uint32_t get_rom_size() { return m_rom_size; }
	uint32_t get_nvram_size() { return m_nvram.size() * sizeof(uint16_t); }
	void set_rom_size(uint32_t val) { m_rom_size = val; }

	void save_nvram()   { device().save_item(NAME(m_nvram)); }

protected:
	device_vsmile_cart_interface(const machine_config &mconfig, device_t &device);

	// internal state
	uint16_t *m_rom;        // this points to the cart rom region
	uint32_t m_rom_size;    // this is the actual game size, not the rom region size!
	std::vector<uint16_t> m_nvram;
};


// ======================> vsmile_cart_slot_device

class vsmile_cart_slot_device : public device_t,
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<device_vsmile_cart_interface>
{
public:
	// construction/destruction
	template <typename T>
	vsmile_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: vsmile_cart_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	vsmile_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~vsmile_cart_slot_device();

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "vsmile_cart"; }
	virtual const char *file_extensions() const noexcept override { return "u1,u3,bin"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	void save_nvram() { if (m_cart && m_cart->get_nvram_size()) m_cart->save_nvram(); }
	uint32_t get_rom_size() { return m_cart ? m_cart->get_rom_size() : 0; }

	// reading and writing
	uint16_t bank0_r(offs_t offset);
	uint16_t bank1_r(offs_t offset);
	uint16_t bank2_r(offs_t offset);
	uint16_t bank3_r(offs_t offset);
	void bank0_w(offs_t offset, uint16_t data);
	void bank1_w(offs_t offset, uint16_t data);
	void bank2_w(offs_t offset, uint16_t data);
	void bank3_w(offs_t offset, uint16_t data);

	// banking
	void set_cs2(bool cs2);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	int m_type;
	device_vsmile_cart_interface *m_cart;
};

// device type definition
DECLARE_DEVICE_TYPE(VSMILE_CART_SLOT, vsmile_cart_slot_device)

#endif // MAME_BUS_VSMILE_VSMILE_SLOT_H

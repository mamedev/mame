// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_CRVISION_SLOT_H
#define MAME_BUS_CRVISION_SLOT_H

#pragma once

#include "imagedev/cartrom.h"


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

/* PCB */
enum
{
	CRV_4K = 0,
	CRV_6K,
	CRV_8K,
	CRV_10K,
	CRV_12K,
	CRV_16K,
	CRV_18K
};


// ======================> device_crvision_cart_interface

class device_crvision_cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_crvision_cart_interface();

	// reading and writing
	virtual uint8_t read_rom40(offs_t offset) { return 0xff; }
	virtual uint8_t read_rom80(offs_t offset) { return 0xff; }

	void rom_alloc(uint32_t size);
	uint8_t* get_rom_base() { return m_rom; }
	uint32_t get_rom_size() { return m_rom_size; }

protected:
	device_crvision_cart_interface(const machine_config &mconfig, device_t &device);

	// internal state
	uint8_t *m_rom;
	uint32_t m_rom_size;
};


// ======================> crvision_cart_slot_device

class crvision_cart_slot_device : public device_t,
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<device_crvision_cart_interface>
{
public:
	// construction/destruction
	template <typename T>
	crvision_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: crvision_cart_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	crvision_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~crvision_cart_slot_device();

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override { }

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "crvision_cart"; }
	virtual const char *file_extensions() const noexcept override { return "bin,rom"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	int get_type() { return m_type; }

	// reading and writing
	uint8_t read_rom40(offs_t offset);
	uint8_t read_rom80(offs_t offset);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	int m_type;
	device_crvision_cart_interface *m_cart;
};

// device type definition
DECLARE_DEVICE_TYPE(CRVISION_CART_SLOT, crvision_cart_slot_device)

#endif // MAME_BUS_CRVISION_SLOT_H

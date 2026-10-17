// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#ifndef MAME_SNK_NGP_CART_H
#define MAME_SNK_NGP_CART_H

#pragma once

#include "bus/generic/slot.h"
#include "machine/intelfsh.h"


//**************************************************************************
//  CARTRIDGE FLASH
//**************************************************************************

// SNK Neo Geo Pocket cartridge flash: Toshiba-style command set with SNK-only
// device IDs, top boot sector layout (16K/8K/8K/32K at the end of the chip)
class ngp_flash_device : public intelfsh8_device
{
public:
	// cartridge contents seeding the flash when no save exists
	void set_cartridge(uint8_t const *data, uint32_t length);
	void deselect();

protected:
	ngp_flash_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, uint32_t clock, uint32_t size, uint16_t device_id);

	// device_nvram_interface implementation
	virtual void nvram_default() override ATTR_COLD;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;
	virtual bool nvram_can_write() const override;

private:
	uint8_t const *m_cart_data;
	uint32_t m_cart_length;
	bool m_selected;
};


// ======================> ngp_flash_4m_device

class ngp_flash_4m_device : public ngp_flash_device
{
public:
	static constexpr uint32_t SIZE = 0x8'0000;
	static constexpr uint32_t MASK = SIZE - 1;

	ngp_flash_4m_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0);
};


// ======================> ngp_flash_8m_device

class ngp_flash_8m_device : public ngp_flash_device
{
public:
	static constexpr uint32_t SIZE = 0x10'0000;
	static constexpr uint32_t MASK = SIZE - 1;

	ngp_flash_8m_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0);
};


// ======================> ngp_flash_16m_device

class ngp_flash_16m_device : public ngp_flash_device
{
public:
	static constexpr uint32_t SIZE = 0x20'0000;
	static constexpr uint32_t MASK = SIZE - 1;

	ngp_flash_16m_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0);
};


//**************************************************************************
//  CARTRIDGE SLOT
//**************************************************************************

class ngp_cart_slot_device : public generic_cartslot_device
{
public:
	template <typename T>
	ngp_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *intf, char const *exts = nullptr)
		: ngp_cart_slot_device(mconfig, tag, owner, uint32_t(0))
	{
		opts(*this);
		set_fixed(false);
		set_interface(intf);
		if (exts)
			set_extensions(exts);
	}
	ngp_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0);

	template <typename T> void set_address_space(T &&tag, int no) { m_address_space.set_tag(std::forward<T>(tag), no); }

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<ngp_flash_4m_device> m_flash4m;
	required_device<ngp_flash_8m_device> m_flash8m;
	required_device<ngp_flash_16m_device> m_flash16m;
	required_device<ngp_flash_16m_device> m_flash16m_b;

	ngp_flash_device *m_chip[2];
	optional_address_space m_address_space;

	void configure_flash(uint32_t size);
	void deselect_flash();

	template <unsigned N, uint32_t Mask> uint8_t flash_r(offs_t offset);
	template <unsigned N, uint32_t Mask> void flash_w(offs_t offset, uint8_t data);
};


// device type definitions
DECLARE_DEVICE_TYPE(NGP_FLASH_4M,  ngp_flash_4m_device)
DECLARE_DEVICE_TYPE(NGP_FLASH_8M,  ngp_flash_8m_device)
DECLARE_DEVICE_TYPE(NGP_FLASH_16M, ngp_flash_16m_device)
DECLARE_DEVICE_TYPE(NGP_CART_SLOT, ngp_cart_slot_device)

#endif // MAME_SNK_NGP_CART_H

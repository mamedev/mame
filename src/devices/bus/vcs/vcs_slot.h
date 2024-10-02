// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_VCS_VCS_SLOT_H
#define MAME_BUS_VCS_VCS_SLOT_H

#pragma once

#include "imagedev/cartrom.h"


class device_vcs_cart_interface;


class vcs_cart_slot_device : public device_t,
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<device_vcs_cart_interface>
{
public:
	// construction/destruction
	template <typename T>
	vcs_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: vcs_cart_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	vcs_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~vcs_cart_slot_device();

	template <typename T> void set_address_space(T &&tag, int no) { m_address_space.set_tag(std::forward<T>(tag), no); }

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "a2600_cart"; }
	virtual const char *file_extensions() const noexcept override { return "bin,a26"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	int get_cart_type() { return m_type; }
	static int identify_cart_type(const uint8_t *ROM, uint32_t len);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	device_vcs_cart_interface *m_cart;
	int m_type;
	optional_address_space m_address_space;

	static bool detect_snowhite(const uint8_t *cart, uint32_t len);
	static bool detect_modeDC(const uint8_t *cart, uint32_t len);
	static bool detect_modeF6(const uint8_t *cart, uint32_t len);
	static bool detect_mode3E(const uint8_t *cart, uint32_t len);
	static bool detect_modeSS(const uint8_t *cart, uint32_t len);
	static bool detect_modeFE(const uint8_t *cart, uint32_t len);
	static bool detect_modeE0(const uint8_t *cart, uint32_t len);
	static bool detect_modeCV(const uint8_t *cart, uint32_t len);
	static bool detect_modeFV(const uint8_t *cart, uint32_t len);
	static bool detect_modeJVP(const uint8_t *cart, uint32_t len);
	static bool detect_modeE7(const uint8_t *cart, uint32_t len);
	static bool detect_modeUA(const uint8_t *cart, uint32_t len);
	static bool detect_8K_mode3F(const uint8_t *cart, uint32_t len);
	static bool detect_32K_mode3F(const uint8_t *cart, uint32_t len);
	static bool detect_super_chip(const uint8_t *cart, uint32_t len);

	friend class device_vcs_cart_interface;
};


class device_vcs_cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_vcs_cart_interface();

	virtual void install_memory_handlers(address_space *space) { }

	virtual void setup_addon_ptr(uint8_t *ptr) { }

	void rom_alloc(uint32_t size, const char *tag);
	void ram_alloc(uint32_t size);
	uint8_t *get_rom_base() { return m_rom; }
	uint8_t *get_ram_base() { return &m_ram[0]; }
	uint32_t get_rom_size() { return m_rom_size; }
	uint32_t get_ram_size() { return m_ram.size(); }

protected:
	device_vcs_cart_interface(const machine_config &mconfig, device_t &device);

	// internal state
	uint8_t *m_rom;
	uint32_t m_rom_size;
	std::vector<uint8_t> m_ram;
};


// device type definition
DECLARE_DEVICE_TYPE(VCS_CART_SLOT, vcs_cart_slot_device)

#endif // MAME_BUS_VCS_VCS_SLOT_H

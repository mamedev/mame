// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_VCS_VCS_SLOT_H
#define MAME_BUS_VCS_VCS_SLOT_H

#pragma once

#include "imagedev/cartrom.h"


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

#define A26SLOT_ROM_REGION_TAG ":cart:rom"

/* PCB */
enum
{
	A26_2K = 0,
	A26_4K,
	A26_F4,
	A26_F6,
	A26_F8,
	A26_F8SW,
	A26_FA,
	A26_FE,
	A26_3E,     // to test
	A26_3F,
	A26_E0,
	A26_E7,
	A26_UA,
	A26_DC,
	A26_CV,
	A26_FV,
	A26_JVP,    // to test
	A26_32IN1,
	A26_8IN1,
	A26_4IN1,
	A26_DPC,
	A26_SS,
	A26_CM,
	A26_X07,
	A26_HARMONY,
};


// ======================> device_vcs_cart_interface

class device_vcs_cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_vcs_cart_interface();

	// reading from ROM
	virtual uint8_t read_rom(offs_t offset) { return 0xff; }
	// writing to RAM chips (sometimes it is in a different range than write_bank!)
	virtual void write_ram(offs_t offset, uint8_t data) { }

	// read/write to bankswitch address
	virtual uint8_t read_bank(address_space &space, offs_t offset) { return 0xff; }
	virtual void write_bank(address_space &space, offs_t offset, uint8_t data) { }

	virtual void setup_addon_ptr(uint8_t *ptr) { }

	void rom_alloc(uint32_t size, const char *tag);
	void ram_alloc(uint32_t size);
	uint8_t* get_rom_base() { return m_rom; }
	uint8_t*  get_ram_base() { return &m_ram[0]; }
	uint32_t  get_rom_size() { return m_rom_size; }
	uint32_t  get_ram_size() { return m_ram.size(); }

protected:
	device_vcs_cart_interface(const machine_config &mconfig, device_t &device);

	// internal state
	uint8_t *m_rom;
	uint32_t m_rom_size;
	std::vector<uint8_t> m_ram;
};


// ======================> vcs_cart_slot_device

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

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "a2600_cart"; }
	virtual const char *file_extensions() const noexcept override { return "bin,a26"; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	int get_cart_type() { return m_type; }
	static int identify_cart_type(const uint8_t *ROM, uint32_t len);

	// reading and writing
	virtual uint8_t read_rom(offs_t offset);
	virtual uint8_t read_bank(address_space &space, offs_t offset);
	virtual void write_bank(address_space &space, offs_t offset, uint8_t data);
	virtual void write_ram(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	device_vcs_cart_interface*       m_cart;
	int m_type;

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
};


// device type definition
DECLARE_DEVICE_TYPE(VCS_CART_SLOT, vcs_cart_slot_device)

#endif // MAME_BUS_VCS_VCS_SLOT_H

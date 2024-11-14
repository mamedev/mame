// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_APF_SLOT_H
#define MAME_BUS_APF_SLOT_H

#include "imagedev/cartrom.h"

/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/


/* PCB */
enum
{
	APF_STD = 0,
	APF_BASIC,
	APF_SPACEDST
};


// ======================> device_apf_cart_interface

class device_apf_cart_interface : public device_interface
{
public:
	// construction/destruction
	device_apf_cart_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_apf_cart_interface();

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) { return 0xff; }
	virtual uint8_t extra_rom(offs_t offset) { return 0xff; }
	virtual uint8_t read_ram(offs_t offset) { return 0xff; }
	virtual void write_ram(offs_t offset, uint8_t data) {}

	void rom_alloc(uint32_t size);
	void ram_alloc(uint32_t size);
	uint8_t* get_rom_base() { return m_rom; }
	uint8_t* get_ram_base() { return &m_ram[0]; }
	uint32_t get_rom_size() { return m_rom_size; }
	uint32_t get_ram_size() { return m_ram.size(); }

	void save_ram() { device().save_item(NAME(m_ram)); }

protected:
	// internal state
	uint8_t *m_rom;
	uint32_t m_rom_size;
	std::vector<uint8_t> m_ram;
};


// ======================> apf_cart_slot_device

class apf_cart_slot_device : public device_t,
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<device_apf_cart_interface>
{
public:
	// construction/destruction
	template <typename T>
	apf_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: apf_cart_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	apf_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~apf_cart_slot_device();

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override {}

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "apfm1000_cart"; }
	virtual const char *file_extensions() const noexcept override { return "bin"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	int get_type() { return m_type; }

	void save_ram() { if (m_cart && m_cart->get_ram_size()) m_cart->save_ram(); }

	// reading and writing
	uint8_t read_rom(offs_t offset);
	uint8_t extra_rom(offs_t offset);
	uint8_t read_ram(offs_t offset);
	void write_ram(offs_t offset, uint8_t data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	int m_type;
	device_apf_cart_interface *m_cart;
};


// device type definition
DECLARE_DEVICE_TYPE(APF_CART_SLOT, apf_cart_slot_device)

#endif // MAME_BUS_APF_SLOT_H

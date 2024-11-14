// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_VC4000_SLOT_H
#define MAME_BUS_VC4000_SLOT_H

#include "imagedev/cartrom.h"


/* PCB */
enum
{
	VC4000_STD = 0,
	VC4000_ROM4K,
	VC4000_RAM1K,
	VC4000_CHESS2
};


// ======================> device_vc4000_cart_interface

class device_vc4000_cart_interface : public device_interface
{
public:
	// construction/destruction
	virtual ~device_vc4000_cart_interface();

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) { return 0xff; }
	virtual uint8_t extra_rom(offs_t offset) { return 0xff; }
	virtual uint8_t read_ram(offs_t offset) { return 0xff; }
	virtual void write_ram(offs_t offset, uint8_t data) { }

	void rom_alloc(uint32_t size);
	void ram_alloc(uint32_t size);
	uint8_t* get_rom_base() { return m_rom; }
	uint8_t* get_ram_base() { return &m_ram[0]; }
	uint32_t get_rom_size() { return m_rom_size; }
	uint32_t get_ram_size() { return m_ram.size(); }

	void save_ram() { device().save_item(NAME(m_ram)); }

protected:
	device_vc4000_cart_interface(const machine_config &mconfig, device_t &device);

	// internal state
	uint8_t *m_rom;
	uint32_t m_rom_size;
	std::vector<uint8_t> m_ram;
};


// ======================> vc4000_cart_slot_device

class vc4000_cart_slot_device : public device_t,
								public device_cartrom_image_interface,
								public device_single_card_slot_interface<device_vc4000_cart_interface>
{
public:
	// construction/destruction
	template <typename T>
	vc4000_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, char const *dflt)
		: vc4000_cart_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	vc4000_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~vc4000_cart_slot_device();

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual void call_unload() override { }

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "vc4000_cart"; }
	virtual const char *file_extensions() const noexcept override { return "bin,rom"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	int get_type() { return m_type; }

	void save_ram() { if (m_cart && m_cart->get_ram_size()) m_cart->save_ram(); }

	// reading and writing
	virtual uint8_t read_rom(offs_t offset);
	virtual uint8_t extra_rom(offs_t offset);
	virtual uint8_t read_ram(offs_t offset);
	virtual void write_ram(offs_t offset, uint8_t data);

protected:
	vc4000_cart_slot_device(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner,
			uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	int m_type;
	device_vc4000_cart_interface *m_cart;
};

class h21_cart_slot_device : public vc4000_cart_slot_device
{
public:
	// construction/destruction
	template <typename T>
	h21_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, char const *dflt)
		: h21_cart_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	h21_cart_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~h21_cart_slot_device();

	virtual const char *image_interface() const noexcept override { return "h21_cart"; }
};

// device type definition
DECLARE_DEVICE_TYPE(VC4000_CART_SLOT, vc4000_cart_slot_device)
DECLARE_DEVICE_TYPE(H21_CART_SLOT,    h21_cart_slot_device)

#endif // MAME_BUS_VC4000_SLOT_H

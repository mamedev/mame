// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_GENERIC_SLOT_H
#define MAME_BUS_GENERIC_SLOT_H

#pragma once

#include "softlist_dev.h"


/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/


// ======================> device_generic_cart_interface

class device_generic_cart_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	virtual ~device_generic_cart_interface();

	// reading and writing
	virtual uint8_t read_rom(offs_t offset) { return 0xff; }
	virtual uint16_t read16_rom(offs_t offset, uint16_t mem_mask) { return 0xffff; }
	virtual uint32_t read32_rom(offs_t offset, uint32_t mem_mask) { return 0xffffffff; }

	virtual uint8_t read_ram(offs_t offset) { return 0xff; }
	virtual void write_ram(offs_t offset, uint8_t data) {}

	virtual void rom_alloc(size_t size, int width, endianness_t end, const char *tag);
	virtual void ram_alloc(uint32_t size);

	uint8_t* get_rom_base()  { return m_rom; }
	uint32_t get_rom_size() { return m_rom_size; }

	uint8_t* get_region_base()  { if (m_region.found()) return m_region->base(); return nullptr; }
	uint32_t get_region_size() { if (m_region.found()) return m_region->bytes(); return 0; }

	uint8_t* get_ram_base() { return &m_ram[0]; }
	uint32_t get_ram_size() { return m_ram.size(); }

	void save_ram() { device().save_item(NAME(m_ram)); }

protected:
	device_generic_cart_interface(const machine_config &mconfig, device_t &device);

	// internal state
	uint8_t  *m_rom;
	uint32_t  m_rom_size;
	std::vector<uint8_t> m_ram;

	// this replaces m_rom for non-user configurable carts!
	optional_memory_region  m_region;
};


enum
{
	GENERIC_ROM8_WIDTH = 1,
	GENERIC_ROM16_WIDTH = 2,
	GENERIC_ROM32_WIDTH = 4
};

#define GENERIC_ROM_REGION_TAG ":cart:rom"



// ======================> generic_slot_device

class generic_slot_device : public device_t,
								public device_image_interface,
								public device_slot_interface
{
public:
	// construction/destruction
	generic_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	virtual ~generic_slot_device();

	template <typename... T> void set_device_load(T &&... args) { m_device_image_load = load_delegate(std::forward<T>(args)...); }
	template <typename... T> void set_device_unload(T &&... args) { m_device_image_unload = unload_delegate(std::forward<T>(args)...); }

	void set_interface(const char * interface) { m_interface = interface; }
	void set_default_card(const char * def) { m_default_card = def; }
	void set_extensions(const char * exts) { m_extensions = exts; }
	void set_must_be_loaded(bool mandatory) { m_must_be_loaded = mandatory; }
	void set_width(int width) { m_width = width; }
	void set_endian(endianness_t end) { m_endianness = end; }

	// device-level overrides
	virtual void device_start() override;

	// image-level overrides
	virtual image_init_result call_load() override;
	virtual void call_unload() override;
	virtual const software_list_loader &get_software_list_loader() const override { return rom_software_list_loader::instance(); }

	uint32_t common_get_size(const char *region);
	void common_load_rom(uint8_t *ROM, uint32_t len, const char *region);

	virtual iodevice_t image_type() const override { return IO_CARTSLOT; }
	virtual bool is_readable()  const override { return true; }
	virtual bool is_writeable() const override { return false; }
	virtual bool is_creatable() const override { return false; }
	virtual bool must_be_loaded() const override { return m_must_be_loaded; }
	virtual bool is_reset_on_load() const override { return true; }
	virtual const char *image_interface() const override { return m_interface; }
	virtual const char *file_extensions() const override { return m_extensions; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	// reading and writing
	virtual uint8_t read_rom(offs_t offset);
	virtual uint16_t read16_rom(offs_t offset, uint16_t mem_mask = 0xffff);
	virtual uint32_t read32_rom(offs_t offset, uint32_t mem_mask = 0xffffffff);

	virtual uint8_t read_ram(offs_t offset);
	virtual void write_ram(offs_t offset, uint8_t data);

	virtual void rom_alloc(size_t size, int width, endianness_t end) { if (m_cart) m_cart->rom_alloc(size, width, end, tag()); }
	virtual void ram_alloc(uint32_t size)  { if (m_cart) m_cart->ram_alloc(size); }

	uint8_t* get_rom_base()
	{
		if (m_cart)
		{
			if (!user_loadable())
				return m_cart->get_region_base();
			else
				return m_cart->get_rom_base();
		}
		return nullptr;
	}
	uint32_t get_rom_size()
	{
		if (m_cart)
		{
			if (!user_loadable())
				return m_cart->get_region_size();
			else
				return m_cart->get_rom_size();
		}
		return 0;
	}
	uint8_t* get_ram_base() { if (m_cart) return m_cart->get_ram_base(); return nullptr; }

	void save_ram() { if (m_cart && m_cart->get_ram_size()) m_cart->save_ram(); }

protected:
	const char *m_interface;
	const char *m_default_card;
	const char *m_extensions;
	bool        m_must_be_loaded;
	int         m_width;
	endianness_t m_endianness;
	device_generic_cart_interface  *m_cart;
	load_delegate	m_device_image_load;
	unload_delegate	m_device_image_unload;
};

class generic_socket_device : public generic_slot_device
{
public:
	template <typename T>
	generic_socket_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *intf, const char *exts = nullptr)
		: generic_socket_device(mconfig, tag, owner, (uint32_t)0)
	{
		opts(*this);
		set_fixed(false);
		set_interface(intf);
		if (exts)
			set_extensions(exts);
	}

	generic_socket_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual iodevice_t image_type() const override { return IO_ROM; }
};

class generic_cartslot_device : public generic_slot_device
{
public:
	template <typename T>
	generic_cartslot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *intf, const char *exts = nullptr)
		: generic_cartslot_device(mconfig, tag, owner, (uint32_t)0)
	{
		opts(*this);
		set_fixed(false);
		set_interface(intf);
		if (exts)
			set_extensions(exts);
	}

	generic_cartslot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual iodevice_t image_type() const override { return IO_CARTSLOT; }
};


// device type definition
DECLARE_DEVICE_TYPE(GENERIC_SOCKET, generic_socket_device)
DECLARE_DEVICE_TYPE(GENERIC_CARTSLOT, generic_cartslot_device)


/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#endif // MAME_BUS_GENERIC_SLOT_H

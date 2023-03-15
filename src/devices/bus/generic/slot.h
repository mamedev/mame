// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_GENERIC_SLOT_H
#define MAME_BUS_GENERIC_SLOT_H

#pragma once

#include "imagedev/cartrom.h"

#include <cassert>


//**************************************************************************
//  MACROS
//**************************************************************************

#define DEVICE_IMAGE_LOAD_MEMBER(_name)             image_init_result _name(device_image_interface &image)
#define DECLARE_DEVICE_IMAGE_LOAD_MEMBER(_name)     DEVICE_IMAGE_LOAD_MEMBER(_name)

#define DEVICE_IMAGE_UNLOAD_MEMBER(_name)           void _name(device_image_interface &image)
#define DECLARE_DEVICE_IMAGE_UNLOAD_MEMBER(_name)   DEVICE_IMAGE_UNLOAD_MEMBER(_name)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_generic_cart_interface : public device_interface
{
public:
	// TODO: find a better home for this helper
	template <unsigned Shift, typename T>
	static void install_non_power_of_two(
			offs_t length,
			offs_t decode_limit,
			offs_t decode_mask,
			offs_t decode_offset,
			offs_t base,
			T &&install)
	{
		assert(!(decode_limit & base));
		for (offs_t remain = length, start = 0U; remain && (decode_limit >= start); )
		{
			unsigned const msb(31 - count_leading_zeros_32(u32(remain)));
			offs_t const chunk(offs_t(1) << msb);
			offs_t range((chunk - 1) & decode_limit);
			offs_t mirror(decode_limit & ~decode_mask & ~range);

			// TODO: deal with small offsets - probably requires breaking into smaller chunks with mirror set
			if (decode_offset & range)
				throw emu_fatalerror("Can't deal with offset/size combination\n");

			range = (range << Shift) | ((offs_t(1) << Shift) - 1);
			mirror <<= Shift;
			offs_t const begin(start << Shift);
			offs_t const end(begin | range);
			offs_t const src(start | ((chunk - 1) & decode_offset));

			install(base | begin, base | end, mirror, src);

			remain ^= chunk;
			start ^= chunk;
		}
	}

	// TODO: find a better home for this helper
	template <typename T, typename U>
	static T map_non_power_of_two(T count, U &&map)
	{
		if (T(0) >= count)
			return T(0);

		T const max(count - 1);
		T mask(max);
		for (unsigned i = 1; (sizeof(T) * 8) > i; i <<= 1)
			mask = T(std::make_unsigned_t<T>(mask) | (std::make_unsigned_t<T>(mask) >> i));
		int bits(0);
		while (BIT(mask, bits))
			++bits;

		for (T entry = T(0); mask >= entry; ++entry)
		{
			T mapped(entry);
			int b(bits - 1);
			while (max < mapped)
			{
				while (BIT(max, b))
					--b;
				assert(0 <= b);
				mapped = T(std::make_unsigned_t<T>(mapped) & ~(std::make_unsigned_t<T>(1) << b--));
			}
			map(entry, mapped);
		}

		return mask;
	}

	// construction/destruction
	virtual ~device_generic_cart_interface();

	// reading and writing
	virtual u8 read_rom(offs_t offset);
	virtual u16 read16_rom(offs_t offset, u16 mem_mask);
	virtual u32 read32_rom(offs_t offset, u32 mem_mask);

	virtual u8 read_ram(offs_t offset);
	virtual void write_ram(offs_t offset, u8 data);

	virtual void rom_alloc(u32 size, int width, endianness_t end, char const *tag);
	virtual void ram_alloc(u32 size);

	u8 *get_rom_base()  { return m_rom; }
	u32 get_rom_size() { return m_rom_size; }

	u8 *get_region_base()  { return m_region.found() ? m_region->base() : nullptr; }
	u32 get_region_size() { return m_region.found() ? m_region->bytes() : 0U; }

	u8 *get_ram_base() { return &m_ram[0]; }
	u32 get_ram_size() { return m_ram.size(); }

	void save_ram() { device().save_item(NAME(m_ram)); }

protected:
	device_generic_cart_interface(machine_config const &mconfig, device_t &device);

	// this replaces m_rom for non-user configurable carts!
	optional_memory_region  m_region;

	// internal state
	std::vector<u8> m_ram;
	u8 *m_rom;
	u32 m_rom_size;
};


enum
{
	GENERIC_ROM8_WIDTH = 1,
	GENERIC_ROM16_WIDTH = 2,
	GENERIC_ROM32_WIDTH = 4
};

#define GENERIC_ROM_REGION_TAG ":cart:rom"



class generic_slot_device : public device_t,
								public device_rom_image_interface,
								public device_single_card_slot_interface<device_generic_cart_interface>
{
public:
	typedef device_delegate<image_init_result (device_image_interface &)> load_delegate;
	typedef device_delegate<void (device_image_interface &)> unload_delegate;

	virtual ~generic_slot_device();

	template <typename... T> void set_device_load(T &&... args) { m_device_image_load.set(std::forward<T>(args)...); }
	template <typename... T> void set_device_unload(T &&... args) { m_device_image_unload.set(std::forward<T>(args)...); }

	void set_interface(char const *interface) { m_interface = interface; }
	void set_default_card(char const *def) { m_default_card = def; }
	void set_extensions(char const *exts) { m_extensions = exts; }
	void set_width(int width) { m_width = width; }
	void set_endian(endianness_t end) { m_endianness = end; }

	// device_image_interface implementation
	virtual image_init_result call_load() override;
	virtual void call_unload() override;
	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual char const *image_interface() const noexcept override { return m_interface; }
	virtual char const *file_extensions() const noexcept override { return m_extensions; }

	// slot interface overrides
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	u32 common_get_size(char const *region);
	void common_load_rom(u8 *ROM, u32 len, char const *region);

	// reading and writing
	virtual u8 read_rom(offs_t offset);
	virtual u16 read16_rom(offs_t offset, u16 mem_mask = 0xffff);
	virtual u32 read32_rom(offs_t offset, u32 mem_mask = 0xffffffff);

	virtual u8 read_ram(offs_t offset);
	virtual void write_ram(offs_t offset, u8 data);

	virtual void rom_alloc(u32 size, int width, endianness_t end) { if (m_cart) m_cart->rom_alloc(size, width, end, tag()); }
	virtual void ram_alloc(u32 size) { if (m_cart) m_cart->ram_alloc(size); }

	u8* get_rom_base()
	{
		if (!m_cart)
			return nullptr;
		else if (!user_loadable())
			return m_cart->get_region_base();
		else
			return m_cart->get_rom_base();
	}
	u32 get_rom_size()
	{
		if (!m_cart)
			return 0U;
		else if (!user_loadable())
			return m_cart->get_region_size();
		else
			return m_cart->get_rom_size();
	}
	u8 *get_ram_base() { return m_cart ? m_cart->get_ram_base() : nullptr; }

	void save_ram() { if (m_cart && m_cart->get_ram_size()) m_cart->save_ram(); }

protected:
	generic_slot_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	char const *m_interface;
	char const *m_default_card;
	char const *m_extensions;
	int m_width;
	endianness_t m_endianness;
	device_generic_cart_interface *m_cart;
	load_delegate m_device_image_load;
	unload_delegate m_device_image_unload;
};

class generic_socket_device : public generic_slot_device
{
public:
	template <typename T>
	generic_socket_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *intf, char const *exts = nullptr)
		: generic_socket_device(mconfig, tag, owner, u32(0))
	{
		opts(*this);
		set_fixed(false);
		set_interface(intf);
		if (exts)
			set_extensions(exts);
	}

	generic_socket_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	virtual const char *image_type_name() const noexcept override { return "romimage"; }
	virtual const char *image_brief_type_name() const noexcept override { return "rom"; }
};

class generic_cartslot_device : public generic_slot_device
{
public:
	template <typename T>
	generic_cartslot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *intf, char const *exts = nullptr)
		: generic_cartslot_device(mconfig, tag, owner, u32(0))
	{
		opts(*this);
		set_fixed(false);
		set_interface(intf);
		if (exts)
			set_extensions(exts);
	}

	generic_cartslot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	virtual const char *image_type_name() const noexcept override { return "cartridge"; }
	virtual const char *image_brief_type_name() const noexcept override { return "cart"; }
};


// device type definition
DECLARE_DEVICE_TYPE(GENERIC_SOCKET, generic_socket_device)
DECLARE_DEVICE_TYPE(GENERIC_CARTSLOT, generic_cartslot_device)

#endif // MAME_BUS_GENERIC_SLOT_H

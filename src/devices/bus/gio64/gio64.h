// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

  gio64.h - SGI GIO64 slot bus and GIO64 device emulation

***************************************************************************/

#ifndef MAME_BUS_GIO64_GIO64_H
#define MAME_BUS_GIO64_GIO64_H

#pragma once

class gio64_device;

class gio64_slot_device : public device_t, public device_slot_interface
{
public:
	enum slot_type_t : uint32_t
	{
		GIO64_SLOT_GFX  = 0,
		GIO64_SLOT_EXP0 = 1,
		GIO64_SLOT_EXP1 = 2,
	};

	// construction/destruction
	template <typename T, typename U>
	gio64_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&gio64_tag, slot_type_t slot_type, U &&opts, const char *dflt)
		: gio64_slot_device(mconfig, tag, owner, (uint32_t)0, slot_type)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
		m_gio64.set_tag(std::forward<T>(gio64_tag));
	}
	gio64_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, slot_type_t slot_type = GIO64_SLOT_EXP0);

protected:
	// device_t implementation
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// configuration
	required_device<gio64_device> m_gio64;
	slot_type_t const m_slot_type;
};

DECLARE_DEVICE_TYPE(GIO64_SLOT, gio64_slot_device)


// class representing interface-specific live GIO64 card
class device_gio64_card_interface : public device_interface
{
	friend class gio64_device;
public:
	// construction/destruction
	virtual ~device_gio64_card_interface();

	// inline configuration
	void set_gio64(gio64_device *gio64, gio64_slot_device::slot_type_t slot_type);

	virtual void mem_map(address_map &map) = 0;

protected:
	device_gio64_card_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;

	gio64_device *m_gio64;
};


class gio64_device : public device_t,
	public device_memory_interface
{
	friend class device_gio64_card_interface;
public:
	// construction/destruction
	template <typename T>
	gio64_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu_tag)
		: gio64_device(mconfig, tag, owner, (uint32_t)0)
	{
	}

	gio64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	template <int N> auto interrupt_cb() { return m_interrupt_cb[N].bind(); }

	virtual space_config_vector memory_space_config() const override;

	const address_space_config m_space_config;

	device_gio64_card_interface *get_gio64_card(int slot);

	template<typename T> void install_card(gio64_slot_device::slot_type_t slot_type, T &device, void (T::*map)(class address_map &map))
	{
		m_device_list[slot_type] = &device;

		switch (slot_type)
		{
		case gio64_slot_device::GIO64_SLOT_GFX:  space(0).install_device(0x000000, 0x3fffff, device, map); break;
		case gio64_slot_device::GIO64_SLOT_EXP0: space(0).install_device(0x400000, 0x5fffff, device, map); break;
		case gio64_slot_device::GIO64_SLOT_EXP1: space(0).install_device(0x600000, 0x9fffff, device, map); break;
		}
	}

	template <int N> void interrupt(int state) { m_interrupt_cb[N](state); }

	u64 read(offs_t offset, u64 mem_mask);
	void write(offs_t offset, u64 data, u64 mem_mask);

protected:
	gio64_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// internal state
	device_gio64_card_interface *m_device_list[3];

private:
	devcb_write_line::array<3> m_interrupt_cb;
};

DECLARE_DEVICE_TYPE(GIO64, gio64_device)


void gio64_cards(device_slot_interface &device);

#endif // MAME_BUS_GIO_GIO_H

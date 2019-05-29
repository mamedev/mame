// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

  gio64.h - SGI GIO64 slot bus and GIO64 device emulation

***************************************************************************/

#ifndef MAME_BUS_GIO64_GIO64_H
#define MAME_BUS_GIO64_GIO64_H

#pragma once

#include "cpu/mips/r4000.h"

class gio64_device;

class gio64_slot_device : public device_t, public device_slot_interface
{
public:
	enum slot_type_t : uint32_t
	{
		GIO64_SLOT_GFX,
		GIO64_SLOT_EXP0,
		GIO64_SLOT_EXP1,

		GIO64_SLOT_COUNT
	};

	// construction/destruction
	template <typename T, typename U>
	gio64_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&gio64_tag, slot_type_t slot_type, U &&opts, const char *dflt)
		: gio64_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
		m_gio64.set_tag(std::forward<T>(gio64_tag));
		m_slot_type = slot_type;
	}
	gio64_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	gio64_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_resolve_objects() override;
	virtual void device_start() override;

	// configuration
	required_device<gio64_device> m_gio64;
	slot_type_t m_slot_type;

	DECLARE_READ32_MEMBER(timeout_r);
	DECLARE_WRITE32_MEMBER(timeout_w);
};

DECLARE_DEVICE_TYPE(GIO64_SLOT, gio64_slot_device)


// class representing interface-specific live GIO64 card
class device_gio64_card_interface : public device_slot_card_interface
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

	virtual void interface_validity_check(validity_checker &valid) const override;
	virtual void interface_pre_start() override;
	virtual void interface_post_start() override;
	virtual void install_device() = 0;

	gio64_device &gio64() { assert(m_gio64); return *m_gio64; }

	gio64_device *m_gio64;
	const char *m_gio64_slottag;
	gio64_slot_device::slot_type_t m_slot_type;
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
		set_cpu_tag(std::forward<T>(cpu_tag));
	}

	gio64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	template <typename T> void set_cpu_tag(T &&tag) { m_maincpu.set_tag(std::forward<T>(tag)); }
	template <int N> auto interrupt_cb() { return m_interrupt_cb[N].bind(); }

	virtual space_config_vector memory_space_config() const override;

	const address_space_config m_space_config;

	void add_gio64_card(gio64_slot_device::slot_type_t slot_type, device_gio64_card_interface *card);
	device_gio64_card_interface *get_gio64_card(int slot);

	template<typename T> void install_graphics(T &device, void (T::*map)(class address_map &map), uint64_t unitmask = ~u64(0))
	{
		m_space->install_device(0x000000, 0x3fffff, device, map, unitmask);
	}

	template<typename T> void install_expansion(int index, T &device, void (T::*map)(class address_map &map), uint64_t unitmask = ~u64(0))
	{
		if (index == 0)
			m_space->install_device(0x400000, 0x5fffff, device, map, unitmask);
		else if (index == 1)
			m_space->install_device(0x600000, 0x9fffff, device, map, unitmask);
		else
			fatalerror("Invalid SGI GIO64 expansion slot index: %d\n", index);
	}

	template <int N> DECLARE_WRITE_LINE_MEMBER(interrupt) { m_interrupt_cb[N](state); }

	DECLARE_READ64_MEMBER(read);
	DECLARE_WRITE64_MEMBER(write);

protected:
	gio64_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;

	// internal state
	required_device<r4000_base_device> m_maincpu;
	address_space *m_space;

	device_gio64_card_interface *m_device_list[3];

private:
	devcb_write_line m_interrupt_cb[3];

	DECLARE_READ64_MEMBER(no_gfx_r);
	DECLARE_READ64_MEMBER(no_exp0_r);
	DECLARE_READ64_MEMBER(no_exp1_r);
	DECLARE_WRITE64_MEMBER(no_gfx_w);
	DECLARE_WRITE64_MEMBER(no_exp0_w);
	DECLARE_WRITE64_MEMBER(no_exp1_w);
};

DECLARE_DEVICE_TYPE(GIO64, gio64_device)


void gio64_cards(device_slot_interface &device);

#endif  // MAME_BUS_GIO_GIO_H

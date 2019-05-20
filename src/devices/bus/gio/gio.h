// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

  gio.h - SGI GIO slot bus and GIO device emulation

***************************************************************************/

#ifndef MAME_BUS_GIO_GIO_H
#define MAME_BUS_GIO_GIO_H

#pragma once

#include "cpu/mips/r4000.h"
#include "machine/hpc3.h"

class gio_device;

class gio_slot_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	template <typename T, typename U>
	gio_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&gio_tag, U &&opts, const char *dflt)
		: gio_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
		m_gio.set_tag(std::forward<T>(gio_tag));
	}
	gio_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	gio_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_resolve_objects() override;
	virtual void device_start() override;

	// configuration
	required_device<gio_device> m_gio;

	DECLARE_READ32_MEMBER(timeout_r);
	DECLARE_WRITE32_MEMBER(timeout_w);
};

DECLARE_DEVICE_TYPE(GIO_SLOT, gio_slot_device)


// class representing interface-specific live GIO card
class device_gio_card_interface : public device_slot_card_interface
{
	friend class gio_device;
public:
	// construction/destruction
	virtual ~device_gio_card_interface();

	// inline configuration
	void set_gio(gio_device *gio, const char *slottag);

	virtual void mem_map(address_map &map) = 0;

protected:
	device_gio_card_interface(const machine_config &mconfig, device_t &device);

	enum gio_slot_type_t : uint32_t
	{
		GIO_SLOT_GFX,
		GIO_SLOT_EXP0,
		GIO_SLOT_EXP1,

		GIO_SLOT_COUNT
	};

	virtual void interface_validity_check(validity_checker &valid) const override;
	virtual void interface_pre_start() override;
	virtual void install_device() = 0;

	gio_device &gio() { assert(m_gio); return *m_gio; }

	gio_device *m_gio;
	const char *m_gio_slottag;
	gio_slot_type_t m_slot_type;
};


class gio_device : public device_t,
	public device_memory_interface
{
	friend class device_gio_card_interface;
public:
	// construction/destruction
	template <typename T, typename U>
	gio_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu_tag, U &&hpc3_tag)
		: gio_device(mconfig, tag, owner, (uint32_t)0)
	{
		set_cpu_tag(std::forward<T>(cpu_tag));
		set_hpc3_tag(std::forward<U>(hpc3_tag));
	}

	gio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	template <typename T> void set_cpu_tag(T &&tag) { m_maincpu.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_hpc3_tag(T &&tag) { m_hpc3.set_tag(std::forward<T>(tag)); }

	virtual space_config_vector memory_space_config() const override;

	const address_space_config m_space_config;

	void add_gio_card(device_gio_card_interface::gio_slot_type_t slot_type, device_gio_card_interface *card);
	device_gio_card_interface *get_gio_card(int slot);

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
			fatalerror("Invalid SGIO GIO expansion slot index: %d\n", index);
	}

	hpc3_base_device* get_hpc3() { return m_hpc3.target(); }

	DECLARE_READ64_MEMBER(read);
	DECLARE_WRITE64_MEMBER(write);

protected:
	gio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;

	// internal state
	required_device<r4000_base_device> m_maincpu;
	required_device<hpc3_base_device> m_hpc3;
	address_space *m_space;

	device_gio_card_interface *m_device_list[3];

private:
	DECLARE_READ64_MEMBER(no_gfx_r);
	DECLARE_READ64_MEMBER(no_exp0_r);
	DECLARE_READ64_MEMBER(no_exp1_r);
	DECLARE_WRITE64_MEMBER(no_gfx_w);
	DECLARE_WRITE64_MEMBER(no_exp0_w);
	DECLARE_WRITE64_MEMBER(no_exp1_w);
};

DECLARE_DEVICE_TYPE(GIO, gio_device)


void gio_cards(device_slot_interface &device);

#endif  // MAME_BUS_GIO_GIO_H

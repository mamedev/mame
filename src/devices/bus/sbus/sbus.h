// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

  sbus.h - Sun SBus slot bus and card emulation

***************************************************************************/

#ifndef MAME_BUS_SBUS_SBUS_H
#define MAME_BUS_SBUS_SBUS_H

#pragma once

#include "cpu/sparc/sparc.h"
#include "machine/bankdev.h"

class device_sbus_card_interface;
class sbus_device;


class sbus_slot_device : public device_t, public device_single_card_slot_interface<device_sbus_card_interface>
{
public:
	// construction/destruction
	template <typename T, typename U>
	sbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&sbus_tag, int slot, U &&opts, const char *dflt, bool fixed = false)
		: sbus_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(fixed);
		m_sbus.set_tag(std::forward<T>(sbus_tag));
		m_slot = slot;
	}
	sbus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	sbus_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// configuration
	required_device<sbus_device> m_sbus;
	int m_slot;
};

DECLARE_DEVICE_TYPE(SBUS_SLOT, sbus_slot_device)


class sbus_device : public device_t,
	public device_memory_interface
{
	friend class device_sbus_card_interface;
public:
	// construction/destruction
	template <typename T, typename U>
	sbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, U &&space_tag, int space_num)
		: sbus_device(mconfig, tag, owner, clock)
	{
		set_cpu(std::forward<T>(cpu_tag));
		set_type1space(std::forward<U>(space_tag), space_num);
	}

	sbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	template <typename T> void set_cpu(T &&tag) { m_maincpu.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_type1space(T &&tag, int num) { m_type1space.set_tag(std::forward<T>(tag), num); }
	template <unsigned Line> auto irq() { return m_irq_cb[Line].bind(); }
	auto buserr() { return m_buserr.bind(); }

	virtual space_config_vector memory_space_config() const override;

	const address_space_config m_space_config;

	void add_sbus_card(int slot, device_sbus_card_interface *card);
	device_sbus_card_interface *get_sbus_card(int slot);

	void set_irq_line(int state, int line);

	template<typename T> void install_device(offs_t addrstart, offs_t addrend, T &device, void (T::*map)(class address_map &map), uint64_t unitmask = ~u64(0))
	{
		m_space->install_device(addrstart, addrend, device, map, unitmask);
	}

	uint32_t read(offs_t offset, uint32_t mem_mask = ~0);
	void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

protected:
	sbus_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// internal state
	required_device<sparc_base_device> m_maincpu;
	required_address_space m_type1space;
	address_space *m_space;

	devcb_write_line::array<7> m_irq_cb;
	devcb_write32 m_buserr;

	device_sbus_card_interface *m_device_list[3];

private:
	void slot1_timeout_map(address_map &map) ATTR_COLD;
	void slot2_timeout_map(address_map &map) ATTR_COLD;
	void slot3_timeout_map(address_map &map) ATTR_COLD;

	template <unsigned Slot> uint32_t slot_timeout_r();
	template <unsigned Slot> void slot_timeout_w(uint32_t data);
};

DECLARE_DEVICE_TYPE(SBUS, sbus_device)


// class representing interface-specific live sbus card
class device_sbus_card_interface : public device_interface
{
	friend class sbus_device;
public:
	// construction/destruction
	virtual ~device_sbus_card_interface();

	// inline configuration
	void set_sbus(sbus_device *sbus, int slot);
	template <typename T> void set_onboard(T &&sbus, int slot) { m_sbus_finder.set_tag(std::forward<T>(sbus)); m_slot = slot; }

	virtual void mem_map(address_map &map) = 0;

protected:
	void raise_irq(int line) { m_sbus->set_irq_line(ASSERT_LINE, line); }
	void lower_irq(int line) { m_sbus->set_irq_line(CLEAR_LINE, line); }

	device_sbus_card_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_validity_check(validity_checker &valid) const override;
	virtual void interface_pre_start() override;
	virtual void interface_post_start() override;
	virtual void install_device() = 0;

	sbus_device &sbus() { assert(m_sbus); return *m_sbus; }

	optional_device<sbus_device> m_sbus_finder;
	sbus_device *m_sbus;
	const char *m_sbus_slottag;
	int m_slot;
	uint32_t m_base;
};

void sbus_cards(device_slot_interface &device);

#endif  // MAME_BUS_SBUS_SBUS_H

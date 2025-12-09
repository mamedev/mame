// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  pwrbkduo.h - Macintosh PowerBook Duo dock bus
  Emulation by R. Belmont

***************************************************************************/

#ifndef MAME_BUS_NUBUS_PWRBKDUO_PWRBKDUO_H
#define MAME_BUS_NUBUS_PWRBKDUO_PWRBKDUO_H

#pragma once

#include "bus/nubus/nubus.h"
#include "cpu/m68000/m68030.h"
#include "screen.h"

#include <functional>
#include <utility>
#include <vector>

class pwrbkduo_device;

class device_pwrbkduo_card_interface : public device_nubus_card_interface
{
public:
	// construction/destruction
	virtual ~device_pwrbkduo_card_interface();

protected:
	device_pwrbkduo_card_interface(const machine_config &mconfig, device_t &device);
	virtual void interface_pre_start() override;
	pwrbkduo_device &pwrbkduo() { return downcast<pwrbkduo_device&>(nubus()); }
};

class pwrbkduo_slot_device : public nubus_slot_device
{
public:
	// construction/destruction
	template <typename T, typename U>
	pwrbkduo_slot_device(const machine_config &mconfig, T &&tag, device_t *owner, const char *nbtag, U &&opts, const char *dflt)
		: pwrbkduo_slot_device(mconfig, tag, owner, (uint32_t)0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_pwrbkduo_slot(std::forward<T>(nbtag), tag);
		m_nubus_tag = nbtag;
	}

	pwrbkduo_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	template <typename T>
	void set_pwrbkduo_slot(T &&tag, const char *slottag)
	{
		set_nubus_slot(tag, slottag);
		m_pwrbkduo.set_tag(std::forward<T>(tag));
		m_pwrbkduo_slottag = slottag;
	}

	const char *get_bus_tag() const { return m_nubus_tag; }

protected:
	pwrbkduo_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// configuration
	required_device<pwrbkduo_device> m_pwrbkduo;
	const char *m_pwrbkduo_slottag, *m_nubus_tag;
};

DECLARE_DEVICE_TYPE(PWRBKDUO_SLOT, pwrbkduo_slot_device)

DECLARE_DEVICE_TYPE(PWRBKDUO, pwrbkduo_device);
class pwrbkduo_device : public nubus_device
{
public:
	pwrbkduo_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: nubus_device(mconfig, PWRBKDUO, tag, owner, clock),
		  m_maincpu(*this, finder_base::DUMMY_TAG),
		  m_internal_screen(*this, finder_base::DUMMY_TAG),
		  m_write_irq(*this)
	{
		m_bus_mode = NORMAL;
	}

	template <typename... T> void set_maincpu_tag(T &&... args) { m_maincpu.set_tag(std::forward<T>(args)...); }
	template <typename... T> void set_screen_tag(T &&...args) { m_internal_screen.set_tag(std::forward<T>(args)...); }

	auto irq_callback() { return m_write_irq.bind(); }

	bool is_slot_empty() const { return m_device_list.empty(); }
	void dock_irq_w(int state) { m_write_irq(state); }

	m68000_musashi_device &maincpu() { return *m_maincpu; }
	screen_device &screen() { return *m_internal_screen; }
	template <typename T> void install_map(T &device, offs_t start, offs_t end, void (T::*map)(address_map &map))
	{
		space(AS_DATA).install_device(start, end, device, map);
	}

private:
	required_device<m68000_musashi_device> m_maincpu;
	required_device<screen_device> m_internal_screen;

	devcb_write_line m_write_irq;
};

#endif  // MAME_BUS_NUBUS_PWRBKDUO_PWRBKDUO_H

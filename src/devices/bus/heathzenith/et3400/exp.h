// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit ET-3400 Microprocessor Trainer 40-pin accessory connector

  The connector carries the buffered address, data and control buses, so a
  card works by installing itself in the trainer's address space.

****************************************************************************/

#ifndef MAME_BUS_HEATHZENITH_ET3400_EXP_H
#define MAME_BUS_HEATHZENITH_ET3400_EXP_H

#pragma once


class et3400_expansion_slot_device;

class device_et3400_expansion_card_interface : public device_interface
{
public:
	// reset key, active low
	virtual void reset_w(int state) {}

protected:
	device_et3400_expansion_card_interface(const machine_config &mconfig, device_t &device);

	address_space &space() const;

	et3400_expansion_slot_device *const m_slot;
};


class et3400_expansion_slot_device : public device_t,
									 public device_single_card_slot_interface<device_et3400_expansion_card_interface>
{
public:
	template <typename T>
	et3400_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt, bool fixed = false)
		: et3400_expansion_slot_device(mconfig, tag, owner, 0)
	{
		set_options(std::forward<T>(opts), dflt, fixed);
	}

	et3400_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <typename T> void set_space(T &&tag, int spacenum) { m_space.set_tag(std::forward<T>(tag), spacenum); }

	void reset_w(int state) { if (m_card) m_card->reset_w(state); }

	address_space &space() const { return *m_space; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_config_complete() override ATTR_COLD;

private:
	required_address_space m_space;
	device_et3400_expansion_card_interface *m_card;
};


DECLARE_DEVICE_TYPE(ET3400_EXPANSION_SLOT, et3400_expansion_slot_device)

void et3400_expansion_cards(device_slot_interface &device);

#endif // MAME_BUS_HEATHZENITH_ET3400_EXP_H

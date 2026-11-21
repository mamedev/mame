// license:BSD-3-Clause
// copyright-holders:Tim Douglas
/***************************************************************************

    Apple Communication Slot

    The card PROM window is at 50f08000-50f09fff, the card I/O window is
    at 50f0a000-50f0bfff, and the card interrupt is on the NuBus slot 9
    line.

***************************************************************************/

#ifndef MAME_BUS_MACCOMM_MACCOMM_H
#define MAME_BUS_MACCOMM_MACCOMM_H

#pragma once


class device_mac_comm_card_interface;

DECLARE_DEVICE_TYPE(MAC_COMM_SLOT, mac_comm_slot_device)


class mac_comm_slot_device : public device_t, public device_single_card_slot_interface<device_mac_comm_card_interface>
{
public:
	template <typename T, typename U>
	mac_comm_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&space_tag, int space_num, U &&opts, char const *dflt)
		: mac_comm_slot_device(mconfig, tag, owner, 0U)
	{
		set_space(std::forward<T>(space_tag), space_num);
		set_options(std::forward<U>(opts), dflt, false);
	}

	mac_comm_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0U);

	template <typename T> void set_space(T &&tag, int spacenum) { m_space.set_tag(std::forward<T>(tag), spacenum); }
	auto irq_cb() { return m_irq_cb.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;

private:
	required_address_space m_space;
	devcb_write_line m_irq_cb;

	friend class device_mac_comm_card_interface;
};


class device_mac_comm_card_interface : public device_interface
{
protected:
	device_mac_comm_card_interface(machine_config const &mconfig, device_t &device);

	virtual void interface_validity_check(validity_checker &valid) const override ATTR_COLD;

	virtual void install(address_space &space, offs_t base) ATTR_COLD = 0;

	void irq_w(int state)
	{
		if (m_slot)
			m_slot->m_irq_cb(state);
	}

private:
	mac_comm_slot_device *const m_slot;

	friend class mac_comm_slot_device;
};

#endif // MAME_BUS_MACCOMM_MACCOMM_H

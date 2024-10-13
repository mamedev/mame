// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#ifndef MAME_MACHINE_K054321_H
#define MAME_MACHINE_K054321_H

#pragma once

#include "machine/gen_latch.h"

class k054321_device : public device_t
{
public:
	template<typename T, typename U>
	k054321_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&left, U &&right) :
		k054321_device(mconfig, tag, owner, 0)
	{
		m_left.set_tag(std::forward<T>(left));
		m_right.set_tag(std::forward<U>(right));
	}

	k054321_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<device_sound_interface> m_left;
	required_device<device_sound_interface> m_right;
	required_device_array<generic_latch_8_device, 3> m_soundlatch;

	std::unique_ptr<float[]> m_left_gains;
	std::unique_ptr<float[]> m_right_gains;

	u8 m_volume;
	u8 m_active;

	void propagate_volume();

	void volume_reset_w(u8 data);
	void volume_up_w(u8 data);
	void active_w(u8 data);

	u8 busy_r();
	void dummy_w(u8 data);
};

DECLARE_DEVICE_TYPE(K054321, k054321_device)

#endif // MAME_MACHINE_K054321_H

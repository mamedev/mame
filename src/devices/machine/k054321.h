// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#ifndef MAME_MACHINE_K054321_H
#define MAME_MACHINE_K054321_H

#pragma once

#include "machine/gen_latch.h"

class k054321_device : public device_t
{
public:
	k054321_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template<typename T>
	k054321_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&speaker) :
		k054321_device(mconfig, tag, owner)
	{
		m_speaker.set_tag(std::forward<T>(speaker));
	}

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

protected:
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<device_sound_interface> m_speaker;
	required_device_array<generic_latch_8_device, 3> m_soundlatch;

	float m_left_gain, m_right_gain;

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

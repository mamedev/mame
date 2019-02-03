// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#ifndef MAME_MACHINE_K054321_H
#define MAME_MACHINE_K054321_H

#pragma once

class k054321_device : public device_t
{
public:
	template<typename T, typename U>
	k054321_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&left, U &&right)
		: k054321_device(mconfig, tag, owner, 0)
	{
		m_left.set_tag(std::forward<T>(left));
		m_right.set_tag(std::forward<U>(right));
	}

	k054321_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void main_map(address_map &map);
	void sound_map(address_map &map);

protected:
	void device_start() override;

private:
	required_device<device_sound_interface> m_left;
	required_device<device_sound_interface> m_right;

	std::unique_ptr<float[]> m_left_gains;
	std::unique_ptr<float[]> m_right_gains;

	u8 m_main1;
	u8 m_main2;
	u8 m_sound1;

	u8 m_volume;
	u8 m_active;

	void propagate_volume();

	DECLARE_READ8_MEMBER( main1_r);
	DECLARE_WRITE8_MEMBER(main1_w);
	DECLARE_READ8_MEMBER( main2_r);
	DECLARE_WRITE8_MEMBER(main2_w);
	DECLARE_READ8_MEMBER( sound1_r);
	DECLARE_WRITE8_MEMBER(sound1_w);

	DECLARE_WRITE8_MEMBER(volume_reset_w);
	DECLARE_WRITE8_MEMBER(volume_up_w);
	DECLARE_WRITE8_MEMBER(active_w);

	DECLARE_READ8_MEMBER(busy_r);
	DECLARE_WRITE8_MEMBER(dummy_w);

	TIMER_CALLBACK_MEMBER(write_main1);
	TIMER_CALLBACK_MEMBER(write_main2);
	TIMER_CALLBACK_MEMBER(write_sound1);
};

DECLARE_DEVICE_TYPE(K054321, k054321_device)

#endif // MAME_MACHINE_K054321_H

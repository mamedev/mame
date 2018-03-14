// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#ifndef MAME_MACHINE_K054321_H
#define MAME_MACHINE_K054321_H

#pragma once

#define MCFG_K054321_ADD(_tag, _left, _right)   \
	MCFG_DEVICE_ADD(_tag, K054321, 0) \
	downcast<k054321_device *>(device)->set_gain_devices(_left, _right);

class k054321_device : public device_t
{
public:
	k054321_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_gain_devices(const char *_left, const char *_right);

	void main_map(address_map &map);
	void sound_map(address_map &map);

protected:
	void device_start() override;

private:
	required_device<device_sound_interface> m_left, m_right;

	u8 m_main1, m_main2;
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
};

DECLARE_DEVICE_TYPE(K054321, k054321_device)

#endif // MAME_MACHINE_K054321_H

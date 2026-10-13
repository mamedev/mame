// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_BB_BALL_H
#define MAME_TVGAMES_XAVIX_IO_BB_BALL_H

#pragma once

class xavix_baseball_ball_device : public device_t
{
public:
	enum class model
	{
		RAD_BB,
		RAD_BB2
	};

	xavix_baseball_ball_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void set_model(model type) { m_model = type; }
	u8 vertical_buttons_r();
	u8 horizontal_buttons_r();
	u8 motion_r();
	void vblank_w(int state);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr u8 MOTION_LEVELS[4] = { 0x00, 0x40, 0xc0, 0xff };

	required_ioport m_controls;
	required_ioport m_motion;
	model m_model;
	u8 m_motion_level;
	u8 m_motion_fields_remaining;
	bool m_motion_armed;
};

DECLARE_DEVICE_TYPE(XAVIX_BASEBALL_BALL, xavix_baseball_ball_device)

#endif // MAME_TVGAMES_XAVIX_IO_BB_BALL_H

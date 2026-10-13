// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_RKMJ_ARM_H
#define MAME_TVGAMES_XAVIX_IO_RKMJ_ARM_H

#pragma once

#include "xavix_io_ir.h"

class xavix_rockman_arm_device : public device_t
{
public:
	xavix_rockman_arm_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto event_callback() { return m_ir.lookup()->event_callback(); }
	int data_r() { return m_ir->data_r(); }
	void vblank_w(int state);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static constexpr u16 CONTROLLER_REPORTS[3][3] =
	{
		{ 0x0937, 0x1375, 0x0e77 }, // neutral, Shot, turned Shot
		{ 0x0157, 0x0755, 0x1a57 }, // raised
		{ 0x1f67, 0x1965, 0x0467 }  // lowered
	};
	static constexpr u8 SHAKE_PHASE_FRAMES = 12;

	int controller_enabled_r();
	u16 controller_report_r();

	required_device<xavix_ir_report_device> m_ir;
	required_ioport m_arm;
	required_ioport m_controller_buttons;
	required_ioport m_controller_power;
	u8 m_shake_frames;
	bool m_shake_lowered;
};

DECLARE_DEVICE_TYPE(XAVIX_ROCKMAN_ARM, xavix_rockman_arm_device)

#endif // MAME_TVGAMES_XAVIX_IO_RKMJ_ARM_H

// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_RKMJ_ARM_H
#define MAME_TVGAMES_XAVIX_IO_RKMJ_ARM_H

#pragma once

#include "xavix_io_ir.h"

class xavix_rockman_arm_device : public xavix_ir_framed_device
{
public:
	xavix_rockman_arm_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual u32 read_report(unsigned report) override;
	virtual bool report_enabled(unsigned report) override;

private:
	static constexpr u16 CONTROLLER_REPORTS[3][3] =
	{
		{ 0x0937, 0x1375, 0x0e77 }, // neutral, Shot, turned Shot
		{ 0x0157, 0x0755, 0x1a57 }, // raised
		{ 0x1f67, 0x1965, 0x0467 }  // lowered
	};
	static constexpr u16 FACTORY_REPORTS[2] = { 0x0d73, 0x1645 };
	static constexpr u8 SHAKE_PHASE_FRAMES = 12;

	required_ioport m_arm;
	required_ioport m_controller_buttons;
	required_ioport m_controller_power;
	required_ioport m_factory_code;
	u8 m_shake_frames;
	bool m_shake_lowered;
};

DECLARE_DEVICE_TYPE(XAVIX_ROCKMAN_ARM, xavix_rockman_arm_device)

#endif // MAME_TVGAMES_XAVIX_IO_RKMJ_ARM_H

// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_IR_MOTION_H
#define MAME_TVGAMES_XAVIX_IO_IR_MOTION_H

#pragma once

#include "xavix_io_ir.h"

class xavix_ir_motion_base_device : public xavix_ir_msb_device_base
{
protected:
	static constexpr u32 LATER_BIT_TICKS = 0xe5 * 32;

	xavix_ir_motion_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u32 bit_ticks = LATER_BIT_TICKS);

	static u8 motion_report(unsigned player, bool button, u8 strength);

private:
	static constexpr u8 MOTION_REPORTS[2][2][4] =
	{
		{ { 0x75, 0x5d, 0x4b, 0x2d }, { 0x69, 0x57, 0x37, 0x15 } },
		{ { 0x73, 0x59, 0x45, 0x2b }, { 0x65, 0x51, 0x31, 0x13 } }
	};
};

class xavix_ir_motion_device : public xavix_ir_motion_base_device
{
public:
	struct controller_config
	{
		ioport_constructor ports;
		u32 bit_ticks;
	};

	static const controller_config PADDLES;
	static const controller_config PING_PONG_RACKETS;
	static const controller_config POWERED_PING_PONG_RACKETS;
	static const controller_config KICK_SENSORS;
	static const controller_config TENNIS_RACKETS;
	static const controller_config SHAKERS;
	static const controller_config SWORDS;
	static const controller_config XAVIX_RACKETS;

	xavix_ir_motion_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void set_controller(const controller_config &config);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual u32 read_report(unsigned report) override;
	virtual bool report_enabled(unsigned report) override;

private:
	static constexpr u32 ORIGINAL_BIT_TICKS = 0x47 * 64;

	required_ioport_array<2> m_ir_strength;
	required_ioport_array<2> m_ir_button;
	optional_ioport m_ir_power;
	ioport_constructor m_ports;
};

DECLARE_DEVICE_TYPE(XAVIX_IR_MOTION, xavix_ir_motion_device)

#endif // MAME_TVGAMES_XAVIX_IO_IR_MOTION_H

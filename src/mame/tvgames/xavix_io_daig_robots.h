// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_DAIG_ROBOTS_H
#define MAME_TVGAMES_XAVIX_IO_DAIG_ROBOTS_H

#pragma once

#include "xavix_io_ir.h"

#include "screen.h"

#include <utility>

class xavix_daigunder_robots_device : public device_t
{
public:
	xavix_daigunder_robots_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename T> void set_screen(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }
	template <typename... T> void set_event_callback(T &&... args) { m_ir.lookup()->set_event_callback(std::forward<T>(args)...); }
	int data_r() { return m_ir->data_r(); }
	void vblank_w(int state);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	static constexpr u8 CONTROL_REPORTS[2][8] =
	{
		{ 0xf6, 0xf5, 0x6a, 0x69, 0xe7, 0xe4, 0xdd, 0x5a },
		{ 0xd7, 0xd4, 0x53, 0xd1, 0x4d, 0x4b, 0xc6, 0xc5 }
	};
	static constexpr u8 ACCESSORY_REPORTS[8] = { 0xfc, 0x7b, 0x78, 0xf0, 0x6f, 0xe1, 0xc8, 0x50 };
	static constexpr u8 IDENTITY_REPORTS[2][11][2] =
	{
		{
			{ 0x3f, 0xa0 }, { 0xbe, 0x21 }, { 0xbd, 0x22 }, { 0x3c, 0xa3 }, { 0xbb, 0x24 }, { 0x3a, 0xa5 },
			{ 0x39, 0xa6 }, { 0xb8, 0x27 }, { 0xb7, 0x28 }, { 0x36, 0xa9 }, { 0x35, 0xaa }
		},
		{
			{ 0x9f, 0x00 }, { 0x1e, 0x81 }, { 0x1d, 0x82 }, { 0x9c, 0x03 }, { 0x1b, 0x84 }, { 0x9a, 0x05 },
			{ 0x99, 0x06 }, { 0x18, 0x87 }, { 0x17, 0x88 }, { 0x96, 0x09 }, { 0x95, 0x0a }
		}
	};

	u16 control_report_r(unsigned player);
	template <unsigned Player> u16 control_report_r() { return control_report_r(Player); }
	u16 accessory_report_r();
	template <unsigned Part> u16 identity_report_r();

	required_device<xavix_ir_report_device> m_ir;
	required_device<screen_device> m_screen;
	required_ioport_array<2> m_robot_controls;
	required_ioport_array<2> m_robot_id;
	required_ioport m_accessory_controls;
};

DECLARE_DEVICE_TYPE(XAVIX_DAIGUNDER_ROBOTS, xavix_daigunder_robots_device)

#endif // MAME_TVGAMES_XAVIX_IO_DAIG_ROBOTS_H

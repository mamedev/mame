// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_COMT_BATON_H
#define MAME_TVGAMES_XAVIX_IO_COMT_BATON_H

#pragma once

#include "xavix_io_ir.h"

#include <utility>

class xavix_comet_baton_device : public device_t
{
public:
	xavix_comet_baton_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename... T> void set_event_callback(T &&... args) { m_ir.lookup()->set_event_callback(std::forward<T>(args)...); }
	void vblank_w(int state);
	void station_w(offs_t offset, u8 data, u8 mem_mask);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// Motion 3 takes priority when the unsupported Motion 1+3 combination is requested.
	static constexpr u8 MOTION_REPORTS[8] = { 0x0d, 0x01, 0x06, 0x03, 0x07, 0x07, 0x09, 0x09 };

	u16 baton_report_r();
	int baton_active_r();
	static bool burst_length(u64 ticks, unsigned units);
	TIMER_CALLBACK_MEMBER(burst_end);

	required_device<xavix_ir_report_device> m_ir;
	required_ioport m_baton;
	required_ioport m_baton_mode;
	required_ioport m_factory_code;
	output_finder<> m_command;

	emu_timer *m_burst_timer;
	attotime m_burst_start;
	attotime m_last_edge;
	u16 m_received_bits;
	u8 m_bit_count;
	bool m_station_level;
	bool m_carrier;
	bool m_receiving;
};

DECLARE_DEVICE_TYPE(XAVIX_COMET_BATON, xavix_comet_baton_device)

#endif // MAME_TVGAMES_XAVIX_IO_COMT_BATON_H

// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_COMT_BATON_H
#define MAME_TVGAMES_XAVIX_IO_COMT_BATON_H

#pragma once

#include "xavix_io_ir.h"

class xavix_comet_baton_device : public xavix_ir_pulse_distance_device, public xavix_ir_receiver_interface
{
public:
	xavix_comet_baton_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void station_w(offs_t offset, u8 data, u8 mem_mask);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// Motion 3 takes priority when the unsupported Motion 1+3 combination is requested.
	static constexpr u8 MOTION_REPORTS[8] = { 0x0d, 0x01, 0x06, 0x03, 0x07, 0x07, 0x09, 0x09 };

	virtual u32 read_report(unsigned report) override;
	virtual bool reports_enabled() override;
	virtual void ir_burst_started(u64 gap_ticks) override;
	virtual void ir_burst_finished(u64 width_ticks) override;
	virtual void ir_idle_timeout() override;
	static bool burst_length(u64 ticks, unsigned units);

	required_ioport m_baton;
	required_ioport m_baton_mode;
	required_ioport m_factory_code;
	output_finder<> m_command;

	u16 m_received_bits;
	u8 m_bit_count;
	bool m_receiving;
};

DECLARE_DEVICE_TYPE(XAVIX_COMET_BATON, xavix_comet_baton_device)

#endif // MAME_TVGAMES_XAVIX_IO_COMT_BATON_H

// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_BKGJ_SURVI_BUSTER_H
#define MAME_TVGAMES_XAVIX_IO_BKGJ_SURVI_BUSTER_H

#pragma once

#include "xavix_io_ir.h"

#include <array>

class xavix_survi_buster_device : public xavix_ir_msb_device
{
public:
	xavix_survi_buster_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device_t implementation
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual u32 read_report(unsigned report) override;

private:
	static constexpr std::array<u32, 28> SURVI_BUSTER_SAMPLE_TICKS =
	{
		4'264, 2'372, 2'372, 2'437, 2'372, 2'372, 2'372, 2'372,
		2'307, 2'371, 2'371, 2'371, 2'371, 2'371, 2'308, 2'371,
		2'372, 2'371, 2'372, 2'435, 2'372, 2'372, 2'371, 2'372,
		2'372, 2'436, 2'372, 35'099
	};
	static constexpr u32 SURVI_BUSTER_TRIGGER_REPORT = 0x01f87e79;

	required_ioport m_survi_buster;
	required_ioport m_survi_buster_power;
};

DECLARE_DEVICE_TYPE(XAVIX_SURVI_BUSTER, xavix_survi_buster_device)

#endif // MAME_TVGAMES_XAVIX_IO_BKGJ_SURVI_BUSTER_H

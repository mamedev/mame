// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_XAVJMAT_MAT_H
#define MAME_TVGAMES_XAVIX_IO_XAVJMAT_MAT_H

#pragma once

#include "xavix_io_ir.h"

#include <array>

class xavix_jmat_device : public device_t
{
public:
	enum class protocol
	{
		JMAT,
		AEROSTEP
	};

	xavix_jmat_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void set_protocol(protocol type) { m_protocol = type; }
	auto event_callback() { return m_ir.lookup()->event_callback(); }
	int data_r() { return m_ir->data_r(); }
	void vblank_w(int state);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_config_complete() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	static constexpr std::array<u32, 16> JMAT_SAMPLE_TICKS =
	{
		10'144, 7'104, 7'104, 6'464, 4'800, 4'800, 7'264, 8'704,
		8'704, 8'704, 8'704, 8'704, 8'704, 8'704, 8'704, 8'704
	};
	static constexpr std::array<u32, 26> AEROSTEP_SAMPLE_TICKS =
	{
		9'216, 11'520, 9'600, 3'584, 5'376, 4'608, 4'096, 4'096,
		4'480, 4'480, 4'096, 4'096, 4'224, 4'224, 4'480, 4'480,
		3'968, 3'968, 4'608, 4'608, 4'096, 4'096, 4'480, 4'480,
		4'224, 4'224
	};
	static constexpr std::array<u32, 16> AEROSTEP_REPORTS =
	{
		0x99e667, 0x99e001, 0x9987ff, 0x999999,
		0x986679, 0x98601f, 0x9807e1, 0x981987,
		0x81e787, 0x81e1e1, 0x81861f, 0x819879,
		0x866799, 0x8661ff, 0x860601, 0x861867
	};

	static u16 jmat_report(u8 pads);
	u32 mat_report_r();

	required_device<xavix_ir_report_device> m_ir;
	required_ioport m_pads;
	required_ioport m_power;
	protocol m_protocol;
};

DECLARE_DEVICE_TYPE(XAVIX_JMAT, xavix_jmat_device)

#endif // MAME_TVGAMES_XAVIX_IO_XAVJMAT_MAT_H

// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_MSX_MSX_MATSUSHITA_H
#define MAME_MSX_MSX_MATSUSHITA_H

#pragma once

#include "msx_switched.h"


DECLARE_DEVICE_TYPE(MSX_MATSUSHITA, msx_matsushita_device)


class msx_matsushita_device : public device_t,
	public msx_switched_interface,
	public device_nvram_interface
{
public:
	msx_matsushita_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto turbo_callback() { return m_turbo_out_cb.bind(); }

	virtual u8 switched_read(offs_t offset) override;
	virtual void switched_write(offs_t offset, u8 data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	static constexpr u8 MANUFACTURER_ID = 0x08;

	required_ioport m_io_config;
	devcb_write_line m_turbo_out_cb;
	bool m_selected;
	u16 m_address;
	std::vector<u8> m_sram;
	u8 m_nibble1;
	u8 m_nibble2;
	u8 m_pattern;
};

#endif // MAME_MSX_MSX_MATSUSHITA_H

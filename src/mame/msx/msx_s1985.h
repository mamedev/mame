// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_MSX_MSX_S1985_H
#define MAME_MSX_MSX_S1985_H

#pragma once

#include "msx_switched.h"


DECLARE_DEVICE_TYPE(MSX_S1985, msx_s1985_device)


class msx_s1985_device : public device_t,
	public msx_switched_interface,
	public device_nvram_interface
{
public:
	msx_s1985_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// msx_switched_interface overrides
	virtual u8 switched_read(offs_t offset) override;
	virtual void switched_write(offs_t offset, u8 data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_nvram_interface overrides
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	static constexpr u8 MANUFACTURER_ID = 0xfe;
	bool m_selected;
	u8 m_backup_ram_address;
	u8 m_backup_ram[0x10];
	u8 m_color1;
	u8 m_color2;
	u8 m_pattern;
};

#endif // MAME_MSX_MSX_S1985_H

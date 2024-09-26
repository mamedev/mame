// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_MSX_MSX_KANJI12_H
#define MAME_MSX_MSX_KANJI12_H

#pragma once

#include "msx_switched.h"


DECLARE_DEVICE_TYPE(MSX_KANJI12, msx_kanji12_device)


class msx_kanji12_device : public device_t,
	public msx_switched_interface
{
public:
	msx_kanji12_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	void set_rom_start(const char *region) { m_rom_region.set_tag(region); }

	// msx_switched_interface overrides
	virtual u8 switched_read(offs_t offset) override;
	virtual void switched_write(offs_t offset, u8 data) override;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	static constexpr u8 MANUFACTURER_ID = 0xf7;
	required_memory_region m_rom_region;

	bool m_selected;
	u8 m_row;
	u8 m_col;
	u32 m_address;
};

#endif // MAME_MSX_MSX_KANJI12_H

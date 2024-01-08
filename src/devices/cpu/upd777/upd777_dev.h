// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_CPU_UPD777_UPD777_DEV_H
#define MAME_CPU_UPD777_UPD777_DEV_H

#pragma once

#include "emupal.h"
#include "screen.h"

#include "upd777.h"

class upd777_device : public upd777_cpu_device {
public:
	upd777_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual bool get_vbl_state() override;

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void palette_init(palette_device &palette) const;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
};

DECLARE_DEVICE_TYPE(UPD777, upd777_device)

#endif // MAME_CPU_UPD777_UPD777_DEV_H

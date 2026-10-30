// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
#ifndef MAME_SINCLAIR_NEXT_SPECNEXT_VTEST_H
#define MAME_SINCLAIR_NEXT_SPECNEXT_VTEST_H

#pragma once

class specnext_vtest_device : public device_t
{
public:
	specnext_vtest_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void set_raster_offset(u16 offset_h, u16 offset_v) { m_offset_h = offset_h; m_offset_v = offset_v; }

	void draw(bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	u16 m_offset_h, m_offset_v;
};


DECLARE_DEVICE_TYPE(SPECNEXT_VTEST, specnext_vtest_device)
#endif // MAME_SINCLAIR_NEXT_SPECNEXT_VTEST_H

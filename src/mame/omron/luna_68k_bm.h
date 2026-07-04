// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay, O. Galibert

#ifndef MAME_OMRON_LUNA_68K_BM_H
#define MAME_OMRON_LUNA_68K_BM_H

#pragma once

#include "luna_68k_video.h"

#include "video/bt45x.h"
#include "video/hd63484.h"
#include "screen.h"

class luna_68k_bm_device : public device_t, public device_luna_68k_video_interface {
public:
	luna_68k_bm_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock=0);

	virtual void vme_map(address_map &map) override;

public:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<hd63484_device> m_acrtc;
	required_device<bt458_device> m_dac;
	required_device<screen_device> m_screen;
	required_shared_ptr<u16> m_fb;

	void hd63484_map(address_map &map);
	void acrtc_display(bitmap_ind16 &bitmap, const rectangle &cliprect, int y, int x, uint16_t data);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

DECLARE_DEVICE_TYPE(LUNA_68K_BM, luna_68k_bm_device)

#endif

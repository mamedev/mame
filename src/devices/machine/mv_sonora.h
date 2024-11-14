// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*********************************************************************

    Mac video support, "Sonora" edition
    Supports 5 different modelines at up to 16bpp

*********************************************************************/
#ifndef MAME_MACHINE_MAC_VIDEO_SONORA_H
#define MAME_MACHINE_MAC_VIDEO_SONORA_H

#pragma once

#include "emupal.h"
#include "screen.h"

class mac_video_sonora_device : public device_t
{
public:
	mac_video_sonora_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~mac_video_sonora_device() = default;

	uint8_t vctrl_r(offs_t offset);
	void vctrl_w(offs_t offset, uint8_t data);
	uint8_t dac_r(offs_t offset);
	void dac_w(offs_t offset, uint8_t data);

	int vblank() const { return m_screen->vblank(); }
	int hblank() const { return m_screen->hblank(); }

	auto screen_vblank() { return m_screen_vblank.bind(); }

	void set_vram_base(const uint64_t *vram) { m_vram = vram; }
	void set_vram_offset(uint32_t offset) { m_vram_offset = offset; }
	void set_32bit() { m_is32bit = true; }
	void set_PDM() { m_isPDM = true; }
	void set_pixel_clock(uint32_t pclk) { m_extPixelClock = pclk; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	struct modeline {
		uint8_t mode_id;
		const char *name;
		uint32_t dotclock;
		uint32_t htot, hfp, hs, hbp;
		uint32_t vtot, vfp, vs, vbp;
		bool supports_16bpp;
		bool monochrome;
	};

	static const modeline modelines[5];

	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_ioport m_monitor_config;
	devcb_write_line m_screen_vblank;

	const uint64_t *m_vram;
	uint32_t m_vram_offset, m_extPixelClock;
	uint8_t m_mode, m_depth, m_monitor_id, m_vtest;
	uint8_t m_pal_address, m_pal_idx, m_pal_control, m_pal_colkey;
	int m_modeline_id;
	bool m_isPDM;
	bool m_is32bit;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

DECLARE_DEVICE_TYPE(MAC_VIDEO_SONORA, mac_video_sonora_device)

#endif  /* MAME_MACHINE_MAC_VIDEO_SONORA_H */

// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    ACT Apricot XEN Monochrome Display Option

***************************************************************************/

#ifndef MAME_BUS_APRICOT_VIDEO_MONO_H
#define MAME_BUS_APRICOT_VIDEO_MONO_H

#pragma once

#include "video.h"
#include "video/mc6845.h"
#include "emupal.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> apricot_mono_display_device

class apricot_mono_display_device : public device_t, public device_apricot_video_interface
{
public:
	// construction/destruction
	apricot_mono_display_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual bool mem_r(offs_t offset, uint16_t &data, uint16_t mem_mask) override;
	virtual bool mem_w(offs_t offset, uint16_t data, uint16_t mem_mask) override;
	virtual bool io_r(offs_t offset, uint16_t &data, uint16_t mem_mask) override;
	virtual bool io_w(offs_t offset, uint16_t data, uint16_t mem_mask) override;

private:
	required_device<hd6845s_device> m_crtc;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;

	void portb_w(uint8_t data);
	void portx_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	MC6845_UPDATE_ROW(crtc_update_row);
	void crtc_de_w(int state);
	void crtc_vsync_w(int state);

	std::unique_ptr<uint16_t[]> m_vram;

	uint8_t m_portb;
	uint8_t m_portc;
	uint8_t m_portx;
};

// device type definition
DECLARE_DEVICE_TYPE(APRICOT_MONO_DISPLAY, apricot_mono_display_device)

#endif // MAME_BUS_APRICOT_VIDEO_MONO_H

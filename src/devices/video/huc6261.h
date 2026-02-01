// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    Hudson/NEC HuC6261 interface and definitions

**********************************************************************/

#ifndef MAME_VIDEO_HUC6261_H
#define MAME_VIDEO_HUC6261_H

#pragma once

#include "video/huc6270.h"
#include "video/huc6271.h"
#include "video/huc6272.h"

#include "screen.h"


class huc6261_device :  public device_t,
						public device_video_interface
{
public:
	// Screen timing stuff
	static constexpr unsigned WPF = 1365;   // width of a line in frame including blanking areas
	static constexpr unsigned LPF = 263;    // max number of lines in a single frame

	// construction/destruction
	huc6261_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename T> void set_vdc1_tag(T &&tag) { m_huc6270[0].set_tag(std::forward<T>(tag)); }
	template <typename T> void set_vdc2_tag(T &&tag) { m_huc6270[1].set_tag(std::forward<T>(tag)); }
	template <typename T> void set_rainbow_tag(T &&tag) { m_huc6271.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_king_tag(T &&tag) { m_huc6272.set_tag(std::forward<T>(tag)); }

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u16 read(offs_t offset);
	void write(offs_t offset, u16 data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_events);

private:
	required_device_array<huc6270_device, 2> m_huc6270;
	required_device<huc6271_device> m_huc6271;
	required_device<huc6272_device> m_huc6272;
	s32  m_last_h;
	s32  m_last_v;
	s32  m_height;

	u16  m_palette[512];
	u16  m_address;
	u16  m_palette_latch;
	u16  m_register;
	u16  m_control;
	u8   m_priority[7];

	u8   m_pixels_per_clock; // Number of pixels to output per colour clock
	u16  m_pixel_data[7]; // for HuC6270 (BG/SPR layer per chips), KING (4 BG layers), RAINBOW
	u16  m_palette_offset[4];
	u8   m_pixel_clock;

	emu_timer   *m_timer;
	bitmap_rgb32  m_bmp;
	s32   m_uv_lookup[65536][3];

	u32 yuv2rgb(u32 yuv) const;
	void apply_pal_offs(u16 &pix_data) const;
};


DECLARE_DEVICE_TYPE(HUC6261, huc6261_device)

#endif // MAME_VIDEO_HUC6261_H

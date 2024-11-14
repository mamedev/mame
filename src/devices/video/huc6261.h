// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************

    Hudson/NEC HuC6261 interface and definitions

**********************************************************************/

#ifndef MAME_VIDEO_HUC6261_H
#define MAME_VIDEO_HUC6261_H

#pragma once

#include "video/huc6270.h"
#include "video/huc6272.h"


class huc6261_device :  public device_t,
						public device_video_interface
{
public:
	// Screen timing stuff
	static constexpr unsigned WPF = 1365;   // width of a line in frame including blanking areas
	static constexpr unsigned LPF = 263;    // max number of lines in a single frame

	// construction/destruction
	huc6261_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_vdc1_tag(T &&tag) { m_huc6270_a.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_vdc2_tag(T &&tag) { m_huc6270_b.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_king_tag(T &&tag) { m_huc6272.set_tag(std::forward<T>(tag)); }

	void video_update(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_events);

private:
	required_device<huc6270_device> m_huc6270_a;
	required_device<huc6270_device> m_huc6270_b;
	required_device<huc6272_device> m_huc6272;
	int     m_last_h;
	int     m_last_v;
	int     m_height;

	uint16_t  m_palette[512];
	uint16_t  m_address;
	uint16_t  m_palette_latch;
	uint16_t  m_register;
	uint16_t  m_control;
	uint8_t   m_priority[7];

	uint8_t   m_pixels_per_clock; /* Number of pixels to output per colour clock */
	uint16_t  m_pixel_data_a;
	uint16_t  m_pixel_data_b;
	uint16_t  m_palette_offset[4];
	uint8_t   m_pixel_clock;

	emu_timer   *m_timer;
	std::unique_ptr<bitmap_rgb32>  m_bmp;
	int32_t   m_uv_lookup[65536][3];

	inline uint32_t yuv2rgb(uint32_t yuv);
	inline void apply_pal_offs(uint16_t *pix_data);
};


DECLARE_DEVICE_TYPE(HUC6261, huc6261_device)

#endif // MAME_VIDEO_HUC6261_H

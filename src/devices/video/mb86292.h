// license:BSD-3-Clause
// copyright-holders:Angelo Salese
#ifndef MAME_VIDEO_MB86292_H
#define MAME_VIDEO_MB86292_H

#pragma once

#include "machine/ram.h"

class mb86292_device : public device_t,
					   public device_video_interface
{
public:
	mb86292_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::GRAPHICS; }

	template <typename T> void set_screen(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_vram(T &&tag) { m_vram.set_tag(std::forward<T>(tag)); }

	virtual u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

	virtual void vregs_map(address_map &map);

protected:
	mb86292_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	required_device<screen_device> m_screen;
	required_device<ram_device> m_vram;

	void reconfigure_screen();
	void process_display_list();
private:
	u16 m_dce = 0;
	struct {
		u32 lsa = 0, lco = 0;
		bool lreq = false;
		u32 cur_address = 0;
	} m_displaylist;

	struct {
		u16 htp = 0, hdp = 0, hdb = 0, hsp = 0;
		u16 vtr = 0, vsp = 0, vdp = 0;
		u8 hsw = 0, vsw = 0;
	} m_crtc;

	struct {
		u16 xres = 0;
	} m_fb;

	struct {
		u32 cm = 0;
		u16 ch = 0;
		u16 cw = 0;
		bool cc = false;
		u32 cda = 0;
	} m_clayer;

	// NOTE: the access must always be aligned
	u32 vram_read_word(offs_t offset) {
		return m_vram->read(offset) | (m_vram->read(offset + 1) << 8);
	}

	void vram_write_word(offs_t offset, u16 data) {
		m_vram->write(offset,     data & 0xff);
		m_vram->write(offset + 1, (data >> 8) & 0xff);
	}

	u32 vram_read_dword(offs_t offset) {
		return m_vram->read(offset) | (m_vram->read(offset + 1) << 8) | (m_vram->read(offset + 2) << 16) | (m_vram->read(offset + 3) << 24);
	}

	void vram_write_dword(offs_t offset, u32 data) {
		m_vram->write(offset,     data & 0xff);
		m_vram->write(offset + 1, (data >> 8) & 0xff);
		m_vram->write(offset + 2, (data >> 16) & 0xff);
		m_vram->write(offset + 3, (data >> 24) & 0xff);
	}
};

DECLARE_DEVICE_TYPE(MB86292, mb86292_device)


#endif // MAME_VIDEO_MB86292_H

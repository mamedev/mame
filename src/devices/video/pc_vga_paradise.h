// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_VIDEO_PC_VGA_PARADISE_H
#define MAME_VIDEO_PC_VGA_PARADISE_H

#pragma once

#include "video/pc_vga.h"

class pvga1a_vga_device : public svga_device
{
public:
	static constexpr feature_type imperfect_features() { return feature::GRAPHICS; }

	pvga1a_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual uint8_t mem_r(offs_t offset) override;
	virtual void mem_w(offs_t offset, uint8_t data) override;
protected:
	pvga1a_vga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void gc_map(address_map &map) override;

	memory_view m_ext_gc_view;
private:
	u8 address_offset_r(offs_t offset);
	void address_offset_w(offs_t offset, u8 data);
	u8 memory_size_r(offs_t offset);
	void memory_size_w(offs_t offset, u8 data);
	u8 video_select_r(offs_t offset);
	void video_select_w(offs_t offset, u8 data);
	u8 crtc_lock_r(offs_t offset);
	void crtc_lock_w(offs_t offset, u8 data);
	u8 video_control_r(offs_t offset);
	void video_control_w(offs_t offset, u8 data);
	u8 ext_gc_status_r(offs_t offset);
	void ext_gc_unlock_w(offs_t offset, u8 data);

	u8 m_memory_size = 0;
	u8 m_video_control = 0;
	bool m_ext_gc_unlock = false;
	u8 m_video_select = 0;
	u8 m_crtc_lock = 0;
};

class wd90c00_vga_device : public pvga1a_vga_device
{
public:
	wd90c00_vga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void crtc_map(address_map &map) override;
	virtual void device_reset() override;

	memory_view m_ext_crtc_view;
private:
	u8 ext_crtc_status_r(offs_t offset);
	void ext_crtc_unlock_w(offs_t offset, u8 data);

	bool m_ext_crtc_write_unlock = false;
	u8 m_pr10_scratch = 0;
};

DECLARE_DEVICE_TYPE(PVGA1A, pvga1a_vga_device)
DECLARE_DEVICE_TYPE(WD90C00, wd90c00_vga_device)

#endif // MAME_VIDEO_PC_VGA_PARADISE_H

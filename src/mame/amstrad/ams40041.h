// license:BSD-3-Clause
// copyright-holders:Curt Coder

#ifndef MAME_AMSTRAD_AMS40041_H
#define MAME_AMSTRAD_AMS40041_H

#pragma once

#include "video/mc6845.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ams40041_device

class ams40041_device : public mc6845_device
{
public:
	// device type constructor
	ams40041_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// screen update callback
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// memory handlers
	uint8_t video_ram_r(offs_t offset);
	void video_ram_w(offs_t offset, uint8_t data);

	// I/O handlers
	uint8_t vdu_r(offs_t offset);
	void vdu_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// VDU helpers
	static int get_display_mode(uint8_t mode);
	offs_t get_char_rom_offset();
	int get_color(uint8_t data);
	MC6845_UPDATE_ROW(draw_alpha);
	MC6845_UPDATE_ROW(draw_graphics_1);
	MC6845_UPDATE_ROW(draw_graphics_2);
	MC6845_UPDATE_ROW(crtc_update_row);

	std::unique_ptr<uint8_t[]> m_video_ram;
	required_region_ptr<uint8_t> m_char_rom;
	required_ioport m_lk;

	// video state
	int m_toggle;
	int m_lpen;
	int m_blink;
	int m_cursor;
	int m_blink_ctr;
	uint8_t m_vdu_mode;
	uint8_t m_vdu_color;
	uint8_t m_vdu_plane;
	uint8_t m_vdu_rdsel;
	uint8_t m_vdu_border;
};

// device type declaration
DECLARE_DEVICE_TYPE(AMS40041, ams40041_device)

#endif // MAME_AMSTRAD_AMS40041_H

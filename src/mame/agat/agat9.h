// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    agat9video.h

    Implementation of Agat-9 onboard video.

*********************************************************************/

#ifndef MAME_AGAT_AGAT9_H
#define MAME_AGAT_AGAT9_H

#pragma once

#include "machine/ram.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class agat9video_device : public device_t, public device_palette_interface
{
public:
	template <typename T, typename U>
	agat9video_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&ram_tag, U &&char_tag)
		: agat9video_device(mconfig, tag, owner, (uint32_t)0)
	{
		m_ram_dev.set_tag(std::forward<T>(ram_tag));
		m_char_region.set_tag(std::forward<U>(char_tag));
	}

	agat9video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	uint8_t apple_read(offs_t offset);
	void apple_write(offs_t offset, uint8_t data);

	bool m_page2 = false;
	bool m_flash = false;
	bool m_mix = false;
	bool m_graphics = false;
	std::unique_ptr<uint16_t[]> m_hires_artifact_map;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual u32 palette_entries() const noexcept override { return 16; }

	void text_update_lores(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void text_update_hires(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void graph_update_mono_lores(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void graph_update_mono_hires(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void graph_update_color_lores(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void graph_update_color_hires(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);

	void text_update_apple(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void hgr_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);

private:
	void do_io(int offset);
	void do_apple_io(int offset);

	void plot_text_character(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code, const uint8_t *textgfx_data, uint32_t textgfx_datalen, int fg, int bg);
	void plot_text_character_apple(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code, const uint8_t *textgfx_data, uint32_t textgfx_datalen, int fg, int bg);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<screen_device> m_screen;
	required_device<ram_device> m_ram_dev;
	required_memory_region m_char_region;

	uint8_t *m_char_ptr;
	int m_char_size;
	uint32_t m_start_address;
	enum {
		TEXT_LORES = 0,
		TEXT_HIRES,
		GRAPHICS_COLOR_LORES,
		GRAPHICS_COLOR_HIRES,
		GRAPHICS_MONO_LORES,
		GRAPHICS_MONO_HIRES,
		APPLE
	} m_video_mode;
	int m_mode = 0;
	int palette_index = 0;
};

// device type definition
DECLARE_DEVICE_TYPE(AGAT9VIDEO, agat9video_device)

#endif // MAME_AGAT_AGAT9_H

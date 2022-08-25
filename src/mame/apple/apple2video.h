// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    video/apple2.h  - Video handling for Apple II and IIgs

*********************************************************************/

#ifndef MAME_SHARED_APPLE2VIDEO_H
#define MAME_SHARED_APPLE2VIDEO_H

#include "emupal.h"

#define BORDER_LEFT (32)
#define BORDER_RIGHT    (32)
#define BORDER_TOP  (16)    // (plus bottom)

class a2_video_device : public device_t, public device_palette_interface, public device_video_interface
{
public:
	// construction/destruction
	a2_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	bool m_page2 = false;
	bool m_flash = false;
	bool m_mix = false;
	bool m_graphics = false;
	bool m_hires = false;
	bool m_dhires = false;
	bool m_80col = false;
	bool m_altcharset = false;
	bool m_an2 = false;
	bool m_80store = false;
	bool m_monohgr = false;
	u8 m_GSfg = 0, m_GSbg = 0, m_GSborder = 0, m_newvideo = 0, m_monochrome = 0, m_rgbmode = 0;
	u32 m_GSborder_colors[16]{}, m_shr_palette[256]{};
	std::unique_ptr<bitmap_ind16> m_8bit_graphics;
	std::unique_ptr<uint16_t[]> m_hires_artifact_map;
	std::unique_ptr<uint16_t[]> m_dhires_artifact_map;

	uint8_t *m_ram_ptr = nullptr, *m_aux_ptr = nullptr, *m_char_ptr = nullptr;
	int m_char_size = 0;

	int m_sysconfig = 0;

	DECLARE_WRITE_LINE_MEMBER(txt_w);
	DECLARE_WRITE_LINE_MEMBER(mix_w);
	DECLARE_WRITE_LINE_MEMBER(scr_w);
	DECLARE_WRITE_LINE_MEMBER(res_w);
	DECLARE_WRITE_LINE_MEMBER(dhires_w);
	DECLARE_WRITE_LINE_MEMBER(an2_w);

	template<bool iie, bool invert, bool flip>
	void text_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);

	void text_update_ultr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void text_update_jplus(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void text_updateGS(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void lores_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void dlores_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void hgr_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void hgr_update_tk2000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void dhgr_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);

	uint32_t screen_update_GS(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_GS_8bit(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	virtual void device_reset() override;
	virtual void device_start() override;

	virtual uint32_t palette_entries() const override;
	void init_palette();

private:
	template <bool iie, bool invert, bool flip>
	void plot_text_character(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code, int fg, int bg);
	void plot_text_character_ultr(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code, int fg, int bg);
	void plot_text_character_jplus(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code, int fg, int bg);
	void plot_text_characterGS(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code, int fg, int bg);
};

// device type definition
DECLARE_DEVICE_TYPE(APPLE2_VIDEO, a2_video_device)

#endif // MAME_SHARED_APPLE2VIDEO_H

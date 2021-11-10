// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    video/apple2.h  - Video handling for Apple II and IIgs

*********************************************************************/

#ifndef MAME_VIDEO_APPLE2_H
#define MAME_VIDEO_APPLE2_H

#include "emupal.h"

#define BORDER_LEFT (32)
#define BORDER_RIGHT    (32)
#define BORDER_TOP  (16)    // (plus bottom)

class a2_video_device : public device_t, public device_palette_interface, public device_video_interface
{
public:
	// construction/destruction
	a2_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	bool m_page2;
	bool m_flash;
	bool m_mix;
	bool m_graphics;
	bool m_hires;
	bool m_dhires;
	bool m_80col;
	bool m_altcharset;
	bool m_an2;
	bool m_80store;
	bool m_monohgr;
	u8 m_GSfg, m_GSbg, m_GSborder, m_newvideo, m_monochrome, m_rgbmode;
	u32 m_GSborder_colors[16], m_shr_palette[256];
	std::unique_ptr<bitmap_ind16> m_8bit_graphics;
	std::unique_ptr<uint16_t[]> m_hires_artifact_map;
	std::unique_ptr<uint16_t[]> m_dhires_artifact_map;

	uint8_t *m_ram_ptr, *m_aux_ptr, *m_char_ptr;
	int m_char_size;

	int m_sysconfig;

	DECLARE_WRITE_LINE_MEMBER(txt_w);
	DECLARE_WRITE_LINE_MEMBER(mix_w);
	DECLARE_WRITE_LINE_MEMBER(scr_w);
	DECLARE_WRITE_LINE_MEMBER(res_w);
	DECLARE_WRITE_LINE_MEMBER(dhires_w);
	DECLARE_WRITE_LINE_MEMBER(an2_w);

	void text_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void text_update_inverse(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void text_update_ultr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void text_update_orig(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void text_update_spectrum(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void text_update_dodo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
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
	void plot_text_character(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code, int fg, int bg);
	void plot_text_character_ultr(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code, int fg, int bg);
	void plot_text_character_orig(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code, int fg, int bg);
	void plot_text_character_jplus(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code, int fg, int bg);
	void plot_text_character_dodo(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code, int fg, int bg);
	void plot_text_characterGS(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code, int fg, int bg);
};

// device type definition
DECLARE_DEVICE_TYPE(APPLE2_VIDEO, a2_video_device)

#endif // MAME_VIDEO_APPLE2_H

// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    video/apple2.h  - Video handling for 8-bit Apple IIs

*********************************************************************/

#ifndef MAME_VIDEO_APPLE2_H
#define MAME_VIDEO_APPLE2_H


class a2_video_device :
	public device_t
{
public:
	// construction/destruction
	a2_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_PALETTE_INIT(apple2);

	bool m_page2;
	bool m_flash;
	bool m_mix;
	bool m_graphics;
	bool m_hires;
	bool m_dhires;
	bool m_80col;
	bool m_altcharset;
	bool m_an2;
	bool m_monohgr;

	std::unique_ptr<uint16_t[]> m_hires_artifact_map;
	std::unique_ptr<uint16_t[]> m_dhires_artifact_map;

	uint8_t *m_ram_ptr, *m_aux_ptr, *m_char_ptr;
	int m_char_size;

	int m_sysconfig;

	void text_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void text_update_ultr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void text_update_orig(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void text_update_jplus(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void lores_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void dlores_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void hgr_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void hgr_update_tk2000(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);
	void dhgr_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int beginrow, int endrow);

protected:
	virtual void device_reset() override;
	virtual void device_start() override;

private:
	void plot_text_character(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code, const uint8_t *textgfx_data, uint32_t textgfx_datalen, int fg, int bg);
	void plot_text_character_ultr(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code, const uint8_t *textgfx_data, uint32_t textgfx_datalen, int fg, int bg);
	void plot_text_character_orig(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code, const uint8_t *textgfx_data, uint32_t textgfx_datalen, int fg, int bg);
	void plot_text_character_jplus(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code, const uint8_t *textgfx_data, uint32_t textgfx_datalen, int fg, int bg);
};

// device type definition
DECLARE_DEVICE_TYPE(APPLE2_VIDEO, a2_video_device)

#endif // MAME_VIDEO_APPLE2_H

#include "audio/m72.h"

class vigilant_state : public driver_device
{
public:
	vigilant_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_audio(*this, "m72"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_videoram;

	required_device<cpu_device> m_maincpu;
	required_device<m72_audio_device> m_audio;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	int m_horiz_scroll_low;
	int m_horiz_scroll_high;
	int m_rear_horiz_scroll_low;
	int m_rear_horiz_scroll_high;
	int m_rear_color;
	int m_rear_disable;
	int m_rear_refresh;
	bitmap_ind16 *m_bg_bitmap;

	DECLARE_WRITE8_MEMBER(vigilant_bank_select_w);
	DECLARE_WRITE8_MEMBER(vigilant_out2_w);
	DECLARE_WRITE8_MEMBER(kikcubic_coin_w);
	DECLARE_WRITE8_MEMBER(vigilant_paletteram_w);
	DECLARE_WRITE8_MEMBER(vigilant_horiz_scroll_w);
	DECLARE_WRITE8_MEMBER(vigilant_rear_horiz_scroll_w);
	DECLARE_WRITE8_MEMBER(vigilant_rear_color_w);
	virtual void video_start();
	virtual void video_reset();
	UINT32 screen_update_vigilant(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_kikcubic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_background();
	void draw_foreground(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority, int opaque );
	void draw_background(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
};

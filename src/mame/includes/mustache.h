#include "audio/seibu.h"    // for seibu_sound_decrypt on the MAIN cpu (not sound)

class mustache_state : public driver_device
{
public:
	mustache_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_cpu_decrypt(*this, "seibu_sound"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;

	required_device<cpu_device> m_maincpu;
	required_device<seibu_sound_device> m_cpu_decrypt;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	tilemap_t *m_bg_tilemap;
	int m_control_byte;
	DECLARE_WRITE8_MEMBER(mustache_videoram_w);
	DECLARE_WRITE8_MEMBER(mustache_video_control_w);
	DECLARE_WRITE8_MEMBER(mustache_scroll_w);
	DECLARE_DRIVER_INIT(mustache);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start();
	DECLARE_PALETTE_INIT(mustache);
	UINT32 screen_update_mustache(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(mustache_scanline);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
};

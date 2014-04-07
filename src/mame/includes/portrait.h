#include "sound/tms5220.h"

class portrait_state : public driver_device
{
public:
	portrait_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_tms(*this, "tms"),
		m_bgvideoram(*this, "bgvideoram"),
		m_fgvideoram(*this, "fgvideoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_device<tms5200_device> m_tms;
	required_shared_ptr<UINT8> m_bgvideoram;
	required_shared_ptr<UINT8> m_fgvideoram;
	int m_scroll;
	tilemap_t *m_foreground;
	tilemap_t *m_background;
	required_shared_ptr<UINT8> m_spriteram;
	DECLARE_WRITE8_MEMBER(portrait_ctrl_w);
	DECLARE_WRITE8_MEMBER(portrait_positive_scroll_w);
	DECLARE_WRITE8_MEMBER(portrait_negative_scroll_w);
	DECLARE_WRITE8_MEMBER(portrait_bgvideo_write);
	DECLARE_WRITE8_MEMBER(portrait_fgvideo_write);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void video_start();
	DECLARE_PALETTE_INIT(portrait);
	UINT32 screen_update_portrait(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline void get_tile_info( tile_data &tileinfo, int tile_index, const UINT8 *source );
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

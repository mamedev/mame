#include "video/bufsprite.h"
#include "sound/msm5205.h"

class tigeroad_state : public driver_device
{
public:
	tigeroad_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_spriteram(*this, "spriteram") ,
		m_videoram(*this, "videoram"),
		m_ram16(*this, "ram16"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_device<buffered_spriteram16_device> m_spriteram;
	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_ram16;
	int m_bgcharbank;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	DECLARE_WRITE16_MEMBER(f1dream_control_w);
	DECLARE_WRITE16_MEMBER(tigeroad_soundcmd_w);
	DECLARE_WRITE16_MEMBER(tigeroad_videoram_w);
	DECLARE_WRITE16_MEMBER(tigeroad_videoctrl_w);
	DECLARE_WRITE16_MEMBER(tigeroad_scroll_w);
	DECLARE_WRITE8_MEMBER(msm5205_w);
	DECLARE_DRIVER_INIT(f1dream);
	DECLARE_DRIVER_INIT(tigeroad);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILEMAP_MAPPER_MEMBER(tigeroad_tilemap_scan);
	virtual void video_start();
	UINT32 screen_update_tigeroad(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int priority );
	void f1dream_protection_w(address_space &space);
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

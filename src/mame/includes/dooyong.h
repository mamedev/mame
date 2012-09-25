#include "video/bufsprite.h"

class dooyong_state : public driver_device
{
public:
	dooyong_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_spriteram(*this, "spriteram"),
		  m_spriteram16(*this, "spriteram16") ,
		m_txvideoram(*this, "txvideoram"),
		m_paletteram_flytiger(*this, "flytiger_palram"){ }

	optional_device<buffered_spriteram8_device> m_spriteram;
	optional_device<buffered_spriteram16_device> m_spriteram16;
	optional_shared_ptr<UINT8> m_txvideoram;
	optional_shared_ptr<UINT8> m_paletteram_flytiger;
	UINT8 m_sprites_disabled;
	UINT8 m_flytiger_palette_bank;
	UINT8 m_flytiger_pri;
	UINT8 m_tx_pri;
	UINT16 m_rshark_pri;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_bg2_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_fg2_tilemap;
	tilemap_t *m_tx_tilemap;
	UINT8 m_bgscroll8[0x10];
	UINT8 m_bg2scroll8[0x10];
	UINT8 m_fgscroll8[0x10];
	UINT8 m_fg2scroll8[0x10];
	UINT8 *m_bg_tilerom;
	UINT8 *m_bg2_tilerom;
	UINT8 *m_fg_tilerom;
	UINT8 *m_fg2_tilerom;
	UINT8 *m_bg_tilerom2;
	UINT8 *m_bg2_tilerom2;
	UINT8 *m_fg_tilerom2;
	UINT8 *m_fg2_tilerom2;
	int m_bg_gfx;
	int m_bg2_gfx;
	int m_fg_gfx;
	int m_fg2_gfx;
	int m_tx_tilemap_mode;

	int m_interrupt_line_1;
	int m_interrupt_line_2;

	DECLARE_WRITE8_MEMBER(lastday_bankswitch_w);
	DECLARE_WRITE8_MEMBER(flip_screen_w);
	DECLARE_WRITE8_MEMBER(dooyong_bgscroll8_w);
	DECLARE_WRITE8_MEMBER(dooyong_bg2scroll8_w);
	DECLARE_WRITE8_MEMBER(dooyong_fgscroll8_w);
	DECLARE_WRITE8_MEMBER(dooyong_fg2scroll8_w);
	DECLARE_WRITE16_MEMBER(dooyong_bgscroll16_w);
	DECLARE_WRITE16_MEMBER(dooyong_bg2scroll16_w);
	DECLARE_WRITE16_MEMBER(dooyong_fgscroll16_w);
	DECLARE_WRITE16_MEMBER(dooyong_fg2scroll16_w);
	DECLARE_WRITE8_MEMBER(dooyong_txvideoram8_w);
	DECLARE_WRITE8_MEMBER(lastday_ctrl_w);
	DECLARE_WRITE8_MEMBER(pollux_ctrl_w);
	DECLARE_WRITE8_MEMBER(primella_ctrl_w);
	DECLARE_WRITE8_MEMBER(paletteram_flytiger_w);
	DECLARE_WRITE8_MEMBER(flytiger_ctrl_w);
	DECLARE_WRITE16_MEMBER(rshark_ctrl_w);
	DECLARE_READ8_MEMBER(unk_r);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg2_tile_info);
	TILE_GET_INFO_MEMBER(flytiger_get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	DECLARE_MACHINE_START(lastday);
	DECLARE_MACHINE_RESET(sound_ym2203);
	DECLARE_VIDEO_START(lastday);
	DECLARE_VIDEO_START(gulfstrm);
	DECLARE_VIDEO_START(pollux);
	DECLARE_VIDEO_START(bluehawk);
	DECLARE_VIDEO_START(flytiger);
	DECLARE_VIDEO_START(primella);
	DECLARE_VIDEO_START(rshark);
	DECLARE_VIDEO_START(popbingo);
	UINT32 screen_update_lastday(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_gulfstrm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_pollux(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_bluehawk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_flytiger(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_primella(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_rshark(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_popbingo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(rshark_scanline);
};

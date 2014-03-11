#include "audio/t5182.h"

class darkmist_state : public driver_device
{
public:
	darkmist_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spritebank(*this, "spritebank"),
		m_scroll(*this, "scroll"),
		m_videoram(*this, "videoram"),
		m_workram(*this, "workram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_t5182(*this, "t5182"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_shared_ptr<UINT8> m_spritebank;
	required_shared_ptr<UINT8> m_scroll;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_workram;
	required_shared_ptr<UINT8> m_spriteram;

	required_device<cpu_device> m_maincpu;
	required_device<t5182_device> m_t5182;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	int m_hw;
	tilemap_t *m_bgtilemap;
	tilemap_t *m_fgtilemap;
	tilemap_t *m_txtilemap;

	DECLARE_WRITE8_MEMBER(darkmist_hw_w);
	DECLARE_DRIVER_INIT(darkmist);
	TILE_GET_INFO_MEMBER(get_bgtile_info);
	TILE_GET_INFO_MEMBER(get_fgtile_info);
	TILE_GET_INFO_MEMBER(get_txttile_info);
	virtual void video_start();
	DECLARE_PALETTE_INIT(darkmist);
	UINT32 screen_update_darkmist(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(darkmist_scanline);
	void set_pens();
	void decrypt_gfx();
	void decrypt_snd();
};

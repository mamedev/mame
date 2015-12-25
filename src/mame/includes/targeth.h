// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
class targeth_state : public driver_device
{
public:
	targeth_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_videoram(*this, "videoram"),
		m_vregs(*this, "vregs"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT16> m_videoram;
	required_shared_ptr<UINT16> m_vregs;
	required_shared_ptr<UINT16> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	tilemap_t *m_pant[2];

	DECLARE_WRITE16_MEMBER(OKIM6295_bankswitch_w);
	DECLARE_WRITE16_MEMBER(targeth_coin_counter_w);
	DECLARE_WRITE16_MEMBER(targeth_vram_w);
	TILE_GET_INFO_MEMBER(get_tile_info_targeth_screen0);
	TILE_GET_INFO_MEMBER(get_tile_info_targeth_screen1);
	virtual void video_start() override;
	UINT32 screen_update_targeth(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(targeth_interrupt);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};

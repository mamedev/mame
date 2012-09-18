class shootout_state : public driver_device
{
public:
	shootout_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_textram(*this, "textram"),
		m_videoram(*this, "videoram"){ }

	tilemap_t *m_background;
	tilemap_t *m_foreground;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_textram;
	required_shared_ptr<UINT8> m_videoram;
	int m_bFlicker;
	DECLARE_WRITE8_MEMBER(shootout_bankswitch_w);
	DECLARE_WRITE8_MEMBER(sound_cpu_command_w);
	DECLARE_WRITE8_MEMBER(shootout_flipscreen_w);
	DECLARE_WRITE8_MEMBER(shootout_coin_counter_w);
	DECLARE_WRITE8_MEMBER(shootout_videoram_w);
	DECLARE_WRITE8_MEMBER(shootout_textram_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	DECLARE_DRIVER_INIT(shootouj);
	DECLARE_DRIVER_INIT(shootout);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_shootout(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_shootouj(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

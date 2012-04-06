class tryout_state : public driver_device
{
public:
	tryout_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_gfx_control;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	UINT8 m_vram_bank;
	UINT8 *m_vram;
	UINT8 *m_vram_gfx;
	UINT8 *m_spriteram;
	UINT8 *m_spriteram2;
	DECLARE_WRITE8_MEMBER(tryout_nmi_ack_w);
	DECLARE_WRITE8_MEMBER(tryout_sound_w);
	DECLARE_WRITE8_MEMBER(tryout_sound_irq_ack_w);
	DECLARE_WRITE8_MEMBER(tryout_bankswitch_w);
	DECLARE_READ8_MEMBER(tryout_vram_r);
	DECLARE_WRITE8_MEMBER(tryout_videoram_w);
	DECLARE_WRITE8_MEMBER(tryout_vram_w);
	DECLARE_WRITE8_MEMBER(tryout_vram_bankswitch_w);
	DECLARE_WRITE8_MEMBER(tryout_flipscreen_w);
};


/*----------- defined in video/tryout.c -----------*/


PALETTE_INIT( tryout );
VIDEO_START( tryout );
SCREEN_UPDATE_IND16( tryout );

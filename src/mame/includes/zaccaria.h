class zaccaria_state : public driver_device
{
public:
	zaccaria_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_dsw;
	int m_active_8910;
	int m_port0a;
	int m_acs;
	int m_last_port0b;
	int m_toggle;
	UINT8 *m_videoram;
	UINT8 *m_attributesram;
	tilemap_t *m_bg_tilemap;
	UINT8 *m_spriteram;
	UINT8 *m_spriteram2;
	UINT8 m_nmi_mask;
	DECLARE_READ8_MEMBER(zaccaria_dsw_r);
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(sound1_command_w);
	DECLARE_READ8_MEMBER(zaccaria_prot1_r);
	DECLARE_READ8_MEMBER(zaccaria_prot2_r);
	DECLARE_WRITE8_MEMBER(coin_w);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(zaccaria_videoram_w);
	DECLARE_WRITE8_MEMBER(zaccaria_attributes_w);
	DECLARE_WRITE8_MEMBER(zaccaria_flip_screen_x_w);
	DECLARE_WRITE8_MEMBER(zaccaria_flip_screen_y_w);
};


/*----------- defined in video/zaccaria.c -----------*/

PALETTE_INIT( zaccaria );
VIDEO_START( zaccaria );
SCREEN_UPDATE_IND16( zaccaria );

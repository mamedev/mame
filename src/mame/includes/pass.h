class pass_state : public driver_device
{
public:
	pass_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	UINT16 *m_bg_videoram;
	UINT16 *m_fg_videoram;
	DECLARE_WRITE16_MEMBER(pass_bg_videoram_w);
	DECLARE_WRITE16_MEMBER(pass_fg_videoram_w);
};


/*----------- defined in video/pass.c -----------*/


VIDEO_START( pass );
SCREEN_UPDATE_IND16( pass );

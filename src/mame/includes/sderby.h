class sderby_state : public driver_device
{
public:
	sderby_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_spriteram;
	size_t m_spriteram_size;

	UINT16 *m_videoram;
	UINT16 *m_md_videoram;
	UINT16 *m_fg_videoram;

	tilemap_t *m_tilemap;
	tilemap_t *m_md_tilemap;
	tilemap_t *m_fg_tilemap;

	UINT16 m_scroll[6];
	DECLARE_READ16_MEMBER(sderby_input_r);
	DECLARE_READ16_MEMBER(roulette_input_r);
	DECLARE_READ16_MEMBER(rprot_r);
	DECLARE_WRITE16_MEMBER(rprot_w);
	DECLARE_WRITE16_MEMBER(sderby_out_w);
	DECLARE_WRITE16_MEMBER(scmatto_out_w);
	DECLARE_WRITE16_MEMBER(roulette_out_w);
	DECLARE_WRITE16_MEMBER(sderby_videoram_w);
	DECLARE_WRITE16_MEMBER(sderby_md_videoram_w);
	DECLARE_WRITE16_MEMBER(sderby_fg_videoram_w);
	DECLARE_WRITE16_MEMBER(sderby_scroll_w);
};


/*----------- defined in video/sderby.c -----------*/

VIDEO_START( sderby );
SCREEN_UPDATE_IND16( sderby );
SCREEN_UPDATE_IND16( pmroulet );

class shaolins_state : public driver_device
{
public:
	shaolins_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu")
		{ }

	size_t m_spriteram_size;
	UINT8 *m_spriteram;
	UINT8 *m_videoram;
	UINT8 *m_colorram;
	int m_palettebank;

	tilemap_t *m_bg_tilemap;
	UINT8 m_nmi_enable;

	required_device<cpu_device> m_maincpu;
	DECLARE_WRITE8_MEMBER(shaolins_videoram_w);
	DECLARE_WRITE8_MEMBER(shaolins_colorram_w);
	DECLARE_WRITE8_MEMBER(shaolins_palettebank_w);
	DECLARE_WRITE8_MEMBER(shaolins_scroll_w);
	DECLARE_WRITE8_MEMBER(shaolins_nmi_w);
};


/*----------- defined in video/sauro.c -----------*/


PALETTE_INIT( shaolins );
VIDEO_START( shaolins );
SCREEN_UPDATE_IND16( shaolins );

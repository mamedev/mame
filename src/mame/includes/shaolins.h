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
};


/*----------- defined in video/sauro.c -----------*/

WRITE8_HANDLER( shaolins_videoram_w );
WRITE8_HANDLER( shaolins_colorram_w );
WRITE8_HANDLER( shaolins_palettebank_w );
WRITE8_HANDLER( shaolins_scroll_w );
WRITE8_HANDLER( shaolins_nmi_w );

PALETTE_INIT( shaolins );
VIDEO_START( shaolins );
SCREEN_UPDATE( shaolins );

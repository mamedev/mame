class srumbler_state : public driver_device
{
public:
	srumbler_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu")
		{ }

	UINT8 *m_backgroundram;
	UINT8 *m_foregroundram;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	int m_scroll[4];

	required_device<cpu_device> m_maincpu;
};


/*----------- defined in video/srumbler.c -----------*/

WRITE8_HANDLER( srumbler_background_w );
WRITE8_HANDLER( srumbler_foreground_w );
WRITE8_HANDLER( srumbler_scroll_w );
WRITE8_HANDLER( srumbler_4009_w );

VIDEO_START( srumbler );
SCREEN_UPDATE( srumbler );
SCREEN_EOF( srumbler );
